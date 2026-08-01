#include "source.hpp"
#include "errors.hpp"
#include "logging.hpp"
#include <cstring>
#include <fstream>
#include <filesystem>
#ifdef __windows__
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

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

ustring moss::get_moss_lib_path() {
#ifdef __linux__
    return "/usr/lib/moss/";
#elif defined(__APPLE__)
    return "/usr/local/lib/moss/";
#elif defined(__windows__)
    return moss::get_local_app_data_path()+"\\moss\\";
#endif
}

std::optional<std::filesystem::path> get_executable_path() {
#ifdef __windows__
    std::vector<wchar_t> buffer(1024);

    while (true) {
        DWORD size = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size())
        );

        if (size == 0) {
            return std::nullopt;
        }

        // Buffer was too small
        if (size < buffer.size() - 1) {
            return std::filesystem::path(buffer.data());
        }

        buffer.resize(buffer.size() * 2);
    }
#elif defined(__linux__)
    std::vector<char> buffer(PATH_MAX);

    while (true) {
        ssize_t size = readlink(
            "/proc/self/exe",
            buffer.data(),
            buffer.size()
        );

        if (size == -1) {
            return std::nullopt;
        }

        // Buffer was too small
        if (static_cast<size_t>(size) < buffer.size()) {
            return std::filesystem::path(
                std::string(buffer.data(), size)
            );
        }

        buffer.resize(buffer.size() * 2);
    }
#elif defined(__APPLE__)
    uint32_t size = 0;

    _NSGetExecutablePath(nullptr, &size);

    std::vector<char> buffer(size);

    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return std::nullopt;
    }

    // Resolve symlinks and normalize the path
    return std::filesystem::canonical(buffer.data());
#endif
    return std::nullopt;
}

std::optional<std::filesystem::path> get_executable_directory() {
    auto p = get_executable_path();
    if (p)
        return p->parent_path();
    return std::nullopt;
}

static std::vector<ustring> paths;

static void init_lookup_path() {
    // See if there is MOSSPATH
    if (const char* value = std::getenv("MOSSPATH")) {
        // On linux and mac the convention for separator is :, on windows it is ;
#if defined(__linux__) || defined(__APPLE_)
        paths = utils::split_csv(value, ':');
#elif defined(__windows__)
        paths = utils::split_csv(value, ';');
#endif
    }
    // Prepend this dir "./"
    paths.insert(paths.begin(), "");

    // Look into system path
    paths.push_back(get_moss_lib_path());

    // Append path to the current exectuable
    if (auto edp = get_executable_directory())
        paths.push_back(edp->string());
}

std::vector<ustring> &moss::get_lookup_path() {
    static bool initialized = false;
    if (!initialized) {
        init_lookup_path();
        initialized = true;
    }
    return paths;
}

// TODO: Add sys.path (which will have MOSSPATH prepended on startup)
std::optional<ustring> moss::get_file_path(ustring file) {
    auto filep = std::filesystem::path(file);
    // Look in current directory
    if (std::filesystem::exists(global_controls::pwd / filep))
        return (global_controls::pwd / filep).string();

    for (auto p: get_lookup_path()) {
        std::filesystem::path base = p;
        if (p.empty())
            base = global_controls::pwd;
        if (std::filesystem::exists(base / filep)) {
            return (base / filep).string();
        }
    }

    return std::nullopt;
}