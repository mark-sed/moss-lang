/// 
/// \file cffi.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss File standard library methods
/// 
/// This contains internal implementations of cffi.ms module
/// 

#ifndef _CFFI_HPP_
#define _CFFI_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of cffi module in mslib.
namespace cffi {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

Value *dlopen(Interpreter *vm, CallFrame *cf, Value *path, Value *&err);

}
}
}


#endif//_SUBPROCESS_HPP_