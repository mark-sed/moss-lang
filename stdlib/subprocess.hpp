/// 
/// \file subprocess.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss File standard library methods
/// 
/// This contains internal implementations of subprocess.ms module
/// 

#ifndef _SUBPROCESS_HPP_
#define _SUBPROCESS_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of subprocess module in mslib.
namespace subprocess {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

Value *system(Interpreter *vm, Value *cmd, Value *&err);
Value *run(Interpreter *vm, CallFrame *cf, Value *command, Value *comb_str, Value *capture_out, Value *&err);

}
}
}


#endif//_SUBPROCESS_HPP_