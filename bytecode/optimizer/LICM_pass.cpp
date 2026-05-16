#include "LICM_pass.hpp"
#include "bytecode_blob.hpp"

using namespace moss;
using namespace opcode;

bool LICMPass::run(BCBlob *bcb) {
    LOGMAX("Running LICM pass on " << bcb->get_debug_name());
    // All stores into const registers can be hoisted outside of the loop.
    return false;
}