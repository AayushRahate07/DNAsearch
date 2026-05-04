#include "input.h"
#include <iostream>
#include <sstream>

bool input_read_query(QueryInput& out) {
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    if (line == "quit" || line == "q") return false;

    std::istringstream iss(line);
    out = QueryInput{};                  // reset

    if (!(iss >> out.pattern)) return false;

    // Try reading optional range: pattern LO HI
    size_t lo, hi;
    if (iss >> lo >> hi) {
        out.use_range = true;
        out.range_lo  = lo;
        out.range_hi  = hi;
    }

    return !out.pattern.empty();
}