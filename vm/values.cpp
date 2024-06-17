#include "values.hpp"

using namespace moss;

Value *Value::get_attr(ustring name) {
    auto index = this->attrs.find(name);
    if (index != this->attrs.end()) {
        return index->second;
    }
    return nullptr;        
}

void Value::set_attr(ustring name, Value *v) {
    assert(v && "Storing nullptr value");
    this->attrs[name] = v;
}