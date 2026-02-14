#include "gc.hpp"
#include "values.hpp"
#include "logging.hpp"
#include <unordered_set>

using namespace moss;
using namespace gcs;

std::vector<ModuleValue *> TracingGC::currently_imported_modules{};
std::list<MemoryPool *> TracingGC::popped_frames{};
std::list<Interpreter *> TracingGC::vms{};

void TracingGC::push_currently_imported_module(ModuleValue *m) {
    currently_imported_modules.push_back(m);
}

void TracingGC::pop_currently_imported_module() {
    currently_imported_modules.pop_back();
}

void TracingGC::push_vm(Interpreter *vm) {
    vms.push_back(vm);
}

void TracingGC::push_popped_frame(MemoryPool *f) {
    if (f->is_global())
        return; // Globals are deleted by VM
    popped_frames.push_back(f);
}

#ifndef NDEBUG
ModuleValue *TracingGC::top_currently_imported_module() {
    return currently_imported_modules.back();
}
#endif

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

    // Remove duplicates
    // This should be done here as this will be run once when GC runs but
    // but push_popped_frame every time when value with attrs is deleted
    std::unordered_set<MemoryPool*> seen;
    for (auto it = popped_frames.begin(); it != popped_frames.end(); ) {
        if (seen.find(*it) != seen.end()) {
            it = popped_frames.erase(it); // Duplicate: erase
        } else {
            seen.insert(*it);
            ++it; // Unique: keep going
        }
    }

    std::list<MemoryPool *>::iterator fi = popped_frames.begin();
    while (fi != popped_frames.end()) {
        auto f = *fi;
        assert(f && "nullptr not removed from popped frames?");
        if (f->is_marked()) {
            f->set_marked(false);
            ++fi;
        }
        else {
            LOGMAX("Deleting frame");
            fi = popped_frames.erase(fi);
            delete f;
        }
    }

    std::list<Interpreter *>::iterator vmi = vms.begin();
    while (vmi != vms.end()) {
        auto v = *vmi;
        assert(v && "nullptr not removed from vms?");
        if (v->is_marked()) {
            v->set_marked(false);
            ++vmi;
        }
        else {
            LOGMAX("Deleting VM " << (v->get_src_file() ? v->get_src_file()->get_module_name() : "?"));
            vmi = vms.erase(vmi);
            delete v;
        }
    }
}

void TracingGC::blacken_value(Value *v) {
    //LOGMAX("Blacken: " << *v);
    // Every value has a type
    mark_value(v->get_type());
    mark_value(v->get_owner());
    // Values might have annotation
    for (auto [_, a]: v->get_annotations()) {
        mark_value(a);
    }
    // Value might have attributes
    if (v->get_attrs()) {
        mark_frame(v->get_attrs());
    }

    if (auto subv = dyn_cast<ListValue>(v)) {
        for (auto v: subv->get_vals()) {
            mark_value(v);
        }
    }
    else if (auto subv = dyn_cast<DictValue>(v)) {
        for (auto [_, vals]: subv->get_vals()) {
            for (auto [k, v]: vals) { 
                mark_value(k);
                mark_value(v);
            }
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
        for (auto f: subv->get_closures()) {
            mark_frame(f);
        }
        mark_roots(subv->get_vm());
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
    else if (auto subv = dyn_cast<ModuleValue>(v)) {
        LOGMAX("Blackening module: " << subv->get_name());
        mark_roots(subv->get_vm());
    }
    else if (auto subv = dyn_cast<SuperValue>(v)) {
        mark_value(subv->get_instance());
    }
    else if (auto spcv = dyn_cast<SpaceValue>(v)) {
        for (auto o: spcv->get_extra_owners()) {
            mark_value(o);
        }
        mark_roots(spcv->get_owner_vm());
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
    // Mark the frame itself as popped frames need to be freed by the GC
    p->set_marked(true);
    // There will be bunch of nullptrs as the pool is initialized that way
    for (auto [k, v] : p->get_pool()) {
        mark_value(v);
    }
    // Spilled values
    for (auto v: p->get_spilled_values()) {
        mark_value(v);
    }
    for (auto c: p->get_catches()) {
        mark_value(c.type);
    }
    mark_roots(p->get_vm_owner());
}

void TracingGC::mark_roots(Interpreter *ivm) {
    if (!ivm || ivm->is_marked())
        return;
    ivm->set_marked(true);
    LOGMAX("Marked VM: " <<  (ivm->get_src_file() ? ivm->get_src_file()->get_module_name() : "?"));

    for (auto mem: ivm->frames) {
        mark_frame(mem);
    }

    // We need to mark also constant pools
    for (auto mem: ivm->const_pools) {
        mark_frame(mem);
    }

    // Mark values pushed as parents
    for (auto pl: ivm->parent_list) {
        mark_value(pl);
    }

    // Call frame marking
    for (auto cf: ivm->call_frames) {
        for (auto arg: cf->get_args()) {
            mark_value(arg.value);
        }
        mark_value(cf->get_extern_return_value());
        mark_value(cf->get_function());
    }

    // Generator notes are kept to be outputted at the end of program
    for (auto n: Interpreter::get_generator_notes()) {
        mark_value(n);
    }

    // libms marking
    if (ivm->is_main() && Interpreter::libms_mod) {
        assert(Interpreter::libms_mod->get_vm() && "sanity check");
        mark_value(Interpreter::libms_mod);
    }
    if (ivm->is_main()) {
        // Modules in the middle of import, this should be done only once, so by main
        for (auto m: currently_imported_modules) {
            mark_value(m);
        }
        // Unwound functions are static so mark only by main
        for (auto f: Interpreter::unwound_funs) {
            mark_value(f);
        }
    }
}

void TracingGC::collect_garbage() {
    LOG1("Running GC");
    if (!Interpreter::libms_mod 
#ifndef NDEBUG
        && !clopts::no_load_libms
#endif
    ) {
        LOGMAX("Libms is not yet loaded, cannot collect garbage");
        return;
    }
#ifndef NDEBUG
    auto pre_run_allocations = Value::allocated_bytes;
#endif
    gray_list.clear();
    // gray list is empty and values are not marked, so all values are white
    mark_roots(vm);
    // values in gray list (and marked also) are gray
    trace_refs();
    // Value is black when it is not in gray stack and marked is set
    sweep();
    // Freed not used values

#ifndef NDEBUG
    LOGMAX("Freed " << (pre_run_allocations - Value::allocated_bytes) << "B");
    if (clopts::stress_test_gc) {
        LOGMAX("Finished GC (stress test so not modifying next_gc)");
        LOG1("Finished GC");
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