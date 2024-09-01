#include <gtest/gtest.h>
#include "clopts.hpp"
#include "errors.hpp"
#include <sstream>
#include <regex>
#include <initializer_list>

namespace{

using namespace moss;

/** Test for options, which terminate the interpreter (-h/-version) */
TEST(CmdOptions, Terminating){
    // help
    const char* argv[] = {
        "moss", "-h"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_EXIT(clopts::parse_clopts(argc, argv), ::testing::ExitedWithCode(0), "");
    
    // version
    const char* argv2[] = {
        "moss", "--version"
    };
    int argc2 = sizeof(argv2) / sizeof(argv2[0]);

    EXPECT_EXIT(clopts::parse_clopts(argc2, argv2), ::testing::ExitedWithCode(0), "");

    // file followed by program argument
    const char* argv3[] = {
        "moss", "--nonexistentflag42", "and one more"
    };
    int argc3 = sizeof(argv3) / sizeof(argv3[0]);

    // Test that we dont exit with other code that 0, force 0 after no exit
    EXPECT_EXIT(clopts::parse_clopts(argc3, argv3), testing::ExitedWithCode(error::ErrorCode::ARGUMENT), ".*Flag could not be matched.*");
}

/** Correct command line options, that should not terminate the program */
TEST(CmdOptions, NonTerminating) {
    // file followed by program argument
    const char* argv[] = {
        "moss", "program.ms", "--nonexistentflag42", "and one more"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    // Test that we dont exit with other code that 0, force 0 after no exit
    ASSERT_EXIT( { clopts::parse_clopts(argc, argv); exit(0); },
            testing::ExitedWithCode(0),
            "");

    clopts::parse_clopts(argc, argv);
    EXPECT_TRUE(args::get(clopts::file_name) == ustring("program.ms"));
    EXPECT_FALSE(clopts::code);

    // code string followed by program argument
    const char* argv1[] = {
        "moss", "-e", "print(\"hi\");", "--nonexistentflag42", "and one more"
    };
    int argc1 = sizeof(argv1) / sizeof(argv1[0]);

    // Test that we dont exit with other code that 0, force 0 after no exit
    EXPECT_EXIT( { clopts::parse_clopts(argc1, argv1); exit(0); },
            testing::ExitedWithCode(0),
            "");

    clopts::parse_clopts(argc1, argv1);
    EXPECT_TRUE(clopts::code);
    EXPECT_FALSE(clopts::file_name);
}

}