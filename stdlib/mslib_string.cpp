#include "mslib_string.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>

using namespace moss;
using namespace mslib;

Value *String::String_constructor(Interpreter *vm, Value *ths, Value *v, Value *&err) {
    (void)ths;
    (void)err;
    if (isa<ObjectValue>(v)) {
        Value *trash_err = nullptr;
        auto rval = mslib::call_type_converter(vm, v, "String", "__String", trash_err);
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