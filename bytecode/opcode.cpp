#include "opcode.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include <sstream>
#include <cmath>

#include <iostream>

using namespace moss;
using namespace moss::opcode;

std::string OpCode::err_mgs(std::string msg, Interpreter *vm) {
    std::stringstream ss;
    ss << vm->get_bci() << "\t" << *this << " :: " << msg;
    return ss.str();
}

/*
When loading unknown value it will be nil, so it cannot be nullptr
void OpCode::check_load(Value *v, Interpreter *vm) {
    if (v) return;
    auto msg = err_mgs("Loading value from non-existent register", vm);
    error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
}*/

/**
 * True iff one of the values is a float and the other is float or int
 */
static bool is_float_expr(Value *v1, Value *v2) {
    return (isa<FloatValue>(v1) && (isa<FloatValue>(v2) || isa<IntValue>(v2))) || 
        (isa<FloatValue>(v2) && (isa<FloatValue>(v1) || isa<IntValue>(v1)));
}

/**
 * True iff both values are ints
 */
static bool is_int_expr(Value *v1, Value *v2) {
    return isa<IntValue>(v1) && isa<IntValue>(v2);
}

static bool is_primitive_type(Value *v) {
    return v->get_kind() != TypeKind::USER_DEF;
}

void End::exec(Interpreter *vm) {
    // No op
}

void Load::exec(Interpreter *vm) {
    auto *v = vm->load_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->store(this->dst, v);
    vm->store_name(dst, name);
}

void LoadAttr::exec(Interpreter *vm) {
    auto *v = vm->load(this->src);
    auto attr = v->get_attr(this->name);
    assert(attr && "TODO: Nonexistent attr raise exception");
    attr->inc_refs();
    vm->store(this->dst, attr);
}

void LoadGlobal::exec(Interpreter *vm) {
    auto *v = vm->load_global_name(this->name);
    // FIXME:
    assert(v && "TODO: Nonexistent name raise exception");
    v->inc_refs();
    vm->store(this->dst, v);
}

void LoadNonLoc::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Store::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    v->inc_refs();
    vm->store(this->dst, v);
}

void StoreName::exec(Interpreter *vm) {
    vm->store_name(dst, name);
}

void StoreConst::exec(Interpreter *vm) {
    auto c = vm->load_const(csrc);
    c->inc_refs();
    vm->store(dst, c);
}

void StoreAddr::exec(Interpreter *vm) {
    vm->store(dst, new AddrValue(addr));
}

void StoreAttr::exec(Interpreter *vm) {
    auto *dstobj = vm->load(this->obj);
    auto *v = vm->load(this->src);
    v->inc_refs();
    dstobj->set_attr(this->name, v);
}

void StoreAddrAttr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void StoreConstAttr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void StoreIntConst::exec(Interpreter *vm) {
    vm->store_const(dst, new IntValue(val));
}

void StoreFloatConst::exec(Interpreter *vm) {
    vm->store_const(dst, new FloatValue(val));
}

void StoreBoolConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new BoolValue(val));
}

void StoreStringConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new StringValue(val));
}

void StoreNilConst::exec(Interpreter *vm) {
    // TODO: Load precreated value
    vm->store_const(dst, new NilValue());
}

void Jmp::exec(Interpreter *vm) {
    vm->set_bci(this->addr);
}

void JmpIfTrue::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

    if (bc->get_value())
        vm->set_bci(this->addr);
}

void JmpIfFalse::exec(Interpreter *vm) {
    auto *v = vm->load(src);
    auto bc = dyn_cast<BoolValue>(v);
    if (!bc) {
        auto msg = err_mgs("Expected Bool value, but got "+v->get_name(), vm);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), vm->get_src_file(), true);
    }

    if (!bc->get_value())
        vm->set_bci(this->addr);
}

void Call::exec(Interpreter *vm) {
    vm->get_call_frame()->set_return_reg(dst);
    vm->get_call_frame()->set_caller_addr(vm->get_bci()+1);
    auto fun_name = vm->get_reg_name(src);
    assert(!fun_name.empty() && "TODO: Raise such function does not exists");
    fun_name += "(";
    bool first = true;
    // Encode function
    for (auto a: vm->get_call_frame()->get_args()) {
        if (first) {
            fun_name += a->get_name();
            first = false;
        }
        else {
            fun_name += "," + a->get_name();
        }
    }
    fun_name += ")";
    // Load address of encoded function
    auto fun_val = vm->load_name(fun_name);
    if (!fun_val) {
        // TODO: Handle inheritance
        assert(false && "TODO: Raise function not found");
    }
    auto fun_addr = dyn_cast<AddrValue>(fun_val);
    if (!fun_addr) {
        assert(false && "TODO: Raise cannot be called");
    }
    vm->push_frame();
    vm->set_bci(fun_addr->get_value());
}

void PushFrame::exec(Interpreter *vm) {
    vm->push_frame();
}

void PopFrame::exec(Interpreter *vm) {
    vm->pop_frame();
}

void PushCallFrame::exec(Interpreter *vm) {
    vm->push_call_frame();
}

void PopCallFrame::exec(Interpreter *vm) {
    // This pops values from call frame into current frame
    // The acutal call frame is removed in return (when return value is set)
    Register r_i = 0;
    for (auto v: vm->get_call_frame()->get_args()) {
        vm->store(r_i, v);
        ++r_i;
    }
}

void Return::exec(Interpreter *vm) {
    auto return_reg = vm->get_call_frame()->get_return_reg();
    auto caller_addr = vm->get_call_frame()->get_caller_addr();
    auto ret_v = vm->load(src);
    vm->pop_call_frame();
    vm->pop_frame();
    assert(ret_v && "Return register does not contain any value??");
    vm->store(return_reg, ret_v);
    vm->set_bci(caller_addr);
}

void ReturnConst::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ReturnAddr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushArg::exec(Interpreter *vm) {
    auto v = vm->load(src);
    assert(v && "Non existent push reg");
    vm->get_call_frame()->push_back(v);
}

void PushConstArg::exec(Interpreter *vm) {
    auto v = vm->load_const(csrc);
    assert(v && "Const does not exist??");
    vm->get_call_frame()->push_back(v);
}

void PushAddrArg::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushNamedArg::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Import::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ImportAll::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PushParent::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void CreateObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void PromoteObject::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildClass::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Copy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void DeepCopy::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void CreateAnnt::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Annotate::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Output::exec(Interpreter *vm) {
    // FIXME: this is just a placeholder
    auto *v = vm->load(src);
    
    std::cout << v->as_string();
}

static Value *concat(Value *s1, Value *s2, Interpreter *vm) {
    assert(s1 && "Value or nil should have been loaded");
    assert(s2 && "Value or nil should have been loaded");

    ustring s1_str = s1->as_string();
    ustring s2_str = s2->as_string();

    return new StringValue(s1_str + s2_str);
}

void Concat::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat2::exec(Interpreter *vm) {
    auto res = concat(vm->load_const(src1), vm->load(src2), vm);
    vm->store(dst, res);
}

void Concat3::exec(Interpreter *vm) {
    auto res = concat(vm->load(src1), vm->load_const(src2), vm);
    vm->store(dst, res);
}

// TODO: For most exprs is to check over/underflow when check is enabled
// TODO: Call user defined method for non native type
static Value *exp(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(static_cast<long>(std::pow(static_cast<double>(i1->get_value()), static_cast<double>(i2->get_value()))));
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(std::pow(s1->as_float(), s2->as_float()));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Exp::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Exp2::exec(Interpreter *vm) {
    auto res = exp(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Exp3::exec(Interpreter *vm) {
    auto res = exp(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *add(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() + i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() + s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Add::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Add2::exec(Interpreter *vm) {
    auto res = add(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Add3::exec(Interpreter *vm) {
    auto res = add(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *sub(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() - i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() - s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Sub::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Sub2::exec(Interpreter *vm) {
    auto res = sub(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Sub3::exec(Interpreter *vm) {
    auto res = sub(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *div(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        if (i2->get_value() == 0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0 exception raise");
        }
        res = new IntValue(i1->get_value() / i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        if (s2->as_float() == 0.0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0.0 exception raise");
        }
        res = new FloatValue(s1->as_float() / s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Div::exec(Interpreter *vm) {
    auto res = div(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Div2::exec(Interpreter *vm) {
    auto res = div(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Div3::exec(Interpreter *vm) {
    auto res = div(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *mul(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() * i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new FloatValue(s1->as_float() * s2->as_float());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Mul::exec(Interpreter *vm) {
    auto res = mul(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Mul2::exec(Interpreter *vm) {
    auto res = mul(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Mul3::exec(Interpreter *vm) {
    auto res = mul(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *mod(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        if (i2->get_value() == 0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0 exception raise");
        }
        res = new IntValue(i1->get_value() % i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        if (s2->as_float() == 0.0) {
            // FIXME: Raise division by 0 exception
            assert(false && "TODO: division by 0.0 exception raise");
        }
        res = new FloatValue(std::fmod(s1->as_float(), s2->as_float()));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Mod::exec(Interpreter *vm) {
    auto res = mod(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Mod2::exec(Interpreter *vm) {
    auto res = mod(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Mod3::exec(Interpreter *vm) {
    auto res = mod(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *eq(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (s1->get_kind() == s2->get_kind()) {
        if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
            IntValue *i2 = dyn_cast<IntValue>(s2);
            res = new BoolValue(i1->get_value() == i2->get_value());
        }
        else if (FloatValue *f1 = dyn_cast<FloatValue>(s1)) {
            FloatValue *f2 = dyn_cast<FloatValue>(s2);
            res = new BoolValue(f1->get_value() == f2->get_value());
        }
        else if (BoolValue *b1 = dyn_cast<BoolValue>(s1)) {
            BoolValue *b2 = dyn_cast<BoolValue>(s2);
            res = new BoolValue(b1->get_value() == b2->get_value());
        }
        else if (StringValue *st1 = dyn_cast<StringValue>(s1)) {
            StringValue *st2 = dyn_cast<StringValue>(s2);
            res = new BoolValue(st1->get_value() == st2->get_value());
        }
        else if (isa<NilValue>(s1)) {
            res = new BoolValue(true);
        }
        else {
            assert(false && "TODO: Call custom method for eq");
        }
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() == s2->as_float());
    }
    else if (isa<NilValue>(s1) || isa<NilValue>(s1)) {
        res = new BoolValue(false);
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Eq::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Eq2::exec(Interpreter *vm) {
    auto res = eq(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Eq3::exec(Interpreter *vm) {
    auto res = eq(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *neq(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_primitive_type(s1) && is_primitive_type(s2)) {
        auto eqRes = eq(s1, s2, vm);
        auto neqRes = new BoolValue(!dyn_cast<BoolValue>(eqRes)->get_value());
        delete eqRes;
        return neqRes;
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Neq::exec(Interpreter *vm) {
    auto res = neq(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Neq2::exec(Interpreter *vm) {
    auto res = neq(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Neq3::exec(Interpreter *vm) {
    auto res = neq(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *bt(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() > i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() > s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() > st2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Bt::exec(Interpreter *vm) {
    auto res = bt(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Bt2::exec(Interpreter *vm) {
    auto res = bt(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Bt3::exec(Interpreter *vm) {
    auto res = bt(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *lt(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() < i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() < s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() < st2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Lt::exec(Interpreter *vm) {
    auto res = lt(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Lt2::exec(Interpreter *vm) {
    auto res = lt(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Lt3::exec(Interpreter *vm) {
    auto res = lt(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *beq(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() >= i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() >= s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() >= st2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Beq::exec(Interpreter *vm) {
    auto res = beq(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Beq2::exec(Interpreter *vm) {
    auto res = beq(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Beq3::exec(Interpreter *vm) {
    auto res = beq(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *leq(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new BoolValue(i1->get_value() <= i2->get_value());
    }
    else if (is_float_expr(s1, s2)) {
        res = new BoolValue(s1->as_float() <= s2->as_float());
    }
    else if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        res = new BoolValue(st1->get_value() <= st2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Leq::exec(Interpreter *vm) {
    auto res = leq(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Leq2::exec(Interpreter *vm) {
    auto res = leq(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Leq3::exec(Interpreter *vm) {
    auto res = leq(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *in(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (isa<StringValue>(s1) && isa<StringValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        StringValue *st2 = dyn_cast<StringValue>(s2);
        // s1 in s2 => s2.find(s1)
        res = new BoolValue(st2->get_value().find(st1->get_value()) != ustring::npos);
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void In::exec(Interpreter *vm) {
    auto res = in(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void In2::exec(Interpreter *vm) {
    auto res = in(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void In3::exec(Interpreter *vm) {
    auto res = in(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *andOP(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() & i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() && b2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void And::exec(Interpreter *vm) {
    auto res = andOP(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void And2::exec(Interpreter *vm) {
    auto res = andOP(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void And3::exec(Interpreter *vm) {
    auto res = andOP(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *orOP(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() | i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() || b2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Or::exec(Interpreter *vm) {
    auto res = orOP(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Or2::exec(Interpreter *vm) {
    auto res = orOP(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Or3::exec(Interpreter *vm) {
    auto res = orOP(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *xorOP(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (is_int_expr(s1, s2)) {
        IntValue *i1 = dyn_cast<IntValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        res = new IntValue(i1->get_value() ^ i2->get_value());
    }
    else if (isa<BoolValue>(s1) && isa<BoolValue>(s2)) {
        BoolValue *b1 = dyn_cast<BoolValue>(s1);
        BoolValue *b2 = dyn_cast<BoolValue>(s2);
        res = new BoolValue(b1->get_value() ^ b2->get_value());
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Xor::exec(Interpreter *vm) {
    auto res = xorOP(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Xor2::exec(Interpreter *vm) {
    auto res = xorOP(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Xor3::exec(Interpreter *vm) {
    auto res = xorOP(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *subsc(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    if (isa<StringValue>(s1) && isa<IntValue>(s2)) {
        StringValue *st1 = dyn_cast<StringValue>(s1);
        IntValue *i2 = dyn_cast<IntValue>(s2);
        if (i2->get_value() < 0) {
            assert(false && "TODO: negative index");
        }
        else if (static_cast<unsigned long>(i2->get_value()) >= st1->get_value().size()) {
            assert(false && "TODO: out of bounds exception");
        }
        res = new StringValue(ustring(1, st1->get_value()[i2->get_value()]));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    return res;
}

void Subsc::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Subsc2::exec(Interpreter *vm) {
    auto res = subsc(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Subsc3::exec(Interpreter *vm) {
    auto res = subsc(vm->load(src1), vm->load_const(src2), vm);
    if (res)
        vm->store(dst, res);
}

static Value *slice(Value *s1, Value *s2, Interpreter *vm) {
    Value *res = nullptr;
    assert(false && "TODO: Unimplemented opcode");
    return res;
}

void Slice::exec(Interpreter *vm) {
    auto res = slice(vm->load(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Slice2::exec(Interpreter *vm) {
    auto res = slice(vm->load_const(src1), vm->load(src2), vm);
    if (res)
        vm->store(dst, res);
}

void Not::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    Value *res = nullptr;
    if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
        res = new IntValue(~(i1->get_value()));
    }
    else if (BoolValue *b1 = dyn_cast<BoolValue>(s1)) {
        res = new BoolValue(!(b1->get_value()));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    if (res)
        vm->store(dst, res);
}

void Neg::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    Value *res = nullptr;
    if (IntValue *i1 = dyn_cast<IntValue>(s1)) {
        res = new IntValue(-(i1->get_value()));
    }
    else if (FloatValue *f1 = dyn_cast<FloatValue>(s1)) {
        res = new FloatValue(-(f1->get_value()));
    }
    else {
        // FIXME: Raise unsupported operator type exception
        assert(false && "TODO: unsupported operator type raise exception");
    }
    if (res)
        vm->store(dst, res);
}

void Assert::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    auto *assert_msg = vm->load(msg);
    
    auto *check = dyn_cast<BoolValue>(s1);
    assert(check && "Assertion value is not a boolean");
    auto *str_msg = dyn_cast<StringValue>(assert_msg);
    assert(str_msg && "Assertion message is not a string");

    if (!check->get_value()) {
        assert(false && "TODO: Raise assertion exception");
    }
}

void CopyArgs::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void Raise::exec(Interpreter *vm) {
    auto *s1 = vm->load(src);
    // TODO: Change
    auto *str_msg = dyn_cast<StringValue>(s1);
    errs << str_msg->get_value();
}

void CheckCatch::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ListPush::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ListPushConst::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void ListPushAddr::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildList::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildDict::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void BuildEnum::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

static Value *range(Value *start, Value *step, Value *end, Interpreter *vm) {
    Value *res = nullptr;
    assert(false && "TODO: Unimplemented range generation");
    return res;
}

void CreateRange::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange2::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange3::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load_const(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange4::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange5::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load_const(step), vm->load(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange6::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange7::exec(Interpreter *vm) {
    auto res = range(vm->load(start), vm->load_const(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void CreateRange8::exec(Interpreter *vm) {
    auto res = range(vm->load_const(start), vm->load_const(step), vm->load_const(end), vm);
    if (res)
        vm->store(dst, res);
}

void Switch::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

void For::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

/*

void ::exec(Interpreter *vm) {
    assert(false && "TODO: Unimplemented opcode");
}

*/