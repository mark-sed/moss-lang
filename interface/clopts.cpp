#include "clopts.hpp"
#include "commons.hpp"
#include "errors.hpp"
#include "moss.hpp"
#include "args.hpp"
#include <string>
#include <iostream>

using namespace moss::clopts;

args::ValueFlag<bool> opt_use_color(interface_group, "0 or 1", "Enables colored error messages", {"use-color"});
args::ValueFlag<std::string> note_format(note_group, "<format-name>", "Converts moss notes to selected format", {'f', "format"});

static std::ostream *output_note_file = nullptr; ///< Stream into which output notes, this has to be explicitly deleted

static std::vector<ustring> program_args;
static WarningLevel warn_level = WarningLevel::WL_IGNORE;

void moss::clopts::parse_clopts(int argc, const char *argv[]) {
    args::HelpFlag help(interface_group, "help", "Display available options", {'h', "help"});
    args::Flag version(interface_group, "version", "Display the version of this program", {"version"});

    file_name.KickOut(true);
    code.KickOut(true);
    arg_parser.Prog(argv[0]);
    const std::vector<ustring> args(argv + 1, argv + argc);
    const auto begin = std::begin(args);
    const auto end = std::end(args);
    auto rest = begin;

    try {
        rest = arg_parser.ParseArgs(begin, end);
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
        outs << "moss " << MOSS_VERSION;
#ifndef NDEBUG
        outs << " (DEBUG build)";
#endif
        outs << "\n";
        exit(0);
    }

    for (;rest != end; ++rest) {
        program_args.push_back(*rest);
    }

    if (warning) {
        auto war_val = args::get(warning);
        if (war_val == "ignore" || war_val == "i") {
            warn_level = WarningLevel::WL_IGNORE;
        } else if (war_val == "all" || war_val == "a") {
            warn_level = WarningLevel::WL_ALL;
        } else if (war_val == "error" || war_val == "e") {
            warn_level = WarningLevel::WL_ERROR;
        } else {
            // TODO: Word more nicely and add explanation for the levels
            error::error(error::ErrorCode::ARGUMENT, ustring("Unknown -W value '" + args::get(warning) + "'! Value can be one for these: all, error, ignore").c_str());
        }
    }
}

std::vector<ustring> moss::clopts::get_program_args() {
    return program_args;
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

std::ostream &moss::clopts::get_note_stream() {
    if (!note_file) return outs;
    if (!output_note_file) {
        output_note_file = new std::ofstream(args::get(note_file));
    }
    return *output_note_file;
}

void moss::clopts::deinit() {
    if (output_note_file)
        delete output_note_file;
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

ustring moss::clopts::get_note_format() {
    if (!note_format) return "txt";
    return args::get(note_format);
}

WarningLevel moss::clopts::get_warning_level() {
    return warn_level;
}