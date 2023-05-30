cmake_minimum_required(VERSION 3.22)
project(zelda64 C)

set(CMAKE_C_STANDARD 17)

add_executable(zelda64 src/main.c lib/rom.c lib/rom.h lib/dma.c lib/dma.h lib/util.h lib/yaz0.c lib/yaz0.h)
