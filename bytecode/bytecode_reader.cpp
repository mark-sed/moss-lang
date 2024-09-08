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
                bc->push_back(new Load(read_register(), read_string()));
            } break;
            case opcode::OpCodes::LOAD_ATTR: {
                bc->push_back(new LoadAttr(read_register(), read_register(), read_string()));
            } break;
            case opcode::OpCodes::LOAD_GLOBAL: {
                bc->push_back(new LoadGlobal(read_register(), read_string()));
            } break;
            case opcode::OpCodes::LOAD_NONLOC: {
                bc->push_back(new LoadNonLoc(read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE: {
                bc->push_back(new Store(read_register(), read_register()));
            } break;
            case opcode::OpCodes::STORE_NAME: {
                bc->push_back(new StoreName(read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE_CONST: {
                bc->push_back(new StoreConst(read_register(), read_register()));
            } break;
            case opcode::OpCodes::STORE_ADDR: {
                bc->push_back(new StoreAddr(read_register(), read_address()));
            } break;
            case opcode::OpCodes::STORE_ATTR: {
                bc->push_back(new StoreAttr(read_register(), read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE_ADDR_ATTR: {
                bc->push_back(new StoreAddrAttr(read_address(), read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE_CONST_ATTR: {
                bc->push_back(new StoreConstAttr(read_register(), read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE_INT_CONST: {
                bc->push_back(new StoreIntConst(read_register(), read_const_int()));
            } break;
            case opcode::OpCodes::STORE_FLOAT_CONST: {
                bc->push_back(new StoreFloatConst(read_register(), read_const_float()));
            } break;
            case opcode::OpCodes::STORE_BOOL_CONST: {
                bc->push_back(new StoreBoolConst(read_register(), read_const_bool()));
            } break;
            case opcode::OpCodes::STORE_STRING_CONST: {
                bc->push_back(new StoreStringConst(read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE_NIL_CONST: {
                bc->push_back(new StoreNilConst(read_register()));
            } break;
            case opcode::OpCodes::JMP: {
                bc->push_back(new Jmp(read_address()));
            } break;
            case opcode::OpCodes::JMP_IF_TRUE: {
                bc->push_back(new JmpIfTrue(read_register(), read_address()));
            } break;
            case opcode::OpCodes::JMP_IF_FALSE: {
                bc->push_back(new JmpIfFalse(read_register(), read_address()));
            } break;
            case opcode::OpCodes::CALL: {
                bc->push_back(new Call(read_register(), read_address()));
            } break;
            case opcode::OpCodes::PUSH_FRAME: {
                bc->push_back(new PushFrame());
            } break;
            case opcode::OpCodes::POP_FRAME: {
                bc->push_back(new PopFrame());
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
            case opcode::OpCodes::IMPORT: {
                bc->push_back(new Import(read_register(), read_string()));
            } break;
            case opcode::OpCodes::IMPORT_ALL: {
                bc->push_back(new ImportAll(read_string()));
            } break;
            case opcode::OpCodes::PUSH_PARENT: {
                bc->push_back(new PushParent(read_register()));
            } break;
            case opcode::OpCodes::CREATE_OBJ: {
                bc->push_back(new CreateObject(read_register(), read_register()));
            } break;
            case opcode::OpCodes::PROMOTE_OBJ: {
                bc->push_back(new PromoteObject(read_register(), read_register()));
            } break;
            case opcode::OpCodes::BUILD_CLASS: {
                bc->push_back(new BuildClass(read_register()));
            } break;
            case opcode::OpCodes::COPY: {
                bc->push_back(new Copy(read_register(), read_register()));
            } break;
            case opcode::OpCodes::DEEP_COPY: {
                bc->push_back(new DeepCopy(read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_ANNT: {
                bc->push_back(new CreateAnnt(read_register(), read_string()));
            } break;
            case opcode::OpCodes::ANNOTATE: {
                bc->push_back(new Annotate(read_register(), read_register()));
            } break;
            case opcode::OpCodes::OUTPUT: {
                bc->push_back(new Output(read_register()));
            } break;
            case opcode::OpCodes::CONCAT: {
                bc->push_back(new Concat(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EXP: {
                bc->push_back(new Exp(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::ADD: {
                bc->push_back(new Add(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUB: {
                bc->push_back(new Sub(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::DIV: {
                bc->push_back(new Div(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MUL: {
                bc->push_back(new Mul(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MOD: {
                bc->push_back(new Mod(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EQ: {
                bc->push_back(new Eq(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::NEQ: {
                bc->push_back(new Neq(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BT: {
                bc->push_back(new Bt(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LT: {
                bc->push_back(new Lt(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BEQ: {
                bc->push_back(new Beq(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LEQ: {
                bc->push_back(new Leq(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::IN: {
                bc->push_back(new In(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::AND: {
                bc->push_back(new And(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::OR: {
                bc->push_back(new Or(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::XOR: {
                bc->push_back(new Xor(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUBSC: {
                bc->push_back(new Subsc(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SLICE: {
                bc->push_back(new Slice(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CONCAT2: {
                bc->push_back(new Concat2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EXP2: {
                bc->push_back(new Exp2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::ADD2: {
                bc->push_back(new Add2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUB2: {
                bc->push_back(new Sub2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::DIV2: {
                bc->push_back(new Div2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MUL2: {
                bc->push_back(new Mul2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MOD2: {
                bc->push_back(new Mod2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EQ2: {
                bc->push_back(new Eq2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::NEQ2: {
                bc->push_back(new Neq2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BT2: {
                bc->push_back(new Bt2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LT2: {
                bc->push_back(new Lt2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BEQ2: {
                bc->push_back(new Beq2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LEQ2: {
                bc->push_back(new Leq2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::IN2: {
                bc->push_back(new In2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::AND2: {
                bc->push_back(new And2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::OR2: {
                bc->push_back(new Or2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::XOR2: {
                bc->push_back(new Xor2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUBSC2: {
                bc->push_back(new Subsc2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SLICE2: {
                bc->push_back(new Slice2(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CONCAT3: {
                bc->push_back(new Concat3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EXP3: {
                bc->push_back(new Exp3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::ADD3: {
                bc->push_back(new Add3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUB3: {
                bc->push_back(new Sub3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::DIV3: {
                bc->push_back(new Div3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MUL3: {
                bc->push_back(new Mul3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::MOD3: {
                bc->push_back(new Mod3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::EQ3: {
                bc->push_back(new Eq3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::NEQ3: {
                bc->push_back(new Neq3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BT3: {
                bc->push_back(new Bt3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LT3: {
                bc->push_back(new Lt3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::BEQ3: {
                bc->push_back(new Beq3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::LEQ3: {
                bc->push_back(new Leq3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::IN3: {
                bc->push_back(new In3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::AND3: {
                bc->push_back(new And3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::OR3: {
                bc->push_back(new Or3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::XOR3: {
                bc->push_back(new Xor3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SUBSC3: {
                bc->push_back(new Subsc3(read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::NOT: {
                bc->push_back(new Not(read_register(), read_register()));
            } break;
            case opcode::OpCodes::NEG: {
                bc->push_back(new Neg(read_register(), read_register()));
            } break;
            case opcode::OpCodes::ASSERT: {
                bc->push_back(new Assert(read_register(), read_register()));
            } break;
            case opcode::OpCodes::COPY_ARGS: {
                bc->push_back(new CopyArgs());
            } break;
            case opcode::OpCodes::RAISE: {
                bc->push_back(new Raise(read_register()));
            } break;
            case opcode::OpCodes::CHECK_CATCH: {
                bc->push_back(new CheckCatch(read_register(), read_register()));
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
                bc->push_back(new BuildDict(read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE: {
                bc->push_back(new CreateRange(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE2: {
                bc->push_back(new CreateRange2(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE3: {
                bc->push_back(new CreateRange3(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE4: {
                bc->push_back(new CreateRange4(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE5: {
                bc->push_back(new CreateRange5(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE6: {
                bc->push_back(new CreateRange6(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE7: {
                bc->push_back(new CreateRange7(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::CREATE_RANGE8: {
                bc->push_back(new CreateRange8(read_register(), read_register(), read_register(), read_register()));
            } break;
            case opcode::OpCodes::SWITCH: {
                bc->push_back(new Switch(read_register(), read_register(), read_address()));
            } break;
            case opcode::OpCodes::FOR: {
                bc->push_back(new For(read_register(), read_register()));
            } break;
            default: 
                std::string msg = "unknown opcode in bytecode reader: "+std::to_string(opcode);
                error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
        }
    } while(!this->stream->eof());

    LOG1("Bytecode read and converted:\n" << *bc);
    return bc;
}