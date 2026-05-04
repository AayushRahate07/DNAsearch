#include "output.h"
#include <iostream>

void output_results(const std::vector<size_t>& positions,
                    const std::string& pattern,
                    const OutputOptions& opts) {

    std::string strand_label = opts.is_complement ? " [complement strand]" : " [original strand]";

    if (opts.use_range) {
        std::cout << "  Range: positions " << opts.range_lo
                  << " to " << opts.range_hi << "\n";
    }

    if (positions.empty()) {
        std::cout << "  Pattern \"" << pattern << "\"" << strand_label
                  << " : NOT FOUND\n";
        return;
    }

    std::cout << "  Pattern \"" << pattern << "\"" << strand_label
              << " : " << positions.size() << " match(es)\n";

    for (size_t i = 0; i < positions.size(); i++)
        std::cout << "    [" << i + 1 << "] byte offset: " << positions[i] << "\n";
}

void output_summary(const std::string& original_pattern,
                    size_t original_count,
                    size_t complement_count) {
    size_t total = original_count + complement_count;
    std::cout << "\n  ── Summary for \"" << original_pattern << "\" ──\n";
    std::cout << "  Original strand  : " << original_count  << " match(es)\n";
    std::cout << "  Complement strand: " << complement_count << " match(es)\n";
    std::cout << "  Total frequency  : " << total << " match(es)\n";
    std::cout << "  ─────────────────────────────────────\n";
}