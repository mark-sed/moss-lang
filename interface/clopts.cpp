#include "clopts.hpp"
#include "os_interface.hpp"
#include "errors.hpp"
#include "moss.hpp"
#include "args.hpp"
#include <string>
#include <iostream>

using namespace moss::clopts;

args::ValueFlag<bool> opt_use_color(arg_parser, "0 or 1", "Enables colored error messages", {"use-color"});

void moss::clopts::parse_clopts(int argc, const char *argv[]) {
    args::HelpFlag help(arg_parser, "help", "Display available options", {'h', "help"});
    args::Flag version(arg_parser, "version", "Display the version of this program", {"version"});

    file_name.KickOut(true);
    code.KickOut(true);
    arg_parser.Prog(argv[0]);
    const std::vector<ustring> args(argv + 1, argv + argc);

    try {
        arg_parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&) {
        // print help
        outs << arg_parser;
        exit(0);
    }
    catch (const args::ParseError& e) {
        error::error(error::ErrorCode::ARGUMENT, e.what());
    }

    if(version) {
        outs << "moss " << MOSS_VERSION << "\n";
        exit(0);
    }
}

int moss::clopts::get_logging_level() {
    #ifndef NDEBUG
        if (verbose1) return 1;
        if (verbose2) return 2;
        if (verbose3) return 3;
        if (verbose4) return 4;
        if (verbose5) return 5;
    #endif
    return 0;
}

ustring moss::clopts::get_logging_list() {
    #ifndef NDEBUG
        if (verbose1) return args::get(verbose1);
        if (verbose2) return args::get(verbose2);
        if (verbose3) return args::get(verbose3);
        if (verbose4) return args::get(verbose4);
        if (verbose5) return args::get(verbose5);
    #endif
    return 0;
}

bool moss::clopts::use_color() {
    if (!opt_use_color) return true;
    return args::get(opt_use_color);
}