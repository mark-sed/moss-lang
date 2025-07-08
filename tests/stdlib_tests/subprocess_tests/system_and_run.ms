import subprocess

rcode = subprocess.system("echo hi!")
assert(rcode == 0);

result = subprocess.run("echo hello?", combine_streams=true)
assert(result.command == "echo hello?", "not saved command")
assert(result.return_code == 0, "incorrect return code")
assert(result.stdout.strip() == "hello?", "output differs")
assert(result.stderr == "", "err output differs")

result2 = subprocess.run("echo_non_existent_command", combine_streams=true)
assert(result2.command == "echo_non_existent_command", "not saved command")
assert(result2.return_code != 0, "incorrect return code")
assert(result2.stdout != "", "output differs")
assert(result2.stderr == "", "err output differs")

result3 = subprocess.run("echo no_cap", combine_streams=true, capture_output=false)
assert(result3.command == "echo no_cap", "not saved command")
assert(result3.return_code == 0, "incorrect return code")
assert(result3.stdout == "", "output differs")
assert(result3.stderr == "", "err output differs")