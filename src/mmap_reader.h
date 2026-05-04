#pragma once
#include <cstddef>
#include <string>

struct MmapHandle {
    const char* data = nullptr;
    size_t      size = 0;
    void*      _impl = nullptr;
};

MmapHandle* mmap_open(const std::string& filepath);

void mmap_close(MmapHandle* handle);
