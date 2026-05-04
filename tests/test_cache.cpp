#include <iostream>
#include <cassert>
#include "../src/hash_cache.h"

void test_rolling_hash_same_input() {
    uint64_t h1 = RollingHash::compute("ATGC");
    uint64_t h2 = RollingHash::compute("ATGC");
    assert(h1 == h2);
    std::cout << "  [PASS] same input produces same hash\n";
}

void test_rolling_hash_different_input() {
    uint64_t h1 = RollingHash::compute("ATGC");
    uint64_t h2 = RollingHash::compute("CGTA");
    assert(h1 != h2);
    std::cout << "  [PASS] different inputs produce different hashes\n";
}

void test_store_and_lookup() {
    HashCache cache(16);
    std::vector<size_t> positions = {102, 580, 4210};
    cache.store("ATGCGT", positions);

    const CacheEntry* r = cache.lookup("ATGCGT");
    assert(r != nullptr);
    assert(r->positions.size() == 3);
    assert(r->positions[0] == 102);
    assert(r->positions[1] == 580);
    assert(r->positions[2] == 4210);
    std::cout << "  [PASS] stored pattern retrieved with correct positions\n";
}

void test_lookup_miss() {
    HashCache cache(16);
    const CacheEntry* miss = cache.lookup("TTTTTT");
    assert(miss == nullptr);
    std::cout << "  [PASS] uncached pattern returns nullptr\n";
}

void test_duplicate_store_is_noop() {
    HashCache cache(16);
    std::vector<size_t> pos1 = {10, 20};
    std::vector<size_t> pos2 = {99};
    cache.store("AAAA", pos1);
    cache.store("AAAA", pos2);

    const CacheEntry* r = cache.lookup("AAAA");
    assert(r != nullptr);
    assert(r->positions.size() == 2);
    std::cout << "  [PASS] duplicate store is no-op, original retained\n";
}

void test_multiple_patterns() {
    HashCache cache(16);
    cache.store("ACGT", {1, 2});
    cache.store("TGCA", {3, 4});
    cache.store("GGGG", {5});

    assert(cache.lookup("ACGT") != nullptr);
    assert(cache.lookup("TGCA") != nullptr);
    assert(cache.lookup("GGGG") != nullptr);
    assert(cache.lookup("CCCC") == nullptr);
    assert(cache.size() == 3);
    std::cout << "  [PASS] multiple patterns stored and retrieved correctly\n";
}

void test_rehash_on_high_load() {
    HashCache cache(4);
    for (int i = 0; i < 20; i++) {
        std::string pat = "PAT" + std::to_string(i);
        cache.store(pat, {static_cast<size_t>(i * 10)});
    }
    for (int i = 0; i < 20; i++) {
        std::string pat = "PAT" + std::to_string(i);
        const CacheEntry* r = cache.lookup(pat);
        assert(r != nullptr);
        assert(r->positions[0] == static_cast<size_t>(i * 10));
    }
    std::cout << "  [PASS] all entries survive rehash under high load\n";
}

void test_range_key_caching() {
    HashCache cache(16);
    std::string key = "ATGC|1000-5000";
    cache.store(key, {1500, 2000});
    const CacheEntry* r = cache.lookup(key);
    assert(r != nullptr);
    assert(r->positions.size() == 2);
    std::cout << "  [PASS] range-qualified cache key stored and retrieved\n";
}

int main() {
    std::cout << "=== Hash Cache Tests ===\n";
    test_rolling_hash_same_input();
    test_rolling_hash_different_input();
    test_store_and_lookup();
    test_lookup_miss();
    test_duplicate_store_is_noop();
    test_multiple_patterns();
    test_rehash_on_high_load();
    test_range_key_caching();
    std::cout << "All Cache tests passed.\n";
    return 0;
}