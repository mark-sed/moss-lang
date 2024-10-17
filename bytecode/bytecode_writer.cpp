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

void BytecodeWriter::write_int(IntConst v) {
    this->stream->write(reinterpret_cast<char *>(&v), BC_INT_SIZE);
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
        else if (auto o = dyn_cast<opcode::StoreAttr>(op_gen)){
            write_register(o->src);
            write_register(o->obj);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::StoreConstAttr>(op_gen)){
            write_register(o->csrc);
            write_register(o->obj);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::StoreIntConst>(op_gen)){
            write_register(o->dst);
            write_int(o->val);
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
            write_register(o->src);
        }
        else if (isa<opcode::PushFrame>(op_gen)){
            // Nothing to do
        }
        else if (isa<opcode::PopFrame>(op_gen)){
            // Nothing to do
        }
        else if (isa<opcode::PushCallFrame>(op_gen)){
            // Nothing to do
        }
        else if (isa<opcode::PopCallFrame>(op_gen)){
            // Nothing to do
        }
        else if (auto o = dyn_cast<opcode::Return>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::ReturnConst>(op_gen)){
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::PushArg>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::PushConstArg>(op_gen)){
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::PushNamedArg>(op_gen)){
            write_register(o->src);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::CreateFun>(op_gen)){
            write_register(o->fun);
            write_string(o->name);
            write_string(o->arg_names);
        }
        else if (auto o = dyn_cast<opcode::FunBegin>(op_gen)){
            write_register(o->fun);
        }
        else if (auto o = dyn_cast<opcode::SetDefault>(op_gen)){
            write_register(o->fun);
            write_int(o->index);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::SetDefaultConst>(op_gen)){
            write_register(o->fun);
            write_int(o->index);
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::SetType>(op_gen)){
            write_register(o->fun);
            write_int(o->index);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::SetVararg>(op_gen)){
            write_register(o->fun);
            write_int(o->index);
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
        else if (auto o = dyn_cast<opcode::Neg>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::Assert>(op_gen)){
            write_register(o->src);
            write_register(o->msg);
        }
        else if (isa<opcode::CopyArgs>(op_gen)){
            // Nothing to do
        }
        else if (auto o = dyn_cast<opcode::Raise>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::CheckCatch>(op_gen)){
            write_register(o->dst);
            write_register(o->klass);
        }
        else if (auto o = dyn_cast<opcode::ListPush>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::ListPushConst>(op_gen)){
            write_register(o->csrc);
        }
        else if (auto o = dyn_cast<opcode::BuildList>(op_gen)){
            write_register(o->dst);
        }
        else if (auto o = dyn_cast<opcode::BuildDict>(op_gen)){
            write_register(o->dst);
            write_register(o->keys);
            write_register(o->vals);
        }
        else if (auto o = dyn_cast<opcode::BuildEnum>(op_gen)){
            write_register(o->dst);
            write_register(o->names);
        }
        else if (auto o = dyn_cast<opcode::CreateRange>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange2>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange3>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange4>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange5>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange6>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange7>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::CreateRange8>(op_gen)){
            write_register(o->dst);
            write_register(o->start);
            write_register(o->step);
            write_register(o->end);
        }
        else if (auto o = dyn_cast<opcode::Switch>(op_gen)){
            write_register(o->src);
            write_register(o->vals);
            write_register(o->addrs);
            write_register(o->default_addr);
        }
        else if (auto o = dyn_cast<opcode::For>(op_gen)){
            write_register(o->index);
            write_register(o->iterator);
        }
        else {
            std::string msg = "unknown opcode in bytecode writer: "+std::to_string(opc);
            error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
        }
    }

    this->stream->flush();
    LOGMAX("Finished writing bytecode");
}