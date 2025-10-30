/// 
/// \file python.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///
/// This contains internal implementations of sys.ms module
/// 

#ifndef _PYTHON_HPP_
#define _PYTHON_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of python module in mslib.
namespace python {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

void init_constants(Interpreter *vm);

}
}
}

#endif//_PYTHON_HPP_