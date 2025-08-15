#include "mslib_dict.hpp"

using namespace moss;
using namespace mslib;

Value *Dict::Dict(Interpreter *vm, Value *ths, Value *iterable, Value *&err) {
    (void)ths;
    auto iterator = iterable->iter(vm);
    DictValue *dv = new DictValue();
    try {
        while(true) {
            auto elem = iterator->next(vm);
            auto elem_lst = dyn_cast<ListValue>(elem);
            if (!elem_lst) {
                err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::DICT_UNEXPECTED_TYPE, elem->get_type()->get_name().c_str()));
                return nullptr;
            }
            if (elem_lst->size() != 2) {
                err = mslib::create_type_error(diags::Diagnostic(*vm->get_src_file(), diags::DICT_BAD_ITER_SIZE, elem_lst->size()));
                return nullptr;
            }
            dv->push(elem_lst->get_vals()[0], elem_lst->get_vals()[1], vm);
        }
    } catch (Value *exc) {
        if (exc->get_type() != BuiltIns::StopIteration) {
            err = exc;
            return nullptr;
        }
    }
    return dv;
}