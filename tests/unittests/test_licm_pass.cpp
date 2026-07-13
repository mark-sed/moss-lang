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
#include "optimizer/LICM_pass.hpp"

namespace{

using namespace moss;
using namespace opcode;
using namespace testing;

TEST(LICMPass, ConstatStoreHoisting) {
    ustring code = R"(
for (i: 0..args.length()) {
    i * 2.0 + 42 ++ "\n"
}
)";

    // We should not see any of the function code, just the global scope
    ustring expected = R"(
STORE_INT_CONST  #301, 0
STORE_FLOAT_CONST  #303, 2
STORE_INT_CONST  #304, 42
STORE_STRING_CONST  #305, "\n"
LOOP_BEGIN
LOOP_END
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc);
    cgen.generate(mod);
    auto pass = new LICMPass();
    std::list<BCPass *> ppln{pass};
    opcode::BCPipeline pipeline(bc, ppln);
    pipeline.run();

    std::stringstream ss;
    ss << "\n";
    for (auto o: bc->get_code()) {
        if (isa<StoreIntConst>(o) || isa<StoreFloatConst>(o) || isa<StoreStringConst>(o)
                || isa<StoreBoolConst>(o) || isa<LoopBegin>(o) || isa<LoopEnd>(o)) {
            ss << *o << "\n";
        }
    }
    EXPECT_EQ(ss.str(), expected);

    delete pass;
    delete bc;
    delete mod;
}

}