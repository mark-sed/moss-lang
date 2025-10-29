#include <gtest/gtest.h>
#include <cstdint>
#include "testing_utils.hpp"
#include "values.hpp"
#include "parser.hpp"
#include "ir_pipeline.hpp"

using namespace testing;
using namespace moss;

int64_t testing::int_val(Value *v) {
    IntValue *iv = dyn_cast<IntValue>(v);
    EXPECT_TRUE(iv != nullptr) << "Value is not an Integer";
    if(!iv) return 0;
    return iv->get_value();
}

double testing::float_val(Value *v) {
    FloatValue *fv = dyn_cast<FloatValue>(v);
    EXPECT_TRUE(fv != nullptr) << "Value is not a Float";
    if(!fv) return 0;
    return fv->get_value();
}

bool testing::bool_val(Value *v) {
    BoolValue *bv = dyn_cast<BoolValue>(v);
    EXPECT_TRUE(bv != nullptr) << "Value is not a Boolean";
    if(!bv) return 0;
    return bv->get_value();
}

ustring testing::string_val(Value *v) {
    StringValue *sv = dyn_cast<StringValue>(v);
    EXPECT_TRUE(sv != nullptr) << "Value is not a String";
    if(!sv) return "";
    return sv->get_value();
}

void testing::check_all_lines_err(std::vector<ustring> lines, ustring test) {
    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod) << test << ": " << code << "\n";
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_TRUE(err) << test << ": " << code << "\n";
        auto rs = dyn_cast<ir::Raise>(err);
        ASSERT_TRUE(rs) << test << ": " << code << "\n";

        delete mod;
        delete err;
    }
}

void testing::check_all_lines_ok(std::vector<ustring> lines, ustring test) {
    for (auto code: lines) {
        SourceFile sf(code, SourceFile::SourceType::STRING);
        Parser parser(sf);

        auto mod = dyn_cast<ir::Module>(parser.parse());
        ASSERT_TRUE(mod) << test << ": " << code << "\n";
        ir::IRPipeline irp(parser);
        auto err = irp.run(mod);
        ASSERT_FALSE(err) << test << ": " << code << "\n";

        delete mod;
        delete err;
    }
}

void testing::check_line_err(ustring code, ustring test) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ASSERT_TRUE(mod) << test << ": " << code;
    ir::IRPipeline irp(parser);
    auto err = irp.run(mod);
    ASSERT_TRUE(err) << test << ": " << code;
    auto rs = dyn_cast<ir::Raise>(err);
    ASSERT_TRUE(rs) << test << ": " << code;

    delete mod;
    delete err;
}

void testing::check_line_ok(ustring code, ustring test) {
    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ASSERT_TRUE(mod) << test << ": " << code;
    ir::IRPipeline irp(parser);
    auto err = irp.run(mod);
    ASSERT_FALSE(err) << test << "Failed but was supposed to pass: " << code;

    delete mod;
    delete err;
}