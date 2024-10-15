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

Value *BuiltIns::Type = new ClassValue("Type");

Value *BuiltIns::Int = new ClassValue("Int");
Value *BuiltIns::Float = new ClassValue("Float");
Value *BuiltIns::Bool = new ClassValue("Bool");
Value *BuiltIns::NilType = new ClassValue("NilType");
Value *BuiltIns::String = new ClassValue("String");

Value *BuiltIns::Address = new ClassValue("Address");

Value *BuiltIns::Function = new ClassValue("Function");
Value *BuiltIns::FunctionList = new ClassValue("FunctionList");