#include "mslib_list.hpp"

using namespace moss;
using namespace mslib;

Value *List::append(Interpreter *vm, Value * ths, Value *v, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    // v is vararg
    auto vls = dyn_cast<ListValue>(v);
    assert(vls && "not vararg");
    for (auto e: vls->get_vals())
        lv->push(e);
    return BuiltIns::Nil;
}

Value *List::pop(Interpreter *vm, Value *ths, Value *index, Value *&err) {
    auto lv = dyn_cast<ListValue>(ths);
    if (!lv) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::BAD_OBJ_PASSED, ths->get_type()->get_name().c_str()));
        return BuiltIns::Nil;
    }
    auto iv = dyn_cast<IntValue>(index);
    assert(iv);
    auto ivint = iv->get_value();
    Value *removed = nullptr;
    if ((ivint < 0 && static_cast<unsigned long>(ivint*-1) > lv->get_vals().size()) || 
        (ivint >= 0 && static_cast<unsigned long>(ivint) >= lv->get_vals().size())) {
            err = mslib::create_index_error(diags::Diagnostic(*vm->get_src_file(), diags::OUT_OF_BOUNDS, lv->get_type()->get_name().c_str(), ivint));
        return BuiltIns::Nil;
    }
    if (ivint < 0) {
        removed = lv->get_vals()[lv->size() + ivint];
        lv->remove(lv->size() + ivint);
    } else {
        removed = lv->get_vals()[ivint];
        lv->remove(ivint);
    }
    return removed;
}

Value *List::count(Interpreter *vm, Value *ths, Value *val, Value *&) {
    auto ls = mslib::get_list(ths);
    return IntValue::get(std::count_if(ls.begin(), ls.end(),
        [&](Value *v) {
            return opcode::eq(val, v, vm);
    }));
}

Value *List::insert(Interpreter *vm, Value *ths, Value *index, Value *value, Value *&err) {
    auto &lst = get_list(ths);
    auto idx = get_int(index);
    if (idx < 0)
        idx = 0;
    else if (idx > lst.size())
        idx = idx = lst.size();

    lst.insert(lst.begin() + idx, value);
    return nullptr;
}

Value *List::List(Interpreter *vm, Value *iterable, Value *&err) {
    auto iterator = iterable->iter(vm);
    std::vector<Value *> lv;
    try {
        while(true) {
            auto ev = iterator->next(vm);
            lv.push_back(ev);
        }
    } catch (Value *exc) {
        if (exc->get_type() != BuiltIns::StopIteration) {
            err = exc;
            return nullptr;
        }
    }
    return new ListValue(lv);
}