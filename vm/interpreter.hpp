/// 
/// \file interpreter.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss bytecode interpreter
/// 
/// It connects all the VM parts into one and runs moss bytecode.
/// 

#ifndef _INTERPRETER_HPP_
#define _INTERPRETER_HPP_

#include "memory.hpp"
#include "bytecode.hpp"
#include "source.hpp"
#include "commons.hpp"
#include "gc.hpp"
#include "values.hpp"
#include <cstdint>
#include <list>

namespace moss {

class MemoryPool;
class Bytecode;
class Value;
class FunValue;
class ModuleValue;
class ClassValue;

namespace gcs {
    class TracingGC;
}

/// Structure that holds information about argument in a function call
struct CallFrameArg {
    ustring name;
    Value *value;
    opcode::Register dst;

    CallFrameArg(ustring name, Value *value, opcode::Register dst)
                : name(name), value(value), dst(dst) {}
    CallFrameArg(ustring name, Value *value) : name(name), value(value), dst(0) {}
    CallFrameArg(Value *value) : name(""), value(value), dst(0) {}

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, CallFrameArg &cf) {
    return cf.debug(os);
}

/// Frame that holds all call related information
/// The arguments, return register and argument type (const, addr, val) 
class CallFrame {
private:
    Value *function;
    std::vector<CallFrameArg> args;
    opcode::Register return_reg;
    opcode::Address caller_addr;
    bool constructor_call;
    bool extern_module_call;
    bool runtime_call;
    Value *extern_return_value;
public:
    CallFrame(Value *function=nullptr) : function(function),
                                         return_reg(0),
                                         constructor_call(false),
                                         extern_module_call(false),
                                         runtime_call(false),
                                         extern_return_value(nullptr) {}

    void push_back(Value *v) { args.push_back(CallFrameArg(v)); }
    void push_back(ustring name, Value *v) { args.push_back(CallFrameArg(name, v)); }
    void push_back(ustring name, Value *v, opcode::Register dst) {
        args.push_back(CallFrameArg(name, v, dst)); 
    }
    void set_function(Value *function) { this->function = function; }
    void set_return_reg(opcode::Register r) { this->return_reg = r; }
    void set_caller_addr(opcode::Address addr) { this->caller_addr = addr; }
    void set_args(std::vector<CallFrameArg> args) { this->args = args; }
    void set_constructor_call(bool b) { this->constructor_call = b; }
    void set_extern_module_call(bool b) { this->extern_module_call = b; }
    void set_runtime_call(bool b) { this->runtime_call = b; }
    void set_extern_return_value(Value *v) { this->extern_return_value = v; }

    Value *get_function() { return this->function; }
    std::vector<CallFrameArg> &get_args() { return this->args; }
    opcode::Register get_return_reg() { return this->return_reg; }
    opcode::Address get_caller_addr() { return this->caller_addr; }
    bool is_constructor_call() { return this->constructor_call; }
    bool is_extern_module_call() { return this->extern_module_call; }
    bool is_runtime_call() { return this->runtime_call; }
    Value *get_extern_return_value() { return this->extern_return_value; }

    Value *get_arg(ustring name, bool optional=false) {
        for (auto a: args) {
            if (a.name == name) {
                return a.value;
            }
        }
        assert(optional && "Argument expected but not found");
        return nullptr;
    }

    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, CallFrame &cf) {
    return cf.debug(os);
}

/// Object for matching moss exceptions to addresses.
struct ExceptionCatch {
    Value *type;
    ustring name;
    opcode::Address addr;

    CallFrame *cf_position;
    MemoryPool *frame_position;

    ExceptionCatch(Value *type,
                   ustring name,
                   opcode::Address addr,
                   CallFrame *cf_position,
                   MemoryPool *frame_position)
        : type(type), name(name), addr(addr), cf_position(cf_position),
          frame_position(frame_position) {
    }
};

/// \brief Interpreter for moss bytecode
/// Interpreter holds memory pools (stack frames) and runs bytecode provided
class Interpreter {
private:
    friend class gcs::TracingGC;
    Bytecode *code;
    File *src_file;
    
    std::list<MemoryPool *> const_pools;
    std::list<MemoryPool *> frames;      ///< Frame stack

    std::list<CallFrame *> call_frames;  ///< Call frame stack
    std::list<ClassValue *> parent_list; ///< Classes that will be used in class construction

    std::list<ExceptionCatch> catches;
    //std::list<Value *> exception_stack;  ///< Stack of raised exceptions, there might be multiple of them

    static gcs::TracingGC *gc;

    opcode::Address bci;

    int exit_code;

    bool bci_modified;
    bool stop;
    bool main;

    MemoryPool *get_global_const_pool() { return this->const_pools.front(); };
    MemoryPool *get_const_pool() { return this->const_pools.back(); }
    MemoryPool *get_local_frame() { return this->frames.back(); }

    void init_const_frame();
    void init_global_frame();
public:
    Interpreter(Bytecode *code, File *src_file=nullptr, bool main=false);
    ~Interpreter();

    void run();

    void cross_module_call(FunValue *fun, CallFrame *cf);
    void runtime_call(FunValue *fun);

    static ModuleValue *libms_mod;

    /// Stores a value into a register
    void store(opcode::Register reg, Value *v);

    /// Stores a value into constant pool
    void store_const(opcode::Register reg, Value *v);

    /// Sets a name for specific register
    void store_name(opcode::Register reg, ustring name);

    void remove_global_name(ustring name);
 
    /// Loads value at specified register index 
    /// If there was no value stored, then Nil is stored there and returned
    Value *load(opcode::Register reg);

    /// Loads a value from constant pool
    Value *load_const(opcode::Register reg);
 
    /// Looks up a name and returns value corresponding to it in symbol table
    /// If there is no such name, then nullptr is returned
    /// \param owner If not null then it will be set to the owner if the value
    ///              is a module or space. Otherwise nullptr.
    Value *load_name(ustring name, Value **owner=nullptr);

    /// Looks up a type (ClassValue) that matches passed in name
    /// If there is no type with such name, then nullptr is returned
    Value *load_type(ustring name);
 
    /// Looks up a name in global frame and returns its value
    /// If there is no such name, then nullptr is returned
    Value *load_global_name(ustring name);

    /// Pushes a new frame (memory pool) into a frame stack
    /// \param fun_owner If set then the function owner of the frame is set,
    ///                  this is needed for accessing function closures
    void push_frame(FunValue *fun_owner=nullptr);
    /// Pops a frame (memory pool) from a frame stack
    void pop_frame();
    /// \return Top frame, meaning the current local frame or global if no local is inserted
    MemoryPool *get_top_frame() { return this->get_local_frame(); }
    MemoryPool *get_global_frame() { return this->frames.front(); }
    std::list<MemoryPool *>& get_frames() { return this->frames; }

    /// Spills value into current frame
    void push_spilled_value(Value *v);

    /// Returns top call frame
    CallFrame *get_call_frame() { 
        assert(!this->call_frames.empty() && "no call frame was pushed");
        return this->call_frames.back();
    }
    bool has_call_frame() {
        return !this->call_frames.empty();
    }
    std::ostream& report_call_stack(std::ostream& os);

    /// Pushes a new empty call frame into call frame stack
    void push_call_frame(Value *fun=nullptr) {
        call_frames.push_back(new CallFrame(fun));
    }
    /// Pops top (most recent) frame from call frame stack and deletes it
    void pop_call_frame() { 
        assert(!this->call_frames.empty() && "no call frame to pop");
        auto cf = call_frames.back();
        call_frames.pop_back(); 
        delete cf;
    }
    /// Pops most recet call frame, but does not delete it
    void drop_call_frame() {
        assert(!this->call_frames.empty() && "no call frame to pop");
        call_frames.pop_back(); 
    }

    /// Pushes a new class into parent list
    void push_parent(ClassValue *cls) { parent_list.push_back(cls); }
    /// Clears parent list
    void clear_parent_list() { parent_list.clear(); }
    std::list<ClassValue *> get_parent_list() { return this->parent_list; }

    /// Pushes a ModuleValue into a list of Modules currently being imported.
    /// This is needed so that GC can mark these values and not collect them
    /// since the module value will be placed into frame after the full run
    void push_currently_imported_module(ModuleValue *m);
    /// Removes last module from list of currently imported modules
    void pop_currently_imported_module();
#ifndef NDEBUG
    /// Returns current last module in the list of currently imported modules.
    /// This method is for debugging
    ModuleValue *top_currently_imported_module();
#endif

    void push_catch(ExceptionCatch ec) {
        this->catches.push_back(ec);
    }
    void pop_catch() {
        assert(!this->catches.empty() && "Popping empty catch stack");
        this->catches.pop_back();
    }
    std::list<ExceptionCatch>& get_catches() { return this->catches; }

    //void push_exception(Value *v) { exception_stack.push_back(v); }
    //void pop_exception() { exception_stack.pop_back(); }
    //Value *top_exception() { return exception_stack.back(); }
    //bool has_exception() { return !exception_stack.empty(); }
    //std::list<Value *> &get_exceptions() { return exception_stack; }

    //void collect_garbage();

    bool is_main() { return this->main; }

    /// \return Interpreter exit code
    int get_exit_code() { return exit_code; }

    void set_exit_code(int c) { this->exit_code = c; }

    void set_stop(bool s) { this->stop = s; }
    bool is_stop() { return this->stop; }

    /// \return Current bytecode index
    opcode::Address get_bci() { return this->bci; }
    /// Sets current bytecode index to passed in value
    void set_bci(opcode::Address v) { 
        this->bci = v;
        this->bci_modified = true; 
    }

    void handle_exception(ExceptionCatch ec, Value *v);

    void restore_to_global_frame();

    opcode::Register get_free_reg(MemoryPool *fr);

    size_t get_code_size();

    Bytecode *get_code() { return this->code; }

    File *get_src_file() { return this->src_file; }
    
    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, Interpreter &i) {
    return i.debug(os);
}

}

#endif//_INTERPRETER_HPP_