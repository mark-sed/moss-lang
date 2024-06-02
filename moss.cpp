#include "moss.hpp"
#include "scanner.hpp"
#include "source.hpp"
#include "os_interface.hpp"
#include "clopts.hpp"
#include "args.hpp"
#include "logging.hpp"
#include <iostream>

#include "bytecode_reader.hpp"
#include "bytecode_writer.hpp"
#include "bytecode.hpp"
#include "interpreter.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#endif

using namespace moss;

static SourceFile get_input() {
    if (clopts::file_name)
        return SourceFile(args::get(clopts::file_name), SourceFile::SourceType::FILE);
    if (clopts::code)
        return SourceFile(args::get(clopts::code), SourceFile::SourceType::STRING);
    if (is_stdin_atty())
        return SourceFile(SourceFile::SourceType::INTERACTIVE);
    return SourceFile(SourceFile::SourceType::STDIN);
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

    /*auto main_file = get_input();

    Scanner scanner(main_file);
    Token *t = scanner.next_nonws_token();
    while(t->get_type() != TokenType::END_OF_FILE) {
        outs << *t << " ";
        delete t;
        t = scanner.next_nonws_token();
    }
    delete t;*/

    BytecodeFile bf("examples/test.msb");
    BytecodeReader *bcreader = new BytecodeReader(bf);
    Bytecode *bc = bcreader->read();

    BytecodeFile bfo("examples/generated.msb");
    BytecodeWriter *bcwriter = new BytecodeWriter(bfo);
    bcwriter->write(bc);

    Interpreter *interpreter = new Interpreter(bc);
    interpreter->run();

    LOGMAX(*interpreter);
    int exit_code = interpreter->get_exit_code();

    delete interpreter;
    delete bc;
    delete bcwriter;
    delete bcreader;

    return exit_code;
}