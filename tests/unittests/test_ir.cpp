#include <gtest/gtest.h>
#include "ir.hpp"
#include "os_interface.hpp"
#include <sstream>
#include <string>

namespace{

using namespace moss;

/** Test correct casting */
TEST(IR, Casting){
    ir::Module *m = new ir::Module("m");
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