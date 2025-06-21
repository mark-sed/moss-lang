#include "mslib_file.hpp"
#include "values_cpp.hpp"
#include <algorithm>
#include <cstdlib>

using namespace moss;
using namespace mslib;

static bool str_to_ios_mode(const std::string& mode, std::ios_base::openmode &ios_mode) {
    static const std::unordered_map<std::string, std::ios_base::openmode> mode_map = {
        {"r", std::ios_base::in},
        {"w", std::ios_base::out},
        {"a", std::ios_base::app},
        {"rb", std::ios_base::in | std::ios_base::binary},
        {"wb", std::ios_base::out | std::ios_base::binary},
        {"ab", std::ios_base::app | std::ios_base::binary},
        {"r+", std::ios_base::in | std::ios_base::out},
        {"w+", std::ios_base::in | std::ios_base::out | std::ios_base::trunc},
        {"a+", std::ios_base::in | std::ios_base::out | std::ios_base::app},
        {"r+b", std::ios_base::in | std::ios_base::out | std::ios_base::binary},
        {"w+b", std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary},
        {"a+b", std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::binary}
    };

    auto it = mode_map.find(mode);
    if (it != mode_map.end()) {
        ios_mode = it->second;
        return true;
    }
    return false;
}

Value *MSFile::open(Interpreter *vm, Value *ths, Value *&err) {
    auto path = mslib::get_attr(ths, "path", vm, err);
    auto mode = mslib::get_attr(ths, "mode", vm, err);
    std::ios_base::openmode ios_mode;
    if (!str_to_ios_mode(mode->as_string(), ios_mode)) {
        err = create_value_error(diags::Diagnostic(*vm->get_src_file(), diags::INVALID_FOPEN_MODE, mode->as_string().c_str()));
        return BuiltIns::Nil;
    }
    std::fstream *fs = new std::fstream(path->as_string(), ios_mode);
    if (!fs->is_open()) {
        // TODO: Give more precise error, if file cannot be open or cannot be found
        err = create_file_not_found_error(diags::Diagnostic(*vm->get_src_file(), diags::CANNOT_OPEN_FILE, path->as_string().c_str()));
        return BuiltIns::Nil;
    }
    ths->set_attr("__fstream", new t_cpp::FStreamValue(fs));
    return BuiltIns::Nil;
}

Value *MSFile::close(Interpreter *vm, Value *ths, Value *&err) {
    // TODO: Check if file is open
    auto fstrm_v = mslib::get_attr(ths, "__fstream", vm, err);
    auto fstrm = dyn_cast<t_cpp::FStreamValue>(fstrm_v);
    assert(fstrm && "Not FStream value");
    fstrm->get_fs()->close();
    ths->set_attr("__fstream", BuiltIns::Nil);
    return BuiltIns::Nil;
}

Value *MSFile::readlines(Interpreter *vm, Value *ths, Value *&err) {
    // TODO: Generate exceptions on errors
    assert(ths->has_attr("__fstream", vm) && "no __fstream generated");
    auto fsv = ths->get_attr("__fstream", vm);
    auto fsfs = dyn_cast<t_cpp::FStreamValue>(fsv);
    assert(fsfs && "fstream is not std::fstream");
    auto lines = new ListValue();
    ustring line;
    std::fstream *fstrm = fsfs->get_fs();
    while(std::getline(*fstrm, line)) {
        lines->push(new StringValue(line));
    }
    return lines;
}

Value *MSFile::write(Interpreter *vm, Value *ths, Value *content, Value *&err) {
    // TODO: Generate exceptions on errors
    assert(ths->has_attr("__fstream", vm) && "no __fstream generated");
    auto fsv = ths->get_attr("__fstream", vm);
    auto fsfs = dyn_cast<t_cpp::FStreamValue>(fsv);
    assert(fsfs && "fstream is not std::fstream");
    *(fsfs->get_fs()) << opcode::to_string(vm, content);
    return BuiltIns::Nil;
}