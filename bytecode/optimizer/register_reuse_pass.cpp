#include "register_reuse_pass.hpp"
#include "bytecode_blob.hpp"
#include <unordered_map>

using namespace moss;
using namespace opcode;

bool RegisterReusePass::run(BCBlob *bcb) {
    LOGMAX("Running register reuse pass on " << bcb->get_debug_name());
    return false;
}