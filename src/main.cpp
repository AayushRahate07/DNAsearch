#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "input.h"
#include "output.h"
#include "mmap_reader.h"
#include "suffix_array.h"
#include "segment_tree.h"
#include "hash_cache.h"

// ─────────────────────────────────────────────────────────────────────────────
// reverse_complement
//   DNA is double-stranded. For every search we also search the complement.
//   Complement mapping: A<->T, C<->G
//   Then reverse the whole string (3' to 5' direction).
//   Example: ACGT -> complement: TGCA -> reverse: ACGT
//            ATGC -> complement: TACG -> reverse: GCAT
// ─────────────────────────────────────────────────────────────────────────────
static std::string reverse_complement(const std::string& pattern) {
    std::string rc = pattern;
    for (char& c : rc) {
        switch (toupper(c)) {
            case 'A': c = 'T'; break;
            case 'T': c = 'A'; break;
            case 'C': c = 'G'; break;
            case 'G': c = 'C'; break;
            default:  break;
        }
    }
    std::reverse(rc.begin(), rc.end());
    return rc;
}

// ─────────────────────────────────────────────────────────────────────────────
// validate_pattern  –  warn if non-nucleotide chars present
// ─────────────────────────────────────────────────────────────────────────────
static void validate_pattern(const std::string& pattern) {
    for (char c : pattern) {
        char u = toupper(c);
        if (u != 'A' && u != 'C' && u != 'G' && u != 'T') {
            std::cout << "  Warning: '" << c
                      << "' is not a standard nucleotide (A/C/G/T).\n";
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <fasta_file> [pattern] [range_lo range_hi]\n";
        std::cerr << "  fasta_file : path to genome file (.fasta)\n";
        std::cerr << "  pattern    : (optional) nucleotide pattern\n";
        std::cerr << "  range_lo/hi: (optional) restrict search to genome positions lo..hi\n";
        return EXIT_FAILURE;
    }

    // ── Open genome file via memory-mapped I/O ────────────────────────────
    std::string filepath = argv[1];
    MmapHandle* handle = mmap_open(filepath);
    if (!handle) {
        std::cerr << "Error: could not open '" << filepath << "'\n";
        return EXIT_FAILURE;
    }

    std::cout << "=== DNA Sequence Search System ===\n";
    std::cout << "File  : " << filepath << "\n";
    std::cout << "Size  : " << handle->size << " bytes\n\n";

    // ── Build Suffix Array index ──────────────────────────────────────────
    std::cout << "Building suffix array index...\n";
    SuffixArray* sa = sa_build(handle->data, handle->size);
    if (!sa) {
        std::cerr << "Error: failed to build suffix array\n";
        mmap_close(handle);
        return EXIT_FAILURE;
    }
    std::cout << "Index ready. " << sa->length << " suffixes indexed.\n\n";

    // ── Build Segment Tree over genome range ─────────────────────────────
    std::cout << "Building segment tree for range queries...\n";
    SegmentTree seg_tree(handle->size);
    std::cout << "Segment tree ready.\n\n";

    // ── Initialise custom hash cache ──────────────────────────────────────
    // 256 initial slots, uses rolling hash internally, no unordered_map
    HashCache cache(256);

    // ── Core search lambda ────────────────────────────────────────────────
    auto do_search = [&](const std::string& pattern, bool is_complement,
                         bool use_range, size_t range_lo, size_t range_hi)
                         -> std::vector<size_t>
    {
        // Build a unique cache key: pattern + range (if any)
        std::string cache_key = pattern;
        if (use_range)
            cache_key += "|" + std::to_string(range_lo) + "-" + std::to_string(range_hi);

        // ── Cache lookup ──────────────────────────────────────────────────
        const CacheEntry* cached = cache.lookup(cache_key);
        if (cached) {
            std::cout << "  [cache hit]\n";
            OutputOptions opts{ is_complement, use_range, range_lo, range_hi };
            output_results(cached->positions, pattern, opts);
            return cached->positions;
        }

        // ── Suffix Array search → candidate positions ─────────────────────
        std::vector<size_t> positions = sa_search(sa, pattern);

        // ── Segment Tree range filter (if range requested) ────────────────
        if (use_range && !positions.empty())
            positions = seg_tree.query_range(positions, range_lo, range_hi);

        // ── Cache and output ──────────────────────────────────────────────
        cache.store(cache_key, positions);
        OutputOptions opts{ is_complement, use_range, range_lo, range_hi };
        output_results(positions, pattern, opts);
        return positions;
    };

    // ── Full search: original + complement strand ─────────────────────────
    auto full_search = [&](const QueryInput& q) {
        validate_pattern(q.pattern);

        std::cout << "\nSearching for: \"" << q.pattern << "\"\n";
        if (q.use_range)
            std::cout << "Range restricted: " << q.range_lo << " to " << q.range_hi << "\n";
        std::cout << "─────────────────────────────────────\n";

        // Original strand
        auto orig_positions = do_search(q.pattern, false,
                                        q.use_range, q.range_lo, q.range_hi);

        // Complement strand
        std::string rc = reverse_complement(q.pattern);
        std::vector<size_t> comp_positions;
        if (rc != q.pattern) {   // skip if pattern is its own complement
            comp_positions = do_search(rc, true,
                                       q.use_range, q.range_lo, q.range_hi);
        } else {
            std::cout << "  (pattern is self-complementary, skipping duplicate search)\n";
        }

        // Summary
        output_summary(q.pattern, orig_positions.size(), comp_positions.size());
    };

    // ── Command-line single query mode ────────────────────────────────────
    if (argc >= 3) {
        QueryInput q;
        q.pattern = argv[2];
        if (argc >= 5) {
            q.use_range = true;
            q.range_lo  = static_cast<size_t>(std::atoll(argv[3]));
            q.range_hi  = static_cast<size_t>(std::atoll(argv[4]));
        }
        full_search(q);
    } else {
        // ── Interactive mode ──────────────────────────────────────────────
        std::cout << "Enter pattern [optional: range_lo range_hi], or 'quit' to exit\n";
        std::cout << "Examples:\n";
        std::cout << "  ACGT\n";
        std::cout << "  ACGT 1000 5000\n\n";

        QueryInput q;
        while (true) {
            std::cout << "> ";
            if (!input_read_query(q)) break;
            if (q.pattern.empty()) continue;
            full_search(q);
        }
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    sa_free(sa);
    mmap_close(handle);
    std::cout << "\nBye.\n";
    return EXIT_SUCCESS;
}