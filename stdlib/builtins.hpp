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
class Interpreter;

/// Names of annotations.
/// @note When adding new annotation that name needs to be added to Annotation opcode,
///       so that it does not raise an exception.
namespace annots {
    inline constexpr char INTERNAL[] = "internal";
    inline constexpr char INTERNAL_BIND[] = "internal_bind";
    inline constexpr char CONVERTER[] = "converter";
    inline constexpr char GENERATOR[] = "generator";
    inline constexpr char FORMATTER[] = "formatter";
    inline constexpr char STATIC_METHOD[] = "static_method";
    inline constexpr char IF_MAIN[] = "if_main";

    inline constexpr char ENABLE_CODE_OUTPUT[] = "enable_code_output";
    inline constexpr char DISABLE_CODE_OUTPUT[] = "disable_code_output";
    inline constexpr char INTERNAL_MODULE[] = "internal_module";
};

/// Names of built-in methods or known built-in attributes/variables.
namespace known_names {
    inline constexpr char TO_STRING_METHOD[] = "__String";
    inline constexpr char TO_INT_METHOD[] = "__Int";
    inline constexpr char TO_FLOAT_METHOD[] = "__Float";
    inline constexpr char TO_BOOL_METHOD[] = "__Bool";

    inline constexpr char BUILT_IN_EXT_VALUE[] = "__value";
    inline constexpr char OBJECT_ITERATOR[] = "__iter";
    inline constexpr char ITERATOR_NEXT[] = "__next";
    inline constexpr char DOC_STRING[] = "__doc";
    inline constexpr char HASH_METHOD[] = "__hash";
    inline constexpr char SUBSC_OPERATOR[] = "__setitem";

    inline constexpr char FILE_FSTREAM_ATT[] = "__fstream";
}

/// This namespace contains values (pointers) for all the built-in types
/// \note: When adding a new value also add it to init_built_ins
///        and instantiate it in builtins.cpp
namespace BuiltIns {

    /// Has to be called to store built ins in global frame.
    /// Without calling this all the values will be GCed.
    void init_built_ins(MemoryPool *gf, opcode::Register &reg);

    /// Initializes constant variables in moss stdlib, such as args.
    void init_constant_variables(MemoryPool *gf, Interpreter *vm);

    /// \return interned int or nullptr if it is not interned.
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
    extern Value *Enum;
    
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
    extern Value *OutputError;
    extern Value *SystemExit;

    extern Value *PythonObject;

    extern Value *Nil;
    extern Value *True;
    extern Value *False;

    /// C++ values as moss values
    namespace Cpp {
        extern Value *CppSpace;

        extern Value *CVoid;
        extern Value *CVoidStar;
        extern Value *CLong;
        extern Value *CDouble;
        extern Value *CCharStar;

        extern Value *FStream;
        extern Value *Ffi_cif;
    }
}
}

#endif//_BUILTINS_HPP_