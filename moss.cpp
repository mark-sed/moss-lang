#include "moss.hpp"
#include "scanner.hpp"
#include "os_interface.hpp"
#include "clopts.hpp"
#include "args.hpp"
#include "logging.hpp"
#include <iostream>

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

    auto main_file = get_input();
    Scanner scanner(main_file);

    Token *t = scanner.next_nonws_token();
    while(t->get_type() != TokenType::END_OF_FILE) {
        /*if(t->get_type() == TokenType::ERROR_TOKEN) {
            outs << "[" << dynamic_cast<ErrorToken*>(t)->get_report() << "] ";
        }
        else {*/
            outs << *t << " ";
        //}
        t = scanner.next_nonws_token();
    }

    return 0;
}