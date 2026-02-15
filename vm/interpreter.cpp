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
std::list<FrameInfo> Interpreter::stack_frames{};
std::vector<Value *> Interpreter::unwound_funs{};
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
        : code(code), src_file(src_file), vms_module(nullptr), bci(0),
          exit_code(0), bci_modified(false), stop(false), main(main),
          marked(false), main_to_run(nullptr) {
    if (main && !gc) {
        gc = new gcs::TracingGC(this);
    }
    gc->push_vm(this);
    assert(gc && "sanity check");
    this->const_pools.push_back(new MemoryPool(this, true, true));
    // Global frame
    if (main)
        push_frame(new MemoryPool(this, false, true), false);
    else
        frames.push_back(new MemoryPool(this, false, true));
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
    init_global_module_values(glob_reg);
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

static inline void store_glob_val(opcode::Register reg, ustring name, Value *v, MemoryPool *gf) {
    gf->store(reg, v);
    gf->store_name(reg, name);
}

void Interpreter::init_global_module_values(opcode::Register &reg) {
    auto gf = get_global_frame();
    // Constants specific to a module
    ustring fname = "";
    if (get_src_file())
        fname = get_src_file()->get_path();
    store_glob_val(reg++, "__FILE", StringValue::get(fname), gf);

    ustring mname = "";
    if (get_src_file())
        fname = get_src_file()->get_module_name();
    store_glob_val(reg++, "__NAME", StringValue::get(fname), gf);
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
    for (auto *fun_val : unwound_funs) {
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
    auto lf = new MemoryPool(this);
    this->frames.push_back(lf);
    CallFrame *matching_cf = nullptr;
    // Match cf only if this frame is for a function
    if (fun_owner && isa<FunValue>(fun_owner) && has_call_frame()) {
        matching_cf = get_call_frame();
        if (!matching_cf->is_matched_to_frame()) {
            matching_cf->set_matched_to_frame(true);
        } else {
            matching_cf = nullptr;
        }
    }
    Interpreter::stack_frames.push_back({lf, matching_cf});
    this->const_pools.push_back(new MemoryPool(this, true));
    if (fun_owner)
        lf->set_pool_owner(fun_owner);
        
}

void Interpreter::push_frame(MemoryPool *pool, bool push_const) {
    LOGMAX("Passed in frame pushed");
    this->frames.push_back(pool);
    CallFrame *cf = nullptr;
    auto owner = pool->get_pool_owner();
    if (has_call_frame() && !pool->is_global() && (!owner || isa<FunValue>(owner))) {
        auto poss_cf = get_call_frame();
        if (!poss_cf->is_matched_to_frame()) {
            cf = poss_cf;
            poss_cf->set_matched_to_frame(true);
        }
    }
    Interpreter::stack_frames.push_back({pool, cf});
    if (push_const)
        this->const_pools.push_back(new MemoryPool(this, true));
}

void Interpreter::pop_frame() {
    LOGMAX("Frame popped");
    assert(frames.size() > 1 && "Trying to pop global frame");
    auto f = frames.back();
    frames.pop_back();
    Interpreter::stack_frames.pop_back();
    gc->push_popped_frame(f);
    assert(const_pools.size() > 1 && "Trying to pop global const frame");
    auto c = const_pools.back();
    const_pools.pop_back();
    delete c;
}

void Interpreter::cross_module_call(FunValue *fun, CallFrame *cf) {
    // No frame push as it will be done in specialized run
    call_frames.push_back(cf);
    set_bci(fun->get_body_addr());
    auto frm = new MemoryPool(this);
    frm->set_pool_owner(fun);
    try {
        run_from_external(frm);
    } catch (Value *e) {
        LOGMAX("Exception in cross_module_call, pop_frame and rethrow");
        throw e;
    }
}

void Interpreter::runtime_call(FunValue *fun) {
    auto pre_call_bci = this->bci;
    auto pre_bci_modified = this->bci_modified;
    auto pre_stop = this->stop;

    LOGMAX("Runtime call to " << *fun << " with: " << *get_call_frame());

    // No frame push as it will be done in specialized run
    get_call_frame()->set_function(fun);
    set_bci(fun->get_body_addr());
    try {
        run_from_external(new MemoryPool(this));
    } catch (Value *e) {
        LOGMAX("Exception in runtime_call, restore vm info and rethrow");
        this->bci = pre_call_bci;
        this->bci_modified = pre_bci_modified;
        this->stop = pre_stop;
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

void Interpreter::exception_in_internal_call() {
    LOGMAX("Exception in internal call, pushing frame to unwound frames and dropping it.")
    unwound_funs.push_back(get_call_frame()->get_function());
    pop_call_frame();
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

void Interpreter::push_call_frame(Value *fun) {
    call_frames.push_back(new CallFrame(fun));
}

void Interpreter::push_catch(ExceptionCatch ec) {
    get_local_frame()->push_catch(ec);
}

void Interpreter::pop_catch(opcode::IntConst amount) {
    get_local_frame()->pop_catch(amount);
}

#ifndef NDEBUG
void Interpreter::print_stack_frame(ustring msg) {
    if (!msg.empty())
        outs << "Stack frame at: " << msg << ":\n";
    outs << "---v-TOP-v---\n";
    size_t i = 0;
    for (auto rfi = stack_frames.rbegin(); rfi != stack_frames.rend(); ++rfi, ++i) {
        auto frm = *rfi;
        if (frm.frame->get_pool_owner())
            outs << i << ". " << frm.frame->get_pool_owner()->get_name();
        else {
            outs << i << ". ";
            if (frm.frame->is_global())
                outs << "GF";
            else {
                outs << "Object frame";
            }
        }
        outs << " [" << frm.frame->get_vm_owner()->get_src_file()->get_module_name() << "]";
        if (!frm.call_frame) {
            outs << "\t\t<no CF>";
        } else {
            outs << "\t\t<" << frm.call_frame->get_function()->get_name() << ">";
        }
        if (this->frames.size() >= i) {
            auto it = this->frames.rbegin();
            std::advance(it, i);
            auto f = *it;
            outs << "\t\t|" << i << ". ";
            if (f->is_global())
                outs << "GF";
            else {
                if (f->get_pool_owner())
                    outs << f->get_pool_owner()->get_name();
                else
                    outs << "Object frame";
            }
            outs << " [" << f->get_vm_owner()->get_src_file()->get_module_name() << "]";
            outs << "|";
        }
        outs << "\n";
    }
    outs << "--^-BOTTOM-^--\n";
}
#endif //NDEBUG

void Interpreter::unwind_stacks(FrameInfo fi) {
    while(stack_frames.back().frame != fi.frame) {
        auto frinf = stack_frames.back();
        if (!frinf.frame->is_global())
            pop_frame();
        if (frinf.call_frame) {
            unwound_funs.push_back(frinf.call_frame->get_function());
            pop_call_frame();
        }
    }
}

void Interpreter::restore_to_global_frame() {
    LOG1("Restoring interpreter to global frame position");
    call_frames.clear();
    get_global_frame()->get_catches().clear();

    assert(!frames.empty() && "sanity check");
    assert(!const_pools.empty() && "sanity check");
    frames.erase(std::next(frames.begin()), frames.end());
    const_pools.erase(std::next(frames.begin()), frames.end());    
}

void Interpreter::run_from_external(MemoryPool *caller_frame) {
    push_frame(get_global_frame());
    if (caller_frame)
        push_frame(caller_frame);
    try {
        run();
    } catch (Value *e) {
        assert((!stack_frames.empty() && stack_frames.back().frame == get_global_frame()) && "Expected global frame on top");
        pop_frame(); // Popping this global frame (duplicated)
        throw e; // rethrow
    }
    pop_frame();
}

void Interpreter::run() {
    LOG1("Running interpreter of " << (src_file ? src_file->get_name() : "??") << "\n----- OUTPUT: -----");

    while(bci < code->size()) {
        opcode::OpCode *opc = (*code)[bci];
        try {
            opc->exec(this);
            // If we get to exectute opcode and unwound stack is not empty, then
            // we got here after some exception was handled or reported (repl),
            // so just clear it. The check for empty() for vector is O(1).
            if (!unwound_funs.empty()) {
                unwound_funs.clear();
            }
        } catch (Value *v) { 
            if (has_finally() && !is_try_not_in_catch()) {
                LOGMAX("Raise before running finally - run finally");
                call_finally();
            } else {
                // Match to known catches otherwise let fall through to next interpreter
                // or interpreter owner to print or exit or both
                bool handled = false;
                FrameInfo prev_p = {nullptr, nullptr};
                int pop_amount = 0;
                for (auto rfi = stack_frames.rbegin(); rfi != stack_frames.rend(); ++rfi, ++pop_amount) {
                    auto frinf = *rfi;
                    auto frm = frinf.frame;
                    // The issue is that in frames are only frames of this VM
                    // but we need to walk the frames across VMs from current
                    // frame back to its caller. So global stack_frame has to be
                    // used and if the owner is not this vm, then just re-raise.
                    if (auto owner = frm->get_vm_owner()) {
                        if (owner != this) {
                            LOGMAX("Rethrowing exception, top of the stack is other VM");
                            // Restore current VMs frames
                            // Pop up until the frame before this;
                            unwind_stacks(prev_p);
                            throw v;
                        }
                    }
                    auto catches = frm->get_catches();
                    for (auto riter = catches.rbegin(); riter != catches.rend(); ++riter) {
                        auto ec = *riter;
                        if (!ec.type || opcode::is_type_eq_or_subtype(v->get_type(), ec.type)) {
                            LOGMAX("Caught exception");
                            handle_exception(ec, v);
                            handled = true;
                            break;
                        }
                    }
                    if (handled)
                        break;
                    prev_p = frinf;
                }
                // Rethrow exception to be handled by next interpreter or unhandled
                if (!handled) {
                    if (has_finally()) {
                        LOGMAX("Uncaught raise, run finally before rethrow");
                        call_finally();
                    }else {
                        LOGMAX("Unwindind and rethrowing exception, no catch caught it");
                        unwind_stacks(prev_p);
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