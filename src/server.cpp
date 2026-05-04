#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <chrono>
#include <cstring>
#include <algorithm>

#include "suffix_array.h"
#include "hash_cache.h"
#include "segment_tree.h"

// ── Windows / Linux sockets ───────────────────────────────────────────────
#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
  using sock_t = SOCKET;
  #define CLOSE_SOCK(s) closesocket(s)
  #define SOCK_ERR INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  using sock_t = int;
  #define CLOSE_SOCK(s) close(s)
  #define SOCK_ERR (-1)
#endif

static const int PORT = 8080;

// ─────────────────────────────────────────────────────────────────────────
// Reverse complement
// ─────────────────────────────────────────────────────────────────────────
static std::string reverse_complement(const std::string& pattern) {
    std::string rc = pattern;
    for (char& c : rc) {
        switch (toupper(c)) {
            case 'A': c = 'T'; break;
            case 'T': c = 'A'; break;
            case 'C': c = 'G'; break;
            case 'G': c = 'C'; break;
        }
    }
    std::reverse(rc.begin(), rc.end());
    return rc;
}

// ─────────────────────────────────────────────────────────────────────────
// Load DNA (FASTA safe)
// ─────────────────────────────────────────────────────────────────────────
static std::string loadDNA(const std::string& path) {
    std::ifstream f(path);
    std::string seq, line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '>' || line[0] == ';') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        seq += line;
    }
    return seq;
}

// ─────────────────────────────────────────────────────────────────────────
// URL decode
// ─────────────────────────────────────────────────────────────────────────
static std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            out += static_cast<char>(strtol(hex, nullptr, 16));
            i += 2;
        } else if (s[i] == '+') out += ' ';
        else out += s[i];
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────
// Query param parser
// ─────────────────────────────────────────────────────────────────────────
static std::string parseParam(const std::string& req, const std::string& key) {
    const std::string k1 = "?" + key + "=";
    const std::string k2 = "&" + key + "=";

    size_t pos = req.find(k1);
    if (pos == std::string::npos) pos = req.find(k2);
    if (pos == std::string::npos) return "";

    pos += key.size() + 2;
    size_t end = req.find_first_of("& \r\n", pos);

    return urlDecode(req.substr(pos, end - pos));
}

// ─────────────────────────────────────────────────────────────────────────
// JSON builder
// ─────────────────────────────────────────────────────────────────────────
static std::string buildJSON(const std::string& pattern,
                             const std::vector<size_t>& pos,
                             double ms,
                             bool hit) {
    std::ostringstream j;
    j << "{";
    j << "\"pattern\":\"" << pattern << "\",";
    j << "\"matches\":" << pos.size() << ",";
    j << "\"time_ms\":" << ms << ",";
    j << "\"cache\":\"" << (hit ? "HIT" : "MISS") << "\",";
    j << "\"positions\":[";
    for (size_t i = 0; i < pos.size(); i++) {
        if (i) j << ",";
        j << pos[i];
    }
    j << "]}";
    return j.str();
}

// ─────────────────────────────────────────────────────────────────────────
// HTTP response
// ─────────────────────────────────────────────────────────────────────────
static void sendResponse(sock_t c, const std::string& body) {
    std::ostringstream r;
    r << "HTTP/1.1 200 OK\r\n"
      << "Content-Type: application/json\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Access-Control-Allow-Origin: *\r\n"
      << "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
      << "\r\n"
      << body;

    std::string s = r.str();
    send(c, s.c_str(), (int)s.size(), 0);
}

// ─────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {

    std::string file = (argc > 1) ? argv[1] : "genome.fasta";

    std::cout << "Loading DNA...\n";
    std::string dna = loadDNA(file);

    std::cout << "Building suffix array...\n";
    SuffixArray* sa = sa_build(dna.c_str(), dna.size());

    SegmentTree seg_tree(dna.size());
    HashCache cache(1024);

#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(srv, (sockaddr*)&addr, sizeof(addr));
    listen(srv, 10);

    std::cout << "Server running → http://localhost:8080\n";

    while (true) {
        sock_t client = accept(srv, nullptr, nullptr);

        char buf[8192] = {};
        recv(client, buf, sizeof(buf), 0);
        std::string req(buf);

        if (req.substr(0, 7) == "OPTIONS") {
            sendResponse(client, "{}");
        }

        else if (req.find("GET /sequence") == 0) {
            std::string json = "{\"sequence\":\"" + dna.substr(0, 5000) +
                               "\",\"length\":" + std::to_string(dna.size()) + "}";
            sendResponse(client, json);
        }

        else if (req.find("GET /search") == 0) {

            std::string pattern = parseParam(req, "q");
            std::string lo_s = parseParam(req, "lo");
            std::string hi_s = parseParam(req, "hi");

            bool use_range = !lo_s.empty() && !hi_s.empty();
            size_t lo = use_range ? std::stoull(lo_s) : 0;
            size_t hi = use_range ? std::stoull(hi_s) : 0;

            std::string key = pattern;
            if (use_range)
                key += "|" + std::to_string(lo) + "-" + std::to_string(hi);

            auto t0 = std::chrono::high_resolution_clock::now();

            std::vector<size_t> positions;
            bool hit = false;

            const CacheEntry* cached = cache.lookup(key);
            if (cached) {
                positions = cached->positions;
                hit = true;
            } else {
                positions = sa_search(sa, pattern);

                if (use_range)
                    positions = seg_tree.query_range(positions, lo, hi);

                std::string rc = reverse_complement(pattern);
                if (rc != pattern) {
                    auto comp = sa_search(sa, rc);
                    if (use_range)
                        comp = seg_tree.query_range(comp, lo, hi);
                    positions.insert(positions.end(), comp.begin(), comp.end());
                }

                cache.store(key, positions);
            }

            double ms = std::chrono::duration<double, std::milli>(
                std::chrono::high_resolution_clock::now() - t0).count();

            sendResponse(client, buildJSON(pattern, positions, ms, hit));
        }

        else {
            sendResponse(client, "{\"error\":\"not found\"}");
        }

        CLOSE_SOCK(client);
    }

    sa_free(sa);
}