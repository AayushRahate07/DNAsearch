#include "suffix_array.h"
#include <cstring>
#include <vector>
#include <string>

void manual_quick_sort(std::vector<size_t>& arr, int left, int right, const char* text) {
    int i = left, j = right;
    size_t pivot_index = arr[left + (right - left) / 2];

    while (i <= j) {
        while (strcmp(text + arr[i], text + pivot_index) < 0) i++;
        while (strcmp(text + arr[j], text + pivot_index) > 0) j--;

        if (i <= j) {
            size_t temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
            j--;
        }
    }

    if (left < j) manual_quick_sort(arr, left, j, text);
    if (i < right) manual_quick_sort(arr, i, right, text);
}

SuffixArray* sa_build(const char* text, size_t length) {
    if (!text || length == 0) return nullptr;
    auto* sa = new SuffixArray();
    sa->text   = text;
    sa->length = length;
    sa->array.resize(length);
    for (size_t i = 0; i < length; i++) sa->array[i] = i;

    if (length > 0) {
        manual_quick_sort(sa->array, 0, (int)length - 1, text);
    }
    return sa;
}

static int prefix_cmp(const SuffixArray* sa, size_t idx, const std::string& pattern) {
    return strncmp(sa->text + idx, pattern.c_str(), pattern.size());
}

std::vector<size_t> sa_search(const SuffixArray* sa, const std::string& pattern) {
    std::vector<size_t> result;
    if (!sa || pattern.empty()) return result;

    long lo = 0, hi = static_cast<long>(sa->length) - 1;
    long left = -1;
    { 
      long l = lo, h = hi;
      while (l <= h) {
          long mid = l + (h - l) / 2;
          int  cmp = prefix_cmp(sa, sa->array[mid], pattern);
          if (cmp == 0) { left = mid; h = mid - 1; }
          else if (cmp < 0) l = mid + 1;
          else              h = mid - 1;
      }
    }
    if (left < 0) return result;

    long right = -1;
    { 
      long l = lo, h = hi;
      while (l <= h) {
          long mid = l + (h - l) / 2;
          int  cmp = prefix_cmp(sa, sa->array[mid], pattern);
          if (cmp == 0) { right = mid; l = mid + 1; }
          else if (cmp < 0) l = mid + 1;
          else              h = mid - 1;
      }
    }

    for (long i = left; i <= right; i++)
        result.push_back(sa->array[i]);

    return result;
}

void sa_free(SuffixArray* sa) { 
    delete sa; 
}