#include "interpreter.hpp"
#include "logging.hpp"
#include "values.hpp"
#include "mslib.hpp"
#include "values.hpp"
#include <exception>
#include <utility>
#include <queue>
#include <unordered_set>
#include <map>

using namespace moss;

#ifndef NDEBUG
long CallFrame::allocated = 0;
#endif

gcs::TracingGC *Interpreter::gc = nullptr;
ModuleValue *Interpreter::libms_mod = nullptr;
T_Converters Interpreter::converters{};
T_Generators Interpreter::generators{};
std::vector<Value *> Interpreter::generator_notes{};
bool Interpreter::running_generator = false;
bool Interpreter::enable_code_output = false;

void Interpreter::add_converter(ustring from, ustring to, FunValue *fun) {
    LOGMAX("Adding a converter " << from << " to " << to << " on " << fun->get_name());
    converters[std::make_pair(from, to)] = fun;
}

void Interpreter::add_generator(ustring format, FunValue *fun) {
    LOGMAX("Adding a generator " << format << " on " << fun->get_name());
    generators[format] = fun;
}

bool Interpreter::is_generator(ustring format) {
    for (const auto& [conv_key, func] : generators) {
        if (conv_key == format)
            return true;
    }
    return false;
}

std::vector<FunValue *> Interpreter::get_converter(ustring from, ustring to) {
    return Interpreter::get_converter(std::make_pair(from, to));
}

std::vector<FunValue *> Interpreter::get_converter(std::pair<ustring, ustring> key) {
    auto found = converters.find(key);
    if (found != converters.end()) {
        return {found->second};
    }

    ustring source = key.first;
    ustring target = key.second;

    // Queue of formats to visit
    std::queue<ustring> q;
    // Track visited formats
    std::unordered_set<ustring> visited;
    // To reconstruct path: maps format to previous format and converter used
    std::unordered_map<ustring, std::pair<ustring, FunValue*>> parent;

    q.push(source);
    visited.insert(source);

    while (!q.empty()) {
        ustring current = q.front();
        q.pop();

        for (const auto& [conv_key, func] : converters) {
            const ustring& from = conv_key.first;
            const ustring& to = conv_key.second;

            if (from != current) continue;
            if (visited.count(to)) continue;

            visited.insert(to);
            parent[to] = {from, func};

            if (to == target) {
                // Reconstruct path
                std::vector<FunValue*> path;
                for (ustring at = target; at != source; at = parent[at].first) {
                    path.push_back(parent[at].second);
                }
                return path;
            }

            q.push(to);
        }
    }

    // No path found
    return {};
}

FunValue * Interpreter::get_generator(ustring format) {
    auto found = generators.find(format);
    if (found != generators.end()) {
        return {found->second};
    }
    return nullptr;
}

Interpreter::Interpreter(Bytecode *code, File *src_file, bool main) 
        : code(code), src_file(src_file), bci(0),
          exit_code(0), bci_modified(false), stop(false), main(main) {
    if (main && !gc) {
        gc = new gcs::TracingGC(this);
    }
    assert(gc && "sanity check");
    this->const_pools.push_back(new MemoryPool(true, true));
    // Global frame
    this->frames.push_back(new MemoryPool(false, true));
    init_const_frame();
    opcode::Register glob_reg = 0;
    if (!libms_mod && !main) {
        // Init libms module
        glob_reg = init_global_frame();
    }
#ifndef NDEBUG
    if (!clopts::no_load_libms)
#endif
    if (!libms_mod && main) {
        // Loading a module will also create an interpreter and so we need to
        // set a flag to not try to load it again
        Interpreter::libms_mod = opcode::load_module(this, "libms");
        assert(libms_mod && "TODO: Raise Could not load libms");
        // Constants should be loaded after libms so that the values can contain
        // libms attributes/methods
        BuiltIns::init_constant_variables(Interpreter::libms_mod->get_vm()->get_global_frame(), libms_mod->get_vm());
    }
    // We don't spill in libms itself so check that it was loaded
    if (libms_mod) {
        push_spilled_value(libms_mod);
        auto gf = this->get_global_frame();
        gf->store(glob_reg, libms_mod);
        gf->store_name(glob_reg, "moss");
        ++glob_reg;
    }
    assert(glob_reg < BC_RESERVED_REGS && "More registers used that is reserved");
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

        for (auto v: generator_notes) {
            delete v;
        }
    }
    // Code is to be deleted by the creator of it
}

opcode::Register Interpreter::get_free_reg(MemoryPool *fr) {
    return fr->get_free_reg();
}

size_t Interpreter::get_code_size() { 
    return this->code->size();
}

opcode::Register Interpreter::init_global_frame() {
    auto gf = get_global_frame();

    opcode::Register reg = 0;
    BuiltIns::init_built_ins(gf, reg);

    return reg;
}

void Interpreter::init_const_frame() {
    auto cf = get_global_const_pool();

    opcode::Register reg = 0;
    cf->store(reg++, BuiltIns::Nil);
    cf->store(reg++, BuiltIns::True);
    cf->store(reg++, BuiltIns::False);
    for (auto i : IntValue::get_interned()) {
        cf->store(reg++, i);
    }

    assert(reg < BC_RESERVED_CREGS && "More c registers used that is reserved");
}

std::ostream& Interpreter::debug(std::ostream& os) const {
    os << "Interpreter {\n";
    /*unsigned index = 0;
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
    }*/
    os << "\tConverters:\n";
    for (auto [p, f]: converters) {
        os << "\"" << p.first << "\"->\"" << p.second << "\": " << f->dump() << "\n";
    }
    os << "\n";
    os << "\tGenerators:\n";
    for (auto [p, f]: generators) {
        os << "\"" << p << "\": " << f->dump() << "\n";
    }
    os << "\n";
    os << "\tExit code: " << exit_code << "\n";
    os << "}\n";
    return os;
}

std::ostream& CallFrameArg::debug(std::ostream& os) const {
    os << "\"" << name << "\": " << *value << " dst = %" << dst;
    return os;
}

std::ostream& CallFrame::debug(std::ostream& os) const {
    os << "CallFrame:\n";
    if (function)
        os << "\tfunction: " << *function << "\n";
    else
        os << "\tfunction: unknown\n";
    os << "\treturn_reg: " << return_reg << "\n"
        << "\tcaller_addr: " << caller_addr << "\n"
        << "\textern_module_call: " << extern_module_call << "\n"
        << "\truntime_call: " << runtime_call << "\n"
        << "\tconstructor call: " << constructor_call << "\n"
        << "\targs:\n";
    unsigned index = 0;
    for(auto a: args) {
        os << "\t\t" << index << ": " << a << "\n";
        ++index;
    }
        
    return os;
}

std::ostream& ExceptionCatch::debug(std::ostream& os) const {
    os << "ExceptionCatch:\n";
    os << "\ttype: " << (type ? type->get_name() : "*") << "\n"
       << "\tname: " << name << "\n"
       << "\taddr: " << addr << "\n"
       << "\tname: " << name << "\n";
    return os;
}

std::ostream& Interpreter::report_call_stack(std::ostream& os) {
    // TODO: Color output
    os << "Stacktrace:\n";
    for (auto riter = call_frames.rbegin(); riter != call_frames.rend(); ++riter) {
        CallFrame *cf = *riter;
        Value *fun_val = cf->get_function();
        if (!fun_val) {
            // This is a case where the exception was raised while calling a
            // function, so we skip this
            continue;
        }
        os << "  ";
        FunValue *fun = dyn_cast<FunValue>(fun_val);
        if (!fun) {
            // Can this happen?
            os << fun_val->get_name() << "\n";
        } else {
            os << fun->get_name() << "(" << fun->get_args_as_str() << ") at ";
            if (fun->get_vm()) {
                os << fun->get_vm()->get_src_file()->get_name() << "\n";
            } else {
                os << "??\n";
            }
        }
    }
    os << "  top-level scope at " << src_file->get_name() << "\n";
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
        auto val = (*riter)->load_name(name, this, owner);
        if (val)
            return val;
    }
    return nullptr;
}

Value *Interpreter::load_type(ustring name) {
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        auto val = (*riter)->load_name(name, this, nullptr);
        if (val && isa<ClassValue>(val))
            return val;
    }
    return nullptr;
}

Value *Interpreter::load_global_name(ustring name) {
    return get_global_frame()->load_name(name, this, nullptr);
}

Value *Interpreter::load_non_local_name(ustring name) {
    if (frames.size() <= 1) return nullptr;
    for (auto riter = std::next(frames.rbegin()); riter != std::prev(frames.rend()); ++riter) {
        auto val = (*riter)->load_name(name, this, nullptr);
        if (val)
            return val;
    }
    return nullptr;
}

bool Interpreter::store_non_local(ustring name, Value *v) {
    if (frames.size() <= 1) return false;
    for (auto riter = std::next(frames.rbegin()); riter != std::prev(frames.rend()); ++riter) {
        auto done = (*riter)->overwrite(name, v, this);
        if (done) {
            return true;
        }
    }
    return false;
}

void Interpreter::push_spilled_value(Value *v) {
    assert(v && "sanity check");
    get_top_frame()->push_spilled_value(v);
}

void Interpreter::push_frame(Value *fun_owner) {
    LOGMAX("Frame pushed");
    auto lf = new MemoryPool();
    this->frames.push_back(lf);
    this->const_pools.push_back(new MemoryPool(true));
    if (fun_owner)
        lf->set_pool_owner(fun_owner);
        
}

void Interpreter::push_frame(MemoryPool *pool) {
    LOGMAX("Passed in frame pushed");
    this->frames.push_back(pool);
    this->const_pools.push_back(new MemoryPool(true));
}

void Interpreter::pop_frame() {
    LOGMAX("Frame popped");
    assert(frames.size() > 1 && "Trying to pop global frame");
    auto f = frames.back();
    frames.pop_back();
    gc->push_popped_frame(f);
    assert(const_pools.size() > 1 && "Trying to pop global const frame");
    auto c = const_pools.back();
    const_pools.pop_back();
    delete c;
}

void Interpreter::cross_module_call(FunValue *fun, CallFrame *cf) {
    push_frame(fun);
    call_frames.push_back(cf);
    set_bci(fun->get_body_addr());
    try {
        run();
    } catch (Value *e) {
        LOGMAX("Exception in cross_module_call, pop_frame and rethrow");
        // No return encountered so pop frame
        pop_frame();
        throw e;
    }
}

void Interpreter::runtime_call(FunValue *fun) {
    auto pre_call_bci = this->bci;
    auto pre_bci_modified = this->bci_modified;
    auto pre_stop = this->stop;

    LOGMAX("Runtime call to " << *fun << " with: " << *get_call_frame());

    push_frame();
    get_call_frame()->set_function(fun);
    set_bci(fun->get_body_addr());
    try {
        run();
    } catch (Value *e) {
        LOGMAX("Exception in runtime_call, restore vm info and pop_frame and rethrow");
        this->bci = pre_call_bci;
        this->bci_modified = pre_bci_modified;
        this->stop = pre_stop;
        // No return encountered so pop frame
        pop_frame();
        throw e;
    }

    this->bci = pre_call_bci;
    this->bci_modified = pre_bci_modified;
    this->stop = pre_stop;
}

void Interpreter::collect_garbage() {
    gc->collect_garbage();
}

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

void Interpreter::push_finally(opcode::Finally *fnl) {
    this->get_top_frame()->push_finally(fnl);
}

void Interpreter::pop_finally() {
    this->get_top_frame()->pop_finally();
}

void Interpreter::push_finally_stack() {
    this->get_top_frame()->push_finally_stack();
}

void Interpreter::pop_finally_stack() {
    this->get_top_frame()->pop_finally_stack();
}

bool Interpreter::has_finally() {
    return !this->get_top_frame()->get_finally_stack().empty();
}

void Interpreter::call_finally() {
    assert(has_finally() && "Getting finally address from empty stack");
    auto fnl = get_top_frame()->get_finally_stack().back();
    int addr_offset = 0;
    if (is_try_not_in_catch())
        addr_offset = -1;
    store_const(fnl->caller, IntValue::get(get_bci()));
    // Subtract 1 instruction if this was called from try body, because
    // pop_catch is at this address.
    set_bci(fnl->addr + addr_offset);
}

bool Interpreter::is_try_not_in_catch() {
    assert(has_finally() && "Getting finally address from empty stack");
    auto fnl = get_top_frame()->get_finally_stack().back();
    auto val = load_const(fnl->caller);
    return isa<NilValue>(val);
}

void Interpreter::handle_exception(ExceptionCatch ec, Value *v) {
    LOGMAX("Exception catch found:\n" << ec);
    set_bci(ec.addr);

    int pop_amount = 0;
    // Restoring frames
    for (auto riter = frames.rbegin(); riter != frames.rend(); ++riter) {
        if (*riter == ec.frame_position) {
            break;
        }
        ++pop_amount;
    }

    for (int i = 0; i < pop_amount; ++i) {
        pop_frame();
    }

    // Restoring finally stack
    auto tf = get_top_frame();
    for (size_t i = 0; i < tf->get_finally_stack_size() - ec.finally_stack_size; ++i) {
        tf->pop_finally_stack();
    }

    pop_amount = 0;
    // Restoring call frames
    for (auto riter = call_frames.rbegin(); riter != call_frames.rend(); ++riter) {
        if (*riter == ec.cf_position) {
            break;
        }
        ++pop_amount;
    }

    for (int i = 0; i < pop_amount; ++i) {
        pop_call_frame();
    }

    // Setting exception value
    auto reg = get_free_reg(get_top_frame());
    store(reg, v);
    store_name(reg, ec.name);
}

void Interpreter::restore_to_global_frame() {
    LOG1("Restoring interpreter to global frame position");
    // clear() should be calling destructors as well
    call_frames.clear();
    catches.clear();

    assert(!frames.empty() && "sanity check");
    assert(!const_pools.empty() && "sanity check");
    frames.erase(std::next(frames.begin()), frames.end());
    const_pools.erase(std::next(frames.begin()), frames.end());    
}

std::optional<moss::ExceptionCatch> Interpreter::get_catch_for_exception(Value *exc, bool only_current_frame) {
    for (auto riter = catches.rbegin(); riter != catches.rend(); ++riter) {
        auto ec = *riter;
        if (only_current_frame && ec.frame_position != get_top_frame())
            return std::nullopt;
        if (!ec.type || opcode::is_type_eq_or_subtype(exc->get_type(), ec.type)) {
            return ec;
        }
    }
    return std::nullopt;
}


void Interpreter::run() {
    LOG1("Running interpreter of " << (src_file ? src_file->get_name() : "??") << "\n----- OUTPUT: -----");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        try {
            opc->exec(this);
        } catch (Value *v) {
            // FIXME: this does not handle case where catch is not run 
            if (has_finally() && !is_try_not_in_catch()) {
                LOGMAX("Raise before running finally - run finally");
                call_finally();
            } else {
                // Match to known catches otherwise let fall through to next interpreter
                // or interpreter owner to print or exit or both
                bool handled = false;
                for (auto riter = catches.rbegin(); riter != catches.rend(); ++riter) {
                    auto ec = *riter;
                    if (!ec.type || opcode::is_type_eq_or_subtype(v->get_type(), ec.type)) {
                        LOGMAX("Caught exception");
                        handle_exception(ec, v);
                        handled = true;
                        break;
                    }
                }
                // Rethrow exception to be handled by next interpreter or unhandled
                if (!handled) {
                    if (has_finally()) {
                        LOGMAX("Uncaught raise, run finally before rethrow");
                        call_finally();
                    }else {
                        LOGMAX("Rethrowing exception, no catch caught it");
                        throw v;
                    }
                }
            }
        }
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
    //assert((!is_main() || !has_finally()) && "Unrun finally");
    LOG1("Finished interpreter");
}