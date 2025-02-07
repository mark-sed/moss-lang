#include "mslib.hpp"
#include "values.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include <functional>
#include <cstdlib>

using namespace moss;
using namespace mslib;

void mslib::exit(Interpreter *vm, Value *code) {
    if (auto i = dyn_cast<IntValue>(code)) {
        vm->set_exit_code(i->get_value());
    }
    else {
        // Print the passed in value and exit with 1
        vm->set_exit_code(1);
        errs << code->as_string() << "\n";
    }
    // Jump to end of file bc
    vm->set_bci(vm->get_code_size()-1);
    global_controls::exit_called = true;
    vm->set_stop(true);
    // No need to set any return as we jump to the end
}

Value *mslib::vardump(Interpreter *vm, Value *v) {
    (void)vm;
    std::stringstream ss;
    ss << *v << "\n";
    return new StringValue(ss.str());
}

Value *mslib::print(Interpreter *vm, Value *msgs, Value *end, Value *separator) {
    (void)vm;
    auto msgs_list = dyn_cast<ListValue>(msgs);
    assert(msgs_list && "msgs is not a vararg?");
    bool first = true;
    for (auto v : msgs_list->get_vals()) {
        if (first) {
            outs << v->as_string();
            first = false;
        }
        else {
            outs << separator->as_string() << v->as_string();
        }
    }
    outs << end->as_string();
    return BuiltIns::Nil;
}

Value *mslib::Int(Interpreter *vm, Value * ths, Value *v, Value *base) {
    (void)vm;
    (void)ths;
    IntValue *base_int = nullptr;
    if (base)
        base_int = dyn_cast<IntValue>(base);

    if (isa<IntValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        assert(base_int && "TODO: Raise type exception as base is not int");
        char *pend;
        errno = 0;
        auto vi = std::strtol(sv->as_string().c_str(), &pend, base_int->get_value());
        if (*pend != '\0')
            assert(false && "TODO: Raise error value is not int");
        if (errno != 0) {
            LOGMAX("Errno error: " << strerror(errno));
            assert(false && "TODO: Raise conversion error");
        }
        return new IntValue(vi);
    }
    if (auto fv = dyn_cast<FloatValue>(v)) {
        return new IntValue(static_cast<opcode::IntConst>(fv->get_value()));
    }
    
    assert(false && "Incorrect arg type");
}

Value *mslib::Exception(Interpreter *vm, Value *ths, Value *msg) {
    (void)vm;
    ths->set_attr("msg", msg);
    return ths;
}

Value *mslib::create_exception(Value *type, ustring msg) {
    auto clt = dyn_cast<ClassValue>(type);
    assert(clt && "Passed non class type value");
    auto err = new ObjectValue(clt);
    err->set_attr("msg", new StringValue(msg));
    return err;
}

Value *mslib::create_exception(Value *type, diags::Diagnostic dmsg) {
    return create_exception(type, error::format_error(dmsg));
}

void mslib::dispatch(Interpreter *vm, ustring name, Value *&err) {
    auto cf = vm->get_call_frame();
    auto args = cf->get_args();
    auto arg_size = args.size();

    Value *ret_v = nullptr;

    // Matching function name to execute internal implementation
    if (name == "exit") {
        assert(arg_size == 1 && "Mismatch of args");
        exit(vm, args[0].value);
        // We must return here as otherwise the bci which exit set would be
        // overriden by the caller bci
        return;
    }
    else if (name == "vardump") {
        assert(arg_size == 1 && "Mismatch of args");
        ret_v = vardump(vm, args[0].value);
    }
    else if (name == "print") {
        assert(arg_size == 3 && "Mismatch of args");
        ret_v = print(vm, cf->get_arg("msgs"), cf->get_arg("end"), cf->get_arg("separator"));
    }
    else if (name == "Int") {
        assert((arg_size == 3 || arg_size == 2) && "Mismatch of args");
        // Base might not be set as this is only for string argument
        ret_v = Int(vm, cf->get_arg("this"), cf->get_arg("v"), cf->get_arg("base", true));
    }
    else if (name == "Exception") {
        assert(arg_size == 2 && "Mismatch of args");
        ret_v = Exception(vm, cf->get_arg("this"), cf->get_arg("msg"));
    }
    else {
        err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, name.c_str()));
    }

    auto return_reg = vm->get_call_frame()->get_return_reg();
    auto caller_addr = vm->get_call_frame()->get_caller_addr();
    vm->pop_call_frame();
    if (!ret_v)
        ret_v = BuiltIns::Nil;
    vm->store(return_reg, ret_v);
    vm->set_bci(caller_addr);
}

/*void mslib::init(MemoryPool *gf, opcode::Register &reg_counter) {
    // Note: Also add function to matching above
    auto create_fun = [gf, &reg_counter](ustring name, ustring arg_names) {
        auto f = new FunValue(name, arg_names);
        // All functions MUST be annotated "@internal"
        f->annotate(annots::INTERNAL, BuiltIns::Nil);
        gf->store(reg_counter, f);
        gf->store_name(reg_counter, name);
        ++reg_counter;
        return f;
    };

    // print(... msgs, end="\n", separator=" ")
    auto f_print = create_fun("print", "msgs,end,separator");
    f_print->set_vararg(0);
    f_print->set_default(1, new StringValue("\n"));
    f_print->set_default(2, new StringValue(" "));
}*/