#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
using namespace std;

typedef long long ll;

const size_t BRUTE_MAX_N = 5000;  // the brute force method stores n * n numbers

struct Answer {
    ll items;  // number of items bought
    ll left;   // money left
};

// buying item `row` with coupon `col` costs `cost`
struct Purchase {
    ll cost;
    int row, col;
};

// order for the min-heap: the cheapest purchase is on top
struct Dearer {
    bool operator()(const Purchase &a, const Purchase &b) const { return a.cost > b.cost; }
};

// brute force: list the cost of every (item, coupon) pair, sort them and buy from the cheapest
Answer solve_brute(const vector<ll> &price, const vector<ll> &coupon, ll d) {
    vector<ll> costs;
    costs.reserve(price.size() * coupon.size());
    for (ll p : price)
        for (ll c : coupon) costs.push_back(p - c);
    sort(costs.begin(), costs.end());
    Answer ans = {0, d};
    for (ll cost : costs) {
        if (cost > ans.left) break;  // all later pairs cost at least as much
        ans.left -= cost;
        ans.items++;
    }
    return ans;
}

// heap method: price is sorted from low to high and coupon from high to low, so
// cost(row, col) = price[row] - coupon[col] grows along every row and every column.
// The heap holds the next unused pair of each row we have started
Answer solve_heap(const vector<ll> &price, const vector<ll> &coupon, ll d) {
    int n = (int)price.size();
    priority_queue<Purchase, vector<Purchase>, Dearer> heap;
    heap.push({price[0] - coupon[0], 0, 0});  // the cheapest pair of all
    Answer ans = {0, d};
    while (!heap.empty()) {
        Purchase top = heap.top();
        if (top.cost > ans.left) break;  // the cheapest pair is too expensive, so stop
        heap.pop();
        ans.left -= top.cost;
        ans.items++;
        // the next pair in the same row
        if (top.col + 1 < n)
            heap.push({price[top.row] - coupon[top.col + 1], top.row, top.col + 1});
        // a row is started only after the first pair of the row above is bought
        if (top.col == 0 && top.row + 1 < n)
            heap.push({price[top.row + 1] - coupon[0], top.row + 1, 0});
    }
    return ans;
}

// count the pairs with cost <= t and add up their cost; false if the total is more than d.
// prefix[k] is the sum of the k largest coupons
static bool can_buy_all_up_to(const vector<ll> &price, const vector<ll> &coupon,
                              const vector<ll> &prefix, ll t, ll d, ll &count, ll &total) {
    count = total = 0;
    size_t k = coupon.size();  // number of coupons that are large enough for the current item
    for (ll p : price) {
        // cost <= t means coupon >= p - t; a higher price leaves fewer such coupons
        while (k > 0 && coupon[k - 1] < p - t) k--;
        if (k == 0) break;
        count += (ll)k;
        total += (ll)k * p - prefix[k];
        if (total > d) return false;  // stop early, this also keeps total from overflowing
    }
    return true;
}

// binary search method: find the largest t such that we can pay for every pair
// with cost <= t, then spend the rest of the money on pairs with cost t + 1
Answer solve_search(const vector<ll> &price, const vector<ll> &coupon, ll d) {
    size_t n = price.size();
    vector<ll> prefix(n + 1, 0);
    for (size_t i = 0; i < n; i++) prefix[i + 1] = prefix[i] + coupon[i];
    ll max_cost = price[n - 1] - coupon[n - 1];
    ll lo = 0, hi = min(d, max_cost);  // t = 0 always works because every cost is at least 1
    ll count, total;
    while (lo < hi) {
        ll mid = lo + (hi - lo + 1) / 2;
        if (can_buy_all_up_to(price, coupon, prefix, mid, d, count, total)) lo = mid;
        else hi = mid - 1;
    }
    can_buy_all_up_to(price, coupon, prefix, lo, d, count, total);
    Answer ans = {count, d - total};
    if (lo < max_cost) {
        // not all pairs of cost lo + 1 can be paid, so there are more of them than we can buy
        ll extra = ans.left / (lo + 1);
        ans.items += extra;
        ans.left -= extra * (lo + 1);
    }
    return ans;
}

// sort the input and run the chosen method
bool solve(const string &method, vector<ll> price, vector<ll> coupon, ll d, Answer &ans) {
    sort(price.begin(), price.end());
    sort(coupon.begin(), coupon.end(), greater<ll>());
    if (method == "heap") ans = solve_heap(price, coupon, d);
    else if (method == "search") ans = solve_search(price, coupon, d);
    else if (method == "brute" && price.size() <= BRUTE_MAX_N) ans = solve_brute(price, coupon, d);
    else return false;
    return true;
}

// Usage: ./coupons [heap|search|brute] [runs] < input
// With no arguments it uses the heap method and prints "items left".
// With runs it times the method: one warmup, then `runs` measured runs
int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string method = (argc > 1) ? argv[1] : "heap";
    int runs = (argc > 2) ? atoi(argv[2]) : 0;
    int n;
    ll d;
    if (!(cin >> n >> d) || n < 1) {
        cerr << "bad input\n";
        return 1;
    }
    vector<ll> price(n), coupon(n);
    for (ll &x : price) cin >> x;
    for (ll &x : coupon) cin >> x;
    if (!cin) {
        cerr << "bad input\n";
        return 1;
    }

    Answer ans;
    if (!solve(method, price, coupon, d, ans)) {
        cerr << "unknown method, or n too large for brute\n";
        return 1;
    }
    if (runs <= 0) {
        cout << ans.items << ' ' << ans.left << '\n';
        return 0;
    }
    // timing mode: only the method is timed (sorting included), not reading the input
    vector<double> ms;
    for (int r = 0; r < runs; r++) {
        auto start = chrono::steady_clock::now();
        solve(method, price, coupon, d, ans);
        auto stop = chrono::steady_clock::now();
        ms.push_back(chrono::duration<double, milli>(stop - start).count());
    }
    sort(ms.begin(), ms.end());
    double median = (ms[(runs - 1) / 2] + ms[runs / 2]) / 2;
    cout << method << ' ' << n << ' ' << d << ' ' << ans.items << ' ' << ans.left << ' '
         << median << ' ' << ms.front() << ' ' << ms.back() << '\n';
    return 0;
}
