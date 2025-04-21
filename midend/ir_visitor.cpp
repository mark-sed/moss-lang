#include "ir_visitor.hpp"
#include "ir.hpp"
#include "logging.hpp"

using namespace moss;
using namespace ir;

void PassManager::visit(Module &mod) {
    for (auto i : mod.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Class &cls) {
    for (auto p: class_passes) {
        cls.accept(*p);
    }

    for (auto i : cls.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Space &spc) {
    for (auto i : spc.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Function &fun) {
    for (auto i : fun.get_body()) {
        i->accept(*this);
    }
}

void PassManager::add_module_pass(IRVisitor *p) { 
    module_passes.push_back(p);
}

void PassManager::add_space_pass(IRVisitor *p) { 
    space_passes.push_back(p);
}

void PassManager::add_class_pass(IRVisitor *p) { 
    class_passes.push_back(p);
}

void PassManager::add_function_pass(IRVisitor *p) { 
    function_passes.push_back(p);
}