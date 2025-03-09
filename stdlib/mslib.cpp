#include "mslib.hpp"
#include "values.hpp"
#include "values_cpp.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include <functional>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <random>

using namespace moss;
using namespace mslib;

static Value *get_attr(Value *obj, ustring name, Interpreter *vm, Value *&err) {
    auto v = obj->get_attr(name);
    if (!v) {
        // TODO: Perhaps this should be a special version of this error?
        err = create_attribute_error(diags::Diagnostic(*vm->get_src_file(), diags::ATTRIB_NOT_DEFINED, obj->get_type()->get_name().c_str(), name.c_str()));
    }
    return v;
}

void exit(Interpreter *vm, Value *code) {
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

Value *vardump(Interpreter *vm, Value *v) {
    (void)vm;
    std::stringstream ss;
    ss << *v << "\n";
    return new StringValue(ss.str());
}

static bool str_to_ios_mode(const std::string& mode, std::ios_base::openmode &ios_mode) {
    static const std::unordered_map<std::string, std::ios_base::openmode> mode_map = {
        {"r", std::ios_base::in},
        {"w", std::ios_base::out},
        {"a", std::ios_base::app},
        {"rb", std::ios_base::in | std::ios_base::binary},
        {"wb", std::ios_base::out | std::ios_base::binary},
        {"ab", std::ios_base::app | std::ios_base::binary},
        {"r+", std::ios_base::in | std::ios_base::out},
        {"w+", std::ios_base::in | std::ios_base::out | std::ios_base::trunc},
        {"a+", std::ios_base::in | std::ios_base::out | std::ios_base::app},
        {"r+b", std::ios_base::in | std::ios_base::out | std::ios_base::binary},
        {"w+b", std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary},
        {"a+b", std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::binary}
    };

    auto it = mode_map.find(mode);
    if (it != mode_map.end()) {
        ios_mode = it->second;
        return true;
    }
    return false;
}

Value *open(Interpreter *vm, Value *ths, Value *&err) {
    auto path = get_attr(ths, "path", vm, err);
    auto mode = get_attr(ths, "mode", vm, err);
    std::ios_base::openmode ios_mode;
    if (!str_to_ios_mode(mode->as_string(), ios_mode)) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::INVALID_FOPEN_MODE, mode->as_string().c_str()));
        return BuiltIns::Nil;
    }
    std::fstream *fs = new std::fstream(path->as_string(), ios_mode);
    if (!fs->is_open()) {
        // TODO: Give more precise error, if file cannot be open or cannot be found
        err = create_file_not_found_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_OPEN_FILE, path->as_string().c_str()));
        return BuiltIns::Nil;
    }
    ths->set_attr("__fstream", new t_cpp::FStreamValue(fs));
    return BuiltIns::Nil;
}

Value *readlines(Interpreter *vm, Value *ths, Value *&err) {
    assert(ths->has_attr("__fstream") && "no __fstream generated");
    auto fsv = ths->get_attr("__fstream");
    auto fsfs = dyn_cast<t_cpp::FStreamValue>(fsv);
    assert(fsfs && "fstream is not std::fstream");
    auto lines = new ListValue();
    ustring line;
    std::fstream *fstrm = fsfs->get_fs();
    while(std::getline(*fstrm, line)) {
        lines->push(new StringValue(line));
    }
    return lines;
}

Value *print(Interpreter *vm, Value *msgs, Value *end, Value *separator) {
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

Value *rand_int(Interpreter *vm, Value *min, Value *max) {
    (void)vm;
    static std::random_device rng_device;
    auto min_int = dyn_cast<IntValue>(min);
    auto max_int = dyn_cast<IntValue>(max);
    assert(min_int && max_int && "not ints");
    std::uniform_int_distribution<opcode::IntConst> distrib(min_int->get_value(), max_int->get_value());
    return new IntValue(distrib(rng_device));
}

Value *input(Interpreter *vm, Value *prompt) {
    (void)vm;
    auto msg = prompt->as_string();
    if (!msg.empty())
        outs << msg;
    ustring line;
    std::getline(std::cin, line);
    return new StringValue(line);
}

Value *Int(Interpreter *vm, Value * ths, Value *v, Value *base) {
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

Value *List_length(Interpreter *vm, Value *ths, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    return new IntValue(lv->get_vals().size());
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
    else if (name == "input") {
        assert(arg_size == 1 && "Mismatch of args");
        ret_v = input(vm, args[0].value);
    }
    else if (name == "Int") {
        assert((arg_size == 3 || arg_size == 2) && "Mismatch of args");
        // Base might not be set as this is only for string argument
        ret_v = Int(vm, cf->get_arg("this"), cf->get_arg("v"), cf->get_arg("base", true));
    }
    else if (name == "open") {
        assert(arg_size == 1 && "Mismatch of args");
        assert(args[0].value->get_type() == BuiltIns::File && "Not File open called");
        ret_v = open(vm, args[0].value, err);
    }
    else if (name == "length") {
        assert(arg_size == 1 && "Mismatch of args");
        if (args[0].value->get_type() == BuiltIns::List) {
            ret_v = List_length(vm, args[0].value, err);
        } else {
            err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, name.c_str()));
        }
    }
    else if (name == "readlines") {
        assert(arg_size == 1 && "Mismatch of args");
        assert(args[0].value->get_type() == BuiltIns::File && "Not File open called");
        ret_v = readlines(vm, args[0].value, err);
    }
    else if (name == "rand_int") {
        assert(arg_size == 2 && "Mismatch of args");
        // Base might not be set as this is only for string argument
        ret_v = rand_int(vm, cf->get_arg("min"), cf->get_arg("max"));
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

void mslib::global_init() {
    // Init rng
    std::random_device rd;
    std::mt19937 gen(rd());
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