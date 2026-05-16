#include <gtest/gtest.h>
#include <sstream>
#include "bytecode_blob.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"
#include "values.hpp"
#include "parser.hpp"
#include "bytecodegen.hpp"
#include "testing_utils.hpp"
#include "optimizer/bc_pipeline.hpp"
#include "optimizer/bc_pass.hpp"
#include "optimizer/register_reuse_pass.hpp"

namespace{

using namespace moss;
using namespace opcode;
using namespace testing;

TEST(RegisterReusePass, PathChecks) {
    ustring code = R"(
fun foo(c) {
    a = 4242
    if (a >= c) {
        b = 4242
    } else {
        c = 4
    }
    d = 4
}
)";

    // We should not see any of the function code, just the global scope
    ustring expected = R"(
STORE_INT_CONST  #0, 4242
STORE_INT_CONST  #2, 4
STORE_INT_CONST  #3, 4
STORE_NIL_CONST  #4
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);
    cgen.generate(mod);
    auto pass = new RegisterReusePass();
    std::list<BCPass *> ppln{pass};
    opcode::BCPipeline pipeline(bc, ppln);
    pipeline.run();

    std::stringstream ss;
    ss << "\n";
    for (auto o: bc->get_code()) {
        if (isa<StoreIntConst>(o) || isa<StoreFloatConst>(o)
                || isa<StoreBoolConst>(o) || isa<StoreNilConst>(o)) {
            ss << *o << "\n";
        }
    }
    EXPECT_EQ(ss.str(), expected);

    delete pass;
    delete bc;
    delete mod;
}

TEST(RegisterReusePass, ConstantReuse) {
    ustring code = R"(
a = -999
b = -999

af = 0.2e-7
bf = 2e-8

ab = true
bb = true

an = nil
bn = nil

as1 = "Hello!"
as2 = "Hello!"
)";

    // We should not see any of the function code, just the global scope
    ustring expected = R"(
STORE_INT_CONST  #300, 999
STORE_FLOAT_CONST  #302, 2e-08
STORE_BOOL_CONST  #304, true
STORE_NIL_CONST  #306
STORE_STRING_CONST  #308, "Hello!"
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);
    cgen.generate(mod);
    auto pass = new RegisterReusePass(true);
    std::list<BCPass *> ppln{pass};
    opcode::BCPipeline pipeline(bc, ppln);
    pipeline.run();

    std::stringstream ss;
    ss << "\n";
    for (auto o: bc->get_code()) {
        if (isa<StoreIntConst>(o) || isa<StoreFloatConst>(o)
                || isa<StoreBoolConst>(o) || isa<StoreNilConst>(o)
                || isa<StoreStringConst>(o)) {
            ss << *o << "\n";
        }
    }
    EXPECT_EQ(ss.str(), expected);

    delete pass;
    delete bc;
    delete mod;
}

}