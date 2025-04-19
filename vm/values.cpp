#include "values.hpp"
#include "builtins.hpp"
#include "logging.hpp"
#include "mslib.hpp"
#include <new>

using namespace moss;

// Forward declaration for clang to match the overloaded delete call
void operator delete(void* ptr, std::size_t size) noexcept;

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

opcode::IntConst moss::hash(Value *v, Interpreter *vm) {
    if (!v->is_hashable())
        opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_HASHABLE, v->get_type()->get_name().c_str())));
    if (auto obj = dyn_cast<ObjectValue>(v)) {
        return opcode::hash_obj(obj, vm);
    }
    return v->hash();
}

Value::Value(TypeKind kind, ustring name, Value *type, MemoryPool *attrs) 
        : marked(false), kind(kind), type(type), name(name), 
          attrs(attrs), annotations{} {
}

Value::~Value() {
    // Values will be deleted by gc
    //if (attrs)
    //    delete attrs;
}

Value *Value::iter(Interpreter *vm) {
    assert(!isa<ObjectValue>(this) && "object value should be handled in caller");
    opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_ITERABLE_TYPE, this->get_type()->get_name().c_str())));
    return nullptr;
}

Value *Value::next(Interpreter *vm) {
    assert(!isa<ObjectValue>(this) && "object value should be handled in caller");
    opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::NOT_ITERABLE_TYPE, this->get_type()->get_name().c_str())));
    return nullptr;
}

void Value::set_subsc(Interpreter *vm, Value *key, Value *val) {
    (void)key;
    (void)val;
    assert(!isa<ObjectValue>(this) && "object value should be handled in caller");
    opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::TYPE_NOT_SUBSCRIPT, this->get_type()->get_name().c_str())));
}

Value *Value::get_attr(ustring name, Interpreter *caller_vm) {
    if (!attrs) return nullptr;
    return attrs->load_name(name, caller_vm);
}

Value *SuperValue::get_attr(ustring name, Interpreter *caller_vm) {
    auto type_v = instance->get_type();
    auto type = dyn_cast<ClassValue>(type_v);
    assert(type && "Instance is not an object");
    for (auto parent: type->get_all_supers()) {
        auto v = parent->get_attr(name, caller_vm);
        if (v)
            return v;
    }
    return nullptr;
}

void Value::set_attrs(MemoryPool *p) {
    assert(this->is_modifiable() && "Setting attribute for not-modifiable value");
    this->attrs = p;
}

void Value::copy_attrs(MemoryPool *p) {
    assert(this->is_modifiable() && "Setting attribute for non-modifiable value");
    this->attrs = p->clone();
}

void Value::set_attr(ustring name, Value *v, bool internal_access) {
    assert((this->is_modifiable() || internal_access) && "Setting attribute for non-modifiable value");
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

Value *DictValue::next(Interpreter *vm) {
    (void)vm;
    if (this->iterator == this->vals.end()) {
        opcode::raise(mslib::create_stop_iteration());
    }
    auto item = iterator->second;
    auto key = item[keys_iterator].first;
    auto val = item[keys_iterator].second;
    ++keys_iterator;
    if (keys_iterator >= item.size()) {
        this->iterator++;
        this->keys_iterator = 0;
    }
    std::vector<Value *> lst_vals{key, val};
    return new ListValue(lst_vals);
}

/*void StringValue::set_subsc(Interpreter *vm, Value *key, Value *val) {
    assert(key->get_type() != BuiltIns::Range && "TODO: Implement range subsc set");
    auto key_int = dyn_cast<IntValue>(key);
    if (!key_int)
        opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::STR_INDEX_NOT_INT_OR_RANGE, key->get_type()->get_name().c_str())));
    auto index = key_int->get_value();
    if ((index < 0 && -1*index > static_cast<opcode::IntConst>(this->value.length())) || (index >= 0 && index >= static_cast<opcode::IntConst>(this->value.length()))) {
        opcode::raise(mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, name.c_str(), index)));
    }
    auto val_str = val->as_string();
    if (index >= 0)
        this->value.replace(key_int->get_value(), 1, val_str);
    else
        this->value.replace(this->value.length() + key_int->get_value(), 1, val_str);
}*/

void ListValue::set_subsc(Interpreter *vm, Value *key, Value *val) {
    assert(key->get_type() != BuiltIns::Range && "TODO: Implement range subsc set");
    auto key_int = dyn_cast<IntValue>(key);
    if (!key_int)
        opcode::raise(mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::LIST_INDEX_NOT_INT_OR_RANGE, key->get_type()->get_name().c_str())));
    auto index = key_int->get_value();
    if ((index < 0 && -1*index > static_cast<opcode::IntConst>(this->vals.size())) || (index >= 0 && index >= static_cast<opcode::IntConst>(this->vals.size()))) {
        opcode::raise(mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, name.c_str(), index)));
    } 
    if (index >= 0)
        this->vals[key_int->get_value()] = val;
    else
    this->vals[this->vals.size() + key_int->get_value()] = val;
}

void DictValue::set_subsc(Interpreter *vm, Value *key, Value *val) {
    this->push(key, val, vm);
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

void DictValue::push(Value *k, Value *v, Interpreter *vm) {
    LOGMAX("Pushing a dict item [" << *k << ": " << *v << "]");
    auto hash_key = moss::hash(k, vm);
    LOGMAX("Got hash: " << hash_key);
    auto value_at_k = vals.find(hash_key);
    if (value_at_k != vals.end()) {
        LOGMAX("Clash of hashes - appending");
        bool contains = false;
        auto vak = value_at_k->second;
        for (size_t vindex = 0; vindex < vak.size(); ++vindex) {
            if (opcode::eq(vak[vindex].first, k, vm)) {
                LOGMAX("Overriding index " << vindex << " with " << *vals[hash_key][vindex].first << ": " << *vals[hash_key][vindex].second);
                contains = true;
                vals[hash_key][vindex] = std::make_pair(k, v);
                break;
            }
        }
        if (!contains) {
            LOGMAX("New key, appending");
            vals[hash_key].push_back(std::make_pair(k, v));
        }
    } else {
        LOGMAX("No clash creating a new item");
        std::vector<std::pair<Value *, Value *>> item{std::make_pair(k, v)};
        vals[hash_key] = item;
    }
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
    if(BuiltIns::List->get_attrs())
        this->attrs = BuiltIns::List->get_attrs()->clone();
}
ListValue::ListValue() : Value(ClassType, "List", BuiltIns::List), vals() {
    if(BuiltIns::List->get_attrs())
        this->attrs = BuiltIns::List->get_attrs()->clone();
}

StringValue::StringValue(opcode::StringConst value) : Value(ClassType, "String", BuiltIns::String), value(value) {
    if(BuiltIns::String->get_attrs())
        this->attrs = BuiltIns::String->get_attrs()->clone();
}

NoteValue::NoteValue(opcode::StringConst format, StringValue *value) 
        : Value(ClassType, "Note", BuiltIns::Note), format(format), value(value) {
    if(BuiltIns::Note->get_attrs())
        this->attrs = BuiltIns::Note->get_attrs()->clone();
    set_attr("format", new StringValue(format), true);
    set_attr("value", value, true);
}

BoolValue::BoolValue(opcode::BoolConst value) : Value(ClassType, "Bool", BuiltIns::Bool), value(value) {
    if(BuiltIns::Bool->get_attrs())
        this->attrs = BuiltIns::Bool->get_attrs()->clone();
}

FloatValue::FloatValue(opcode::FloatConst value) : Value(ClassType, "Float", BuiltIns::Float), value(value) {
    if(BuiltIns::Float->get_attrs())
        this->attrs = BuiltIns::Float->get_attrs()->clone();
}

IntValue::IntValue(opcode::IntConst value) : Value(ClassType, "Int", BuiltIns::Int), value(value) {
    if(BuiltIns::Int->get_attrs())
        this->attrs = BuiltIns::Int->get_attrs()->clone();
}

DictValue::DictValue(ListValue *keys, ListValue *values, Interpreter *vm)
        : Value(ClassType, "Dict", BuiltIns::Dict) {
    if(BuiltIns::Dict->get_attrs())
        this->attrs = BuiltIns::Dict->get_attrs()->clone();
    LOGMAX("Creating dict");
    auto kvs = keys->get_vals();
    auto vvs = values->get_vals();
    assert(kvs.size() == vvs.size() && "key size does not match value size");
    for (size_t i = 0; i < kvs.size(); ++i) {
        auto k = kvs[i];
        auto v = vvs[i];
        push(k, v, vm);
    }
}
DictValue::DictValue(std::map<opcode::IntConst, std::vector<std::pair<Value *, Value *>>> vals)
        : Value(ClassType, "Dict", BuiltIns::Dict), vals(vals) {
    if(BuiltIns::Dict->get_attrs())
        this->attrs = BuiltIns::Dict->get_attrs()->clone();
}
DictValue::DictValue() : Value(ClassType, "Dict", BuiltIns::Dict) {
    if(BuiltIns::Dict->get_attrs())
        this->attrs = BuiltIns::Dict->get_attrs()->clone();
}