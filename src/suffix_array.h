#pragma once
#include <vector>
#include <string>
#include <cstddef>

struct SuffixArray {
    std::vector<size_t> array;   
    const char*         text;
    size_t              length;
};

SuffixArray* sa_build(const char* text, size_t length);

std::vector<size_t> sa_search(const SuffixArray* sa, const std::string& pattern);

void sa_free(SuffixArray* sa);
