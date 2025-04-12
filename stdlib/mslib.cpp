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
#include <cmath>
#include <chrono>
#include <thread>

using namespace moss;
using namespace mslib;

static Value *get_attr(Value *obj, ustring name, Interpreter *vm, Value *&err) {
    auto v = obj->get_attr(name, vm);
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

Value *File_open(Interpreter *vm, Value *ths, Value *&err) {
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

Value *File_close(Interpreter *vm, Value *ths, Value *&err) {
    // TODO: Check if file is open
    auto fstrm_v = get_attr(ths, "__fstream", vm, err);
    auto fstrm = dyn_cast<t_cpp::FStreamValue>(fstrm_v);
    assert(fstrm && "Not FStream value");
    fstrm->get_fs()->close();
    ths->set_attr("__fstream", BuiltIns::Nil);
    return BuiltIns::Nil;
}

Value *File_readlines(Interpreter *vm, Value *ths, Value *&err) {
    // TODO: Generate exceptions on errors
    assert(ths->has_attr("__fstream", vm) && "no __fstream generated");
    auto fsv = ths->get_attr("__fstream", vm);
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

Value *File_write(Interpreter *vm, Value *ths, Value *content, Value *&err) {
    // TODO: Generate exceptions on errors
    assert(ths->has_attr("__fstream", vm) && "no __fstream generated");
    auto fsv = ths->get_attr("__fstream", vm);
    auto fsfs = dyn_cast<t_cpp::FStreamValue>(fsv);
    assert(fsfs && "fstream is not std::fstream");
    *(fsfs->get_fs()) << content->as_string();
    return BuiltIns::Nil;
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

Value *rand_float(Interpreter *vm, Value *min, Value *max) {
    (void)vm;
    static std::random_device rng_device;
    assert((isa<FloatValue>(min) || isa<IntValue>(min)) && "not int/float");
    assert((isa<FloatValue>(max) || isa<IntValue>(max)) && "not int/float");
    auto min_int = min->as_float();
    auto max_int = max->as_float();
    std::uniform_real_distribution<opcode::FloatConst> distrib(min_int, max_int);
    return new FloatValue(distrib(rng_device));
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

Value *Int(Interpreter *vm, Value *ths, Value *v, Value *base) {
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
    
    // TODO: Raise error 
    assert(false && "Incorrect arg type");
    return new IntValue(0);
}

Value *Float(Interpreter *vm, Value *ths, Value *v) {
    (void)vm;
    (void)ths;

    if (isa<FloatValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        char *pend;
        errno = 0;
        double vf = std::strtod(sv->as_string().c_str(), &pend);
        if (*pend != '\0')
            assert(false && "TODO: Raise error value is not int");
        if (errno != 0) {
            LOGMAX("Errno error: " << strerror(errno));
            assert(false && "TODO: Raise conversion error");
        }
        return new FloatValue(vf);
    }
    if (auto fv = dyn_cast<IntValue>(v)) {
        return new FloatValue(static_cast<opcode::FloatConst>(fv->get_value()));
    }
    
    assert(false && "Incorrect arg type");
    return new FloatValue(0.0);
}

Value *Bool(Interpreter *vm, Value *ths, Value *v) {
    (void)vm;
    (void)ths;

    if (isa<BoolValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        return new BoolValue(!sv->get_value().empty());
    }
    if (auto iv = dyn_cast<IntValue>(v)) {
        return new BoolValue(iv->get_value() != 0);
    }
    if (auto fv = dyn_cast<FloatValue>(v)) {
        return new BoolValue(fv->get_value() != 0.0);
    }
    if (isa<NilValue>(v)) {
        return new BoolValue(false);
    }
    if (auto lv = dyn_cast<ListValue>(v)) {
        return new BoolValue(!lv->get_vals().empty());
    }
    // TODO: Add dict once implemented
    //if (auto dv = dyn_cast<DictValue>(v)) {
    //    return new BoolValue(!dv->get_keys().empty());
    //}
    
    return new BoolValue(true);
}

Value *String(Interpreter *vm, Value *ths, Value *v) {
    (void)vm;
    (void)ths;
    return new StringValue(v->as_string());
}

Value *Note(Interpreter *vm, Value *ths, Value *format, Value *value) {
    (void)vm;
    (void)ths;
    auto str_val = dyn_cast<StringValue>(value);
    assert(str_val && "Note did not take string value");
    return new NoteValue(format->as_string(), str_val);
}

Value *List_length(Interpreter *vm, Value *ths, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    return new IntValue(lv->get_vals().size());
}

Value *List_append(Interpreter *vm, Value * ths, Value *v, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    // v is vararg
    auto vls = dyn_cast<ListValue>(v);
    assert(vls && "not vararg");
    for (auto e: vls->get_vals())
        lv->push(e);
    return BuiltIns::Nil;
}

Value *List_pop(Interpreter *vm, Value *ths, Value *index, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    auto iv = dyn_cast<IntValue>(index);
    assert(iv);
    auto ivint = iv->get_value();
    Value *removed = nullptr;
    if ((ivint < 0 && static_cast<unsigned long>(ivint*-1) > lv->get_vals().size()) || 
        (ivint >= 0 && static_cast<unsigned long>(ivint) >= lv->get_vals().size())) {
            err = mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, lv->get_type()->get_name().c_str(), ivint));
        return BuiltIns::Nil;
    }
    if (ivint < 0) {
        removed = lv->get_vals()[lv->size() + ivint];
        lv->remove(lv->size() + ivint);
    } else {
        removed = lv->get_vals()[ivint];
        lv->remove(ivint);
    }
    return removed;
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

const std::unordered_map<std::string, mslib::mslib_dispatcher>& FunctionRegistry::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"append", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            if (args[1].value->get_type() == BuiltIns::List) {
                return List_append(vm, cf->get_arg("this"), cf->get_arg("v"), err);
            } else {
                err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, "append"));
                return nullptr;
            }
        }},
        {"Bool", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            return Bool(vm, cf->get_arg("this"), cf->get_arg("v"));
        }},
        {"close", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::File);
            return File_close(vm, args[0].value, err);
        }},
        {"cos", [](Interpreter*, CallFrame* cf, Value*&) {
            return new FloatValue(std::cos(cf->get_args()[0].value->as_float()));
        }},
        {"exit", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1 && "Mismatch of args");
            exit(vm, args[0].value);
            return nullptr; // Won’t be used, return early in dispatch()
        }},
        {"Float", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            return Float(vm, cf->get_arg("this"), cf->get_arg("v"));
        }},
        {"input", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return input(vm, cf->get_args()[0].value);
        }},
        {"Int", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert((args.size() == 2 || args.size() == 3));
            return Int(vm, cf->get_arg("this"), cf->get_arg("v"), cf->get_arg("base", true));
        }},
        {"length", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_args()[0].value;
            if (arg->get_type() == BuiltIns::List) {
                return List_length(vm, arg, err);
            } else if (auto stv = dyn_cast<StringValue>(arg)) {
                return new IntValue(stv->get_value().length());
            } else {
                err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, "length"));
                return nullptr;
            }
        }},
        {"List", [](Interpreter*, CallFrame* cf, Value*&) {
            assert(cf->get_args().size() == 2);
            return cf->get_arg("vals");
        }},
        {"NilType", [](Interpreter*, CallFrame*, Value*&) {
            return new NilValue();
        }},
        {"Note", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 3);
            return Note(vm, cf->get_arg("this"), cf->get_arg("format"), cf->get_arg("value"));
        }},
        {"open", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::File);
            return File_open(vm, args[0].value, err);
        }},
        {"pop", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            if (args[1].value->get_type() == BuiltIns::List) {
                return List_pop(vm, cf->get_arg("this"), cf->get_arg("index"), err);
            } else {
                err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, "pop"));
                return nullptr;
            }
        }},
        {"print", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 3);
            return print(vm, cf->get_arg("msgs"), cf->get_arg("end"), cf->get_arg("separator"));
        }},
        {"rand_float", [](Interpreter* vm, CallFrame* cf, Value*&) {
            assert(cf->get_args().size() == 2);
            return rand_float(vm, cf->get_arg("min"), cf->get_arg("max"));
        }},
        {"rand_int", [](Interpreter* vm, CallFrame* cf, Value*&) {
            assert(cf->get_args().size() == 2);
            return rand_int(vm, cf->get_arg("min"), cf->get_arg("max"));
        }},
        {"readlines", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::File);
            return File_readlines(vm, args[0].value, err);
        }},
        {"sin", [](Interpreter*, CallFrame* cf, Value*&) {
            return new FloatValue(std::sin(cf->get_args()[0].value->as_float()));
        }},
        {"sleep", [](Interpreter*, CallFrame* cf, Value*&) {
            auto seconds = static_cast<opcode::IntConst>(cf->get_args()[0].value->as_float() * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
            return BuiltIns::Nil;
        }},
        {"String", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            return String(vm, cf->get_arg("this"), cf->get_arg("v"));
        }},
        {"strip", [](Interpreter*, CallFrame* cf, Value*&) {
            auto strv = dyn_cast<StringValue>(cf->get_args()[0].value)->get_value();
            utils::trim(strv);
            return new StringValue(strv);
        }},
        {"tan", [](Interpreter*, CallFrame* cf, Value*&) {
            return new FloatValue(std::tan(cf->get_args()[0].value->as_float()));
        }},
        {"vardump", [](Interpreter* vm, CallFrame* cf, Value*& err)  {
            (void)err;
            assert(cf->get_args().size() == 1);
            return vardump(vm, cf->get_args()[0].value);
        }},
        {"write", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 2);
            assert(args[1].value->get_type() == BuiltIns::File);
            return File_write(vm, cf->get_arg("this"), cf->get_arg("content"), err);
        }},
    };
    return registry;
};

void mslib::dispatch(Interpreter *vm, ustring name, Value *&err) {
    auto cf = vm->get_call_frame();

    const auto& registry = FunctionRegistry::get_registry();
    Value *ret_v = nullptr;

    auto it = registry.find(name);
    // Matching function name to execute internal implementation
    if (it != registry.end()) {
        ret_v = it->second(vm, cf, err);
        // Special case for `exit`, return early to prevent BCI override
        if (name == "exit")
            return;
    } else {
        err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::INTERNAL_WITHOUT_BODY, name.c_str()));
    }

    auto return_reg = vm->get_call_frame()->get_return_reg();
    auto caller_addr = vm->get_call_frame()->get_caller_addr();
    vm->drop_call_frame();
    if (!ret_v)
        ret_v = BuiltIns::Nil;
    vm->store(return_reg, ret_v);
    vm->set_bci(caller_addr);
}
