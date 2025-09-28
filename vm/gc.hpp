/// 
/// \file gc.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Tracing moss garbage collector
/// 

#ifndef _GC_HPP_
#define _GC_HPP_

#include "interpreter.hpp"
#include "values.hpp"
#include "memory.hpp"
#include <vector>
#include <list>

namespace moss {

class Interpreter;
class Value;
class ModuleValue;
class MemoryPool;

/// Namespace for garbage collector resources
namespace gcs {

/// \brief Tracing (mark-and-sweep) garbage collector
/// 
/// This GC goes from root values and marks values that are reachable from
/// withing these roots. It then frees all those not marked.
class TracingGC {
private:
    Interpreter *vm;

    std::vector<Value *> gray_list; ///< Work list of marked values

    void mark_roots(Interpreter *vm);
    void mark_frame(MemoryPool *p);
    void mark_value(Value *v);
    void trace_refs();
    void blacken_value(Value *v);
    void sweep();

    static std::vector<ModuleValue *> currently_imported_modules;
    static std::list<MemoryPool *> popped_frames;
public:
    TracingGC(Interpreter *vm);

    /// \brief Runs GC
    void collect_garbage();

    /// While module import is happening GC might ran and it would delete
    /// the module and all its values, pushing it will add it to known accessible
    /// values for GC
    static void push_currently_imported_module(ModuleValue *m);
    /// Once import of a module is done, it should be popped
    static void pop_currently_imported_module();

    /// Adds a new removed memory pool for collection
    static void push_popped_frame(MemoryPool *f);
#ifndef NDEBUG
    static ModuleValue *top_currently_imported_module();
    static size_t get_currently_imported_modules_size() { return currently_imported_modules.size(); }
    static size_t get_popped_frames_size() { return popped_frames.size(); }
    static std::list<MemoryPool *> get_popped_frames() { return popped_frames; }
#endif
};

}

}

#endif//_GC_HPP_