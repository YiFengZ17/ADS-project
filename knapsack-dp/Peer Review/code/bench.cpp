// Performance test: time one algorithm on one instance file.
// Usage: ./bench algorithm instance-file [runs]
// Runs once as warmup and then `runs` times (default 10). Only the algorithm is timed, not reading.
// Prints: algorithm n c wmax value work median_ms min_ms max_ms

#include "knapsack.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>

using namespace std;

// run the algorithm once and return the wall time in milliseconds
static double timed_run(const string &name, const Instance &in, Result &r) {
    string error;
    auto start = chrono::steady_clock::now();
    if (!run_algorithm(name, in, r, error)) {
        cerr << error << "\n";
        exit(1);
    }
    auto stop = chrono::steady_clock::now();
    return chrono::duration<double, milli>(stop - start).count();
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "usage: bench algorithm instance-file [runs]\n";
        return 1;
    }
    string name = argv[1];
    int runs = (argc > 3) ? atoi(argv[3]) : 10;
    ifstream file(argv[2]);
    Instance in;
    if (runs < 1 || !file || !read_instance(file, in)) {
        cerr << "bad arguments or input\n";
        return 1;
    }
    ll wmax = 0;
    for (const Item &it : in.items) wmax = max(wmax, it.w);

    Result first;
    timed_run(name, in, first);  // warmup, not measured
    vector<double> times;
    for (int i = 0; i < runs; i++) {
        Result r;
        times.push_back(timed_run(name, in, r));
        if (r.value != first.value) {  // the algorithms are deterministic
            cerr << "value changed between runs\n";
            return 1;
        }
    }
    sort(times.begin(), times.end());
    double median = (runs % 2) ? times[runs / 2] : (times[runs / 2 - 1] + times[runs / 2]) / 2;
    cout << name << " " << in.items.size() << " " << in.c << " " << wmax << " " << first.value
         << " " << first.work << " " << median << " " << times.front() << " " << times.back() << "\n";
    return 0;
}
