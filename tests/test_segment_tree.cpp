#include <iostream>
#include <cassert>
#include "../src/segment_tree.h"

void test_all_in_range() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {10, 20, 30, 40, 50};
    auto result = seg.query_range(candidates, 0, 99);
    assert(result.size() == 5);
    std::cout << "  [PASS] all candidates within full range returned\n";
}

void test_partial_filter() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {5, 25, 50, 75, 95};
    auto result = seg.query_range(candidates, 20, 80);
    assert(result.size() == 3);
    for (size_t pos : result) {
        assert(pos >= 20 && pos <= 80);
    }
    std::cout << "  [PASS] positions outside range correctly excluded\n";
}

void test_none_in_range() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {0, 5, 10};
    auto result = seg.query_range(candidates, 50, 90);
    assert(result.empty());
    std::cout << "  [PASS] no candidates in range returns empty\n";
}

void test_exact_boundary() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {20, 21, 79, 80};
    auto result = seg.query_range(candidates, 20, 80);
    assert(result.size() == 4);
    std::cout << "  [PASS] boundary positions 20 and 80 included\n";
}

void test_single_position_range() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {42, 43, 44};
    auto result = seg.query_range(candidates, 43, 43);
    assert(result.size() == 1);
    assert(result[0] == 43);
    std::cout << "  [PASS] single-position range returns exactly one match\n";
}

void test_empty_candidates() {
    SegmentTree seg(100);
    std::vector<size_t> candidates = {};
    auto result = seg.query_range(candidates, 0, 99);
    assert(result.empty());
    std::cout << "  [PASS] empty candidates vector returns empty result\n";
}

void test_genome_length() {
    SegmentTree seg(500);
    assert(seg.genome_length() == 500);
    std::cout << "  [PASS] genome_length() returns correct value\n";
}

int main() {
    std::cout << "=== Segment Tree Tests ===\n";
    test_all_in_range();
    test_partial_filter();
    test_none_in_range();
    test_exact_boundary();
    test_single_position_range();
    test_empty_candidates();
    test_genome_length();
    std::cout << "All Segment Tree tests passed.\n";
    return 0;
}