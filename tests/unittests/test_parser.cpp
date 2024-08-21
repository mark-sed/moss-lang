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

// Silent
~42
~nil
)";

    IRType expected[] = {
        IRType::ASSERT,
        IRType::ASSERT,
        
        IRType::RAISE,

        IRType::RETURN,
        IRType::RETURN,

        IRType::CONTINUE,
        IRType::BREAK,

        IRType::SILENT,
        IRType::SILENT,
        
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

TEST(Parser, Expressions){
    ustring code = R"(
4545156
-121
0.054
-0.0
true
false
not true
-----9 // Still valid
not not not false
nil
)";

    IRType expected[] = {
        IRType::INT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::FLOAT_LITERAL,
        IRType::UNARY_EXPR,
        IRType::BOOL_LITERAL,
        IRType::BOOL_LITERAL,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::UNARY_EXPR,
        IRType::NIL_LITERAL,
        
        IRType::END_OF_FILE
    };

    OperatorKind expected_ops[] = {
        OperatorKind::OP_NEG,
        OperatorKind::OP_NEG,
        OperatorKind::OP_NOT,
        OperatorKind::OP_NEG,
        OperatorKind::OP_NOT,
    };

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<Module>(parser.parse());

    int index = 0;
    int op_index = 0;
    for (auto decl: mod->get_body()) {
        EXPECT_TRUE(decl->get_type() == expected[index++]) << "Incorrect IR at index: " << index;
        if (decl->get_type() == IRType::UNARY_EXPR) {
            auto un_expr = dyn_cast<UnaryExpr>(decl);
            EXPECT_TRUE(un_expr->get_op().get_kind() == expected_ops[op_index++]) << "Incorrect Operator at index: " 
                << index << " (" << op_index << ")" << "\nExpected: " << Operator(expected_ops[op_index-1]) << "\nBut got: " << un_expr->get_op();
        }
        else if (decl->get_type() == IRType::BINARY_EXPR) {
            
        }
    }

    delete mod;
}


}