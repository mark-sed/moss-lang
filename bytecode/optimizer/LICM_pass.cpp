#include "LICM_pass.hpp"
#include "bytecode_blob.hpp"
#include "clopts.hpp"

using namespace moss;
using namespace opcode;

bool LICMPass::run(BCBlob *bcb) {
    if (clopts::opt_no_licm) {
        LOGMAX("Skipping LICM pass (no-licm-pass option set)");
        return false;
    }
    LOGMAX("Running LICM pass on " << bcb->get_debug_name());
    // All stores into const registers can be hoisted outside of the loop.
    bool modified = false;
    // We care only about being in any loop, not how deep. We will hoist all
    // constant registers above the top one. This might cause extra stores in
    // case the inner loop is not run, but will save performance when run more
    // than 1 times.
    int loop_depth = 0;
    Address loop_start_bci = 0;
    std::vector<std::pair<OpCode *, Address>> consts_to_hoist_to;
    for (BCBlobIterator oit = bcb->begin(); oit != bcb->end(); ++oit) {
        auto o = *oit;
        auto bci = oit.index();
        if (loop_depth > 0) {
            if (isa<LoopBegin>(o)) {
                ++loop_depth;
            } else if (isa<LoopEnd>(o)) {
                --loop_depth;
            } else if (isa<StoreIntConst>(o) || isa<StoreFloatConst>(o)
                    || isa<StoreBoolConst>(o) || isa<StoreStringConst>(o)
                    || isa<StoreNilConst>(o)) {
                // For hoising basic const store we don't have to keep track of
                // CFG merges as it will be always executed and the value can
                // be used and even if the value was previously under a
                // condition then it still does not matter, yes we will store
                // an extra value but it will not change semantics - ahead of
                // time execution (prediction).
                consts_to_hoist_to.push_back({o, loop_start_bci});
                bcb->replace_with_nop(bci, /* no_delete */ true);
                modified = true;
            }
        } else if (isa<LoopBegin>(o)) {
            loop_start_bci = bci;
            ++loop_depth;
        }
    }

    // Hoist all at once in the end to not invalidate iterator of blob above.
    size_t insert_offset = 0;
    for (auto [opc, addr] : consts_to_hoist_to) {
        bcb->insert_at_bci(opc, addr + insert_offset);
        ++insert_offset;
    }

    return modified;
}