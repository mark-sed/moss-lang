#include "warnings.hpp"
#include "moss.hpp"
#include "builtins.hpp"
#include "errors.hpp"

using namespace moss;
using namespace mslib;
using namespace warnings;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& warnings::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"warn", [](Interpreter*, CallFrame* cf, Value*&) -> Value* {
            auto args = cf->get_args();
            assert(args.size() == 1);
            ustring msg = args[0].value->as_string();
            error::warning(msg.c_str());
            return nullptr;
        }},
    };
    return registry;
}