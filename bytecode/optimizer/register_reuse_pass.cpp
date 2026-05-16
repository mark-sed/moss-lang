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
    // overwritten.
    
    // TODO: Fill this for global frame with known values from builtin.
    std::map<IntConst, Register> int_map;
    std::map<FloatConst, Register> float_map;
    std::map<BoolConst, Register> bool_map;
    std::optional<Register> nil_reg{};
    std::list<Address> jmp_targets;
    bool modified = false;
    bool reached_fun_start = false;

    auto clear_maps = [&]() {
        int_map.clear();
        float_map.clear();
        bool_map.clear();
        nil_reg = std::nullopt;
    };
    
    for (BCBlobIterator oit = bcb->begin(); oit != bcb->end();) {
        auto o = *oit;
        // If function blob, then skip until frame start, which is at pop call
        // frame.
        if (bcb->get_type() == BlobType::FUN_BLOB && !reached_fun_start && !isa<PopCallFrame>(o)) {
            ++oit;
            continue;
        } else {
            reached_fun_start = true;
            ++oit;
            continue;
        }
        bool erased = false;
        Address bci = oit.index();
        // Invalidate if this is a merging path (target of a jump)
        // TODO: Add check if register assigment dominates current use and use
        // that for reuse rather than clearing the table on merging paths.
        //     a = 4
        //     if (...) {}
        //     else {
        //         b = 4 // Wont reuse 'a' value since else is merging point.
        //     }
        auto target_it = std::find(jmp_targets.begin(), jmp_targets.end(), bci);
        if (target_it != jmp_targets.end()) {
            clear_maps();
            jmp_targets.erase(target_it);
        }
        // No need to invalidate constants on reaching a jump since the constant
        // register cannot be overwritten.
        if (opcode::modifies_CFG(o)) {
            if (auto jmp = dyn_cast<Jmp>(o)) {
                jmp_targets.push_back(jmp->addr);
            } else if (auto jmp = dyn_cast<JmpIfTrue>(o)) {
                jmp_targets.push_back(jmp->addr);
            } else if (auto jmp = dyn_cast<JmpIfFalse>(o)) {
                jmp_targets.push_back(jmp->addr);
            } else if (auto jmp = dyn_cast<BreakTo>(o)) {
                jmp_targets.push_back(jmp->addr);
            }
        } else if (isa<RunFinally>(o)) {
            // We need to clear on run finally just to avoid issues inside of
            // finally since it might be run not only after try or catch but
            // also out of order when it leaves scope (on return).
            // Run finally is used for try blocks and it is bit too strict
            // but other opcodes like pop finally would not work.
            clear_maps();
        } else if (auto si = dyn_cast<StoreIntConst>(o)) {
            auto it = int_map.find(si->val);
            if (it != int_map.end()) {
                bcb->replace_register(si->dst, it->second, true);
                bcb->replace_with_nop(bci);
            } else {
                int_map[si->val] = si->dst;
            }
        } else if (auto si = dyn_cast<StoreFloatConst>(o)) {
            auto it = float_map.find(si->val);
            if (it != float_map.end()) {
                bcb->replace_register(si->dst, it->second, true);
                bcb->replace_with_nop(bci);
            } else {
                float_map[si->val] = si->dst;
            }
        } else if (auto si = dyn_cast<StoreBoolConst>(o)) {
            auto it = bool_map.find(si->val);
            if (it != bool_map.end()) {
                bcb->replace_register(si->dst, it->second, true);
                bcb->replace_with_nop(bci);
            } else {
                bool_map[si->val] = si->dst;
            }
        } else if (auto si = dyn_cast<StoreNilConst>(o)) {
            if (nil_reg.has_value()) {
                bcb->replace_register(si->dst, nil_reg.value(), true);
                bcb->replace_with_nop(bci);
            } else {
                nil_reg = si->dst;
            }
        }
        if (!erased) {
            ++oit;
        }
    }
    
    return modified;
}