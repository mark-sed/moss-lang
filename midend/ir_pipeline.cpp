#include "ir_pipeline.hpp"
#include "method_analyzer.hpp"

using namespace moss;
using namespace ir;

IRPipeline::IRPipeline() {
    // Method analyzer
    add_class_pass(new MethodAnalyzer());
}

IRPipeline::~IRPipeline() {
    for (auto p: this->pass_instances) {
        delete p;
    }
}

void IRPipeline::run(ir::IR *decl) {
    decl->accept(pm);
}

void IRPipeline::add_module_pass(IRVisitor *p) { 
    pm.add_module_pass(p);
    this->pass_instances.push_back(p);
}

void IRPipeline::add_space_pass(IRVisitor *p) { 
    pm.add_space_pass(p);
    this->pass_instances.push_back(p);
}

void IRPipeline::add_class_pass(IRVisitor *p) { 
    pm.add_class_pass(p);
    this->pass_instances.push_back(p);
}

void IRPipeline::add_function_pass(IRVisitor *p) { 
    pm.add_function_pass(p);
    this->pass_instances.push_back(p);
}