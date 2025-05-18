/// 
/// \file method_analyzer.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Analysis of methods
/// 

#ifndef _METHOD_ANALYZER_HPP_
#define _METHOD_ANALYZER_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"
#include "logging.hpp"

namespace moss {
namespace ir {

/// Does analysis of methods
/// - Marks class methods
/// - Checks returns in constructors
class MethodAnalyzer : public IRVisitor {
private:
    bool in_constructor;

    void check_constructor(class Function &method, ustring class_name);
public:
    MethodAnalyzer(Parser &parser) : IRVisitor(parser) {}
    
    virtual void visit(class Class &cls) override;
    virtual void visit(class Function &fun) override;
    virtual void visit(class Return &ret) override;
};

}
}

#endif//_METHOD_ANALYZER_HPP_