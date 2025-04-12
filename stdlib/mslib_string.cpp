#include "mslib_string.hpp"
#include <algorithm>
#include <cstdlib>

using namespace moss;
using namespace mslib;

Value *String::capitalize(Interpreter *vm, Value * ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring res = strv->get_value();
    res[0] = std::toupper(res[0]);
    return new StringValue(res);
}

Value *String::upper(Interpreter *vm, Value * ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::toupper);
    return new StringValue(res);
}

Value *String::lower(Interpreter *vm, Value * ths, Value *&err) {
    auto strv = dyn_cast<StringValue>(ths);
    assert(strv && "not string");
    ustring text = strv->get_value();
    ustring res(text.length(), '\0');
    std::transform(text.begin(), text.end(), res.begin(), ::tolower);
    return new StringValue(res);
}