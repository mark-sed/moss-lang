/// 
/// \file expression_analyzer.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Analysis of expression.
/// 

#ifndef _EXPRESSION_ANALYZER_HPP_
#define _EXPRESSION_ANALYZER_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"
#include "logging.hpp"
#include <unordered_set>

namespace moss {
namespace ir {

/// Does analysis of expressions.
class ExpressionAnalyzer : public IRVisitor {
private:
    void check_call_arg(Expression *arg, std::unordered_set<ustring> &named_args);
public:
    ExpressionAnalyzer(Parser &parser) : IRVisitor(parser) {}
    
    virtual void visit(class BinaryExpr &be) override;
    virtual void visit(class UnaryExpr &be) override;
    virtual void visit(class Call &c) override;
};

}
}

#endif//_EXPRESSION_ANALYZER_HPP_