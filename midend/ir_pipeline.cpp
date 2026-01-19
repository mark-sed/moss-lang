#include "ir_pipeline.hpp"
#include "analysis/method_analyzer.hpp"
#include "analysis/function_analyzer.hpp"
#include "analysis/expression_analyzer.hpp"
#include "ir.hpp"

using namespace moss;
using namespace ir;

IRPipeline::IRPipeline(Parser &parser) : pm(parser), parser(parser) {
    // Method analyzer
    add_pass(new MethodAnalyzer(parser)); // Method analyzer has to be run before function analyzer (it uses method tag).
    add_pass(new FunctionAnalyzer(parser));
    add_pass(new ExpressionAnalyzer(parser));
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