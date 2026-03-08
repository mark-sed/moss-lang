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
Value *capitalize(Value * ths);
Value *upper(Value * ths);
Value *lower(Value * ths);
Value *replace(Value *ths, Value *target, Value *value, Value *count);
Value *multi_replace(Interpreter *vm, Value *ths, Value *mappings, Value *&err);
Value *split_lines(Value *ths, Value *keep_ends);
Value *split(Value *ths, Value *sep, Value *max_split);
Value *rsplit(Value *ths, Value *sep, Value *max_split);
Value *index(Value *ths, Value *value);
Value *rindex(Value *ths, Value *value);
Value *isfun(Value *ths, std::function<bool(std::wint_t)> fn);
Value *swapcase(StringValue *ths);
Value *count(Value *ths, Value *sub);
//Value *join(Value * ths, Value *iterable);

}
}
}


#endif//_MSLIB_STRING_HPP_