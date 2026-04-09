///
/// \file bc_pass.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode optimization pass.
///

#ifndef _BC_PASS_HPP_
#define _BC_PASS_HPP_

#include "bytecode.hpp"
#include "bytecode_blob.hpp"

namespace moss {
namespace opcode {

class BCPass {
public:
    virtual void run(BCBlob *bcb) = 0;
};

}
}

#endif//_BC_PASS_HPP_