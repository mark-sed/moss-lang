#include "ir_pipeline.hpp"
#include "method_analyzer.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

IRPipeline::IRPipeline(Parser &parser) : pm(parser), parser(parser) {
    // Method analyzer
    add_class_pass(new MethodAnalyzer(parser));
}

IRPipeline::~IRPipeline() {
    for (auto p: this->pass_instances) {
        delete p;
    }
}

ir::IR *IRPipeline::run(ir::IR *decl) {
    try {
        decl->accept(pm);
    } catch (Raise *raise) {
        return raise;
    }
    return nullptr;
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