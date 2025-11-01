#include "builtins.hpp"
#include "values.hpp"
#include "commons.hpp"
#include "mslib.hpp"
#include "interpreter.hpp"

using namespace moss;
using namespace BuiltIns;

static inline void store_glob_val(opcode::Register reg, ustring name, Value *v, MemoryPool *gf) {
    gf->store(reg, v);
    gf->store_name(reg, name);
}

static void init_cpp_built_ins() {
    using namespace Cpp;
    CppSpace->set_attr("cvoid", CVoid);
    CppSpace->set_attr("cvoid_star", CVoidStar);
    CppSpace->set_attr("clong", CLong);
    CppSpace->set_attr("cdouble", CDouble);
    CppSpace->set_attr("cchar_star", CCharStar);
    CppSpace->set_attr("fstream", FStream);
    CppSpace->set_attr("ffi_cif", Ffi_cif);
}

void BuiltIns::init_constant_variables(MemoryPool *gf, Interpreter *vm) {
    // args
    auto pr_args = clopts::get_program_args();
    std::vector<Value *> largs;
    for (auto a: pr_args) {
        largs.push_back(StringValue::get(a));
    }
    store_glob_val(gf->get_free_reg(), "args", new ListValue(largs), gf);
    // end args

    // Generators.html.STYLE_PATH
#ifdef __linux__
    ustring style_path = "/lib/moss/mossy.css";
#elif defined(__windows__)
    ustring style_path = moss::get_local_app_data_path()+"\\moss\\mossy.css";
#endif
    Value *err = nullptr;
    auto generators_space = mslib::get_space("Generators", vm, err);
    assert(!err && "Generators not in libms?");
    auto html_space = generators_space->get_attr("HTML", vm);
    assert(html_space && "html space not in Generators?");
    html_space->set_attr("STYLE_PATH", StringValue::get(style_path));
    // End STYLE_PATH
}

void BuiltIns::init_built_ins(MemoryPool *gf, opcode::Register &reg) {
    store_glob_val(reg++, "Type", BuiltIns::Type, gf);
    store_glob_val(reg++, "Int", BuiltIns::Int, gf);
    store_glob_val(reg++, "Float", BuiltIns::Float, gf);
    store_glob_val(reg++, "Bool", BuiltIns::Bool, gf);
    store_glob_val(reg++, "List", BuiltIns::List, gf);
    store_glob_val(reg++, "Dict", BuiltIns::Dict, gf);
    store_glob_val(reg++, "NilType", BuiltIns::NilType, gf);
    store_glob_val(reg++, "String", BuiltIns::String, gf);
    store_glob_val(reg++, "Note", BuiltIns::Note, gf);
    store_glob_val(reg++, "Function", BuiltIns::Function, gf);
    // Should this be here? Should it be accessible?
    store_glob_val(reg++, "FunctionList", BuiltIns::FunctionList, gf);
    store_glob_val(reg++, "Module", BuiltIns::Module, gf);
    store_glob_val(reg++, "Space", BuiltIns::Space, gf);
    store_glob_val(reg++, "Enum", BuiltIns::Enum, gf);

    store_glob_val(reg++, "super", BuiltIns::super, gf);

    store_glob_val(reg++, "Range", BuiltIns::Range, gf);
    store_glob_val(reg++, "File", BuiltIns::File, gf);

    store_glob_val(reg++, "StopIteration", BuiltIns::StopIteration, gf);
    store_glob_val(reg++, "Exception", BuiltIns::Exception, gf);
    store_glob_val(reg++, "NameError", BuiltIns::NameError, gf);
    store_glob_val(reg++, "AttributeError", BuiltIns::AttributeError, gf);
    store_glob_val(reg++, "ModuleNotFoundError", BuiltIns::ModuleNotFoundError, gf);
    store_glob_val(reg++, "TypeError", BuiltIns::TypeError, gf);
    store_glob_val(reg++, "AssertionError", BuiltIns::AssertionError, gf);
    store_glob_val(reg++, "NotImplementedError", BuiltIns::NotImplementedError, gf);
    store_glob_val(reg++, "ParserError", BuiltIns::ParserError, gf);
    store_glob_val(reg++, "SyntaxError", BuiltIns::SyntaxError, gf);
    store_glob_val(reg++, "LookupError", BuiltIns::LookupError, gf);
    store_glob_val(reg++, "IndexError", BuiltIns::IndexError, gf);
    store_glob_val(reg++, "KeyError", BuiltIns::KeyError, gf);
    store_glob_val(reg++, "ValueError", BuiltIns::ValueError, gf);
    store_glob_val(reg++, "MathError", BuiltIns::MathError, gf);
    store_glob_val(reg++, "DivisionByZeroError", BuiltIns::DivisionByZeroError, gf);
    store_glob_val(reg++, "OSError", BuiltIns::OSError, gf);
    store_glob_val(reg++, "FileNotFoundError", BuiltIns::FileNotFoundError, gf);
    store_glob_val(reg++, "EOFError", BuiltIns::EOFError, gf);
    store_glob_val(reg++, "OutputError", BuiltIns::OutputError, gf);
    store_glob_val(reg++, "SystemExit", BuiltIns::SystemExit, gf);

    store_glob_val(reg++, "PythonObject", BuiltIns::PythonObject, gf);

    store_glob_val(reg++, "cpp", BuiltIns::Cpp::CppSpace, gf);
    
    init_cpp_built_ins();
}

Value *BuiltIns::Type = new ClassValue("Type");

Value *BuiltIns::Int = new ClassValue("Int");
Value *BuiltIns::Float = new ClassValue("Float");
Value *BuiltIns::Bool = new ClassValue("Bool");
Value *BuiltIns::NilType = new ClassValue("NilType");
Value *BuiltIns::String = new ClassValue("String");
Value *BuiltIns::Note = new ClassValue("Note");
Value *BuiltIns::List = new ClassValue("List");
Value *BuiltIns::Dict = new ClassValue("Dict");

Value *BuiltIns::Function = new ClassValue("Function");
Value *BuiltIns::FunctionList = new ClassValue("FunctionList");

Value *BuiltIns::Module = new ClassValue("Module");
Value *BuiltIns::Space = new ClassValue("Space");
Value *BuiltIns::Enum = new ClassValue("Enum");

Value *BuiltIns::super = new ClassValue("super");

Value *BuiltIns::Range = new ClassValue("Range");
Value *BuiltIns::File = new ClassValue("File");

Value *BuiltIns::StopIteration = new ClassValue("StopIteration");
Value *BuiltIns::Exception = new ClassValue("Exception");
Value *BuiltIns::NameError = new ClassValue("NameError");
Value *BuiltIns::AttributeError = new ClassValue("AttributeError");
Value *BuiltIns::ModuleNotFoundError = new ClassValue("ModuleNotFoundError");
Value *BuiltIns::TypeError = new ClassValue("TypeError");
Value *BuiltIns::AssertionError = new ClassValue("AssertionError");
Value *BuiltIns::NotImplementedError = new ClassValue("NotImplementedError");
Value *BuiltIns::ParserError = new ClassValue("ParserError");
Value *BuiltIns::SyntaxError = new ClassValue("SyntaxError");
Value *BuiltIns::LookupError = new ClassValue("LookupError");
Value *BuiltIns::IndexError = new ClassValue("IndexError");
Value *BuiltIns::KeyError = new ClassValue("KeyError");
Value *BuiltIns::ValueError = new ClassValue("ValueError");
Value *BuiltIns::MathError = new ClassValue("MathError");
Value *BuiltIns::DivisionByZeroError = new ClassValue("DivisionByZeroError");
Value *BuiltIns::OSError = new ClassValue("OSError");
Value *BuiltIns::FileNotFoundError = new ClassValue("FileNotFoundError");
Value *BuiltIns::EOFError = new ClassValue("EOFError");
Value *BuiltIns::OutputError = new ClassValue("OutputError");
Value *BuiltIns::SystemExit = new ClassValue("SystemExit");
Value *BuiltIns::PythonObject = new ClassValue("PythonObject");

Value *BuiltIns::Cpp::CppSpace = new SpaceValue("cpp", nullptr);
Value *BuiltIns::Cpp::CVoid = new ClassValue("cvoid");
Value *BuiltIns::Cpp::CVoidStar = new ClassValue("cvoid_star");
Value *BuiltIns::Cpp::CLong = new ClassValue("clong");
Value *BuiltIns::Cpp::CDouble = new ClassValue("cdouble");
Value *BuiltIns::Cpp::CCharStar = new ClassValue("cchar_star");
Value *BuiltIns::Cpp::FStream = new ClassValue("fstream");
Value *BuiltIns::Cpp::Ffi_cif = new ClassValue("ffi_cif");

Value *BuiltIns::Nil = NilValue::Nil();
Value *BuiltIns::True = BoolValue::True();
Value *BuiltIns::False = BoolValue::False();
