#include "math.hpp"
#include <cmath>
#include <numeric>
#include <functional>

using namespace moss;
using namespace mslib;
using namespace math;

template <typename Op>
opcode::IntConst gcd_or_lcm_all(std::vector<opcode::IntConst> vals, Op op) {
    auto it = vals.begin();
    opcode::IntConst result = *it++;
    for (; it != vals.end(); ++it)
        result = op(result, *it);
    return result;
}

Value *gcd_or_lcm(Interpreter *vm, CallFrame *cf, bool is_gcd, Value *ints, Value *&err) {
    auto valsv = mslib::get_list(ints);
    if (valsv.empty()) {
        return IntValue::get(0);
    }
    std::vector<opcode::IntConst> as_i;
    as_i.reserve(valsv.size());
    // Check that all values are ints
    for (size_t i = 0; i < valsv.size(); ++i) {
        auto iv = valsv[i];
        if (auto intval = dyn_cast<IntValue>(iv)) {
            as_i.push_back(intval->get_value());
        } else {
            err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::GCD_OR_LCM_EXPECTS_INTS,
                    is_gcd ? "gcd" : "lcm", i, iv->get_type()->get_name().c_str()));
            return nullptr;
        }
    }
    if (valsv.size() == 1)
        return valsv[0];
    
    using Op = opcode::IntConst (*)(opcode::IntConst, opcode::IntConst);
    auto result = gcd_or_lcm_all(as_i, is_gcd ? static_cast<Op>(std::gcd) : static_cast<Op>(std::lcm));
    return IntValue::get(result);
}

const std::unordered_map<std::string, mslib::mslib_dispatcher>& math::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"atan", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::atan(cf->get_args()[0].value->as_float()));
        }},
        {"atan2", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::atan2(cf->get_arg("x")->as_float(), cf->get_arg("y")->as_float()));
        }},
        {"cosh", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::cosh(cf->get_args()[0].value->as_float()));
        }},
        {"gcd", [](Interpreter *vm, CallFrame* cf, Value*&err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            return gcd_or_lcm(vm, cf, true, args[0].value, err);
        }},
        {"isnan", [](Interpreter*, CallFrame* cf, Value*&) {
            return BoolValue::get(std::isnan(cf->get_args()[0].value->as_float()));
        }},
        {"lcm", [](Interpreter *vm, CallFrame* cf, Value*&err) {
            auto args = cf->get_args();
            assert(args.size() == 1);
            return gcd_or_lcm(vm, cf, false, args[0].value, err);
        }},
        {"sinh", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::sinh(cf->get_args()[0].value->as_float()));
        }},
        {"tanh", [](Interpreter*, CallFrame* cf, Value*&) {
            return FloatValue::get(std::tanh(cf->get_args()[0].value->as_float()));
        }},
        
    };
    return registry;
}
