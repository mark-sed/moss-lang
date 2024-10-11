#include "bytecode_reader.hpp"
#include "errors.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"
#include "logging.hpp"
#include <cstdlib>

using namespace moss;
using namespace moss::opcode;

Register BytecodeReader::read_register() {
    char buffer[BC_REGISTER_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_REGISTER_SIZE);
    return *(Register *)&buffer[0];
}

StringConst BytecodeReader::read_string() {
    char buffer[BC_STR_LEN_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_STR_LEN_SIZE);
    strlen_t str_len = *(strlen_t *)&buffer[0];

    if (str_len > buffer_size) {
        // Change buffer to size of nearest higher multiple of 64
        buffer_size = ((str_len - 1) | 63) + 1;
        this->str_buffer = (char *)std::realloc(str_buffer, buffer_size);
    }

    this->stream->read(str_buffer, str_len);
    return StringConst(str_buffer, str_len);
}

IntConst BytecodeReader::read_const_int() {
    char buffer[BC_INT_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_INT_SIZE);
    return *(IntConst *)&buffer[0];
}

FloatConst BytecodeReader::read_const_float() {
    char buffer[BC_FLOAT_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_FLOAT_SIZE);
    return *(FloatConst *)&buffer[0];
}

BoolConst BytecodeReader::read_const_bool() {
    char buffer[BC_BOOL_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_BOOL_SIZE);
    return *(BoolConst *)&buffer[0];
}

Address BytecodeReader::read_address() {
    char buffer[BC_ADDR_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_ADDR_SIZE);
    return *(Address *)&buffer[0];
}

Bytecode *BytecodeReader::read() {
    LOG1("Reading bytecode from file " << this->file.get_name());

    Bytecode *bc = new Bytecode();

    char opcode_char;
    opcode_t opcode;

    // TODO: first read bytecode header with version and such

    do {
        this->stream->read(&opcode_char, BC_OPCODE_SIZE);
        opcode = static_cast<opcode_t>(opcode_char);
        if (this->stream->eof()) 
            break;

        switch (opcode) {
            case opcode::OpCodes::END: {
                bc->push_back(new End());
            } break;
            case opcode::OpCodes::LOAD: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new Load(reg, str));
            } break;
            case opcode::OpCodes::LOAD_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new LoadAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::LOAD_GLOBAL: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new LoadGlobal(reg, str));
            } break;
            case opcode::OpCodes::LOAD_NONLOC: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new LoadNonLoc(reg, str));
            } break;
            case opcode::OpCodes::STORE: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Store(reg1, reg2));
            } break;
            case opcode::OpCodes::STORE_NAME: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new StoreName(reg, str));
            } break;
            case opcode::OpCodes::STORE_CONST: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new StoreConst(reg1, reg2));
            } break;
            case opcode::OpCodes::STORE_ADDR: {
                auto reg = read_register();
                auto addr = read_address();
                bc->push_back(new StoreAddr(reg, addr));
            } break;
            case opcode::OpCodes::STORE_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new StoreAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::STORE_ADDR_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new StoreAddrAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::STORE_CONST_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new StoreConstAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::STORE_INT_CONST: {
                auto reg = read_register();
                auto cint = read_const_int();
                bc->push_back(new StoreIntConst(reg, cint));
            } break;
            case opcode::OpCodes::STORE_FLOAT_CONST: {
                auto reg = read_register();
                auto cfl = read_const_float();
                bc->push_back(new StoreFloatConst(reg, cfl));
            } break;
            case opcode::OpCodes::STORE_BOOL_CONST: {
                auto reg = read_register();
                auto cbool = read_const_bool();
                bc->push_back(new StoreBoolConst(reg, cbool));
            } break;
            case opcode::OpCodes::STORE_STRING_CONST: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new StoreStringConst(reg, str));
            } break;
            case opcode::OpCodes::STORE_NIL_CONST: {
                bc->push_back(new StoreNilConst(read_register()));
            } break;
            case opcode::OpCodes::JMP: {
                bc->push_back(new Jmp(read_address()));
            } break;
            case opcode::OpCodes::JMP_IF_TRUE: {
                auto reg = read_register();
                auto addr = read_address();
                bc->push_back(new JmpIfTrue(reg, addr));
            } break;
            case opcode::OpCodes::JMP_IF_FALSE: {
                auto reg = read_register();
                auto addr = read_address();
                bc->push_back(new JmpIfFalse(reg, addr));
            } break;
            case opcode::OpCodes::CALL: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Call(reg1, reg2));
            } break;
            case opcode::OpCodes::PUSH_FRAME: {
                bc->push_back(new PushFrame());
            } break;
            case opcode::OpCodes::POP_FRAME: {
                bc->push_back(new PopFrame());
            } break;
            case opcode::OpCodes::PUSH_CALL_FRAME: {
                bc->push_back(new PushCallFrame());
            } break;
            case opcode::OpCodes::POP_CALL_FRAME: {
                bc->push_back(new PopCallFrame());
            } break;
            case opcode::OpCodes::RETURN: {
                bc->push_back(new Return(read_register()));
            } break;
            case opcode::OpCodes::RETURN_CONST: {
                bc->push_back(new ReturnConst(read_register()));
            } break;
            case opcode::OpCodes::RETURN_ADDR: {
                bc->push_back(new ReturnAddr(read_address()));
            } break;
            case opcode::OpCodes::PUSH_ARG: {
                bc->push_back(new PushArg(read_register()));
            } break;
            case opcode::OpCodes::PUSH_CONST_ARG: {
                bc->push_back(new PushConstArg(read_register()));
            } break;
            case opcode::OpCodes::PUSH_ADDR_ARG: {
                bc->push_back(new PushAddrArg(read_address()));
            } break;
            case opcode::OpCodes::PUSH_NAMED_ARG: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new PushNamedArg(reg, str));
            } break;
            case opcode::OpCodes::IMPORT: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new Import(reg, str));
            } break;
            case opcode::OpCodes::IMPORT_ALL: {
                bc->push_back(new ImportAll(read_string()));
            } break;
            case opcode::OpCodes::PUSH_PARENT: {
                bc->push_back(new PushParent(read_register()));
            } break;
            case opcode::OpCodes::CREATE_OBJ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new CreateObject(reg1, reg2));
            } break;
            case opcode::OpCodes::PROMOTE_OBJ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new PromoteObject(reg1, reg2));
            } break;
            case opcode::OpCodes::BUILD_CLASS: {
                bc->push_back(new BuildClass(read_register()));
            } break;
            case opcode::OpCodes::COPY: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Copy(reg1, reg2));
            } break;
            case opcode::OpCodes::DEEP_COPY: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new DeepCopy(reg1, reg2));
            } break;
            case opcode::OpCodes::CREATE_ANNT: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new CreateAnnt(reg, str));
            } break;
            case opcode::OpCodes::ANNOTATE: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Annotate(reg1, reg2));
            } break;
            case opcode::OpCodes::OUTPUT: {
                bc->push_back(new Output(read_register()));
            } break;
            case opcode::OpCodes::CONCAT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Concat(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EXP: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Exp(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::ADD: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Add(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUB: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Sub(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::DIV: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Div(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MUL: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mul(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MOD: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mod(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EQ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Eq(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::NEQ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Neq(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Bt(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Lt(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BEQ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Beq(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LEQ: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Leq(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::IN: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new In(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::AND: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new And(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::OR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Or(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::XOR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Xor(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUBSC: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Subsc(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SLICE: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Slice(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::CONCAT2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Concat2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EXP2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Exp2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::ADD2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Add2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUB2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Sub2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::DIV2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Div2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MUL2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mul2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MOD2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mod2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EQ2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Eq2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::NEQ2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Neq2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BT2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Bt2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LT2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Lt2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BEQ2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Beq2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LEQ2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Leq2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::IN2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new In2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::AND2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new And2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::OR2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Or2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::XOR2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Xor2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUBSC2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Subsc2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SLICE2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Slice2(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::CONCAT3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Concat3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EXP3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Exp3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::ADD3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Add3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUB3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Sub3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::DIV3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Div3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MUL3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mul3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::MOD3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Mod3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::EQ3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Eq3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::NEQ3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Neq3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BT3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Bt3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LT3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Lt3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BEQ3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Beq3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::LEQ3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Leq3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::IN3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new In3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::AND3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new And3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::OR3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Or3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::XOR3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Xor3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUBSC3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new Subsc3(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::NOT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Not(reg1, reg2));
            } break;
            case opcode::OpCodes::NEG: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Neg(reg1, reg2));
            } break;
            case opcode::OpCodes::ASSERT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Assert(reg1, reg2));
            } break;
            case opcode::OpCodes::COPY_ARGS: {
                bc->push_back(new CopyArgs());
            } break;
            case opcode::OpCodes::RAISE: {
                bc->push_back(new Raise(read_register()));
            } break;
            case opcode::OpCodes::CHECK_CATCH: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new CheckCatch(reg1, reg2));
            } break;
            case opcode::OpCodes::LIST_PUSH: {
                bc->push_back(new ListPush(read_register()));
            } break;
            case opcode::OpCodes::LIST_PUSH_CONST: {
                bc->push_back(new ListPushConst(read_register()));
            } break;
            case opcode::OpCodes::LIST_PUSH_ADDR: {
                bc->push_back(new ListPushAddr(read_address()));
            } break;
            case opcode::OpCodes::BUILD_LIST: {
                bc->push_back(new BuildList(read_register()));
            } break;
            case opcode::OpCodes::BUILD_DICT: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new BuildDict(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::BUILD_ENUM: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new BuildEnum(reg1, reg2));
            } break;
            case opcode::OpCodes::CREATE_RANGE: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE2: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange2(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE3: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange3(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE4: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange4(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE5: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange5(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE6: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange6(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE7: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange7(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::CREATE_RANGE8: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto reg4 = read_register();
                bc->push_back(new CreateRange8(reg1, reg2, reg3, reg4));
            } break;
            case opcode::OpCodes::SWITCH: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                auto addr = read_address();
                bc->push_back(new Switch(reg1, reg2, reg3, addr));
            } break;
            case opcode::OpCodes::FOR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new For(reg1, reg2));
            } break;
            default: 
                std::string msg = "unknown opcode in bytecode reader: "+std::to_string(opcode);
                error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
        }
    } while(!this->stream->eof());

    LOG1("Bytecode read and converted:\n" << *bc);
    return bc;
}