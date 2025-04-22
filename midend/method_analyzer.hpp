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

class MethodAnalyzer : public IRVisitor {
public:
    MethodAnalyzer(Parser &parser) : IRVisitor(parser) {}
    virtual void visit(class Class &cls) override;
    void check_constructor(class Function &method, ustring class_name);
};

}
}

#endif//_METHOD_ANALYZER_HPP_