// Test data generator. Usage: ./gen n type seed [wmax hmax]   (strip width W is always 1000)
// type: uniform (w in [1, wmax], h in [1, hmax]), equalh (w in [1, wmax], every h = hmax),
//       heavy (w in [1, wmax], h = hmax / k with k in [1, 100]: many short, a few tall),
//       cut (a 1000 x 10n rectangle cut into n pieces, so the best height is exactly 10n)
// Defaults: wmax = 1000, hmax = 1000. Prints "n W", then n lines "w h".
// The same arguments give the same data with any compiler.

#include <climits>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <tuple>
#include <vector>
using namespace std;

typedef long long ll;

const ll W = 1000;  // strip width used for all generated data

// random integer in [lo, hi]; we do not use uniform_int_distribution,
// because GCC and Clang give different numbers with it for the same seed
ll uniform(mt19937_64 &rng, ll lo, ll hi) {
    unsigned long long range = (unsigned long long)(hi - lo) + 1;
    unsigned long long limit = ULLONG_MAX - ULLONG_MAX % range;  // a multiple of range
    unsigned long long x = rng();
    while (x >= limit) x = rng();  // drop the few values that would make small results more likely
    return lo + (ll)(x % range);
}

// cut a W x 10n rectangle into n pieces: always cut the largest piece across its longer side
vector<pair<ll, ll>> cut_pieces(mt19937_64 &rng, ll n) {
    // (area, id, w, h); the id makes the order strict, so every library pops the same piece
    priority_queue<tuple<ll, ll, ll, ll>> pieces;
    ll next_id = 0;
    pieces.push(make_tuple(W * 10 * n, next_id++, W, 10 * n));
    while ((ll)pieces.size() < n) {
        ll area, id, w, h;
        tie(area, id, w, h) = pieces.top();
        pieces.pop();
        if (w >= h) {  // vertical cut
            ll a = uniform(rng, 1, w - 1);
            pieces.push(make_tuple(a * h, next_id++, a, h));
            pieces.push(make_tuple((w - a) * h, next_id++, w - a, h));
        } else {  // horizontal cut
            ll a = uniform(rng, 1, h - 1);
            pieces.push(make_tuple(w * a, next_id++, w, a));
            pieces.push(make_tuple(w * (h - a), next_id++, w, h - a));
        }
    }
    vector<pair<ll, ll>> result;
    while (!pieces.empty()) {
        result.push_back({get<2>(pieces.top()), get<3>(pieces.top())});
        pieces.pop();
    }
    // shuffle by hand, because std::shuffle also differs between libraries
    for (ll i = (ll)result.size() - 1; i > 0; i--) swap(result[i], result[uniform(rng, 0, i)]);
    return result;
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 6) {
        cerr << "usage: gen n uniform|equalh|heavy|cut seed [wmax hmax]\n";
        return 1;
    }
    ll n = atoll(argv[1]);
    string type = argv[2];
    mt19937_64 rng((unsigned long long)atoll(argv[3]));
    ll wmax = argc == 6 ? atoll(argv[4]) : W;
    ll hmax = argc == 6 ? atoll(argv[5]) : 1000;
    if (n < 1 || n > 1000000 || wmax < 1 || wmax > W || hmax < 1 || hmax > 1000000000) {
        cerr << "need 1 <= n <= 10^6, 1 <= wmax <= 1000 and 1 <= hmax <= 10^9\n";
        return 1;
    }

    vector<pair<ll, ll>> rects;
    if (type == "cut") {
        rects = cut_pieces(rng, n);
    } else if (type == "uniform" || type == "equalh" || type == "heavy") {
        for (ll i = 0; i < n; i++) {
            ll w = uniform(rng, 1, wmax);
            ll h = hmax;
            if (type == "uniform") h = uniform(rng, 1, hmax);
            if (type == "heavy") h = max(1LL, hmax / uniform(rng, 1, 100));
            rects.push_back({w, h});
        }
    } else {
        cerr << "unknown type " << type << "\n";
        return 1;
    }

    cout << n << ' ' << W << '\n';
    for (auto &t : rects) cout << t.first << ' ' << t.second << '\n';
    return 0;
}
