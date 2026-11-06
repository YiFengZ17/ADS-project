// Test data generator.
// Usage: ./gen n R type seed [ratio]
// type: uncorrelated, weak, strong, inverse or subset (see below); weights are about 1..R.
// The capacity is ratio (default 0.5) times the total weight. Prints "n c" and n lines "w p".

#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

typedef long long ll;

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
    auto uniform = [&](ll lo, ll hi) { return uniform_int_distribution<ll>(lo, hi)(rng); };
    ll gap = max(1LL, R / 10);

    vector<ll> w(n), p(n);
    ll total = 0;
    for (ll i = 0; i < n; i++) {
        if (type == "uncorrelated") {  // weight and profit are independent
            w[i] = uniform(1, R);
            p[i] = uniform(1, R);
        } else if (type == "weak") {  // profit close to weight
            w[i] = uniform(1, R);
            p[i] = max(1LL, uniform(w[i] - gap, w[i] + gap));
        } else if (type == "strong") {  // profit = weight + R/10
            w[i] = uniform(1, R);
            p[i] = w[i] + gap;
        } else if (type == "inverse") {  // weight = profit + R/10
            p[i] = uniform(1, R);
            w[i] = p[i] + gap;
        } else if (type == "subset") {  // profit = weight
            w[i] = uniform(1, R);
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
