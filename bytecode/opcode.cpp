#include "opcode.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include "commons.hpp"
#include "mslib.hpp"
#include "source.hpp"
#include "parser.hpp"
#include "bytecode_reader.hpp"
#include "bytecodegen.hpp"
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
/// \return Value returned by the function
Value *runtime_method_call(Interpreter *vm, FunValue *funV, std::initializer_list<Value *> args) {
    LOGMAX("Doing a runtime call to " << *funV);
    vm->push_call_frame(funV);
    auto cf = vm->get_call_frame();
    cf->set_function(funV);
    int argi = 0;
    for (auto v: args) {
        cf->push_back(v);
        if (argi < funV->get_args().size())
            cf->get_args().back().name = funV->get_args()[argi]->name;
        cf->get_args().back().dst = argi;
        ++argi;
    }
    assert(args.size() != 0 && "Missing this argument");
    cf->get_args().back().name = "this";

    if (funV->get_vm() != vm) {
        LOGMAX("Function detected as external, doing cross module call");
        cf->set_extern_module_call(true);
        funV->get_vm()->cross_module_call(funV, cf);
    }
    else {
        cf->set_runtime_call(true);
        vm->runtime_call(funV);
    }
    auto ret_v = cf->get_extern_return_value();

    LOGMAX("Runtime call finished");
    return ret_v;
}

/// Converts value to string
/// \note This might do a runtime call to __String method
opcode::StringConst opcode::to_string(Interpreter *vm, Value *v) {
    if (auto ov = dyn_cast<ObjectValue>(v)) {
        auto string_attr = ov->get_attr(known_names::TO_STRING_METHOD, vm);
        FunValue *string_fun = nullptr;
        if (string_attr && isa<FunValue>(string_attr))
            string_fun = dyn_cast<FunValue>(string_attr);
        else if (string_attr && isa<FunValueList>(string_attr)) {
            auto string_funs = dyn_cast<FunValueList>(string_attr);
            for (auto f : string_funs->get_funs()) {
                if (f->get_args().size() == 0) {
                    string_fun = f;
                    break;
                }
            }
        }
        if (string_fun) {
            // We need to have a runtime call to the function that executes
            // right now
            auto r_val = runtime_method_call(vm, string_fun, {v});
            assert(r_val && "Runtime call did not return any value");
            return r_val->as_string();
        }
    }
    return v->as_string();
}

void opcode::raise(Value *exc) {
    throw exc;
}

void End::exec(Interpreter *vm) {
    (void) vm;
    // No op
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
    if (!attr && (isa<ObjectValue>(v) || isa<ClassValue>(v))) {
        ClassValue *cls = nullptr;
        if (isa<ObjectValue>(v)) {
            cls = dyn_cast<ClassValue>(v->get_type());
        } else {
            cls = dyn_cast<ClassValue>(v);
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
            v->get_name().c_str(), this->name.c_str())));
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
    op_assert(dstobj->is_mutable(), mslib::create_attribute_error(
        diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_CREATE_ATTR,
            dstobj->get_name().c_str())));
    auto *v = vm->load(this->src);
    assert(v && "non existent register");
    dstobj->set_attr(this->name, v);
}

void StoreConstAttr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->store_const(dst, new IntValue(val));
}

void StoreFloatConst::exec(Interpreter *vm) {
    vm->store_const(dst, new FloatValue(val));
}

void StoreBoolConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new BoolValue(val));
}

void StoreStringConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new StringValue(val));
}

void StoreNilConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new NilValue());
}

void Jmp::exec(Interpreter *vm) {
    assert(state == JMPState::SET && "Break/continue jmp was not setup or break/continue is outside of loop");
    vm->set_bci(this->addr);
}

void JmpIfTrue::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    op_assert(bc, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::BOOL_EXPECTED, v->get_name().c_str())));

    if (bc->get_value())
        vm->set_bci(this->addr);
}

void JmpIfFalse::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    op_assert(bc, mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::BOOL_EXPECTED, v->get_name().c_str())));

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
            call_args.assign(og_call_args.begin(), og_call_args.end()-1);
            ths = &(og_call_args.back());
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
    bool super_call = false;
    // Class constructor call
    if (isa<ClassValue>(funV) || isa<SuperValue>(funV)) {
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
        if (!cls) {
            // super() call, so extract the class based on the mro
            auto spr = dyn_cast<SuperValue>(funV);
            assert(spr && "some other value allowed?");
            auto inst_type = spr->get_instance()->get_type();
            auto inst_cls = dyn_cast<ClassValue>(inst_type);
            assert(inst_cls && "Object type is not a class");
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
                vm->pop_call_frame();
                return;
            }
            super_call = true;
            LOGMAX("Super call to: " << *cls);
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
                vm->pop_call_frame();
                return;
            }
        }
    }
    else if (!cf->get_args().empty() && cf->get_args().back().name == "this" && !has_methods(cf->get_args().back().value)) {
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
            vm->pop_call_frame();
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, 
                fvl->back()->get_name().c_str(), diags::DIAG_MSGS[*err_id])));
        }
    }
    else {
        std::optional<diags::DiagID> err_id = can_call(fun, cf);
        if (err_id) {
            // Pop frame so that call stack is correct
            vm->pop_call_frame();
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL,
                fun->get_name().c_str(), diags::DIAG_MSGS[*err_id])));
        }
    }

    cf->set_function(fun);

    // Set this object if constructor is being called
    if (constructor_of) {
        auto obj = new ObjectValue(constructor_of);
        cf->get_args().push_back(CallFrameArg("this", obj, cf->get_args().size()));
        cf->set_constructor_call(true);
        LOGMAX("Constructor detected, creating object and passing in: " << *obj);
        LOGMAX(*cf);
    }

    if (fun->has_annotation(annots::INTERNAL)) {
        Value *err = nullptr;
        mslib::dispatch(vm, fun->get_name(), err);
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
    if (cf->is_extern_module_call() || cf->is_runtime_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        vm->drop_call_frame();
        vm->pop_frame();
    } else {
        vm->pop_frame();
        vm->store(return_reg, ret_v);
        vm->set_bci(caller_addr);
        vm->pop_call_frame();
    }
}

void ReturnConst::exec(Interpreter *vm) {
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
    if (cf->is_extern_module_call() || cf->is_runtime_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        vm->drop_call_frame();
        vm->pop_frame();
    } else {
        vm->pop_frame();
        vm->store(return_reg, ret_v);
        vm->set_bci(caller_addr);
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

void CreateFun::exec(Interpreter *vm) {
    FunValue *funval = new FunValue(name, arg_names, vm);
    for (auto riter = vm->get_frames().rbegin(); riter != vm->get_frames().rend(); ++riter) {
        // Push all latest local frames as closures of current function
        if ((*riter)->is_global())
            break;
        funval->push_closure(*riter);
    }
    auto f = vm->get_top_frame()->load_name(name, vm);
    if (f && (isa<FunValue>(f) || isa<FunValueList>(f))) {
        if (auto fv = dyn_cast<FunValue>(f)) {
            vm->store(this->fun, new FunValueList(std::vector<FunValue *>{fv, funval}));
            vm->store_name(this->fun, name);
        }
        else {
            vm->store(this->fun, funval);
            auto fvl = dyn_cast<FunValueList>(f);
            fvl->push_back(funval);
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

        op_assert(tv && isa<ClassValue>(tv), mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_A_TYPE, tv->as_string().c_str())));
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
        LOGMAX("Read bytecode: \n" << *bc);
    } else {
        auto module_file = new SourceFile(path, SourceFile::SourceType::FILE);
        input_file = module_file;
        Parser parser(*module_file);
        auto module_ir = parser.parse(false);
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

void PushParent::exec(Interpreter *vm) {
    auto v = vm->load(parent);
    assert(v && "Non existent class");
    auto cv = dyn_cast<ClassValue>(v);
    assert(cv && "Pushed parent is not a class");
    vm->push_parent(cv);
}

void CreateObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PromoteObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildClass::exec(Interpreter *vm) {
    auto cls = new ClassValue(name, vm->get_parent_list());
    vm->store(dst, cls);
    vm->store_name(dst, name);
    vm->clear_parent_list();
    vm->push_frame();
    cls->set_attrs(vm->get_top_frame());
}

void Copy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void DeepCopy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Annotate::exec(Interpreter *vm) {
    auto *d = vm->load(dst);
    assert(d && "Cannot load dst");
    auto *v = vm->load(val);
    assert(v && "Cannot load val");
    if (auto fl = dyn_cast<FunValueList>(d)) {
        // Annotate is called right after CreateFun so it has the be the top
        // one in the list
        fl->back()->annotate(name, v);
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
    }
}

void Output::exec(Interpreter *vm) {
    // FIXME: this is just a placeholder
    auto *v = vm->load(src);
    assert(v && "Cannot load src");

    std::cout << to_string(vm, v);
}

static Value *concat(Value *s1, Value *s2, Register src1, Register src2, Interpreter *vm) {
    (void)vm;
    assert(s1 && "Value or nil should have been loaded");
    assert(s2 && "Value or nil should have been loaded");

    ustring s1_str = to_string(vm, s1);
    ustring s2_str = to_string(vm, s2);

    return new StringValue(s1_str + s2_str);
}

void Concat::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load(src2), src1, src2, vm);
    vm->store(dst, res);
}

void Concat2::exec(Interpreter *vm) {
    auto res = concat(vm->load_const(src1), vm->load(src2), src1, src2, vm);
    vm->store(dst, res);
}

void Concat3::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load_const(src2), src1, src2, vm);
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

static Value *eq(Value *s1, Value *s2, Register dst, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (s1->get_kind() == s2->get_kind()) {
        if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
            IntValue *i2 = dyn_cast<IntValue>(s2);
            res = new BoolValue(i1->get_value() == i2->get_value());
        }
        else if (FloatValue *f1 = dyn_cast<FloatValue>(s1)) {
            FloatValue *f2 = dyn_cast<FloatValue>(s2);
            res = new BoolValue(f1->get_value() == f2->get_value());
        }
        else if (BoolValue *b1 = dyn_cast<BoolValue>(s1)) {
            BoolValue *b2 = dyn_cast<BoolValue>(s2);
            res = new BoolValue(b1->get_value() == b2->get_value());
        }
        else if (StringValue *st1 = dyn_cast<StringValue>(s1)) {
            StringValue *st2 = dyn_cast<StringValue>(s2);
            res = new BoolValue(st1->get_value() == st2->get_value());
        }
        else if (isa<NilValue>(s1)) {
            res = new BoolValue(true);
        }
        else if (EnumValue *ev1 = dyn_cast<EnumValue>(s1)) {
            EnumValue *ev2 = dyn_cast<EnumValue>(s2);
            res = new BoolValue(ev1 == ev2);
        }
        else {
            call_operator(vm, "==", s1, s2, dst);
        }
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() == s2->as_float());
    }
    else if (isa<NilValue>(s1) || isa<NilValue>(s1)) {
        res = new BoolValue(false);
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "==", s1, s2, dst);
    } else {
        res = new BoolValue(false);
    }
    return res;
}

void Eq::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Eq2::exec(Interpreter *vm) {
    auto res = eq(vm->load_const(src1), vm->load(src2), dst, vm);
    if (res)
        vm->store(dst, res);
}

void Eq3::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load_const(src2), dst, vm);
    if (res)
        vm->store(dst, res);
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
        auto eqRes = eq(s1, s2, dst, vm);
        auto neqRes = new BoolValue(!dyn_cast<BoolValue>(eqRes)->get_value());
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
              (i2->get_value() > 0 && static_cast<unsigned long>(i2->get_value()) >= lt1->get_vals().size())) {
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

static Value *slice(Value *s1, Value *s2, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    assert(false && "TODO: Unimplemented opcode");
    return res;
}

void Slice::exec(Interpreter *vm) {
    auto res = slice(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Slice2::exec(Interpreter *vm) {
    auto res = slice(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
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

void CopyArgs::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
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
    vm->push_catch(ExceptionCatch(type, name, addr, cf, vm->get_top_frame()));
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
    vm->pop_catch();
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
    assert(false && "TODO: Unimplemented opcode");
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
    auto spc = new SpaceValue(name, vm, anonymous);
    vm->store(dst, spc);
    vm->store_name(dst, name);
    if (anonymous)
        vm->push_spilled_value(spc);
    vm->push_frame();
    spc->set_attrs(vm->get_top_frame());
}

static void range(Value *start, Value *step, Value *end, Register dst, Interpreter *vm) {
    // We cannot call range if step is nil with this value as it won't match the type
    if (isa<NilValue>(step))
        call(vm, dst, BuiltIns::Range, {start, end});
    else
        call(vm, dst, BuiltIns::Range, {start, end, step});
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
                FunValue *funv = dyn_cast<FunValue>(eq_op);
                if (!funv) {
                    auto eqlist = dyn_cast<FunValueList>(eq_op);
                    for (auto f : eqlist->get_funs()) {
                        if (f->get_args().size() == 1) {
                            if (f->get_args()[0]->types.size() == 0) {
                                funv = f;
                                break;
                            } else {
                                for (auto t: f->get_args()[0]->types) {
                                    if (t == cv->get_type()) {
                                        funv = f;
                                        break;
                                    }
                                }
                                if (funv)
                                    break;
                            }
                        }
                    }
                }
                res = runtime_method_call(vm, funv, {cv, lvals[i]});
                assert(res && "runtime call did not return");
            } else {
                res = eq(lvals[i], cv, 0, vm);
            }
        } else {
            res = eq(lvals[i], cv, 0, vm);
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

void For::exec(Interpreter *vm) {
    auto coll = vm->load(this->collection);
    assert(coll && "sanity check");
    if (isa<ObjectValue>(coll)) {
        auto nextv = coll->get_attr("__next", vm);
        if (nextv) {
            FunValue *nextf = dyn_cast<FunValue>(nextv);
            if (!nextf && isa<FunValueList>(nextv)) {
                auto nextlist = dyn_cast<FunValueList>(nextv);
                for (auto f : nextlist->get_funs()) {
                    if (f->get_args().size() == 0) {
                        nextf = f;
                        break;
                    }
                }
            }
            // If it cannot be called, then don't call it
            if (nextf) {
                try {
                    auto v = runtime_method_call(vm, nextf, {coll});
                    assert(v && "sanity check");
                    vm->store(this->index, v);
                } catch (Value *v) {
                    if (v->get_type() == BuiltIns::StopIteration)
                        vm->set_bci(this->addr);
                    else
                        throw v;
                }
            } else {
                raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_NEXT_DEFINED, coll->get_type()->get_name().c_str())));
            }
        } else {
            raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_NEXT_DEFINED, coll->get_type()->get_name().c_str())));
        }
    } else {
        try {
            auto v = coll->next(vm);
            assert(v && "sanity check");
            vm->store(this->index, v);
        } catch (Value *v) {
            if (v->get_type() == BuiltIns::StopIteration)
                vm->set_bci(this->addr);
            else
                throw v;
        }
    }
}

void Iter::exec(Interpreter *vm) {
    auto coll = vm->load(this->collection);
    assert(coll && "sanity check");
    // For object call __iter because value does not have access to all
    // this data
    if (isa<ObjectValue>(coll)) {
        // Call __iter only if it exists otherwise use this object
        auto iterv = coll->get_attr("__iter", vm);
        if (iterv) {
            FunValue *iterf = dyn_cast<FunValue>(iterv);
            if (!iterf && isa<FunValueList>(iterv)) {
                auto iterlist = dyn_cast<FunValueList>(iterv);
                for (auto f : iterlist->get_funs()) {
                    if (f->get_args().size() == 0) {
                        iterf = f;
                        break;
                    }
                }
            }
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
            LOGMAX("No object iterator found, using the object");
            vm->store(iterator, coll);
        }
    } else {
        auto new_iter = coll->iter(vm);
        vm->store(iterator, new_iter);
    }
}

#undef op_assert