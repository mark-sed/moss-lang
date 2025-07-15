/// 
/// \file values_cpp.hpp
/// \author Marek Sedlacek
/// \copyright Copyright 2024-2025 Marek Sedlacek. All rights reserved.
///            See accompanied LICENSE file.
/// 
/// \brief Moss VM types that enclose C++ types
/// 

#ifndef _VALUES_CPP_HPP_
#define _VALUES_CPP_HPP_

#include "values.hpp"
#include <ffi.h>

namespace moss {
namespace t_cpp {

    /// C++ value encompased in a Moss value
    class CppValue : public Value {
    public:
        CppValue(TypeKind ClassType, ustring name, Value *type) 
            : Value(ClassType, name, type) {}
    
        ~CppValue() {}

        virtual Value *clone() override {
            assert(false && "Cannot coppy CppValue");
            return nullptr;
        }

        virtual inline bool is_hashable() override { return true; }

        virtual Value *to_moss() {
            assert(false && "Called to_moss on a type which does not have a conversion");
            return BuiltIns::Nil;
        }
    
        virtual opcode::StringConst as_string() const override {
            return "<C++ value of type " + name + ">";
        }
    
        virtual std::ostream& debug(std::ostream& os) const override {
            os << type->get_name() << "(" << name << ")";
            return os;
        }
    };

    class CVoidValue : public CppValue {
    public:
        static const TypeKind ClassType = TypeKind::CPP_CVOID;
    
        CVoidValue() : CppValue(ClassType, "void", BuiltIns::Cpp::CVoid) {}
        ~CVoidValue() {
            // This is a general pointer, which is not handled nor allocated
            // by this object, so it cannot free it
        }

        virtual Value *to_moss() override {
            // Void returns nil because this can be then used in return value
            // and also there is no equivalent.
            return BuiltIns::Nil;
        }
    
        virtual Value *clone() override {
            return this;
        }
    };

    class CVoidStarValue : public CppValue {
    private:
        void *value;
        bool delete_data;
    public:
        static const TypeKind ClassType = TypeKind::CPP_CVOID_STAR;
    
        CVoidStarValue(void *value, bool delete_data=false) : CppValue(ClassType, "void*", BuiltIns::Cpp::CVoidStar), value(value), delete_data(delete_data) {}
        ~CVoidStarValue() {
            // When delete_data was set then this value is the owner of the
            // object and has to free it.
            if (delete_data) {
                // Deleting void * is not "safe"
                delete value;
            }
        }

        void *get_value() { return this->value; }

        virtual void *get_data_pointer() override {
            return &value;
        }
    
        virtual Value *clone() override {
            return this;
        }
    };

    class CLongValue : public CppValue {
    private:
        long value;
    public:
        static const TypeKind ClassType = TypeKind::CPP_CLONG;
    
        CLongValue(long value) : CppValue(ClassType, "long", BuiltIns::Cpp::CLong), value(value) {}
        ~CLongValue() { }

        long get_value() { return this->value; }

        virtual Value *to_moss() override {
            return new IntValue(value);
        }

        virtual void *get_data_pointer() override {
            return &value;
        }
    
        virtual Value *clone() override {
            return this;
        }
    };

    class CDoubleValue : public CppValue {
    private:
        double value;
    public:
        static const TypeKind ClassType = TypeKind::CPP_CDOUBLE;
    
        CDoubleValue(double value) : CppValue(ClassType, "double", BuiltIns::Cpp::CDouble), value(value) {}
        ~CDoubleValue() { }

        double get_value() { return this->value; }

        virtual Value *to_moss() override {
            return new FloatValue(value);
        }

        virtual void *get_data_pointer() override {
            return &value;
        }
    
        virtual Value *clone() override {
            return this;
        }
    };

    // TODO: Maybe just having a pointer type is enough?
    class CCharStarValue : public CppValue {
    private:
        char *value;
    public:
        static const TypeKind ClassType = TypeKind::CPP_CCHAR_STAR;
    
        CCharStarValue(char *value) : CppValue(ClassType, "char*", BuiltIns::Cpp::CDouble), value(value) {}
        ~CCharStarValue() { }

        char *get_value() { return this->value; }

        virtual Value *to_moss() override {
            return new StringValue(ustring(value));
        }

        virtual void *get_data_pointer() override {
            return &value;
        }
    
        virtual Value *clone() override {
            return this;
        }
    };
    
    /// C++'s std::fstream as a moss value
    class FStreamValue : public CppValue {
    private:
        std::fstream *fs;
    public:
        static const TypeKind ClassType = TypeKind::CPP_FSTREAM;
    
        FStreamValue(std::fstream *fs) 
            : CppValue(ClassType, "std::fstream", BuiltIns::Cpp::FStream), fs(fs) {}
        ~FStreamValue() {
            delete fs;
        }

        std::fstream *get_fs() { return this->fs; }
    
        virtual Value *clone() override {
            // TODO: Maybe copy the value
            return new FStreamValue(fs);
        }
    };

    class Ffi_cifValue : public CppValue {
    private:
        ffi_cif value;
    public:
        static const TypeKind ClassType = TypeKind::CPP_FFI_CIF;
    
        Ffi_cifValue(ffi_cif value) : CppValue(ClassType, "ffi_cif", BuiltIns::Cpp::Ffi_cif), value(value) {}
        ~Ffi_cifValue() {
        }

        ffi_cif get_value() { return this->value; }
    
        virtual Value *clone() override {
            return this;
        }
    };
}
}

#endif//_VALUES_CPP_HPP_