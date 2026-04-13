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
public:
    auto begin() {
        return bc.code.begin() + start_;
    }

    auto end() {
        return bc.code.begin() + end_;
    }

    OpCode* operator[](size_t i) const {
        assert(start_ + i < end_ && "Accessing blob with [] out of bounds");
        return bc.code[start_ + i];
    }

    size_t size() const {
        return end_ - start_;
    }

    OpCode* front() const { return bc.code[start_]; }
    OpCode* back()  const { return bc.code[end_-1]; }

    std::ostream& debug(std::ostream& os) {
        return bc.debug(os, start_, end_);
    }

    void set_inner_blobs(std::vector<BCBlob *> inner_blobs) {
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