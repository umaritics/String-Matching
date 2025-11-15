#include <iostream>
#include <fstream>
#include <sstream>
#include<vector>
#include<unordered_map>
#include<cstdint>

using namespace std;

/*
Divide & Conquer Longest Common Substring (LCS) using rolling hashes.
- Uses double hashing (two moduli) to reduce collision probability.
- Recursively splits both strings; in merge step it finds the best substring
  that crosses a midpoint (either in A or in B) by binary searching substring length
  and checking hashes in O(lenA + lenB).
- Base-case: small ranges handled by brute-force substring scan.
*/

// -----------------------------
// RollingHash - double-hash
// -----------------------------
struct RollingHash {
    static const long long MOD1 = 1000000007LL;
    static const long long MOD2 = 1000000009LL;

    vector<long long> h1, h2;    // prefix hashes
    vector<long long> p1, p2;    // powers of base

    RollingHash() = default;

    // Build rolling hash for string s with chosen base
    void build(const string &s, long long baseIn = 911382323) {
        int n = (int)s.size();
        h1.assign(n + 1, 0);
        h2.assign(n + 1, 0);
        p1.assign(n + 1, 0);
        p2.assign(n + 1, 0);
        p1[0] = p2[0] = 1;

        // use two different smaller bases
        long long b1 = 911382323 % MOD1;
        long long b2 = 972663749 % MOD2;

        for (int i = 0; i < n; ++i) {
            h1[i + 1] = ( (h1[i] * b1) + (unsigned char)s[i] + 1 ) % MOD1;
            h2[i + 1] = ( (h2[i] * b2) + (unsigned char)s[i] + 1 ) % MOD2;
            p1[i + 1] = (p1[i] * b1) % MOD1;
            p2[i + 1] = (p2[i] * b2) % MOD2;
        }
    }

    // Get double-hash for substring s[l..r] inclusive (0-indexed, indices relative to the string used for build)
    pair<long long,long long> getHash(int l, int r) const {
        if (l > r) return {0,0};
        // safety guard
        if (l < 0) l = 0;
        if (r + 1 > (int)h1.size()-1) r = (int)h1.size()-2;
        long long x1 = h1[r+1];
        long long y1 = (h1[l] * p1[r - l + 1]) % MOD1;
        long long res1 = (x1 + MOD1 - y1) % MOD1;
        long long x2 = h2[r+1];
        long long y2 = (h2[l] * p2[r - l + 1]) % MOD2;
        long long res2 = (x2 + MOD2 - y2) % MOD2;
        return {res1, res2};
    }
};

// -----------------------------
// Helper: brute-force LCS for small ranges (safe & exact)
// -----------------------------
string bruteLCS(const string &A, int aL, int aR, const string &B, int bL, int bR) {
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (la <= 0 || lb <= 0) return "";
    string best = "";
    for (int i = aL; i <= aR; ++i) {
        for (int j = bL; j <= bR; ++j) {
            int k = 0;
            while (i + k <= aR && j + k <= bR && A[i + k] == B[j + k]) ++k;
            if (k > (int)best.size()) best = A.substr(i, k);
        }
    }
    return best;
}

// Helper pack - combine two 32-bit-ish hashes into one 64-bit key
static inline uint64_t packHash(const pair<long long,long long> &h) {
    uint64_t a = (uint64_t)(uint32_t)h.first;
    uint64_t b = (uint64_t)(uint32_t)h.second;
    return (a << 32) | b;
}

// -----------------------------
// crossCheckA, crossCheckB (unchanged logic) but use packHash
// -----------------------------
string crossCheckA(const string &A, int aL, int aR, int midA,
                   const string &B, int bL, int bR,
                   const RollingHash &hashA, const RollingHash &hashB,
                   int L)
{
    if (L <= 0) return "";
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (L > la || L > lb) return "";

    unordered_map<uint64_t, vector<int>> mp;
    mp.reserve(max(8, lb - L + 1));

    for (int j = bL; j + L - 1 <= bR; ++j) {
        auto h = hashB.getHash(j, j + L - 1);
        mp[packHash(h)].push_back(j);
    }

    int s_min = max(aL, midA - L + 1);
    int s_max = min(midA, aR - L + 1);
    for (int s = s_min; s <= s_max; ++s) {
        int e = s + L - 1;
        if (e > aR) continue;
        if (!(s <= midA && e >= midA + 1)) continue;
        auto hA = hashA.getHash(s, e);
        uint64_t key = packHash(hA);
        auto it = mp.find(key);
        if (it == mp.end()) continue;
        for (int posB : it->second) {
            bool ok = true;
            for (int k = 0; k < L; ++k) {
                if (A[s + k] != B[posB + k]) { ok = false; break; }
            }
            if (ok) return A.substr(s, L);
        }
    }
    return "";
}

string crossCheckB(const string &A, int aL, int aR,
                   const string &B, int bL, int bR, int midB,
                   const RollingHash &hashA, const RollingHash &hashB,
                   int L)
{
    if (L <= 0) return "";
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (L > la || L > lb) return "";

    unordered_map<uint64_t, vector<int>> mp;
    mp.reserve(max(8, la - L + 1));
    for (int i = aL; i + L - 1 <= aR; ++i) {
        auto h = hashA.getHash(i, i + L - 1);
        mp[packHash(h)].push_back(i);
    }

    int s_min = max(bL, midB - L + 1);
    int s_max = min(midB, bR - L + 1);
    for (int s = s_min; s <= s_max; ++s) {
        int e = s + L - 1;
        if (e > bR) continue;
        if (!(s <= midB && e >= midB + 1)) continue;
        auto hB = hashB.getHash(s, e);
        uint64_t key = packHash(hB);
        auto it = mp.find(key);
        if (it == mp.end()) continue;
        for (int posA : it->second) {
            bool ok = true;
            for (int k = 0; k < L; ++k) {
                if (A[posA + k] != B[s + k]) { ok = false; break; }
            }
            if (ok) return B.substr(s, L);
        }
    }
    return "";
}

// -----------------------------
// maxCrossA / maxCrossB unchanged
// -----------------------------
string maxCrossA(const string &A, int aL, int aR, int midA,
                 const string &B, int bL, int bR,
                 const RollingHash &hashA, const RollingHash &hashB)
{
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (la <= 0 || lb <= 0) return "";
    int lo = 1, hi = min(la, lb);
    string best = "";
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        string cand = crossCheckA(A, aL, aR, midA, B, bL, bR, hashA, hashB, mid);
        if (!cand.empty()) {
            if ((int)cand.size() > (int)best.size()) best = cand;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return best;
}

string maxCrossB(const string &A, int aL, int aR,
                 const string &B, int bL, int bR, int midB,
                 const RollingHash &hashA, const RollingHash &hashB)
{
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (la <= 0 || lb <= 0) return "";
    int lo = 1, hi = min(la, lb);
    string best = "";
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        string cand = crossCheckB(A, aL, aR, B, bL, bR, midB, hashA, hashB, mid);
        if (!cand.empty()) {
            if ((int)cand.size() > (int)best.size()) best = cand;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return best;
}

// -----------------------------
// FIXED: crossCheckBothMidpoints now uses relative indices when invoking hashes
// -----------------------------
string crossCheckBothMidpoints(const string &A, int aL, int aR, int midA,
                              const string &B, int bL, int bR, int midB,
                              const RollingHash &hashA, const RollingHash &hashB) {
    string best = "";

    int max_possible_len = min(aR - aL + 1, bR - bL + 1);
    int search_range = min(100, min(midA - aL + 1, midB - bL + 1));
    for (int startA = max(aL, midA - search_range); startA <= midA; ++startA) {
        for (int startB = max(bL, midB - search_range); startB <= midB; ++startB) {
            int max_len = min(aR - startA + 1, bR - startB + 1);
            int len = 0;
            int lo = 1, hi = max_len;
            while (lo <= hi) {
                int mid_len = (lo + hi) / 2;

                // Convert absolute indices -> relative to the slices used when hashes were built
                int relA_l = startA - aL;
                int relA_r = relA_l + mid_len - 1;
                int relB_l = startB - bL;
                int relB_r = relB_l + mid_len - 1;

                // safety: if these are out of range for hashes, reduce hi
                if (relA_l < 0 || relA_r >= (int)hashA.h1.size() - 1 ||
                    relB_l < 0 || relB_r >= (int)hashB.h1.size() - 1) {
                    hi = mid_len - 1;
                    continue;
                }

                auto hash1 = hashA.getHash(relA_l, relA_r);
                auto hash2 = hashB.getHash(relB_l, relB_r);

                if (hash1 == hash2) {
                    bool match = true;
                    for (int k = 0; k < mid_len && match; ++k) {
                        if (A[startA + k] != B[startB + k]) match = false;
                    }
                    if (match) {
                        len = mid_len;
                        lo = mid_len + 1;
                    } else {
                        hi = mid_len - 1;
                    }
                } else {
                    hi = mid_len - 1;
                }
            }

            if (len > (int)best.size() &&
                startA <= midA && startA + len - 1 >= midA &&
                startB <= midB && startB + len - 1 >= midB) {
                best = A.substr(startA, len);
            }
        }
    }

    return best;
}

// -----------------------------
// Main recursive divide-and-conquer LCS
// -----------------------------
string lcs_divide_conquer(const string &A, int aL, int aR,
                          const string &B, int bL, int bR, int depth = 0)
{
    int la = aR - aL + 1;
    int lb = bR - bL + 1;
    if (la <= 0 || lb <= 0) return "";

    const int BRUTE_THRESHOLD = 120;
    if (la <= BRUTE_THRESHOLD || lb <= BRUTE_THRESHOLD) {
        return bruteLCS(A, aL, aR, B, bL, bR);
    }

    // Build hashes on the slices (these hashes expect indices relative to 0..slice_len-1)
    string subA = A.substr(aL, la);
    string subB = B.substr(bL, lb);
    RollingHash absHashA, absHashB;
    absHashA.build(subA);
    absHashB.build(subB);

    int midA = (aL + aR) >> 1;
    int midB = (bL + bR) >> 1;

    string left = lcs_divide_conquer(A, aL, midA, B, bL, midB);
    string right = lcs_divide_conquer(A, midA + 1, aR, B, midB + 1, bR);

    int midA_rel = midA - aL;
    int midB_rel = midB - bL;

    string crossA = maxCrossA(subA, 0, la - 1, midA_rel, subB, 0, lb - 1, absHashA, absHashB);
    string crossB = maxCrossB(subA, 0, la - 1, subB, 0, lb - 1, midB_rel, absHashA, absHashB);
    string crossBoth = crossCheckBothMidpoints(A, aL, aR, midA, B, bL, bR, midB, absHashA, absHashB);

    string ans = left;
    if ((int)right.size() > (int)ans.size()) ans = right;
    if ((int)crossA.size() > (int)ans.size()) ans = crossA;
    if ((int)crossB.size() > (int)ans.size()) ans = crossB;
    if ((int)crossBoth.size() > (int)ans.size()) ans = crossBoth;

    // if (depth == 0) {
    //     cout << "Debug - Left: '" << left << "' (len=" << left.size() << ")\n";
    //     cout << "Debug - Right: '" << right << "' (len=" << right.size() << ")\n";
    //     cout << "Debug - CrossA: '" << crossA << "' (len=" << crossA.size() << ")\n";
    //     cout << "Debug - CrossB: '" << crossB << "' (len=" << crossB.size() << ")\n";
    // }

    return ans;
}

// verifyLCS (unchanged)
string verifyLCS(const string &A, const string &B) {
    string best = "";
    for (int i = 0; i < (int)A.length(); i++) {
        for (int j = 0; j < (int)B.length(); j++) {
            int k = 0;
            while (i + k < (int)A.length() && j + k < (int)B.length() && A[i + k] == B[j + k]) {
                k++;
            }
            if (k > (int)best.length()) {
                best = A.substr(i, k);
            }
        }
    }
    return best;
}

string longestCommonSubstring(const string &A, const string &B) {
    if (A.empty() || B.empty()) return "";
    return lcs_divide_conquer(A, 0, (int)A.size() - 1, B, 0, (int)B.size() - 1);
}

// -----------------------------
// Main (file reading) - unchanged
// -----------------------------
int main() {
    string fileA, fileB;

    cout << "Enter first file name (with extension): ";
    getline(cin, fileA);

    cout << "Enter second file name (with extension): ";
    getline(cin, fileB);

    ifstream fa(fileA, ios::binary);
    if (!fa.is_open()) { cout << "Error: Could not open file '" << fileA << "'\n"; return 0; }
    string A((istreambuf_iterator<char>(fa)), istreambuf_iterator<char>());
    fa.close();

    ifstream fb(fileB, ios::binary);
    if (!fb.is_open()) { cout << "Error: Could not open file '" << fileB << "'\n"; return 0; }
    string B((istreambuf_iterator<char>(fb)), istreambuf_iterator<char>());
    fb.close();

    if (A.empty() || B.empty()) {
        cout << "One of the files is empty. No common substring.\n";
        return 0;
    }

    cout << "\nComputing longest common substring (divide & conquer + rolling hash)...\n";
    string lcs = longestCommonSubstring(A, B);

    if (lcs.empty()) cout << "No common substring found.\n";
    else {
        cout << "Longest common substring (length = " << lcs.size() << "):\n";
        cout <<'\''<< lcs<<'\'' << "\n";
    }

    return 0;
}