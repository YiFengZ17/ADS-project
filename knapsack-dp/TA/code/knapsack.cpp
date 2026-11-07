#include "knapsack.h"

#include <algorithm>
#include <climits>
#include <functional>
#include <numeric>
#include <random>

using namespace std;

__extension__ typedef __int128 i128;  // wide integer for products and penalties

const ll MAX_ITEMS = 10000000;
const ll MAX_VALUE = 1000000000;             // largest allowed weight or profit
const ll MAX_CAPACITY = 1000000000000000000LL;
const ll SMALL_MAX_WEIGHT = 2000;            // the bonus method needs arrays of size 2w^2
const ll NEG = LLONG_MIN / 4;                // marks a weight that no subset reaches

// read "n c" and the items; reject sizes and values outside the allowed range
bool read_instance(istream &is, Instance &in) {
    ll n;
    if (!(is >> n >> in.c)) return false;
    if (n < 0 || n > MAX_ITEMS || in.c < 0 || in.c > MAX_CAPACITY) return false;
    in.items.resize(n);
    for (Item &it : in.items) {
        if (!(is >> it.w >> it.p)) return false;
        if (it.w < 1 || it.w > MAX_VALUE || it.p < 0 || it.p > MAX_VALUE) return false;
    }
    return true;
}

// DP-1 from class, kept in one row: z[d] is the best profit for capacity d using the items so far
Result dp1(const Instance &in) {
    ll c = in.c;
    vector<ll> z(c + 1, 0);  // row 0: no items, profit 0 for every capacity
    ll work = 0;
    for (const Item &it : in.items) {
        // go from high d to low d, so z[d - w] still holds the value of the previous row
        for (ll d = c; d >= it.w; d--)
            z[d] = max(z[d], z[d - it.w] + it.p);
        work += c + 1;  // one full row of the table
    }
    return {z[c], work};
}

struct State {
    ll w, p;  // weight sum and profit sum of one partial solution
};

// add item (w, p) to every state; the list is sorted by weight, so stop at the first one over c
static void add_item(const vector<State> &list, const Item &it, ll c, vector<State> &out) {
    out.clear();
    for (const State &s : list) {
        if (s.w + it.w > c) break;
        out.push_back({s.w + it.w, s.p + it.p});
    }
}

// merge two lists sorted by weight and drop every state that another state beats
// (Merge-Lists in Fig. 3.6 of Kellerer, Pferschy and Pisinger)
static void merge_lists(const vector<State> &a, const vector<State> &b, vector<State> &out) {
    out.clear();
    size_t i = 0, j = 0;
    while (i < a.size() || j < b.size()) {
        State s;
        if (j == b.size() || (i < a.size() && a[i].w <= b[j].w)) s = a[i++];
        else s = b[j++];
        if (!out.empty() && s.p <= out.back().p) continue;         // not lighter and not better
        if (!out.empty() && s.w == out.back().w) out.pop_back();   // same weight, more profit
        out.push_back(s);
    }
}

// DP-with-Lists (Section 3.4 of the book): the same recursion as DP-1, but each row only keeps
// the (weight, profit) pairs that no lighter pair beats
Result dp_lists(const Instance &in) {
    vector<State> list = {{0, 0}}, shifted, merged;
    ll work = 1;
    for (const Item &it : in.items) {
        add_item(list, it, in.c, shifted);
        merge_lists(list, shifted, merged);
        swap(list, merged);
        work += list.size();
    }
    return {list.back().p, work};  // the heaviest state also has the largest profit
}

// true if a has a larger profit per unit of weight than b, compared without division
static bool more_efficient(const Item &a, const Item &b) {
    return (i128)a.p * b.w > (i128)b.p * a.w;
}

// items that fit into the knapsack alone, sorted by decreasing efficiency
static vector<Item> sorted_by_efficiency(const Instance &in) {
    vector<Item> items;
    for (const Item &it : in.items)
        if (it.w <= in.c) items.push_back(it);
    stable_sort(items.begin(), items.end(), more_efficient);
    return items;
}

// DP-with-Upper-Bounds (Section 3.5 of the book): DP-with-Lists on items sorted by efficiency,
// dropping every state whose upper bound can not beat the best solution found so far
Result dp_bounds(const Instance &in) {
    vector<Item> items = sorted_by_efficiency(in);
    size_t n = items.size();
    ll c = in.c;
    // prefix sums of weight and profit in efficiency order
    vector<ll> W(n + 1, 0), P(n + 1, 0);
    for (size_t k = 0; k < n; k++) {
        W[k + 1] = W[k] + items[k].w;
        P[k + 1] = P[k] + items[k].p;
    }
    // the greedy solution gives the first value of z, the best profit found so far
    ll z = 0, room = c;
    for (const Item &it : items) {
        if (it.w <= room) {
            room -= it.w;
            z += it.p;
        }
    }

    // upper bound of a state after items 1..j: fill the room c - w with items j+1, j+2, ... in order,
    // and take the first item that does not fit only in part
    auto bound = [&](size_t j, const State &s) {
        ll target = W[j] + (c - s.w);
        size_t k = upper_bound(W.begin() + j, W.end(), target) - W.begin() - 1;  // items j+1..k fit
        ll u = s.p + P[k] - P[j];
        if (k < n) u += (ll)((i128)(target - W[k]) * items[k].p / items[k].w);
        return u;
    };

    vector<State> list = {{0, 0}}, shifted, merged;
    ll work = 1;
    for (size_t j = 0; j < n && !list.empty(); j++) {
        add_item(list, items[j], c, shifted);
        merge_lists(list, shifted, merged);
        work += merged.size();
        z = max(z, merged.back().p);  // every state is a feasible solution
        list.clear();
        for (const State &s : merged)
            if (bound(j + 1, s) > z) list.push_back(s);  // keep only states that may still beat z
    }
    return {z, work};
}

// SMAWK (Aggarwal et al. 1987): the largest entry of every row, for a matrix whose row maxima move
// right from row to row; ans[r] gets the column for row r, the rightmost one on ties
template <class F>
static void smawk(const vector<int> &rows, const vector<int> &cols, F &val, vector<int> &ans) {
    if (rows.empty()) return;
    // reduce: keep at most one candidate column per row
    vector<int> kept;
    for (int col : cols) {
        while (!kept.empty()) {
            int r = rows[kept.size() - 1];
            if (val(r, col) < val(r, kept.back())) break;
            kept.pop_back();  // this column is never the rightmost maximum of any row
        }
        if (kept.size() < rows.size()) kept.push_back(col);
    }
    // solve the odd rows first
    vector<int> odd;
    for (size_t i = 1; i < rows.size(); i += 2) odd.push_back(rows[i]);
    smawk(odd, kept, val, ans);
    // each even row has its maximum between the answers of the odd rows around it
    size_t k = 0;
    for (size_t i = 0; i < rows.size(); i += 2) {
        int r = rows[i];
        int last = (i + 1 < rows.size()) ? ans[rows[i + 1]] : kept.back();
        int best = kept[k];
        while (true) {
            if (val(r, kept[k]) >= val(r, best)) best = kept[k];
            if (kept[k] == last) break;
            k++;
        }
        ans[r] = best;
    }
}

// out[i] = max over j of a[j] + b[i - j], where the steps of b never grow (b is concave);
// then the best j moves right as i grows, so SMAWK finds all of them in linear time
static void concave_convolution(const vector<ll> &a, const vector<ll> &b, vector<ll> &out) {
    int len = (int)a.size(), m = (int)b.size() - 1;
    const i128 BIG = (i128)1 << 70;    // slope outside b, steeper than any real profit
    const i128 LOW = -((i128)1 << 100);  // stands for NEG inside the matrix
    // extend b to all integers with very steep sides, so it stays concave everywhere
    auto b_ext = [&](int t) -> i128 {
        if (t < 0) return b[0] + (i128)t * BIG;
        if (t > m) return b[m] - (i128)(t - m) * BIG;
        return b[t];
    };
    auto val = [&](int i, int j) -> i128 {
        i128 x = (a[j] == NEG) ? LOW : (i128)a[j];
        return x + b_ext(i - j);
    };
    vector<int> rows(len), cols(len), arg(len);
    iota(rows.begin(), rows.end(), 0);
    iota(cols.begin(), cols.end(), 0);
    smawk(rows, cols, val, arg);
    out.assign(len, NEG);
    for (int i = 0; i < len; i++) {
        int j = arg[i];
        // a fake entry only wins when the row has no real entry at all
        if (j <= i && i - j <= m && a[j] != NEG) out[i] = a[j] + b[i - j];
    }
}

// best total value for every exact weight 0..T (values may be negative); all items of one weight h
// are added together, with one call of concave_convolution for each remainder mod h
static vector<ll> best_exact(const vector<Item> &items, ll smax, ll T, ll &work) {
    vector<ll> f(T + 1, NEG);
    f[0] = 0;
    vector<vector<ll>> by_weight(smax + 1);
    for (const Item &it : items) by_weight[it.w].push_back(it.p);
    vector<ll> a, b, out;
    for (ll h = 1; h <= smax; h++) {
        vector<ll> &vals = by_weight[h];
        ll m = min((ll)vals.size(), T / h);  // more than T / h items of weight h never fit
        if (m == 0) continue;
        nth_element(vals.begin(), vals.begin() + (m - 1), vals.end(), greater<ll>());
        sort(vals.begin(), vals.begin() + m, greater<ll>());
        // b[k] is the value of the k best items; it is concave because the values go down
        b.assign(m + 1, 0);
        for (ll k = 0; k < m; k++) b[k + 1] = b[k] + vals[k];
        for (ll r = 0; r < h && r <= T; r++) {
            a.clear();
            for (ll x = r; x <= T; x += h) a.push_back(f[x]);
            concave_convolution(a, b, out);
            for (size_t i = 0; i < out.size(); i++) f[r + (ll)i * h] = out[i];
            work += (ll)a.size();
        }
    }
    return f;
}

// greedy prefix: take items by decreasing efficiency until the next one does not fit;
// found like quickselect in expected O(n) time, without sorting all items
static vector<char> maximal_prefix(const vector<Item> &items, ll c) {
    size_t n = items.size();
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    vector<char> in_prefix(n, 0);
    mt19937 rng(12345);
    size_t lo = 0, hi = n;
    ll room = c;
    while (lo < hi) {
        Item pivot = items[idx[lo + rng() % (hi - lo)]];
        // three groups: more efficient [lo, lt), as efficient [lt, gt), less efficient [gt, hi)
        size_t lt = lo, i = lo, gt = hi;
        while (i < gt) {
            const Item &x = items[idx[i]];
            if (more_efficient(x, pivot)) swap(idx[lt++], idx[i++]);
            else if (more_efficient(pivot, x)) swap(idx[i], idx[--gt]);
            else i++;
        }
        ll high = 0;
        for (i = lo; i < lt; i++) high += items[idx[i]].w;
        if (high > room) {  // the prefix ends inside the more efficient group
            hi = lt;
            continue;
        }
        for (i = lo; i < lt; i++) in_prefix[idx[i]] = 1;
        room -= high;
        for (i = lt; i < gt; i++) {
            if (items[idx[i]].w > room) return in_prefix;  // first item that does not fit
            in_prefix[idx[i]] = 1;
            room -= items[idx[i]].w;
        }
        lo = gt;
    }
    return in_prefix;
}

// bonus method of Polak, Rohwedder and Wegrzycki (2021): some optimal solution differs from the
// greedy prefix in at most 2w items, so we only remove and add items of total weight at most 2w^2
Result small_items(const Instance &in) {
    vector<Item> items;
    ll total_w = 0, total_p = 0, s = 0;
    for (const Item &it : in.items) {
        if (it.w > in.c) continue;  // can never be used
        items.push_back(it);
        total_w += it.w;
        total_p += it.p;
        s = max(s, it.w);
    }
    if (total_w <= in.c) return {total_p, (ll)items.size()};

    vector<char> in_prefix = maximal_prefix(items, in.c);
    ll prefix_w = 0, prefix_p = 0;
    vector<Item> removable, addable;
    for (size_t i = 0; i < items.size(); i++) {
        if (in_prefix[i]) {
            prefix_w += items[i].w;
            prefix_p += items[i].p;
            removable.push_back({items[i].w, -items[i].p});  // removing an item loses its profit
        } else {
            addable.push_back(items[i]);
        }
    }
    ll delta = in.c - prefix_w;  // room left by the prefix, less than s
    ll D = 2 * s * s;            // at most 2s changed items, each of weight at most s
    ll remove_limit = min(D, prefix_w);
    ll add_limit = min(D + delta, total_w - prefix_w);

    ll work = (ll)items.size();
    vector<ll> lose = best_exact(removable, s, remove_limit, work);
    vector<ll> gain = best_exact(addable, s, add_limit, work);
    for (ll y = 1; y <= add_limit; y++) gain[y] = max(gain[y], gain[y - 1]);  // weight at most y

    // remove weight exactly k, then add weight at most k + delta
    ll best = 0;
    for (ll k = 0; k <= remove_limit; k++)
        if (lose[k] != NEG) best = max(best, lose[k] + gain[min(k + delta, add_limit)]);
    return {prefix_p + best, work};
}

// names of all algorithms, in the order used by "all"
const vector<string> &algorithm_names() {
    static const vector<string> names = {"dp1", "lists", "lists-sorted", "bounds", "small"};
    return names;
}

// run one algorithm by name; refuse inputs that would need too much memory
bool run_algorithm(const string &name, const Instance &in, Result &out, string &error) {
    if (name == "dp1") {
        if (in.c > DP1_MAX_CAPACITY) {
            error = "capacity too large for dp1";
            return false;
        }
        out = dp1(in);
    } else if (name == "lists") {
        out = dp_lists(in);
    } else if (name == "lists-sorted") {
        // the same list method on items in efficiency order, to separate ordering from bounding
        Instance sorted = {sorted_by_efficiency(in), in.c};
        out = dp_lists(sorted);
    } else if (name == "bounds") {
        out = dp_bounds(in);
    } else if (name == "small") {
        ll s = 0;
        for (const Item &it : in.items)
            if (it.w <= in.c) s = max(s, it.w);
        if (s > SMALL_MAX_WEIGHT) {
            error = "weights too large for small";
            return false;
        }
        out = small_items(in);
    } else {
        error = "unknown algorithm " + name;
        return false;
    }
    return true;
}
