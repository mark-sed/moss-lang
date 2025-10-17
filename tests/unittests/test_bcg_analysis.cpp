#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "expression_analyzer.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"
#include "bytecodegen.hpp"

namespace{

using namespace moss;
using namespace testing;

void check_all_lines_err_bcg(std::vector<ustring> lines, ustring test) {
    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod) << test << ": " << code << "\n";
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_FALSE(err) << test << ": " << code << "\n";
        auto bc = new Bytecode();
        auto bcgn = bcgen::BytecodeGen(bc, &parser);
        auto exc_caught = false;
        try {
            bcgn.generate(mod);
        } catch (ir::IR *err) {
            exc_caught = true;
            delete err;
        }
        EXPECT_TRUE(exc_caught) << "Bytecodegen did not raise: " << test << ": " << code << "\n";

        delete mod;
        delete bc;
    }
}

void check_all_lines_ok_bcg(std::vector<ustring> lines, ustring test) {
    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod) << test << ": " << code << "\n";
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_FALSE(err) << test << ": " << code << "\n";
        auto bc = new Bytecode();
        auto bcgn = bcgen::BytecodeGen(bc, &parser);
        auto exc_caught = false;
        try {
            bcgn.generate(mod);
        } catch (ir::IR *err) {
            exc_caught = true;
            delete err;
        }
        EXPECT_FALSE(exc_caught) << "Bytecodegen raise: " << test << ": " << code << "\n";

        delete mod;
        delete bc;
    }
}

/// This tests that there cannot be break and continue outside of a loop.
TEST(BCGenAnalysis, BreakAndContinueOutsideOfLoop){
    std::vector<ustring> lines = {
"break",
"continue",
"space F { break; }",
"class X { fun X() { continue; }; }",
};

    check_all_lines_err_bcg(lines, "BreakAndContinueOutsideOfLoop");

    std::vector<ustring> lines2 = {
"for (x: [1,2,3]) break",
"do { continue; } while(false)",
"while (true) { break; }",
};

    check_all_lines_ok_bcg(lines2, "BreakAndContinueOutsideOfLoop");
}

}