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

/// Blob is a span into bytecode from [start, end)
class BCBlob {
private:
    Bytecode &bc;
    Address start_;
    Address end_;
public:
    BCBlob(Bytecode &bc, Address start, Address end) : bc(bc), start_(start), end_(end) {
        assert(start < bc.size() && "Out of bounds blob start BCI");
        assert(end <= bc.size() && "Out of bounds blob end BCI");
        assert(end >= start && "Blob end bci is before start bci");
    }

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
};

inline std::ostream& operator<< (std::ostream& os, BCBlob &bcb) {
    return bcb.debug(os);
}

}
}

#endif//_BYTECODE_BLOB_HPP_