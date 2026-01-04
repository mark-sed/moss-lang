/// 
/// \file re.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///
/// This contains internal implementations of re.ms module
/// 

#ifndef _RE_HPP_
#define _RE_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of re module in mslib.
namespace re {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

Value *Pattern(Interpreter *vm, CallFrame *cf, Value *ths, Value *pattern, Value *flags, Value *&err);

Value *match_or_search(bool match, Interpreter *vm, CallFrame *cf, Value *ths, Value *text, Value *&err);

Value *replace(Interpreter *vm, CallFrame *cf, Value *ths, Value *repl, Value *text, Value *count, Value *&err);


}
}
}

#endif//_RE_HPP_