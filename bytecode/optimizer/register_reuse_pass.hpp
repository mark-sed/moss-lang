///
/// \file register_reuse_pass.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode optimization pass for reuse of registers.
///

#ifndef _REGISTER_REUSE_PASS_HPP_
#define _REGISTER_REUSE_PASS_HPP_

#include "bytecode.hpp"
#include "bc_pass.hpp"
#include "bytecode_blob.hpp"

namespace moss {
namespace opcode {

class RegisterReusePass : public BCPass {
public:
    virtual void run(BCBlob *bcb) override;
};

}
}

#endif//_REGISTER_REUSE_PASS_HPP_