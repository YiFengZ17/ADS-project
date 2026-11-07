#include "knapsack.h"

using namespace std;

// usage: ./knapsack [dp1|lists|lists-sorted|bounds|small|all] < instance
// one name prints the optimal value; "all" prints "name value" for every algorithm
int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string name = (argc > 1) ? argv[1] : "all";
    Instance in;
    if (!read_instance(cin, in)) {
        cerr << "bad input\n";
        return 1;
    }
    if (name != "all") {
        Result r;
        string error;
        if (!run_algorithm(name, in, r, error)) {
            cerr << error << "\n";
            return 1;
        }
        cout << r.value << "\n";
        return 0;
    }
    // every algorithm on the same instance; an algorithm that refuses the input prints "-"
    for (const string &each : algorithm_names()) {
        Result r;
        string error;
        if (run_algorithm(each, in, r, error)) cout << each << " " << r.value << "\n";
        else cout << each << " -\n";
    }
    return 0;
}
