Zelda64
=======

Zelda64 is a library and tool fo (de)compressing, patching, extracting, and verifying Nintendo 64 Zelda ROMs.

Building
--------

The library `libzelda64` is written in pure C17 with no dependencies at all. Simply build the `zelda64` target using
CMake with a modern C compiler. Zelda64 is actively checked to compile with GNU GCC, Apple Clang, and the Microsoft
Visual C/C++ Compiler.

To build the application `zelda64`, make sure that you `zlib` installed and on the header and library search paths. To
build the application simply build the `zelda64-bin` CMake target.

Library
-------

* `libzelda64` provides a modern C API to manipulate Nintendo 64 Zelda ROMs.

Tools
-----

* `zelda64` is a command line application that serves as a reference consumer of `libzelda64`.

License
--------

Zelda64 is entirely GPL-3.0 licensed. Please refer to the LICENSE file for detailed information.
