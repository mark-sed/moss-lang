#include "ir_visitor.hpp"
#include "ir.hpp"
#include "logging.hpp"
#include "parser.hpp"

using namespace moss;
using namespace ir;

IRVisitor::IRVisitor(Parser &parser) : parser(parser) {}

void PassManager::visit(Module &mod) {
    for (auto p: passes) {
        mod.accept(*p);
    }

    for (auto i : mod.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Class &cls) {
    for (auto p: passes) {
        cls.accept(*p);
    }

    for (auto i : cls.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Space &spc) {
    for (auto p: passes) {
        spc.accept(*p);
    }

    for (auto i : spc.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Function &fun) {
    for (auto p: passes) {
        fun.accept(*p);
    }

    for (auto i : fun.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Else &els) {
    for (auto i : els.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(If &i) {
    for (auto j : i.get_body()) {
        j->accept(*this);
    }
    auto els = i.get_else();
    if (els)
        els->accept(*this);
}

void PassManager::visit(Return &ret) {
    for (auto p: passes) {
        ret.accept(*p);
    }
}

void PassManager::visit(Lambda &fun) {
    // TODO
}

void PassManager::add_pass(IRVisitor *p) { 
    passes.push_back(p);
}