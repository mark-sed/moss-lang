/**
 * @file opcode.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Bytecode opcodes
 */

#ifndef _TESTING_UTILS_HPP_
#define _TESTING_UTILS_HPP_

#include <cstdint>
#include "values.hpp"

namespace testing {

int64_t int_val(moss::Value *v);
double float_val(moss::Value *v);
bool bool_val(moss::Value *v);
ustring string_val(moss::Value *v);

}

#endif//_TESTING_UTILS_HPP_