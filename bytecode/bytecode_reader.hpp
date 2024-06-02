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
#include "opcode.hpp"
#include <fstream>
#include <cstdlib>

namespace moss {

/** Reader of bytecode files, it converts them into bytecode object */
class BytecodeReader {
private:
    BytecodeFile &file;
    std::istream *stream;

    char *str_buffer;
    size_t buffer_size;

    opcode::Register read_register();
    opcode::StringVal read_string();
    opcode::IntConst read_const_int();
public:
    BytecodeReader(BytecodeFile &file) : file(file), buffer_size(256) {
        this->stream = file.get_new_stream();
        // Buffer might be reallocated, so malloc has to be used
        this->str_buffer = (char *)std::malloc(buffer_size);
    }
    ~BytecodeReader() {
        delete stream;
        std::free(str_buffer);
    }

    /** Reads bytecode into Bytecode object */
    Bytecode *read();
};

}

#endif//_BYTECODE_READER_HPP_