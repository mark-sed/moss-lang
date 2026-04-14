#include "bytecode_blob.hpp"

using namespace moss;
using namespace opcode;

BCBlob *BCBlob::parse_bc_impl(Bytecode &bc, Address start, Address end, BlobType type, bool is_glob) {
    std::vector<BCBlob *> inner_blobs;

    for (Address i = start + (is_glob ? 0 : 1); i < end; ++i) {
        OpCode *opc = bc.get_code()[i];
        switch(opc->get_type()) {
            case OpCodes::CREATE_FUN: {
                auto inner_start = i;
                for (; i < end; ++i) {
                    if (auto jmp = dyn_cast<opcode::Jmp>(bc.get_code()[i])) {
                        inner_blobs.push_back(parse_bc_impl(bc, inner_start, jmp->addr, BlobType::FUN_BLOB));
                        break;
                    }
                }
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