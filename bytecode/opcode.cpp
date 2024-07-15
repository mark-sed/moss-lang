#include "opcode.hpp"
#include "errors.hpp"
#include <sstream>

#include <iostream>

using namespace moss;
using namespace moss::opcode;

std::string OpCode::err_mgs(std::string msg, Interpreter *vm) {
    std::stringstream ss;
    ss << vm->get_bci() << "\t" << *this << " :: " << msg;
    return ss.str();
}

void OpCode::check_load(Value *v, Interpreter *vm) {
    if (v) return;
    auto msg = err_mgs("Loading value from non-existent register", vm);
    error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
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
    check_load(v, vm);
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
    check_load(v, vm);
    v->inc_refs();
    vm->store(this->dst, v);
}

void StoreName::exec(Interpreter *vm) {
    vm->store_name(dst, name);
}

void StoreConst::exec(Interpreter *vm) {
    auto c = vm->load_const(csrc);
    check_load(c, vm);
    c->inc_refs();
    vm->store(dst, c);
}

void StoreAddr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void StoreAttr::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    check_load(dstobj, vm);
    auto *v = vm->load(this->src);
    check_load(v, vm);
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
    check_load(v, vm);
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
    check_load(v, vm);
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
    check_load(v, vm);
    
    std::cout << v->as_string();
}

void Concat::exec(Interpreter *vm) {
    auto *s1 = vm->load(src1);
    check_load(s1, vm);
    auto *s2 = vm->load(src2);
    check_load(s2, vm);

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

    StringValue *ccat = new StringValue(s1_str + s2_str);

    vm->store(dst, ccat);
}

void Concat2::exec(Interpreter *vm) {
    auto *s1 = vm->load_const(src1);
    check_load(s1, vm);
    auto *s2 = vm->load(src2);
    check_load(s2, vm);

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

    StringValue *ccat = new StringValue(s1_str + s2_str);

    vm->store(dst, ccat);
}

void Concat3::exec(Interpreter *vm) {
    auto *s1 = vm->load(src1);
    check_load(s1, vm);
    auto *s2 = vm->load_const(src2);
    check_load(s2, vm);

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

    StringValue *ccat = new StringValue(s1_str + s2_str);

    vm->store(dst, ccat);
}

/*

void ::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

*/