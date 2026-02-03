#include <gtest/gtest.h>
#include "commons.hpp"
#include "parser.hpp"
#include "source.hpp"
#include "interpreter.hpp"
#include "bytecodegen.hpp"
#include "gc.hpp"
#include <sstream>
#include <string>

using namespace moss;
using namespace testing;

static bool has_popped_frame(const gcs::TracingGC *gc, Value *owner) {
    for (auto f: gc->get_popped_frames()) {
        if (f->get_pool_owner() == owner) {
            return true;
        }
    }
    return false;
}

static bool value_exists_on_heap(const Value *val) {
    for (auto v: Value::all_values) {
        if (v == val)
            return true;
    }
    return false;
}

static void init_gc() {
    Value::next_gc = std::numeric_limits<size_t>::max();
    Value::all_values.clear();
    gcs::TracingGC::re_init_gc();
}

static void deinit_gc() {
    Value::next_gc = 1024 * 1024;
}

/** Testing garbage collection */
TEST(GarbageCollection, BasicFrameDeletion){
    init_gc();

    ustring code = R"(
fun foo() {
    a = 3400 // Has to be big to not get interned
    return 5
}

~foo()
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc, &parser);
    cgen.generate(mod);

    Interpreter *i = new Interpreter(bc, &sf, true);
    i->run();

    auto foo = i->load_name("foo");
    ASSERT_TRUE(foo);

    EXPECT_EQ(i->get_exit_code(), 0);
    
    auto gc = i->__get_gc();
    ASSERT_TRUE(gc);

    auto frames = i->get_frames().size();
    auto callframes = i->get_call_frame_size();
    
    EXPECT_EQ(frames, 1);
    EXPECT_EQ(callframes, 0);

    // There ale libms frames in GC, so look for frame that was foo's.
    EXPECT_TRUE(has_popped_frame(gc, foo)) << "frame of foo is not in gc frames to free";

    // Check that "a" is still in memory
    Value *a = nullptr;
    for (auto f: gc->get_popped_frames()) {
        if (f->get_pool_owner() == foo) {
            a = f->load_name("a", i);
            break;
        }
    }
    ASSERT_TRUE(a) << "a is not in frame";
    EXPECT_TRUE(value_exists_on_heap(a));

    i->collect_garbage();

    // GC will have popped frames from moss, which cannot be cleared (are referenced).
    EXPECT_FALSE(has_popped_frame(gc, foo));
    EXPECT_FALSE(value_exists_on_heap(a));

    delete i;
    delete bc;
    delete mod;

    deinit_gc();
}

TEST(GarbageCollection, Loops){
    init_gc();

    ustring code = R"(
fun foo() {
    a = 1000 // Avoid using value which could be interned
    while (a < 1010) {
        space Fpp {

        }
        a += 1
    }
}

~foo()
)";

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());

    auto bc = new Bytecode();
    bcgen::BytecodeGen cgen(bc, &parser);
    cgen.generate(mod);

    Interpreter *i = new Interpreter(bc, &sf, true);
    i->run();

    auto foo = i->load_name("foo");
    ASSERT_TRUE(foo);

    EXPECT_EQ(i->get_exit_code(), 0);
    
    auto gc = i->__get_gc();
    ASSERT_TRUE(gc);

    auto frames = i->get_frames().size();
    auto callframes = i->get_call_frame_size();
    
    EXPECT_EQ(frames, 1);
    EXPECT_EQ(callframes, 0);

    // There ale libms frames in GC, so look for frame that was foo's.
    EXPECT_TRUE(has_popped_frame(gc, foo)) << "frame of foo is not in gc frames to free";

    // Check that "a" is still in memory
    Value *a = nullptr;
    Value *Fpp = nullptr;
    for (auto f: gc->get_popped_frames()) {
        if (f->get_pool_owner() == foo) {
            a = f->load_name("a", i);
            Fpp = f->load_name("Fpp", i);
            break;
        }
    }
    ASSERT_TRUE(a) << "a is not in frame";
    EXPECT_TRUE(value_exists_on_heap(a));
    ASSERT_TRUE(Fpp) << "Fpp is not in frame";
    EXPECT_TRUE(value_exists_on_heap(Fpp));

    i->collect_garbage();

    // GC will have popped frames from moss, which cannot be cleared (are referenced).
    EXPECT_FALSE(has_popped_frame(gc, foo));
    EXPECT_FALSE(value_exists_on_heap(Fpp));
    EXPECT_FALSE(value_exists_on_heap(a));

    delete i;
    delete bc;
    delete mod;

    deinit_gc();
}