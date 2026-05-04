#pragma once
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// RollingHash
//   Rabin-Karp style polynomial rolling hash over a string.
//   hash(s) = s[0]*BASE^(m-1) + s[1]*BASE^(m-2) + ... + s[m-1]  (mod MOD)
//   Computing hash of a new string is O(m).
//   Intended use: hash a pattern string once before cache lookup/store.
// ─────────────────────────────────────────────────────────────────────────────
class RollingHash {
public:
    static constexpr uint64_t BASE = 131;        // prime > alphabet size
    static constexpr uint64_t MOD  = 1000000007ULL;

    // Compute hash of an entire string in O(m)
    static uint64_t compute(const std::string& s);
};

// ─────────────────────────────────────────────────────────────────────────────
// CacheEntry  –  what we store per cached pattern
// ─────────────────────────────────────────────────────────────────────────────
struct CacheEntry {
    std::string          pattern;    // stored for collision resolution
    std::vector<size_t>  positions;
    bool                 valid = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// HashCache
//   Custom open-addressing hash table.
//   Keys   : pattern strings  (hashed via RollingHash)
//   Values : CacheEntry (positions vector)
//   Collision resolution : linear probing
//   No std::unordered_map used anywhere.
// ─────────────────────────────────────────────────────────────────────────────
class HashCache {
public:
    // slots must be a power of two for fast modulo via bitmask
    explicit HashCache(size_t slots = 256);

    // Returns pointer to entry if pattern is cached, nullptr otherwise
    const CacheEntry* lookup(const std::string& pattern) const;

    // Stores pattern → positions.  No-op if already present.
    void store(const std::string& pattern, const std::vector<size_t>& positions);

    size_t size()     const { return count_; }
    size_t capacity() const { return slots_; }

private:
    size_t               slots_;      // always power of two
    size_t               count_;      // number of stored entries
    std::vector<CacheEntry> table_;   // flat array, open addressing

    size_t slot_for(uint64_t hash) const { return hash & (slots_ - 1); }
    void   rehash();                  // doubles table when load > 0.7
};