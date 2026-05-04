#pragma once
#include <vector>
#include <string>
#include <cstddef>

struct OutputOptions {
    bool   is_complement  = false;   // true if these are complement strand results
    bool   use_range      = false;   // true if search was range-restricted
    size_t range_lo       = 0;
    size_t range_hi       = 0;
};

// Print results for one pattern (original or complement strand)
void output_results(const std::vector<size_t>& positions,
                    const std::string& pattern,
                    const OutputOptions& opts = {});

// Print combined summary after both strands are searched
void output_summary(const std::string& original_pattern,
                    size_t original_count,
                    size_t complement_count);