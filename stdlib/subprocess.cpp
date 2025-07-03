#include "subprocess.hpp"

using namespace moss;
using namespace mslib;
using namespace subprocess;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& subprocess::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"run", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return subprocess::run(vm, args[0].value, err);
        }},
    };
    return registry;
}

Value *subprocess::run(Interpreter *vm, Value *cmd, Value *&err) {
    // TODO: Finish
    return nullptr;
}