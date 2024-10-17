/**
 * @file bytecode_writer.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Writes bytecode to a file or object
 */

#ifndef _BYTECODE_WRITER_HPP_
#define _BYTECODE_WRITER_HPP_

#include "source.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"
#include <fstream>
#include <cstdlib>

namespace moss {

/** 
 * Writes bytecode object into a file, which can be again read
 * by BytecodeReader and run. 
 */
class BytecodeWriter {
private:
    BytecodeFile &file;
    std::ostream *stream;

    void write_register(opcode::Register reg);
    void write_address(opcode::Address addr);
    void write_string(opcode::StringConst val);
    void write_int(opcode::IntConst v);
public:
    BytecodeWriter(BytecodeFile &file) : file(file) {
        this->stream = file.create_out_stream();
    }
    ~BytecodeWriter() {
        delete this->stream;
    }

    /** Writes bytecode into a file */
    void write(Bytecode *code);
};

}

#endif//_BYTECODE_WRITER_HPP_