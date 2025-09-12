/// 
/// \file inspect.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss File standard library methods
/// 
/// This contains internal implementations of inspect.ms module
/// 

#ifndef _INSPECT_HPP_
#define _INSPECT_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of inspect module in mslib.
namespace inspect {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

}
}
}


#endif//_INSPECT_HPP_