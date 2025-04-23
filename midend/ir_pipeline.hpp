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

class IRPipeline {
private:
    std::list<IRVisitor *> pass_instances;
    PassManager pm;
    Parser &parser;
public:
    IRPipeline(Parser &parser);
    ~IRPipeline();

    void add_pass(IRVisitor *p);

    PassManager &get_pm() { return this->pm; }
    ir::IR *run(ir::IR *decl);
};

}
}

#endif//_IR_PIPELINE_HPP_