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

namespace moss {

class Token;
class Scanner;
class ErrorToken;

/// Diagnostic messages and its resources
namespace diags {

/// \brief ID of diagnostic error
/// This value corresponds to the error message tied to this error type
enum DiagID {
    UNKNOWN = 0,            ///< This value should not be reported
    SYNTAX_ERROR,           ///< Error with syntax (in scanner)
    EXPECTED_END,           ///< Missing ; or nl 
    ASSERT_MISSING_PARENTH, ///< Assert specific - missing () for when someone uses raise syntax
    ASSERT_EXPECTS_ARG,     ///< Assert specific - incorrect arguments
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
    CANNOT_CREATE_ATTR,     ///< Creating attribute on immutable value
    NON_LOC_IN_GLOB,        ///< Use of $ in global frame
    NO_NON_LOC_BINDING,     ///< Cannot bind $ var
    SPACE_IMPORT_AS_ITSELF, ///< ::Name
    INVALID_FOPEN_MODE,     ///< Not a known file open mode
    CANNOT_OPEN_FILE,       ///< Cannot open file
    BAD_OBJ_PASSED,         ///< Method got wrong this object type

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
    "Expecting an identificator or a member",
    "Expecting an identificator",
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
    "Dangling outter annotation — to annotate the module use inner annotation ('!@')",
    "Function '%s' is marked as '@internal', but does not have an internal body",
    "Expecting '=' for multivalue asignment",
    "Found non-assignable expression",
    "Name '%s' is not defined",
    "Global name '%s' is not defined",
    "'%s' has no attribute '%s'",
    "Expected Bool value, but got '%s'",
    "Type '%s' is not callable",
    "Incorrect call to function '%s' — %s",
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
    "Accessing non-local variable ('%s') cannot appear on global scope",
    "No binding for non-local variable '%s'",
    "Cannot import space as itself",
    "Invalid file open mode '%s'",
    "File '%s' was not found or cannot be open",
    "Incorrect object type ('%s') passed as this argument",
};

/// \brief Diagnostic message for error reporting
/// This class holds all resources needed to provide detailed error report to
/// the user. 
class Diagnostic {
public:
    DiagID id;
    const File &src_f;
    Token *token;
    Scanner *scanner;
    ustring msg;

    template<typename ... Args>
    Diagnostic(File &src_f, Token *token, Scanner *scanner, DiagID id, Args ... args) 
               : id(id), src_f(src_f), token(token), scanner(scanner) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
        if (sizeof...(Args) > 0)
            this->msg = utils::formatv(DIAG_MSGS[id], args ...);
        else
            this->msg = DIAG_MSGS[id];
    }

    Diagnostic(ErrorToken *token, Scanner *scanner);

    template<typename ... Args>
    Diagnostic(File &src_f, DiagID id, Args ... args) 
               : id(id), src_f(src_f), token(nullptr), scanner(nullptr) {
        static_assert(DiagID::NUMBER_OF_IDS == sizeof(DIAG_MSGS)/sizeof(char *) && "Mismatch in error IDs and messages");
        assert(id < std::end(DIAG_MSGS)-std::begin(DIAG_MSGS) && "Diagnostics ID does not have a corresponding message");
        if (sizeof...(Args) > 0)
            this->msg = utils::formatv(DIAG_MSGS[id], args ...);
        else
            this->msg = DIAG_MSGS[id];
    }
};

}

}

#endif//_DIAGNOSTICS_HPP_