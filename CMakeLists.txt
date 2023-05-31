cmake_minimum_required(VERSION 3.22)
project(zelda64 C)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

# Library.
add_library(zelda64 SHARED
        lib/util.h
        lib/rom.c lib/rom.h
        lib/dma.c lib/dma.h
        lib/yaz0.c lib/yaz0.h
        lib/crc32.c lib/crc32.h
        lib/decompressor.c lib/decompressor.h)
set_target_properties(zelda64 PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

# Binary based on the library.
add_executable(zelda64-bin src/main.c)
set_target_properties(zelda64-bin PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)
target_link_libraries(zelda64-bin
        PRIVATE zelda64
        PRIVATE ZLIB::ZLIB)
