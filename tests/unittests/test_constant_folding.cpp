#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "transforms/constant_folding.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace ir;
using namespace testing;

ustring process_and_fold(ustring code) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());
    assert(mod && "failed parsing");
    ir::IRPipeline irp(parser);
    irp.get_pm().clear_passes();
    auto aip = new ConstantFoldingPass(parser);
    irp.add_pass(aip);
    auto err = irp.run(mod);
    assert(!err && "failed pipeline");

    std::stringstream ss;
    for (auto decl: mod->get_body()) {
        ss << *decl << "\n";
    }
    return ss.str();
}

/// Test Int and Float folding
TEST(ConstantFolding, IntAndFloatFolding){
    ustring code = R"(a = 40 + 2
af = 1.1 + 2.2 + 3.3 + 4 + 0.0

b = 0 - 9 - 128
bf = 9.9 - 8.8 - 0.2

c = 2 ^ 2 ^ 3
cf = 100 ^ 1.52

d = 10000 / 1000 / 2
df = 82.8 / 0.5 / 1.0

e = 7 * 2 * 4 * 1 * 1
ef = 1.0 * 1.0 * 1.2

f = 40 % 11 % 3
ff = 40.8 % 2.2

g = 255 and 0xF and 0xFF

h = 0xF0 or 0xF

i = 123 xor 123

j = 1 > 3
jf = 9.2 > 0.9

k = 800 < 124
kf = 9 < 9.1

l = 100 >= 102
lf = 2.0 >= 0.1

m = 10 <= 2
mf = 10.1 <= 12.2

n = 10 == 11
nf = 10.0 == 10

o = 1 == 23
of = 0.0 == 0.000

/// Complex

c1 = 1 + 2 * 3 + 4 / 2.0 ^ 1 - 3

fun foo() {
    return 3.4 * 3 + 2
}

for (i : 0..2*2) {
    i
}
)";

    ustring expected = R"((a = 42)
(af = 10.6)
(b = -137)
(bf = 0.9)
(c = 256)
(cf = 1096.48)
(d = 5)
(df = 165.6)
(e = 56)
(ef = 1.2)
(f = 1)
(ff = 1.2)
(g = 15)
(h = 255)
(i = 0)
(j = false)
(jf = true)
(k = false)
(kf = true)
(l = false)
(lf = true)
(m = false)
(mf = true)
(n = false)
(nf = true)
(o = false)
(of = true)
(c1 = 6)
fun foo() {
return 12.2
}
for (i: (0..4)) {
i
}
<IR: <end-of-file>>
)";

    EXPECT_EQ(process_and_fold(code), expected);
}

/// Test Bool folding
TEST(ConstantFolding, BoolFolding){
    ustring code = R"(b1 = true and true and true
b2 = true and false and true

b3 = true or false or false
b4 = false or false or false or false or false

b5 = true xor true xor true
b6 = false xor false

b7 = true == true == true
b8 = false == true

b9 = true != true
b10 = false != true

b11 = 4 == 4 == true

fun foo(a=true or false) {
    return true and true
}
)";

    ustring expected = R"((b1 = true)
(b2 = false)
(b3 = true)
(b4 = false)
(b5 = true)
(b6 = false)
(b7 = true)
(b8 = false)
(b9 = false)
(b10 = true)
(b11 = true)
fun foo(a=true) {
return true
}
<IR: <end-of-file>>
)";

    EXPECT_EQ(process_and_fold(code), expected);
}

/// Test String folding
TEST(ConstantFolding, StringFolding){
    ustring code = R"(s1 = "Hi, " == "there"

s3 = "h" != "h"
s4 = "" != "sdfs"

s5 = "abc" < "bcd"
s6 = "aabc" < "aaabc"

s7 = "abc" > "bcd"
s8 = "aabc" > "aaabc"

s9 = "a" >= "a"
s10 = "ab" >= "a"

s11 = "" <= ""

s12 = "abc" in "--abc--"
s13 = "H" in "hello"
)";

    ustring expected = R"((s1 = false)
(s3 = false)
(s4 = true)
(s5 = true)
(s6 = false)
(s7 = false)
(s8 = true)
(s9 = true)
(s10 = true)
(s11 = true)
(s12 = true)
(s13 = false)
<IR: <end-of-file>>
)";

    EXPECT_EQ(process_and_fold(code), expected);
}

}