#include "bytecode_writer.hpp"
#include "bytecode_header.hpp"
#include "logging.hpp"
#include "bytecode.hpp"
#include "commons.hpp"
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

void BytecodeWriter::write_header(bc_header::BytecodeHeader bch) {
    this->stream->write(reinterpret_cast<const char *>(&bch.id), BCH_ID_SIZE);
    this->stream->write(reinterpret_cast<char *>(&bch.checksum), BCH_CHECKSUM_SIZE);
    this->stream->write(reinterpret_cast<char *>(&bch.version), BCH_VERSION_SIZE);
    this->stream->write(reinterpret_cast<char *>(&bch.timestamp), BCH_TIMESTAMP_SIZE);
}

void BytecodeWriter::write(Bytecode *code) {
    LOGMAX("Writing bytecode to file: " << this->file.get_path());
    assert(code && "Bytecode to be written is null");

    bc_header::BytecodeHeader header = bc_header::create_header();
    write_header(header);

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
        else if (auto o = dyn_cast<opcode::StoreGlobal>(op_gen)){
            write_register(o->src);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::StoreNonLoc>(op_gen)){
            write_register(o->src);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::StoreSubsc>(op_gen)){
            write_register(o->src);
            write_register(o->obj);
            write_register(o->key);
        }
        else if (auto o = dyn_cast<opcode::StoreConstSubsc>(op_gen)){
            write_register(o->csrc);
            write_register(o->obj);
            write_register(o->key);
        }
        else if (auto o = dyn_cast<opcode::StoreSubscConst>(op_gen)){
            write_register(o->src);
            write_register(o->obj);
            write_register(o->ckey);
        }
        else if (auto o = dyn_cast<opcode::StoreConstSubscConst>(op_gen)){
            write_register(o->csrc);
            write_register(o->obj);
            write_register(o->ckey);
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
        else if (auto o = dyn_cast<opcode::CallFormatter>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
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
        else if (auto o = dyn_cast<opcode::PushUnpacked>(op_gen)){
            write_register(o->src);
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
            write_register(o->type);
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
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::PushParent>(op_gen)){
            write_register(o->parent);
        }
        else if (auto o = dyn_cast<opcode::BuildClass>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::Annotate>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
            write_register(o->val);
        }
        else if (auto o = dyn_cast<opcode::AnnotateMod>(op_gen)){
            write_string(o->name);
            write_register(o->val);
        }
        else if (auto o = dyn_cast<opcode::Document>(op_gen)){
            write_register(o->dst);
            write_string(o->val);
        }
        else if (auto o = dyn_cast<opcode::Output>(op_gen)){
            write_register(o->src);
        }
        // Binary expressions
        else if (opc >= OpCodes::CONCAT && opc <= OpCodes::SUBSCREST){
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
        else if (auto o = dyn_cast<opcode::Raise>(op_gen)){
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::Catch>(op_gen)){
            write_string(o->name);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::CatchTyped>(op_gen)){
            write_string(o->name);
            write_register(o->type);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::PopCatch>(op_gen)){
            write_int(o->amount);
        }
        else if (auto o = dyn_cast<opcode::Finally>(op_gen)){
            write_address(o->addr);
            write_register(o->caller);
        }
        else if (isa<opcode::PopFinally>(op_gen)){
            // Nothing to do
        }
        else if (auto o = dyn_cast<opcode::FinallyReturn>(op_gen)){
            write_register(o->caller);
        }
        else if (auto o = dyn_cast<opcode::ListPush>(op_gen)){
            write_register(o->dst);
            write_register(o->src);
        }
        else if (auto o = dyn_cast<opcode::ListPushConst>(op_gen)){
            write_register(o->dst);
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
            write_register(o->vals);
            write_string(o->name);
        }
        else if (auto o = dyn_cast<opcode::BuildSpace>(op_gen)){
            write_register(o->dst);
            write_string(o->name);
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
            write_register(o->collection);
            write_address(o->addr);
        }
        else if (auto o = dyn_cast<opcode::ForMulti>(op_gen)){
            write_register(o->vars);
            write_register(o->collection);
            write_address(o->addr);
            write_register(o->unpack);
        }
        else if (auto o = dyn_cast<opcode::Iter>(op_gen)){
            write_register(o->iterator);
            write_register(o->collection);
        }
        else {
            std::string msg = "unknown opcode in bytecode writer: "+std::to_string(opc);
            error::error(error::ErrorCode::BYTECODE, msg.c_str(), &this->file, true);
        }
    }

    this->stream->flush();
    LOGMAX("Finished writing bytecode");
}

void BytecodeWriter::write_textual(Bytecode *code) {
    LOGMAX("Writing textual bytecode to file: " << this->file.get_path());
    assert(code && "Bytecode to be written is null");

    *this->stream << *code;

    LOGMAX("Finished writing textual bytecode");
}