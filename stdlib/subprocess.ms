d"""
This module allows to create processes, get their output
and return codes.
"""

@internal
fun system(cmd:String) {}

class SubprocessResult {
    fun SubprocessResult(command:String, return_code:Int, stdout:String, stderr:String) {
        this.command = command
        this.return_code = return_code
        this.stdout = stdout
        this.stderr = stderr
    }
}

// TODO: Add option to read bytes and timeout
@internal
fun run(command:String, combine_streams:Bool=false, capture_output:Bool=true) {}