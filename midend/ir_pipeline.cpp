#include "ir_pipeline.hpp"
#include "method_analyzer.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

IRPipeline::IRPipeline(Parser &parser) : pm(parser), parser(parser) {
    // Method analyzer
    add_pass(new MethodAnalyzer(parser));
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

void IRPipeline::add_pass(IRVisitor *p) { 
    pm.add_pass(p);
    this->pass_instances.push_back(p);
}