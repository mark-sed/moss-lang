#include "bytecode_writer.hpp"
#include "logging.hpp"
#include "bytecode.hpp"
#include "os_interface.hpp"
#include "opcode.hpp"
#include "errors.hpp"
#include <cassert>
#include <iterator>

using namespace moss;
using namespace moss::opcode;

void BytecodeWriter::write_register(Register reg) {
    this->stream->write(reinterpret_cast<char *>(&reg), BC_REGISTER_SIZE);
}

void BytecodeWriter::write_string(StringVal val) {
    const char *txt = val.c_str();
    strlen_t len = val.size();
    this->stream->write(reinterpret_cast<char *>(&len), BC_STR_LEN_SIZE);
    this->stream->write(txt, val.size());
}

void BytecodeWriter::write(Bytecode *code) {
    LOGMAX("Writing bytecode to file: " << this->file.get_path());
    assert(code && "Bytecode to be written is null");

    for (opcode::OpCode *op_gen: code->get_code()) {
        opcode_t opc = op_gen->get_type();
        this->stream->write(reinterpret_cast<char *>(&opc), BC_OPCODE_SIZE);
        if (isa<opcode::End>(op_gen)){
            // Nothing to do
        }
        else if (auto o = dyn_cast<opcode::Load>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        /*else if (auto o = dyn_cast<opcode::LOAD_ATTR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LOAD_GLOBAL>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LOAD_NONLOC>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }*/
        else if (auto o = dyn_cast<opcode::StoreName>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        /*else if (auto o = dyn_cast<opcode::ALIAS>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::STORE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }*/
        else if (auto o = dyn_cast<opcode::StoreConst>(op_gen)){
            write_register(o->dst);
            write_register(o->csrc);
        }
        /*else if (auto o = dyn_cast<opcode::STORE_ADDR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::STORE_ATTR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::STORE_ADDR_ATTR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::STORE_CONST_ATTR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }*/
        else if (auto o = dyn_cast<opcode::StoreIntConst>(op_gen)){
            write_register(o->dst);
            auto val = o->val;
            this->stream->write(reinterpret_cast<char *>(&val), BC_INT_SIZE);
        }
        /*else if (auto o = dyn_cast<opcode::STORE_FLOAT_CONST>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::STORE_STR_CONST>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::JMP>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::JMP_IF_TRUE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::JMP_IF_FALSE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CALL>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::RETURN>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::RETURN_CONST>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::RETURN_ADDR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::PUSH_ARG>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::PUSH_CONST_ARG>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::PUSH_ADDR_ARG>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::IMPORT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::IMPORT_ALL>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::PUSH_PARENT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_OBJ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::PROMOTE_OBJ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BUILD_CLASS>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::COPY>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::DEEP_COPY>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_ANNT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::ANNOTATE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::OUTPUT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CONCAT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EXP>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::ADD>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUB>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::DIV>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MUL>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MOD>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EQ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::NEQ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BEQ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LEQ>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::IN>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::AND>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::OR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::NOT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::XOR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_AND>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_OR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUBSC>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SLICE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CONCAT2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EXP2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::ADD2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUB2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::DIV2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MUL2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MOD2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EQ2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::NEQ2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BT2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LT2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BEQ2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LEQ2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::IN2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::AND2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::OR2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::XOR2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_AND2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_OR2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUBSC2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SLICE2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CONCAT3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EXP3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::ADD3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUB3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::DIV3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MUL3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::MOD3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::EQ3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::NEQ3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BT3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LT3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BEQ3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LEQ3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::IN3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::AND3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::OR3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::XOR3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_AND3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SC_OR3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SUBSC3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::ASSERT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::COPY_ARGS>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::RAISE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CHECK_CATCH>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LIST_PUSH>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LIST_PUSH_CONST>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::LIST_PUSH_ADDR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BUILD_LIST>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::BUILD_DICT>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE2>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE3>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE4>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE5>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE6>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE7>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::CREATE_RANGE8>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::SWITCH>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }
        else if (auto o = dyn_cast<opcode::FOR>(op_gen)){
            assert(false && "TODO: Unimplemented opcode in writer");
        }*/
        else
            error::error(error::ErrorCode::BYTECODE, "unknown opcode", &this->file, true);
    }

    this->stream->flush();
    LOGMAX("Finished writing bytecode");
}