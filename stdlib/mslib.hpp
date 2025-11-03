/// 
/// \file mslib.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
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
#include <functional>
#include <unordered_map>

namespace moss {

namespace mslib {

/// \brief Type of internal function dispatcher.
using mslib_dispatcher = std::function<Value*(Interpreter*, CallFrame*, Value*&)>;

/// \brief Class that holds map of internal functions and their executors.
class FunctionRegistry {
public:
    /// \return Moss stdin function registery (map of function names and their c++ functions)
    static const std::unordered_map<std::string, mslib_dispatcher>& get_registry(ustring module_name);
};

/// Some modules need to intialize their constants internally this method will call such initializers.
/// \note This is not called for all modules imported only those marked "internal_module"
void call_const_initializer(ustring module_name, Interpreter *vm);

/// \brief Tries to extract attribute name from obj, if it is not present, then err is set to attribute_error.
/// \return obj's attribute name if present, nullptr otherwise and sets err.
Value *get_attr(Value *obj, ustring name, Interpreter *vm, Value *&err);

/// \brief Looks up an enum in current VM scope and returns it if present, otherwise err is set to name or type error.
/// \return enum name or nullptr and err is set. 
EnumTypeValue *get_enum(ustring name, Interpreter *vm, Value *&err);

/// \brief Looks up an enum in VM of cf function scope and returns it if present, otherwise err is set to name or type error.
/// \return enum name or nullptr and err is set. 
EnumTypeValue *get_enum(ustring name, CallFrame *cf, Value *&err);

/// \brief Looks up a space in current VM scope and returns it if present, otherwise err is set to name or type error.
/// \return space name or nullptr and err is set. 
SpaceValue *get_space(ustring name, Interpreter *vm, Value *&err);

/// \brief Looks up a name in global frame symbol table and returns its register.
/// If name is not found then name error exception is raised.
/// \return Register of name in global frame.
/// \warning Raises name error if not found.
opcode::Register get_global_register_of(Interpreter *vm, ustring name);

/// \brief Calls a converter fname to convert to type tname of v.
/// For example this is to call __Int converter of some object to convert it to Int.
/// This function does runtime_method_call and if any check fails sets err to type error, but runtime call might also
/// raise (this will not be set to err).
/// \return Value returned by type converter or nullptr and err set to an exception.
Value *call_type_converter(Interpreter *vm, Value *v, const char *tname, const char *fname, Value *&err);

/// \brief Calls constructor name in VM of cf with arguments args.
/// This function does runtime_constructor_call and if any check fails sets err to type error or name error, but
/// runtime call might raise (this will not be set to err).
/// \return Object constructed by called constructor or nullptr and err set to an exception.
Value *call_constructor(Interpreter *vm, CallFrame *cf, ustring name, std::initializer_list<Value *> args, Value *&err);

/// \brief Extracts StringConst from Value, which has to be StringValue.
/// This function only asserts, does not raise. Type of v has to be checked before call to this.
inline opcode::StringConst get_string(Value *v) {
    assert(v && "Passed nullptr to extractor");
    auto strv = dyn_cast<StringValue>(v);
    assert(strv && "Value to extract string is not a StringValue");
    return strv->get_value();
} 

/// \brief Extracts IntConst from Value, which has to be IntValue.
/// This function only asserts, does not raise. Type of v has to be checked before call to this.
inline opcode::IntConst get_int(Value *v) {
    assert(v && "Passed nullptr to extractor");
    auto vv = dyn_cast<IntValue>(v);
    assert(vv && "Value to extract int is not a IntValue");
    return vv->get_value();
}

/// \brief Extracts FloatConst from Value, which has to be FloatValue.
/// This function only asserts, does not raise. Type of v has to be checked before call to this.
inline opcode::FloatConst get_float(Value *v) {
    assert(v && "Passed nullptr to extractor");
    auto vv = dyn_cast<FloatValue>(v);
    assert(vv && "Value to extract float is not a FloatValue");
    return vv->get_value();
}

/// \brief Extracts BoolConst from Value, which has to be BoolValue.
/// This function only asserts, does not raise. Type of v has to be checked before call to this.
inline opcode::BoolConst get_bool(Value *v) {
    assert(v && "Passed nullptr to extractor");
    auto vv = dyn_cast<BoolValue>(v);
    assert(vv && "Value to extract bool is not a BoolValue");
    return vv->get_value();
}

/// \brief Extracts std::vector from Value, which has to be ListValue.
/// This function only asserts, does not raise. Type of v has to be checked before call to this.
inline std::vector<Value *> &get_list(Value *v) {
    assert(v && "Passed nullptr to extractor");
    auto vv = dyn_cast<ListValue>(v);
    assert(vv && "Value to extract list is not a ListValue");
    return vv->get_vals();
}

/// \brief Executes a runtime function
/// \param vm VM for accessing resources
/// \param module_name Name of the module where the function resides
/// \param name Name of the function to execute
/// \param err Possible exception from execution
void dispatch(Interpreter *vm, ustring module_name, ustring name, Value *&err);

/// Calls deinitializers for modules, which need it, like python module.
void deinitialize_modules();

/// \brief Constructs an exception from given type and message.
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

inline Value *create_not_implemented_error(ustring msg) {
    return create_exception(BuiltIns::NotImplementedError, msg);
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

inline Value *create_key_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::KeyError, dmsg);
}

inline Value *create_value_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::ValueError, dmsg);
}

inline Value *create_file_not_found_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::FileNotFoundError, dmsg);
}

inline Value *create_eof_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::EOFError, dmsg);
}

inline Value *create_output_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::OutputError, dmsg);
}

inline Value *create_os_error(diags::Diagnostic dmsg) {
    return create_exception(BuiltIns::OSError, dmsg);
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