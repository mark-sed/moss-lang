/// 
/// \file mslib_string.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss String standard library methods
/// 
/// This contains internal implementations of std String methods.
/// 

#ifndef _MSLIB_STRING_HPP_
#define _MSLIB_STRING_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// Moss String function space
namespace String {

Value *String_constructor(Interpreter *vm, Value *v, Value *&err);
Value *capitalize(Interpreter *vm, Value * ths, Value *&err);
Value *upper(Interpreter *vm, Value * ths, Value *&err);
Value *lower(Interpreter *vm, Value * ths, Value *&err);
Value *replace(Interpreter *vm, Value *ths, Value *target, Value *value, Value *count, Value *&err);
Value *multi_replace(Interpreter *vm, Value *ths, Value *mappings, Value *&err);
//Value *join(Interpreter *vm, Value * ths, Value *iterable, Value *&err);

}
}
}


#endif//_MSLIB_STRING_HPP_