#include "mslib_cpp.hpp"
#include "utils.hpp"

using namespace moss;
using namespace mslib;
using namespace t_cpp;
using namespace Cpp;

/*Value *Cpp::to_moss(Interpreter *, Value *v, Value *&err) {
    if (auto c = dyn_cast<t_cpp::CDoubleValue>(v))
        return FloatValue::get(c->get_value());

    err = mslib::create_not_implemented_error("Conversion for returned type is not yet implemented\n");
    return nullptr;
}*/