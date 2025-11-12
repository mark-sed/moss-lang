#include "source.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include <cstring>
#include <fstream>
#include <filesystem>

using namespace moss;

const SourceFile SourceInfo::dummy_file("<internal>", SourceFile::SourceType::FILE);

std::istream *SourceFile::get_new_stream() {
    switch(this->type) {
        case SourceFile::SourceType::FILE: {
            std::ifstream *f = new std::ifstream(this->path_or_code);
            if(f->fail()){
                error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
            }
            return f;
        }
        case SourceFile::SourceType::STRING: {
            return new std::istringstream(this->path_or_code);
        }
        case SourceFile::SourceType::STDIN: return &std::cin;
        case SourceFile::SourceType::REPL: {
            // REPL input is handled on its own, but reads from stdin
            return &std::cin;
        }
    }
    error::error(error::ErrorCode::INTERNAL, "Unknown input format", this);
    return nullptr;
}

std::istream *BytecodeFile::get_new_stream() {
    std::ifstream *f = new std::ifstream(this->path, std::ios_base::binary);
    if(f->fail()){
        error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
    }
    return f;
}

std::ostream *BytecodeFile::create_out_stream() {
    std::ofstream *f = new std::ofstream(this->path, std::ios_base::binary);
    if (f->fail()) {
        error::error(error::ErrorCode::FILE_ACCESS, std::strerror(errno), this, true);
    }
    LOGMAX("Created new output bytecode file: " << this->path);
    return f;
}

#ifdef __windows__
#include <windows.h>
#include <shlobj.h>

ustring moss::get_local_app_data_path() {
    char path[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);  // For %LOCALAPPDATA% (Roaming)
    return ustring(path);
}
#endif

// TODO: Add sys.path (which will have MOSSPATH prepended on startup)
std::optional<ustring> moss::get_file_path(ustring file) {
    auto filep = std::filesystem::path(file);
    // Look in current directory
    if (std::filesystem::exists(global_controls::pwd / filep))
        return (global_controls::pwd / filep).string();

    // See if there is MOSSPATH and look there
    if (const char* value = std::getenv("MOSSPATH")) {
        std::vector<ustring> paths;
        // On linux and mac the convention for separator is :, on windows it is ;
#if defined(__linux__) || defined(__APPLE_)
        paths = utils::split_csv(value, ':');
#elif defined(__windows__)
        paths = utils::split_csv(value, ';');
#endif
        for (auto p: paths) {
            if (std::filesystem::exists(std::filesystem::path(p) / filep)) {
                return (std::filesystem::path(p) / filep).string();
            }
        }
    }

    // Look into system path
#ifdef __linux__
    if (std::filesystem::exists(std::filesystem::path("/lib/moss") / filep)) {
        return (std::filesystem::path("/lib/moss") / filep).string();
    }
#elif defined(__APPLE__)
    if (std::filesystem::exists(std::filesystem::path("/usr/local/lib/moss") / filep)) {
        return (std::filesystem::path("/usr/local/lib/moss") / filep).string();
    }
#elif defined(__windows__)
    static std::filesystem::path LIB_PATH = std::filesystem::path(get_local_app_data_path()+"/moss");
    if (std::filesystem::exists(LIB_PATH / filep)) {
        return (LIB_PATH / filep).string();
    }
#endif

    return std::nullopt;
}