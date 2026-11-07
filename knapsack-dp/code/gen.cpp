// Test data generator. Usage: ./gen n R type seed [ratio]
// type: uncorrelated, weak, strong, inverse or subset; weights are about 1..R.
// The capacity is ratio (default 0.5) times the total weight. Prints "n c" and n lines "w p".
// The same arguments give the same data with any compiler.

#include <climits>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

typedef long long ll;

// random integer in [lo, hi]; we do not use uniform_int_distribution,
// because GCC and Clang give different numbers with it for the same seed
ll uniform(mt19937_64 &rng, ll lo, ll hi) {
    unsigned long long range = (unsigned long long)(hi - lo) + 1;
    unsigned long long limit = ULLONG_MAX - ULLONG_MAX % range;  // a multiple of range
    unsigned long long x = rng();
    while (x >= limit) x = rng();  // drop the few values that would make small results more likely
    return lo + (ll)(x % range);
}

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    if (argc != 5 && argc != 6) {
        cerr << "usage: gen n R uncorrelated|weak|strong|inverse|subset seed [ratio]\n";
        return 1;
    }
    ll n = atoll(argv[1]), R = atoll(argv[2]);
    string type = argv[3];
    mt19937_64 rng((unsigned long long)atoll(argv[4]));
    double ratio = (argc == 6) ? atof(argv[5]) : 0.5;
    if (n < 1 || R < 1 || R > 100000000 || ratio < 0 || ratio > 1) {
        cerr << "need n >= 1, 1 <= R <= 1e8 and 0 <= ratio <= 1\n";
        return 1;
    }
    ll gap = max(1LL, R / 10);

    vector<ll> w(n), p(n);
    ll total = 0;
    for (ll i = 0; i < n; i++) {
        if (type == "uncorrelated") {  // weight and profit are independent
            w[i] = uniform(rng, 1, R);
            p[i] = uniform(rng, 1, R);
        } else if (type == "weak") {  // profit close to weight
            w[i] = uniform(rng, 1, R);
            p[i] = max(1LL, uniform(rng, w[i] - gap, w[i] + gap));
        } else if (type == "strong") {  // profit = weight + R/10
            w[i] = uniform(rng, 1, R);
            p[i] = w[i] + gap;
        } else if (type == "inverse") {  // weight = profit + R/10
            p[i] = uniform(rng, 1, R);
            w[i] = p[i] + gap;
        } else if (type == "subset") {  // profit = weight
            w[i] = uniform(rng, 1, R);
            p[i] = w[i];
        } else {
            cerr << "unknown type " << type << "\n";
            return 1;
        }
        total += w[i];
    }
    cout << n << " " << (ll)(ratio * (double)total) << "\n";
    for (ll i = 0; i < n; i++) cout << w[i] << " " << p[i] << "\n";
    return 0;
}
