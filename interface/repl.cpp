#include "repl.hpp"
#include "logging.hpp"
#include "commons.hpp"
#include "moss.hpp"
#include "errors.hpp"
#include "ir.hpp"
#include "bytecode.hpp"
#include "bytecodegen.hpp"
#include "interpreter.hpp"
#include "clopts.hpp"
#include "bytecode_writer.hpp"
#include "ir_pipeline.hpp"
#include <vector>

using namespace moss;

void Repl::output_header() {
    using namespace error;
    using namespace colors;
    outs << std::endl
<< colorize(LIGHT_GREEN) << "88888b.d88b.   .d88b.  .d8888b  .d8888b   " << reset() << "| Version " << MOSS_VERSION << "\n"
<< colorize(LIGHT_GREEN) <<       "888 \"888 \"88b d88\"\"88b 88K      88K       " << reset() << "| Type \"help()\" for information and \"exit()\" to quit.\n"
<< colorize(GREEN) <<       "888  888  888 888  888 \"Y8888b. \"Y8888b.  " << reset() << "|\n"
<< colorize(LIGHT_PURPLE) <<      "888  888  888 Y88..88P      X88      X88  " << reset() << "| GitHub repository: https://github.com/mark-sed/moss-lang\n"
<< colorize(PURPLE) <<      "888  888  888  \"Y88P\"   88888P'  88888P'  " << reset() << "| Documentation: https://github.com/mark-sed/moss-lang/wiki\n"
         << std::endl;
}

int Repl::run() {
    LOGMAX("REPL started");

    // TODO: Add cmd option to not print header or maybe print it just as text
    output_header();

    Bytecode *bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);

    Interpreter *interpreter = new Interpreter(bc, &src_file, true);
    ir::IRPipeline ipl(parser);

    bool eof_reached = false;
    while (!eof_reached && !global_controls::exit_called) {
        outs << error::colors::colorize(error::colors::LIGHT_GREEN) << "moss> " << error::colors::reset();
        std::vector<ir::IR *> line_irs = parser.parse_line();
        for (auto i : line_irs) {
            LOGMAX(*i);
            if (isa<ir::EndOfFile>(i)) {
                eof_reached = true;
                outs << "\n";
            }
            // IR pipeline run
            if (auto err = ipl.run(i)) {
                i = err;
            }
#ifndef NDEBUG
            if (!clopts::parse_only)
                cgen.generate(i);
#else
            cgen.generate(i);
#endif
        }

#ifndef NDEBUG
        if (!clopts::parse_only && !clopts::output_only) {
#else
        if (!clopts::output_only) {
#endif
            //LOGMAX(*bc);
            try {
                interpreter->run();
                // TODO: Also dont print if line is silent
                if (line_irs.size() != 0)
                    outs << "\n";
            } catch (Value *v) {
                if (v->get_type() == BuiltIns::SystemExit) {
                    auto se = dyn_cast<ObjectValue>(v);
                    assert(se);
                    auto ex_code = se->get_attr("code", interpreter);
                    assert(ex_code);
                    if (auto ci = dyn_cast<IntValue>(ex_code)) {
                        interpreter->set_exit_code(ci->get_value());
                    } else {
                        errs << opcode::to_string(interpreter, ex_code);
                        interpreter->set_exit_code(1);
                    }
                    break;
                } else {
                    interpreter->report_call_stack(errs);
                    errs << opcode::to_string(interpreter, v);
                    interpreter->restore_to_global_frame();
                    interpreter->set_bci(interpreter->get_code()->get_code().size());
                }
            }
        }

        for (auto i : line_irs) {
            delete i;
        }
    }

    // Output notes if generator is used
    if (Interpreter::is_generator(clopts::get_note_format()) && interpreter->is_main() && interpreter->get_exit_code() == 0) {
        assert(!Interpreter::running_generator);
        try {
            opcode::output_generator_notes(interpreter);
        } catch (Value *v) {
            interpreter->report_call_stack(errs);
            errs << opcode::to_string(interpreter, v);
            // We're exiting, no need for restore.
        }
    }

    if (clopts::output) {
        std::filesystem::path bcpath = args::get(clopts::output);
        if (bcpath.extension() != ".msb") {
            bcpath += ".msb";
        }
        LOGMAX("Outputting bytecode to path: " << bcpath);

        BytecodeFile bfo(bcpath.string());
        BytecodeWriter bc_writer(bfo);
        bc_writer.write(bc);
    }

    auto exit_code = interpreter->get_exit_code();
    delete interpreter;
    delete bc;

    LOGMAX("REPL finished");
    return exit_code;
}