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
    bool has_main;
    void check_arguments(const std::vector<ir::Argument *> &args, ustring fname);
    void check_annotated_fun(class IR &fun, const std::vector<ir::Argument *> &args);
public:
    FunctionAnalyzer(Parser &parser) : IRVisitor(parser), has_main(false) {}
    
    virtual IR *visit(class Function &fun) override;
    virtual IR *visit(class Lambda &lf) override;
    virtual IR *visit(class Return &ret) override;
};

}
}

#endif//_FUNCTION_ANALYZER_HPP_