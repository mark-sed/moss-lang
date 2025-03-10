#include "values.hpp"
#include "builtins.hpp"
#include "logging.hpp"
#include "mslib.hpp"

using namespace moss;

int Value::tab_depth = 0;
size_t Value::allocated_bytes = 0;
size_t Value::next_gc = 1024 * 1024;
std::list<Value *> Value::all_values{};

bool moss::has_methods(Value *v) {
    assert(v->get_kind() != TypeKind::DICT && "TODO: Add dict to this function");
    return isa<ObjectValue>(v) || isa<ClassValue>(v) || isa<IntValue>(v) 
        || isa<FloatValue>(v) || isa<BoolValue>(v) || isa<NilValue>(v)
        || isa<StringValue>(v) || isa<ListValue>(v);
}

Value::Value(TypeKind kind, ustring name, Value *type, MemoryPool *attrs) 
        : marked(false), kind(kind), type(type), name(name), 
          attrs(attrs), annotations{} {
}

Value::~Value() {
    // Values will be deleted by gc
    if (attrs)
        delete attrs;
}

Value *Value::iter(Interpreter *vm) {
    opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_ITERABLE_TYPE, this->get_type()->get_name().c_str())));
    return nullptr;
}

Value *Value::next(Interpreter *vm) {
    opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_ITERABLE_TYPE, this->get_type()->get_name().c_str())));
    return nullptr;
}

Value *Value::get_attr(ustring name, Interpreter *caller_vm) {
    if (!attrs) return nullptr;
    return attrs->load_name(name, caller_vm);
}

void Value::set_attrs(MemoryPool *p) {
    assert(this->is_mutable() && "Setting attribute for immutable value");
    this->attrs = p;
}

void Value::copy_attrs(MemoryPool *p) {
    assert(this->is_mutable() && "Setting attribute for immutable value");
    this->attrs = p->clone();
}

void Value::set_attr(ustring name, Value *v) {
    assert(this->is_mutable() && "Setting attribute for immutable value");
    if (!attrs) {
        this->attrs = new MemoryPool();
    }
    auto reg = attrs->get_free_reg();
    attrs->store(reg, v);
    attrs->store_name(reg, name);
}

void Value::annotate(ustring name, Value *val) {
    assert(!isa<FunValueList>(this) && "Annotating fun list not a function");
    annotations[name] = val;
}

void *Value::operator new(size_t size) {
    Value::allocated_bytes += size;
    if (Value::allocated_bytes > Value::next_gc) {
        LOGMAX("Allocations reached the GC threshold: " << Value::allocated_bytes << "B allocated; " << Value::next_gc << "B is the threshold");
        global_controls::trigger_gc = true;
        Value::next_gc *= global_controls::gc_grow_factor;
        /*if (Value::next_gc > global_controls::max_next_gc) {
            Value::next_gc = global_controls::max_next_gc;
        }*/
        LOGMAX("New gc threshold set to: " << Value::next_gc << "B");
    }
    void *v = ::operator new(size);
    assert(v && "Allocation failed?");
    all_values.push_back(static_cast<Value *>(v));
#ifndef NDEBUG
    if (clopts::stress_test_gc) {
        global_controls::trigger_gc = true;
    }
#endif
    return v;
}

void Value::operator delete(void * p, size_t size) {
    Value::allocated_bytes -= size;
    ::operator delete(p, size);
}

Value *StringValue::next(Interpreter *vm) {
    (void)vm;
    if (this->iterator >= this->value.size()) {
        opcode::raise(mslib::create_stop_iteration());
    }
    auto chr = this->value[iterator];
    this->iterator++;
    return new StringValue(ustring(1, chr));
}

Value *ListValue::next(Interpreter *vm) {
    (void)vm;
    if (this->iterator >= this->vals.size()) {
        opcode::raise(mslib::create_stop_iteration());
    }
    auto val = this->vals[iterator];
    this->iterator++;
    return val;
}

std::ostream& ClassValue::debug(std::ostream& os) const {
    // TODO: Output all needed debug info
    os << "Class " << name;
    bool first = true;
    for (auto s : supers) {
        if (first) {
            os << " : " << s->get_name();
            first = false;
        }
        else {
            os << ", " << s->get_name();
        }
    }
    os << " {";
    if (!attrs || attrs->is_empty_sym_table()) {
        os << "}";
    }
    else {
        attrs->debug_sym_table(os, tab_depth);
        os << "\n" << std::string(tab_depth*2, ' ') << "}";
    }

    return os;
}

std::ostream& ObjectValue::debug(std::ostream& os) const {
    // TODO: Output all needed debug info
    os << "Object : " << type->get_name() << " {"; 
    if (!attrs || attrs->is_empty_sym_table()) {
        os << "}";
    }
    else {
        attrs->debug_sym_table(os, tab_depth);
        os << "\n" << std::string(tab_depth*2, ' ') << "}";
    }

    return os;
}

std::ostream& SpaceValue::debug(std::ostream& os) const {
    // TODO: Output all needed debug info
    os << "Space : " << name << " {"; 
    if (!attrs || attrs->is_empty_sym_table()) {
        os << "}";
    }
    else {
        attrs->debug_sym_table(os, tab_depth);
        os << "\n" << std::string(tab_depth*2, ' ') << "}";
    }

    return os;
}

std::ostream& ModuleValue::debug(std::ostream& os) const {
    // TODO: Output all attributes and so on
    os << "(Module)" << name;
    if (attrs)
        os << ": " << *attrs;
    return os;
}

ModuleValue::~ModuleValue()  {
    delete vm->get_src_file();
    delete vm;
    // Attrs need to be set to nullptr as ~Value will be called, but
    // vm destructor should delete global frame which is attrs
    this->attrs = nullptr;
}

EnumValue::EnumValue(EnumTypeValue *type, ustring name) : Value(ClassType, name, type) {
}

Value *EnumValue::clone() {
    assert(isa<EnumTypeValue>(this->type) && "Incorrect type for enum value");
    return new EnumValue(dyn_cast<EnumTypeValue>(this->type), this->name);
}

std::list<ClassValue *> ClassValue::get_all_supers() {
    std::list<ClassValue *> sups(supers);
    // Append supers of supers
    for (auto s: supers) {
        auto s_sups = s->get_all_supers();
        sups.insert(sups.end(), s_sups.begin(), s_sups.end());
    }
    return sups;
}

ListValue::ListValue(std::vector<Value *> vals) : Value(ClassType, "List", BuiltIns::List), vals(vals) {
    assert(BuiltIns::List->get_attrs() && "no attribs");
    this->attrs = BuiltIns::List->get_attrs()->clone();
}
ListValue::ListValue() : Value(ClassType, "List", BuiltIns::List), vals() {
    assert(BuiltIns::List->get_attrs() && "no attribs");
    this->attrs = BuiltIns::List->get_attrs()->clone();
}
StringValue::StringValue(opcode::StringConst value) : Value(ClassType, "String", BuiltIns::String), value(value) {
    //assert(BuiltIns::String->get_attrs() && "no attribs");
    //this->attrs = BuiltIns::String->get_attrs()->clone();
}
BoolValue::BoolValue(opcode::BoolConst value) : Value(ClassType, "Bool", BuiltIns::Bool), value(value) {
    //assert(BuiltIns::Bool->get_attrs() && "no attribs");
    //this->attrs = BuiltIns::Bool->get_attrs()->clone();
}
FloatValue::FloatValue(opcode::FloatConst value) : Value(ClassType, "Float", BuiltIns::Float), value(value) {
    //assert(BuiltIns::Float->get_attrs() && "no attribs");
    //this->attrs = BuiltIns::Float->get_attrs()->clone();
}
IntValue::IntValue(opcode::IntConst value) : Value(ClassType, "Int", BuiltIns::Int), value(value) {
    //assert(BuiltIns::Int->get_attrs() && "no attribs");
    //this->attrs = BuiltIns::Int->get_attrs()->clone();
}