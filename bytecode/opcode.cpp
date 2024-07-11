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
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
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
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
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
    // FIXME:
    assert(dstobj && "TODO: Nonexistent name raise exception");
    auto *v = vm->load(this->src);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
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
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
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
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
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
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    
    std::cout << v->as_string();
}

/*

void ::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

*/