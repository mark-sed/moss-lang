#include "mslib_dict.hpp"

using namespace moss;
using namespace mslib;

Value *Dict::Dict(Interpreter *vm, Value *iterable, Value *&err) {
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

Value *Dict::get(Interpreter *vm, Value *ths, Value *key, Value *def_val, Value *&err) {
    auto dt = dyn_cast<DictValue>(ths);
    assert(dt && "Non-dict passed in");
    auto vals = dt->get_vals();
    auto res = def_val;
    auto dit = vals.find(hash(key, vm));
    if (dit != vals.end()) {
        for (std::pair<Value *, Value *> p: dit->second) {
            if (opcode::eq(p.first, key, vm)) {
                res = p.second;
                break;
            }
        }
    }
    return res;
}

Value *Dict::pop(Interpreter *vm, Value *ths, Value *key, Value *def_val, Value *&err) {
    auto dv = dyn_cast<DictValue>(ths);
    assert(dv && "Dict not passed in");
    opcode::IntConst hsh = 0;
    try {
        hsh = moss::hash(key, vm);
    } catch (Value *v) {
        err = v;
        return nullptr;
    }
    auto found = dv->get_vals().find(hsh);
    if (found != dv->get_vals().end()) {
        auto vals = found->second;
        for (size_t vindex = 0; vindex < vals.size(); ++vindex) {
            try {
                if (opcode::eq(vals[vindex].first, key, vm)) {
                    vals.erase(vals.begin() + vindex);
                    if (vals.empty()) {
                        dv->get_vals().erase(found);
                    }
                    return vals[vindex].second;
                }
            } catch (Value *verr) {
                err = verr;
                return nullptr;
            }
        }
    }
    if (def_val) {
        return def_val;
    }
    err = mslib::create_key_error(diags::Diagnostic(*vm->get_src_file(), diags::KEY_NOT_FOUND, key->as_string().c_str()));
    return nullptr;
}