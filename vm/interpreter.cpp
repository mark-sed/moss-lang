#include "interpreter.hpp"
#include "logging.hpp"
#include "values.hpp"
#include "mslib.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code, File *src_file) 
        : code(code), src_file(src_file), gc(new gcs::TracingGC(this)), bci(0), exit_code(0),
          bci_modified(false) {
    this->const_pools.push_back(new MemoryPool(true, true));
    // Global frame
    this->frames.push_back(new MemoryPool(false, true));
    init_const_frame();
    init_global_frame();
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
    delete gc;
    // Code is to be deleted by the creator of it
}

size_t Interpreter::get_free_reg(MemoryPool *fr) {
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

    mslib::init(gf, reg);

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

Value *Interpreter::load_name(ustring name) {
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        auto val = (*riter)->load_name(name);
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
    get_top_frame()->push_spilled_value(v);
}

void Interpreter::push_frame() {
    LOGMAX("Frame pushed");
    this->frames.push_back(new MemoryPool());
    this->const_pools.push_back(new MemoryPool(true));
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

void Interpreter::run() {
    LOG1("Running interpreter\n----- OUTPUT: -----");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        opc->exec(this);

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