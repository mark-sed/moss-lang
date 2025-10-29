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
    void check_arguments(const std::vector<ir::Argument *> &args, ustring fname);
    void check_annotated_fun(class IR &fun, const std::vector<ir::Argument *> &args);
public:
    FunctionAnalyzer(Parser &parser) : IRVisitor(parser) {}
    
    virtual void visit(class Function &fun) override;
    virtual void visit(class Lambda &lf) override;
};

}
}

#endif//_FUNCTION_ANALYZER_HPP_