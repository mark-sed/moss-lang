/// 
/// \file utils.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief General helper functions
/// 

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include "commons.hpp"
#include <string>
#include <set>
#include <vector>
#include <memory>
#include <algorithm>

namespace utils {

/// Parses csv values
/// \param csv String of csv values
/// \param delim CSV delimiter. Comma by default.
/// \return Vector of these value 
std::vector<ustring> split_csv(ustring csv, char delim=','); 
std::set<ustring> split_csv_set(ustring csv, char delim=',');

/// Sanitizes text by removing escape characters with their written out form
/// \param text Text to sanitize
ustring sanitize(const ustring &text);

/// Trim whitespace from the left of the string
inline void ltrim(ustring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

/// Trim whitespace from the right of the string
inline void rtrim(ustring &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

/// Trim whitespace from string on both sides
inline void trim(ustring &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

/// Formats message using (s)printf style.
/// \warning this function cannot be called with string as it will mess up the output. Use c_str(). 
template<typename ... Args>
inline ustring formatv(const char *format, Args&& ... args) {
    if(sizeof...(Args) == 0)
        return format;
    int size_s = std::snprintf(nullptr, 0, format, std::forward<Args>(args) ...) + 1;
    if( size_s <= 0 ){
        // TODO: Call error
        assert(false);
        return "[ERROR]";
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[ size ]);
    std::snprintf(buf.get(), size, format, args ...);
    return std::string(buf.get(), buf.get() + size - 1);
}

/// Replaces n or all occurences of target to value in string str.
/// \param count if negative then all occurences are replaces
inline ustring replace_n(ustring str, const ustring& target, const ustring& value, int count=-1) {
    if (target.empty() || count == 0) return str;
    size_t start_pos = 0;
    while ((start_pos = str.find(target, start_pos)) != ustring::npos && count != 0) {
        str.replace(start_pos, target.length(), value);
        start_pos += value.length();
        --count;
    }
    return str;
}

inline void replace_in_n(ustring &str, const ustring& target, const ustring& value, int count=-1) {
    if (target.empty() || count == 0) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(target, start_pos)) != ustring::npos && count != 0) {
        str.replace(start_pos, target.length(), value);
        start_pos += value.length();
        --count;
    }
}

}

#endif//_UTILS_HPP_