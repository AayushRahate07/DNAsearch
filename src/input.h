#pragma once
#include <string>
#include <cstddef>

// ─────────────────────────────────────────────────────────────────────────────
// QueryInput  –  everything the user provides for one search
// ─────────────────────────────────────────────────────────────────────────────
struct QueryInput {
    std::string pattern;

    // Range query: if use_range is true, search only within [range_lo, range_hi]
    bool   use_range  = false;
    size_t range_lo   = 0;
    size_t range_hi   = 0;
};

// Reads a pattern + optional range from stdin.
// Format accepted:
//   ACGT                      (whole genome search)
//   ACGT 1000 5000            (range search: positions 1000 to 5000)
// Returns false on EOF or "quit".
bool input_read_query(QueryInput& out);