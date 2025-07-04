#include "subprocess.hpp"
#include "opcode.hpp"
#include <cstdlib>

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

#ifndef __windows__
std::pair<int, std::string> run_command(const std::string& cmd, Value *&err) {
    std::string result;
    constexpr std::size_t buffer_size = 4096;
    char buffer[buffer_size];

    // Run command with stderr redirected to stdout
    std::string full_cmd = cmd + " 2>&1";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
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
#ifdef __windows__
    err = create_not_implemented_error("Function subprocess.run is not yet implemented for Windows systems.\n");
    return nullptr;
#else
    if (!combine_b->get_value()) {
        // FIXME: run without combined
        err = create_not_implemented_error("Subprocess.run option for non combined output (combine_output=false) is not yet implemented.\n");
        return nullptr;
    } else {
        auto [code, result] = run_command(cmds->get_value(), err);
        code_v = new IntValue(code);
        stdout_v = new StringValue(result);
    }
#endif
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