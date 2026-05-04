#include "hash_cache.h"
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// RollingHash::compute
// ─────────────────────────────────────────────────────────────────────────────
uint64_t RollingHash::compute(const std::string& s) {
    uint64_t hash = 0;
    for (unsigned char c : s)
        hash = (hash * BASE + c) % MOD;
    return hash;
}

// ─────────────────────────────────────────────────────────────────────────────
// HashCache  –  constructor
// ─────────────────────────────────────────────────────────────────────────────
HashCache::HashCache(size_t slots) : count_(0) {
    // Round up to next power of two so bitmask modulo works
    size_t s = 1;
    while (s < slots) s <<= 1;
    slots_ = s;
    table_.resize(slots_);           // all entries default to valid=false
}

// ─────────────────────────────────────────────────────────────────────────────
// HashCache::lookup
// ─────────────────────────────────────────────────────────────────────────────
const CacheEntry* HashCache::lookup(const std::string& pattern) const {
    uint64_t h    = RollingHash::compute(pattern);
    size_t   slot = h & (slots_ - 1);

    // Linear probe until we find the pattern or an empty slot
    for (size_t i = 0; i < slots_; i++) {
        size_t idx = (slot + i) & (slots_ - 1);
        const CacheEntry& e = table_[idx];
        if (!e.valid)          return nullptr;   // empty slot = not cached
        if (e.pattern == pattern) return &e;     // found
        // else: collision, keep probing
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// HashCache::store
// ─────────────────────────────────────────────────────────────────────────────
void HashCache::store(const std::string& pattern, const std::vector<size_t>& positions) {
    // Rehash if load factor exceeds 0.7
    if (count_ * 10 >= slots_ * 7) rehash();

    uint64_t h    = RollingHash::compute(pattern);
    size_t   slot = h & (slots_ - 1);

    for (size_t i = 0; i < slots_; i++) {
        size_t idx = (slot + i) & (slots_ - 1);
        CacheEntry& e = table_[idx];

        if (!e.valid) {
            // Empty slot: insert here
            e.pattern   = pattern;
            e.positions = positions;
            e.valid     = true;
            count_++;
            return;
        }
        if (e.pattern == pattern) return;   // already cached, no-op
        // else: collision, probe next slot
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// HashCache::rehash  –  doubles the table and reinserts all entries
// ─────────────────────────────────────────────────────────────────────────────
void HashCache::rehash() {
    size_t new_slots = slots_ * 2;
    std::vector<CacheEntry> new_table(new_slots);

    for (const CacheEntry& e : table_) {
        if (!e.valid) continue;
        uint64_t h    = RollingHash::compute(e.pattern);
        size_t   slot = h & (new_slots - 1);
        for (size_t i = 0; i < new_slots; i++) {
            size_t idx = (slot + i) & (new_slots - 1);
            if (!new_table[idx].valid) {
                new_table[idx] = e;
                break;
            }
        }
    }

    slots_ = new_slots;
    table_ = std::move(new_table);
}