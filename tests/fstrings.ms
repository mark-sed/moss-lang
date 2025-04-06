f"v: { 0x20 + 0xA + 1 } end\n";

name = "moss user"
f"name = {name}\n"

f""
f"\n"
f"{{}\n"

f"{f"{"hi"}"}\n"

fun wholoveswhat(name, lang) {
    return f"{name} loves coding in {lang}.\n"
}
name = "Jon"
lang = "Moss"
wholoveswhat(name, lang)

// todo: fix
//f"{}"

name = "Bob"
assert(f"Hello, {name}" == "Hello, Bob")

a, b = [3, 4]
f"{a} + {b} = {a + b}\n"

value = 10
~print(f"{f"Value: {value}"}!")

f"Greet: {(fun(name) = name++"!")("Marek")}\n"

data = {"key": "value"}
f"dict = {data["key"]}\n"

// todo: fix
//f"""{{Hello}}
//there {name}\n"""

title = "Title"
p = "lorem ipsum"
f"""# {title}
{p}.
"""