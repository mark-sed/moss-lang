///
/// \file bytecode_blob.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Section of bytecode.
///

#ifndef _BYTECODE_BLOB_HPP_
#define _BYTECODE_BLOB_HPP_

#include "bytecode.hpp"

namespace moss {
namespace opcode {

enum BlobType {
    BC_BLOB,
    FUN_BLOB,
    SPACE_BLOB,
    CLASS_BLOB,
};

inline ustring BlobType2string(BlobType type) {
    switch(type) {
        case BlobType::BC_BLOB: return "Blob";
        case BlobType::FUN_BLOB: return "Function blob";
        case BlobType::SPACE_BLOB: return "Space blob";
        case BlobType::CLASS_BLOB: return "Class blob";
        default: {
            assert(false && "Missing blob type to string convertor");
            return "Unknown blob";
        }
    }
}

class BCBlobIterator;

/// Blob is a span into bytecode from [start, end)
class BCBlob {
protected:
    BlobType blob_type;

    Bytecode &bc;
    Address start_;
    Address end_;

    std::vector<BCBlob *> inner_blobs;
public:
    BCBlob(Bytecode &bc, Address start, Address end) : blob_type(BC_BLOB), bc(bc), start_(start), end_(end) {
        assert(start < bc.size() && "Out of bounds blob start BCI");
        assert(end <= bc.size() && "Out of bounds blob end BCI");
        assert(end >= start && "Blob end bci is before start bci");
    }
    BCBlob(BlobType type, Bytecode &bc, Address start, Address end) : blob_type(type), bc(bc), start_(start), end_(end) {
        assert(start < bc.size() && "Out of bounds blob start BCI");
        assert(end <= bc.size() && "Out of bounds blob end BCI");
        assert(end >= start && "Blob end bci is before start bci");
    }
    virtual ~BCBlob() {}

private:
    friend class opcode::BCBlobIterator;
    static BCBlob *parse_bc_impl(Bytecode &bc, Address start, Address end, BlobType type, bool is_glob=false);

public:
    static BCBlob *parse_bc(Bytecode &bc);

    BCBlobIterator begin();

    BCBlobIterator end();

    void insert_blob_index(OpCode *opc, Address i) {
        bc.insert(opc, start_ + i);
    }

    void insert_at_bci(OpCode *opc, Address bci) {
        bc.insert(opc, bci);
    }

    //void erase(OpCode *opc) {
    //    auto it = std::find(bc.code.begin(), bc.code.end(), opc);
    //    if (it != bc.code.end()) {
    //        bc.code.erase(it);
    //    }
    //}

    BCBlobIterator erase(BCBlobIterator opc);

    void replace_with_nop(Address bci, bool no_delete=false) {
        auto op = bc.code[bci];
        bc.code[bci] = new Nop();
        if (!no_delete)
            delete op;
    }

    // OpCode* operator[](size_t i) const {
    //     assert(start_ + i < end_ && "Accessing blob with [] out of bounds");
    //     return bc.code[start_ + i];
    // }

    size_t size() const {
        return end_ - start_;
    }

    OpCode* front() const { return bc.code[start_]; }
    OpCode* back()  const { return bc.code[end_-1]; }

    Bytecode &get_bc() { return this->bc; }

    void replace_register(Register prev, Register replacement, bool constant_reg);

    ustring get_debug_name() {
        return BlobType2string(blob_type) + " [" + std::to_string(start_) + "; " + std::to_string(end_) + ")";
    }

    void print_bc_tree(std::ostream &os, int indent = 0) {
        size_t i = 0;
        for (auto b: inner_blobs) {
            if (i == inner_blobs.size()-1)
                os << ";" << std::string(indent, ' ') << " └─ ";
            else
                os << ";" << std::string(indent, ' ') << " ├─ ";
            os << b->get_debug_name() << "\n";
            b->print_bc_tree(os, indent + 2);
            ++i;
        }
    }

    std::ostream& debug(std::ostream& os) {
        os << "; " << BlobType2string(blob_type) << "\n";
        print_bc_tree(os);
        return bc.debug(os, start_, end_);
    }

    void set_inner_blobs(std::vector<BCBlob *> inner_blobs) {
        std::sort(inner_blobs.begin(), inner_blobs.end(),
              [](BCBlob* a, BCBlob* b) {
                  return a->start_ < b->start_;
              });
        this->inner_blobs = inner_blobs;
    }

    std::vector<BCBlob *> &get_inner_blobs() { return inner_blobs; }

    bool isa_fun() { return this->blob_type == BlobType::FUN_BLOB; }
    bool isa_space() { return this->blob_type == BlobType::SPACE_BLOB; }
    bool isa_class() { return this->blob_type == BlobType::CLASS_BLOB; }
    bool isa_blob() { return this->blob_type == BlobType::BC_BLOB; }

    BlobType get_type() { return blob_type; }
};

inline std::ostream& operator<< (std::ostream& os, BCBlob &bcb) {
    return bcb.debug(os);
}

class BCBlobIterator {
    Bytecode *bc;
    size_t i;
    size_t end;

    const std::vector<BCBlob*> *blobs;
    size_t blob_idx = 0;

public:
    BCBlobIterator(Bytecode *bc, size_t start, size_t end,
                   const std::vector<BCBlob*> *blobs, size_t blob_idx = 0)
        : bc(bc), i(start), end(end), blobs(blobs), blob_idx(blob_idx) {}

    OpCode* operator*() const {
        return bc->code[i];
    }

    BCBlobIterator& operator++() {
        ++i;
        // skip blobs
        while (blob_idx < blobs->size()) {
            auto *b = (*blobs)[blob_idx];

            if (i < b->start_) break;

            if (i >= b->start_ && i < b->end_) {
                i = b->end_;
            }

            ++blob_idx;
        }

        return *this;
    }

    size_t index() const { return i; }

    bool operator!=(const BCBlobIterator &other) const {
        return i != other.i;
    }
};

// class FunBlob : public BCBlob {
// private:
//     ustring name;
// public:
//     static const BlobType ClassType = BlobType::FUN_BLOB;
// 
//     FunBlob(ustring name, Bytecode &bc, Address start, Address end) : BCBlob(ClassType, bc, start, end), name(name) {
//     }
// };
// 
// class ClassBlob : public BCBlob {
// private:
//     ustring name;
// public:
//     static const BlobType ClassType = BlobType::CLASS_BLOB;
// 
//     ClassBlob(ustring name, Bytecode &bc, Address start, Address end) : BCBlob(ClassType, bc, start, end), name(name) {
//     }
// };
// 
// class SpaceBlob : public BCBlob {
// private:
//     ustring name;
// public:
//     static const BlobType ClassType = BlobType::SPACE_BLOB;
// 
//     SpaceBlob(ustring name, Bytecode &bc, Address start, Address end) : BCBlob(ClassType, bc, start, end), name(name) {
//     }
// };

}

// Helper functions
// template<class T>
// bool isa(opcode::BCBlob& o) {
//     return o.get_type() == T::ClassType;
// }
// 
// template<class T>
// bool isa(opcode::BCBlob* o) {
//     return o->get_type() == T::ClassType;
// }
// 
// template<class T>
// T *dyn_cast(opcode::BCBlob* o) {
//     if (!isa<T>(o)) return nullptr;
//     return dynamic_cast<T *>(o);
// }

}

#endif//_BYTECODE_BLOB_HPP_