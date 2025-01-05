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
    auto base_int = dyn_cast<IntValue>(base);
    assert(base_int && "TODO: Raise type exception as base is not int");

    if (isa<IntValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        char *pend;
        auto vi = std::strtol(sv->as_string().c_str(), &pend, base_int->get_value());
        if (*pend != '\0')
            assert(false && "TODO: Raise error value is not int");
        if (errno != 0)
            assert(false && "TODO: Raise conversion error");
        return new IntValue(vi);
    }
    if (auto fv = dyn_cast<FloatValue>(v)) {
        return new IntValue(static_cast<opcode::IntConst>(fv->get_value()));
    }
    
    assert(false && "Incorrect arg type");
}

void mslib::dispatch(Interpreter *vm, ustring name, Value *&err) {
    auto args = vm->get_call_frame()->get_args();
    // TODO: Generalize the argument extraction and type checking
    Value *ret_v = nullptr;
    if (name == "exit") {
        assert(args.size() == 1 && "Mismatch of args");
        exit(vm, args[0].value);
        // We must return here as otherwise the bci which exit set would be
        // overriden by the caller bci
        return;
    }
    else if (name == "vardump") {
        assert(args.size() == 1 && "Mismatch of args");
        ret_v = vardump(vm, args[0].value);
    }
    else if (name == "print") {
        assert(args.size() == 3 && "Mismatch of args");
        Value *msgs = nullptr;
        Value *separator = nullptr;
        Value *end = nullptr;
        for (auto a: args) {
            if (a.name == "msgs")
                msgs = a.value;
            else if (a.name == "end")
                end = a.value;
            else if (a.name == "separator")
                separator = a.value;
        }
        assert(msgs);
        assert(separator);
        assert(end);
        ret_v = print(vm, msgs, end, separator);
    }
    else if (name == "Int") {
        assert(args.size() == 3 && "Mismatch of args");
        Value *ths = nullptr;
        Value *v = nullptr;
        Value *base = nullptr;
        for (auto a : args) {
            if (a.name == "this")
                ths = a.value;
            else if (a.name == "v")
                v = a.value;
            else if (a.name == "base")
                base = a.value;
        }
        assert(ths);
        assert(v);
        assert(base);
        ret_v = Int(vm, ths, v, base);
    }
    else {
        auto msg = error::format_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, name.c_str()));
        // TODO: Change to exception
        err = new StringValue(msg);
    }

    auto return_reg = vm->get_call_frame()->get_return_reg();
    auto caller_addr = vm->get_call_frame()->get_caller_addr();
    vm->pop_call_frame();
    if (!ret_v)
        ret_v = BuiltIns::Nil;
    vm->store(return_reg, ret_v);
    vm->set_bci(caller_addr);
}

static void init_builtins(MemoryPool *gf) {
    auto create_method = [gf](ustring name, ustring arg_names) {
        auto f = new FunValue(name, arg_names);
        // All functions MUST be annotated "@internal"
        f->annotate(annots::INTERNAL, BuiltIns::Nil);
        return f;
    };

    auto int_constr = create_method("Int", "v,base");
    int_constr->set_default(1, BuiltIns::IntConstants[10]);
    // TODO: Use set_type
    BuiltIns::Int->set_attr("Int", int_constr);
}

void mslib::init(MemoryPool *gf, opcode::Register &reg_counter) {
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
    
    // exit(code=0)
    auto f_exit = create_fun("exit", "code");
    f_exit->set_default(0, BuiltIns::IntConstants[0]);

    // vardump(value)
    create_fun("vardump", "value");

    // print(... msgs, end="\n", separator=" ")
    auto f_print = create_fun("print", "msgs,end,separator");
    f_print->set_vararg(0);
    f_print->set_default(1, new StringValue("\n"));
    f_print->set_default(2, new StringValue(" "));

    init_builtins(gf);
}