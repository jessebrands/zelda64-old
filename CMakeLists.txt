cmake_minimum_required(VERSION 3.22)
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
        lib/crc32.c include/zelda64/crc32.h
        lib/zpf.c include/zelda64/zpf.h)
set_target_properties(zelda64 PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
target_include_directories(zelda64 PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Binary based on the library.
add_executable(zelda64-bin
        src/main.c
        src/decompressor.c src/decompressor.h
        src/patcher.c src/patcher.h)
set_target_properties(zelda64-bin PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)
target_link_libraries(zelda64-bin
        PRIVATE zelda64
        PRIVATE ZLIB::ZLIB)
target_include_directories(zelda64-bin PRIVATE ${CMAKE_SOURCE_DIR}/include)
