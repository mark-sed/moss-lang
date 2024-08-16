#include "source.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include <cstring>
#include <fstream>

using namespace moss;

std::istream *SourceFile::get_new_stream() {
    switch(this->type) {
        case SourceFile::SourceType::FILE: {
            std::ifstream *f = new std::ifstream(this->path_or_code);
            if(f->fail()){
                error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
            }
            return f;
        }
        case SourceFile::SourceType::STRING: {
            return new std::istringstream(this->path_or_code);
        }
        case SourceFile::SourceType::STDIN: return &std::cin;
        case SourceFile::SourceType::REPL: {
            // REPL input is handled on its own, but reads from stdin
            return &std::cin;
        }
    }
    error::error(error::ErrorCode::INTERNAL, "Unknown input format", this);
    return nullptr;
}

std::istream *BytecodeFile::get_new_stream() {
    std::ifstream *f = new std::ifstream(this->path, std::ios_base::binary);
    if(f->fail()){
        error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
    }
    return f;
}

std::ostream *BytecodeFile::create_out_stream() {
    std::ofstream *f = new std::ofstream(this->path, std::ios_base::binary);
    if (f->fail()) {
        error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
    }
    LOGMAX("Created new output bytecode file: " << this->path);
    return f;
}