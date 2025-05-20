#include "mslib.hpp"
#include "values.hpp"
#include "values_cpp.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "mslib_list.hpp"
#include "mslib_string.hpp"
#include "mslib_file.hpp"
#include <functional>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <random>
#include <cmath>
#include <chrono>
#include <thread>
#include <bitset>
#include <climits>

using namespace moss;
using namespace mslib;

Value *mslib::get_attr(Value *obj, ustring name, Interpreter *vm, Value *&err) {
    auto v = obj->get_attr(name, vm);
    if (!v) {
        // TODO: Perhaps this should be a special version of this error?
        err = create_attribute_error(diags::Diagnostic(*vm->get_src_file(), diags::ATTRIB_NOT_DEFINED, obj->get_type()->get_name().c_str(), name.c_str()));
    }
    return v;
}

Value *vardump(Interpreter *vm, Value *v) {
    (void)vm;
    std::stringstream ss;
    ss << *v << "\n";
    return new StringValue(ss.str());
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

Value *round(Interpreter *vm, Value *n, Value *ndigits) {
    (void)vm;
    assert(isa<FloatValue>(n) || isa<IntValue>(n));
    if (isa<NilValue>(ndigits)) {
        if (isa<IntValue>(n)) return n;
        return new IntValue(std::round(n->as_float()));
    } else {
        auto nfc = dyn_cast<IntValue>(ndigits)->get_value();
        double factor = std::pow(10.0, nfc);
        auto rounded = std::round(n->as_float() * factor) / factor;
        return new FloatValue(rounded);
    }
}

Value *input(Interpreter *vm, Value *prompt, Value *&err) {
    (void)vm;
    auto msg = prompt->as_string();
    if (!msg.empty())
        outs << msg;
    ustring line;
    std::getline(std::cin, line);
    if (std::cin.eof()) {
        err = create_eof_error(diags::Diagnostic(*vm->get_src_file(), diags::EOF_INPUT));
        return nullptr;
    } else if (std::cin.fail()) {
        // TODO: Handle
        assert(false && "error in input");
    }
    return new StringValue(line);
}

Value *hex(Interpreter *vm, Value *number) {
    (void)vm;
    auto ni = dyn_cast<IntValue>(number);
    assert(ni);
    bool is_negative = ni->get_value() < 0;
    std::stringstream ss;
    ss << std::hex << std::abs(ni->get_value());
    std::string hex_str = ss.str();
    return new StringValue((is_negative ? "-0x" : "0x") + hex_str);
}

Value *bin(Interpreter *vm, Value *number) {
    (void)vm;
    auto ni = dyn_cast<IntValue>(number);
    assert(ni);
    std::stringstream ss;
    bool is_negative = ni->get_value() < 0;
    ss << std::bitset<sizeof(opcode::IntConst) * CHAR_BIT>(std::abs(ni->get_value()));
    std::string bin_str = ss.str();
    size_t non_zero_pos = bin_str.find_first_not_of('0');
    if (non_zero_pos != std::string::npos) {
        return new StringValue((is_negative ? "-0b" : "0b") + bin_str.substr(non_zero_pos));
    }
    return new StringValue("0b0");
}

Value *oct(Interpreter *vm, Value *number) {
    (void)vm;
    auto ni = dyn_cast<IntValue>(number);
    assert(ni);
    bool is_negative = ni->get_value() < 0;
    std::stringstream ss;
    ss << std::oct << std::abs(ni->get_value());
    std::string oct_str = ss.str();
    return new StringValue((is_negative ? "-0q" : "0q") + oct_str);
}

Value *attrs(Interpreter *vm, Value *obj, Value *&err) {
    (void)err;
    MemoryPool *frame = nullptr;
    ListValue *ats = new ListValue();
    if (obj) {
        frame = obj->get_attrs();
    } else {
        frame = vm->get_global_frame();
    }
    if (!frame)
        return ats;
    for (auto name: frame->get_sym_table_keys()) {
        ats->push(new StringValue(name));
    }
    return ats;
}

Value *divmod(Interpreter *vm, Value *x, Value *y, Value *&err) {
    ListValue *res = new ListValue();
    if (isa<FloatValue>(x) || isa<FloatValue>(y)) {
        if (y->as_float() == 0.0) {
            err = mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::FDIV_BY_ZERO));
            return nullptr;
        }
        opcode::FloatConst quotient = std::floor(x->as_float() / y->as_float());
        res->push(new FloatValue(quotient));
        opcode::FloatConst remainder = std::fmod(x->as_float(), y->as_float());
        res->push(new FloatValue(remainder));
    } else {
        auto xi = dyn_cast<IntValue>(x);
        assert(xi);
        auto yi = dyn_cast<IntValue>(y);
        assert(yi);
        if (yi->get_value() == 0.0) {
            err = mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::DIV_BY_ZERO));
            return nullptr;
        }
        auto divres = std::ldiv(xi->get_value(), yi->get_value());
        res->push(new IntValue(divres.quot));
        res->push(new IntValue(divres.rem));
    }
    assert(res->size() == 2);
    return res;
}

Value *call_type_converter(Interpreter *vm, Value *v, const char *tname, const char *fname, Value *&err) {
    if (!isa<ObjectValue>(v)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::TYPE_CANNOT_BE_CONV, v->get_type()->get_name().c_str(), tname));
        return nullptr;
    }
    
    Value *rval = nullptr;
    diags::DiagID did = diags::DiagID::UNKNOWN;
    auto int_f = opcode::lookup_method(vm, v, fname, {}, did);
    if (int_f) {
        rval = opcode::runtime_method_call(vm, int_f, {v});
    } else {
        if (did == diags::DiagID::UNKNOWN) {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NO_TYPE_CONV_F_DEFINED, v->get_type()->get_name().c_str(), tname, fname));
        } else {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, fname, diags::DIAG_MSGS[did]));
        }
        return nullptr;
    }
    assert(rval && "Nothing returned?");
    return rval;
}

Value *Int(Interpreter *vm, Value *ths, Value *v, Value *base, Value *&err) {
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
    assert(!base && "v should be String if base is not null");
    auto rval = call_type_converter(vm, v, "Int", "__Int", err);
    if (!err && rval && !isa<IntValue>(rval)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Int", rval->get_type()->get_name().c_str()));
    }
    return rval;
}

Value *Float(Interpreter *vm, Value *ths, Value *v, Value *&err) {
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
    auto rval = call_type_converter(vm, v, "Float", "__Float", err);
    if (!err && rval && !isa<FloatValue>(rval)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Float", rval->get_type()->get_name().c_str()));
    }
    return rval;
}

Value *Bool(Interpreter *vm, Value *ths, Value *v, Value *&err) {
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
    if (auto dv = dyn_cast<DictValue>(v)) {
        return new BoolValue(dv->size() != 0);
    }
    if (isa<ObjectValue>(v)) {
        auto rval = call_type_converter(vm, v, "Bool", "__Bool", err);
        if (!err && rval && !isa<BoolValue>(rval)) {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Bool", rval->get_type()->get_name().c_str()));
        }
        return rval;
    }
    
    return new BoolValue(true);
}

Value *String_String(Interpreter *vm, Value *ths, Value *v, Value *&err) {
    (void)ths;
    (void)err;
    if (isa<ObjectValue>(v)) {
        Value *trash_err = nullptr;
        auto rval = call_type_converter(vm, v, "String", "__String", trash_err);
        if (rval && !trash_err) {
            if (!isa<StringValue>(rval)) {
                rval = new StringValue(rval->as_string());
            }
            return rval;
        }
    }
    return new StringValue(v->as_string());
}

Value *Note(Interpreter *vm, Value *ths, Value *format, Value *value) {
    (void)vm;
    (void)ths;
    auto str_val = dyn_cast<StringValue>(value);
    assert(str_val && "Note did not take string value");
    return new NoteValue(format->as_string(), str_val);
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
        {"abs", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            auto args = cf->get_args();
            assert(args.size() == 1);
            if (auto vi = dyn_cast<IntValue>(args[0].value))
                return new IntValue(std::abs(vi->get_value()));
            else {
                assert(isa<FloatValue>(args[0].value));
                return new FloatValue(std::fabs(args[0].value->as_float()));
            }
        }},
        {"append", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            if (args[1].value->get_type() == BuiltIns::List) {
                return List::append(vm, cf->get_arg("this"), cf->get_arg("v"), err);
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, args[1].value->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"attrs", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            assert(args.size() <= 1);
            Value *obj = nullptr;
            if (args.size() == 1) {
                obj = args[0].value;
            }
            return attrs(vm, obj, err);
        }},
        {"bin", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return bin(vm, args[0].value);
        }},
        {"Bool", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            return Bool(vm, cf->get_arg("this"), cf->get_arg("v"), err);
        }},
        {"capitalize", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::String);
            return String::capitalize(vm, args[0].value, err);
        }},
        {"chr", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            auto ii = dyn_cast<IntValue>(cf->get_args()[0].value);
            assert(ii);
            auto iiv = ii->get_value();
            if (iiv > 0x10ffff || iiv < 0) {
                err = create_value_error(
                    diags::Diagnostic(*vm->get_src_file(), 
                        diags::CHR_NOT_IN_RANGE, ii->get_value()));
            }
            return new StringValue(ustring(1, ii->get_value()));
        }},
        {"close", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::File);
            return MSFile::close(vm, args[0].value, err);
        }},
        {"cos", [](Interpreter*, CallFrame* cf, Value*&) {
            return new FloatValue(std::cos(cf->get_args()[0].value->as_float()));
        }},
        {"delattr", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            (void)err;
            assert(cf->get_args().size() == 2);
            auto obj = cf->get_arg("obj");
            assert(obj);
            if (!obj->is_modifiable()) {
                err = create_attribute_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_DELETE_ATTR, obj->get_type()->get_name().c_str()));
                return nullptr;
            }
            auto name = dyn_cast<StringValue>(cf->get_arg("name"));
            assert(name);
            if (!obj->del_attr(name->get_value(), vm)) {
                err = create_attribute_error(diags::Diagnostic(*vm->get_src_file(), diags::ATTRIB_NOT_DEFINED, obj->get_type()->get_name().c_str(), name->get_value().c_str()));
                return nullptr;
            }
            return BuiltIns::Nil;
        }},
        {"divmod", [](Interpreter* vm, CallFrame* cf, Value *&err) {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return divmod(vm, cf->get_arg("x"), cf->get_arg("y"), err);
        }},
        {"Float", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            return Float(vm, cf->get_arg("this"), cf->get_arg("v"), err);
        }},
        {"getattr", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            (void)err;
            assert(cf->get_args().size() == 2);
            auto name = dyn_cast<StringValue>(cf->get_arg("name"));
            assert(name);
            auto obj = cf->get_arg("obj");
            assert(name);
            auto attr_ret = get_attr(obj, name->get_value(), vm, err);
            return attr_ret;
        }},
        {"hasattr", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            (void)err;
            assert(cf->get_args().size() == 2);
            auto name = dyn_cast<StringValue>(cf->get_arg("name"));
            assert(name);
            auto obj = cf->get_arg("obj");
            assert(name);
            return new BoolValue(obj->has_attr(name->get_value(), vm));
        }},
        {"hash", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return new IntValue(moss::hash(cf->get_arg("obj"), vm));
        }},
        {"hex", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return hex(vm, cf->get_args()[0].value);
        }},
        {"id", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            return new IntValue(reinterpret_cast<opcode::IntConst>(cf->get_args()[0].value));
        }},
        {"input", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return input(vm, cf->get_args()[0].value, err);
        }},
        {"Int", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert((args.size() == 2 || args.size() == 3));
            return Int(vm, cf->get_arg("this"), cf->get_arg("v"), cf->get_arg("base"), err);
        }},
        /*{"join", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            if (args[1].value->get_type() == BuiltIns::String) {
                return String::join(vm, cf->get_arg("this"), cf->get_arg("iterable"), err);
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, args[1].value->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},*/
        {"length", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_args()[0].value;
            if (auto lv = dyn_cast<ListValue>(arg)) {
                return new IntValue(lv->get_vals().size());
            } else if (auto stv = dyn_cast<StringValue>(arg)) {
                return new IntValue(stv->get_value().length());
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"List", [](Interpreter*, CallFrame* cf, Value*&) {
            assert(cf->get_args().size() == 2);
            return cf->get_arg("vals");
        }},
        {"log", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)vm;
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            auto xf = cf->get_arg("x");
            assert(xf);
            auto basef = cf->get_arg("base");
            assert(basef);
            return new FloatValue(std::log(xf->as_float()) / std::log(basef->as_float()));
        }},
        {"lower", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::String);
            return String::lower(vm, args[0].value, err);
        }},
        {"lshift", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)vm;
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            auto ai = dyn_cast<IntValue>(cf->get_arg("a"));
            assert(ai);
            auto counti = dyn_cast<IntValue>(cf->get_arg("count"));
            assert(counti);
            return new IntValue(ai->get_value() << counti->get_value());
        }},
        {"NilType", [](Interpreter*, CallFrame*, Value*&) {
            return new NilValue();
        }},
        {"Note", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 3);
            return Note(vm, cf->get_arg("this"), cf->get_arg("format"), cf->get_arg("value"));
        }},
        {"oct", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return oct(vm, args[0].value);
        }},
        {"open", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::File);
            return MSFile::open(vm, args[0].value, err);
        }},
        {"ord", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            auto s = dyn_cast<StringValue>(cf->get_args()[0].value);
            assert(s);
            auto sv = s->get_value();
            if (sv.size() != 1) {
                err = create_value_error(
                    diags::Diagnostic(*vm->get_src_file(), 
                        diags::ORD_INCORRECT_LENGTH, sv.c_str()));
            }
            return new IntValue(static_cast<opcode::IntConst>(sv[0]));
        }},
        {"pop", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            if (args[1].value->get_type() == BuiltIns::List) {
                return List::pop(vm, cf->get_arg("this"), cf->get_arg("index"), err);
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, args[1].value->get_type()->get_name().c_str()));
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
            return MSFile::readlines(vm, args[0].value, err);
        }},
        {"round", [](Interpreter* vm, CallFrame* cf, Value*&) {
            assert(cf->get_args().size() == 2);
            return round(vm, cf->get_arg("n"), cf->get_arg("ndigits"));
        }},
        {"rshift", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)vm;
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 2);
            auto ai = dyn_cast<IntValue>(cf->get_arg("a"));
            assert(ai);
            auto counti = dyn_cast<IntValue>(cf->get_arg("count"));
            assert(counti);
            return new IntValue(ai->get_value() >> counti->get_value());
        }},
        {"setattr", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            (void)err;
            assert(cf->get_args().size() == 3);
            auto obj = cf->get_arg("obj");
            assert(obj);
            if (!obj->is_modifiable()) {
                err = create_attribute_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_CREATE_ATTR, obj->get_type()->get_name().c_str()));
                return nullptr;
            }
            auto name = dyn_cast<StringValue>(cf->get_arg("name"));
            assert(name);
            auto value = cf->get_arg("value");
            assert(value);
            obj->set_attr(name->get_value(), value);
            return BuiltIns::Nil;
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
            (void)vm;
            assert(cf->get_args().size() == 2);
            return String_String(vm, cf->get_arg("this"), cf->get_arg("v"), err);
        }},
        {"strip", [](Interpreter*, CallFrame* cf, Value*&) {
            auto strv = dyn_cast<StringValue>(cf->get_args()[0].value)->get_value();
            utils::trim(strv);
            return new StringValue(strv);
        }},
        {"tan", [](Interpreter*, CallFrame* cf, Value*&) {
            return new FloatValue(std::tan(cf->get_args()[0].value->as_float()));
        }},
        {"type", [](Interpreter* vm, CallFrame* cf, Value*& err)  {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            return cf->get_args()[0].value->get_type();
        }},
        {"upper", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(args[0].value->get_type() == BuiltIns::String);
            return String::upper(vm, args[0].value, err);
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
            return MSFile::write(vm, cf->get_arg("this"), cf->get_arg("content"), err);
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
    } else {
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
