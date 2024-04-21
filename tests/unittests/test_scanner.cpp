#include "scanner.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>

namespace{

using namespace moss;

/** Test correct tokenization */
TEST(Scanner, CharacterTokens){
    ustring code = "( )  {}[];\n,$ ?\\~.. . : :: @ @!";
    ustring expected = "LEFT_PAREN WS RIGHT_PAREN WS LEFT_CURLY RIGHT_CURLY LEFT_SQUARE RIGHT_SQUARE END END_NL COMMA NON_LOCAL WS QUESTION_M BACK_SLASH SILENT RANGE WS DOT WS COLON WS SCOPE WS OUT_ANNOTATION WS IN_ANNOTATION ";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Scanner scanner(sf);

    Token *t = scanner.next_token();
    std::stringstream ss;
    while(t->get_type() != TokenType::END_OF_FILE) {
        ss << t->get_type() << " ";
        delete t;
        t = scanner.next_token();
    }
    delete t;

    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

// Runs tokenization without WS outputting the type to stringstream with space as a separator
static void run_tokenizer(std::stringstream &ss, ustring code) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Scanner scanner(sf);
    Token *t = scanner.next_nonws_token();
    while(t->get_type() != TokenType::END_OF_FILE) {
        ss << t->get_type() << " ";
        delete t;
        t = scanner.next_nonws_token();
    }
    delete t;
}

/** Test for operators tokenization */
TEST(Scanner, OperatorTokens){
    ustring code = "++ ++= + += ^ ^= - -= / /= * *= % %=  = == != > >= < <= << && ||";
    ustring expected = "CONCAT SET_CONCAT PLUS SET_PLUS EXP SET_EXP MINUS SET_MINUS DIV SET_DIV MUL SET_MUL MOD SET_MOD SET EQ NEQ BT BEQ LT LEQ UNWRAP SHORT_C_AND SHORT_C_OR ";

    std::stringstream ss;
    run_tokenizer(ss, code);

    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

/** Tokens that are keyword */
TEST(Scanner, KeywordTokens){
    // TODO
}

}