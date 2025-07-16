/// 
/// \file moss.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Entry point of the moss interpreter
/// 
/// Moss is a modern interpreted language with simple syntax
/// and allows for note creation inside of the source code,
/// these notes can be then used to generate different
/// outputs, such as pdf files or tables with data computed
/// with moss.
/// 

#ifndef _MOSS_HPP_
#define _MOSS_HPP_

#define MACRO_TO_STR_HELPER(x) #x
/// Macro to convert INT macro value into string value
#define MACRO_TO_STR(x) MACRO_TO_STR_HELPER(x)

#define MOSS_VERSION_MAJOR 0  ///< Interpreter's major version
#define MOSS_VERSION_MINOR 6  ///< Interpreter's minor version
#define MOSS_VERSION_PATCH 5  ///< Interpreter's patch

/// Moss version as a string
#define MOSS_VERSION MACRO_TO_STR(MOSS_VERSION_MAJOR) "." MACRO_TO_STR(MOSS_VERSION_MINOR) "." MACRO_TO_STR(MOSS_VERSION_PATCH)

#endif//_MOSS_HPP_