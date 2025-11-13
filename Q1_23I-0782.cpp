#include <iostream>
#include <vector>
using namespace std;

// ---------------------------------------------------------
// Naïve String Search
// ---------------------------------------------------------
// Time Complexity: O(n * m)
// Space Complexity: O(1)
void naiveSearch(const string &text, const string &pattern) {
    int n = text.size();
    int m = pattern.size();
    bool found = false;

    for (int i = 0; i <= n - m; i++) {
        int j = 0;
        while (j < m && text[i + j] == pattern[j])
            j++;
        if (j == m) {
            cout << "Pattern found at index " << i << " using Naive Search\n";
            found = true;
        }
    }
    if (!found)
        cout << "Pattern not found using Naive Search\n";
}

// ---------------------------------------------------------
// KMP Algorithm
// ---------------------------------------------------------
// Preprocessing: O(m), Search: O(n)
vector<int> computeLPS(const string &pattern) {
    int m = pattern.size();
    vector<int> lps(m, 0);
    int len = 0, i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len])
            lps[i++] = ++len;
        else {
            if (len != 0)
                len = lps[len - 1];
            else
                lps[i++] = 0;
        }
    }
    return lps;
}

void KMPsearch(const string &text, const string &pattern) {
    int n = text.size(), m = pattern.size();
    vector<int> lps = computeLPS(pattern);
    int i = 0, j = 0;
    bool found = false;

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++; j++;
        }
        if (j == m) {
            cout << "Pattern found at index " << i - j << " using KMP\n";
            j = lps[j - 1];
            found = true;
        } else if (i < n && text[i] != pattern[j]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i++;
        }
    }
    if (!found)
        cout << "Pattern not found using KMP\n";
}

// ---------------------------------------------------------
// Rabin–Karp Algorithm
// ---------------------------------------------------------
// Average: O(n + m), Worst-case: O(n * m)
void rabinKarpSearch(const string &text, const string &pattern) {
    const int d = 256;  // number of possible characters
    const int q = 101;  // a prime number for hashing
    int n = text.size(), m = pattern.size();

    int p = 0, t = 0, h = 1;
    bool found = false;

    for (int i = 0; i < m - 1; i++)
        h = (h * d) % q;

    for (int i = 0; i < m; i++) {
        p = (d * p + pattern[i]) % q;
        t = (d * t + text[i]) % q;
    }

    for (int i = 0; i <= n - m; i++) {
        if (p == t) {
            bool match = true;
            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                cout << "Pattern found at index " << i << " using Rabin-Karp\n";
                found = true;
            }
        }
        if (i < n - m) {
            t = (d * (t - text[i] * h) + text[i + m]) % q;
            if (t < 0)
                t += q;
        }
    }
    if (!found)
        cout << "Pattern not found using Rabin-Karp\n";
}

// ---------------------------------------------------------
// Check if text is periodic (for adaptive switching)
// ---------------------------------------------------------
bool isPeriodic(const string& text) {
    int n = text.size();
    for (int len = 2; len <= 6; ++len) { // check lengths 2 to 6
        for (int i = 0; i + 2*len <= n; ++i) {
            if (text.substr(i, len) == text.substr(i + len, len))
                return true;
        }
    }
    return false;
}

// ---------------------------------------------------------
// Adaptive Strategy Controller
// ---------------------------------------------------------
void adaptiveStringMatch(const string &text, const vector<string> &patterns) {
    for (auto &pattern : patterns) {
        if (pattern.empty()) {
            cout << "Invalid pattern (empty string)\n";
            continue;
        }

        if (pattern.size() < 5) {
            naiveSearch(text, pattern);
        } else if (isPeriodic(text)) {
            KMPsearch(text, pattern);
        } else if (patterns.size() > 1) {
            rabinKarpSearch(text, pattern);
        } else {
            KMPsearch(text, pattern);
        }
        cout << "----------------------------------------\n";
    }
}

// ---------------------------------------------------------
// MAIN FUNCTION
// ---------------------------------------------------------
int main() {
    cin.tie(nullptr);

    cout << "Enter text: ";
    string text;
    getline(cin, text);

    int numPatterns;
    cout << "Enter number of patterns: ";
    cin >> numPatterns;
    cin.ignore();

    vector<string> patterns(numPatterns);
    for (int i = 0; i < numPatterns; i++) {
        cout << "Enter pattern " << i + 1 << ": ";
        getline(cin, patterns[i]);
    }

    cout << "\n--- Adaptive String Matching ---\n";
    adaptiveStringMatch(text, patterns);

    return 0;
}
