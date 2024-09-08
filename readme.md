# Moss language

Moss is a modern interpreted language with simple syntax and allows for note creation inside of the source code, these notes can be then used to generate different outputs, such as pdf files or tables with data computed with moss.

## Building Moss

Moss can be built and run on Linux and Windows.
On Mac it should be possible as well, but it was not yet tested.

__Requirements:__
* C++17 compatible compiler (GCC recommended)
* CMake

__Build:__
```shell
git clone --recursive https://github.com/mark-sed/moss-lang.git
cd moss-lang
cmake -S . -B build
cmake --build build -j8

./build/moss --version
```