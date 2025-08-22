#include "mslib_string.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>

using namespace moss;
using namespace mslib;

Value *String::String_constructor(Interpreter *vm, Value *v, Value *&err) {
    (void)err;
    if (isa<ObjectValue>(v)) {
        Value *trash_err = nullptr;
        auto rval = mslib::call_type_converter(vm, v, "String", known_names::TO_STRING_METHOD, trash_err);
        if (rval && !trash_err) {
            if (!isa<StringValue>(rval)) {
                rval = new StringValue(rval->as_string());
            }
            return rval;
        }
    }
    return new StringValue(v->as_string());
}

Value *String::capitalize(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring res = strv->get_value();
    res[0] = std::toupper(res[0]);
    return new StringValue(res);
}

Value *String::upper(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::toupper);
    return new StringValue(res);
}

Value *String::lower(Interpreter *vm, Value *ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::tolower);
    return new StringValue(res);
}

Value *String::replace(Interpreter *vm, Value *ths, Value *target, Value *value, Value *count, Value *&err) {
    // TODO: Have empty string replace be in between letters:
    // >>> "hello".replace("", "X")
    // 'XhXeXlXlXoX'
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto targv = dyn_cast<StringValue>(target);
    assert(targv && "not string");
    auto valuev = dyn_cast<StringValue>(value);
    assert(valuev && "no string");
    auto countv = dyn_cast<IntValue>(count);
    assert(countv && "not int");
    return new StringValue(utils::replace_n(strv->get_value(), targv->get_value(), valuev->get_value(), countv->get_value()));
}

Value *String::multi_replace(Interpreter *vm, Value *ths, Value *mappings, Value *&err) {
    // TODO: Have empty string replace be in between letters:
    // >>> "hello".replace("", "X")
    // 'XhXeXlXlXoX'
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    auto mapgsv = dyn_cast<ListValue>(mappings);
    assert(mapgsv && "not string");
    auto result = strv->get_value();
    for (auto elem: mapgsv->get_vals()) {
        auto elemv = dyn_cast<ListValue>(elem);
        bool correct_elem = true;
        if (!elemv)
            correct_elem = false;
        else if (elemv->get_vals().size() != 2)
            correct_elem = false;
        else if (!isa<StringValue>(elemv->get_vals()[0]) || !isa<StringValue>(elemv->get_vals()[1]))
            correct_elem = false;
        if (!correct_elem) {
            err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_MULTI_REPLACE_ELEM));
            return nullptr;
        }
        auto target = dyn_cast<StringValue>(elemv->get_vals()[0]);
        auto value = dyn_cast<StringValue>(elemv->get_vals()[1]);
        utils::replace_in_n(result, target->get_value(), value->get_value());
    }
    return new StringValue(result);
}