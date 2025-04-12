/// 
/// \file mslib_file.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss File standard library methods
/// 
/// This contains internal implementations of std File methods.
/// 

#ifndef _MSLIB_FILE_HPP_
#define _MSLIB_FILE_HPP_

#include "interpreter.hpp"
#include "commons.hpp"
#include "values.hpp"
#include "mslib.hpp"

namespace moss {
namespace mslib {

/// This namespace hold methods of File class in mslib.
/// The name is prefixed with MS to avoid clashes and overspecification,
/// because of C++'s File class.
namespace MSFile {

Value *open(Interpreter *vm, Value *ths, Value *&err);

Value *close(Interpreter *vm, Value *ths, Value *&err);

Value *readlines(Interpreter *vm, Value *ths, Value *&err);

Value *write(Interpreter *vm, Value *ths, Value *content, Value *&err);

}
}
}


#endif//_MSLIB_FILE_HPP_