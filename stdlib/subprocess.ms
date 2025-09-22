d"""
This module allows to create processes, get their output
and return codes.
"""

@internal
fun system(cmd:String) {
    d"""
    Runs a system command and returns its return code.
    For capturing output see `run` function.
    """
}

class SubprocessResult {
    d"""
    Results from running a subprocess. Contains the following:
      - command: the command executed,
      - return_code: code returned by the subprocess,
      - stdout: standard output from the subprocess,
      - stderr: standard error output from the subprocess.
    """

    fun SubprocessResult(command:String, return_code:Int, stdout:String, stderr:String) {
        this.command = command
        this.return_code = return_code
        this.stdout = stdout
        this.stderr = stderr
    }
}

class SubprocessError : Exception {
    d"Exception for when subprocess run fails internally."

    fun SubprocessError(msg, command) {
        this.msg = msg
        this.command = command
    }

    fun __String() {
        return f"SubprocessError: Error when running '{this.command}': {this.msg}.\n"
    }
}

// TODO: Add option to read bytes and timeout
@internal
fun run(command:String, combine_streams:Bool=false, capture_output:Bool=true) {
    d"""
    Runs a system command.
    @param command Command to execute.
    @param combine_streams When true, then stdout and stderr will be merged.
    @param capture_output When true, then output will be captured in SubprocessResult.
    """
}