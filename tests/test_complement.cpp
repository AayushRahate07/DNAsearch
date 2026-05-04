#include <iostream>
#include <cassert>
#include <string>
#include <algorithm>
#include <cctype>
#include "../src/suffix_array.h"

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

void test_basic_complement() {
    assert(reverse_complement("ACGT") == "ACGT");
    std::cout << "  [PASS] ACGT is self-complementary\n";
}

void test_atgc_complement() {
    assert(reverse_complement("ATGC") == "GCAT");
    std::cout << "  [PASS] ATGC reverse complement is GCAT\n";
}

void test_single_base() {
    assert(reverse_complement("A") == "T");
    assert(reverse_complement("T") == "A");
    assert(reverse_complement("C") == "G");
    assert(reverse_complement("G") == "C");
    std::cout << "  [PASS] single base complements correct\n";
}

void test_complement_of_complement() {
    std::string original = "ATGCGTA";
    std::string rc = reverse_complement(original);
    std::string rc_rc = reverse_complement(rc);
    assert(rc_rc == original);
    std::cout << "  [PASS] double reverse complement returns original\n";
}

void test_complement_searchable_in_suffix_array() {
    const char* genome = "ATGCGCAT";
    SuffixArray* sa = sa_build(genome, 8);
    assert(sa != nullptr);

    std::string pattern = "ATGC";
    std::string rc = reverse_complement(pattern);

    auto orig = sa_search(sa, pattern);
    auto comp = sa_search(sa, rc);

    assert(!orig.empty());
    assert(!comp.empty());
    std::cout << "  [PASS] both original and complement strand found in genome\n";

    sa_free(sa);
}

void test_self_complementary_skip() {
    std::string pattern = "ACGT";
    std::string rc = reverse_complement(pattern);
    assert(rc == pattern);
    std::cout << "  [PASS] self-complementary pattern correctly detected (rc == pattern)\n";
}

int main() {
    std::cout << "=== Reverse Complement Tests ===\n";
    test_basic_complement();
    test_atgc_complement();
    test_single_base();
    test_complement_of_complement();
    test_complement_searchable_in_suffix_array();
    test_self_complementary_skip();
    std::cout << "All Complement tests passed.\n";
    return 0;
}