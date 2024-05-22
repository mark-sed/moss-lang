#include "bytecode_reader.hpp"
#include "errors.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"

using namespace moss;

Bytecode *BytecodeReader::read() {
    size_t buffer_size = 256;
    unsigned char *buffer = new unsigned char[buffer_size];
    char opcode_char;
    opcode_t opcode;

    // TODO first read bytecode header with version and such

    do {
        this->stream->read(&opcode_char, BC_OPCODE_SIZE);
        opcode = static_cast<opcode_t>(opcode_char);

        switch (opcode) {
        case Bytecode::OpCodes::END: {} break;
        case Bytecode::OpCodes::LOAD: {} break;
        case Bytecode::OpCodes::LOAD_ATTR: {} break;
        case Bytecode::OpCodes::LOAD_GLOBAL: {} break;
        case Bytecode::OpCodes::LOAD_NONLOC: {} break;
        case Bytecode::OpCodes::STORE_NAME: {} break;
        case Bytecode::OpCodes::ALIAS: {} break;
        case Bytecode::OpCodes::STORE: {} break;
        case Bytecode::OpCodes::STORE_CONST: {} break;
        case Bytecode::OpCodes::STORE_ADDR: {} break;
        case Bytecode::OpCodes::STORE_ATTR: {} break;
        case Bytecode::OpCodes::STORE_ADDR_ATTR: {} break;
        case Bytecode::OpCodes::STORE_CONST_ATTR: {} break;
        case Bytecode::OpCodes::STORE_INT_CONST: {} break;
        case Bytecode::OpCodes::STORE_FLOAT_CONST: {} break;
        case Bytecode::OpCodes::STORE_STR_CONST: {} break;
        case Bytecode::OpCodes::JMP: {} break;
        case Bytecode::OpCodes::JMP_IF_TRUE: {} break;
        case Bytecode::OpCodes::JMP_IF_FALSE: {} break;
        case Bytecode::OpCodes::CALL: {} break;
        case Bytecode::OpCodes::RETURN: {} break;
        case Bytecode::OpCodes::RETURN_CONST: {} break;
        case Bytecode::OpCodes::RETURN_ADDR: {} break;
        case Bytecode::OpCodes::PUSH_ARG: {} break;
        case Bytecode::OpCodes::PUSH_CONST_ARG: {} break;
        case Bytecode::OpCodes::PUSH_ADDR_ARG: {} break;
        case Bytecode::OpCodes::IMPORT: {} break;
        case Bytecode::OpCodes::IMPORT_ALL: {} break;
        case Bytecode::OpCodes::PUSH_PARENT: {} break;
        case Bytecode::OpCodes::CREATE_OBJ: {} break;
        case Bytecode::OpCodes::PROMOTE_OBJ: {} break;
        case Bytecode::OpCodes::BUILD_CLASS: {} break;
        case Bytecode::OpCodes::COPY: {} break;
        case Bytecode::OpCodes::DEEP_COPY: {} break;
        case Bytecode::OpCodes::CREATE_ANNT: {} break;
        case Bytecode::OpCodes::ANNOTATE: {} break;
        case Bytecode::OpCodes::OUTPUT: {} break;
        case Bytecode::OpCodes::CONCAT: {} break;
        case Bytecode::OpCodes::EXP: {} break;
        case Bytecode::OpCodes::ADD: {} break;
        case Bytecode::OpCodes::SUB: {} break;
        case Bytecode::OpCodes::DIV: {} break;
        case Bytecode::OpCodes::MUL: {} break;
        case Bytecode::OpCodes::MOD: {} break;
        case Bytecode::OpCodes::EQ: {} break;
        case Bytecode::OpCodes::NEQ: {} break;
        case Bytecode::OpCodes::BT: {} break;
        case Bytecode::OpCodes::LT: {} break;
        case Bytecode::OpCodes::BEQ: {} break;
        case Bytecode::OpCodes::LEQ: {} break;
        case Bytecode::OpCodes::IN: {} break;
        case Bytecode::OpCodes::AND: {} break;
        case Bytecode::OpCodes::OR: {} break;
        case Bytecode::OpCodes::NOT: {} break;
        case Bytecode::OpCodes::XOR: {} break;
        case Bytecode::OpCodes::SC_AND: {} break;
        case Bytecode::OpCodes::SC_OR: {} break;
        case Bytecode::OpCodes::SUBSC: {} break;
        case Bytecode::OpCodes::SLICE: {} break;
        case Bytecode::OpCodes::CONCAT2: {} break;
        case Bytecode::OpCodes::EXP2: {} break;
        case Bytecode::OpCodes::ADD2: {} break;
        case Bytecode::OpCodes::SUB2: {} break;
        case Bytecode::OpCodes::DIV2: {} break;
        case Bytecode::OpCodes::MUL2: {} break;
        case Bytecode::OpCodes::MOD2: {} break;
        case Bytecode::OpCodes::EQ2: {} break;
        case Bytecode::OpCodes::NEQ2: {} break;
        case Bytecode::OpCodes::BT2: {} break;
        case Bytecode::OpCodes::LT2: {} break;
        case Bytecode::OpCodes::BEQ2: {} break;
        case Bytecode::OpCodes::LEQ2: {} break;
        case Bytecode::OpCodes::IN2: {} break;
        case Bytecode::OpCodes::AND2: {} break;
        case Bytecode::OpCodes::OR2: {} break;
        case Bytecode::OpCodes::XOR2: {} break;
        case Bytecode::OpCodes::SC_AND2: {} break;
        case Bytecode::OpCodes::SC_OR2: {} break;
        case Bytecode::OpCodes::SUBSC2: {} break;
        case Bytecode::OpCodes::SLICE2: {} break;
        case Bytecode::OpCodes::CONCAT3: {} break;
        case Bytecode::OpCodes::EXP3: {} break;
        case Bytecode::OpCodes::ADD3: {} break;
        case Bytecode::OpCodes::SUB3: {} break;
        case Bytecode::OpCodes::DIV3: {} break;
        case Bytecode::OpCodes::MUL3: {} break;
        case Bytecode::OpCodes::MOD3: {} break;
        case Bytecode::OpCodes::EQ3: {} break;
        case Bytecode::OpCodes::NEQ3: {} break;
        case Bytecode::OpCodes::BT3: {} break;
        case Bytecode::OpCodes::LT3: {} break;
        case Bytecode::OpCodes::BEQ3: {} break;
        case Bytecode::OpCodes::LEQ3: {} break;
        case Bytecode::OpCodes::IN3: {} break;
        case Bytecode::OpCodes::AND3: {} break;
        case Bytecode::OpCodes::OR3: {} break;
        case Bytecode::OpCodes::XOR3: {} break;
        case Bytecode::OpCodes::SC_AND3: {} break;
        case Bytecode::OpCodes::SC_OR3: {} break;
        case Bytecode::OpCodes::SUBSC3: {} break;
        case Bytecode::OpCodes::ASSERT: {} break;
        case Bytecode::OpCodes::COPY_ARGS: {} break;
        case Bytecode::OpCodes::RAISE: {} break;
        case Bytecode::OpCodes::CHECK_CATCH: {} break;
        case Bytecode::OpCodes::LIST_PUSH: {} break;
        case Bytecode::OpCodes::LIST_PUSH_CONST: {} break;
        case Bytecode::OpCodes::LIST_PUSH_ADDR: {} break;
        case Bytecode::OpCodes::BUILD_LIST: {} break;
        case Bytecode::OpCodes::BUILD_DICT: {} break;
        case Bytecode::OpCodes::CREATE_RANGE: {} break;
        case Bytecode::OpCodes::CREATE_RANGE2: {} break;
        case Bytecode::OpCodes::CREATE_RANGE3: {} break;
        case Bytecode::OpCodes::CREATE_RANGE4: {} break;
        case Bytecode::OpCodes::CREATE_RANGE5: {} break;
        case Bytecode::OpCodes::CREATE_RANGE6: {} break;
        case Bytecode::OpCodes::CREATE_RANGE7: {} break;
        case Bytecode::OpCodes::CREATE_RANGE8: {} break;
        case Bytecode::OpCodes::SWITCH: {} break;
        case Bytecode::OpCodes::FOR: {} break;
        default: error::error(error::ErrorCode::BYTECODE, "unknown opcode", &this->file, true);
        }
    } while(!this->stream->eof());

    delete[] buffer;
    return nullptr;
}