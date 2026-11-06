#pragma once

#include <iostream>
#include <string>
#include <vector>

typedef long long ll;

struct Item {
    ll w, p;  // weight and profit
};

struct Instance {
    std::vector<Item> items;
    ll c;  // knapsack capacity
};

// value is the optimal profit; work counts DP cells, list states or convolution entries
struct Result {
    ll value;
    ll work;
};

// largest capacity DP-1 accepts, because it needs an array of c + 1 numbers
const ll DP1_MAX_CAPACITY = 200000000;

// read "n c" and then n lines "w p"; false if the input is missing or out of range
bool read_instance(std::istream &is, Instance &in);

Result dp1(const Instance &in);           // standard DP from class, Theta(nc)
Result dp_lists(const Instance &in);      // Section 3.4: lists of undominated states
Result dp_bounds(const Instance &in);     // Section 3.5: lists plus upper bounds
Result small_items(const Instance &in);   // bonus: O(n + w^3) for small weights

// names accepted by run_algorithm, in the order used by "all"
const std::vector<std::string> &algorithm_names();

// run the named algorithm; false with a message in error if the name or input is not allowed
bool run_algorithm(const std::string &name, const Instance &in, Result &out, std::string &error);
