#include "interpreter.hpp"
#include "logging.hpp"
#include "values.hpp"
#include "mslib.hpp"
#include "values.hpp"

using namespace moss;

gcs::TracingGC *Interpreter::gc = nullptr;
ModuleValue *Interpreter::libms_mod = nullptr;

Interpreter::Interpreter(Bytecode *code, File *src_file, bool main) 
        : code(code), src_file(src_file), bci(0), exit_code(0),
          bci_modified(false), stop(false), main(main) {
    if (main && !gc) {
        gc = new gcs::TracingGC(this);
    }
    assert(gc && "sanity check");
    this->const_pools.push_back(new MemoryPool(true, true));
    // Global frame
    this->frames.push_back(new MemoryPool(false, true));
    init_const_frame();
    if (!libms_mod && !main) {
        // Init libms module
        init_global_frame();
    }
    if (!libms_mod && main) {
        // Loading a module will also create an interpreter and so we need to
        // set a flag to not try to load it again
        Interpreter::libms_mod = opcode::load_module(this, "libms");
        assert(libms_mod && "TODO: Raise Could not load libms");
    }
    // We don't spill in libms itself so check that it was loaded
    if (libms_mod)
        push_spilled_value(libms_mod);
}

Interpreter::~Interpreter() {
    for (auto p: const_pools) {
        delete p;
    }
    for (auto p: frames) {
        delete p;
    }
    for (auto f: call_frames) {
        delete f;
    }
    for (auto p: parent_list) {
        delete p;
    }
    if (main) {
        // Only main is the holder of gc
        delete gc;
        // It needs to be set to nullptr in case some other new Interpreter
        // is created, like in unit test case
        Interpreter::gc = nullptr;
    }
    // Code is to be deleted by the creator of it
}

opcode::Register Interpreter::get_free_reg(MemoryPool *fr) {
    return fr->get_free_reg();
}

size_t Interpreter::get_code_size() { 
    return this->code->size();
}

static inline void store_glob_val(opcode::Register reg, ustring name, Value *v, MemoryPool *gf) {
    gf->store(reg, v);
    gf->store_name(reg, name);
}

void Interpreter::init_global_frame() {
    auto gf = get_global_frame();

    opcode::Register reg = 0;
    store_glob_val(reg++, "Type", BuiltIns::Type, gf);
    store_glob_val(reg++, "Int", BuiltIns::Int, gf);
    store_glob_val(reg++, "Float", BuiltIns::Float, gf);
    store_glob_val(reg++, "Bool", BuiltIns::Bool, gf);
    store_glob_val(reg++, "List", BuiltIns::List, gf);
    store_glob_val(reg++, "NilType", BuiltIns::NilType, gf);
    store_glob_val(reg++, "String", BuiltIns::String, gf);
    store_glob_val(reg++, "Address", BuiltIns::Address, gf);
    store_glob_val(reg++, "Function", BuiltIns::Function, gf);
    store_glob_val(reg++, "FunctionList", BuiltIns::FunctionList, gf);
    store_glob_val(reg++, "Module", BuiltIns::Module, gf);

    store_glob_val(reg++, "Exception", BuiltIns::Exception, gf);

    assert(reg < BC_RESERVED_REGS && "More registers used that is reserved");
}

void Interpreter::init_const_frame() {
    auto cf = get_global_const_pool();

    opcode::Register reg = 0;
    cf->store(reg++, BuiltIns::Nil);
    for (auto i : BuiltIns::IntConstants) {
        cf->store(reg++, i);
    }

    assert(reg < BC_RESERVED_CREGS && "More c registers used that is reserved");
}

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    unsigned index = 0;
    for (auto f: frames) {
        if (index == 0)
            os << "\t[" << index << "] Global frame:\n" << *f << "\n";
        else
            os << "\t[" << index << "] Local frame:\n" << *f << "\n";
        ++index;
    }
    index = 0;
    for (auto f: const_pools) {
        os << "\t[" << index << "] Constant frame:\n" << *f << "\n";
        ++index;
    }
    os << "\tExit code: " << exit_code << "\n";
    os << "}\n";
    return os;
}

std::ostream& CallFrameArg::debug(std::ostream& os) const {
    os << "\"" << name << "\": " << *value << " dst = %" << dst;
    return os;
}

void Interpreter::store(opcode::Register reg, Value *v) {
    get_local_frame()->store(reg, v);
}

void Interpreter::store_const(opcode::Register reg, Value *v) {
    get_const_pool()->store(reg, v);
}

Value *Interpreter::load(opcode::Register reg) {
    return get_local_frame()->load(reg);
}

Value *Interpreter::load_const(opcode::Register reg) {
    return get_const_pool()->load(reg);
}

void Interpreter::store_name(opcode::Register reg, ustring name) {
    get_local_frame()->store_name(reg, name);
}

void Interpreter::remove_global_name(ustring name) {
    get_global_frame()->remove_name(name);
}

Value *Interpreter::load_name(ustring name, Value **owner) {
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        auto val = (*riter)->load_name(name, owner);
        if (val)
            return val;
    }
    return nullptr;
}

Value *Interpreter::load_type(ustring name) {
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        auto val = (*riter)->load_name(name);
        if (val && isa<ClassValue>(val))
            return val;
    }
    return nullptr;
}

Value *Interpreter::load_global_name(ustring name) {
    return get_global_frame()->load_name(name);
}

void Interpreter::push_spilled_value(Value *v) {
    assert(v && "sanity check");
    get_top_frame()->push_spilled_value(v);
}

void Interpreter::push_frame(FunValue *fun_owner) {
    LOGMAX("Frame pushed");
    auto lf = new MemoryPool();
    this->frames.push_back(lf);
    this->const_pools.push_back(new MemoryPool(true));
    if (fun_owner)
        lf->set_pool_fun_owner(fun_owner);
        
}

void Interpreter::pop_frame() {
    LOGMAX("Frame popped");
    assert(frames.size() > 1 && "Trying to pop global frame");
    //auto f = frames.back();
    frames.pop_back();
    //delete f;
    assert(const_pools.size() > 1 && "Trying to pop global const frame");
    auto c = const_pools.back();
    const_pools.pop_back();
    delete c;
}

void Interpreter::cross_module_call(FunValue *fun, CallFrame *cf) {
    push_frame(fun);
    call_frames.push_back(cf);
    set_bci(fun->get_body_addr());
    run();
}

void Interpreter::runtime_call(FunValue *fun) {
    auto pre_call_bci = this->bci;
    auto pre_bci_modified = this->bci_modified;
    auto pre_stop = this->stop;

    push_frame();
    set_bci(fun->get_body_addr());
    run();

    this->bci = pre_call_bci;
    this->bci_modified = pre_bci_modified;
    this->stop = pre_stop;
}

/*void Interpreter::collect_garbage() {
    gc->collect_garbage();
}*/

void Interpreter::push_currently_imported_module(ModuleValue *m) {
    gcs::TracingGC::push_currently_imported_module(m);
}

void Interpreter::pop_currently_imported_module() {
    gcs::TracingGC::pop_currently_imported_module();
}

#ifndef NDEBUG
ModuleValue *Interpreter::top_currently_imported_module() {
    return gcs::TracingGC::top_currently_imported_module();
}
#endif

void Interpreter::run() {
    LOG1("Running interpreter of " << (src_file ? src_file->get_name() : "??") << "\n----- OUTPUT: -----");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        opc->exec(this);
        if (stop || global_controls::exit_called) {
            stop = false;
            break;
        }

        // If bci was modified (jmp), then don't change it
        if (bci_modified)
            bci_modified = false;
        else
            ++bci;

        if (global_controls::trigger_gc) {
            gc->collect_garbage();
            global_controls::trigger_gc = false;
        }
    }

    LOG1("Finished interpreter");
}