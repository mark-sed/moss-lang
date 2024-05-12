/**
 * @file bytecode_reader.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Reads bytecode from a stream
 */

#ifndef _BYTECODE_READER_HPP_
#define _BYTECODE_READER_HPP_

#include "source.hpp"
#include "bytecode.hpp"
#include <fstream>

namespace moss {

class BytecodeReader {
private:
    BytecodeFile &file;
    std::istream *stream;
public:
    BytecodeReader(BytecodeFile &file) : file(file) {
        this->stream = file.get_new_stream();
    }
    ~BytecodeReader() {
        delete stream;
    }

    Bytecode *read();
};

}

#endif//_BYTECODE_READER_HPP_