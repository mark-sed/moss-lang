#include "mslib.hpp"
#include "values.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include <functional>

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
}

void mslib::dispatch(Interpreter *vm, ustring name, Value *&err) {
    auto args = vm->get_call_frame()->get_args();
    // TODO: Generalize the argument extraction and type checking
    if (name == "exit") {
        assert(args.size() == 1 && "Mismatch of args");
        exit(vm, args[0].value);
    }
    else {
        auto msg = error::format_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, name.c_str()));
        // TODO: Change to exception
        err = new StringValue(msg);
    }
}

static inline void store_glob_val(opcode::Register reg, ustring name, Value *v, MemoryPool *gf) {
    gf->store(reg, v);
    gf->store_name(reg, name);
}

static FunValue *create_fun(ustring name, ustring arg_names) {
    auto f = new FunValue(name, arg_names);
    f->annotate(annots::INTERNAL, BuiltIns::Nil);
    return f;
}

void mslib::init(MemoryPool *gf, opcode::Register &reg_counter) {
    // All functions MUST be annotated "@internal"
    auto f = create_fun("exit", "code");
    
    // exit(code=0)
    store_glob_val(reg_counter++, "exit", f, gf);
    f->set_default(0, BuiltIns::IntConstants[0]);
}