#pragma once
#include <vector>
#include <cstddef>

// ─────────────────────────────────────────────────────────────────────────────
// SegmentTree
//
//   Stores the genome as a range structure.
//   Each leaf node covers one byte position in the genome file.
//   Each internal node stores the min and max position of its range.
//
//   Primary use: given a user query range [range_start, range_end],
//   efficiently collect all suffix-array candidate positions that
//   fall within that genome region.
//
//   query_range(positions, lo, hi)
//     - Takes a list of candidate positions from the suffix array
//     - Returns only those that fall within [lo, hi]
//     - O(k log n) where k = number of candidates
//
//   This means the suffix array finds WHERE the pattern occurs globally,
//   and the segment tree filters to only the region the user cares about.
// ─────────────────────────────────────────────────────────────────────────────

struct SegmentNode {
    size_t range_min;   // smallest genome position in this node's subtree
    size_t range_max;   // largest  genome position in this node's subtree
};

class SegmentTree {
public:
    // Build from genome length — O(n)
    explicit SegmentTree(size_t genome_length);

    // Filter candidates: return only those within [lo, hi]
    std::vector<size_t> query_range(const std::vector<size_t>& candidates,
                                    size_t lo, size_t hi) const;

    size_t genome_length() const { return n_; }

private:
    size_t n_;                        // genome length
    std::vector<SegmentNode> tree_;   // 1-indexed, size = 4*n

    void build(size_t node, size_t start, size_t end);

    // Returns true if this node's range overlaps [lo, hi]
    bool overlaps(size_t node, size_t lo, size_t hi) const;
};