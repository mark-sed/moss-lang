import sys
import sys.Platform

if (args.empty())
    exit("Expected platform to be passed in ([Linux/Windows/Darwin]).")

switch(args[0].lower()) {
    case "linux": platform = Platform.Linux
    case "windows": platform = Platform.Windows
    case "darwin": platform = Platform.Darwin
    default: exit("Unknown platform (use [Linux/Windows/Darwin]).")
}

title = "# 🌿 Moss " ++ sys.version ++ " Release"
Note(title, "md")

md"""
More information can be found on the official [Github page](https://github.com/mark-sed/moss-lang/).
"""

md"""
## 📦 Requirements
Since Moss provides built-in Python 3 interoperability that means that Python 3
has to be installed on your machine. You can download it (here)[https://www.python.org/downloads/].
"""

md"""
## ⚙️ Installation
The binary from this release can be run as is, but the exact path will need to
be specified and libraries won't be on expected path. To have a proper Moss
installation please run the installation script provided, it will add Moss
to path so it's runnable as a command and copy libraries to correct paths:
"""

// Add installation instruction based on the release type
if (platform == Platform.Linux || platform == Platform.Darwin) {
    md"""
From inside of the release folder in your terminal run:
```
sudo bash install.sh
```
"""
} else if (platform == Platform.Windows) {
    md"""
From inside of the release folder in your command line run:
```
.\\install.ps1
```

Or just double click on the `install.ps1` file.
"""
}

md"""
Now if you run `moss --version` moss version should be printed.
"""