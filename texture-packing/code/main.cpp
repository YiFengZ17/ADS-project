#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
using namespace std;

typedef long long ll;

const ll MAX_N = 1000000;        // largest number of textures we accept
const ll MAX_SIDE = 1000000000;  // largest width or height we accept, so all sums fit in ll

struct Rect {
    ll w, h;
};

// lower-left corner of one placed texture
struct Place {
    ll x, y;
};

// the result of one algorithm: the corner of every texture (in input order) and the total height
struct Packing {
    vector<Place> pos;
    ll height;
};

// indices of the textures from tall to short; equal heights go wider first, then by input order
vector<int> by_height(const vector<Rect> &r) {
    vector<int> order(r.size());
    for (size_t i = 0; i < r.size(); i++) order[i] = (int)i;
    stable_sort(order.begin(), order.end(), [&](int a, int b) {
        if (r[a].h != r[b].h) return r[a].h > r[b].h;
        return r[a].w > r[b].w;
    });
    return order;
}

// NFDH: fill one level from left to right; when a texture does not fit, close it and open a new one
Packing nfdh(const vector<Rect> &r, ll W) {
    Packing p{vector<Place>(r.size()), 0};
    ll base = 0, level_h = 0, x = 0;
    bool started = false;
    for (int i : by_height(r)) {
        if (!started || x + r[i].w > W) {  // open a new level on top of the current one
            base += level_h;
            level_h = r[i].h;  // the first texture is the tallest one of its level
            x = 0;
            started = true;
        }
        p.pos[i] = {x, base};
        x += r[i].w;
    }
    p.height = base + level_h;
    return p;
}

// a max tree over the levels: each node keeps the largest free width among the levels below it
struct LevelTree {
    int size;
    vector<ll> best;

    LevelTree(int n) {
        size = 1;
        while (size < n) size *= 2;
        best.assign(2 * size, -1);  // -1 means the level is not opened yet
    }

    // set the free width of level i and update its ancestors
    void set(int i, ll free_width) {
        i += size;
        best[i] = free_width;
        for (i /= 2; i >= 1; i /= 2) best[i] = max(best[2 * i], best[2 * i + 1]);
    }

    // the lowest level with at least w free width, or -1 if there is none
    int first_fit(ll w) const {
        if (best[1] < w) return -1;
        int node = 1;
        while (node < size) node = best[2 * node] >= w ? 2 * node : 2 * node + 1;  // go left if possible
        return node - size;
    }
};

// FFDH (our main algorithm): put each texture on the lowest level where it fits, found with the tree
Packing ffdh(const vector<Rect> &r, ll W) {
    Packing p{vector<Place>(r.size()), 0};
    vector<ll> base, used;  // bottom and used width of every level
    LevelTree tree((int)r.size());
    for (int i : by_height(r)) {
        int k = tree.first_fit(r[i].w);
        if (k < 0) {  // no level has room: open a new one on top
            k = (int)base.size();
            base.push_back(p.height);
            used.push_back(0);
            p.height += r[i].h;
        }
        p.pos[i] = {used[k], base[k]};
        used[k] += r[i].w;
        tree.set(k, W - used[k]);
    }
    return p;
}

// FFDH with a plain scan over all levels; same result as ffdh, used to show what the tree saves
Packing ffdh_scan(const vector<Rect> &r, ll W) {
    Packing p{vector<Place>(r.size()), 0};
    vector<ll> base, used;
    for (int i : by_height(r)) {
        size_t k = 0;
        while (k < base.size() && W - used[k] < r[i].w) k++;  // first level with enough room
        if (k == base.size()) {
            base.push_back(p.height);
            used.push_back(0);
            p.height += r[i].h;
        }
        p.pos[i] = {used[k], base[k]};
        used[k] += r[i].w;
    }
    return p;
}

// BFDH: put each texture on the level that leaves the least free width (lowest level on ties)
Packing bfdh(const vector<Rect> &r, ll W) {
    Packing p{vector<Place>(r.size()), 0};
    vector<ll> base, used;
    set<pair<ll, int>> free_width;  // (free width, level) for every open level
    for (int i : by_height(r)) {
        auto it = free_width.lower_bound({r[i].w, -1});  // smallest free width that is still enough
        int k;
        if (it == free_width.end()) {
            k = (int)base.size();
            base.push_back(p.height);
            used.push_back(0);
            p.height += r[i].h;
        } else {
            k = it->second;
            free_width.erase(it);
        }
        p.pos[i] = {used[k], base[k]};
        used[k] += r[i].w;
        free_width.insert({W - used[k], k});
    }
    return p;
}

// Sleator: stack the wide textures, fill one full-width level, then fill levels in two halves
Packing sleator(const vector<Rect> &r, ll W) {
    Packing p{vector<Place>(r.size()), 0};
    ll top = 0;
    vector<int> narrow;  // textures not wider than W/2, from tall to short
    for (int i : by_height(r)) {
        if (2 * r[i].w > W) {  // wide: no two of them fit side by side, so stack them
            p.pos[i] = {0, top};
            top += r[i].h;
        } else {
            narrow.push_back(i);
        }
    }
    ll half = W / 2;  // the left half is [0, half), the right half is [half, W)
    ll left_top = top, right_top = top;
    size_t k = 0;
    ll x = 0;
    // one level over the full width; remember how high it reaches in each half
    while (k < narrow.size() && x + r[narrow[k]].w <= W) {
        int i = narrow[k];
        p.pos[i] = {x, top};
        if (x < half) left_top = max(left_top, top + r[i].h);
        if (x + r[i].w > half) right_top = max(right_top, top + r[i].h);
        x += r[i].w;
        k++;
    }
    // the rest goes into levels of width W/2, always on the half that is lower now
    while (k < narrow.size()) {
        bool left = left_top <= right_top;
        ll x0 = left ? 0 : half;
        ll width = left ? half : W - half;
        ll base = left ? left_top : right_top;
        ll level_h = r[narrow[k]].h;
        x = 0;
        while (k < narrow.size() && x + r[narrow[k]].w <= width) {
            p.pos[narrow[k]] = {x0 + x, base};
            x += r[narrow[k]].w;
            k++;
        }
        (left ? left_top : right_top) = base + level_h;
    }
    p.height = max(left_top, right_top);
    return p;
}

// a simple lower bound on the best height: total area / W, the tallest texture, and the wide textures
ll lower_bound_height(const vector<Rect> &r, ll W) {
    ll area = 0, tallest = 0, wide = 0;
    for (const Rect &t : r) {
        area += t.w * t.h;
        tallest = max(tallest, t.h);
        if (2 * t.w > W) wide += t.h;  // two wide textures can never be side by side
    }
    return max({(area + W - 1) / W, tallest, wide});
}

typedef Packing (*Algorithm)(const vector<Rect> &, ll);

// find an algorithm by its name, or return nullptr
Algorithm find_algorithm(const string &name) {
    if (name == "ffdh") return ffdh;
    if (name == "ffdh-scan") return ffdh_scan;
    if (name == "nfdh") return nfdh;
    if (name == "bfdh") return bfdh;
    if (name == "sleator") return sleator;
    return nullptr;
}

// read "n W" and then n lines "w h"; return false if the input is broken
bool read_input(vector<Rect> &r, ll &W) {
    ll n;
    if (!(cin >> n >> W)) return false;
    if (n < 1 || n > MAX_N || W < 1 || W > MAX_SIDE) return false;
    r.resize(n);
    for (Rect &t : r) {
        if (!(cin >> t.w >> t.h)) return false;
        if (t.w < 1 || t.w > W || t.h < 1 || t.h > MAX_SIDE) return false;
    }
    string extra;
    return !(cin >> extra);  // nothing may follow the last texture
}

// time one algorithm: 1 warmup and `runs` measured runs; a short run repeats until it takes 1 ms
void time_algorithm(Algorithm algo, const string &name, const vector<Rect> &r, ll W, int runs) {
    typedef chrono::steady_clock clock;
    auto start = clock::now();
    Packing p = algo(r, W);  // warmup, also gives the answer we print
    double once = chrono::duration<double, milli>(clock::now() - start).count();
    int repeat = once >= 1.0 ? 1 : (int)min(100000.0, 1.0 / max(once, 1e-6)) + 1;
    vector<double> times;
    ll check = 0;  // keeps the compiler from skipping the repeated calls
    for (int run = 0; run < runs; run++) {
        start = clock::now();
        for (int k = 0; k < repeat; k++) check += algo(r, W).height;
        times.push_back(chrono::duration<double, milli>(clock::now() - start).count() / repeat);
    }
    if (check != p.height * runs * repeat) cerr << "timing runs disagree\n";
    sort(times.begin(), times.end());
    double median = runs % 2 ? times[runs / 2] : (times[runs / 2 - 1] + times[runs / 2]) / 2;
    cout << name << ' ' << r.size() << ' ' << W << ' ' << p.height << ' ' << lower_bound_height(r, W) << ' '
         << median << ' ' << times.front() << ' ' << times.back() << '\n';
}

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string name = argc >= 2 ? argv[1] : "ffdh";
    Algorithm algo = find_algorithm(name);
    if (!algo || argc > 3) {
        cerr << "usage: pack [ffdh|ffdh-scan|nfdh|bfdh|sleator] [RUNS] < input\n";
        return 1;
    }
    vector<Rect> r;
    ll W;
    if (!read_input(r, W)) {
        cerr << "bad input\n";
        return 1;
    }
    if (argc == 3) {  // timing mode
        int runs = atoi(argv[2]);
        if (runs < 1) {
            cerr << "RUNS must be at least 1\n";
            return 1;
        }
        time_algorithm(algo, name, r, W, runs);
        return 0;
    }
    Packing p = algo(r, W);
    cout << p.height << ' ' << lower_bound_height(r, W) << '\n';
    for (const Place &q : p.pos) cout << q.x << ' ' << q.y << '\n';
    return 0;
}
