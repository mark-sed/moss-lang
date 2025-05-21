/// 
/// \file interpreter.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
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
#include "logging.hpp"
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

using T_Converters = std::map<std::pair<ustring, ustring>, FunValue *>;
using T_Generators = std::map<ustring, FunValue *>;

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
                                         caller_addr(0),
                                         constructor_call(false),
                                         extern_module_call(false),
                                         runtime_call(false),
                                         extern_return_value(nullptr) {}

    /// Pushes a Value as a new argument into the call frame stack
    void push_back(Value *v) { args.push_back(CallFrameArg(v)); }
    /// Pushes a named Value as a new argument into the call frame stack
    void push_back(ustring name, Value *v) { args.push_back(CallFrameArg(name, v)); }
    /// Pushes a names Value as a new argument into the call frame stack and
    /// sets its destination register in the function frame
    void push_back(ustring name, Value *v, opcode::Register dst) {
        args.push_back(CallFrameArg(name, v, dst)); 
    }
    /// Sets function, which will receive this call frame.
    /// This is needed mostly for stack tracing.
    void set_function(Value *function) { this->function = function; }
    /// Sets which register should contain the function return value
    void set_return_reg(opcode::Register r) { this->return_reg = r; }
    /// Sets from which address was this called and therefore where to return
    void set_caller_addr(opcode::Address addr) { this->caller_addr = addr; }
    /// Bulk argument setting
    void set_args(std::vector<CallFrameArg> args) { this->args = args; }
    /// Denotes if this call is a constructor call (to handle return value)
    void set_constructor_call(bool b) { this->constructor_call = b; }
    /// Denotes if this call is to another module
    void set_extern_module_call(bool b) { this->extern_module_call = b; }
    /// Denotes if this call was generated in runtime and not in codegen
    void set_runtime_call(bool b) { this->runtime_call = b; }
    /// Sets value to return when this call was external or runtime
    void set_extern_return_value(Value *v) { this->extern_return_value = v; }

    Value *get_function() { return this->function; }
    std::vector<CallFrameArg> &get_args() { return this->args; }
    opcode::Register get_return_reg() { return this->return_reg; }
    opcode::Address get_caller_addr() { return this->caller_addr; }
    bool is_constructor_call() { return this->constructor_call; }
    bool is_extern_module_call() { return this->extern_module_call; }
    bool is_runtime_call() { return this->runtime_call; }
    Value *get_extern_return_value() { return this->extern_return_value; }

    /// Extracts argument from call frame
    Value *get_arg(ustring name) {
        for (auto a: args) {
            if (a.name == name) {
                return a.value;
            }
        }
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

    std::list<ExceptionCatch> catches;   ///< List of current catches

    static gcs::TracingGC *gc;  ///< Garbage collector for this VM
    // TODO: change to unordered_map
    static T_Converters converters; ///< Mapping of formats and their converters
    static T_Generators generators; ///< Mapping of formats and their generators
    static std::vector<Value *> generator_notes; ///< List of notes to be passed to a generator
    // TODO: Should this be static?
    bool enable_code_output; ///< When enabled then output will be enclosed as the output of code notes
    
    opcode::Address bci; ///< Current bytecode index
    
    int exit_code; ///< VM's exit code
    
    bool bci_modified;
    bool stop;
    bool main;
    
    MemoryPool *get_global_const_pool() { return this->const_pools.front(); };
    MemoryPool *get_const_pool() { return this->const_pools.back(); }
    MemoryPool *get_local_frame() { return this->frames.back(); }
    
    void init_const_frame();
    opcode::Register init_global_frame();
public:
    static bool running_generator; ///< When true it means that the currently run code is generator of the output

    Interpreter(Bytecode *code, File *src_file=nullptr, bool main=false);
    ~Interpreter();

    /// Runs interpreter
    void run();

    /// Call to another VM's function
    /// \param fun Function that is called
    /// \param cf Call frame to use for the call
    void cross_module_call(FunValue *fun, CallFrame *cf);

    /// Runtime generated call to a function
    void runtime_call(FunValue *fun);

    static ModuleValue *libms_mod;  ///< Standard library as module

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

    /// Looks up a symbol from top-1 frame up until global frame (excluding it)
    Value *load_non_local_name(ustring name);
    bool store_non_local(ustring name, Value *v);

    /// Pushes a new frame (memory pool) into a frame stack
    /// \param fun_owner If set then the function owner of the frame is set,
    ///                  this is needed for accessing function closures
    void push_frame(FunValue *fun_owner=nullptr);
    /// Pops a frame (memory pool) from a frame stack
    void pop_frame();
    /// \return Top frame, meaning the current local frame or global if no local is inserted
    MemoryPool *get_top_frame() { return this->get_local_frame(); }
    MemoryPool *get_top_const_frame() { return this->get_const_pool(); }
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
    size_t get_call_frame_size() {
        return this->call_frames.size();
    }
    /// Outputs call stack in the VM
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

    /// Pushes a new catch exception block into the catch stack
    void push_catch(ExceptionCatch ec) {
        this->catches.push_back(ec);
    }
    /// Removes value from top of the stack
    void pop_catch() {
        assert(!this->catches.empty() && "Popping empty catch stack");
        this->catches.pop_back();
    }
    std::list<ExceptionCatch>& get_catches() { return this->catches; }

    /// Adds a new converter from to type into the list of converters
    static void add_converter(ustring from, ustring to, FunValue *fun);
    static std::vector<FunValue *> get_converter(ustring from, ustring to);
    static std::vector<FunValue *> get_converter(std::pair<ustring, ustring> key);
    /// Adds a new generator to the list of all generators
    static void add_generator(ustring format, FunValue *fun);
    static FunValue *get_generator(ustring format);
    /// \return true if there exists generator for given format (even if there is also a convertor)
    static bool is_generator(ustring format);
    /// Pushes a new note for a generator
    static void add_generator_note(Value *note) { Interpreter::generator_notes.push_back(note); }
    static std::vector<Value *> get_generator_notes() { return Interpreter::generator_notes; }
    void set_enable_code_output(bool s) { this->enable_code_output = s; }
    bool is_enable_code_output() { return this->enable_code_output; }

    //void push_exception(Value *v) { exception_stack.push_back(v); }
    //void pop_exception() { exception_stack.pop_back(); }
    //Value *top_exception() { return exception_stack.back(); }
    //bool has_exception() { return !exception_stack.empty(); }
    //std::list<Value *> &get_exceptions() { return exception_stack; }

    //void collect_garbage();

    /// \return true if the current VM is vm of the main module
    bool is_main() { return this->main; }

    /// \return Interpreter exit code
    int get_exit_code() { return exit_code; }

    void set_exit_code(int c) { this->exit_code = c; }

    /// When set to true, interpreter will halt on the next instruction
    void set_stop(bool s) {
        LOGMAX("Stop was set for interpreter: " << src_file->get_name());
        this->stop = s;
    }
    bool is_stop() { return this->stop; }

    /// \return Current bytecode index
    opcode::Address get_bci() { return this->bci; }
    /// Sets current bytecode index to passed in value
    void set_bci(opcode::Address v) { 
        this->bci = v;
        this->bci_modified = true; 
    }

    /// Handler for an exception
    void handle_exception(ExceptionCatch ec, Value *v);

    /// Restores frames to a global frame state
    void restore_to_global_frame();

    /// Returns number of the first free possible register in a frame
    opcode::Register get_free_reg(MemoryPool *fr);

    /// Returns number of instructions in the bytecode this interprets
    size_t get_code_size();

    /// Returns interpreted bytecode
    Bytecode *get_code() { return this->code; }

    /// Returns source file this interprets
    File *get_src_file() { return this->src_file; }
    
    std::ostream& debug(std::ostream& os) const;
};

inline std::ostream& operator<< (std::ostream& os, Interpreter &i) {
    return i.debug(os);
}

}

#endif//_INTERPRETER_HPP_