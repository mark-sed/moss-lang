/**
 * @file clopts.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Command line options handler
 */

#ifndef _CLOPTS_HPP_
#define _CLOPTS_HPP_

#include "errors.hpp"
#include "moss.hpp"
#include "args.hpp"
#include <string>
#include <iostream>

namespace moss {

namespace clopts {

    args::ArgumentParser arg_parser("\b\bMoss Language Interpreter");
    args::Positional<std::string> file_name(arg_parser, "<file name>", "Input moss program to run");
    args::ValueFlag<std::string> code(arg_parser, "<code>", "String of moss code to be run", {'e', "execute"});
    
    void parse_clopts(int argc, const char *argv[]) {
        args::HelpFlag help(arg_parser, "help", "Display available options", {'h', "help"});
        args::Flag version(arg_parser, "version", "Display the version of this program", {"version"});

        file_name.KickOut(true);
        code.KickOut(true);
        arg_parser.Prog(argv[0]);
        const std::vector<std::string> args(argv + 1, argv + argc);
        const auto prog_args_begin = std::begin(args);
        const auto prog_args_end = std::end(args);

        try {
            arg_parser.ParseCLI(argc, argv);
        }
        catch (const args::Help&) {
            // print help
            std::cout << arg_parser;
            // TODO: Handle this more elegantly
            exit(0);
        }
        catch (const args::ParseError& e) {
            // TODO: call errors
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        if(version) {
            std::cout << "moss " << MOSS_VERSION << "\n";
            exit(0);
        }

        // TODO: In case of code being set, remove first arg (-e) and
        // replace second one with empty string
        /*for(auto ra = prog_args_begin; ra != prog_args_end; ++ra) {
            std::cout << "Arg: " << *ra << "\n";
        }*/
    }
}

}

#endif//_CLOPTS_HPP_