/// 
/// \file source.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Holds information about source file and general source info
/// 

#ifndef _SOURCE_HPP_
#define _SOURCE_HPP_

#include "commons.hpp"
#include <utility>
#include <filesystem>
#include <fstream>
#include <optional>

namespace moss {

/// \brief Base class for all sources
/// Base "file", but this may be also just a string or stdin it mainly
/// provides get_new_stream method for reading this source.
class File {
protected:
    ustring path;

    File(ustring path) : path(path) {}

    /// \return Source as an input stream 
    virtual std::istream *get_new_stream() = 0;
public:
    virtual ~File() {}

    /// Getter for path to this file. But as this may hold also string or stream
    /// source, then this path is not an actual path and this should be checked 
    /// \return Path to this file
    /// \warning The path returned might not be valid nor an actual path
    ustring get_path() { return this->path; }

    /// Getter for name of the file/stream. This may be the path to the file
    /// or the stream/string name.
    /// \return Name or path of this file
    virtual ustring get_name() const { return this->path; }
};

/// \brief Representation of source file
/// 
/// Holds information about source "file".
/// It does not have to be physical file, it can be a string of code or stdin.
/// This provides simple interface to get the file type, name and stream to
/// read it.
class SourceFile : public File {
public:
    /// Source File type (file, string of code or standard input)
    enum class SourceType {
        FILE,
        STRING,
        STDIN,
        REPL
    };
private:
    ustring path_or_code;
    SourceType type;
public:
    SourceFile(ustring path_or_code, SourceType src_type) : File(path_or_code), path_or_code(path_or_code), type(src_type) {
        assert(src_type != SourceType::STDIN && "Path set, but type is STDIN");
        assert(src_type != SourceType::REPL && "Path set, but type is REPL");
    }
    SourceFile(SourceType src_type) : File("<stdin>"), type(src_type) {
        assert((src_type == SourceType::STDIN || src_type == SourceType::REPL) && "File without path");
        if(src_type == SourceType::REPL)
            this->path_or_code = "<repl>";
        else
            this->path_or_code = "<stdin>";
    }

    /// \return Path to this source file if it is a file, if the source is a repl
    ///         input or stdin, then it will be its name and if the source is a
    ///         string then it will be its contents.
    ustring get_path_or_code() { return this->path_or_code; }

    /// \return Type of this source file
    SourceType get_type() { return this->type; }

    /// For SourceFile that is an actual file this will open it as std::ifstream,
    /// for string source it will be a new std::istringstream and for stdin or
    /// repl it will be std::cin
    virtual std::istream *get_new_stream() override;

    /// \return Name of this source file
    virtual ustring get_name() const override {
        if (type == SourceType::STRING) return "<one-liner>";
        return path_or_code;
    }
 
    /// \return Name that is stemmed -- just the file name without path and
    ///         extensions.
    ustring get_module_name() {
        return std::filesystem::path(get_name()).stem().string();
    }
};

/// \brief Bytecode source file
/// A source file that contains moss bytecode (.msb file)
class BytecodeFile : public File {
public:
    BytecodeFile(ustring path) : File(path) {}

    virtual std::istream *get_new_stream() override;

    /// Creates a new binary std::ofstream for writing a bytecode into this file
    /// \return Created output stream 
    std::ostream *create_out_stream();
};

/// Stores source file information for a given token
class SourceInfo {
private:
    const SourceFile &file;
    std::pair<unsigned, unsigned> lines; ///< Range of lines of the token
    std::pair<unsigned, unsigned> cols;  ///< Starting column and ending column

    static const SourceFile dummy_file;
public:
    SourceInfo(const SourceFile &file, unsigned line_start, unsigned line_end, unsigned col_start, unsigned col_end) 
        : file(file), lines(std::make_pair(line_start, line_end)), cols(std::make_pair(col_start, col_end)) {}
    SourceInfo(const SourceFile &file, std::pair<unsigned, unsigned> lines, std::pair<unsigned, unsigned> cols) 
        : file(file), lines(lines), cols(cols) {}
    SourceInfo() : file(dummy_file), lines(std::make_pair(0, 0)), cols(std::make_pair(0, 0)) {}

    const SourceFile &get_file() { return file; }
    std::pair<unsigned, unsigned> get_lines() { return lines; }
    std::pair<unsigned, unsigned> get_cols() { return cols; }
    void set_lines(std::pair<unsigned, unsigned> l) { this->lines = l; }
    void set_cols(std::pair<unsigned, unsigned> c) { this->cols = c; }
    void set_end_line(unsigned l) { this->lines = std::make_pair(this->lines.first, l); }
    void update_ends(SourceInfo srci) {
        this->lines = std::make_pair(this->lines.first, srci.lines.second);
        this->cols = std::make_pair(this->cols.first, srci.cols.second);
    }
};

std::optional<ustring> get_file_path(ustring file);

}

#endif//_SOURCE_HPP_