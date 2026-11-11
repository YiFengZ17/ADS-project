// Test data generator. Usage: ./gen n d type seed
// type: random (prices and coupons spread widely), close (every pair costs 1 to 4000),
//       equal (every pair costs exactly 1), far (every pair costs more than 10^6)
// Prints "n d", then n prices, then n coupon values. The largest coupon is always less
// than the lowest price. The same arguments give the same data with any compiler.

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
    if (argc != 5) {
        cerr << "usage: gen n d random|close|equal|far seed\n";
        return 1;
    }
    ll n = atoll(argv[1]), d = atoll(argv[2]);
    string type = argv[3];
    mt19937_64 rng((unsigned long long)atoll(argv[4]));
    if (n < 1 || d < 1) {
        cerr << "need n >= 1 and d >= 1\n";
        return 1;
    }

    // prices come from [price_lo, price_hi] and coupons from [coupon_lo, coupon_hi]
    ll price_lo, price_hi, coupon_lo, coupon_hi;
    if (type == "random") {  // costs from 1 up to 2 * 10^6
        coupon_lo = 1, coupon_hi = 1000000, price_lo = 1000001, price_hi = 2000000;
    } else if (type == "close") {  // costs from 1 to 4000, so many pairs are cheap
        coupon_lo = 500000000 - 2000, coupon_hi = 500000000 - 1;
        price_lo = 500000000, price_hi = 500000000 + 1999;
    } else if (type == "equal") {  // all prices equal and all coupons equal: every cost is 1
        coupon_lo = coupon_hi = 999999998;
        price_lo = price_hi = 999999999;
    } else if (type == "far") {  // costs of at least 4 * 10^8: nothing can be bought
        coupon_lo = 1, coupon_hi = 100000000, price_lo = 500000000, price_hi = 1000000000;
    } else {
        cerr << "unknown type " << type << "\n";
        return 1;
    }

    vector<ll> price(n), coupon(n);
    for (ll &x : price) x = uniform(rng, price_lo, price_hi);
    for (ll &x : coupon) x = uniform(rng, coupon_lo, coupon_hi);

    // print in the input format of the problem
    cout << n << ' ' << d << '\n';
    for (ll i = 0; i < n; i++) cout << price[i] << (i + 1 < n ? ' ' : '\n');
    for (ll i = 0; i < n; i++) cout << coupon[i] << (i + 1 < n ? ' ' : '\n');
    return 0;
}
