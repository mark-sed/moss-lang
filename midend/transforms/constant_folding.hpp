/// 
/// \file constant_folding.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Constant folding pass.
/// 

#ifndef _CONSTANT_FOLDING_HPP_
#define _CONSTANT_FOLDING_HPP_

#include "ir.hpp"
#include "ir_visitor.hpp"

namespace moss {
namespace ir {

/// Does constant folding transformation of IR.
class ConstantFoldingPass : public IRVisitor {
public:
    ConstantFoldingPass(Parser &parser) : IRVisitor(parser) {}
    
    virtual IR *visit(class BinaryExpr &be) override;
    virtual IR *visit(class UnaryExpr &ue) override;
};

}
}

#endif//_CONSTANT_FOLDING_HPP_