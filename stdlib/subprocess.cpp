#include "subprocess.hpp"
#include <cstdlib>

using namespace moss;
using namespace mslib;
using namespace subprocess;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& subprocess::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"system", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return subprocess::system(vm, args[0].value, err);
        }},
    };
    return registry;
}

Value *subprocess::system(Interpreter *vm, Value *cmd, Value *&err) {
    auto cmds = dyn_cast<StringValue>(cmd);
    assert(cmds && "Non-string value passed in");
    int result = std::system(cmds->get_value().c_str());
    return new IntValue(static_cast<opcode::IntConst>(result));
}