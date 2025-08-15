/// 
/// \file mslib_dict.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss Dict standard library methods
/// 
/// This contains internal implementations of std Dict methods.
/// 

#ifndef _MSLIB_DICT_HPP_
#define _MSLIB_DICT_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// moss Dict function space
namespace Dict {

Value *pop(Interpreter *vm, Value *ths, Value *key, Value *def_val, Value *&err);

Value *Dict(Interpreter *vm, Value *ths, Value *iterable, Value *&err);

}
}
}


#endif//_MSLIB_DICT_HPP_