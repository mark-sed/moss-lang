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

/// @return true if t1 is same as t2 or t1 is subtype of t2 (child class)
static bool is_type_eq_or_subtype(Value *t1, Value *t2) {
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

void opcode::raise(Interpreter *vm, Value *exc) {
    (void) vm;
    errs << exc->as_string();
}

void End::exec(Interpreter *vm) {
    (void) vm;
    // No op
}

void Load::exec(Interpreter *vm) {
    auto v = vm->load_name(this->name);
    if (!v) {
        // The refered value could possibly be method or attribute in the
        // class or object. Look if "this" is set
        auto ths = vm->load_name("this");
        if (ths) {
            v = ths->get_attr(this->name);
        }
    }
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    vm->store(this->dst, v);
}

void LoadAttr::exec(Interpreter *vm) {
    auto *v = vm->load(this->src);
    auto attr = v->get_attr(this->name);
    // This could possibly be an object or a class
    if (!attr && (isa<ObjectValue>(v) || isa<ClassValue>(v))) {
        ClassValue *cls = nullptr;
        if (isa<ObjectValue>(v)) {
            cls = dyn_cast<ClassValue>(v->get_type());
        } else {
            cls = dyn_cast<ClassValue>(v);
        }

        for (auto sup: cls->get_all_supers()) {
            attr = sup->get_attr(this->name);
            if (attr)
                break;
        }
    } else if (isa<ModuleValue>(v) && (isa<FunValue>(attr) || isa<FunValueList>(attr))) {
        // If we're loading an attribute from a module, then set the value's
        // owner to that module so it can be correctly called, this is needed
        // only for functions and lambdas
        attr->set_external_module_owner(v);
    }
    assert(attr && "TODO: Nonexistent attr raise exception");
    vm->store(this->dst, attr);
}

void LoadGlobal::exec(Interpreter *vm) {
    auto *v = vm->load_global_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    vm->store(this->dst, v);
}

void LoadNonLoc::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
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
    auto *v = vm->load(this->src);
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
    vm->set_bci(this->addr);
}

void JmpIfTrue::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

    if (bc->get_value())
        vm->set_bci(this->addr);
}

void JmpIfFalse::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

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
static bool can_call(FunValue *f, CallFrame *cf) {
    LOGMAX("Checking if can call " << *f);
    auto &og_call_args = cf->get_args();
    const auto fun_args = f->get_args();

    for (auto a : og_call_args) {
        LOGMAX(a);
    }

    std::vector<CallFrameArg> call_args;
    CallFrameArg *ths = nullptr;
    if (og_call_args.empty() || og_call_args.back().name != "this")
        call_args.assign(og_call_args.begin(), og_call_args.end());
    else {
        // When class method is called then this is set to the class
        if (dyn_cast<ClassValue>(og_call_args.back().value)) {
            // In such case we use the last arg as this
            if (!og_call_args[og_call_args.size()-2].name.empty()) {
                LOGMAX("Cannot call, no this argument provided");
                return false;
            }
            ths = &(og_call_args[og_call_args.size()-2]);
            if (!ths->name.empty()) {
                LOGMAX("Cannot call, last argument has set name");
                return false;
            }
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
            return false;
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
                return false;
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
                    return false;
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
                    return false;
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
        return true;
    }
    return false;
}

void call(Interpreter *vm, Register dst, Value *funV) {
    auto cf = vm->get_call_frame();
    cf->set_return_reg(dst);
    cf->set_caller_addr(vm->get_bci()+1);

    assert(funV && "TODO: Raise function not found");

    ClassValue *constructor_of = nullptr;
    Value *called_fun_owner = nullptr;
    // Class constructor call
    if (auto cls = dyn_cast<ClassValue>(funV)) {
        constructor_of = cls;
        if (cls->get_attrs())
            funV = cls->get_attrs()->load_name(cls->get_name());
        if (!funV) {
            // No constructor provided, look for one in parent classes
            for (auto parent : cls->get_all_supers()) {
                funV = parent->get_attrs()->load_name(parent->get_name());
                if (funV)
                    break;
            }
            // No constructor is provided so execute implicit one
            // by setting this and returning
            if (!funV) {
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
        called_fun_owner = cf->get_args().back().value;
        cf->get_args().pop_back();
    }
    FunValue *fun = dyn_cast<FunValue>(funV);
    if (!fun) {
        auto fvl = dyn_cast<FunValueList>(funV);
        if (!fvl) {
            assert(false && "TODO: Raise value not callable");
        }
        // Walk functions and check if it can be called
        for (auto f: fvl->get_funs()) {
            if (can_call(f, cf)) {
                fun = f;
                break;
            }
        }
        if (!fun) {
            assert(false && "TODO: Raise incorrect function call");
        }
    }
    else {
        if (!can_call(fun, cf)) {
            assert(false && "TODO: Raise incorrect function call");
        }
    }

    // Lets try to get owner if this is a dot imported function
    if (!called_fun_owner)
        called_fun_owner = fun->get_external_module_owner();
    // We can call the function, but it might be external module/space call
    // So lets load it again and extract owner
    if (!called_fun_owner)
        vm->load_name(fun->get_name(), &called_fun_owner);
#ifndef NDEBUG
    if (called_fun_owner) {
        LOGMAX("Function detected to be from other module");
    }
#endif

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
            raise(vm, err);
            return;
        }
    }
    else if (called_fun_owner && isa<ModuleValue>(called_fun_owner)) {
        LOGMAX("Setting up module for its function call");
        auto mod = dyn_cast<ModuleValue>(called_fun_owner);
        // This calls the function
        LOGMAX("Calling different module's function");
        cf->set_extern_module_call(true);
        mod->get_vm()->cross_module_call(fun, vm->get_call_frame());
        LOGMAX("External function has handed over control to original module");
        // This is after return from the function
        vm->store(cf->get_return_reg(), cf->get_extern_return_value());
        vm->set_bci(cf->get_caller_addr());
        // Remove already pushed in call frame
        vm->pop_call_frame();
        if (mod->get_vm()->get_exit_code() != 0) {
            LOGMAX("Delagating exit code from external module");
            vm->set_exit_code(mod->get_vm()->get_exit_code());
        }
    }
    else {
        vm->push_frame(fun);
        vm->set_bci(fun->get_body_addr());
    }
}

void call(Interpreter *vm, Register dst, Value *funV, std::initializer_list<Value *> args) {
    vm->push_call_frame();
    for (auto v: args) {
        vm->get_call_frame()->push_back(v);
    }
    call(vm, dst, funV);
}

void call_method(Interpreter *vm, Register dst, Value *funV, std::initializer_list<Value *> args) {
    vm->push_call_frame();
    for (auto v: args) {
        vm->get_call_frame()->push_back(v);
    }
    assert(args.size() != 0 && "Missing this argument");
    vm->get_call_frame()->get_args().back().name = "this";
    call(vm, dst, funV);
}

void call_operator(Interpreter *vm, ustring op, Value *s1, Value *s2, Register dst) {
    auto v = s1->get_attr(op);
    assert(v && "TODO: Operator function not found for object error");
    call_method(vm, dst, v, {s2, s1});
}

void call_operator_unary(Interpreter *vm, ustring op, Value *s1, Register dst) {
    auto v = s1->get_attr(op);
    assert(v && "TODO: Operator function not found for object error");
    call_method(vm, dst, v, {s1});
}

void Call::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "TODO: Raise error function not found");
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
    if (cf->is_extern_module_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        vm->drop_call_frame();
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
    if (cf->is_extern_module_call()) {
        // We need to propagete the return value back using the CallFrame
        // which is used by both VMs. We also need to stop the current
        // vm and this will return back to the call.
        cf->set_extern_return_value(ret_v);
        vm->set_stop(true);
        // pop_call_frame deletes the frame, so we cannot call it here
        // the original owner will delete it.
        vm->drop_call_frame();
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
    FunValue *funval = new FunValue(name, arg_names);
    for (auto riter = vm->get_frames().rbegin(); riter != vm->get_frames().rend(); ++riter) {
        // Push all latest local frames as closures of current function
        if ((*riter)->is_global())
            break;
        funval->push_closure(*riter);
    }
    auto f = vm->load_name(name);
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
    auto type = vm->load_type(name);
    assert(type && "TODO: Raise unknown type exception for type");
    fv->set_type(index, type);
}

void SetVararg::exec(Interpreter *vm) {
    auto fv = load_last_fun(fun, vm);
    fv->set_vararg(index);
}

static ModuleValue *load_module(Interpreter *vm, ustring name) {
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
        LOGMAX("Read bytecode: \n" << *bc);
    } else {
        auto module_file = new SourceFile(path, SourceFile::SourceType::FILE);
        input_file = module_file;
        Parser parser(*module_file);
        auto module_ir = parser.parse(false);

        bc = new Bytecode();
        bcgen::BytecodeGen cgen(bc);
        cgen.generate(module_ir);
        LOGMAX("Generated bytecode: \n" << *bc);
        delete module_ir;
    }

    auto mod_i = new Interpreter(bc, input_file);
    mod_i->run();
    if (mod_i->get_exit_code() != 0) {
        LOGMAX("Import exited, delegating exit code");
        vm->set_exit_code(mod_i->get_exit_code());
    }
    return new ModuleValue(name, mod_i->get_global_frame(), mod_i);
}

void Import::exec(Interpreter *vm) {
    auto mod = load_module(vm, name);
    assert(mod && "TODO: Raise module not found error");
    vm->store(dst, mod);
}

void ImportAll::exec(Interpreter *vm) {
    auto mod = vm->load(src);
    assert(mod && "Non existent module for import all");
    // TODO: Uncomment
    //assert(isa<ModuleValue>(mod) || isa<SpaceValue>(mod) && "Spilling something else than module or space");
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
    d->annotate(name, v);
}

void Output::exec(Interpreter *vm) {
    // FIXME: this is just a placeholder
    auto *v = vm->load(src);
    assert(v && "Cannot load src");

    std::cout << v->as_string();
}

static Value *concat(Value *s1, Value *s2, Register src1, Register src2, Interpreter *vm) {
    (void)vm;
    assert(s1 && "Value or nil should have been loaded");
    assert(s2 && "Value or nil should have been loaded");

    // TODO: Check if s1 is object and if it has ++ method to be called
    // if so then call it and return it `as_string` if its not
    // if it does not then do as for all other

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

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

// TODO: For most exprs is to check over/underflow when check is enabled
// TODO: Call user defined method for non native type
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        if (i2->get_value() == 0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0 exception raise");
        }
        res = new IntValue(i1->get_value() / i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        if (s2->as_float() == 0.0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0.0 exception raise");
        }
        res = new FloatValue(s1->as_float() / s2->as_float());
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "/", s1, s2, dst);
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        if (i2->get_value() == 0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0 exception raise");
        }
        res = new IntValue(i1->get_value() % i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        if (s2->as_float() == 0.0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0.0 exception raise");
        }
        res = new FloatValue(std::fmod(s1->as_float(), s2->as_float()));
    }
    else if (isa<ObjectValue>(s1)) {
        call_operator(vm, "%", s1, s2, dst);
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
    }
    // TODO compare class values and enum values and such
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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

static Value *subsc(Value *s1, Value *s2, Interpreter *vm) {
    (void) vm;
    Value *res = nullptr;
    if (isa<StringValue>(s1) && isa<IntValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        if (i2->get_value() < 0) {
            assert(false && "TODO: negative index");
        }
        else if (static_cast<unsigned long>(i2->get_value()) >= st1->get_value().size()) {
            assert(false && "TODO: out of bounds exception");
        }
        res = new StringValue(ustring(1, st1->get_value()[i2->get_value()]));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Subsc::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Subsc2::exec(Interpreter *vm) {
    auto res = subsc(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Subsc3::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load_const(src2), vm);
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
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
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    if (res)
        vm->store(dst, res);
}

void Assert::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    auto *assert_msg = vm->load(msg);
    
    auto *check = dyn_cast<BoolValue>(s1);
    assert(check && "Assertion value is not a boolean");
    auto *str_msg = dyn_cast<StringValue>(assert_msg);
    assert(str_msg && "Assertion message is not a string");

    if (!check->get_value()) {
        assert(false && "TODO: Raise assertion exception");
    }
}

void CopyArgs::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Raise::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    assert(s1 && "sanity check");
    raise(vm, s1);
}

void CheckCatch::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
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

static Value *range(Value *start, Value *step, Value *end, Interpreter *vm) {
    Value *res = nullptr;
    assert(false && "TODO: Unimplemented range generation");
    return res;
}

void CreateRange::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange2::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange3::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load_const(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange4::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange5::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load_const(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange6::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange7::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load_const(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange8::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load_const(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void Switch::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void For::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}
