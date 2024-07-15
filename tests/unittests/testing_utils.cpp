#include <gtest/gtest.h>
#include <cstdint>
#include "testing_utils.hpp"
#include "values.hpp"

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