/// 
/// \file builtins.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss bytecode interpreter
/// 
/// It connects all the VM parts into one and runs moss bytecode.
/// 

#ifndef _BUILTINS_HPP_
#define _BUILTINS_HPP_

#include "commons.hpp"

namespace moss {

class Value;
class MemoryPool;

/// This namespace contains values (pointers) for all the built-in types
/// \note: When adding a new value also add it to init_built_ins
///        and instantiate it in builtins.cpp
namespace BuiltIns {

    /// Has to be called to store built ins in global frame
    /// Without calling this all the values will be GCed
    void init_built_ins(MemoryPool *gf, opcode::Register &reg);

    void init_constant_variables(MemoryPool *gf);

    /// \return interned int or nullptr if it is not interned
    Value *get_interned_int(opcode::IntConst v);

    extern Value *Type;
    extern Value *Int;
    extern Value *Float;
    extern Value *Bool;
    extern Value *NilType;
    extern Value *String;
    extern Value *Note;
    extern Value *List;
    extern Value *Dict;
    extern Value *Function;
    extern Value *FunctionList;
    extern Value *Function;
    extern Value *Module;
    extern Value *Space;
    
    extern Value *super;
    
    extern Value *Range;
    extern Value *File;

    extern Value *StopIteration;
    extern Value *Exception;
    extern Value *NameError;
    extern Value *AttributeError;
    extern Value *ModuleNotFoundError;
    extern Value *TypeError;
    extern Value *AssertionError;
    extern Value *NotImplementedError;
    extern Value *ParserError;
    extern Value *SyntaxError;
    extern Value *LookupError;
    extern Value *IndexError;
    extern Value *KeyError;
    extern Value *ValueError;
    extern Value *MathError;
    extern Value *DivisionByZeroError;
    extern Value *OSError;
    extern Value *FileNotFoundError;
    extern Value *EOFError;
    extern Value *SystemExit;

    extern Value *Nil;
    extern Value *True;
    extern Value *False;
    #define BUILT_INS_INT_CONSTANTS_AM 262
    extern Value *IntConstants[BUILT_INS_INT_CONSTANTS_AM];

    /// C++ values as moss values
    namespace Cpp {
        extern Value *CppSpace;
        extern Value *FStream;
        extern Value *VoidStar;
    }
}
}

#endif//_BUILTINS_HPP_