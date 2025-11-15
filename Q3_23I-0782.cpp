#include <bits/stdc++.h>
using namespace std;

/*
Aho-Corasick implementation with:
- case-insensitive matching (ASCII letters only)
- wildcard '?' expanded to printable ASCII (32..126)
- overlapping matches reported
- alphabet: printable ASCII 32..126 -> size 95
- patterns: provided by user
- text: provided by user
*/

// Alphabet mapping (printable ASCII)
static const int CH_L = 32;   // first printable
static const int CH_R = 126;  // last printable
static const int ALPH = CH_R - CH_L + 1; // 95

inline int ch_id(char c) {
    unsigned char uc = (unsigned char)c;
    if (uc < CH_L || uc > CH_R) return -1;
    return (int)(uc - CH_L);
}

inline char norm_char(char c) {
    // lowercase ASCII letters for case-insensitive match
    if (c >= 'A' && c <= 'Z') return char(c - 'A' + 'a');
    return c;
}

struct Node {
    // children indices (ALPH), -1 means absent
    vector<int> next;
    int link;       // failure link
    vector<int> out; // pattern ids that end here

    Node(): next(ALPH, -1), link(-1), out() {}
};

struct Aho {
    vector<Node> trie;
    Aho() { trie.emplace_back(); } // root

    void insert_pattern(const string &pat, int pid) {
        // insert pattern into trie; '?' expands to all printable chars
        // we assume pat is already normalized (lowercase)
        // This expansion keeps the automaton deterministic.
        // We'll insert paths for all expansions of '?' characters.
        // To avoid exponential blowup, implement iterative expansion using a vector of current nodes.

        vector<int> cur_nodes = {0}; // start at root
        for (char cc : pat) {
            char c = norm_char(cc);
            vector<int> next_nodes;
            if (c == '?') {
                // expand to all printable ASCII
                for (int node : cur_nodes) {
                    for (int ch = CH_L; ch <= CH_R; ++ch) {
                        int id = ch - CH_L;
                        int nxt = trie[node].next[id];
                        if (nxt == -1) {
                            // create
                            nxt = trie.size();
                            trie[node].next[id] = nxt;
                            trie.emplace_back();
                        }
                        next_nodes.push_back(nxt);
                    }
                }
            } else {
                int cid = ch_id(c);
                if (cid == -1) {
                    // character outside printable range, treat as literal by mapping via insertion if desired
                    // we'll skip such char (rare) — or create new branch keyed by c if you expect such chars.
                    for (int node : cur_nodes) {
                        next_nodes.push_back(node); // no movement
                    }
                } else {
                    for (int node : cur_nodes) {
                        int nxt = trie[node].next[cid];
                        if (nxt == -1) {
                            nxt = trie.size();
                            trie[node].next[cid] = nxt;
                            trie.emplace_back();
                        }
                        next_nodes.push_back(nxt);
                    }
                }
            }
            // deduplicate next_nodes to avoid explosion from repeated identical nodes
            sort(next_nodes.begin(), next_nodes.end());
            next_nodes.erase(unique(next_nodes.begin(), next_nodes.end()), next_nodes.end());
            cur_nodes.swap(next_nodes);
        }

        // mark all current nodes as output for pattern id
        for (int node : cur_nodes) {
            trie[node].out.push_back(pid);
        }
    }

    void build_links() {
        queue<int> q;
        trie[0].link = 0;
        // init root's children: link = 0
        for (int c = 0; c < ALPH; ++c) {
            int v = trie[0].next[c];
            if (v != -1) {
                trie[v].link = 0;
                q.push(v);
            } else {
                trie[0].next[c] = 0; // convenience: fill missing with root for deterministic go
            }
        }
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int c = 0; c < ALPH; ++c) {
                int u = trie[v].next[c];
                if (u != -1) {
                    int j = trie[v].link;
                    // follow failure links to find next
                    trie[u].link = trie[j].next[c];
                    // aggregate output
                    for (int pid : trie[trie[u].link].out)
                        trie[u].out.push_back(pid);
                    q.push(u);
                } else {
                    trie[v].next[c] = trie[trie[v].link].next[c];
                }
            }
        }
    }

    // search returns vector of (position_start, pattern_id)
    vector<pair<int,int>> search_all(const string &text) {
        vector<pair<int,int>> res;
        int v = 0;
        for (int i = 0; i < (int)text.size(); ++i) {
            char c = norm_char(text[i]);
            int cid = ch_id(c);
            if (cid == -1) {
                // non-printable or outside alphabet -> treat as root transition
                v = 0;
                continue;
            }
            v = trie[v].next[cid];
            if (!trie[v].out.empty()) {
                for (int pid : trie[v].out) {
                    // pattern length unknown here — to report start index we need pattern lengths externally
                    res.emplace_back(i, pid); // store match end index; caller can convert with pattern length
                }
            }
        }
        return res;
    }
};

// --------------------------
// Example usage (main)
// --------------------------
int main() {

    cout << "Enter number of patterns: ";
    int n;
    if (!(cin >> n)) return 0;
    string dummy;
    getline(cin, dummy); // consume newline

    vector<string> patterns;
    patterns.reserve(n);
    cout << "Enter patterns (each on its own line). Use '?' as wildcard (single char):\n";
    for (int i = 0; i < n; ++i) {
        string p;
        getline(cin, p);
        // convert to lowercase now (ASCII letters), store
        for (char &ch : p) ch = norm_char(ch);
        patterns.push_back(p);
    }

    cout << "Enter text (single line or press Enter then paste multi-line, finish with EOF Ctrl+D/Ctrl+Z):\n";
    string text;
    // Read remainder of stdin as text
    string all;
    while (true) {
        string line;
        if (!getline(cin, line)) break;
        if (!all.empty()) all.push_back('\n');
        all += line;
    }
    text = all;
    for (char &ch : text) ch = norm_char(ch);

    // Build automaton
    Aho aho;
    for (int i = 0; i < (int)patterns.size(); ++i) {
        aho.insert_pattern(patterns[i], i);
    }
    aho.build_links();

    // Search
    auto matches = aho.search_all(text);

    // To get start positions, we need pattern lengths (but note wildcards expand: original pattern length is the number of pattern chars)
    vector<int> patlen(n);
    for (int i = 0; i < n; ++i) patlen[i] = (int)patterns[i].size();

    // Print matches with start positions (0-based)
    cout << "\nMatches found (pattern_id, start_index, matched_text):\n";
    for (auto &m : matches) {
        int end_idx = m.first;
        int pid = m.second;
        int len = patlen[pid];
        int start = end_idx - len + 1;
        if (start < 0) continue;
        string matched = text.substr(start, len);
        cout << "(" << pid << ", " << start << "): '" << matched << "'  pattern='" << patterns[pid] << "'\n";
    }

    return 0;
}
