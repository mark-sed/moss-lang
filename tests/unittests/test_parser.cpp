#include <gtest/gtest.h>
#include "parser.hpp"
#include "source.hpp"
#include "ir.hpp"
#include "os_interface.hpp"
#include <sstream>
#include <string>

namespace{

using namespace moss;
using namespace ir;

/** Test correct parsing regardless of the new line */
TEST(Parser, WhiteSpace){
    ustring code = R"(


// Comment

  /* Another comment


  */ // And here as well


// Comment
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    ASSERT_TRUE(mod->size() == 1) << "Empty program has more/less than just EOF";
    EXPECT_TRUE(isa<EndOfFile>(mod->back()));

    delete mod;
}

TEST(Parser, Statements){
    ustring code = R"(
// Assert
assert(a, b);
assert(cond)

// Raise
raise except

// Return
return
return value

// Continue
continue

// Break
break
)";

    IRType expected[] = {
        IRType::ASSERT,
        IRType::ASSERT,
        
        IRType::RAISE,

        IRType::RETURN,
        IRType::RETURN,

        IRType::CONTINUE,
        IRType::BREAK,
        
        IRType::END_OF_FILE
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index;
    }

    delete mod;
}


}