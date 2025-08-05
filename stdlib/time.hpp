/// 
/// \file time.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// This contains internal implementations of time.ms module
/// 

#ifndef _TIME_HPP_
#define _TIME_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of time module in mslib.
namespace time {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

Value *time(Interpreter *vm, Value *&err);
Value *localtime(Interpreter *vm, CallFrame *cf, Value *secs, Value *&err);
Value *strftime(Interpreter *vm, CallFrame *cf, Value *format, Value *t, Value *&err);

}
}
}


#endif//_TIME_HPP_