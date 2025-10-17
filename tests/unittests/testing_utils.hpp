/// 
/// \file testing_utils.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Helper functions for unit testing
/// 

#ifndef _TESTING_UTILS_HPP_
#define _TESTING_UTILS_HPP_

#include <cstdint>
#include "values.hpp"

namespace testing {

int64_t int_val(moss::Value *v);
double float_val(moss::Value *v);
bool bool_val(moss::Value *v);
ustring string_val(moss::Value *v);

void check_all_lines_err(std::vector<ustring> lines, ustring test);
void check_all_lines_ok(std::vector<ustring> lines, ustring test);
void check_line_err(ustring code, ustring test);
void check_line_ok(ustring code, ustring test);

template<typename T>
std::vector<T> list2vect(std::list<T> l) {
    std::vector<T> body{ std::begin(l), std::end(l) };
    return body;
}

}

#endif//_TESTING_UTILS_HPP_