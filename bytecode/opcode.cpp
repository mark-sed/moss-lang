#include "opcode.hpp"

#include <iostream>

using namespace moss;
using namespace moss::opcode;

void End::exec(Interpreter *vm) {
    // TODO
}

void Load::exec(Interpreter *vm) {
    auto *v = vm->get_reg_pool()->load_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->get_reg_pool()->store(this->dst, v);
}

void StoreName::exec(Interpreter *vm) {
    vm->get_reg_pool()->store_name(dst, name);
}

void StoreConst::exec(Interpreter *vm) {
    auto c = vm->get_const_pool()->load(csrc);
    c->inc_refs();
    vm->get_reg_pool()->store(dst, c);
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->get_const_pool()->store(dst, new IntValue(val));
}