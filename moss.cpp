#include "moss.hpp"
#include "scanner.hpp"
#include "source.hpp"
#include "commons.hpp"
#include "clopts.hpp"
#include "args.hpp"
#include "parser.hpp"
#include "repl.hpp"
#include "logging.hpp"
#include "bytecodegen.hpp"
#include "bytecode.hpp"
#include "interpreter.hpp"
#include "bytecode_reader.hpp"
#include "bytecode_writer.hpp"
#include "opcode.hpp"
#include "mslib.hpp"
#include <iostream>
#include <filesystem>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#endif

using namespace moss;

static SourceFile *get_input() {
#ifndef NDEBUG
    if (clopts::use_repl_mode) {
        LOGMAX("Forcing input into repl since --use-repl-mode was used");
        return new SourceFile(SourceFile::SourceType::REPL);
    }
#endif
    if (clopts::file_name)
        return new SourceFile(args::get(clopts::file_name), SourceFile::SourceType::FILE);
    if (clopts::code)
        return new SourceFile(args::get(clopts::code), SourceFile::SourceType::STRING);
    if (is_stdin_atty())
        return new SourceFile(SourceFile::SourceType::REPL);
    return new SourceFile(SourceFile::SourceType::STDIN);
}


int main(int argc, const char *argv[]) {
    // On Windows we need to set the output to accept utf8 strings
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Set console code page to UTF-8 so console known how to interpret string data
    SetConsoleOutputCP(CP_UTF8);
    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

    clopts::parse_clopts(argc, argv);

    if(clopts::get_logging_level() > 0) {
        // Enable logging
        Logger::get().set_logging_level(clopts::get_logging_level());
        Logger::get().set_flags(std::ios_base::boolalpha);
        Logger::get().set_log_everything(clopts::get_logging_list() == "all");
        Logger::get().set_enabled(utils::split_csv_set(clopts::get_logging_list()));
    }

    LOG1("Moss " << MOSS_VERSION);
    LOG1("Logging enabled with level: " << clopts::get_logging_level());
    LOG5("Unicode output test (sushi emoji, umlaut u and japanese): " << "🍣 ü ラーメン");

    bool input_is_msb = false;
    // Check if input is msb
    if (clopts::file_name) {
        std::filesystem::path msbpath = args::get(clopts::file_name);
        input_is_msb = msbpath.extension() == ".msb" || clopts::input_bc;
    }
    
    // Error options check
    if (!clopts::file_name && clopts::input_bc) {
        error::error(error::ErrorCode::ARGUMENT, "Input msb file needs to be specified for '--bytecode' option", nullptr, true);
    }
    if (input_is_msb && clopts::output && !clopts::dump_text_bc) {
        error::error(error::ErrorCode::ARGUMENT, "Trying to dump bytecode for bytecode input", nullptr, true);
    }
    if (clopts::dump_text_bc && !clopts::output) {
        error::error(error::ErrorCode::ARGUMENT, "Option -S requires -o specified", nullptr, true);
    }

    ir::IR *main_mod = nullptr;
    File *input_file = nullptr;
    // .ms input
    if (!input_is_msb) {
        auto main_file = get_input();
        input_file = main_file;

        // REPL
        if (main_file->get_type() == SourceFile::SourceType::REPL) {
            Repl repl(*main_file);
            auto exit_code = repl.run();
            return exit_code;
        }
        else if (main_file->get_type() == SourceFile::SourceType::FILE) {
            // Set pwd for finding other modules
            std::filesystem::path main_f_path = main_file->get_name();
            global_controls::pwd = main_f_path.parent_path();
            LOGMAX("Setting pwd to: " << global_controls::pwd);
        }

        Parser parser(*main_file);
        main_mod = parser.parse(true);
        if (auto exc = dyn_cast<ir::Raise>(main_mod)) {
            // An exception was raised in the parser, lets report it straight away
            ir::StringLiteral *err_msg = dyn_cast<ir::StringLiteral>(exc->get_exception());
            errs << "SyntaxError: " << err_msg->get_value();
            error::exit(error::ErrorCode::RUNTIME);
        }

#ifndef NDEBUG
        // When --parse-only is set, then let's not interpret this
        if (clopts::parse_only) {
            delete main_mod;
            return 0;
        }
#endif
        LOG3("Parsed: " << *main_mod);
    }

    Bytecode *bc = nullptr;
    
    // Bytecode reading or generation
    if (input_is_msb) {
        LOGMAX("Reading bytecode");
        auto ibf = new BytecodeFile(args::get(clopts::file_name));
        input_file = ibf;
        BytecodeReader bcreader(*ibf);
        bc = bcreader.read();
        LOG2("Read bytecode: \n" << *bc);
        if (clopts::print_bc_info) {
            auto header = bc->get_header();
            outs << *header;

            delete bc;
            delete input_file;
            delete main_mod;
            return 0;
        }
    } else {
        bc = new Bytecode();
        bcgen::BytecodeGen cgen(bc);
        cgen.generate(main_mod);
        LOG2("Generated bytecode: \n" << *bc);
    }

    // If options were correct info would be printed and exited already
    if (clopts::print_bc_info) {
        error::error(error::ErrorCode::ARGUMENT, "Option '--print-bc-header' can be used only for bytecode input", nullptr, true);
    }

    // Bytecode output
    if (clopts::output) {
        // Bytecode output
        std::filesystem::path bcpath = args::get(clopts::output);
        if (bcpath.extension() != ".msb" && !clopts::dump_text_bc) {
            bcpath += ".msb";
        }
        LOG2("Outputting bytecode to path: " << bcpath);

        BytecodeFile bfo(bcpath.string());
        BytecodeWriter bc_writer(bfo);
        if (clopts::dump_text_bc)
            bc_writer.write_textual(bc);
        else
            bc_writer.write(bc);
    }

    int exit_code = 0;
    // Interpretation
    if (!clopts::output_only) {
        Interpreter *interpreter = new Interpreter(bc, input_file, true);

        try {
            interpreter->run();
        } catch (Value *v) {
            // Print call stack
            interpreter->report_call_stack(errs);
            errs << opcode::to_string(interpreter, v);
            interpreter->set_exit_code(1);
        }

        LOGMAX(*interpreter);
        exit_code = interpreter->get_exit_code();

        delete interpreter;
    }

    // Cleanup
    delete bc;
    delete input_file;
    delete main_mod;

    if (clopts::delete_values_on_exit) {
        for (auto v: Value::all_values) {
            delete v;
        }
    }

    return exit_code;
}