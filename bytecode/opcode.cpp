#include "opcode.hpp"
#include "errors.hpp"
#include <sstream>
#include <cmath>

#include <iostream>

using namespace moss;
using namespace moss::opcode;

std::string OpCode::err_mgs(std::string msg, Interpreter *vm) {
    std::stringstream ss;
    ss << vm->get_bci() << "\t" << *this << " :: " << msg;
    return ss.str();
}

/*
When loading unknown value it will be nil, so it cannot be nullptr
void OpCode::check_load(Value *v, Interpreter *vm) {
    if (v) return;
    auto msg = err_mgs("Loading value from non-existent register", vm);
    error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
}*/

/**
 * True iff one of the values is a float and the other is float or int
 */
static bool is_float_expr(Value *v1, Value *v2) {
    return (isa<FloatValue>(v1) && (isa<FloatValue>(v2) || isa<IntValue>(v2))) || 
        (isa<FloatValue>(v2) && (isa<FloatValue>(v1) || isa<IntValue>(v1)));
}

/**
 * True iff both values are ints
 */
static bool is_int_expr(Value *v1, Value *v2) {
    return isa<IntValue>(v1) && isa<IntValue>(v2);
}

void End::exec(Interpreter *vm) {
    // No op
}

void Load::exec(Interpreter *vm) {
    auto *v = vm->load_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->store(this->dst, v);
}

void LoadAttr::exec(Interpreter *vm) {
    auto *v = vm->load(this->src);
    auto attr = v->get_attr(this->name);
    assert(attr && "TODO: Nonexistent attr raise exception");
    attr->inc_refs();
    vm->store(this->dst, attr);
}

void LoadGlobal::exec(Interpreter *vm) {
    auto *v = vm->load_global_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->store(this->dst, v);
}

void LoadNonLoc::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Store::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    v->inc_refs();
    vm->store(this->dst, v);
}

void StoreName::exec(Interpreter *vm) {
    vm->store_name(dst, name);
}

void StoreConst::exec(Interpreter *vm) {
    auto c = vm->load_const(csrc);
    c->inc_refs();
    vm->store(dst, c);
}

void StoreAddr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void StoreAttr::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    auto *v = vm->load(this->src);
    v->inc_refs();
    dstobj->set_attr(this->name, v);
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->store_const(dst, new IntValue(val));
}

void StoreFloatConst::exec(Interpreter *vm) {
    vm->store_const(dst, new FloatValue(val));
}

void StoreBoolConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new BoolValue(val));
}

void StoreStrConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new StringValue(val));
}

void StoreNilConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new NilValue());
}

void Jmp::exec(Interpreter *vm) {
    vm->set_bci(this->addr);
}

void JmpIfTrue::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

    if (bc->get_value())
        vm->set_bci(this->addr);
}

void JmpIfFalse::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

    if (!bc->get_value())
        vm->set_bci(this->addr);
}

void Call::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Return::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ReturnConst::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ReturnAddr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushArg::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushConstArg::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushAddrArg::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Import::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ImportAll::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushParent::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void CreateObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PromoteObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildClass::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Copy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void DeepCopy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void CreateAnnt::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Annotate::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Output::exec(Interpreter *vm) {
    // FIXME: this is just a placeholder
    auto *v = vm->load(src);
    
    std::cout << v->as_string();
}

static Value *concat(Value *s1, Value *s2, Interpreter *vm) {
    assert(s1 && "Value or nil should have been loaded");
    assert(s2 && "Value or nil should have been loaded");

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

    return new StringValue(s1_str + s2_str);
}

void Concat::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat2::exec(Interpreter *vm) {
    auto res = concat(vm->load_const(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat3::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load_const(src2), vm);
    vm->store(dst, res);
}

static Value *exp(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(static_cast<long>(std::pow(static_cast<double>(i1->get_value()), static_cast<double>(i2->get_value()))));
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(std::pow(s1->as_float(), s2->as_float()));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Exp::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Exp2::exec(Interpreter *vm) {
    auto res = exp(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Exp3::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *add(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() + i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() + s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Add::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Add2::exec(Interpreter *vm) {
    auto res = add(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Add3::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *sub(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() - i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() - s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Sub::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Sub2::exec(Interpreter *vm) {
    auto res = sub(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Sub3::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

/*

void ::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

*/