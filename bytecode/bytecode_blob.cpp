#include "bytecode_blob.hpp"

using namespace moss;
using namespace opcode;

BCBlobIterator BCBlob::begin() {
    return BCBlobIterator(&bc, start_, end_, &inner_blobs);
}

BCBlobIterator BCBlob::end() {
    return BCBlobIterator(&bc, end_, end_, &inner_blobs);
}

BCBlob *BCBlob::parse_bc_impl(Bytecode &bc, Address start, Address end, BlobType type, bool is_glob) {
    std::vector<BCBlob *> inner_blobs;

    for (Address i = start + (is_glob ? 0 : 1); i < end; ++i) {
        OpCode *opc = bc.get_code()[i];
        switch(opc->get_type()) {
            case OpCodes::CREATE_FUN: {
                auto inner_start = i;
                // FIXME: Add correct parsing for lambdas in argument (jmp before this fun jmp)
                for (; i < end; ++i) {
                    if (auto jmp = dyn_cast<opcode::Jmp>(bc.get_code()[i])) {
                        inner_blobs.push_back(parse_bc_impl(bc, inner_start, jmp->addr, BlobType::FUN_BLOB));
                        i = jmp->addr-1;
                        break;
                    }
                }
            } break;
            case OpCodes::BUILD_SPACE:
            case OpCodes::BUILD_CLASS: {
                // Iterate forward looking for a matching pop_frame.
                size_t nesting = 1;
                auto inner_start = i;
                ++i;
                for (; i < end; ++i) {
                    OpCode *inner_opc = bc.get_code()[i];
                    switch(inner_opc->get_type()) {
                        case OpCodes::POP_FRAME:
                            --nesting;
                        break;
                        case OpCodes::BUILD_SPACE:
                        case OpCodes::BUILD_CLASS:
                            ++nesting;
                        break;
                        default: break;
                    }
                    if (nesting == 0)
                        break;
                }
                inner_blobs.push_back(parse_bc_impl(bc, inner_start, i, (opc->get_type() == OpCodes::BUILD_SPACE ? BlobType::SPACE_BLOB : BlobType::CLASS_BLOB)));
            } break;
            default: break;
        };
    }

    auto rb = new BCBlob(type, bc, start, end);
    if (!inner_blobs.empty())
        rb->set_inner_blobs(inner_blobs);
    return rb;
}

BCBlob *BCBlob::parse_bc(Bytecode &bc) {
    return parse_bc_impl(bc, 0, bc.size(), BlobType::BC_BLOB, true);
}

void BCBlob::replace_register(Register prev, Register replacement, bool constant_reg) {
    for (auto o: *this) {
        o->replace_register(prev, replacement, constant_reg);
    }
}

BCBlobIterator BCBlob::erase(BCBlobIterator it) {
    size_t idx = it.index();

    bc.erase(idx);
    --end_;

    for (auto* b : inner_blobs) {
        if (b->start_ > idx) {
            --b->start_;
            --b->end_;
        } else if (b->end_ > idx) {
            --b->end_;
        }
    }

    // construct final iterator directly (no assignment)
    BCBlobIterator next(&bc, idx, end_, &inner_blobs);

    // normalize without assignment
    while (true) {
        bool jumped = false;

        for (auto* b : inner_blobs) {
            if (next.index() >= b->start_ && next.index() < b->end_) {
                next = BCBlobIterator(&bc, b->end_, end_, &inner_blobs);
                jumped = true;
                break;
            }
        }

        if (!jumped) break;
    }

    return next;
}