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

void BytecodeWriter::write_address(Address addr) {
    this->stream->write(reinterpret_cast<char *>(&addr), BC_ADDR_SIZE);
}

void BytecodeWriter::write_string(StringConst val) {
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
        else if (auto o = dyn_cast<opcode::LoadAttr>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::LoadGlobal>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::LoadNonLoc>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::Store>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::StoreName>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::StoreConst>(op_gen)){
            write_register(o->dst);
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::StoreAddr>(op_gen)){
            write_register(o->dst);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::StoreAttr>(op_gen)){
            write_register(o->src);
            write_register(o->obj);
            write_string(o->name);
        }
        /*else if (auto o = dyn_cast<opcode::STORE_ADDR_ATTR>(op_gen)){
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
        else if (auto o = dyn_cast<opcode::StoreFloatConst>(op_gen)){
            write_register(o->dst);
            auto val = o->val;
            this->stream->write(reinterpret_cast<char *>(&val), BC_FLOAT_SIZE);
        }
        else if (auto o = dyn_cast<opcode::StoreBoolConst>(op_gen)){
            write_register(o->dst);
            auto val = o->val;
            this->stream->write(reinterpret_cast<char *>(&val), BC_BOOL_SIZE);
        }
        else if (auto o = dyn_cast<opcode::StoreStringConst>(op_gen)){
            write_register(o->dst);
            write_string(o->val);
        }
        else if (auto o = dyn_cast<opcode::StoreNilConst>(op_gen)){
            write_register(o->dst);
        }
        else if (auto o = dyn_cast<opcode::Jmp>(op_gen)){
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::JmpIfTrue>(op_gen)){
            write_register(o->src);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::JmpIfFalse>(op_gen)){
            write_register(o->src);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::Call>(op_gen)){
            write_register(o->dst);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::Return>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::ReturnConst>(op_gen)){
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::ReturnAddr>(op_gen)){
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::PushArg>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::PushConstArg>(op_gen)){
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::PushAddrArg>(op_gen)){
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::Import>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::ImportAll>(op_gen)){
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::PushParent>(op_gen)){
            write_register(o->parent);
        }
        else if (auto o = dyn_cast<opcode::CreateObject>(op_gen)){
            write_register(o->dst);
            write_register(o->cls);
        }
        else if (auto o = dyn_cast<opcode::PromoteObject>(op_gen)){
            write_register(o->src);
            write_register(o->cls);
        }
        else if (auto o = dyn_cast<opcode::BuildClass>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::Copy>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::DeepCopy>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::CreateAnnt>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::Annotate>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::Output>(op_gen)){
            write_register(o->src);
        }
        // Binary expressions
        else if (opc >= OpCodes::CONCAT && opc <= OpCodes::SUBSC3){
            auto binExp = dynamic_cast<opcode::BinExprOpCode *>(op_gen);
            write_register(binExp->dst);
            write_register(binExp->src1);
            write_register(binExp->src2);
        }
        else if (auto o = dyn_cast<opcode::Not>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        /*else if (auto o = dyn_cast<opcode::ASSERT>(op_gen)){
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