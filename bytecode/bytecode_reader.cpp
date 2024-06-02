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

StringVal BytecodeReader::read_string() {
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
    return StringVal(str_buffer, str_len);
}

IntConst BytecodeReader::read_const_int() {
    char buffer[BC_INT_SIZE];
    char *buffer_ptr = &buffer[0];
    this->stream->read(buffer_ptr, BC_INT_SIZE);
    return *(IntConst *)&buffer[0];
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
            case opcode::OpCodes::END: {} break;
            case opcode::OpCodes::LOAD: {
                bc->push_back(new Load(read_register(), read_string()));
            } break;
            case opcode::OpCodes::LOAD_ATTR: {} break;
            case opcode::OpCodes::LOAD_GLOBAL: {} break;
            case opcode::OpCodes::LOAD_NONLOC: {} break;
            case opcode::OpCodes::STORE_NAME: {
                bc->push_back(new StoreName(read_register(), read_string()));
            } break;
            case opcode::OpCodes::STORE: {} break;
            case opcode::OpCodes::STORE_CONST: {
                bc->push_back(new StoreConst(read_register(), read_register()));
            } break;
            case opcode::OpCodes::STORE_ADDR: {} break;
            case opcode::OpCodes::STORE_ATTR: {} break;
            case opcode::OpCodes::STORE_ADDR_ATTR: {} break;
            case opcode::OpCodes::STORE_CONST_ATTR: {} break;
            case opcode::OpCodes::STORE_INT_CONST: {
                bc->push_back(new StoreIntConst(read_register(), read_const_int()));
            } break;
            case opcode::OpCodes::STORE_FLOAT_CONST: {} break;
            case opcode::OpCodes::STORE_STR_CONST: {} break;
            case opcode::OpCodes::JMP: {} break;
            case opcode::OpCodes::JMP_IF_TRUE: {} break;
            case opcode::OpCodes::JMP_IF_FALSE: {} break;
            case opcode::OpCodes::CALL: {} break;
            case opcode::OpCodes::RETURN: {} break;
            case opcode::OpCodes::RETURN_CONST: {} break;
            case opcode::OpCodes::RETURN_ADDR: {} break;
            case opcode::OpCodes::PUSH_ARG: {} break;
            case opcode::OpCodes::PUSH_CONST_ARG: {} break;
            case opcode::OpCodes::PUSH_ADDR_ARG: {} break;
            case opcode::OpCodes::IMPORT: {} break;
            case opcode::OpCodes::IMPORT_ALL: {} break;
            case opcode::OpCodes::PUSH_PARENT: {} break;
            case opcode::OpCodes::CREATE_OBJ: {} break;
            case opcode::OpCodes::PROMOTE_OBJ: {} break;
            case opcode::OpCodes::BUILD_CLASS: {} break;
            case opcode::OpCodes::COPY: {} break;
            case opcode::OpCodes::DEEP_COPY: {} break;
            case opcode::OpCodes::CREATE_ANNT: {} break;
            case opcode::OpCodes::ANNOTATE: {} break;
            case opcode::OpCodes::OUTPUT: {} break;
            case opcode::OpCodes::CONCAT: {} break;
            case opcode::OpCodes::EXP: {} break;
            case opcode::OpCodes::ADD: {} break;
            case opcode::OpCodes::SUB: {} break;
            case opcode::OpCodes::DIV: {} break;
            case opcode::OpCodes::MUL: {} break;
            case opcode::OpCodes::MOD: {} break;
            case opcode::OpCodes::EQ: {} break;
            case opcode::OpCodes::NEQ: {} break;
            case opcode::OpCodes::BT: {} break;
            case opcode::OpCodes::LT: {} break;
            case opcode::OpCodes::BEQ: {} break;
            case opcode::OpCodes::LEQ: {} break;
            case opcode::OpCodes::IN: {} break;
            case opcode::OpCodes::AND: {} break;
            case opcode::OpCodes::OR: {} break;
            case opcode::OpCodes::NOT: {} break;
            case opcode::OpCodes::XOR: {} break;
            case opcode::OpCodes::SC_AND: {} break;
            case opcode::OpCodes::SC_OR: {} break;
            case opcode::OpCodes::SUBSC: {} break;
            case opcode::OpCodes::SLICE: {} break;
            case opcode::OpCodes::CONCAT2: {} break;
            case opcode::OpCodes::EXP2: {} break;
            case opcode::OpCodes::ADD2: {} break;
            case opcode::OpCodes::SUB2: {} break;
            case opcode::OpCodes::DIV2: {} break;
            case opcode::OpCodes::MUL2: {} break;
            case opcode::OpCodes::MOD2: {} break;
            case opcode::OpCodes::EQ2: {} break;
            case opcode::OpCodes::NEQ2: {} break;
            case opcode::OpCodes::BT2: {} break;
            case opcode::OpCodes::LT2: {} break;
            case opcode::OpCodes::BEQ2: {} break;
            case opcode::OpCodes::LEQ2: {} break;
            case opcode::OpCodes::IN2: {} break;
            case opcode::OpCodes::AND2: {} break;
            case opcode::OpCodes::OR2: {} break;
            case opcode::OpCodes::XOR2: {} break;
            case opcode::OpCodes::SC_AND2: {} break;
            case opcode::OpCodes::SC_OR2: {} break;
            case opcode::OpCodes::SUBSC2: {} break;
            case opcode::OpCodes::SLICE2: {} break;
            case opcode::OpCodes::CONCAT3: {} break;
            case opcode::OpCodes::EXP3: {} break;
            case opcode::OpCodes::ADD3: {} break;
            case opcode::OpCodes::SUB3: {} break;
            case opcode::OpCodes::DIV3: {} break;
            case opcode::OpCodes::MUL3: {} break;
            case opcode::OpCodes::MOD3: {} break;
            case opcode::OpCodes::EQ3: {} break;
            case opcode::OpCodes::NEQ3: {} break;
            case opcode::OpCodes::BT3: {} break;
            case opcode::OpCodes::LT3: {} break;
            case opcode::OpCodes::BEQ3: {} break;
            case opcode::OpCodes::LEQ3: {} break;
            case opcode::OpCodes::IN3: {} break;
            case opcode::OpCodes::AND3: {} break;
            case opcode::OpCodes::OR3: {} break;
            case opcode::OpCodes::XOR3: {} break;
            case opcode::OpCodes::SC_AND3: {} break;
            case opcode::OpCodes::SC_OR3: {} break;
            case opcode::OpCodes::SUBSC3: {} break;
            case opcode::OpCodes::ASSERT: {} break;
            case opcode::OpCodes::COPY_ARGS: {} break;
            case opcode::OpCodes::RAISE: {} break;
            case opcode::OpCodes::CHECK_CATCH: {} break;
            case opcode::OpCodes::LIST_PUSH: {} break;
            case opcode::OpCodes::LIST_PUSH_CONST: {} break;
            case opcode::OpCodes::LIST_PUSH_ADDR: {} break;
            case opcode::OpCodes::BUILD_LIST: {} break;
            case opcode::OpCodes::BUILD_DICT: {} break;
            case opcode::OpCodes::CREATE_RANGE: {} break;
            case opcode::OpCodes::CREATE_RANGE2: {} break;
            case opcode::OpCodes::CREATE_RANGE3: {} break;
            case opcode::OpCodes::CREATE_RANGE4: {} break;
            case opcode::OpCodes::CREATE_RANGE5: {} break;
            case opcode::OpCodes::CREATE_RANGE6: {} break;
            case opcode::OpCodes::CREATE_RANGE7: {} break;
            case opcode::OpCodes::CREATE_RANGE8: {} break;
            case opcode::OpCodes::SWITCH: {} break;
            case opcode::OpCodes::FOR: {} break;
            default: error::error(error::ErrorCode::BYTECODE, "unknown opcode", &this->file, true);
        }
    } while(!this->stream->eof());

    LOG1("Bytecode read and converted:\n" << *bc);
    return bc;
}