#include <iostream>
#include <cassert>
#include <cstring>
#include "../src/suffix_array.h"
#include "../src/segment_tree.h"
#include "../src/hash_cache.h"

void test_suffix_to_segment_filter() {
    const char* genome = "AAATGCAAATGCTTTTGCAAATGC";
    size_t len = strlen(genome);

    SuffixArray* sa = sa_build(genome, len);
    SegmentTree seg(len);

    auto all_positions = sa_search(sa, "ATG");
    assert(all_positions.size() == 3);

    auto filtered = seg.query_range(all_positions, 0, 10);
    assert(filtered.size() == 2);
    for (size_t pos : filtered) {
        assert(pos <= 10);
    }

    std::cout << "  [PASS] suffix array -> segment tree filter correct\n";
    sa_free(sa);
}

void test_cache_hit_returns_same_positions() {
    const char* genome = "ATGATGATG";
    size_t len = strlen(genome);

    SuffixArray* sa = sa_build(genome, len);
    HashCache cache(16);

    auto positions = sa_search(sa, "ATG");
    assert(positions.size() == 3);

    cache.store("ATG", positions);

    const CacheEntry* hit = cache.lookup("ATG");
    assert(hit != nullptr);
    assert(hit->positions.size() == positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
        assert(hit->positions[i] == positions[i]);
    }

    std::cout << "  [PASS] cached result matches suffix array result\n";
    sa_free(sa);
}

void test_range_then_cache() {
    const char* genome = "GCATGCATGCAT";
    size_t len = strlen(genome);

    SuffixArray* sa = sa_build(genome, len);
    SegmentTree seg(len);
    HashCache cache(16);

    auto all_pos = sa_search(sa, "ATG");
    auto filtered = seg.query_range(all_pos, 2, 8);

    std::string cache_key = "ATG|2-8";
    cache.store(cache_key, filtered);

    const CacheEntry* hit = cache.lookup(cache_key);
    assert(hit != nullptr);
    for (size_t pos : hit->positions) {
        assert(pos >= 2 && pos <= 8);
    }

    std::cout << "  [PASS] range-filtered results stored and retrieved from cache\n";
    sa_free(sa);
}

void test_pattern_not_found_full_flow() {
    const char* genome = "AAACCCTTTGGG";
    size_t len = strlen(genome);

    SuffixArray* sa = sa_build(genome, len);
    SegmentTree seg(len);
    HashCache cache(16);

    auto positions = sa_search(sa, "TTTT");
    assert(positions.empty());

    auto filtered = seg.query_range(positions, 0, len - 1);
    assert(filtered.empty());

    cache.store("TTTT", filtered);
    const CacheEntry* hit = cache.lookup("TTTT");
    assert(hit != nullptr);
    assert(hit->positions.empty());

    std::cout << "  [PASS] not-found pattern flows through all layers correctly\n";
    sa_free(sa);
}

void test_cache_miss_triggers_search() {
    const char* genome = "CGATCGATCG";
    size_t len = strlen(genome);

    SuffixArray* sa = sa_build(genome, len);
    HashCache cache(16);

    const CacheEntry* miss = cache.lookup("GAT");
    assert(miss == nullptr);

    auto positions = sa_search(sa, "GAT");
    assert(!positions.empty());

    cache.store("GAT", positions);
    const CacheEntry* hit = cache.lookup("GAT");
    assert(hit != nullptr);
    assert(hit->positions.size() == positions.size());

    std::cout << "  [PASS] cache miss triggers search, result then cached\n";
    sa_free(sa);
}

int main() {
    std::cout << "=== Integration Tests ===\n";
    test_suffix_to_segment_filter();
    test_cache_hit_returns_same_positions();
    test_range_then_cache();
    test_pattern_not_found_full_flow();
    test_cache_miss_triggers_search();
    std::cout << "All Integration tests passed.\n";
    return 0;
}