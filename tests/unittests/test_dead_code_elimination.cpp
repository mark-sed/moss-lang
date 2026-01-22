#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "transforms/dead_code_elimination.hpp"
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"

namespace{

using namespace moss;
using namespace ir;
using namespace testing;

ustring process_and_dce(ustring code) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());
    assert(mod && "failed parsing");
    ir::IRPipeline irp(parser);
    irp.get_pm().clear_passes();
    auto aip = new DeadCodeEliminationPass(parser);
    irp.add_pass(aip);
    auto err = irp.run(mod);
    assert(!err && "failed pipeline");

    std::stringstream ss;
    for (auto decl: mod->get_body()) {
        ss << *decl << "\n";
    }
    return ss.str();
}

size_t count_substrings(const std::string& str, const std::string& sub) {
    size_t count = 0;
    auto it = str.begin();

    while ((it = std::search(it, str.end(), sub.begin(), sub.end())) != str.end()) {
        ++count;
        it += sub.size();
    }

    return count;
}

/// Test deletion of code after known return
TEST(DeadCodeElimination, CodeAfterReturn){
    ustring code = R"(
fun foo() {
    "hi\n"
    return 1
    "dead"
    "code"
    a = 42
}

fun foo2(a, b) {
    if (a)
        return 3
    ab = 43
    if (b)
        if (ab > 4)
            return 3
    ac = 58
    return
    return 4
    "hey"
    ad = 8
}
)";

    ustring expected = R"(fun foo() {
"hi\n"
return 1
}
fun foo2(a, b) {
if (a) {
return 3
}
(ab = 43)
if (b) {
if ((ab > 4)) {
return 3
}
}
(ac = 58)
return nil
}
<IR: <end-of-file>>
)";

    EXPECT_EQ(process_and_dce(code), expected);
}

/// Test deletion of code break and continue
TEST(DeadCodeElimination, CodeAfterContinueBreak){
    ustring code = R"(
for (a : [1,2,3]) {
    a
    break
    "dead code"
    c = 32
}

fun f(a3) {
    while (a3 < 5) {
        for (a : [1,2,3]) {
            a
            continue
            "dead code"
            c = 32
        }
        break
        "dead code"
        cf = 58
    }
}

do {
    continue
    "dead code"
    if (args[5]) continue
} while (args.length() > 5)

while (true) {
    do {
        while (true) {
            break
            continue
            "dead code"
        }
        continue
        "dead code"
    } while (false)
    break
    "dead code"
}

for (c: 1..6) {
    if (c == 10)
        break
    a = "check"
}

while (args.length() == 1) {
    if (args[0] == 3) {
        continue
        "dead code"
    }
    c = "check"
    if (args[0] == 56) {
        break
        "dead code"
    }
    if (args[0] == 0) {

    } else {
        "check"
        break
        "dead code"
    }
}

for (a : 1..3) {
    try {
        a()
        break
        "dead code"
    } catch (e) {
        "check"
        continue
        "dead code"
    } finally {
        "check"
        break
        b = "dead code"
    }
}

while(true) {
    switch(args.length()) {
        case 1: {
            break
            "dead code"
        }
        case 2: {
            "check"
            continue
            "dead code"
        }
        default: {
            break
            "dead code"
        }
    }
}
)";

    ustring proc_ir = process_and_dce(code);
    // Check that no "check"s were deleted, these are possible deletions
    EXPECT_EQ(count_substrings(proc_ir, "check"), count_substrings(code, "check")) << "Parsed: " << proc_ir;
    EXPECT_EQ(count_substrings(proc_ir, "dead code"), 0) << "Parsed: " << proc_ir;
}

}