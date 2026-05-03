#include "register_reuse_pass.hpp"
#include "bytecode_blob.hpp"
#include <unordered_map>

using namespace moss;
using namespace opcode;

bool RegisterReusePass::run(BCBlob *bcb) {
    LOGMAX("Running register reuse pass on " << bcb->get_debug_name());
    // TODO: Keep track of constant registers (which dont change) and if value
    // is needed that is already in some const register, then reuse it.
    // This table has to be reset on merging of CFG (we have to be sure that
    // the value we expect was really assigned).
    // Exceptions should not be an issue as the const register should not be
    // overriten.
    return false;
}