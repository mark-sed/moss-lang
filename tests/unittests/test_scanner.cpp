#include <gtest/gtest.h>
#include "scanner.hpp"
#include "source.hpp"
#include "commons.hpp"
#include <sstream>
#include <string>

namespace{

using namespace moss;

/** Test correct tokenization */
TEST(Scanner, CharacterTokens){
    ustring code = "( )  {}[];\n,$ ?\\~.. ... . : :: @ @!";
    ustring expected = "LEFT_PAREN WS RIGHT_PAREN WS LEFT_CURLY RIGHT_CURLY LEFT_SQUARE RIGHT_SQUARE END END_NL COMMA NON_LOCAL WS QUESTION_M BACK_SLASH SILENT RANGE WS THREE_DOTS WS DOT WS COLON WS SCOPE WS OUT_ANNOTATION WS IN_ANNOTATION ";

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

// Runs tokenization without WS check the type to stringstream with space as a separator
static void run_tokenizer_check_type(ustring code, TokenType tp) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Scanner scanner(sf);
    Token *t = scanner.next_nonws_token();
    while(t->get_type() != TokenType::END_OF_FILE) {
        EXPECT_TRUE(t->get_type() == tp) << t->get_value() << " is of type " << tp << " but was tokenized as " << t->get_type();
        delete t;
        t = scanner.next_nonws_token();
    }
    delete t;
}

/** Test for operators tokenization */
TEST(Scanner, OperatorTokens){
    ustring code = "++ ++= + += ^ ^= - -= / /= * *= % %=  = == != > >= < <= << && ||";
    ustring expected = "CONCAT SET_CONCAT PLUS SET_PLUS EXP SET_EXP MINUS SET_MINUS DIV SET_DIV MUL SET_MUL MOD SET_MOD SET EQ NEQ BT BEQ LT LEQ UNPACK SHORT_C_AND SHORT_C_OR ";

    std::stringstream ss;
    run_tokenizer(ss, code);

    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

/** Tokens that are keyword */
TEST(Scanner, KeywordTokens){
    ustring code = "and  for  switch  case  try  in  import  as  if  else  while  catch  or  not  xor  do  finally  return  enum  class  assert  fun  nil  true  false  super  this  break  continue  raise  space";
    ustring expected = "AND FOR SWITCH CASE TRY IN IMPORT AS IF ELSE WHILE CATCH OR NOT XOR DO FINALLY RETURN ENUM CLASS ASSERT FUN NIL TRUE FALSE SUPER THIS BREAK CONTINUE RAISE SPACE ";

    std::stringstream ss;
    run_tokenizer(ss, code);

    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

/** Unicode identifiers */
TEST(Scanner, IDsAndUnicodeTokens) {
    ustring code = "🍣sůšo = řeka ça + va j日本語 _ _1 Aa3 hello name__me_2_";
    ustring expected = "ID SET ID ID PLUS ID ID ID ID ID ID ID ";

    std::stringstream ss;
    run_tokenizer(ss, code);

    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

/** Tokenization of numbers */
TEST(Scanner, NumbersTokens) {
    ustring floats_code = "0.0 0000.1 223.2 1e5 2e-32 45e+4 2.15e10 33.00011e-8 0.e+3 12.e-1 1_200.300_001e+1_0 1.000_000_001 3e+1_0 4.e-1_0";
    run_tokenizer_check_type(floats_code, TokenType::FLOAT);

    ustring ints_code = "00 123 0xAe12 0b011001 0q170 5 98984645421215444 0x10 0x1a_2b 0b0101_1001";
    run_tokenizer_check_type(ints_code, TokenType::INT);

    ustring incorrect_nums = "1a 0x 0x12gA 0t1 0q 0b 0b112 1a 0xa.2 42e-12_";
    run_tokenizer_check_type(incorrect_nums, TokenType::ERROR_TOKEN);
}

/** Tokenization of strings */
TEST(Scanner, StringTokens) {
    ustring strs_code = R"(
"" "hello" """hi"""
"Chi +!= ! [] # chi"
"""
Multi
Line
String
"""
""" "" """ """a"b""" """"""
"""\"\"""" "\"" "a\"b..\"c"
"\n\t")";
    ustring expected = "END_NL STRING STRING STRING END_NL STRING END_NL STRING END_NL STRING STRING STRING END_NL STRING STRING STRING END_NL STRING ";

    std::stringstream ss;
    run_tokenizer(ss, strs_code);
    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

/** Tokenization of fstrings */
TEST(Scanner, FStringTokens) {
    {
        ustring strs_code = R"(
f"""
exp{value}"""
f"foovalue"
f"some { x + 25 * 8 }! {{}}"
)";
        ustring expected = "END_NL FSTRING END_NL FSTRING END_NL FSTRING ";

        std::stringstream ss;
        run_tokenizer(ss, strs_code);
        EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
    }


    {
        ustring strs_code = R"(
f"""
exp{"""
)";
        ustring expected = "END_NL ERROR_TOKEN ";

        std::stringstream ss;
        run_tokenizer(ss, strs_code);
        EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
    }

    {
        ustring strs_code = R"(
f"someval {}"
)";
        // 2 errors since '"' will be also parsed
        ustring expected = "END_NL ERROR_TOKEN ERROR_TOKEN ";

        std::stringstream ss;
        run_tokenizer(ss, strs_code);
        EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
    }

    {
        ustring strs_code = R"(
f"{{}"
)";
        ustring expected = "END_NL ERROR_TOKEN ";

        std::stringstream ss;
        run_tokenizer(ss, strs_code);
        EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
    }

    {
        ustring strs_code = R"(
f"""
{\"}"""
)"; // escaped quote outside of string
        ustring expected = "END_NL ERROR_TOKEN ";

        std::stringstream ss;
        run_tokenizer(ss, strs_code);
        EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
    }
}

/** Tokenization of strings */
TEST(Scanner, Comments) {
    ustring cmts_code = R"(
aaa/* comment */bbb
/*
 multi
 comment
 42
        */
ccc // One line comment
// Hello
/**Hello
There)";
    ustring expected = "END_NL ID ID END_NL END_NL END_NL ID END_NL END_NL ERROR_TOKEN ";

    std::stringstream ss;
    run_tokenizer(ss, cmts_code);
    EXPECT_TRUE(ss.str() == expected) << ss.str() << "\n != \n" << expected << "\n";
}

}