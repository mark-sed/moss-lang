///
/// \file LICM_pass.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode optimization pass for Loop Invariant Code Motion transforms.
///

#ifndef _LICM_PASS_HPP_
#define _LICM_PASS_HPP_

#include "bytecode.hpp"
#include "bc_pass.hpp"
#include "bytecode_blob.hpp"

namespace moss {
namespace opcode {

class LICMPass : public BCPass {
public:
    virtual bool run(BCBlob *bcb) override;
};

}
}

#endif//_LICM_PASS_HPP_