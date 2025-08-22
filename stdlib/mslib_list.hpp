/// 
/// \file mslib_list.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss List standard library methods
/// 
/// This contains internal implementations of std List methods.
/// 

#ifndef _MSLIB_LIST_HPP_
#define _MSLIB_LIST_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// moss List function space
namespace List {

Value *append(Interpreter *vm, Value * ths, Value *v, Value *&err);

Value *pop(Interpreter *vm, Value *ths, Value *index, Value *&err);

Value *List(Interpreter *vm, Value *iterable, Value *&err);

}
}
}


#endif//_MSLIB_LIST_HPP_