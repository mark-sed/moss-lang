/**
 * @file source.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Holds information about source file and general source info
 */

#ifndef _SOURCE_HPP_
#define _SOURCE_HPP_

#include "os_interface.hpp"
#include <utility>
#include <filesystem>
#include <fstream>

namespace moss {

class File {
protected:
    ustring path;

    File(ustring path) : path(path) {}
    virtual ~File() {}
    virtual std::istream *get_new_stream() = 0;
public:
    ustring get_path() { return this->path; }
    virtual ustring get_name() { return this->path; }
};

/** 
 * @brief Representation of source file
 * 
 * Holds information about source "file".
 * It does not have to be physical file, it can be a string of code or stdin.
 * This provides simple interface to get the file type, name and stream to
 * read it.
 */
class SourceFile : public File {
public:
    /** Source File type (file, string of code or standard input) */
    enum class SourceType {
        FILE,
        STRING,
        STDIN,
        INTERACTIVE
    };
private:
    ustring path_or_code;
    SourceType type;
public:
    SourceFile(ustring path_or_code, SourceType src_type) : File(path_or_code), path_or_code(path_or_code), type(src_type) {
        assert(src_type != SourceType::STDIN && "Path set, but type is STDIN");
        assert(src_type != SourceType::INTERACTIVE && "Path set, but type is INTERACTIVE");
    }
    SourceFile(SourceType src_type) : File("<stdin>"), type(src_type) {
        assert((src_type == SourceType::STDIN || src_type == SourceType::INTERACTIVE) && "File without path");
        if(src_type == SourceType::INTERACTIVE)
            this->path_or_code = "<im>";
        else
            this->path_or_code = "<stdin>";
    }

    ustring get_path_or_code() { return this->path_or_code; }
    SourceType get_type() { return this->type; }
    virtual std::istream *get_new_stream() override;
    virtual ustring get_name() override {
        if (type == SourceType::STRING) return "<one-liner>";
        return path_or_code;
    }
    ustring get_module_name() {
        return std::filesystem::path(get_name()).stem();
    }
};


/** Stores source file information for a given token */
class SourceInfo {
private:
    const SourceFile &file;
    std::pair<unsigned, unsigned> lines; ///< Range of lines of the token
    std::pair<unsigned, unsigned> cols;  ///< Starting column and ending column

public:
    SourceInfo(const SourceFile &file, unsigned line_start, unsigned line_end, unsigned col_start, unsigned col_end) 
        : file(file), lines(std::make_pair(line_start, line_end)), cols(std::make_pair(col_start, col_end)) {}
    SourceInfo(const SourceFile &file, std::pair<unsigned, unsigned> lines, std::pair<unsigned, unsigned> cols) 
        : file(file), lines(lines), cols(cols) {}

    const SourceFile &get_file() { return file; }
};


class BytecodeFile : public File {
public:
    BytecodeFile(ustring path) : File(path) {}

    virtual std::istream *get_new_stream() override;
};

}

#endif//_SOURCE_HPP_