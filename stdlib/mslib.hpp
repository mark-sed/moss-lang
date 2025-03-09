/// 
/// \file mslib.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss standard library
/// 
/// This contains internal implementations of std functions.
/// 

#ifndef _MSLIB_HPP_
#define _MSLIB_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "memory.hpp"
#include "values.hpp"
#include "diagnostics.hpp"
#include <cassert>

namespace moss {

namespace mslib {

/// \brief Executes a runtime function
/// \param vm VM for accessing resources
/// \param name Name of the function to execute
/// \param err Possible exception from execution
void dispatch(Interpreter *vm, ustring name, Value *&err);
void global_init();

Value *create_exception(Value *type, ustring msg);
Value *create_exception(Value *type, diags::Diagnostic dmsg);

inline Value *create_name_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::NameError, dmsg);
}

inline Value *create_attribute_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::AttributeError, dmsg);
}

inline Value *create_module_not_found_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::ModuleNotFoundError, dmsg);
}

inline Value *create_type_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::TypeError, dmsg);
}

inline Value *create_assertion_error(ustring msg) {
    return create_exception(BuiltIns::AssertionError, msg);
}

inline Value *create_not_implemented_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::NotImplementedError, dmsg);
}

inline Value *create_parser_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::ParserError, dmsg);
}

inline Value *create_syntax_error(ustring msg) {
    return create_exception(BuiltIns::SyntaxError, msg);
}

inline Value *create_syntax_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::SyntaxError, dmsg);
}

inline Value *create_index_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::IndexError, dmsg);
}

inline Value *create_value_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::ValueError, dmsg);
}

inline Value *create_file_not_found_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::FileNotFoundError, dmsg);
}

inline Value *create_stop_iteration() {
    auto clt = dyn_cast<ClassValue>(BuiltIns::StopIteration);
    assert(clt && "Passed non class type value");
    return new ObjectValue(clt);
}

inline Value *create_math_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::MathError, dmsg);
}

inline Value *create_division_by_zero_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::DivisionByZeroError, dmsg);
}

}

}

#endif//_MSLIB_HPP_