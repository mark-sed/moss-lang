/// 
/// \file function_analyzer.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Analysis of functions.
/// 

#ifndef _FUNCTION_ANALYZER_HPP_
#define _FUNCTION_ANALYZER_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"
#include "logging.hpp"

namespace moss {
namespace ir {

/// Does analysis of function.
class FunctionAnalyzer : public IRVisitor {
private:
    void check_arguments(class Function &fun);
public:
    FunctionAnalyzer(Parser &parser) : IRVisitor(parser) {}
    
    virtual void visit(class Function &fun) override;
};

}
}

#endif//_FUNCTION_ANALYZER_HPP_