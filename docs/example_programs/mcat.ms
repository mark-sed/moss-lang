/*
#!/usr/bin/moss
*/

fun format_number(n:Int) {
    s = String(n)
    padding = [" " : _ = s.length()..6]
    return "".join(padding) ++ s ++ "  "
}

parsing_files = false
show_ends = false
number = false
line_number = 1

if ("--help" in args) {
    """Usage: mcat [OPTIONS]... [FILE]...
Concatenate FILE(s) to standard output.

  -E, --show-ends          display $ at end of each line
  -n, --number             number all output lines
"""
    exit(0)
}

for (a: args) {
    if (not parsing_files) {
        switch (a) {
            case "-E", "--show-ends": show_ends = true
            case "-n", "--number": number = true
            default: parsing_files = true
        }
    }
    if (parsing_files) {
        try {
            f = open(a, "r")
        } catch (e: FileNotFoundError) {
            ~print(f"mcat: {a}: No such file or directory")
            continue
        }
        c = f.readlines()
        first = true
        for (l: c) {
            if (number) {
                format_number(line_number)
                line_number += 1
            }
            l
            if (show_ends) {
                "$"
            }
            // TODO: output only if it is present in the file
            "\n"
        }
        ~f.close()
    }
}

if (not parsing_files) {
    // TODO: Implement
    exit("Reading STDIN is not yet implemented")    
}