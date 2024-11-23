#include "mslib.hpp"
#include "values.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include <functional>

using namespace moss;
using namespace mslib;

static inline void set_return(Interpreter *vm, Value *v) {
    vm->store(vm->get_call_frame()->get_return_reg(), v);
}

static inline void set_return_nil(Interpreter *vm) {
    set_return(vm, BuiltIns::Nil);
}

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
    std::stringstream ss;
    ss << *v << "\n";
    return new StringValue(ss.str());
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

void mslib::init(MemoryPool *gf, opcode::Register &reg_counter) {
    // All functions MUST be annotated "@internal"

    auto create_fun = [gf, &reg_counter](ustring name, ustring arg_names) {
        auto f = new FunValue(name, arg_names);
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
}