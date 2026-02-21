#include "bytecode_reader.hpp"
#include "errors.hpp"
#include "bytecode.hpp"
#include "opcode.hpp"
#include "logging.hpp"
#include "errors.hpp"
#include <cstdlib>

using namespace moss;
using namespace moss::opcode;

void BytecodeReader::read_raw(char* data, std::size_t size) {
    this->stream->read(data, size);

    if (this->stream->gcount() == size) {
        for (size_t i = 0; i < size; ++i) {
            crc_checksum ^= static_cast<unsigned char>(data[i]);
            for (int j = 0; j < 8; ++j)
                crc_checksum = (crc_checksum >> 1) ^ (0xEDB88320 & -(crc_checksum & 1));
        }
    }

    if (!this->stream && !this->stream->eof()) {
        error::error(error::ErrorCode::BYTECODE, "Error reading bytecode", &this->file, true);
    }
}

Register BytecodeReader::read_register() {
    char buffer[BC_REGISTER_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_REGISTER_SIZE);
    Register reg;
    std::memcpy(&reg, buffer, sizeof(reg));
    return reg;
}

StringConst BytecodeReader::read_string() {
    char buffer[BC_STR_LEN_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_STR_LEN_SIZE);
    strlen_t str_len;
    std::memcpy(&str_len, buffer, sizeof(str_len));

    if (str_len > buffer_size) {
        // Change buffer to size of nearest higher multiple of 64
        buffer_size = ((str_len - 1) | 63) + 1;
        this->str_buffer = (char *)std::realloc(str_buffer, buffer_size);
    }

    read_raw(str_buffer, str_len);
    return StringConst(str_buffer, str_len);
}

IntConst BytecodeReader::read_const_int() {
    char buffer[BC_INT_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_INT_SIZE);
    IntConst c;
    std::memcpy(&c, buffer, sizeof(c));
    return c;
}

FloatConst BytecodeReader::read_const_float() {
    char buffer[BC_FLOAT_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_FLOAT_SIZE);
    FloatConst c;
    std::memcpy(&c, buffer, sizeof(c));
    return c;
}

BoolConst BytecodeReader::read_const_bool() {
    char buffer[BC_BOOL_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_BOOL_SIZE);
    BoolConst c;
    std::memcpy(&c, buffer, sizeof(c));
    return c;
}

Address BytecodeReader::read_address() {
    char buffer[BC_ADDR_SIZE];
    char *buffer_ptr = &buffer[0];
    read_raw(buffer_ptr, BC_ADDR_SIZE);
    Address addr;
    std::memcpy(&addr, buffer, sizeof(addr));
    return addr;
}

bc_header::BytecodeHeader BytecodeReader::read_header() {
    bc_header::BytecodeHeader header{};
    this->stream->read(reinterpret_cast<char*>(&header), sizeof(header));

    // Check read success (size).
    if (this->stream->gcount() != sizeof(header)) {
        std::string msg = "Invalid moss bytecode file — file size cannot fit header";
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
    }

    // Validate ID.
    constexpr std::uint32_t EXPECTED_ID = 0xFF00002A;
    if (header.id != EXPECTED_ID) {
        std::string msg = "Invalid moss bytecode file — moss ID not matched";
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
    }

    // Check BC version compatibility.
    if (header.bc_version != bc_header::BYTECODE_VERSION) {
        std::string msg = "Incompatible moss bytecode version — interpreter uses version " +
            std::to_string(bc_header::BYTECODE_VERSION) + ", but file uses version " +
            std::to_string(header.bc_version);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
    }

    // Check Moss version and warn if bc was compiled with newer version.
    if (header.moss_version > MOSS_VERSION_UINT32) {
        auto version_string = header.get_version_string();
        error::warning(diags::Diagnostic(true, file, diags::WarningID::MSB_COMPILED_WITH_NEWER_VER, version_string.c_str()));
    }

    return header;
}

Bytecode *BytecodeReader::read() {
    LOG1("Reading bytecode from file " << this->file.get_name());

    Bytecode *bc = new Bytecode();

    bc_header::BytecodeHeader header_val = read_header();
    bc_header::BytecodeHeader *header = new bc_header::BytecodeHeader(header_val);
    LOGMAX("Read header: " << *header);
    bc->set_header(header);

    char opcode_char;
    opcode_t opcode;

    do {
        read_raw(&opcode_char, BC_OPCODE_SIZE);
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
            case opcode::OpCodes::STORE_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new StoreAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::STORE_CONST_ATTR: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto str = read_string();
                bc->push_back(new StoreConstAttr(reg1, reg2, str));
            } break;
            case opcode::OpCodes::STORE_GLOBAL: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new StoreGlobal(reg, str));
            } break;
            case opcode::OpCodes::STORE_NONLOC: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new StoreNonLoc(reg, str));
            } break;
            case opcode::OpCodes::STORE_SUBSC: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new StoreSubsc(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::STORE_CONST_SUBSC: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new StoreConstSubsc(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::STORE_SUBSC_CONST: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new StoreSubscConst(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::STORE_C_SUBSC_C: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new StoreConstSubscConst(reg1, reg2, reg3));
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
            case opcode::OpCodes::BREAK_TO: {
                bc->push_back(new BreakTo(read_address()));
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
            case opcode::OpCodes::CALL_FORMATTER: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new CallFormatter(reg, str));
            } break;
            //case opcode::OpCodes::PUSH_FRAME: {
            //    bc->push_back(new PushFrame());
            //} break;
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
            case opcode::OpCodes::PUSH_ARG: {
                bc->push_back(new PushArg(read_register()));
            } break;
            case opcode::OpCodes::PUSH_CONST_ARG: {
                bc->push_back(new PushConstArg(read_register()));
            } break;
            case opcode::OpCodes::PUSH_NAMED_ARG: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new PushNamedArg(reg, str));
            } break;
            case opcode::OpCodes::PUSH_UNPACKED: {
                bc->push_back(new PushUnpacked(read_register()));
            } break;
            case opcode::OpCodes::CREATE_FUN: {
                auto reg = read_register();
                auto str1 = read_string();
                auto str2 = read_string();
                bc->push_back(new CreateFun(reg, str1, str2));
            } break;
            case opcode::OpCodes::FUN_BEGIN: {
                auto reg = read_register();
                bc->push_back(new FunBegin(reg));
            } break;
            case opcode::OpCodes::SET_DEFAULT: {
                auto reg1 = read_register();
                auto index = read_const_int();
                auto reg2 = read_register();
                bc->push_back(new SetDefault(reg1, index, reg2));
            } break;
            case opcode::OpCodes::SET_DEFAULT_CONST: {
                auto reg1 = read_register();
                auto index = read_const_int();
                auto reg2 = read_register();
                bc->push_back(new SetDefaultConst(reg1, index, reg2));
            } break;
            case opcode::OpCodes::SET_TYPE: {
                auto reg1 = read_register();
                auto index = read_const_int();
                auto reg2 = read_register();
                bc->push_back(new SetType(reg1, index, reg2));
            } break;
            case opcode::OpCodes::SET_VARARG: {
                auto reg = read_register();
                auto index = read_const_int();
                bc->push_back(new SetVararg(reg, index));
            } break;
            case opcode::OpCodes::IMPORT: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new Import(reg, str));
            } break;
            case opcode::OpCodes::IMPORT_ALL: {
                bc->push_back(new ImportAll(read_register()));
            } break;
            case opcode::OpCodes::PUSH_PARENT: {
                bc->push_back(new PushParent(read_register()));
            } break;
            case opcode::OpCodes::BUILD_CLASS: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new BuildClass(reg, str));
            } break;
            case opcode::OpCodes::ANNOTATE: {
                auto reg1 = read_register();
                auto str = read_string();
                auto reg2 = read_register();
                bc->push_back(new Annotate(reg1, str, reg2));
            } break;
            case opcode::OpCodes::ANNOTATE_MOD: {
                auto str = read_string();
                auto reg = read_register();
                bc->push_back(new AnnotateMod(str, reg));
            } break;
            case opcode::OpCodes::DOCUMENT: {
                auto reg = read_register();
                auto str = read_string();
                bc->push_back(new Document(reg, str));
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
            case opcode::OpCodes::SUBSCLAST: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new SubscLast(reg1, reg2, reg3));
            } break;
            case opcode::OpCodes::SUBSCREST: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto reg3 = read_register();
                bc->push_back(new SubscRest(reg1, reg2, reg3));
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
            case opcode::OpCodes::RAISE: {
                bc->push_back(new Raise(read_register()));
            } break;
            case opcode::OpCodes::CATCH: {
                auto name = read_string();
                auto addr = read_address();
                bc->push_back(new Catch(name, addr));
            } break;
            case opcode::OpCodes::CATCH_TYPED: {
                auto name = read_string();
                auto type = read_register();
                auto addr = read_address();
                bc->push_back(new CatchTyped(name, type, addr));
            } break;
            case opcode::OpCodes::POP_CATCH: {
                auto val = read_const_int();
                bc->push_back(new PopCatch(val));
            } break;
            case opcode::OpCodes::FINALLY: {
                auto addr = read_address();
                auto reg = read_register();
                bc->push_back(new Finally(addr, reg));
            } break;
            case opcode::OpCodes::POP_FINALLY: {
                bc->push_back(new PopFinally());
            } break;
            case opcode::OpCodes::FINALLY_RETURN: {
                auto reg = read_register();
                bc->push_back(new FinallyReturn(reg));
            } break;
            case opcode::OpCodes::LIST_PUSH: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new ListPush(reg1, reg2));
            } break;
            case opcode::OpCodes::LIST_PUSH_CONST: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new ListPushConst(reg1, reg2));
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
                auto str = read_string();
                bc->push_back(new BuildEnum(reg1, reg2, str));
            } break;
            case opcode::OpCodes::BUILD_SPACE: {
                auto reg1 = read_register();
                auto str = read_string();
                // Anonymous spaces have name starting with number.
                bool anonymous = !str.empty() && std::isdigit(str[0]);
                bc->push_back(new BuildSpace(reg1, str, anonymous));
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
                auto addr = read_address();
                bc->push_back(new For(reg1, reg2, addr));
            } break;
            case opcode::OpCodes::FOR_MULTI: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                auto addr = read_address();
                auto reg3 = read_register();
                bc->push_back(new ForMulti(reg1, reg2, addr, reg3));
            } break;
            case opcode::OpCodes::ITER: {
                auto reg1 = read_register();
                auto reg2 = read_register();
                bc->push_back(new Iter(reg1, reg2));
            } break;
            case opcode::OpCodes::LOOP_BEGIN: {
                bc->push_back(new LoopBegin());
            } break;
            case opcode::OpCodes::LOOP_END: {
                bc->push_back(new LoopEnd());
            } break;
            default: {
                std::string msg = "unknown opcode in bytecode reader: "+std::to_string(opcode);
                error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
            }
        }
    } while(!this->stream->eof());

    std::uint32_t computed = ~crc_checksum;
    std::uint32_t expected = header->checksum;

    if (computed != expected) {
        std::string msg = "Moss bytecode checksum mismatch (corrupted file) " + std::to_string(computed) + " != " + std::to_string(expected);
        error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
    }

    return bc;
}