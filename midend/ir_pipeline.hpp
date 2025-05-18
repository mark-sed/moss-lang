/// 
/// \file ir_pipeline.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Constructs and handles pass manager passes
/// 

#ifndef _IR_PIPELINE_HPP_
#define _IR_PIPELINE_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"
#include <list>

namespace moss {
namespace ir {

/// Holds IRVisitors (passes), which will be applied to IR
class IRPipeline {
private:
    std::list<IRVisitor *> pass_instances;
    PassManager pm;
    Parser &parser;
public:
    /// Constructs new default pipeline 
    IRPipeline(Parser &parser);
    ~IRPipeline();

    /// Runs the whole pipeline
    ir::IR *run(ir::IR *decl);
    
    /// Registers a new visitor pass in the pass pipeline (at the current end)
    void add_pass(IRVisitor *p);

    PassManager &get_pm() { return this->pm; }
};

}
}

#endif//_IR_PIPELINE_HPP_