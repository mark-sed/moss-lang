#include "repl.hpp"
#include "logging.hpp"
#include "os_interface.hpp"
#include "moss.hpp"
#include "errors.hpp"
#include "ir.hpp"
#include "bytecode.hpp"
#include "bytecodegen.hpp"
#include "interpreter.hpp"
#include "clopts.hpp"
#include <vector>

using namespace moss;

void Repl::output_header() {
    outs << std::endl
<< error::colors::LIGHT_GREEN << "88888b.d88b.   .d88b.  .d8888b  .d8888b   " << error::colors::RESET << "| Version " << MOSS_VERSION << "\n"
<< error::colors::LIGHT_GREEN <<       "888 \"888 \"88b d88\"\"88b 88K      88K       " << error::colors::RESET << "| Type \"help()\" for information and \"exit()\" to quit.\n"
<< error::colors::GREEN <<       "888  888  888 888  888 \"Y8888b. \"Y8888b.  " << error::colors::RESET << "|\n"
<< error::colors::LIGHT_PURPLE <<      "888  888  888 Y88..88P      X88      X88  " << error::colors::RESET << "| GitHub repository: https://github.com/mark-sed/moss-lang\n"
<< error::colors::PURPLE <<      "888  888  888  \"Y88P\"   88888P'  88888P'  " << error::colors::RESET << "| Documentation: https://github.com/mark-sed/moss-lang/wiki\n"
         << std::endl;
}

void Repl::run() {
    LOGMAX("REPL started");

    // TODO: Add cmd option to not print header or maybe print it just as text
    output_header();

    Bytecode *bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);

    Interpreter *interpreter = new Interpreter(bc, &src_file);

    bool eof_reached = false;
    while (!eof_reached) {
        outs << error::colors::LIGHT_GREEN << "moss> " << error::colors::RESET;
        std::vector<ir::IR *> line_irs = parser.parse_line();
        for (auto i : line_irs) {
            LOGMAX(*i);
            if (isa<ir::EndOfFile>(i)) {
                eof_reached = true;
                outs << "\n";
            }
#ifndef NDEBUG
            if (!clopts::parse_only)
                cgen.generate(i);
#else
            cgen.generate(i);
#endif
        }

#ifndef NDEBUG
        if (!clopts::parse_only) {
            //LOGMAX(*bc);
            interpreter->run();
            // TODO: Also dont print if line is silent
            if (line_irs.size() != 0)
                outs << "\n";
        }
#else
        interpreter->run();
        // TODO: Also dont print if line is silent
        if (line_irs.size() != 0)
            outs << "\n";
#endif

        for (auto i : line_irs) {
            delete i;
        }
    }

    LOGMAX("REPL finished");
}