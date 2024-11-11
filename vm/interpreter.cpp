#include "interpreter.hpp"
#include "logging.hpp"
#include "values.hpp"
#include "mslib.hpp"

using namespace moss;

Interpreter::Interpreter(Bytecode *code, File *src_file) : code(code), src_file(src_file), bci(0), exit_code(0), bci_modified(false) {
    this->const_pool = new MemoryPool(true, true);
    // Global frame
    this->frames.push_back(new MemoryPool(false, true));
    init_const_frame();
    init_global_frame();
}

Interpreter::~Interpreter() {
    delete const_pool;
    for(auto p: frames) {
        delete p;
    }
    // Code is to be deleted by the creator of it
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
    store_glob_val(reg++, "NilType", BuiltIns::NilType, gf);
    store_glob_val(reg++, "String", BuiltIns::String, gf);
    store_glob_val(reg++, "Address", BuiltIns::Address, gf);
    store_glob_val(reg++, "Function", BuiltIns::Function, gf);
    store_glob_val(reg++, "FunctionList", BuiltIns::FunctionList, gf);

    mslib::init(gf, reg);

    assert(reg < BC_RESERVED_REGS && "More registers used that is reserved");
}

void Interpreter::init_const_frame() {
    auto cf = get_const_pool();

    opcode::Register reg = 0;
    cf->store(reg++, BuiltIns::Nil);
    for (auto i : BuiltIns::IntConstants) {
        cf->store(reg++, i);
    }

    assert(reg < BC_RESERVED_CREGS && "More c registers used that is reserved");
}

/*std::ostream& Interpreter::debug_pool(std::ostream &os) const {

}*/

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
    os << "\tConstant pool:\n" << *const_pool << "\n";
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

Value *Interpreter::load_global_name(ustring name) {
    return get_global_frame()->load_name(name);
}

/*ustring Interpreter::get_reg_pure_name(opcode::Register reg) {
    return get_local_frame()->get_reg_pure_name(reg);
}

std::vector<ustring> Interpreter::get_reg_names(opcode::Register reg) {
    return get_local_frame()->get_reg_names(reg);
}

std::pair<Value *, opcode::Register> Interpreter::load_name_reg(ustring name) {
    // TODO: Walk frames???
    return get_local_frame()->load_name_reg(name);
}

void Interpreter::copy_names(opcode::Register from, opcode::Register to) {
    // TODO: Walk frames??
    return get_local_frame()->copy_names(from, to);
}*/

void Interpreter::push_frame() {
    this->frames.push_back(new MemoryPool());
}

void Interpreter::pop_frame() {
    assert(frames.size() > 1 && "Trying to pop global frame");
    auto top = frames.back();
    frames.pop_back();
    // TODO: We cannot delete the frame as it might be used as attr and also
    // it might hold values used in other places. This means that it probably
    // needs to be stored and then values from it collected by the GC
    //delete top;
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
    }

    LOG1("Finished interpreter");
}