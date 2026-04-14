///
/// \file bc_pipeline.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Bytecode optimization pipeline.
///

#ifndef _BC_PIPELINE_HPP_
#define _BC_PIPELINE_HPP_

#include "bc_pass.hpp"
#include "bytecode.hpp"
#include "logging.hpp"
#include <list>

namespace moss {
namespace opcode {

class BCPipeline {
private:
    Bytecode *bc;
    std::list<BCPass *> &pipeline;

public:
    BCPipeline(Bytecode *bc, std::list<BCPass *> &pipeline) : bc(bc), pipeline(pipeline) {}

    void run() {
        LOG1("Running BC optimization pipeline");
        auto mod_blob = BCBlob::parse_bc(*bc);
        // outs << *mod_blob << "\n---Inner:---\n";
        // 
        // for (auto b: mod_blob->get_inner_blobs()) {
        //     outs << *b << "\n---\n";
        // }
    }
};

extern std::list<BCPass *> O1Pipeline;

}
}

#endif//_BC_PIPELINE_HPP_