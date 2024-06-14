#include "opcode.hpp"

#include <iostream>

using namespace moss;
using namespace moss::opcode;

void End::exec(Interpreter *vm) {
    // TODO
    assert(false && "TODO: Unimplemented opcode");
}

void Load::exec(Interpreter *vm) {
    auto *v = vm->load_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->store(this->dst, v);
}

void LoadAttr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
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
    assert(false && "TODO: Unimplemented opcode");
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->store_const(dst, new IntValue(val));
}

/*

void ::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

*/