#include "inspect.hpp"
#include "opcode.hpp"
#include "interpreter.hpp"

using namespace moss;
using namespace mslib;
using namespace inspect;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& inspect::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"enum_values", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            auto ev = dyn_cast<EnumTypeValue>(cf->get_args()[0].value);
            assert(ev && "Other type than enum passed in");
            auto evvl = ev->get_values();
            std::vector<Value *> en_vals;
            en_vals.reserve(evvl.size());
            for (auto e: evvl) {
                en_vals.push_back(e);
            }
            return new ListValue(en_vals);
        }},
        {"name", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            auto arg = cf->get_args()[0].value;
            if (auto fvl = dyn_cast<FunValueList>(arg)) {
                arg = fvl->get_funs()[0];
            }
            return new StringValue(arg->get_name());
        }},
        {"signature", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            (void)vm;
            assert(cf->get_args().size() == 1);
            auto fv = dyn_cast<FunValue>(cf->get_args()[0].value);
            assert(fv && "Other type than function passed in");
            return new StringValue(fv->get_signature());
        }},
    };
    return registry;
}