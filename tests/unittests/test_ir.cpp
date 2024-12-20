#include <gtest/gtest.h>
#include "ir.hpp"
#include "commons.hpp"
#include <sstream>
#include <string>

namespace{

using namespace moss;

/** Test correct casting */
TEST(IR, Casting){
    SourceFile f(SourceFile::SourceType::STDIN);
    ir::Module *m = new ir::Module("m", f, true);
    ir::IR *i = m;

    EXPECT_TRUE(isa<ir::Module>(*i));
    EXPECT_TRUE(isa<ir::Module>(i));
    EXPECT_TRUE(isa<ir::Module>(m));
    EXPECT_TRUE(dyn_cast<ir::Module>(i));

    EXPECT_FALSE(isa<ir::Space>(i));
    EXPECT_FALSE(isa<ir::Space>(m));
    EXPECT_FALSE(dyn_cast<ir::Space>(i));

    delete m;
}

}