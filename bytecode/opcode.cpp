#include "opcode.hpp"

using namespace moss;
using namespace moss::opcode;

void End::exec(Interpreter *vm) {
    // TODO
}

void Load::exec(Interpreter *vm) {
    // TODO
}

void StoreName::exec(Interpreter *vm) {
    // TODO
}

void StoreConst::exec(Interpreter *vm) {
    // TODO
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->get_const_pool()->store(dst, new IntValue(val));
}