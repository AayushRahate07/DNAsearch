# DNA Sequence Search System

A fast, disk-based genomic search system written in C++.

## Architecture

| Component | Role |
|---|---|
| Suffix Array | Main index. Binary search finds exact matches in O(log n) per query |
| Segment Tree | Range filter. Restricts results to genome positions X–Y |
| Custom Hash Map + Rolling Hash | From-scratch cache. Rabin-Karp rolling hash as key function, O(1) lookup |
| Memory-Mapped I/O | Loads only accessed chunks of large genome files into RAM |
| Reverse Complement | Auto-applied to every query so both DNA strands are always searched |

## Build — Mac / Linux
```bash
mkdir build && cd build
cmake ..
make
```

## Build — Windows (MinGW)
```bat
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## Run
```bash
# Interactive mode
./bin/dna_search ../data/sample.fasta

# Single query — whole genome
./bin/dna_search ../data/sample.fasta ATGCGT

# Single query — range restricted (positions 1000 to 5000)
./bin/dna_search ../data/sample.fasta ATGCGT 1000 5000
```

## Run Tests
```bash
cd build
ctest --output-on-failure
```

## Search Flow
```
User types pattern + optional range (X to Y)
  → Reverse complement computed automatically
  → Segment Tree restricts candidate region if range given
  → Suffix Array + Binary Search finds exact match positions
  → Cache checked first — if hit, result returned instantly
  → If cache miss, search runs, result stored in custom hash map
  → Output: positions, strand label, frequency count
```

## Nucleotide Patterns to Try
| Pattern | Meaning |
|---|---|
| ATG | Start codon |
| TAA | Stop codon |
| ATGCGT | Custom gene fragment |
| ACGT | All four nucleotides (self-complementary) |
| ATG 0 120 | Start codon within first 120 positions |

## File Structure
```
src/
  main.cpp                 entry point, search orchestration
  suffix_array.cpp / .h    suffix array index + binary search
  segment_tree.cpp / .h    range query filter
  hash_cache.cpp / .h      custom hash map with rolling hash
  mmap_reader.cpp / .h     cross-platform memory-mapped file access
  input.cpp / .h           stdin parsing
  output.cpp / .h          result formatting
tests/
  test_suffix_array.cpp    suffix array build and search
  test_segment_tree.cpp    range filter correctness
  test_cache.cpp           rolling hash + custom hash map
  test_complement.cpp      reverse complement logic
  test_integration.cpp     full pipeline: suffix array → segment tree → cache
data/
  sample.fasta             3 sample DNA sequences in FASTA format
```