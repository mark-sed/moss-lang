#include "register_reuse_pass.hpp"

using namespace moss;
using namespace opcode;

void RegisterReusePass::run(BCBlob *bcb) {
    LOGMAX("Running register reuse pass on " << bcb->get_debug_name());
}