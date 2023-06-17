cmake_minimum_required(VERSION 3.21)
project(zelda64 C)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

# Library.
add_library(zelda64 SHARED
        lib/util.h
        lib/rom.c include/zelda64/rom.h
        lib/dma.c include/zelda64/dma.h
        lib/yaz0.c include/zelda64/yaz0.h
        lib/crc32.c include/zelda64/crc32.h)

set_target_properties(zelda64 PROPERTIES
        C_STANDARD 23
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_include_directories(zelda64 PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Binary based on the library.
add_executable(zelda64-bin src/main.c)

set_target_properties(zelda64-bin PROPERTIES
        C_STANDARD 23
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

target_link_libraries(zelda64-bin
        PRIVATE zelda64)
target_include_directories(zelda64-bin PRIVATE ${CMAKE_SOURCE_DIR}/include)
