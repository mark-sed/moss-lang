///
/// \file bytecode_header.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Information about bytecode header for bytecode reader and writer
///

#ifndef _BYTECODE_HEADER_HPP_
#define _BYTECODE_HEADER_HPP_

#include "moss.hpp"
#include <cstdint>
#include <ctime>

namespace moss {

/// All bytecode header related resources
namespace bc_header {

#define BCH_ID_SIZE 4        ///< Size of id in bytes
#define BCH_CHECKSUM_SIZE 4  ///< Size of checksum in bytes
#define BCH_VERSION_SIZE 4   ///< Size of version in bytes
#define BCH_TIMESTAMP_SIZE 4 ///< Size of timestamp in bytes
#define BCH_SIZE 16          ///< Overall header size

/// Bytecode header consists of:
/// 
/// 4B | 0xFF 0x2A 0x00 0x00
/// 4B | reserved (for checksum?)
/// 4B | major, minor, patch, 0x00
/// 4B | timestamp
/// 
#ifdef __linux__
struct __attribute__((packed)) BytecodeHeader {
#else
// TODO: Handle other OSs
struct BytecodeHeader {
#endif
    const std::uint32_t id = 0xFF'00'00'2A;
    std::uint32_t checksum;
    std::uint32_t version;
    std::uint32_t timestamp;

    uint32_t get_version_major() {
        return (version & 0xFF000000) >> 24;
    }

    uint32_t get_version_minor() {
        return (version & 0x00FF0000) >> 16;
    }

    uint32_t get_version_patch() {
        return (version & 0x0000FF00) >> 8;
    }
};

inline std::ostream& operator<< (std::ostream& os, BytecodeHeader &bch) {
    time_t ts = bch.timestamp;
    os << "Bytecode header:"
       << "\n  id:        0x" << std::hex << bch.id
       << "\n  checksum:  0x" << bch.checksum
       << "\n  version:   (0x" << bch.version << std::dec << ") "
          << bch.get_version_major() << "." << bch.get_version_minor() << "." << bch.get_version_patch() 
       << "\n  timestamp: (" << bch.timestamp << ") "
       << std::dec <<  std::asctime(std::localtime(&ts));
    return os;
}

/// Creates header for bytecode with current info 
inline BytecodeHeader create_header() {
    static_assert(sizeof(BytecodeHeader) == BCH_SIZE);

    BytecodeHeader bch;
    bch.checksum = 0;
    bch.version = (MOSS_VERSION_MAJOR << 24) | (MOSS_VERSION_MINOR << 16) | (MOSS_VERSION_PATCH << 8);
    bch.timestamp = std::time(nullptr);
    return bch;
}

}

}

#endif//_BYTECODE_HEADER_HPP_