/// 
/// \file diagnostics.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Diagnostic messages for errors
/// 

#ifndef _DIAGNOSTICS_HPP_
#define _DIAGNOSTICS_HPP_

#include "source.hpp"
#include "scanner.hpp"
#include "commons.hpp"
#include "utils.hpp"
#include <cassert>
#include <optional>

namespace moss {

class Token;
class Scanner;
class ErrorToken;

/// Diagnostic messages and its resources
namespace diags {

/// \brief ID of diagnostic error
/// This value corresponds to the error message tied to this error type
enum DiagID : unsigned {
    UNKNOWN = 0,            ///< This value should not be reported
    SYNTAX_ERROR,           ///< Error with syntax (in scanner)
    EXPECTED_END,           ///< Missing ; or nl 
    ASSERT_MISSING_PARENTH, ///< Assert specific - missing () for when someone uses raise syntax
    ASSERT_EXPECTS_ARG,     ///< Assert specific - incorrect arguments
    INVALID_NOTE_PREFIX,    ///< When note prfix is not a variable
    EXPR_EXPECTED,          ///< Parser expected an expression
    DECL_EXPECTED,          ///< Parser expected a declaration
    EXPR_EXPECTED_NOTE,     ///< As above but with extra note
    MISSING_RIGHT_PAREN,    ///< No ')' found
    MISSING_RIGHT_SQUARE,   ///< No ']' found
    UNMATCHED_RIGHT_PAREN,  ///< Extra ')' found
    CHAINED_COMPOUND_ASSIGN,    ///< Chained compound assignment operators
    CANNOT_CHAIN_SILENT,        ///< Chained ~
    TERNARY_IF_MISSING_FALSE,   ///< Missing false branch in ternary if
    TERNARY_IF_MISSING_COND,    ///< Missing condition in ternary if
    RANGE_EXPECTED,         ///< Range without ..
    MISSING_SPACE_BODY,     ///< Incorrect space construction
    MISSING_BLOCK,          ///< Expected {
    MISSING_RIGHT_CURLY,    ///< No '}' found
    UNKNOWN_ESC_SEQ,        ///< Unknown escape sequence in string
    BIN_OP_REQUIRES_LHS,    ///< Binary operator requires a lhs value
    NO_LHS_FOR_SET,         ///< No left hand side value for assignment
    NO_LHS_IN_RANGE,        ///< Range without start
    NO_LHS_IN_ACCESS,       ///< Element access has no lhs
    NO_LHS_IN_SUBSCRIPT,    ///< No lhs for [] or [..]
    ENUM_REQUIRES_NAME,     ///< For when enum declaration has no name
    INCORRECT_ENUM_VALUE,   ///< Unknown value inside of an enum
    MISSING_ENUM_SEPAR,     ///< Missing separator after enum value
    ENUM_VALUE_REDEFINITION,///< Enum value already defined
    IF_REQUIRES_PARENTH,    ///< If has to be followed by parenthesis
    WHILE_REQUIRES_PARENTH, ///< While has to be followed by parenthesis
    FOR_REQUIRES_PARENTH,   ///< For loop has to be followed by parenthesis
    CATCH_REQUIRES_PARENTH, ///< Catch has to be followed by parenthesis
    SWITCH_REQUIRES_PARENTH,///< Switch has to be followed by parenthesis
    FUN_REQUIRES_PARENTH,   ///< Fun has to have arg list
    ELSE_WITHOUT_IF,        ///< Standalone else
    CASE_OUTSIDE_SWITCH,    ///< Standalone case
    DEFAULT_OUTSIDE_SWITCH, ///< Standalone default
    CATCH_WITHOUT_TRY,      ///< Standalone catch
    FINALLY_WITHOUT_TRY,    ///< Standalone finally
    UNEXPECTED_EOF,         ///< Found EOF but some construct was not fully parsed
    NO_WHILE_AFTER_DO,      ///< Do but then no while
    FOR_MISSING_COLON,      ///< No : after iterator in for
    CATCH_EXPECTED,         ///< Try without catch
    INCORRECT_ARGUMENT,     ///< Value is not ID for argument
    TYPE_EXPECTED,          ///< Expecting type name
    MEMBER_OR_ID_EXPECTED,   ///< Expecting name or member
    ID_EXPECTED,            ///< Name expected
    SWITCH_BODY_EXPECTED,   ///< Missing body after switch
    SWITCH_CASE_EXPECTED,   ///< Expecting case or default
    CASE_MISSING_COLON,     ///< No : after case values
    MULTIPLE_DEFAULTS,      ///< More than 1 default in a switch
    PARENT_LIST_EXPECTED,   ///< No parents after a colon
    MISSING_CLASS_NAME,     ///< Class without name
    DEFAULT_NOT_ALLOWED,    ///< Default value assigned where it cannot be
    MISSING_FUN_BODY,       ///< No = or {} after function declaration
    MULTIPLE_VARARGS,       ///< Multiple variable arguments in single function
    NOT_PURE_VARARG,        ///< Setting type or value to vararg
    ANONYMOUS_FUN,          ///< Function without a name
    SET_EXPECTED,           ///< Expecting '='
    LAMBDA_WITH_BODY,       ///< {} after a lambda
    SET_NOT_ALLOWED,        ///< Assignment is not allowed here
    DICT_NO_COLON,          ///< No : after a key
    EMPTY_DICT_WITHOUT_COLON, ///< {} is not an empty dict
    STAR_IMPORT_GLOBAL,     ///< ::*
    STAR_MEMBER_OUTSIDE_IMPORT, ///< x.* not in import
    COLON_EXPECTED,         ///< Expecting :
    LIST_COMP_NOT_ASSIGN,   ///< Value in assignment section of list comprehension is not an assignment
    ANNOT_EXPECTS_ID_NAME,  ///< Annotation has to have an ID name
    CANNOT_BE_ANNOTATED,    ///< Construct that cannot be annotated
    DANGLING_ANNOTATION,    ///< Annotation not followed by anything
    INTERNAL_WITHOUT_BODY,  ///< Internally marked function is not internal or has not been implemented
    SET_EXPECTED_FOR_MULTIVAL, ///< a,b,c = val but missing =
    EXPR_CANNOT_BE_ASSIGN_TO, /// a+1, b = 2
    NAME_NOT_DEFINED,       ///< When name is not found
    GLOB_NAME_NOT_DEFINED,  ///< When name is not found
    ATTRIB_NOT_DEFINED,     ///< When attr is not 
    BOOL_EXPECTED,          ///< Got other type than bool
    NOT_CALLABLE,           ///< Calling non function type
    INCORRECT_CALL,         ///< Cannot call it like this
    INCORRECT_CALL_DETAILED,///< Cannot call function with more details about the call
    OPERATOR_NOT_DEFINED,   ///< Operator was not created for this class
    NOT_A_TYPE,             ///< Name is not a type or does not exist
    CANNOT_FIND_MODULE,     ///< Module cannot be found on known paths
    MISSING_ANNOT_TYPE_ARGUMENT, ///< Annotation has Nil or incorrect type argument
    UNEXPECTED_TYPE,        ///< Got unexpected type
    UNSUPPORTED_OPERAND_TYPE, ///< Operator used on incorrect type
    UNSUPPORTED_UN_OPERAND_TYPE, ///< Unary operator used on incorrect type
    OUT_OF_BOUNDS,          ///< Outside of range
    CLASS_CALL_NEEDS_THIS,  ///< Calling class method without this arg
    PASSED_MORE_ARGS,       ///< Function takes less args
    ARG_MISMATCH,           ///< Argument were not matched to params
    NOT_ITERABLE_TYPE,      ///< Cannot iterate over this type
    NO_NEXT_DEFINED,        ///< Object does not have __next for for
    DIV_BY_ZERO,            ///< Division by 0
    FDIV_BY_ZERO,           ///< Float division by 0
    CANNOT_CREATE_ATTR,     ///< Creating attribute on non-modifiable value
    CANNOT_DELETE_ATTR,     ///< Deleting attribute of non-modifiable value
    NON_LOC_IN_GLOB,        ///< Use of $ in global frame
    NO_NON_LOC_BINDING,     ///< Cannot bind $ var
    SPACE_IMPORT_AS_ITSELF, ///< ::Name
    INVALID_FOPEN_MODE,     ///< Not a known file open mode
    CANNOT_OPEN_FILE,       ///< Cannot open file
    BAD_OBJ_PASSED,         ///< Method got wrong this object type
    TYPE_NOT_SUBSCRIPT,     ///< Cannot [] the type
    STR_INDEX_NOT_INT_OR_RANGE, ///< Indexing with other type than int or range
    LIST_INDEX_NOT_INT_OR_RANGE, ///< Indexing with other type than int or range
    NO_SETITEM_DEFINED,     ///< Calling [a] = b without __setitem method
    NON_BOOL_FROM_EQ,       ///< When (==) returns non-bool type but was used by operator
    NON_BOOL_FROM_NEQ,      ///< When (!=) returns non-bool type but was used by operator
    NOT_HASHABLE,           ///< Cannot create hash of it
    NON_INT_FROM_HASH,      ///< __hash did not return an Int value
    NO_HASH_DEFINED,        ///< When __hash is missing
    KEY_NOT_FOUND,          ///< For key error
    EXPECTED_CLOSE_FSTRING_EXPR,  ///< More than 1 expression or missing }
    MISMATCHED_ANNOT_ARG_AM,///< Incorrect amount of arguments to an annotation
    CONVERTER_ON_NONFUN,    ///< Annotating non-function as a converter
    GENERATOR_ON_NONFUN,    ///< Annotating non-function as a generator
    CANNOT_FIND_CONVERTER,  ///< No conversion from format of note to output format
    DOC_STRING_AS_EXPR,     ///< a = d"something"
    CANNOT_BE_DOCUMENTED,   ///< Using docstring on some value it cannot store it
    DOC_STRING_NOT_AT_START,///< Doc-string in the middle of the body
    DOC_STRING_IN_REPL,     ///< When trying to document repl
    LAMBDA_CONSTRUCTOR,     ///< Trying to make constructor a lambda
    NON_NIL_RETURN_IN_CONSTR, ///< Constructor with return with a non-nil value
    CHR_NOT_IN_RANGE,       ///< function chr argument outside of specified range
    ORD_INCORRECT_LENGTH,   ///< Ord received string not character
    EOF_INPUT,              ///< Reached EOF
    ENABLE_CODE_OUT_ARG_SET,///< When someone sets arguments for enable code output annotation
    DISABLE_CODE_OUT_ARG_SET,///< Same as above but disabled
    INTERNAL_MODULE_ARG_SET, ///< Same as above but for internal_module annotation
    UNKNOWN_MODULE_ANNOTATION, ///< Annotation name that is not known
    UNKNOWN_ANNOTATION,     ///< ANnotation name that is not known
    NO_TYPE_CONV_F_DEFINED, ///< When class does not have __Int
    TYPE_CANNOT_BE_CONV,    ///< Passed non convertable value
    SHORT_HEX_ESC_SEQ,      ///< When HEX escape sequence is not 2 values
    SHORT_OCT_ESC_SEQ,      ///< When OCT escape sequence is not 3 values
    SHORT_UNICODE16_ESC_SEQ,///< When UNICODE escape sequence is not 4 values
    SHORT_UNICODE32_ESC_SEQ,///< When UNICODE escape sequence is not 8 values
    INCORRECT_HEX_ESC_SEQ,  ///< Incorrect value which is not hexadecimal
    INCORRECT_OCT_ESC_SEQ,  ///< Incorrect value which is not octal
    INCORRECT_UNICODE16_ESC_SEQ, ///< Incorrect value which is not decimal
    INCORRECT_UNICODE32_ESC_SEQ, ///< Incorrect value which is not decimal
    UNIMPLEMENTED_SYNTAX_FEATURE, ///< When something is missing in parser and is returned as a syntax error, but is unimplemented
    KEYWORD_NOT_A_STRING,   ///< When a keyword/argument in a unpacked function call dict is not a string
    MULTIPLE_3DOT_MULTIVAR, ///< When there is more than 1 ...var in multivar
    COMMA_FOR_MULTIVAR_EXPECTED, ///< When there is a sure multivar (...a), but no comma after it
    TOO_MANY_VALS_UNPACK,   ///< When multivar is missing a var to fully unpack
    TOO_FEW_VALS_UNPACK,    ///< When multivar has more vars to unpack than values
    NO_KNOWN_TYPE_CONV_TO_C,///< When type cannot be converted to C equivalent in ffi
    NOT_CPP_MOSS_VALUE,     ///< When t_cpp value was expected, but was other value
    BAD_MULTI_REPLACE_ELEM, ///< When multi_replace does not get list of list of 2 values
    NO_SUPER,               ///< Call to a super without a super
    TIME_TIMESTAMP_OOR,     ///< When timestamp value is oor
    DICT_UNEXPECTED_TYPE,   ///< Dict iterable must return List
    DICT_BAD_ITER_SIZE,     ///< Size of elements in Dict constructor has to be 2
    PARENT_CONFLICT,        ///< When extending multiple built-in classes
    UNEXPECTED_TYPE_FOR_ATTR,///< When the type of attribue is not what was expected
    CANNOT_EXTEND_WITH_BASE,///< When NilType or Bool is used as a base type
    DUPLICATE_ARG,          ///< When there are multiple arguments with the same name in function definition
    NON_DEFAULT_ARG_AFTER_VARARG, ///< When there is argument after vararg one as has no default value
    NON_DEFAULT_ARG_AFTER_DEFVAL, ///< When there is default value argument and after it one without default value
    INCORRECT_ACCESS_SYNATAX,///< Inocorrect type of right in access - e.g. a.2
    NON_LOCAL_AFTER_GLOBAL, ///< When you have ::$x
    INCORRECT_ARG_NAME,     ///< When argument name is not just an ID
    SILENT_ARG,             ///< foo(~a)
    SILENT_IN_EXPR,         ///< When silent is in an expression
    ISINSTANCE_REQUIRES_CLASS,///< When non-class value is passed to isinstance
    ISSUBCLASS_REQUIRES_CLASS,///< When 2nd arg of issubclass is not a class or list of classes
    DUPLICATE_NAME_IN_CALL, ///< When call has duplicate named arg specified
    MIN_LT_MAX_IN_RANDINT,  ///< When min > max in rand_int
    MIN_LT_MAX_IN_RANDF,    ///< When min > max in rand_float
    OP_FUN_OUTSIDE_CLASS,   ///< When operator function appears outside of a class
    DISALLOWED_METHOD_ANNOT, ///< When method is annotated by annotation for only functions
    BREAK_OUTSIDE_OF_LOOP,  ///< When break is not inside of a loop
    CONTINUE_OUTSIDE_OF_LOOP, ///< When continue is outside of a loop
    RECURSIVE_CONVERTER_CALL,///< When converter is called within a converter
    INCOMPATIBLE_ANNOTS,    ///< When there are multiple incompatible annotations
    CONVERTER_INCORR_ARGS,  ///< When converter has incorrect args
    GENERATOR_INCORR_ARGS,  ///< When generator has incorrect args
    MAIN_INCORR_ARGS,       ///< When main has incorrect args
    UNKNOWN_BYTES_ENCODING, ///< When Bytes does not know the format
    EXPECTED_BYTES_IN_WRITE,///< When writing to a binary file something else then bytes
    RETURN_OUTSIDE_FUN,     ///< Return outside of a function
    RETURN_IN_GENERATOR,    ///< When generator has non-nil return
    RETURN_IN_MAIN,         ///< When main function returns non-nil value
    NON_GLOBAL_MAIN,        ///< When function annotated as main is non-global
    MULTIPLE_MAINS,         ///< When a module has more then 1 main annotated function
    LAMBDA_MAIN,            ///< When lambda is marked as main
    DUPLICATE_CLASS_BASE,   ///< When extending twice with the same class
    CANNOT_EXTEND_SEALED_CLASS, ///< When extending @sealed class
    INT_BASE_NOT_INT,       ///< When base in call to Int is not an Int
    COULD_NOT_PARSE_INT,    ///< When Int could not be parsed
    INT_PARSE_CONVERSION_ERR, ///< When there is overflow or underflow in parsing of int
    UNEXPECTED_FUN_RETURN_TYPE, ///< When __Int returns something else than Int
    COULD_NOT_PARSE_FLOAT,  ///< When float could not be parsed
    INPUT_ERROR,            ///< When there is some system error when reading from stdin
    OPERATION_ON_CLOSED_FILE, ///< When trying to work with closed file
    GCD_OR_LCM_EXPECTS_INTS, ///< When gcd or lcm received List with non-int value
    NON_INT_IN_RANGE,       ///< When non-int value is used in range

    NUMBER_OF_IDS           ///< This value should not be reported it can be used to get the amount of IDs
};
 
/// This array holds messages (formatting strings) corresponding to DiagIDs.
/// \note There has to be an entry for every value in DiagsIDs, but the last one
///       used for getting the enum size. 
static const char * DIAG_MSGS[] = {
    "Unknown error",
    "Syntax error", // This is for ErrorToken so it will be replaced with custom message
    "Missing a new line ('\\n') or a semicolon (';') after a statement",
    "Assert expects its arguments in parenthesis",
    "Assert expects 1 or 2 arguments — condition and optional message",
    "Note prefix has to be an identificator — if you require an expression, use function call",
    "Expecting an expression",
    "Expecting a declaration",
    "Expecting an expression — %s",
    "Missing ')'",
    "Missing ']'",
    "Unmatched ')'",
    "Compound assignment operators ('%s') cannot be chained",
    "Silence operator ('~') cannot be chained",
    "Missing ':' — ternary if requires false branch",
    "Ternary if requires a condition",
    "Expecting '..' in a range expression",
    "Space must have a body ('{}')",
    "Expecting code block ('{}')",
    "Missing '}'",
    "Unknown string escape sequence '\\%c'",
    "Incorrect binary operator syntax — operator '%s' requires value on the left and right of it",
    "Assignment requires a left-hand side value",
    "Range requires a starting value",
    "Element access ('.') requires a value to access from",
    "Subscript or slice requires a left-hand side value",
    "Enum declaration requires a name",
    "Incorrect enum value",
    "Enum values have to be separated by comma (','), semicolon (';') or a new line",
    "Value '%s' is already defined in enum '%s'",
    "If statement must have its condition in parenthesis",
    "While statement must have its condition in parenthesis",
    "For loop must have its iterator and collection in parenthesis",
    "Catch must have its argument in parenthesis",
    "Switch must have its argument in parenthesis",
    "Function declaration requires arguments in parenthesis or empty parenthesis ('()')",
    "'else' without a previous 'if'",
    "'case' outside of a switch",
    "'default' outside of a switch",
    "'catch' without a previous 'try'",
    "'finally' without a previous 'try'",
    "Unexpected end of file",
    "Keyword 'do' must be followed by 'while' and a condition",
    "Iterator in for loop must be followed by a colon (':')",
    "Expecting 'catch' after try",
    "Incorrect argument",
    "Type expected",
    "Expecting an identifier or a member",
    "Expecting an identifier",
    "Expecting a switch body with cases",
    "Expecting 'case' or 'default'",
    "Case value or values have to be followed by a colon (':')",
    "Multiple default cases found in a switch — at most 1 default case can be present",
    "Expecting a list of parent classes",
    "Class must have a name",
    "Default value cannot be assigned",
    "Function must have a body ('{}') or lambda must return a value ('=')",
    "Function can have only 1 variable argument ('...')",
    "Variable argument cannot be typed nor have a default value",
    "Function must have a name, only lambdas can be anonymous",
    "Expecting '='",
    "Lambda cannot have a body",
    "Assignment is not allowed here — perhaps you meant to use '=='?",
    "Dictionary key must be followed by a colon (':')",
    "Incorrect '{}' found — perhaps you meant an empty dictionary '{:}' or 'space' for block of code?",
    "Cannot star import from global scope",
    "Star member can appear only in import",
    "Expecting ':'",
    "Only assignments are allowed in assignment section of list comprehension",
    "Annotation must have an ID name",
    "'%s' cannot be annotated — perhaps you meant to use inner annotation ('!@')",
    "Dangling outer annotation — to annotate the module use inner annotation ('!@')",
    "Function '%s' is marked as '@internal', but does not have an internal body",
    "Expecting '=' for multivalue assignment",
    "Found non-assignable expression",
    "Name '%s' is not defined",
    "Global name '%s' is not defined",
    "'%s' has no attribute '%s'",
    "Expected Bool value, but got '%s'",
    "Type '%s' is not callable",
    "Incorrect call to function '%s' — %s",
    "Incorrect call to function '%s' with arguments '(%s)'%s",
    "'%s' has no operator '%s'",
    "'%s' does not name a type",
    "Cannot find module '%s'",
    "Annotation '%s' expects '%s' argument",
    "Expecting type '%s', but got '%s'",
    "Unsupported operand type for operator '%s' — '%s' and '%s'",
    "Unsupported operand type for unary operator '%s' — '%s'",
    "'%s' index '%ld' is out of bounds",
    "calling class method requires object as the last argument",
    "passed more arguments than the function accepts",
    "could not match arguments",
    "Type '%s' is not iterable",
    "Object of class '%s' is not iterable — __next method has to be defined",
    "Division by zero",
    "Float division by zero",
    "Cannot assign to attributes for type '%s'",
    "Cannot delete attribues for type '%s'",
    "Accessing non-local variable ('%s') cannot appear on global scope",
    "No binding for non-local variable '%s'",
    "Cannot import space as itself",
    "Invalid file open mode '%s'",
    "File '%s' was not found or cannot be open",
    "Incorrect object type ('%s') passed as this argument",
    "Type '%s' is not subscriptable",
    "String indices must be Int or Range, but got '%s'",
    "List indices must be Int or Range, but got '%s'",
    "Object of class '%s' cannot set indexed value — __setitem method has to be defined",
    "(==) operator function for type '%s' returned value of type '%s', but Bool is expected",
    "(!=) operator function for type '%s' returned value of type '%s', but Bool is expected",
    "Type '%s' is not hashable",
    "Function __hash for type '%s' returned value of type '%s', but Int is expected",
    "Object of class '%s' cannot be hashed — __hash method has to be defined",
    "Key '%s' not found",
    "Expected closing fstring expression brace ('}')",
    "Annotation '%s' expects '%ld' arguments, but got '%ld'",
    "Only functions and lambdas can be annotated as 'converter', but got '%s'",
    "Only functions and lambdas can be annotated as 'generator', but got '%s'",
    "Found no suitable conversion pipeline from note format '%s' to '%s'",
    "Doc-string ('d\"\"') cannot be used as an expression",
    "Construct '%s' cannot have internal documentation — perhaps use just a comment ('//')",
    "Doc-string ('d\"\"') can appear only at the beginning of the construct",
    "REPL cannot be documented",
    "Constructor cannot be a lambda",
    "Constructor can contain only 'return' or 'return nil'",
    "Value '%ld' for function 'chr' not in range <0; 0x10ffff>",
    "Value ('%s') for function 'ord' has to be exactly 1 character long String",
    "End of input",
    "Annotation 'enable_code_output' does not take any arguments",
    "Annotation 'disable_code_output' does not take any arguments",
    "Annotation 'internal_module' does not take any arguments",
    "Unknown module annotation name '%s'",
    "Unknown annotation name '%s'",
    "Object of type '%s' cannot be converted to %s — %s method has to be defined",
    "Type '%s' cannot be converted to %s",
    "Short hexadecimal escape sequence (has to be 2 values — '\\xhh')",
    "Short octal escape sequence (has to be 3 values — '\\qooo')",
    "Short 16-bit unicode escape sequence (has to be 4 — '\\uxxxx')",
    "Short 32-bit unicode escape sequence (has to be 8 values — '\\Uxxxxxxxx')",
    "Incorrect hexadecimal value in escape sequence ('%s')",
    "Incorrect octal value in escape sequence ('%s')",
    "Incorrect 16-bit unicode value in escape sequence ('%s')",
    "Incorrect 32-bit unicode value in escape sequence ('%s') — perhaps you meant to use 16-bit sequence, which uses '\\uxxxx'",
    "Unimplemented syntax feature. %s",
    "All keywords in a function call with unpacked Dict have to be a String, but found '%s'",
    "Only one variable in multi-variable assignment can be `...`",
    "`...` multivar expects list of variables after the first one",
    "Too many values to unpack (expected %ld values)",
    "Not enough values to unpack (expected at least %ld values, but got %ld)",
    "No known conversion for type '%s' in foreign function interface",
    "Expected C++ compatible value (from cpp space), but got value of type '%s'",
    "String.multi_replace expects as an argument a List of Lists of 2 Strings (e.g.: '[[\"a\", \"b\"], [\"c\", \"d\"]])",
    "Class '%s' does not have a super class to call",
    "Timestamp is too small or too large",
    "Dict constructor expects iterable of List values, but got '%s'",
    "Dict constructor iterable must contain Lists of size 2, got size %ld",
    "Cannot extend multiple built-in classes (extending %s and %s)",
    "Attribute '%s' has expected type of '%s', but got '%s'",
    "%s cannot be a base type for a class",
    "Duplicate argument name '%s' in function '%s'",
    "Arguments after variable argument must have default value (argument '%s' in function '%s')",
    "Argument without default value cannot follow argument with a default value (argument '%s' in function '%s')",
    "Incorrect access ('.') syntax",
    "Cannot have non-local symbol ('$') after global scope ('::')",
    "Incorrect argument name expression",
    "Function argument cannot be silenced",
    "Silent cannot be a part of an expression",
    "Function `isinstance` expects class value or list of class values as the second argument",
    "Function `issubclass` expects class value or list of class values as the second argument",
    "Repeated named argument '%s'",
    "Argument 'min' (%ld) cannot be greater than 'max` in call to rand_int (%ld)",
    "Argument 'min' (%f) cannot be greater than 'max` in call to rand_float (%f)",
    "Operator methods ('%s') can be declared only inside of classes",
    "Method ('%s') cannot be annotated as '%s'",
    "Break outside of a loop",
    "Continue outside of a loop",
    "Recursive converter (from: '%s', to: '%s') call — outputting inside of a converter",
    "Incompatible annotations '%s' and '%s' (on '%s')",
    "Function ('%s') annotated as 'converter' has to take exactly 1 untyped argument or typed as '[String,Note]'",
    "Function ('%s') annotated as 'generator' has to take exactly 1 untyped argument or typed as 'List'",
    "Function ('%s') annotated as 'main' has to take no arguments",
    "Unknown encoding '%s' for conversion to bytes",
    "Bytes type is expected when writing to a binary file, but got '%s'",
    "Return outside function",
    "Generator has to return nil",
    "Function annotated as '@main' has to return nil",
    "Function annotated as '@main' has to be global and only in module scope",
    "Redefinition of '@main' within a module — only 1 function can be annotated as '@main'",
    "Lambda function ('%s') cannot be marked as '@main'",
    "Duplicate base class '%s'",
    "Class '%s' is marked as '@sealed' and cannot be extended",
    "Base in call to `Int` has to be an Int value, but `%s` was passed in",
    "Value '%s' could not be parsed as an Int of base '%ld'",
    "Value '%s' could not be parsed as an Int of base '%ld' — %s",
    "Unexpected return type in call to '%s' — expected '%s', but got '%s'",
    "Value '%s' could not be parsed as a Float",
    "Reading from standard input has failed",
    "I/O operation on closed file",
    "Function %s as an argument expects a List of Ints, but value at index %zu is of type '%s'",
    "Range ('start,next...end') expects Int values, but %s is of type '%s'",
};

/// \brief ID of diagnostic error
/// This value corresponds to the error message tied to this error type
enum WarningID : unsigned {
    WARN_UNKNOWN = 0,           ///< This value should not be reported
    INT_CANNOT_FIT,             ///< When value used is bigger or lower than moss Int
    FLOAT_CANNOT_FIT,           ///< Same as before
    MSB_COMPILED_WITH_NEWER_VER,///< When bytecode was compiled with newer version of moss.
    
    NUMBER_OF_WARNING_IDS      ///< This value should not be reported it can be used to get the amount of IDs
};

/// This array holds messages (formatting strings) corresponding to WarningID.
/// \note There has to be an entry for every value in WarningID, but the last one
///       used for getting the enum size. 
static const char * WARNING_MSGS[] = {
    "Unknown error",
    "Value '%s' cannot fit into Int",
    "Value '%s' is too big or too close to 0 to fit into Float",
    "Program was compiled with newer version of Moss (%s)",
};

/// \brief Diagnostic message for error reporting
/// This class holds all resources needed to provide detailed error report to
/// the user. 
class Diagnostic {
public:
    unsigned id;
    const File &src_f;
    std::optional<SourceInfo> src_info;
    Scanner *scanner;
    ustring msg;
    bool warning;

    template<typename ... Args>
    Diagnostic(File &src_f, SourceInfo src_info, Scanner *scanner, DiagID id, Args ... args) 
               : id(id), src_f(src_f), src_info(src_info), scanner(scanner), warning(false) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
        if (sizeof...(Args) > 0)
            this->msg = utils::formatv(DIAG_MSGS[id], args ...);
        else
            this->msg = DIAG_MSGS[id];
    }

    // Warning
    template<typename ... Args>
    Diagnostic(bool warning, File &src_f, SourceInfo src_info, Scanner *scanner, WarningID id, Args ... args) 
               : id(id), src_f(src_f), src_info(src_info), scanner(scanner), warning(warning) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        static_assert(WarningID::NUMBER_OF_WARNING_IDS == sizeof(WARNING_MSGS)/sizeof(char *) && "Mismatch in warning IDs and messages");
        
        if (!warning) {
            assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
            if (sizeof...(Args) > 0)
                this->msg = utils::formatv(DIAG_MSGS[id], args ...);
            else
                this->msg = DIAG_MSGS[id];
        } else {
            assert(id < std::end(WARNING_MSGS)-std::begin(WARNING_MSGS) && "Warning ID does not have a corresponding message");
            if (sizeof...(Args) > 0)
                this->msg = utils::formatv(WARNING_MSGS[id], args ...);
            else
                this->msg = WARNING_MSGS[id];
        }
    }

    Diagnostic(ErrorToken *token, Scanner *scanner);

    template<typename ... Args>
    Diagnostic(File &src_f, DiagID id, Args ... args) 
               : id(id), src_f(src_f), src_info(std::nullopt), scanner(nullptr), warning(false) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
        if (sizeof...(Args) > 0)
            this->msg = utils::formatv(DIAG_MSGS[id], args ...);
        else
            this->msg = DIAG_MSGS[id];
    }

    template<typename ... Args>
    Diagnostic(bool warning, File &src_f, WarningID id, Args ... args) 
               : id(id), src_f(src_f), src_info(std::nullopt), scanner(nullptr), warning(warning) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        static_assert(WarningID::NUMBER_OF_WARNING_IDS == sizeof(WARNING_MSGS)/sizeof(char *) && "Mismatch in warning IDs and messages");
        
        if (!warning) {
            assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
            if (sizeof...(Args) > 0)
                this->msg = utils::formatv(DIAG_MSGS[id], args ...);
            else
                this->msg = DIAG_MSGS[id];
        } else {
            assert(id < std::end(WARNING_MSGS)-std::begin(WARNING_MSGS) && "Warning ID does not have a corresponding message");
            if (sizeof...(Args) > 0)
                this->msg = utils::formatv(WARNING_MSGS[id], args ...);
            else
                this->msg = WARNING_MSGS[id];
        }
    }
};

}

}

#endif//_DIAGNOSTICS_HPP_