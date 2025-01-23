/// 
/// \file gc.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
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
public:
    TracingGC(Interpreter *vm);

    void collect_garbage();

    static void push_currently_imported_module(ModuleValue *m);
    static void pop_currently_imported_module();
#ifndef NDEBUG
    static ModuleValue *top_currently_imported_module();
#endif
};

}

}

#endif//_GC_HPP_