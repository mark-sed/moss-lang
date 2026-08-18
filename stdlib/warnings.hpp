/// 
/// \file warnings.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2026 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// This contains internal implementations of warnings.ms module
/// 

#ifndef _WARNINGS_HPP_
#define _WARNINGS_HPP_

#include "commons.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of warnings module in mslib.
namespace warnings {

const std::unordered_map<std::string, mslib::mslib_dispatcher>& get_registry();

}
}
}


#endif//_WARNINGS_HPP_