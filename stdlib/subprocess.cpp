#include "subprocess.hpp"
#include "opcode.hpp"
#include <cstdlib>
#include <vector>
#include <stdexcept>

#ifdef __windows__
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include <array>
#endif

using namespace moss;
using namespace mslib;
using namespace subprocess;

const std::unordered_map<std::string, mslib::mslib_dispatcher>& subprocess::get_registry() {
    static const std::unordered_map<std::string, mslib::mslib_dispatcher> registry = {
        {"system", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 1);
            return subprocess::system(vm, args[0].value, err);
        }},
        {"run", [](Interpreter* vm, CallFrame* cf, Value*& err) -> Value* {
            (void)err;
            auto args = cf->get_args();
            assert(args.size() == 3);
            return subprocess::run(vm, cf, cf->get_arg("command"), cf->get_arg("combine_streams"), cf->get_arg("capture_output"), err);
        }},
    };
    return registry;
}

Value *subprocess::system(Interpreter *vm, Value *cmd, Value *&err) {
    auto cmds = dyn_cast<StringValue>(cmd);
    assert(cmds && "Non-string value passed in");
    int result = std::system(cmds->get_value().c_str());
    return new IntValue(static_cast<opcode::IntConst>(result));
}

struct command_result {
    ustring stdout_text;
    ustring stderr_text;
    int exit_code;
};

#ifndef __windows__
std::pair<int, ustring> run_command(const ustring& cmd, Value *&err) {
    ustring result;
    constexpr std::size_t buffer_size = 4096;
    char buffer[buffer_size];

    // Run command with stderr redirected to stdout
    ustring full_cmd = cmd + " 2>&1";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
    if (!pipe) {
        // FIXME: Correct error
        err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
        return {-127, ""};
    }

    while (fgets(buffer, buffer_size, pipe.get()) != nullptr) {
        result += buffer;
    }

    int status = pclose(pipe.release());
    int exit_code = -1;

    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    }

    return std::make_pair(exit_code, result);
}

command_result run_command_separate(const ustring& command, bool combine_streams, Value *&err) {
    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) == -1) {
        // FIXME: Correct error
        err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
        return {"", "", -127};
    }

    if (!combine_streams && pipe(stderr_pipe) == -1) {
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        // FIXME: Correct error
        err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
        return {"", "", -127};
    }

    pid_t pid = fork();
    if (pid < 0) {
        // FIXME: Correct error
        err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
        return {"", "", -127};
    }

    if (pid == 0) {
        // Child process
        dup2(stdout_pipe[1], STDOUT_FILENO);

        if (combine_streams) {
            dup2(stdout_pipe[1], STDERR_FILENO);  // redirect stderr → stdout
        } else {
            dup2(stderr_pipe[1], STDERR_FILENO);  // separate stderr
        }

        // Close unused pipe ends
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        if (!combine_streams) {
            close(stderr_pipe[0]); close(stderr_pipe[1]);
        }

        // Execute via shell
        execl("/bin/sh", "sh", "-c", command.c_str(), (char*)nullptr);
        _exit(127); // only reached if exec fails
    }

    // Parent process
    close(stdout_pipe[1]);
    if (!combine_streams) close(stderr_pipe[1]);

    auto read_fd = [](int fd) -> std::string {
        std::string output;
        std::array<char, 4096> buffer;
        ssize_t count;
        while ((count = read(fd, buffer.data(), buffer.size())) > 0) {
            output.append(buffer.data(), count);
        }
        return output;
    };

    std::string stdout_output = read_fd(stdout_pipe[0]);
    std::string stderr_output;

    if (!combine_streams) {
        stderr_output = read_fd(stderr_pipe[0]);
        close(stderr_pipe[0]);
    }

    close(stdout_pipe[0]);

    int status;
    waitpid(pid, &status, 0);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    return {stdout_output, stderr_output, exit_code};
}


#else
command_result run_command_separate(const ustring& command, bool combine_streams, Value *&err) {
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    HANDLE stdout_read, stdout_write;
    HANDLE stderr_read, stderr_write;

    // Create pipes for stdout and stderr
    if (!combine_streams) {
        if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0) ||
                !CreatePipe(&stderr_read, &stderr_write, &sa, 0)) {
            // FIXME: Correct error
            err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
            return {"", "", -127};
        }
    } else {
        if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0)) {
            // FIXME: Correct error
            err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
            return {"", "", -127};
        }
    }

    // Prevent parent from inheriting the read ends
    SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);
    if (!combine_streams)
        SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.hStdOutput = stdout_write;
    if (!combine_streams)
        si.hStdError = stderr_write;
    else
        si.hStdError = stdout_write;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ustring cmd = "cmd.exe /C " + command;
    if (!CreateProcessA(
            NULL, &cmd[0], NULL, NULL, TRUE,
            0, NULL, NULL, &si, &pi)) {
        CloseHandle(stdout_read); CloseHandle(stdout_write);
        if (!combine_streams)
            CloseHandle(stderr_read); CloseHandle(stderr_write);
        // FIXME: Correct error
        err = create_not_implemented_error("Subprocess.run failed, exception not yet implemented.\n");
        return {"", "", -127};
    }

    // Close write ends in parent
    CloseHandle(stdout_write);
    if (!combine_streams)
        CloseHandle(stderr_write);

    auto read_all = [](HANDLE h) -> ustring {
        ustring result;
        char buffer[4096];
        DWORD read;
        while (ReadFile(h, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
            buffer[read] = '\0';
            result += buffer;
        }
        return result;
    };

    ustring stdout_output = read_all(stdout_read);
    ustring stderr_output;
    if (!combine_streams)
        stderr_output = read_all(stderr_read);

    CloseHandle(stdout_read);
    if (!combine_streams)
        CloseHandle(stderr_read);

    // Wait and get exit code
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = -1;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return { stdout_output, stderr_output, static_cast<int>(exit_code) };
}
#endif

Value *subprocess::run(Interpreter *vm, CallFrame *cf, Value *command, Value *comb_str, Value *capture_out, Value *&err) {
    auto cmds = dyn_cast<StringValue>(command);
    assert(cmds && "non-string");
    auto combine_b = dyn_cast<BoolValue>(comb_str);
    assert(combine_b && "non-bool");
    auto capture_out_b = dyn_cast<BoolValue>(capture_out);
    assert(capture_out_b && "non-bool");
    Value *code_v = nullptr;
    Value *stdout_v = nullptr;
    Value *stderr_v = nullptr;
    if (!capture_out_b->get_value()) {
        code_v = subprocess::system(vm, command, err);
    } else {
        auto cmd_res = run_command_separate(cmds->get_value(), combine_b->get_value(), err);
        code_v = new IntValue(cmd_res.exit_code);
        stdout_v = new StringValue(cmd_res.stdout_text);
        stderr_v = new StringValue(cmd_res.stderr_text);
    }

    if (!code_v) {
        code_v = new IntValue(-127);
    }
    if (!stdout_v) {
        stdout_v = new StringValue("");
    }
    if (!stderr_v) {
        stderr_v = new StringValue("");
    }

    auto funv = cf->get_function();
    assert(funv);
    auto fun = dyn_cast<FunValue>(funv);
    auto subres_class_v = fun->get_vm()->load_name("SubprocessResult");
    // TODO: Maybe also turn this into internal error? Or NameError?
    assert(subres_class_v && "Could not load SubprocessResult");
    auto subres_class = dyn_cast<ClassValue>(subres_class_v);
    assert(subres_class && "Not a class");
    diags::DiagID did = diags::DiagID::UNKNOWN;
    auto constr = opcode::lookup_method(vm, subres_class, "SubprocessResult", {command, code_v, stdout_v, stderr_v}, did);
    assert(constr && "No constructor for SubprocessResult");
    auto subres = opcode::runtime_constructor_call(vm, constr, {command, code_v, stdout_v, stderr_v}, subres_class);
    assert(subres && "sanity check");
    return subres;
}