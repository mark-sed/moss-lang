#include "moss.hpp"
#include "scanner.hpp"
#include "os_interface.hpp"
#include "clopts.hpp"
#include "args.hpp"
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

    auto main_file = get_input();
    Scanner scanner(main_file);

    return 0;
}