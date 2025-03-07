/// 
/// \file values_cpp.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss VM types that enclose C++ types
/// 

#ifndef _VALUES_CPP_HPP_
#define _VALUES_CPP_HPP_

#include "values.hpp"

namespace moss {
namespace t_cpp {

    class CppValue : public Value {
    public:
        CppValue(TypeKind ClassType, ustring name, Value *type) 
            : Value(ClassType, name, type) {}
    
        virtual Value *clone() {
            assert(false && "Cannot coppy CppValue");
            return nullptr;
        }
    
        virtual opcode::StringConst as_string() const override {
            return "<C++ value of type " + name + ">";
        }
    
        virtual std::ostream& debug(std::ostream& os) const override {
            os << type->get_name() << "(" << name << ")";
            return os;
        }
    };
    
    class FStreamValue : public CppValue {
    private:
        std::fstream &fs;
    public:
        static const TypeKind ClassType = TypeKind::CPP_FSTREAM;
    
        FStreamValue(std::fstream &fs) 
            : CppValue(ClassType, "std::fstream", BuiltIns::Cpp::FStream), fs(fs) {}
    
        std::fstream &get_fs() { return this->fs; }
    
        virtual Value *clone() {
            // TODO: Maybe copy the value
            return new FStreamValue(fs);
        }
    };
}
}

#endif//_VALUES_CPP_HPP_