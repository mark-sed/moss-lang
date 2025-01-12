#include "gc.hpp"
#include "logging.hpp"

using namespace moss;
using namespace gcs;

TracingGC::TracingGC(Interpreter *vm) : vm(vm) {
    LOGMAX("Initializing Tracing GC");
}

void TracingGC::sweep() {
    std::list<Value *>::iterator i = Value::all_values.begin();
    while (i != Value::all_values.end()) {
        auto v = *i;
        assert(v && "nullptr not removed from all_values?");
        if (v->is_marked()) {
            v->set_marked(false);
            ++i;
        }
        else {
            // Not used value
            // We cannot output using the debug method as some parts of a structure
            // might have been deleted and then the structure itself will be deleted
            LOGMAX("Deleting: " << TypeKind2String(v->get_kind()) << "(" << v->get_name() << ")");
            i = Value::all_values.erase(i);
            delete v;
        }
    }
}

void TracingGC::blacken_value(Value *v) {
    LOGMAX("Blacken: " << *v);
    // Every value has a type
    mark_value(v->get_type());
    // Values might have annotation
    for (auto [_, a]: v->get_annotations()) {
        mark_value(a);
    }
    // Value might have attributes
    if (v->get_attrs()) {
        LOGMAX(v->get_attrs());
        mark_frame(v->get_attrs());
    }

    if (auto subv = dyn_cast<ListValue>(v)) {
        for (auto v: subv->get_vals()) {
            mark_value(v);
        }
    }
    else if (auto subv = dyn_cast<ClassValue>(v)) {
        for (auto s : subv->get_supers()) {
            mark_value(s);
        }
    }
    else if (auto subv = dyn_cast<FunValue>(v)) {
        for (auto arg: subv->get_args()) {
            for (auto t: arg->types) {
                mark_value(t);
            }
            mark_value(arg->default_value);
        }
    }
    else if (auto subv = dyn_cast<FunValueList>(v)) {
        for (auto fv: subv->get_funs()) {
            mark_value(fv);
        }
    }
    else if (auto subv = dyn_cast<EnumTypeValue>(v)) {
        for (auto v: subv->get_values()) {
            mark_value(v);
        }
    }
}

void TracingGC::trace_refs() {
    while (!gray_list.empty()) {
        Value *v = gray_list.back();
        gray_list.pop_back();
        blacken_value(v);
    }
}

void TracingGC::mark_value(Value *v) {
    if (!v || v->is_marked())
        return;
    v->set_marked(true);
    gray_list.push_back(v);
    LOGMAX("Marked: " << v->get_name() << " = " << *v)
}

void TracingGC::mark_frame(MemoryPool *p) {
    assert(p && "passed in nullptr memory pool");
    // There will be bunch of nullptrs as the pool is initialized that way
    for (auto v : p->get_pool()) {
        mark_value(v);
    }
}

void TracingGC::mark_roots() {
    for (auto mem: vm->frames) {
        mark_frame(mem);
    }

    // We need to mark also constant pools
    for (auto mem: vm->const_pools) {
        mark_frame(mem);
    }

    // Mark values pushed as parents
    for (auto pl: vm->parent_list) {
        mark_value(pl);
    }

    // Call frame marking
    for (auto cf: vm->call_frames) {
        for (auto arg: cf->get_args()) {
            mark_value(arg.value);
        }
        mark_value(cf->get_extern_return_value());
    }
}

void TracingGC::collect_garbage() {
    LOG1("Running GC");
#ifndef NDEBUG
    auto pre_run_allocations = Value::allocated_bytes;
#endif
    gray_list.clear();
    // gray list is empty and values are not marked, so all values are white
    mark_roots();
    // values in gray list (and marked also) are gray
    trace_refs();
    // Value is black when it is not in gray stack and marked is set
    sweep();
    // Freed not used values

#ifndef NDEBUG
    LOGMAX("Freed " << (pre_run_allocations - Value::allocated_bytes) << "B");
    if (clopts::stress_test_gc) {
        LOGMAX("Finished GC (stress test so not modifying next_gc)");
        return;
    }
#endif

    // TODO: Keep this? If we have trigger also by time then maybe remove this
    // If enought values were freed there is no need to have such a high next_gc, reset it.
    if ((Value::next_gc / global_controls::gc_grow_factor) / 2 > Value::allocated_bytes) {
        Value::next_gc /= global_controls::gc_grow_factor;
    }
    LOG1("Finished GC");
}