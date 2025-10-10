#include "mslib.hpp"
#include "values.hpp"
#include "values_cpp.hpp"
#include "errors.hpp"
#include "diagnostics.hpp"
#include "logging.hpp"
#include "mslib_list.hpp"
#include "mslib_string.hpp"
#include "mslib_file.hpp"
#include "mslib_dict.hpp"
#include "subprocess.hpp"
#include "inspect.hpp"
#include "sys.hpp"
#include "cffi.hpp"
#include "time.hpp"
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
#include <regex>
#include <cctype>
#include <cwctype>

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

template<class T>
T *get_subtype_value(Value *v, Value *type, Interpreter *vm, Value *&err) {
    if (auto cv = dyn_cast<T>(v)) {
        return cv;
    }
    if (opcode::is_type_eq_or_subtype(v->get_type(), type)) {
        auto att_val = get_attr(v, known_names::BUILT_IN_EXT_VALUE, vm, err);
        if (!att_val) {
            return nullptr;
        }
        auto t_val = dyn_cast<T>(att_val);
        if (!t_val) {
            err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE_FOR_ATTR, known_names::BUILT_IN_EXT_VALUE, type->get_name().c_str(), att_val->get_type()->get_name().c_str()));
            return nullptr;
        }
        return t_val;
    }
    return nullptr;
}

EnumTypeValue *mslib::get_enum(ustring name, Interpreter *vm, Value *&err) {
    auto enumv = vm->load_name(name);
    if (!enumv) {
        err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, name.c_str()));
        return nullptr;
    }
    auto enumtype = dyn_cast<EnumTypeValue>(enumv);
    if (!enumtype) {
        err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Type", enumv->get_type()->get_name().c_str()));
        return nullptr;
    }
    return enumtype;
}

EnumTypeValue *mslib::get_enum(ustring name, CallFrame *cf, Value *&err) {
    auto funv = cf->get_function();
    assert(funv && "no function in cf");
    auto fun = dyn_cast<FunValue>(funv);
    assert(fun && "Function in cf is not a function?");
    auto vm = fun->get_vm();
    assert(vm && "vm in function not set?");
    return get_enum(name, vm, err);
}

SpaceValue *mslib::get_space(ustring name, Interpreter *vm, Value *&err) {
    auto spacev = vm->load_name(name);
    if (!spacev) {
        err = create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, name.c_str()));
        return nullptr;
    }
    auto spacetype = dyn_cast<SpaceValue>(spacev);
    if (!spacetype) {
        err = create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Space", spacev->get_type()->get_name().c_str()));
        return nullptr;
    }
    return spacetype;
}

opcode::Register mslib::get_global_register_of(Interpreter *vm, ustring name) {
    auto reg = vm->get_global_frame()->get_name_register(name);
    if (!reg)
        opcode::raise(mslib::create_name_error(
            diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, name.c_str())));
    return *reg;
}

Value *mslib::call_type_converter(Interpreter *vm, Value *v, const char *tname, const char *fname, Value *&err) {
    if (!isa<ObjectValue>(v)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::TYPE_CANNOT_BE_CONV, v->get_type()->get_name().c_str(), tname));
        return nullptr;
    }
    
    Value *rval = nullptr;
    diags::DiagID did = diags::DiagID::UNKNOWN;
    auto int_f = opcode::lookup_method(vm, v, fname, {v}, did);
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

Value *mslib::call_constructor(Interpreter *vm, CallFrame *cf, ustring name, std::initializer_list<Value *> args, Value *&err) {
    auto funv = cf->get_function();
    assert(funv);
    auto fun = dyn_cast<FunValue>(funv);
    auto subres_class_v = fun->get_vm()->load_name(name);
    if (!subres_class_v) {
        err = mslib::create_name_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, name.c_str()));
        return nullptr;
    }
    auto subres_class = dyn_cast<ClassValue>(subres_class_v);
    if (!subres_class) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, name.c_str(), subres_class_v->get_type()->get_name().c_str()));
        return nullptr;
    }
    diags::DiagID did = diags::DiagID::UNKNOWN;
    auto constr = opcode::lookup_method(vm, subres_class, name, args, did);
    Value *res = nullptr;
    if (constr) {
        res = opcode::runtime_constructor_call(vm, constr, args, subres_class);
    } else {
        if (did == diags::DiagID::UNKNOWN) {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NAME_NOT_DEFINED, name.c_str()));
        } else {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::INCORRECT_CALL, name.c_str(), diags::DIAG_MSGS[did]));
        }
        return nullptr;
    }
    assert(res && "sanity check");
    return res;
}

Value *mslib::create_exception(Value *type, ustring msg) {
    auto clt = dyn_cast<ClassValue>(type);
    assert(clt && "Passed non class type value");
    auto err = new ObjectValue(clt);
    err->set_attr("msg", StringValue::get(msg));
    return err;
}

Value *mslib::create_exception(Value *type, diags::Diagnostic dmsg) {
    return create_exception(type, error::format_error(dmsg));
}

Value *vardump(Interpreter *vm, Value *v) {
    (void)vm;
    std::stringstream ss;
    ss << *v << "\n";
    return StringValue::get(ss.str());
}

Value *print(Interpreter *vm, Value *msgs, Value *end, Value *separator) {
    (void)vm;
    auto msgs_list = dyn_cast<ListValue>(msgs);
    assert(msgs_list && "msgs is not a vararg?");
    bool first = true;
    for (auto v : msgs_list->get_vals()) {
        if (first) {
            outs << opcode::to_string(vm, v);
            first = false;
        }
        else {
            outs << opcode::to_string(vm, separator) << opcode::to_string(vm, v);
        }
    }
    outs << opcode::to_string(vm, end);
    return BuiltIns::Nil;
}

Value *rand_int(Interpreter *vm, Value *min, Value *max, Value *&err) {
    (void)vm;
    static std::random_device rng_device;
    auto min_int = dyn_cast<IntValue>(min);
    auto max_int = dyn_cast<IntValue>(max);
    assert(min_int && max_int && "not ints");
    if (min_int->get_value() > max_int->get_value()) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::MIN_LT_MAX_IN_RANDINT, min_int->get_value(), max_int->get_value()));
        return nullptr;
    }
    std::uniform_int_distribution<opcode::IntConst> distrib(min_int->get_value(), max_int->get_value());
    return IntValue::get(distrib(rng_device));
}

Value *rand_float(Interpreter *vm, Value *min, Value *max, Value *&err) {
    (void)vm;
    static std::random_device rng_device;
    assert((isa<FloatValue>(min) || isa<IntValue>(min)) && "not int/float");
    assert((isa<FloatValue>(max) || isa<IntValue>(max)) && "not int/float");
    auto min_f = min->as_float();
    auto max_f = max->as_float();
    if (min_f > max_f) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::MIN_LT_MAX_IN_RANDF, min_f, max_f));
        return nullptr;
    }
    std::uniform_real_distribution<opcode::FloatConst> distrib(min_f, max_f);
    return FloatValue::get(distrib(rng_device));
}

Value *round(Interpreter *vm, Value *n, Value *ndigits) {
    (void)vm;
    assert(isa<FloatValue>(n) || isa<IntValue>(n));
    if (isa<NilValue>(ndigits)) {
        if (isa<IntValue>(n)) return n;
        return IntValue::get(std::round(n->as_float()));
    } else {
        auto nfc = dyn_cast<IntValue>(ndigits)->get_value();
        double factor = std::pow(10.0, nfc);
        auto rounded = std::round(n->as_float() * factor) / factor;
        return FloatValue::get(rounded);
    }
}

Value *input(Interpreter *vm, Value *prompt, Value *&err) {
    (void)vm;
    auto msg = opcode::to_string(vm, prompt);
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
    return StringValue::get(line);
}

Value *hex(Interpreter *vm, Value *number) {
    (void)vm;
    auto ni = dyn_cast<IntValue>(number);
    assert(ni);
    bool is_negative = ni->get_value() < 0;
    std::stringstream ss;
    ss << std::hex << std::abs(ni->get_value());
    std::string hex_str = ss.str();
    return StringValue::get((is_negative ? "-0x" : "0x") + hex_str);
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
        return StringValue::get((is_negative ? "-0b" : "0b") + bin_str.substr(non_zero_pos));
    }
    return StringValue::get("0b0");
}

Value *oct(Interpreter *vm, Value *number) {
    (void)vm;
    auto ni = dyn_cast<IntValue>(number);
    assert(ni);
    bool is_negative = ni->get_value() < 0;
    std::stringstream ss;
    ss << std::oct << std::abs(ni->get_value());
    std::string oct_str = ss.str();
    return StringValue::get((is_negative ? "-0q" : "0q") + oct_str);
}

Value *callable(Interpreter *vm, Value *obj) {
    (void)vm;
    bool is_callable = isa<FunValue>(obj) || isa<FunValueList>(obj) || isa<ClassValue>(obj);
    return BoolValue::get(is_callable);
}

Value *attrs(Interpreter *vm, Value *obj, Value *&err) {
    // Regex to match anonymous lambda names and anonymous space names
    static const std::regex ANON_VALUES(R"(^\d+(?:l|s)$)");
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
        if (!std::regex_match(name, ANON_VALUES))
            ats->push(StringValue::get(name));
    }
    return ats;
}

Value *symbols(Interpreter *vm, MemoryPool *frame) {
    auto sym_tbl = frame->get_sym_table_keys();
    std::vector<Value *> keys;
    std::vector<Value *> vals;
    for (auto name: sym_tbl) {
        keys.push_back(StringValue::get(name));
        auto v = frame->load_name(name, vm);
        vals.push_back(v);
    }
    auto dc = new DictValue();
    dc->push(keys, vals, vm);
    return dc;
}

Value *divmod(Interpreter *vm, Value *x, Value *y, Value *&err) {
    ListValue *res = new ListValue();
    if (isa<FloatValue>(x) || isa<FloatValue>(y)) {
        if (y->as_float() == 0.0) {
            err = mslib::create_division_by_zero_error(diags::Diagnostic(*vm->get_src_file(), diags::FDIV_BY_ZERO));
            return nullptr;
        }
        opcode::FloatConst quotient = std::floor(x->as_float() / y->as_float());
        res->push(FloatValue::get(quotient));
        opcode::FloatConst remainder = std::fmod(x->as_float(), y->as_float());
        res->push(FloatValue::get(remainder));
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
        res->push(IntValue::get(divres.quot));
        res->push(IntValue::get(divres.rem));
    }
    assert(res->size() == 2);
    return res;
}

Value *Int(Interpreter *vm, Value *v, Value *base, Value *&err) {
    (void)vm;
    IntValue *base_int = nullptr;
    if (base)
        base_int = dyn_cast<IntValue>(base);

    if (isa<IntValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        assert(base_int && "TODO: Raise type exception as base is not int");
        char *pend;
        errno = 0;
        auto vi = std::strtoll(sv->get_value().c_str(), &pend, base_int->get_value());
        if (*pend != '\0')
            assert(false && "TODO: Raise error value is not int");
        if (errno != 0) {
            LOGMAX("Errno error: " << strerror(errno));
            assert(false && "TODO: Raise conversion error");
        }
        return IntValue::get(vi);
    }
    if (auto fv = dyn_cast<FloatValue>(v)) {
        return IntValue::get(static_cast<opcode::IntConst>(fv->get_value()));
    }
    assert(!base && "v should be String if base is not null");
    auto rval = call_type_converter(vm, v, "Int", known_names::TO_INT_METHOD, err);
    if (!err && rval && !isa<IntValue>(rval)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Int", rval->get_type()->get_name().c_str()));
    }
    return rval;
}

Value *Float(Interpreter *vm, Value *v, Value *&err) {
    (void)vm;

    if (isa<FloatValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        char *pend;
        errno = 0;
        double vf = std::strtod(sv->get_value().c_str(), &pend);
        if (*pend != '\0')
            assert(false && "TODO: Raise error value is not int");
        if (errno != 0) {
            LOGMAX("Errno error: " << strerror(errno));
            assert(false && "TODO: Raise conversion error");
        }
        return FloatValue::get(vf);
    }
    if (auto fv = dyn_cast<IntValue>(v)) {
        return FloatValue::get(static_cast<opcode::FloatConst>(fv->get_value()));
    }
    auto rval = call_type_converter(vm, v, "Float", known_names::TO_FLOAT_METHOD, err);
    if (!err && rval && !isa<FloatValue>(rval)) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Float", rval->get_type()->get_name().c_str()));
    }
    return rval;
}

Value *Bool(Interpreter *vm, Value *v, Value *&err) {
    if (isa<BoolValue>(v))
        return v;
    if (auto sv = dyn_cast<StringValue>(v)) {
        return BoolValue::get(!sv->get_value().empty());
    }
    if (auto iv = dyn_cast<IntValue>(v)) {
        return BoolValue::get(iv->get_value() != 0);
    }
    if (auto fv = dyn_cast<FloatValue>(v)) {
        return BoolValue::get(fv->get_value() != 0.0);
    }
    if (isa<NilValue>(v)) {
        return BoolValue::False();
    }
    if (auto lv = dyn_cast<ListValue>(v)) {
        return BoolValue::get(!lv->get_vals().empty());
    }
    if (auto dv = dyn_cast<DictValue>(v)) {
        return BoolValue::get(dv->size() != 0);
    }
    if (isa<ObjectValue>(v)) {
        auto rval = call_type_converter(vm, v, "Bool", known_names::TO_BOOL_METHOD, err);
        if (!err && rval && !isa<BoolValue>(rval)) {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::UNEXPECTED_TYPE, "Bool", rval->get_type()->get_name().c_str()));
        }
        return rval;
    }
    
    return BoolValue::get(true);
}

Value *Note(Interpreter *vm, Value *format, Value *value) {
    (void)vm;
    auto str_val = dyn_cast<StringValue>(value);
    assert(str_val && "Note did not take string value");
    return new NoteValue(format->as_string(), str_val);
}

Value *isinstance(Interpreter *vm, Value *obj, Value *types, Value *&err) {
    auto type_list = dyn_cast<ListValue>(types);
    bool is_class = true;
    if (type_list) {
        for (auto v: type_list->get_vals()) {
            if (!isa<ClassValue>(v)) {
                is_class = false;
                break;
            }
        }
    } else {
        is_class = isa<ClassValue>(types);
    }
    if (!is_class) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::ISINSTANCE_REQUIRES_CLASS));
        return nullptr;
    }

    if (type_list) {
        for (auto v: type_list->get_vals()) {
            if (opcode::is_type_eq_or_subtype(obj->get_type(), v)) {
                return BuiltIns::True;
            }
        }
    } else {
        if (opcode::is_type_eq_or_subtype(obj->get_type(), types))
            return BuiltIns::True;
    }
    return BuiltIns::False;
}

Value *issubclass(Interpreter *vm, Value *cls, Value *types, Value *&err) {
    assert(isa<ClassValue>(cls) && "Not a class passed in");
    auto type_list = dyn_cast<ListValue>(types);
    bool is_class = true;
    if (type_list) {
        for (auto v: type_list->get_vals()) {
            if (!isa<ClassValue>(v)) {
                is_class = false;
                break;
            }
        }
    } else {
        is_class = isa<ClassValue>(types);
    }
    if (!is_class) {
        err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::ISSUBCLASS_REQUIRES_CLASS));
        return nullptr;
    }

    if (type_list) {
        for (auto v: type_list->get_vals()) {
            if (opcode::is_type_eq_or_subtype(cls, v)) {
                return BuiltIns::True;
            }
        }
    } else {
        if (opcode::is_type_eq_or_subtype(cls, types))
            return BuiltIns::True;
    }
    return BuiltIns::False;
}

Value *String_isfun(Interpreter *vm, CallFrame *cf, std::function<bool(std::wint_t)> fn, Value *&err) {
    assert(cf->get_args().size() == 1);
    auto arg = cf->get_arg("this");
    auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
    if (err)
        return nullptr;
    if (!sv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
        return nullptr;
    }
    return String::isfun(vm, arg, fn, err);
}

const std::unordered_map<std::string, mslib::mslib_dispatcher>& FunctionRegistry::get_registry(ustring module_name) {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> libms_registry = {
        {known_names::OBJECT_ITERATOR, [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            auto ths = args[0].value;
            if (isa<ObjectValue>(ths)) {
                ths = get_attr(ths, known_names::BUILT_IN_EXT_VALUE, vm, err);
                if (!ths) {
                    return nullptr;
                } 
            }
            return ths->iter(vm);
        }},
        {known_names::ITERATOR_NEXT, [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            auto ths = args[0].value;
            if (isa<ObjectValue>(ths)) {
                ths = get_attr(ths, known_names::BUILT_IN_EXT_VALUE, vm, err);
                if (!ths) {
                    return nullptr;
                } 
            }
            return ths->next(vm);
        }},
        {"abs", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            auto args = cf->get_args();
            assert(args.size() == 1);
            if (auto vi = dyn_cast<IntValue>(args[0].value))
                return IntValue::get(std::abs(vi->get_value()));
            else {
                assert(isa<FloatValue>(args[0].value));
                return FloatValue::get(std::fabs(args[0].value->as_float()));
            }
        }},
        {"append", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            auto ths = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(ths, BuiltIns::List, vm, err)) {
                return List::append(vm, lv, cf->get_arg("v"), err);
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
        {"Bool", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            assert(cf->get_args().size() == 2);
            auto ths = cf->get_arg("this");
            auto bv = Bool(vm, cf->get_arg("v"), err);
            if (ths->get_type() == BuiltIns::Bool) {
                return bv;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::Bool)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, bv);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"capitalize", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_args()[0].value;
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            return String::capitalize(vm, sv, err);
        }},
        {"callable", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            auto args = cf->get_args();
            assert(cf->get_args().size() == 1);
            return callable(vm, args[0].value);
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
            return StringValue::get(ustring(1, ii->get_value()));
        }},
        {"clear", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(arg, BuiltIns::List, vm, err)) {
                lv->clear();
                return nullptr;
            } else if (auto dv = get_subtype_value<DictValue>(arg, BuiltIns::Dict, vm, err)) {
                dv->clear();
                return nullptr;
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"close", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(opcode::is_type_eq_or_subtype(args[0].value->get_type(), BuiltIns::File));
            return MSFile::close(vm, args[0].value, err);
        }},
        {"copy", [](Interpreter* vm, CallFrame* cf, Value*&) -> Value* {
            auto arg = cf->get_arg("obj");
            assert(arg && "mssing arg?");
            return arg->clone();
        }},
        {"cos", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::cos(cf->get_args()[0].value->as_float()));
        }},
        {"count", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(arg, BuiltIns::List, vm, err)) {
                return List::count(vm, arg, cf->get_arg("val"), err);
            } else if (auto stv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err)) {
                return String::count(vm, arg, cf->get_arg("sub"), err);
            } else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
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
        {"delete", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            auto arg = cf->get_arg("this");
            if (isa<ListValue>(arg)) {
                assert(cf->get_args().size() == 2);
                return List::List_delete(vm, arg, cf->get_arg("index"), err);
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"Dict", [](Interpreter *vm, CallFrame* cf, Value *&err) -> Value *{
            assert(cf->get_args().size() == 2 || cf->get_args().size() == 1);
            auto ths = cf->get_arg("this");
            Value *dv = nullptr;
            if (cf->get_args().size() == 2)
                dv = Dict::Dict(vm, cf->get_arg("iterable"), err);
            else
                dv = new DictValue();
            if (ths->get_type() == BuiltIns::Dict) {
                return dv;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::Dict)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, dv);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"divmod", [](Interpreter* vm, CallFrame* cf, Value *&err) {
            auto args = cf->get_args();
            assert(args.size() == 2);
            return divmod(vm, cf->get_arg("x"), cf->get_arg("y"), err);
        }},
        {"Float", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            assert(cf->get_args().size() == 2);
            auto ths = cf->get_arg("this");
            auto fv = Float(vm, cf->get_arg("v"), err);
            if (ths->get_type() == BuiltIns::Float) {
                return fv;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::Float)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, fv);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"get", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            auto arg = cf->get_arg("this");
            if (isa<DictValue>(arg)) {
                assert(cf->get_args().size() == 3);
                return Dict::get(vm, arg, cf->get_arg("key"), cf->get_arg("def_val"), err);
            } else {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
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
        {"globals", [](Interpreter* vm, CallFrame* cf, Value*&) {
            auto args = cf->get_args();
            assert(args.size() == 0);
            return symbols(vm, vm->get_global_frame());
        }},
        {"hasattr", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value *{
            (void)err;
            assert(cf->get_args().size() == 2);
            auto name = dyn_cast<StringValue>(cf->get_arg("name"));
            assert(name);
            auto obj = cf->get_arg("obj");
            assert(name);
            return BoolValue::get(obj->has_attr(name->get_value(), vm));
        }},
        {"hash", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return IntValue::get(moss::hash(cf->get_arg("obj"), vm));
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
            return IntValue::get(reinterpret_cast<opcode::IntConst>(cf->get_args()[0].value));
        }},
        {"index", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_arg("this");
            /*if (auto lv = get_subtype_value<ListValue>(arg, BuiltIns::List, vm, err)) {
                return IntValue::get(lv->get_vals().size());
            } else */if (auto stv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err)) {
                return String::index(vm, arg, cf->get_arg("value"), err);
            }/* else if (auto dv = get_subtype_value<DictValue>(arg, BuiltIns::Dict, vm, err)) {
                return IntValue::get(dv->size());
            }*/ else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"input", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 1);
            return input(vm, cf->get_args()[0].value, err);
        }},
        {"insert", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(arg, BuiltIns::List, vm, err)) {
                return List::insert(vm, arg, cf->get_arg("index"), cf->get_arg("value"), err);
            }/* else if (auto stv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err)) {
                return String::index(vm, arg, cf->get_arg("value"), err);
            } else if (auto dv = get_subtype_value<DictValue>(arg, BuiltIns::Dict, vm, err)) {
                return IntValue::get(dv->size());
            }*/ else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"Int", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert((args.size() == 2 || args.size() == 3));
            auto ths = cf->get_arg("this");
            auto iv = Int(vm, cf->get_arg("v"), cf->get_arg("base"), err);
            if (ths->get_type() == BuiltIns::Int) {
                return iv;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::Int)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, iv);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"isinstance", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            auto obj = cf->get_arg("obj");
            auto types = cf->get_arg("types");
            return isinstance(vm, obj, types, err);
        }},
        {"issubclass", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            (void)err;
            assert(cf->get_args().size() == 2);
            auto cls = cf->get_arg("cls");
            auto types = cf->get_arg("types");
            return issubclass(vm, cls, types, err);
        }},
        {"isalpha", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswalpha), err);
        }},
        {"isalnum", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswalnum), err);
        }},
        {"isdigit", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswdigit), err);
        }},
        {"islower", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswlower), err);
        }},
        {"isprintable", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswprint), err);
        }},
        {"isspace", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(std::iswspace), err);
        }},
        {"isupper", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            return String_isfun(vm, cf, static_cast<int(*)(std::wint_t)>(iswupper), err);
        }},
        {"length", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto arg = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(arg, BuiltIns::List, vm, err)) {
                return IntValue::get(lv->get_vals().size());
            } else if (auto stv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err)) {
                return IntValue::get(stv->get_value().length());
            } else if (auto dv = get_subtype_value<DictValue>(arg, BuiltIns::Dict, vm, err)) {
                return IntValue::get(dv->size());
            } else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"List", [](Interpreter *vm, CallFrame* cf, Value *&err) -> Value *{
            if (cf->get_args().size() == 1)
                return new ListValue();
            assert(cf->get_args().size() == 2);
            auto ths = cf->get_arg("this");
            auto lv = List::List(vm, cf->get_arg("iterable"), err);
            if (ths->get_type() == BuiltIns::List) {
                return lv;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::List)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, lv);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"locals", [](Interpreter* vm, CallFrame* cf, Value*&) {
            auto args = cf->get_args();
            assert(args.size() == 0);
            return symbols(vm, vm->get_top_frame());
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
            return FloatValue::get(std::log(xf->as_float()) / std::log(basef->as_float()));
        }},
        {"lower", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_args()[0].value;
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            return String::lower(vm, sv, err);
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
            return IntValue::get(ai->get_value() << counti->get_value());
        }},
        {"multi_replace", [](Interpreter* vm, CallFrame* cf, Value *&err) -> Value* {
            auto args = cf->get_args();
            auto ths = cf->get_arg("this");
            if (auto sv = get_subtype_value<StringValue>(ths, BuiltIns::String, vm, err)) {
                assert(args.size() == 2);
                return String::multi_replace(vm, sv, cf->get_arg("mapping"), err);
            } else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, args[1].value->get_type()->get_name().c_str()));
                return nullptr;
            }
        }},
        {"NilType", [](Interpreter*, CallFrame*, Value*&) {
            return NilValue::Nil();
        }},
        {"Note", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            assert(cf->get_args().size() == 3);
            auto ths = cf->get_arg("this");
            auto nn = Note(vm, cf->get_arg("format"), cf->get_arg("value"));
            if (ths->get_type() == BuiltIns::Note) {
                return nn;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::Note)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, nn);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
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
            assert(opcode::is_type_eq_or_subtype(args[0].value->get_type(), BuiltIns::File));
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
            return IntValue::get(static_cast<opcode::IntConst>(sv[0]));
        }},
        {"pop", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            auto args = cf->get_args();
            auto ths = cf->get_arg("this");
            if (auto lv = get_subtype_value<ListValue>(ths, BuiltIns::List, vm, err)) {
                return List::pop(vm, lv, cf->get_arg("index"), err);
            } else if (auto dv = get_subtype_value<DictValue>(ths, BuiltIns::Dict, vm, err)) {
                return Dict::pop(vm, dv, cf->get_arg("key"), cf->get_arg("def_val"), err);
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
        {"rand_float", [](Interpreter* vm, CallFrame* cf, Value *&err) {
            assert(cf->get_args().size() == 2);
            return rand_float(vm, cf->get_arg("min"), cf->get_arg("max"), err);
        }},
        {"rand_int", [](Interpreter* vm, CallFrame* cf, Value *&err) {
            assert(cf->get_args().size() == 2);
            return rand_int(vm, cf->get_arg("min"), cf->get_arg("max"), err);
        }},
        {"readlines", [](Interpreter* vm, CallFrame* cf, Value*& err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            assert(opcode::is_type_eq_or_subtype(args[0].value->get_type(), BuiltIns::File));
            return MSFile::readlines(vm, args[0].value, err);
        }},
        {"replace", [](Interpreter* vm, CallFrame* cf, Value *&err) -> Value* {
            auto args = cf->get_args();
            auto ths = cf->get_arg("this");
            if (auto sv = get_subtype_value<StringValue>(ths, BuiltIns::String, vm, err)) {
                assert(args.size() == 4);
                return String::replace(vm, sv, cf->get_arg("target"), cf->get_arg("value"), cf->get_arg("count"), err);
            } else {
                if (!err)
                    err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, args[1].value->get_type()->get_name().c_str()));
                return nullptr;
            }
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
            return IntValue::get(ai->get_value() >> counti->get_value());
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
            return FloatValue::get(std::sin(cf->get_args()[0].value->as_float()));
        }},
        {"sleep", [](Interpreter*, CallFrame* cf, Value*&) {
            auto seconds = static_cast<opcode::IntConst>(cf->get_args()[0].value->as_float() * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
            return BuiltIns::Nil;
        }},
        {"split", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 3);
            auto arg = cf->get_arg("this");
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            return String::split(vm, arg, cf->get_arg("sep"), cf->get_arg("maxsplit"), err);
        }},
        {"String", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 2);
            auto ns = String::String_constructor(vm, cf->get_arg("v"), err);
            auto ths = cf->get_arg("this");
            if (ths->get_type() == BuiltIns::String) {
                return ns;
            }
            if (opcode::is_type_eq_or_subtype(ths->get_type(), BuiltIns::String)) {
                ths->set_attr(known_names::BUILT_IN_EXT_VALUE, ns);
                return ths;
            }
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
            return nullptr;
        }},
        {"strip", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_args()[0].value;
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            auto strv = sv->get_value();
            utils::trim(strv);
            return StringValue::get(strv);
        }},
        {"swapcase", [](Interpreter *vm, CallFrame *cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_arg("this");
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            return String::swapcase(vm, sv, err);
        }},
        {"tan", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::tan(cf->get_args()[0].value->as_float()));
        }},
        {"type", [](Interpreter* vm, CallFrame* cf, Value*& err)  {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            return cf->get_args()[0].value->get_type();
        }},
        {"upper", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_args()[0].value;
            auto sv = get_subtype_value<StringValue>(arg, BuiltIns::String, vm, err);
            if (err)
                return nullptr;
            if (!sv) {
                err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, arg->get_type()->get_name().c_str()));
                return nullptr;
            }
            return String::upper(vm, sv, err);
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
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> subprocess_registry = subprocess::get_registry();
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> cffi_registry = cffi::get_registry();
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> sys_registry = sys::get_registry();
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> time_registry = time::get_registry();
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> inspect_registry = inspect::get_registry();
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> empty_registry{};

    // Based on module name return correct function registry
    if (module_name == "libms")
        return libms_registry;
    else if (module_name == "subprocess")
        return subprocess_registry;
    else if (module_name == "cffi")
        return cffi_registry;
    else if (module_name == "sys")
        return sys_registry;
    else if (module_name == "time")
        return time_registry;
    else if (module_name == "inspect")
        return inspect_registry;
    else {
        // We want to raise exception, not to assert, this will make it so
        // that the "internal" function will not be found.
        // It could also be modified so that the exception reads specification
        // that the module registry was not found, but it is pretty much the
        // same. 
        return empty_registry;
    }
};

void mslib::dispatch(Interpreter *vm, ustring module_name, ustring name, Value *&err) {
    auto cf = vm->get_call_frame();

    const auto& registry = FunctionRegistry::get_registry(module_name);
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

void mslib::call_const_initializer(ustring module_name, Interpreter *vm) {
    if (module_name == "sys")
        sys::init_constants(vm);
}