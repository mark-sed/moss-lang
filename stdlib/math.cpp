#include "math.hpp"
#include <cmath>

using namespace moss;
using namespace mslib;
using namespace math;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& math::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"atan", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::atan(cf->get_args()[0].value->as_float()));
        }},
        {"atan2", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::atan2(cf->get_arg("x")->as_float(), cf->get_arg("y")->as_float()));
        }},
    };
    return registry;
}
