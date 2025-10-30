#include "python.hpp"

using namespace moss;
using namespace mslib;
using namespace python;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& python::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        //{"", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
        //    (void)err;
        //    auto args = cf->get_args();
        //    assert(args.size() == 0);
        //    return nullptr;
        //}},
    };
    return registry;
}

void python::init_constants(Interpreter *vm) {
    
}