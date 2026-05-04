cmake_minimum_required(VERSION 3.15)
project(DNASearchSystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    message(STATUS "Platform: Windows")
    add_compile_definitions(PLATFORM_WINDOWS)
else()
    message(STATUS "Platform: Unix/macOS")
    add_compile_definitions(PLATFORM_UNIX)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# ── COMMON SOURCES (added for consistency) ──────────────────────────────────
set(COMMON_SOURCES
    src/mmap_reader.cpp
    src/suffix_array.cpp
    src/hash_cache.cpp
    src/segment_tree.cpp
)

# ── MAIN EXECUTABLE ─────────────────────────────────────────────────────────
add_executable(dna_search
    src/main.cpp
    src/input.cpp
    src/output.cpp
    ${COMMON_SOURCES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(dna_search m)
endif()

# ── TESTS ───────────────────────────────────────────────────────────────────
set(TEST_SHARED_SOURCES
    src/input.cpp
    src/output.cpp
    ${COMMON_SOURCES}
)

add_executable(test_suffix_array tests/test_suffix_array.cpp ${TEST_SHARED_SOURCES})
add_executable(test_cache        tests/test_cache.cpp        ${TEST_SHARED_SOURCES})

enable_testing()
add_test(NAME SuffixArray COMMAND test_suffix_array)
add_test(NAME Cache       COMMAND test_cache)

# ── DNA HTTP SERVER ─────────────────────────────────────────────────────────
add_executable(dna_server
    src/server.cpp
    ${COMMON_SOURCES}
)

target_include_directories(dna_server PRIVATE src)

if(WIN32)
    target_link_libraries(dna_server ws2_32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(dna_server m)
endif()