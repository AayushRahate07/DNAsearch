cmake_minimum_required(VERSION 3.10)
project(DNASearch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(src)

set(SOURCES
    src/main.cpp
    src/kmp.cpp
    src/bm.cpp
    src/mmap_reader.cpp
    src/suffix_array.cpp
    src/segment_tree.cpp
    src/hash_cache.cpp
    src/input.cpp
    src/output.cpp
    ${COMMON_SOURCES}
)