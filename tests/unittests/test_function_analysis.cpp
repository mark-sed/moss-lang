#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "function_analyzer.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace testing;

/// This tests that FunctionAnalysis reports duplicate argument names.
TEST(FunctionAnalysis, DuplicateArgs){
    std::vector<ustring> lines = {
"fun g(a, b, a, c) {}",
"fun g(a, b, ... a) {}"
};

    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_TRUE(err);
        auto rs = dyn_cast<ir::Raise>(err);
        ASSERT_TRUE(rs);

        delete mod;
        delete err;
    }
}


}