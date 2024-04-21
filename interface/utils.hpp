/**
 * @file utils.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief General helper functions
 */

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include "os_interface.hpp"
#include <string>
#include <set>
#include <vector>
#include <algorithm>

namespace utils {

/**
 * Parses csv values
 * @param csv String of csv values
 * @param delim CSV delimiter. Comma by default.
 * @return Vector of these value
 */
std::vector<ustring> split_csv(ustring csv, char delim=','); 
std::set<ustring> split_csv_set(ustring csv, char delim=',');

/**
 * Sanitizes text by removing escape characters with their written out form
 * @param text Text to sanitize
 */ 
ustring sanitize(const ustring &text);

/** Trim whitespace from the left of the string */
inline void ltrim(ustring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

/** Trim whitespace from the right of the string */
inline void rtrim(ustring &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void trim(ustring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

}

#endif//_UTILS_HPP_