/// 
/// \file sys.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///
/// This contains internal implementations of sys.ms module
/// 

#ifndef _SYS_HPP_
#define _SYS_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of sys module in mslib.
namespace sys {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

void init_constants(Interpreter *vm);

Value *platform(Interpreter *vm, CallFrame *cf, Value *&err);

}
}
}

#endif//_SYS_HPP_