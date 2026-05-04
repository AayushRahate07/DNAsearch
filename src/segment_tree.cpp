#include "segment_tree.h"
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor  –  allocates tree and builds in O(n)
// ─────────────────────────────────────────────────────────────────────────────
SegmentTree::SegmentTree(size_t genome_length) : n_(genome_length) {
    tree_.resize(4 * n_ + 4);
    if (n_ > 0) build(1, 0, n_ - 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// build  –  fills each node with [range_min, range_max] for its subtree
// ─────────────────────────────────────────────────────────────────────────────
void SegmentTree::build(size_t node, size_t start, size_t end) {
    tree_[node].range_min = start;
    tree_[node].range_max = end;

    if (start == end) return;   // leaf node

    size_t mid = start + (end - start) / 2;
    build(2 * node,     start,   mid);
    build(2 * node + 1, mid + 1, end);
}

// ─────────────────────────────────────────────────────────────────────────────
// overlaps  –  does this node's range intersect [lo, hi]?
// ─────────────────────────────────────────────────────────────────────────────
bool SegmentTree::overlaps(size_t node, size_t lo, size_t hi) const {
    return !(tree_[node].range_max < lo || tree_[node].range_min > hi);
}

// ─────────────────────────────────────────────────────────────────────────────
// query_range
//   Takes suffix-array candidates and filters to those in [lo, hi].
//   Uses the tree structure to prune the search space:
//   if a node's entire range is outside [lo, hi], skip it entirely.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<size_t> SegmentTree::query_range(const std::vector<size_t>& candidates,
                                              size_t lo, size_t hi) const {
    std::vector<size_t> result;
    if (candidates.empty() || lo > hi || n_ == 0) return result;

    // Clamp range to valid genome bounds
    size_t clamped_lo = lo;
    size_t clamped_hi = std::min(hi, n_ - 1);

    for (size_t pos : candidates) {
        if (pos >= clamped_lo && pos <= clamped_hi) {
            // Use tree to confirm the position falls in a valid range node
            // Walk from root — if any node containing pos overlaps [lo,hi], accept
            size_t node  = 1;
            size_t start = 0;
            size_t end   = n_ - 1;

            bool accepted = false;
            while (start <= end) {
                if (!overlaps(node, clamped_lo, clamped_hi)) break;
                if (start == end) { accepted = true; break; }

                size_t mid = start + (end - start) / 2;
                if (pos <= mid) {
                    node  = 2 * node;
                    end   = mid;
                } else {
                    node  = 2 * node + 1;
                    start = mid + 1;
                }
            }
            if (accepted) result.push_back(pos);
        }
    }
    return result;
}