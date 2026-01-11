/// 
/// \file math.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
///
/// This contains internal implementations of math.ms module
/// 

#ifndef _MATH_HPP_
#define _MATH_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of re module in mslib.
namespace math {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

}
}
}

#endif//_RE_HPP_