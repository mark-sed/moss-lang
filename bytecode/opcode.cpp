#include "opcode.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include "commons.hpp"
#include "mslib.hpp"
#include "source.hpp"
#include "parser.hpp"
#include "bytecode_reader.hpp"
#include "bytecodegen.hpp"
#include "ir_pipeline.hpp"
#include "mslib_list.hpp"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <initializer_list>

using namespace moss;
using namespace moss::opcode;


/// This macro asserts that condition is true otherwise it raises passed value 
#define op_assert(cond, msg) do { if(!(cond)) raise(msg); } while(0)

FunValue *lookup_method(Interpreter *vm, Value *obj, ustring name, std::initializer_list<Value *> args, diags::DiagID &err);

std::string OpCode::err_mgs(std::string msg, Interpreter *vm) {
    std::stringstream ss;
    ss << vm->get_bci() << "\t" << *this << " :: " << msg;
    return ss.str();
}

/*
When loading unknown value it will be nil, so it cannot be nullptr
void OpCode::check_load(Value *v, Interpreter *vm) {
    if (v) return;
    auto msg = err_mgs("Loading value from non-existent register", vm);
    error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
}*/

/// True iff one of the values is a float and the other is float or int
static bool is_float_expr(Value *v1, Value *v2) {
    return (isa<FloatValue>(v1) && (isa<FloatValue>(v2) || isa<IntValue>(v2))) || 
        (isa<FloatValue>(v2) && (isa<FloatValue>(v1) || isa<IntValue>(v1)));
}

/// True iff both values are ints 
static bool is_int_expr(Value *v1, Value *v2) {
    return isa<IntValue>(v1) && isa<IntValue>(v2);
}

bool opcode::is_type_eq_or_subtype(Value *t1, Value *t2) {
    if (t1 == t2)
        return true;
    if (auto cls1 = dyn_cast<ClassValue>(t1)) {
        auto cls2 = dyn_cast<ClassValue>(t2);
        if (!cls2)
            return false;
        return cls1->has_parent(cls2);
    }
    return false;
}

/// \brief Does a call to a function in the code
/// \param vm Current vm
/// \param funV Function which to call
/// \param args Arguments to pass in there the 0th arg is `this` argument
/// \param constr_class If set them this is taken as constructor call and object is pushed
/// \return Value returned by the function
Value *opcode::runtime_call(Interpreter *vm, FunValue *funV, std::initializer_list<Value *> args, ClassValue *constr_class, bool function_call) {
    assert(funV && "runtime call to nullptr");
    LOGMAX("Doing a runtime call to " << *funV << ", Constructor call: " << (constr_class ? "true" : "false"));
#ifndef NDEBUG
    auto pre_call_cf_size = vm->get_call_frame_size();
#endif
    vm->push_call_frame(funV);
    auto cf = vm->get_call_frame();
    cf->set_function(funV);
    size_t argi = 0;
    for (auto v: args) {
        cf->push_back(v);
        if (argi < funV->get_args().size())
            cf->get_args().back().name = funV->get_args()[argi]->name;
        cf->get_args().back().dst = argi;
        ++argi;
    }
    if (constr_class) {
        LOGMAX("Creating new object for constructor call");
        auto obj = new ObjectValue(constr_class);
        cf->get_args().push_back(CallFrameArg("this", obj, cf->get_args().size()));
        cf->set_constructor_call(true);
    } else if (!function_call) {
        assert(args.size() != 0 && "Missing this argument");
        cf->get_args().back().name = "this";
    }

    Value *ret_v = nullptr;
    if (funV->get_vm() != vm) {
        LOGMAX("Function detected as external, doing cross module call");
        cf->set_extern_module_call(true);
        LOGMAX("Call frame: " << *cf);
        try {
            funV->get_vm()->cross_module_call(funV, cf);
        } catch(Value *v) {
            LOGMAX("Exception in external function call, dropping frame and rethrowing");
            LOGMAX("Dropping call frame after exception");
            vm->drop_call_frame();
            assert(pre_call_cf_size == vm->get_call_frame_size());
            throw v;
        }
        ret_v = cf->get_extern_return_value();
        LOGMAX("Popping call frame");
        vm->pop_call_frame();
    }
    else {
        LOGMAX("Runtime call to method");
        cf->set_runtime_call(true);
        LOGMAX("Call frame: " << *cf);
        try {
            vm->runtime_call(funV);
        } catch(Value *v) {
            LOGMAX("Exception in runtime function call, dropping frame and rethrowing");
            LOGMAX("Dropping call frame after exception");
            vm->pop_call_frame();
            throw v;
        }
        ret_v = cf->get_extern_return_value();
    }
    LOGMAX("Runtime call finished");
    assert(pre_call_cf_size == vm->get_call_frame_size());
    assert(ret_v && "return value not extracted");
    return ret_v;
}

Value *opcode::runtime_function_call(Interpreter *vm, FunValue *funV, std::initializer_list<Value *> args) {
    return runtime_call(vm, funV, args, nullptr, true);
}

Value *opcode::runtime_method_call(Interpreter *vm, FunValue *funV, std::initializer_list<Value *> args) {
    return runtime_call(vm, funV, args, nullptr, false);
}

Value *opcode::runtime_constructor_call(Interpreter *vm, FunValue *funV, std::initializer_list<Value *> args, ClassValue *constr_class) {
    return runtime_call(vm, funV, args, constr_class, false);
}

opcode::IntConst opcode::hash_obj(ObjectValue *obj, Interpreter *vm) {
    diags::DiagID did = diags::DiagID::UNKNOWN;
    auto hashf = opcode::lookup_method(vm, obj, "__hash", {obj}, did);
    if (!hashf) {
        if (did == diags::DiagID::UNKNOWN) {
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_HASH_DEFINED, obj->get_type()->get_name().c_str())));
        } else {
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL,
                "__hash", diags::DIAG_MSGS[did])));
        }
    }
    else {
        auto rv = runtime_method_call(vm, hashf, {obj});
        IntValue *intrv = dyn_cast<IntValue>(rv);
        op_assert(intrv, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NON_INT_FROM_HASH,
            obj->get_type()->get_name().c_str(), rv->get_type()->get_name().c_str())));
        return intrv->get_value();
    }
    return 0;
}

/// Converts value to string
/// \note This might do a runtime call to __String method
opcode::StringConst opcode::to_string(Interpreter *vm, Value *v) {
    if (auto ov = dyn_cast<ObjectValue>(v)) {
        auto string_attr = ov->get_attr(known_names::TO_STRING_METHOD, vm);
        if (string_attr && (isa<FunValue>(string_attr) || isa<FunValueList>(string_attr))) {
            // We need to have a runtime call to the function that executes
            // right now
            Value *err = nullptr;
            auto r_val = mslib::call_type_converter(vm, v, "String", known_names::TO_STRING_METHOD, err);
            if (err)
                return v->as_string();
            assert(r_val && "Runtime call did not return any value");
            return r_val->as_string();
        }
    }
    return v->as_string();
}

void opcode::raise(Value *exc) {
    throw exc;
}

void opcode::output_generator_notes(Interpreter *vm) {
    LOGMAX("Generating notes using generator");
    Interpreter::running_generator = true;
    auto lines = new ListValue(Interpreter::get_generator_notes());
    auto generator = Interpreter::get_generator(clopts::get_note_format());
    assert(generator && "non-existent generator");
    runtime_function_call(vm, generator, {lines});
}

void End::exec(Interpreter *vm) {
    (void)vm;
}

void Load::exec(Interpreter *vm) {
    // Special case is when super is called
    if (this->name == "super") {
        auto ths_v = vm->load_name("this");
        assert(ths_v && "Not inside of a class");
        auto ths = dyn_cast<ObjectValue>(ths_v);
        assert(ths && "this is not an object?!");
        vm->store(this->dst, new SuperValue(ths));
        return;
    }
    auto v = vm->load_name(this->name);
    /*if (!v) {
        // The refered value could possibly be method or attribute in the
        // class or object. Look if "this" is set
        auto ths = vm->load_name("this");
        if (ths) {
            v = ths->get_attr(this->name);
        }
    }*/
    op_assert(v, mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, this->name.c_str())));
    vm->store(this->dst, v);
}

void LoadAttr::exec(Interpreter *vm) {
    auto *v = vm->load(this->src);
    // TODO: Raise type error if type cannot have attributes, such as function
    auto attr = v->get_attr(this->name, vm);
    // This could possibly be an object or a class
    if (!attr && v->get_type() && !isa<EnumTypeValue>(v)) {
        ClassValue *cls = nullptr;
        if (isa<ClassValue>(v)) {
            cls = dyn_cast<ClassValue>(v);
        } else {
            cls = dyn_cast<ClassValue>(v->get_type());
        }

        for (auto sup: cls->get_all_supers()) {
            attr = sup->get_attr(this->name, vm);
            if (attr)
                break;
        }
    } else if (!attr && isa<EnumTypeValue>(v)) {
        auto etv = dyn_cast<EnumTypeValue>(v);
        for (auto ev: etv->get_values()) {
            if (ev->get_name() == this->name) {
                attr = ev;
                break;
            }
        }
    }
    op_assert(attr, mslib::create_attribute_error(
        diags::Diagnostic(*vm->get_src_file(), diags::ATTRIB_NOT_DEFINED,
            v->get_type()->get_name().c_str(), this->name.c_str())));
    vm->store(this->dst, attr);
}

void LoadGlobal::exec(Interpreter *vm) {
    auto *v = vm->load_global_name(this->name);
    op_assert(v, mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::GLOB_NAME_NOT_DEFINED, this->name.c_str())));
    vm->store(this->dst, v);
}

void LoadNonLoc::exec(Interpreter *vm) {
    op_assert(vm->get_top_frame() != vm->get_global_frame(), mslib::create_syntax_error(diags::Diagnostic(*vm->get_src_file(), diags::NON_LOC_IN_GLOB, this->name.c_str())));
    auto v = vm->load_non_local_name(this->name);
    op_assert(v, mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_NON_LOC_BINDING, this->name.c_str())));
    vm->store(this->dst, v);
}

void Store::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    vm->store(this->dst, v);
}

void StoreName::exec(Interpreter *vm) {
    vm->store_name(dst, name);
}

void StoreConst::exec(Interpreter *vm) {
    auto c = vm->load_const(csrc);
    vm->store(dst, c);
}

void StoreAttr::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    op_assert(dstobj->is_modifiable(), mslib::create_attribute_error(
        diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_CREATE_ATTR,
            dstobj->get_name().c_str())));
    auto *v = vm->load(this->src);
    assert(v && "non existent register");
    dstobj->set_attr(this->name, v);
}

void StoreConstAttr::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    op_assert(dstobj->is_modifiable(), mslib::create_attribute_error(
        diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_CREATE_ATTR,
            dstobj->get_name().c_str())));
    auto *v = vm->load_const(this->csrc);
    assert(v && "non existent register");
    dstobj->set_attr(this->name, v);
}

void StoreGlobal::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "non-existent register");
    op_assert(vm->get_global_frame()->overwrite(name, v, vm), mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::GLOB_NAME_NOT_DEFINED, this->name.c_str())));
}

void StoreNonLoc::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "non-existent register");
    op_assert(vm->store_non_local(name, v), mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_NON_LOC_BINDING, this->name.c_str())));
}

static void set_subsc(Interpreter *vm, Value *src, Value *obj, Value *key) {
    if (auto objval = dyn_cast<ObjectValue>(obj)) {
        diags::DiagID did = diags::DiagID::UNKNOWN;
        FunValue *setf = opcode::lookup_method(vm, objval, "__setitem", {key, src, obj}, did);
        if (setf) {
            runtime_method_call(vm, setf, {key, src, objval});
        } else {
            if (did == diags::DiagID::UNKNOWN) {
                raise(mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_SETITEM_DEFINED, objval->get_type()->get_name().c_str())));
            } else {
                raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, "__setitem", diags::DIAG_MSGS[did])));
            }
        }
    } else {
        obj->set_subsc(vm, key, src);
    }
}

void StoreSubsc::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    auto *v = vm->load(this->src);
    assert(v && "non existent register");
    auto *k = vm->load(this->key);
    assert(k && "non existent register");
    set_subsc(vm, v, dstobj, k);
}

void StoreConstSubsc::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    auto *v = vm->load_const(this->csrc);
    assert(v && "non existent register");
    auto *k = vm->load(this->key);
    assert(k && "non existent register");
    set_subsc(vm, v, dstobj, k);
}

void StoreSubscConst::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    auto *v = vm->load(this->src);
    assert(v && "non existent register");
    auto *k = vm->load_const(this->ckey);
    assert(k && "non existent register");
    set_subsc(vm, v, dstobj, k);
}

void StoreConstSubscConst::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    assert(dstobj && "non existent register");
    auto *v = vm->load_const(this->csrc);
    assert(v && "non existent register");
    auto *k = vm->load_const(this->ckey);
    assert(k && "non existent register");
    set_subsc(vm, v, dstobj, k);
}

void StoreIntConst::exec(Interpreter *vm) {
    auto interned = BuiltIns::get_interned_int(val);
    if (interned)
        vm->store_const(dst, interned);
    else
        vm->store_const(dst, new IntValue(val));
}

void StoreFloatConst::exec(Interpreter *vm) {
    vm->store_const(dst, new FloatValue(val));
}

void StoreBoolConst::exec(Interpreter *vm) {
    if (val)
        vm->store_const(dst, BuiltIns::True);
    else
        vm->store_const(dst, BuiltIns::False);
}

void StoreStringConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new StringValue(val));
}

void StoreNilConst::exec(Interpreter *vm) {
    vm->store_const(dst, BuiltIns::Nil);
}

void Jmp::exec(Interpreter *vm) {
    vm->set_bci(this->addr);
}

void BreakTo::exec(Interpreter *vm) {
    assert(state == BreakState::SET && "Break/continue jmp was not setup or break/continue is outside of loop");
    // If there is a finally then we need to execute it first
    if (vm->has_finally()) {
        vm->call_finally();
        return;
    }
    vm->set_bci(this->addr);
}

void JmpIfTrue::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    op_assert(bc, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::BOOL_EXPECTED, v->get_type()->get_name().c_str())));

    if (bc->get_value())
        vm->set_bci(this->addr);
}

void JmpIfFalse::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    op_assert(bc, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::BOOL_EXPECTED, v->get_type()->get_name().c_str())));

    if (!bc->get_value())
        vm->set_bci(this->addr);
}

// TODO: Return reason why it cannot be called
// We determine if we can call passed in function with passed in call frame
// First we copy call arguments so that we can modify them (add default ones and such)
// Then we check if we have more call arguments than function argument, if that is the case
//   then there has to be a vararg, otherwise this cannot be called
// Then we start parsing call arguments, if we encounter named one, then we remember this
//   to check and make sure that only named ones are after this
// For named args we go through actual function args and try to match the names
//   If name is matched then type is matched
// For non-named args we check if the function argument at given position is vararg
//   If it is vararg, then we eat all non-named arguments and add them into a ListValue
//   then we delete call argument collected and replace it with this ListValue
//   If it is not a vararg, so just a positional argument, then we check that the type
//   matches the function argument type at given position
// Some arguments might not have been set and have default value, for those we go
//   though function arguments and if the argument is a vararg it is set to empty list
//   and if it has a default value then we set it to it (clone of the value)
// Finally we just check if fun and call arg size match
/// \return nullptr on success or diagnostic on error
static std::optional<diags::DiagID> can_call(FunValue *f, CallFrame *cf) {
    assert(f && "sanity check");
    LOGMAX("Checking if can call " << *f);
    auto &og_call_args = cf->get_args();
    const auto fun_args = f->get_args();

#ifndef NDEBUG
    unsigned indx = 0;
    for (auto a : og_call_args) {
        LOGMAX(indx << ": " << a);
        ++indx;
    }
#endif

    std::vector<CallFrameArg> call_args;
    CallFrameArg *ths = nullptr;
    if (og_call_args.empty() || og_call_args.back().name != "this")
        call_args.assign(og_call_args.begin(), og_call_args.end());
    else {
        // When class method is called then this is set to the class
        if (dyn_cast<ClassValue>(og_call_args.back().value)) {
            // In such case we use the last arg as this
            if (og_call_args.size() < 2 || !og_call_args[og_call_args.size()-2].name.empty()) {
                LOGMAX("Cannot call, no this argument provided");
                return diags::CLASS_CALL_NEEDS_THIS;
            }
            ths = &(og_call_args[og_call_args.size()-2]);
            ths->name = "this";
            call_args.assign(og_call_args.begin(), og_call_args.end()-2);
            LOGMAX("Set this argument to the last one passed in: " << *ths);
        }
        else {
            // We remove "this" argument for mathching and then append it back
            call_args.assign(og_call_args.begin(), og_call_args.end());
            //ths = &(og_call_args.back());
            LOGMAX("This argument passed in");
        }
    }

#ifndef NDEBUG
    if (call_args.empty()) {
        LOGMAX("No arguments after manipulation");
    } else {
        LOGMAX("Arguments after manipulation");
        indx = 0;
        for (auto a : call_args) {
            LOGMAX(indx << ": " << a);
            ++indx;
        }
    }
#endif

    if (call_args.size() > fun_args.size()) {
        bool is_vararg = false;
        for (auto fa : fun_args) {
            if (fa->vararg) {
                is_vararg = true;
                break;
            }
        }
        if (!is_vararg) {
            LOGMAX("Passed more arguments that the function has parameters");
            return diags::PASSED_MORE_ARGS;
        }
    }

    bool named_args = false;
    for (unsigned i = 0; i < call_args.size(); ++i) {
        CallFrameArg &arg = call_args[i];
        if (named_args || !arg.name.empty()) {
            // Argument with name specified (e.g. a=3)
            assert(!arg.name.empty() && "Non-named arg after a named one");
            bool matched = false;
            for (unsigned j = 0; j < fun_args.size(); ++j) {
                if (fun_args[j]->name == arg.name) {
                    // Check that also type matches
                    matched = fun_args[j]->types.empty();
                    for (auto type: fun_args[j]->types) {
                        if (is_type_eq_or_subtype(arg.value->get_type(), type)) {
                            matched = true;
                            break;
                        }
                    }
                    arg.dst = j;
                    break;
                }
            }
            if (!matched)
                return diags::ARG_MISMATCH;
            // Once first named argument is encountered then all have to be named
            named_args = true;
        }
        else {
            // Unnamed arg
            auto fa = fun_args[i];
            if (fa->vararg) {
                std::vector<Value *> vararg_vals;
                // Eat all arguments that is possible
                unsigned j = i;
                for (; j < call_args.size(); ++j) {
                    CallFrameArg &varg = call_args[j];
                    if (varg.name.empty()) {
                        vararg_vals.push_back(varg.value);
                    }
                    else {
                        break;
                    }
                }
                LOGMAX("Setting call argument " << i << " to " << j << " as vararg list");
                // Erase arguments since they will be 1 list
                call_args.erase(call_args.begin() + i, call_args.begin() + j);
                call_args.insert(call_args.begin() + i, CallFrameArg(fa->name, new ListValue(vararg_vals), i));
            }
            else {
                // No types always matches otherwise it will be false and set in for
                bool matched = fa->types.empty();
                for (auto type: fa->types) {
                    if (is_type_eq_or_subtype(arg.value->get_type(), type)) {
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                    return diags::ARG_MISMATCH;
                arg.dst = i;
                arg.name = fa->name;
                LOGMAX("Assigned argument " << arg.name);
            }
        }
    }

    // Find not set values and set them in call
    // We have to go through all the fun_args, because there might be a case
    // where some value in the middle is left out by dirrect assignment
    // E.g.:
    //      fun foo(a=1, b=2, c=3) {}
    //      foo(c=0, a=2)
    //
    if (fun_args.size() > call_args.size()) {
        LOGMAX("Call args are missing some values set -- try setting defaults");
        for (unsigned i = 0; i < fun_args.size(); ++i) {
            auto fa = fun_args[i];
            LOGMAX("Analyzing arg " << fa->name);
            bool found = false;
            for (auto &ca: call_args) {
                if (ca.name == fa->name) {
                    found = true;
                    break;
                }
            }
            // If not found then set it if it has default value or is vararg
            if (!found) {
                if (fa->vararg) {
                    LOGMAX("Setting empty list for vararg argument " << fa->name << " %" << i);
                    call_args.push_back(CallFrameArg(fa->name, new ListValue(), i));
                }
                else if (fa->default_value) {
                    LOGMAX("Setting default value for argument " << fa->name << " %" << i << " as " << *fa->default_value);
                    call_args.push_back(CallFrameArg(fa->name, fa->default_value->clone(), i));
                }
                else {
                    LOGMAX("Argument does not have a default value and is not set: " << fa->name);
                    return diags::ARG_MISMATCH;
                }
            }
        }
    }

    // The function can be called only if call arguments are 1:1 to function args
    if (call_args.size() == fun_args.size()) {
        if (ths) {
            ths->name = "this";
            ths->dst = call_args.size();
            call_args.push_back(*ths);
        }
        cf->set_args(call_args);
        LOGMAX("Call frame set -- function callable: " << *f << " with:\n" << *cf);
        return std::nullopt;
    }
    return diags::ARG_MISMATCH;
}

void call(Interpreter *vm, Register dst, Value *funV) {
    LOGMAX("Call to : " << *funV);
    auto cf = vm->get_call_frame();
    cf->set_return_reg(dst);
    cf->set_caller_addr(vm->get_bci()+1);

    assert(funV && "nullptr function passed in");

    ClassValue *constructor_of = nullptr;
    Value *super_caller = nullptr;
    auto as_fun = dyn_cast<FunValue>(funV);
    bool constr_call = isa<ClassValue>(funV) || isa<SuperValue>(funV);
    if (as_fun)
        // Just asign as if this is true then the above cannot be
        constr_call = as_fun->is_constructor();
    // Class constructor call
    if (constr_call) {
        LOGMAX("Constructor call");
        // Check if this is set and if so remove it (e.g `mod1.MyClass()`)
        if (!cf->get_args().empty() && cf->get_args().back().name == "this") {
            // this argument is set for all attribute calls (as we don't know the
            // type in bytecodegen), so remove it if the value should not know it
            // and since we are in a constructor call there shall be no this.
            LOGMAX("Removing this arg from constructor call");
            cf->get_args().pop_back();
        }
        ClassValue *cls = dyn_cast<ClassValue>(funV);
        if (auto spr = dyn_cast<SuperValue>(funV)) {
            super_caller = vm->load_name("this");
            assert(super_caller && "Super call, yet super caller is not set");
            // super() call, so extract the class based on the mro
            auto inst_type = spr->get_instance()->get_type();
            auto inst_cls = dyn_cast<ClassValue>(inst_type);
            assert(inst_cls && "Object type is not a class");
            op_assert(!inst_cls->get_all_supers().empty(), mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_SUPER, inst_cls->get_name().c_str())));
            // We need to go through mro and find the first parent with constructor
            for (auto parent: inst_cls->get_all_supers()) {
                if (!parent->get_attrs())
                    continue;
                if (parent->get_attrs()->load_name(parent->get_name(), vm)) {
                    cls = parent;
                    break;
                }
            }
            if (!cls) {
                LOGMAX("Super call has no constructor to call, return");
                vm->store(cf->get_return_reg(), BuiltIns::Nil);
                LOGMAX("Popping call frame 1");
                vm->pop_call_frame();
                return;
            }
            LOGMAX("Super call to: " << *cls);
        } else if (as_fun) {
            cls = as_fun->get_constructee();
        }
        assert(cls && "sanity check");
        constructor_of = cls;
        if (cls->get_attrs())
            funV = cls->get_attrs()->load_name(cls->get_name(), vm);
        if (!funV) {
            // No constructor provided, look for one in parent classes
            for (auto parent : cls->get_all_supers()) {
                if (!parent->get_attrs()) continue;
                funV = parent->get_attrs()->load_name(parent->get_name(), vm);
                if (funV)
                    break;
            }
            // No constructor is provided so execute implicit one
            // by setting this and returning
            if (!funV) {
                LOGMAX("No constructor provided, executing implicit one");
                auto obj = new ObjectValue(constructor_of);
                vm->store(cf->get_return_reg(), obj);
                // Also pop call frame
                LOGMAX("Popping call frame 2");
                vm->pop_call_frame();
                return;
            }
        }
    }
    else if ((!cf->get_args().empty() && cf->get_args().back().name == "this" && !has_methods(cf->get_args().back().value)) 
              || funV->has_annotation("staticmethod")) {
        // this argument is set for all attribute calls (as we don't know the
        // type in bytecodegen), so remove it if the value should not know it
        LOGMAX("Removing this from non-object call");
        cf->get_args().pop_back();
    }
    FunValue *fun = dyn_cast<FunValue>(funV);
    if (!fun) {
        auto fvl = dyn_cast<FunValueList>(funV);
        op_assert(fvl, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_CALLABLE, funV->get_name().c_str())));
        
        std::optional<diags::DiagID> err_id;
        // Walk functions and check if it can be called
        for (auto f: fvl->get_funs()) {
            err_id = can_call(f, cf);
            if (!err_id) {
                fun = f;
                break;
            }
        }
        if (!fun) {
            LOGMAX("Popping call frame 3");
            vm->pop_call_frame();
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, 
                fvl->back()->get_name().c_str(), diags::DIAG_MSGS[*err_id])));
        }
    }
    else {
        std::optional<diags::DiagID> err_id = can_call(fun, cf);
        if (err_id) {
            // Pop frame so that call stack is correct
            LOGMAX("Popping call frame 4");
            vm->pop_call_frame();
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL,
                fun->get_name().c_str(), diags::DIAG_MSGS[*err_id])));
        }
    }

    cf->set_function(fun);

    // Set this object if constructor is being called
    if (constructor_of) {
        Value *obj = nullptr;
        if (super_caller) {
            obj = super_caller;
            LOGMAX("Super constructor detected, passing this: " << *obj);
        } else {
            obj = new ObjectValue(constructor_of);
            LOGMAX("Constructor detected, creating object and passing in: " << *obj);
        }
        cf->get_args().push_back(CallFrameArg("this", obj, cf->get_args().size()));
        cf->set_constructor_call(true);
        LOGMAX(*cf);
    } else if (!cf->get_args().empty() && cf->get_args().back().name == "this" && isa<SuperValue>(cf->get_args().back().value)) {
        LOGMAX("Super value used as this, passing in 'this' from current frame");
        auto ths_super = vm->load_name("this");
        assert(ths_super && "Calling method of super, but this is not present");
        cf->get_args().back().value = ths_super;
        LOGMAX("This for super call set to: " << *ths_super);
    }

    if (fun->has_annotation(annots::INTERNAL)) {
        Value *err = nullptr;
        ustring name = fun->get_name();
        ustring module_name = fun->get_vm()->get_src_file()->get_module_name();
        mslib::dispatch(vm, module_name, name, err);
        if (err) {
            raise(err);
            return;
        }
    }
    else if (fun->get_vm() != vm) { // External
        LOGMAX("Function detected to be from other module");
        LOGMAX("Setting up module for its function call");
        // This calls the function
        LOGMAX("Calling different module's function");
        cf->set_extern_module_call(true);
        fun->get_vm()->cross_module_call(fun, vm->get_call_frame());
        LOGMAX("External function has handed over control to original module");
        // This is after return from the function
        vm->store(cf->get_return_reg(), cf->get_extern_return_value());
        vm->set_bci(cf->get_caller_addr());
        // Remove already pushed in call frame
        LOGMAX("Popping call frame 6");
        vm->pop_call_frame();
        if (fun->get_vm()->get_exit_code() != 0) {
            LOGMAX("Delagating exit code from external module");
            vm->set_exit_code(fun->get_vm()->get_exit_code());
        }
    }
    else {
        vm->push_frame(fun);
        vm->set_bci(fun->get_body_addr());
    }
}

static FunValue *select_function(Value *fun, std::initializer_list<Value *> args, diags::DiagID &err) {
    assert(fun);
    FunValue *funf = dyn_cast<FunValue>(fun);
    if (!funf && isa<FunValueList>(fun)) {
        auto funflist = dyn_cast<FunValueList>(fun);
        for (auto f : funflist->get_funs()) {
            CallFrame cf;
            for (auto a: args) {
                cf.push_back(a);
            }
            auto rv = can_call(f, &cf);
            if (!rv) {
                return f;
            }
            err = *rv;
        }
    }
    if (funf) {
        CallFrame cf;
        for (auto a: args) {
            cf.push_back(a);
        }
        auto rv = can_call(funf, &cf);
        if (!rv) {
            return funf;
        }
        err = *rv;
    }
    return nullptr;
}

FunValue *opcode::lookup_method(Interpreter *vm, Value *obj, ustring name, std::initializer_list<Value *> args, diags::DiagID &err) {
    auto method = obj->get_attr(name, vm);
    if (method) {
        return select_function(method, args, err);
    }
    return nullptr;
}

FunValue *lookup_function(Interpreter *vm, ustring name, std::initializer_list<Value *> args, diags::DiagID &err) {
    auto fun = vm->load_name(name);
    if (fun) {
        return select_function(fun, args, err);
    }
    return nullptr;
}

void call(Interpreter *vm, Register dst, Value *funV, std::initializer_list<Value *> args) {
    vm->push_call_frame(funV);
    for (auto v: args) {
        vm->get_call_frame()->push_back(v);
    }
    call(vm, dst, funV);
}

void call_method(Interpreter *vm, Register dst, Value *funV, std::initializer_list<Value *> args) {
    vm->push_call_frame(funV);
    for (auto v: args) {
        vm->get_call_frame()->push_back(v);
    }
    assert(args.size() != 0 && "Missing this argument");
    vm->get_call_frame()->get_args().back().name = "this";
    call(vm, dst, funV);
}

void call_operator(Interpreter *vm, const char * op, Value *s1, Value *s2, Register dst) {
    auto v = s1->get_attr(op, vm);
    op_assert(v, mslib::create_type_error(
        diags::Diagnostic(*vm->get_src_file(), diags::OPERATOR_NOT_DEFINED,
            s1->get_type()->get_name().c_str(), op)));
    call_method(vm, dst, v, {s2, s1});
}

void call_operator_unary(Interpreter *vm, ustring op, Value *s1, Register dst) {
    auto v = s1->get_attr(op, vm);
    op_assert(v, mslib::create_type_error(
        diags::Diagnostic(*vm->get_src_file(), diags::OPERATOR_NOT_DEFINED,
            s1->get_type()->get_name().c_str(), op.c_str())));
    call_method(vm, dst, v, {s1});
}

void Call::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "register does not contain a value");
    call(vm, dst, v);
}

void CallFormatter::exec(Interpreter *vm) {
    diags::DiagID did;
    assert(vm->get_call_frame()->get_args().size() == 1 && "Note should have 1 arg and that is the note string");
    auto arg = vm->get_call_frame()->get_args().back();
    auto formatf = lookup_function(vm, name, {arg.value}, did);
    if (formatf && formatf->has_annotation("formatter")) {
        LOGMAX("Formatter found");
        call(vm, dst, formatf);
    }
    else {
        LOGMAX("Formatter not found, storing string as NoteValue");
        auto strarg = dyn_cast<StringValue>(arg.value);
        assert(strarg && "somehow note value is not a string value");
        vm->store(dst, new NoteValue(name, strarg));
        vm->pop_call_frame();
    }
}

void PushFrame::exec(Interpreter *vm) {
    vm->push_frame();
}

void PopFrame::exec(Interpreter *vm) {
    vm->pop_frame();
}

void PushCallFrame::exec(Interpreter *vm) {
    vm->push_call_frame();
}

void PopCallFrame::exec(Interpreter *vm) {
    // This pops values from call frame into current frame
    // The acutal call frame is removed in return (when return value is set)
    for (auto a: vm->get_call_frame()->get_args()) {
        vm->store(a.dst, a.value);
        assert(!a.name.empty() && "argument name in call frame was not set");
        vm->store_name(a.dst, a.name);
    }
}

void Return::exec(Interpreter *vm) {
    // If there is a finally then we need to execute it first
    if (vm->has_finally()) {
        vm->call_finally();
        return;
    }

    auto cf = vm->get_call_frame();
    auto return_reg = cf->get_return_reg();
    auto caller_addr = cf->get_caller_addr();
    Value *ret_v = nullptr;
    if (cf->is_constructor_call()) {
        ret_v = cf->get_args().back().value;
    }
    else {
        ret_v = vm->load(src);
    }
    assert(ret_v && "Return register does not contain any value??");

    // Formatter return value conversion to NoteValue
    assert(cf->get_function() && "cannot check function annotations");
    if (cf->get_function()->has_annotation("formatter") && !isa<NoteValue>(ret_v)) {
        StringValue *rv_str = dyn_cast<StringValue>(ret_v);
        if (!rv_str)
            rv_str = new StringValue(to_string(vm, ret_v));
        ret_v = new NoteValue(cf->get_function()->get_name(), rv_str);
    }

    if (cf->is_extern_module_call() || cf->is_runtime_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        LOGMAX("Dropping call frame");
        vm->drop_call_frame();
        vm->pop_frame();
    } else {
        vm->pop_frame();
        vm->store(return_reg, ret_v);
        vm->set_bci(caller_addr);
        LOGMAX("Popping call frame");
        vm->pop_call_frame();
    }
}

void ReturnConst::exec(Interpreter *vm) {
    // If there is a finally then we need to execute it first
    if (vm->has_finally()) {
        vm->call_finally();
        return;
    }

    auto cf = vm->get_call_frame();
    auto return_reg = cf->get_return_reg();
    auto caller_addr = cf->get_caller_addr();
    Value *ret_v = nullptr;
    if (cf->is_constructor_call()) {
        // this value is the last argument
        ret_v = cf->get_args().back().value;
    }
    else {
        ret_v = vm->load_const(csrc);
    }
    assert(ret_v && "Return register does not contain any value??");

    // Formatter return value conversion to NoteValue
    assert(cf->get_function() && "cannot check function annotations");
    if (cf->get_function()->has_annotation("formatter") && !isa<NoteValue>(ret_v)) {
        StringValue *rv_str = dyn_cast<StringValue>(ret_v);
        if (!rv_str)
            rv_str = new StringValue(to_string(vm, ret_v));
        ret_v =  new NoteValue(cf->get_function()->get_name(), rv_str);
    }

    if (cf->is_extern_module_call() || cf->is_runtime_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        LOGMAX("Dropping call frame");
        vm->drop_call_frame();
        vm->pop_frame();
    } else {
        vm->pop_frame();
        vm->store(return_reg, ret_v);
        vm->set_bci(caller_addr);
        LOGMAX("Popping call frame");
        vm->pop_call_frame();
    }
}

void PushArg::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "Non existent push reg");
    vm->get_call_frame()->push_back(v);
}

void PushConstArg::exec(Interpreter *vm) {
    auto v = vm->load_const(csrc);
    assert(v && "Const does not exist??");
    vm->get_call_frame()->push_back(v);
}

void PushNamedArg::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "Const does not exist??");
    vm->get_call_frame()->push_back(name, v);
}

void PushUnpacked::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "Const does not exist??");
    if (auto vobj = dyn_cast<ObjectValue>(v)) {
        diags::DiagID did = diags::DiagID::UNKNOWN;
        FunValue *iterf = lookup_method(vm, vobj, "__iter", {vobj}, did);
        Value *iterator = vobj;
        if (iterf) {
            // When iter is found then call it and use the return value otherwise use the object itself
            iterator = runtime_method_call(vm, iterf, {iterator});
        }
        if (isa<ObjectValue>(iterator)) {
            did = diags::DiagID::UNKNOWN;
            FunValue *nextf = lookup_method(vm, iterator, "__next", {iterator}, did);
            if (nextf) {
                while (true) {
                    try {
                        auto v = runtime_method_call(vm, nextf, {iterator});
                        assert(v && "sanity check");
                        vm->get_call_frame()->push_back(v);
                    } catch (Value *ve) {
                        if (ve->get_type() == BuiltIns::StopIteration) {
                            break;
                        }
                        else
                            throw ve;
                    }
                }
            } else {
                if (did == diags::DiagID::UNKNOWN)
                    raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_NEXT_DEFINED, vobj->get_type()->get_name().c_str())));
                else
                    raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, "__next", diags::DIAG_MSGS[did])));
            }
            return;
        } else {
            v = iterator;
        }
    }
    
    if (auto vlst = dyn_cast<ListValue>(v)) {
        for (auto elem: vlst->get_vals()) {
            vm->get_call_frame()->push_back(elem);
        }
    } else if (auto vstr = dyn_cast<StringValue>(v)) {
        for (auto elem: vstr->get_value()) {
            vm->get_call_frame()->push_back(new StringValue(ustring(1, elem)));
        }
    } else if (auto vdct = dyn_cast<DictValue>(v)) {
        for (auto elem: vdct->get_vals()) {
            for (auto [k, v]: elem.second) {
                auto kname = dyn_cast<StringValue>(k);
                op_assert(kname, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::KEYWORD_NOT_A_STRING,
                    k->get_type()->get_name().c_str())));
                vm->get_call_frame()->push_back(kname->get_value(), v);
            }
        }
    } else {
        raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_ITERABLE_TYPE,
                v->get_type()->get_name().c_str())));
    }
}

void CreateFun::exec(Interpreter *vm) {
    FunValue *funval = new FunValue(name, arg_names, vm);
    // Check if this is in class frame and if the names match, set this as constructor
    Value *pown = vm->get_top_frame()->get_pool_owner();
    if (pown && isa<ClassValue>(pown)) {
        if (name == pown->get_name()) {
            LOGMAX("Setting function " << name << " as constructor");
            funval->set_constructee(dyn_cast<ClassValue>(pown));
        } else if (pown->has_annotation("internal_bind")) {
            // This might be a constructor of internal_bind class where the names don't match
            auto annt = pown->get_annotation("internal_bind");
            if (auto ann_v = dyn_cast<StringValue>(annt)) {
                if (ann_v->get_value() == name) {
                    LOGMAX("Setting function " << name << " as constructor (matched on internal_bind name)");
                    // This value will change in class bind once internal_bind is executed. But this needs to be 
                    // denoted as a constructor
                    funval->set_constructee(dyn_cast<ClassValue>(pown));
                }
            } else {
                assert(false && "internal_bind annotation without string value");
            }
        }
    }
    for (auto riter = vm->get_frames().rbegin(); riter != vm->get_frames().rend(); ++riter) {
        // Push all latest local frames as closures of current function
        if ((*riter)->is_global())
            break;
        funval->push_closure(*riter);
    }
    auto f = vm->get_top_frame()->load_name(name, vm);
    if (f && (isa<FunValue>(f) || isa<FunValueList>(f))) {
        // In here we alway push the function into the function list as it is
        // not fully initialized and the possible override happens after
        // types are set (FUN_BEGIN)
        if (auto fv = dyn_cast<FunValue>(f)) {
            vm->store(this->fun, new FunValueList(std::vector<FunValue *>{fv, funval}));
            vm->store_name(this->fun, name);
        }
        else {
            auto fvl = dyn_cast<FunValueList>(f);
            fvl->push_back(funval);
            vm->store(this->fun, fvl);
        }
    }
    else {
        vm->store(this->fun, funval);
        vm->store_name(this->fun, name);
    }
}

static FunValue *load_last_fun(Register fun, Interpreter *vm) {
    auto v = vm->load(fun);
    assert(v && "Fun not bound?");
    auto fv = dyn_cast<FunValue>(v);
    if (fv)
        return fv;
    auto fvl = dyn_cast<FunValueList>(v);
    assert(fvl && "FunBegin bound to non-function value");
    fv = fvl->back();
    assert(fv && "nullptr returned from funvaluelist");
    return fv;
}

void FunBegin::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    // Set address to the opcode after jump which is after this
    fv->set_body_addr(vm->get_bci()+2);
    // Now we can see it this function should override some other in the
    // possible FunValueList
    auto v = vm->load(fun);
    if (auto fvl = dyn_cast<FunValueList>(v)) {
        for (size_t i = 0; i < fvl->get_funs().size() - 1; ++i) {
            if (fv->equals(fvl->get_funs()[i])) {
                if (fvl->get_funs().size() == 2) {
                    vm->store(fun, fv);
                } else {
                    fvl->get_funs()[i] = fv;
                    // Remove the last one, which is fv
                    fvl->get_funs().pop_back();
                    break;
                }
            }
        }
    }
}

void SetDefault::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    auto defv = vm->load(src);
    assert(defv && "Value does not exist anymore?");
    fv->set_default(index, defv);
}

void SetDefaultConst::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    auto defv = vm->load_const(csrc);
    assert(defv && "Value does not exist anymore?");
    fv->set_default(index, defv);
}

void SetType::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    assert(fv && "Could not load fun");
    auto tv = vm->load(type);
    assert(tv && "No longer exists?");
    if (!isa<ClassValue>(tv)) {
        // When using the same type as is the class we are in, there might be
        // constructor first found, in that case find the type by the name
        ustring name;
        if (auto tvf = dyn_cast<FunValue>(tv)) {
            name = tvf->get_name();
        } else if (auto tvf = dyn_cast<FunValueList>(tv)) {
            name = tvf->back()->get_name();
        }
        if (!name.empty()) {
            tv = vm->load_type(name);
        }

        op_assert(tv && (isa<ClassValue>(tv) || isa<EnumTypeValue>(tv)), mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_A_TYPE, tv->as_string().c_str())));
    }
    fv->set_type(index, tv);
}

void SetVararg::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    fv->set_vararg(index);
}

ModuleValue *opcode::load_module(Interpreter *vm, ustring name) {
    LOGMAX("Loading module: " << name);
    // First see if we have .msb and then .ms
    std::optional<ustring> path_opt = get_file_path(name+".msb");
    bool is_msb = true;
    if (path_opt == std::nullopt) {
        LOGMAX(".msb module not found");
        path_opt = get_file_path(name+".ms");
        is_msb = false;
        if (path_opt == std::nullopt) {
            LOGMAX(".ms module not found");
            return nullptr;
        }
    }
    ustring path = *path_opt;
    Bytecode *bc = nullptr;
    File *input_file = nullptr;

    if (is_msb) {
        LOGMAX("Reading bytecode");
        auto ibf = new BytecodeFile(path);
        input_file = ibf;
        BytecodeReader bcreader(*ibf);
        bc = bcreader.read();
        // TODO: Allow for incorrect read, but raise an exception
#ifndef NDEBUG
        if (name != "libms")
            LOGMAX("Read bytecode: \n" << *bc);
#endif
    } else {
        auto module_file = new SourceFile(path, SourceFile::SourceType::FILE);
        input_file = module_file;
        Parser parser(*module_file);
        auto module_ir = parser.parse();
        ir::IRPipeline ipl(parser);
        if (auto err = ipl.run(module_ir)) {
            module_ir = err;
        }
        if (auto exc = dyn_cast<ir::Raise>(module_ir)) {
            // Parser error... there is not VM so it is returned as a StringValue
            // place it into an exception and raise it correctly
            auto msg = dyn_cast<ir::StringLiteral>(exc->get_exception());
            assert(msg && "Raise from parser should be string value");
            auto v = mslib::create_syntax_error(msg->get_value());
            delete module_ir;
            raise(v);
        }
        bc = new Bytecode();
        bcgen::BytecodeGen cgen(bc);
        cgen.generate(module_ir);
        LOGMAX("Generated bytecode: \n" << *bc);
        delete module_ir;
    }

    auto mod_i = new Interpreter(bc, input_file);
    // We need to create the module value before running it so that gc can
    // access its values while its running
    auto gen_mod = new ModuleValue(name, mod_i->get_global_frame(), mod_i);
    vm->push_currently_imported_module(gen_mod);
    mod_i->run();
    if (mod_i->get_exit_code() != 0) {
        LOGMAX("Import exited, delegating exit code");
        vm->set_exit_code(mod_i->get_exit_code());
    }
    assert(vm->top_currently_imported_module() == gen_mod && "Currently generated incorrectly popped?");
    vm->pop_currently_imported_module();
    return gen_mod;
}

void Import::exec(Interpreter *vm) {
    auto mod = load_module(vm, name);
    op_assert(mod, mslib::create_module_not_found_error(
        diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_FIND_MODULE, name.c_str())));
    vm->store(dst, mod);
}

void ImportAll::exec(Interpreter *vm) {
    auto mod = vm->load(src);
    assert(mod && "Non existent module for import all");
    assert((isa<ModuleValue>(mod) || isa<SpaceValue>(mod)) && "TODO: raise spilling something else than module or space");
    vm->push_spilled_value(mod);
}

// These are the built in classes where layout would clash
static bool is_built_in_class(ClassValue *c) {
    return c == BuiltIns::Int || c == BuiltIns::Float || c == BuiltIns::Bool || c == BuiltIns::String ||
        c == BuiltIns::Note || c == BuiltIns::NilType || c == BuiltIns::List || c == BuiltIns::Dict;
}

void PushParent::exec(Interpreter *vm) {
    auto v = vm->load(parent);
    assert(v && "Non existent class");
    auto cv = dyn_cast<ClassValue>(v);
    assert(cv && "Pushed parent is not a class");
    if (is_built_in_class(cv)) {
        for (auto p: vm->get_parent_list()) {
            op_assert(!is_built_in_class(p), mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(),
                diags::PARENT_CONFLICT, p->get_name().c_str(), cv->get_name().c_str())));
        }
    }
    op_assert(cv != BuiltIns::NilType, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_EXTEND_NIL)));
    vm->push_parent(cv);
}

void BuildClass::exec(Interpreter *vm) {
    auto cls = new ClassValue(name, vm->get_parent_list());
    vm->store(dst, cls);
    vm->store_name(dst, name);
    vm->clear_parent_list();
    vm->push_frame(cls);
    cls->set_attrs(vm->get_top_frame());
}

std::vector<ustring> get_str_list_annot(Value *args, unsigned long arg_am, ustring an_name, Interpreter *vm) {
    std::vector<ustring> extr;
    if(auto argl = dyn_cast<ListValue>(args)) {
        for (auto v : argl->get_vals()) {
            auto els = dyn_cast<StringValue>(v);
            // TODO: Change to error message for unexpected type in annotation
            op_assert(els, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE,
                "String", v->get_type()->get_name().c_str())));
            extr.push_back(els->get_value());
        }
    } else {
        auto argstr = dyn_cast<StringValue>(args);
        if (argstr && arg_am == 1) {
            return {argstr->get_value()};
        }
        // TODO: Change to error message for unexpected type in annotation
        raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE,
            "List of Strings", args->get_type()->get_name().c_str())));
    }
    op_assert(extr.size() == arg_am, mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::MISMATCHED_ANNOT_ARG_AM,
        an_name.c_str(), arg_am, extr.size())));
    return extr;
}

void Annotate::exec(Interpreter *vm) {
    auto *d = vm->load(dst);
    assert(d && "Cannot load dst");
    auto *v = vm->load(val);
    assert(v && "Cannot load val");
    // Try to cast it for possible later usage
    FunValue *fn = dyn_cast<FunValue>(d);
    if (auto fl = dyn_cast<FunValueList>(d)) {
        // Annotate is called right after CreateFun so it has the be the top
        // one in the list
        fn = fl->back();
        fn->annotate(name, v);
    } else {
        d->annotate(name, v);
    }

    if (name == "internal_bind") {
        LOGMAX("Internal binding");
        auto bind_name = dyn_cast<StringValue>(v);
        op_assert(bind_name, mslib::create_type_error(
            diags::Diagnostic(*vm->get_src_file(), diags::MISSING_ANNOT_TYPE_ARGUMENT,
                "internal_bind", "String")));
        auto bind_val = vm->load_name(bind_name->get_value());
        op_assert(bind_val, mslib::create_name_error(
            diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, bind_name->get_value().c_str())));
        auto bind_class = dyn_cast<ClassValue>(bind_val);
        op_assert(bind_class, mslib::create_type_error(
            diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE,
                "Type", bind_val->get_type()->get_name().c_str())));
        auto ref_class = dyn_cast<ClassValue>(d);
        op_assert(ref_class, mslib::create_type_error(
            diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE,
                "Type", d->get_type()->get_name().c_str())));
        bind_class->bind(ref_class);
        // Remove the bound class
        vm->remove_global_name(ref_class->get_name());
    } else if (name == "converter") {
        op_assert(fn, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::CONVERTER_ON_NONFUN,
            d->get_type()->get_name().c_str())));
        auto args = get_str_list_annot(v, 2, name, vm);
        Interpreter::add_converter(args[0], args[1], fn);
    } else if (name == "generator") {
        op_assert(fn, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::GENERATOR_ON_NONFUN,
            d->get_type()->get_name().c_str())));
        auto args = get_str_list_annot(v, 1, name, vm);
        Interpreter::add_generator(args[0], fn);
    }
}

void AnnotateMod::exec(Interpreter *vm) {
    auto *v = vm->load(val);
    assert(v && "Cannot load val");
    if (name == "enable_code_output") {
        op_assert(isa<NilValue>(v), mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::ENABLE_CODE_OUT_ARG_SET)));
        vm->set_enable_code_output(true);
    } else if (name == "disable_code_output") {
        op_assert(isa<NilValue>(v), mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::DISABLE_CODE_OUT_ARG_SET)));
        vm->set_enable_code_output(false);
    } else if (name == "internal_module") {
        op_assert(isa<NilValue>(v), mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::DISABLE_CODE_OUT_ARG_SET)));
        mslib::call_const_initializer(vm->get_src_file()->get_module_name(), vm);
    } else {
        raise(mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::UNKNOWN_MODULE_ANNOTATION, name.c_str())));
    }
}

void Document::exec(Interpreter *vm) {
    auto d = vm->load(dst);
    assert(d && "loading non-existent register");
    d->set_attr("__doc", new StringValue(val), true);
}

void Output::exec(Interpreter *vm) {
    if (clopts::disable_notes) {
        LOGMAX("Notes disabled, not outputting");
        return;
    }
    auto *v = vm->load(src);
    assert(v && "Cannot load src");

    auto get_value_format = [vm](Value *v) {
        if (auto nv = dyn_cast<NoteValue>(v)) {
            auto f = nv->get_attr("format", vm);
            assert(f && "note does not have a format");
            return f->as_string();
        }
        return ustring("txt");
    };

    auto val_format = get_value_format(v);
    auto target_format = clopts::get_note_format();
    if (!Interpreter::running_generator && Interpreter::is_generator(target_format)) {
        LOGMAX("Generator format selected so saving note for later call to generator");
        Interpreter::add_generator_note(v);
        return;
    }
    else if (val_format != target_format) {
        auto key = std::make_pair(val_format, target_format);
        auto converter = Interpreter::get_converter(key);

        op_assert(!converter.empty(), mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_FIND_CONVERTER,
            val_format.c_str(), target_format.c_str())));
    
        assert(v);
        for (auto f: converter) {
            v = runtime_function_call(vm, f, {v});
            assert(v && "no return from converter?");
        }
    }
    auto ov = to_string(vm, v);
    
    // notebook output
    if (vm->is_enable_code_output() && val_format == "txt") {
        if (target_format == "md") {
            ov = "_[Output]:_\n```\n" + ov;
            if (ov.back() != '\n')
                ov += "\n";
            ov += "```\n";
        } else if (target_format == "html") {
            if (ov.back() == '\n')
                ov.pop_back();
            ov = "<pre><code class=\"moss-output\">" + ov + "</code></pre>";
        }
    }
    
    clopts::get_note_stream() << ov;
    if (clopts::print_notes)
        outs << ov;
}

static Value *concat(Value *s1, Value *s2, Interpreter *vm) {
    (void)vm;
    assert(s1 && "Value or nil should have been loaded");
    assert(s2 && "Value or nil should have been loaded");

    ustring s1_str = to_string(vm, s1);
    ustring s2_str = to_string(vm, s2);

    return new StringValue(s1_str + s2_str);
}

void Concat::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat2::exec(Interpreter *vm) {
    auto res = concat(vm->load_const(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat3::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load_const(src2), vm);
    vm->store(dst, res);
}

static void raise_operand_exc(Interpreter *vm, const char *op, Value *s1, Value *s2) {
    raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNSUPPORTED_OPERAND_TYPE,
                op, s1->get_type()->get_name().c_str(), s2->get_type()->get_name().c_str())));
}

static void raise_operand_exc(Interpreter *vm, const char *op, Value *s1) {
    raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNSUPPORTED_UN_OPERAND_TYPE,
                op, s1->get_type()->get_name().c_str())));
}

// TODO: For most exprs is to check over/underflow when check is enabled
static Value *exp(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void)vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(static_cast<long>(std::pow(static_cast<double>(i1->get_value()), static_cast<double>(i2->get_value()))));
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(std::pow(s1->as_float(), s2->as_float()));
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "^", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "^", s1, s2);
    }
    return res;
}

void Exp::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Exp2::exec(Interpreter *vm) {
    auto res = exp(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Exp3::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *add(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void)vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() + i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() + s2->as_float());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "+", s1, s2, dst);
    }
    else if (isa<ListValue>(s1) && isa<ListValue>(s2)) {
        auto vec1 = dyn_cast<ListValue>(s1)->get_vals();
        auto vec2 = dyn_cast<ListValue>(s2)->get_vals();
        std::vector<Value *> joint;
        joint.insert(joint.end(), vec1.begin(), vec1.end());
        joint.insert(joint.end(), vec2.begin(), vec2.end());
        res = new ListValue(joint);
    }
    else {
        raise_operand_exc(vm, "+", s1, s2);
    }
    return res;
}

void Add::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Add2::exec(Interpreter *vm) {
    auto res = add(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Add3::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *sub(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void)vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() - i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() - s2->as_float());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "-", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "-", s1, s2);
    }
    return res;
}

void Sub::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Sub2::exec(Interpreter *vm) {
    auto res = sub(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Sub3::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *div(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void)vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        op_assert(i2->get_value() != 0, mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::DIV_BY_ZERO)));
        res = new IntValue(i1->get_value() / i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        op_assert(s2->as_float() != 0.0,  mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::FDIV_BY_ZERO)));
        res = new FloatValue(s1->as_float() / s2->as_float());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "/", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "/", s1, s2);
    }
    return res;
}

void Div::exec(Interpreter *vm) {
    auto res = div(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Div2::exec(Interpreter *vm) {
    auto res = div(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Div3::exec(Interpreter *vm) {
    auto res = div(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *mul(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() * i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() * s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<IntValue>(s2)) {
        // String repeating
        ustring ss1 = dyn_cast<StringValue>(s1)->get_value();
        IntConst i1 = dyn_cast<IntValue>(s2)->get_value();
        if (i1 <= 0 || ss1.empty())
            res = new StringValue("");
        else if (ss1.size() == 1) {
            res = new StringValue(ustring(i1, ss1[0]));
        } else {
            ustring result;
            result.reserve(ss1.size() * i1);
            for (int i = 0; i < i1; ++i)
                result += ss1;
            res = new StringValue(result);
        }
    } 
    else if (isa<ListValue>(s1) && isa<IntValue>(s2)) {
        // List repeating
        auto l1 = dyn_cast<ListValue>(s1)->get_vals();
        IntConst i2 = dyn_cast<IntValue>(s2)->get_value();
        if (i2 <= 0 || l1.empty())
            res = new ListValue();
        else {
            std::vector<Value *> result;
            result.reserve(l1.size() * i2);  // optional but efficient

            for (IntConst i = 0; i < i2; ++i) {
                result.insert(result.end(), l1.begin(), l1.end());
            }
            res = new ListValue(result);
        }
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "*", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "*", s1, s2);
    }
    return res;
}

void Mul::exec(Interpreter *vm) {
    auto res = mul(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Mul2::exec(Interpreter *vm) {
    auto res = mul(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Mul3::exec(Interpreter *vm) {
    auto res = mul(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *mod(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        op_assert(i2->get_value() != 0, mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::DIV_BY_ZERO)));
        res = new IntValue(i1->get_value() % i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        op_assert(s2->as_float() != 0.0,  mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::FDIV_BY_ZERO)));
        res = new FloatValue(std::fmod(s1->as_float(), s2->as_float()));
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "%", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "%", s1, s2);
    }
    return res;
}

void Mod::exec(Interpreter *vm) {
    auto res = mod(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Mod2::exec(Interpreter *vm) {
    auto res = mod(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Mod3::exec(Interpreter *vm) {
    auto res = mod(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

bool opcode::eq(Value *s1, Value *s2, Interpreter *vm) {
    if (s1->get_kind() == s2->get_kind() && !isa<ObjectValue>(s1)) {
        if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
            IntValue *i2 = dyn_cast<IntValue>(s2);
            return i1->get_value() == i2->get_value();
        }
        else if (FloatValue *f1 = dyn_cast<FloatValue>(s1)) {
            FloatValue *f2 = dyn_cast<FloatValue>(s2);
            return f1->get_value() == f2->get_value();
        }
        else if (BoolValue *b1 = dyn_cast<BoolValue>(s1)) {
            BoolValue *b2 = dyn_cast<BoolValue>(s2);
            return b1->get_value() == b2->get_value();
        }
        else if (StringValue *st1 = dyn_cast<StringValue>(s1)) {
            StringValue *st2 = dyn_cast<StringValue>(s2);
            return st1->get_value() == st2->get_value();
        }
        else if (isa<NilValue>(s1)) {
            return true;
        }
        else if (EnumValue *ev1 = dyn_cast<EnumValue>(s1)) {
            EnumValue *ev2 = dyn_cast<EnumValue>(s2);
            return ev1 == ev2;
        }
        else if (ListValue *lt1 = dyn_cast<ListValue>(s1)) {
            ListValue *lt2 = dyn_cast<ListValue>(s2);
            if (lt1->get_vals().size() != lt2->get_vals().size())
                return false;
            for (size_t i = 0; i < lt1->get_vals().size(); ++i) {
                auto v1 = lt1->get_vals()[i];
                auto v2 = lt2->get_vals()[i];
                if (!eq(v1, v2, vm))
                    return false;
            }
            return true;
        }
        else if (DictValue *dv1 = dyn_cast<DictValue>(s1)) {
            DictValue *dv2 = dyn_cast<DictValue>(s2);
            if (dv1->get_vals().size() != dv2->get_vals().size())
                return false;
            auto vals1 = dv1->get_vals();
            auto vals2 = dv2->get_vals();
            for (auto [hsh, vect] : vals1) {
                auto vect2_it = vals2.find(hsh);
                if (vect2_it == vals2.end()) {
                    return false;
                }
                for (auto [k, v] : vect) {
                    bool matched = false;
                    for (auto [k2, v2] : vect2_it->second) {
                        if (eq(k, k2, vm) && eq(v, v2, vm)) {
                            matched = true;
                            break;
                        }
                    }
                    if (!matched)
                        return false;
                }
            }
            return true;
        }
        else {
            return s1 == s2;
        }
    }
    else if (is_float_expr(s1, s2)) {
        return s1->as_float() == s2->as_float();
    }
    else if (isa<NilValue>(s1) || isa<NilValue>(s1)) {
        return false;
    }
    else if (ObjectValue *ob1 = dyn_cast<ObjectValue>(s1)) {
        diags::DiagID did = diags::DiagID::UNKNOWN;
        auto op_fun = lookup_method(vm, ob1, "==", {s2, ob1}, did);
        if (!op_fun) {
            if (did == diags::DiagID::UNKNOWN) {
                raise(mslib::create_type_error(
                    diags::Diagnostic(*vm->get_src_file(), diags::OPERATOR_NOT_DEFINED,
                        s1->get_type()->get_name().c_str(), "==")));
            } else {
                raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL,
                    "(==)", diags::DIAG_MSGS[did])));
            }
        }
        auto rv = runtime_method_call(vm, op_fun, {s2, s1});
        BoolValue *boolrv = dyn_cast<BoolValue>(rv);
        op_assert(boolrv, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NON_BOOL_FROM_EQ,
            ob1->get_type()->get_name().c_str(), rv->get_type()->get_name().c_str())));
        return boolrv->get_value();
    }
    return false;
}

void Eq::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load(src2), vm);
    vm->store(dst, new BoolValue(res));
}

void Eq2::exec(Interpreter *vm) {
    auto res = eq(vm->load_const(src1), vm->load(src2), vm);
    vm->store(dst, new BoolValue(res));
}

void Eq3::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load_const(src2), vm);
    vm->store(dst, new BoolValue(res));
}

/*static bool can_eq(Value *s) {
    return isa<IntValue>(s) || isa<FloatValue>(s) || isa<BoolValue>(s) ||
        isa<NilValue>(s) || isa<StringValue>(s) || isa<ListValue>(s);
}*/

static Value *neq(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    Value *res = nullptr;
    if (isa<ObjectValue>(s1) || isa<ObjectValue>(s2)) {
        call_operator(vm, "!=", s1, s2, dst);
    }
    else {
        // TODO: Perhaps this should call can_eq??
        auto eqRes = eq(s1, s2, vm);
        auto neqRes = new BoolValue(!eqRes);
        return neqRes;
    }
    return res;
}

void Neq::exec(Interpreter *vm) {
    auto res = neq(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Neq2::exec(Interpreter *vm) {
    auto res = neq(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Neq3::exec(Interpreter *vm) {
    auto res = neq(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *bt(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() > i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() > s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() > st2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, ">", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, ">", s1, s2);
    }
    return res;
}

void Bt::exec(Interpreter *vm) {
    auto res = bt(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Bt2::exec(Interpreter *vm) {
    auto res = bt(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Bt3::exec(Interpreter *vm) {
    auto res = bt(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *lt(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() < i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() < s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() < st2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "<", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "<", s1, s2);
    }
    return res;
}

void Lt::exec(Interpreter *vm) {
    auto res = lt(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Lt2::exec(Interpreter *vm) {
    auto res = lt(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Lt3::exec(Interpreter *vm) {
    auto res = lt(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *beq(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() >= i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() >= s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() >= st2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, ">=", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, ">=", s1, s2);
    }
    return res;
}

void Beq::exec(Interpreter *vm) {
    auto res = beq(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Beq2::exec(Interpreter *vm) {
    auto res = beq(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Beq3::exec(Interpreter *vm) {
    auto res = beq(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *leq(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() <= i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() <= s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() <= st2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "<=", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "<=", s1, s2);
    }
    return res;
}

void Leq::exec(Interpreter *vm) {
    auto res = leq(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Leq2::exec(Interpreter *vm) {
    auto res = leq(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Leq3::exec(Interpreter *vm) {
    auto res = leq(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *in(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        // s1 in s2 => s2.find(s1)
        res = new BoolValue(st2->get_value().find(st1->get_value()) != ustring::npos);
    }
    else if (isa<ObjectValue>(s2)) {
        // Here we need to flip the arguments as to call the method on the collection
        call_operator(vm, "in", s2, s1, dst);
    }
    else if (auto lst = dyn_cast<ListValue>(s2)) {
        for (auto v: lst->get_vals()) {
            if (eq(v, s1, vm)) {
                return new BoolValue(true);
            }
        }
        return new BoolValue(false);
    }
    else if (auto dct = dyn_cast<DictValue>(s2)) {
        auto vals = dct->get_vals();
        auto dit = vals.find(hash(s1, vm));
        if (dit == vals.end())
            return new BoolValue(false);
        for (std::pair<Value *, Value *> p: dit->second) {
            if (opcode::eq(p.first, s1, vm)) {
                return new BoolValue(true);
            }
        }
        return new BoolValue(false);
    }
    else {
        raise_operand_exc(vm, "in", s1, s2);
    }
    return res;
}

void In::exec(Interpreter *vm) {
    auto res = in(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void In2::exec(Interpreter *vm) {
    auto res = in(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void In3::exec(Interpreter *vm) {
    auto res = in(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *andOP(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() & i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() && b2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "and", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "and", s1, s2);
    }
    return res;
}

void And::exec(Interpreter *vm) {
    auto res = andOP(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void And2::exec(Interpreter *vm) {
    auto res = andOP(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void And3::exec(Interpreter *vm) {
    auto res = andOP(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *orOP(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() | i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() || b2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "or", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "or", s1, s2);
    }
    return res;
}

void Or::exec(Interpreter *vm) {
    auto res = orOP(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Or2::exec(Interpreter *vm) {
    auto res = orOP(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Or3::exec(Interpreter *vm) {
    auto res = orOP(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static Value *xorOP(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() ^ i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() ^ b2->get_value());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "xor", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "xor", s1, s2);
    }
    return res;
}

void Xor::exec(Interpreter *vm) {
    auto res = xorOP(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Xor2::exec(Interpreter *vm) {
    auto res = xorOP(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Xor3::exec(Interpreter *vm) {
    auto res = xorOP(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

static void extract_range(Value *r, opcode::IntConst &start, opcode::IntConst &end, opcode::IntConst &step, Interpreter *vm) {
    // TODO: Raise exception if asserted
    auto rngobj = dyn_cast<ObjectValue>(r);
    assert(rngobj && "sanity check");
    auto start_v = rngobj->get_attr("start", vm);
    assert(start_v && "range does not have a start attribute");
    auto step_v = rngobj->get_attr("step", vm);
    assert(step_v && "range does not have a step attribute");
    auto end_v = rngobj->get_attr("end", vm);
    assert(end_v && "range does not have a end attribute");

    auto start_i = dyn_cast<IntValue>(start_v);
    assert(start_i && "start in range is not an int");
    auto step_i = dyn_cast<IntValue>(step_v);
    assert(step_i && "step in range is not an int");
    auto end_i = dyn_cast<IntValue>(end_v);
    assert(end_i && "end in range is not an int");

    start = start_i->get_value();
    end = end_i->get_value();
    step = step_i->get_value();
}

static inline ustring str_index(StringValue *s, IntConst i) {
    auto st = s->get_value();
    if (i < 0) {
        return ustring(1, st[st.size()+i]);
    }
    return ustring(1, st[i]);
}

static inline bool is_oob(StringValue *s, IntConst i) {
    ustring st = s->get_value();
    return (i < 0 && static_cast<unsigned long>(i*-1) > st.size()) || 
              (i > 0 && static_cast<unsigned long>(i) >= st.size());
}

static inline bool is_oob(ListValue *s, IntConst i) {
    auto st = s->get_vals();
    return (i < 0 && static_cast<unsigned long>(i*-1) > st.size()) || 
              (i > 0 && static_cast<unsigned long>(i) >= st.size());
}

static inline Value *list_index(ListValue *s, IntConst i) {
    auto lt = s->get_vals();
    if (i < 0) {
        return lt[lt.size()+i];
    }
    return lt[i];
}

static Value *subsc(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    // "txt"[0]
    if (auto st1 = dyn_cast<StringValue>(s1)) {
        if (auto i2 = dyn_cast<IntValue>(s2)) {
            if (is_oob(st1, i2->get_value())) {
                raise(mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, s1->get_type()->get_name().c_str(), i2->get_value())));
            }
            res = new StringValue(str_index(st1, i2->get_value()));
        } else if (s2->get_type() == BuiltIns::Range) {
            opcode::IntConst start;
            opcode::IntConst end;
            opcode::IntConst step;
            extract_range(s2, start, end, step, vm);
            std::stringstream ss;
            // Here we don't do oob check
            // Note: this is not in line with Python's slice, but does not raise exception on oob
            for (IntConst i = start; (step < 0 && i > end) || (step >= 0 && i < end); i += step) {
                if (is_oob(st1, i)) break;
                ss << str_index(st1, i);
            }
            res = new StringValue(ss.str());
        } else {
            raise_operand_exc(vm, "[]", s1, s2);
        }
    }
    else if (auto lt1 = dyn_cast<ListValue>(s1)) {
        if (auto i2 = dyn_cast<IntValue>(s2)) {
            if ((i2->get_value() < 0 && static_cast<unsigned long>(i2->get_value()*-1) > lt1->get_vals().size()) || 
              (i2->get_value() >= 0 && static_cast<unsigned long>(i2->get_value()) >= lt1->get_vals().size())) {
                raise(mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, s1->get_type()->get_name().c_str(), i2->get_value())));
            }
            if (i2->get_value() < 0) {
                res = lt1->get_vals()[lt1->get_vals().size()+i2->get_value()];
            } else {
                res = lt1->get_vals()[i2->get_value()];
            }
        } else if (s2->get_type() == BuiltIns::Range) {
            opcode::IntConst start;
            opcode::IntConst end;
            opcode::IntConst step;
            extract_range(s2, start, end, step, vm);
            std::vector<Value *> vals;
            for (IntConst i = start; (step < 0 && i > end) || (step >= 0 && i < end); i += step) {
                if (is_oob(lt1, i)) break;
                vals.push_back(list_index(lt1, i));
            }
            res = new ListValue(vals);
        } else {
            raise_operand_exc(vm, "[]", s1, s2);
        }
    }
    else if (auto dt1 = dyn_cast<DictValue>(s1)) {
        auto vals = dt1->get_vals();
        auto dit = vals.find(hash(s2, vm));
        if (dit == vals.end())
            raise(mslib::create_key_error(diags::Diagnostic(*vm->get_src_file(), diags::KEY_NOT_FOUND, s2->as_string().c_str())));
        for (std::pair<Value *, Value *> p: dit->second) {
            if (opcode::eq(p.first, s2, vm)) {
                res = p.second;
                break;
            }
        }
        if (!res) {
            raise(mslib::create_key_error(diags::Diagnostic(*vm->get_src_file(), diags::KEY_NOT_FOUND, s2->as_string().c_str())));
        }
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "[]", s1, s2, dst);
    }
    else {
        raise_operand_exc(vm, "[]", s1, s2);
    }
    return res;
}

void Subsc::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Subsc2::exec(Interpreter *vm) {
    auto res = subsc(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Subsc3::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void SubscLast::exec(Interpreter *vm) {
    auto lstv = vm->load(src1);
    auto indexv = vm->load_const(src2);
    auto res = subsc(lstv, indexv, dst, vm);
    if (res)
        vm->store(dst, res);
    // Check that the src2 index is the last one
    auto lst = dyn_cast<ListValue>(lstv);
    assert(lst && "casting to List was not called when subsclast was generated?");
    auto index = dyn_cast<IntValue>(indexv);
    assert(index && "subsclast was generated with non-int value");
    op_assert(static_cast<IntConst>(lst->get_vals().size()) == index->get_value() + 1, 
        mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::TOO_MANY_VALS_UNPACK, index->get_value())));
}

static void subsc_rest(Interpreter *vm, ListValue *vars, ListValue *vals, IntValue *index) {
    auto iv = index->get_value();
    // Unpacking nothing is valid, so don't count the ...value
    op_assert(vars->size() - (iv >= 0 ? 1 : 0) <= vals->size(), 
        mslib::create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::TOO_FEW_VALS_UNPACK, index->get_value(), vals->size())));

    auto regs = vars->get_vals();
    auto values = vals->get_vals();
    if (iv >= 0) {
        // Copy up to the index of rest
        IntConst i = 0;
        for (; i < iv; ++i) {
            auto r = dyn_cast<IntValue>(regs[i]);
            assert(r && "Non-int value was stored as register in vars");
            vm->store(r->get_value(), values[i]);
        }
        // Copy from the back up to index of rest
        IntConst back_iter = 0;
        for (IntConst j = vars->size()-1; j > iv; --j, ++back_iter) {
            auto r = dyn_cast<IntValue>(regs[j]);
            assert(r && "Non-int value was stored as register in vars");
            vm->store(r->get_value(), values[values.size() - back_iter - 1]);
        }
        // Extract whats left from i to j
        std::vector<Value*> rest_vals(values.begin() + i, values.end() - back_iter);

        auto r = dyn_cast<IntValue>(regs[iv]);
        assert(r && "Non-int value was stored as rest register in vars");
        vm->store(r->get_value(), new ListValue(rest_vals));
    } else {
        for (size_t i = 0; i < regs.size(); ++i) {
            auto r = dyn_cast<IntValue>(regs[i]);
            assert(r && "Non-int value was stored as register in vars");
            vm->store(r->get_value(), values[i]);
        }
    }
}

void SubscRest::exec(Interpreter *vm) {
    auto varsv = vm->load(dst);
    auto valsv = vm->load(src1);
    auto indexv = vm->load_const(src2);

    auto vars = dyn_cast<ListValue>(varsv);
    assert(vars && "vars are not a list");
    auto vals = dyn_cast<ListValue>(valsv);
    assert(valsv && "vals are not a list");
    auto index = dyn_cast<IntValue>(indexv);
    assert(index && "index is not an int");

    subsc_rest(vm, vars, vals, index);
}

void Not::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    Value *res = nullptr;
    if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
        res = new IntValue(~(i1->get_value()));
    }
    else if (BoolValue *b1 = dyn_cast<BoolValue>(s1)) {
        res = new BoolValue(!(b1->get_value()));
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator_unary(vm, "not", s1, dst);
    }
    else {
        raise_operand_exc(vm, "not", s1);
    }
    if (res)
        vm->store(dst, res);
}

void Neg::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    Value *res = nullptr;
    if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
        res = new IntValue(-(i1->get_value()));
    }
    else if (FloatValue *f1 = dyn_cast<FloatValue>(s1)) {
        res = new FloatValue(-(f1->get_value()));
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator_unary(vm, "-", s1, dst);
    }
    else {
        raise_operand_exc(vm, "-", s1);
    }
    if (res)
        vm->store(dst, res);
}

void Assert::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    auto *assert_msg = vm->load(msg);
    assert(s1 && "Nonexistent value");
    assert(assert_msg && "Nonexistent value msg");
    
    auto *check = dyn_cast<BoolValue>(s1);
    op_assert(check, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::BOOL_EXPECTED, s1->get_type()->get_name().c_str())));
    auto str_msg = to_string(vm, assert_msg);

    if (!check->get_value()) {
        raise(mslib::create_assertion_error(str_msg));
    }
}

void Raise::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    assert(s1 && "sanity check");
    raise(s1);
}

static void catch_op(Interpreter *vm, Value *type, ustring name, Address addr) {
    CallFrame *cf = nullptr;
    if (vm->has_call_frame()) {
        cf = vm->get_call_frame();
    }
    auto frm = vm->get_top_frame();
    vm->push_catch(ExceptionCatch(type, name, addr, cf, frm, frm->get_finally_stack_size()));
}

void Catch::exec(Interpreter *vm) {
    catch_op(vm, nullptr, name, addr);
}

void CatchTyped::exec(Interpreter *vm) {
    auto t = vm->load(type);
    assert(t && "Could not load type");
    catch_op(vm, t, name, addr);
}

void PopCatch::exec(Interpreter *vm) {
    vm->pop_catch(amount);
}

void Finally::exec(Interpreter *vm) {
    vm->push_finally(this);
}

void PopFinally::exec(Interpreter *vm) {
    vm->pop_finally();
}

void FinallyReturn::exec(Interpreter *vm) {
    auto addrv = vm->load_const(caller);
    auto addr = dyn_cast<IntValue>(addrv);
    if (addr && addr->get_value() > 0) {
        // Set bci only when addr was set (not nil)
        vm->set_bci(addr->get_value());
    }
}

void ListPush::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "non existent source");
    auto d = vm->load(dst);
    assert(d && "non existent destination");
    auto dlist = dyn_cast<ListValue>(d);
    assert(dlist && "somehow list push is not into list");
    dlist->push(v);
}

void ListPushConst::exec(Interpreter *vm) {
    auto v = vm->load_const(csrc);
    assert(v && "non existent source");
    auto d = vm->load(dst);
    assert(d && "non existent destination");
    auto dlist = dyn_cast<ListValue>(d);
    assert(dlist && "somehow list push is not into list");
    dlist->push(v);
}

void BuildList::exec(Interpreter *vm) {
    vm->store(dst, new ListValue());
}

void BuildDict::exec(Interpreter *vm) {
    auto k = vm->load(keys);
    assert(k && "keys don't exist");
    auto v = vm->load(vals);
    assert(v && "values don't exist");
    auto kl = dyn_cast<ListValue>(k);
    assert(kl && "keys are not a list");
    auto vl = dyn_cast<ListValue>(v);
    assert(vl && "values are not a list");
    vm->store(dst, new DictValue(kl, vl, vm));
}

void BuildEnum::exec(Interpreter *vm) {
    auto lvals = vm->load(vals);
    assert(lvals && "sanity check");
    auto vals_list = dyn_cast<ListValue>(lvals);
    assert(vals_list && "Enum isn't constructed from list");
    
    auto enum_type = new EnumTypeValue(name);
    std::vector<EnumValue *> evals;
    for (auto name: vals_list->get_vals()) {
        evals.push_back(new EnumValue(enum_type, name->as_string()));
    }
    enum_type->set_values(evals);
    vm->store(dst, enum_type);
    vm->store_name(dst, name);
}

void BuildSpace::exec(Interpreter *vm) {
    // This either has to extend a space or create a new one, try loading it
    // first, but only from the top frame, so that a function does not
    // extend a global space.
    auto existing_v = vm->get_top_frame()->load_name(name, vm);
    if (existing_v && isa<SpaceValue>(existing_v)) {
        LOGMAX("BuildSpace detected existing space so pushing its frame");
        // Extending so just push spaces frame and return.
        auto spc_ex = dyn_cast<SpaceValue>(existing_v);
        assert(spc_ex->get_attrs() && "Space does not have frame set");
        vm->push_frame(spc_ex->get_attrs());
    } else {
        auto spc = new SpaceValue(name, vm, anonymous);
        vm->store(dst, spc);
        vm->store_name(dst, name);
        if (anonymous)
            vm->push_spilled_value(spc);
        vm->push_frame();
        spc->set_attrs(vm->get_top_frame());
    }
}

static void range(Value *start, Value *step, Value *end, Register dst, Interpreter *vm) {
    diags::DiagID did = diags::DiagID::UNKNOWN;
    FunValue *constr;
    ClassValue *range_cls = dyn_cast<ClassValue>(BuiltIns::Range);
    assert(range_cls && "Range is not a class type");
    // We cannot call range if step is nil with this value as it won't match the type
    if (isa<NilValue>(step)) {
        constr = opcode::lookup_method(vm, range_cls, range_cls->get_name(), {start, end}, did);
    } else {
        constr = opcode::lookup_method(vm, range_cls, range_cls->get_name(), {start, end, step}, did);
    }
    if (!constr) {
        if (did == diags::DiagID::UNKNOWN) {
            raise(mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, "Range")));
        } else {
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL,
                constr->get_name().c_str(), diags::DIAG_MSGS[did])));
        }
    }
    Value *rval = nullptr;
    rval = runtime_constructor_call(vm, constr, {start, end, step}, range_cls);
    vm->store(dst, rval);
}

void CreateRange::exec(Interpreter *vm) {
    range(vm->load(start), vm->load(step), vm->load(end), dst, vm);
}

void CreateRange2::exec(Interpreter *vm) {
    range(vm->load_const(start), vm->load(step), vm->load(end), dst, vm);
}

void CreateRange3::exec(Interpreter *vm) {
    range(vm->load(start), vm->load_const(step), vm->load(end), dst, vm);
}

void CreateRange4::exec(Interpreter *vm) {
    range(vm->load(start), vm->load(step), vm->load_const(end), dst, vm);
}

void CreateRange5::exec(Interpreter *vm) {
    range(vm->load_const(start), vm->load_const(step), vm->load(end), dst, vm);}

void CreateRange6::exec(Interpreter *vm) {
    range(vm->load_const(start), vm->load(step), vm->load_const(end), dst, vm);
}

void CreateRange7::exec(Interpreter *vm) {
    range(vm->load(start), vm->load_const(step), vm->load_const(end), dst, vm);
}

void CreateRange8::exec(Interpreter *vm) {
    range(vm->load_const(start), vm->load_const(step), vm->load_const(end), dst, vm);
}

void Switch::exec(Interpreter *vm) {
    auto cv = vm->load(this->src);
    assert(cv && "sanity check");
    
    auto v_vals = vm->load(this->vals);
    assert(v_vals && "sanity check");
    auto val_list = dyn_cast<ListValue>(v_vals);
    assert(val_list && "switch value list is not a list");

    auto v_addr = vm->load(this->addrs);
    assert(v_addr && "sanity check");
    auto addr_list = dyn_cast<ListValue>(v_addr);
    assert(addr_list && "switch addr list is not a list");

    auto lvals = val_list->get_vals();
    bool matched = false;
    for (size_t i = 0; i < lvals.size() && !matched; ++i) {
        Value *res = nullptr;
        if (isa<ObjectValue>(lvals[i])) {
            auto objv = dyn_cast<ObjectValue>(lvals[i]);
            auto eq_op = objv->get_attr("==", vm);
            if (eq_op && (isa<FunValue>(eq_op) || isa<FunValueList>(eq_op))) {
                diags::DiagID did = diags::DiagID::UNKNOWN;
                FunValue *funv = lookup_method(vm, lvals[i], "==", {cv, lvals[i]}, did);
                if (!funv) {
                    res = new BoolValue(eq(lvals[i], cv, vm)); 
                } else {
                    res = runtime_method_call(vm, funv, {cv, lvals[i]});
                    assert(res && "runtime call did not return");
                }
            } else {
                res = new BoolValue(eq(lvals[i], cv, vm));
            }
        } else {
            res = new BoolValue(eq(lvals[i], cv, vm));
        }
        assert(res && "sanity check");
        auto resbool = dyn_cast<BoolValue>(res);
        if (!resbool)
            continue;
        if (resbool->get_value()) {
            auto addr_int = dyn_cast<IntValue>(addr_list->get_vals()[i]);
            assert(addr_int && "switch address is not an int");
            vm->set_bci(addr_int->get_value());
            matched = true;
        }
    }
    if (!matched)
        vm->set_bci(default_addr);
}

static void unpack_val(Interpreter *vm, Value *v, Register index, IntValue *unpack) {
    // ths is ignored so lets not create extra value
    Value *err = nullptr;
    auto lv = mslib::List::List(vm, v, err);
    if (err != nullptr) {
        raise(err);
    }
    auto indexv = vm->load(index);
    auto vars = dyn_cast<ListValue>(indexv);
    assert(vars && "multivar index is not list of vars");
    auto vals = dyn_cast<ListValue>(lv);
    assert(vals && "List did not return a ListValue");
    subsc_rest(vm, vars, vals, unpack);
}

static void op_for(Interpreter *vm, Value *coll, Register index, Address addr, bool multi=false, IntValue *unpack=nullptr) {
    try {
        auto v = coll->next(vm);
        assert(v && "sanity check");
        if (!multi) {
            vm->store(index, v);
        } else {
            unpack_val(vm, v, index, unpack);
        }
    } catch (Value *v) {
        if (v->get_type() == BuiltIns::StopIteration) {
            vm->set_bci(addr);
        } else
            throw v;
    }
}

void For::exec(Interpreter *vm) {
    auto coll = vm->load(this->collection);
    assert(coll && "sanity check");
    op_for(vm, coll, index, addr);
}

void ForMulti::exec(Interpreter *vm) {
    auto coll = vm->load(this->collection);
    assert(coll && "sanity check");
    auto unpackv = vm->load_const(this->unpack);
    assert(unpackv && "sanity check");
    auto unpack_i = dyn_cast<IntValue>(unpackv);
    assert(unpack_i && "Unpack index is not int value");
    op_for(vm, coll, vars, addr, true, unpack_i);
}

void Iter::exec(Interpreter *vm) {
    auto coll = vm->load(this->collection);
    assert(coll && "sanity check");
    // For object call __iter because value does not have access to all
    // this data
    if (isa<ObjectValue>(coll)) {
        // Call __iter only if it exists otherwise use this object
        diags::DiagID did = diags::DiagID::UNKNOWN;
        FunValue *iterf = lookup_method(vm, coll, "__iter", {coll}, did);
        if (iterf) {
            LOGMAX("Calling object iterator");
            auto new_iter = runtime_method_call(vm, iterf, {coll});
            if (!isa<ObjectValue>(new_iter))
                new_iter->iter(vm);
            vm->store(iterator, new_iter);
        } else {
            // If it cannot be called, then don't call 
            LOGMAX("No object iterator found, using the object");
            vm->store(iterator, coll);
        }
    } else {
        auto new_iter = coll->iter(vm);
        vm->store(iterator, new_iter);
    }
}

void LoopBegin::exec(Interpreter *vm) {
    vm->push_finally_stack();
}

void LoopEnd::exec(Interpreter *vm) {
    vm->pop_finally_stack();
}

#undef op_assert