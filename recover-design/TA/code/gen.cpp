// Input generator for the large tests.
// Usage: ./gen n m density type seed
// type: planted (has a solution), broken (planted grid with one degree moved),
//       random (random degrees 1..4)

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using namespace std;

struct Segment { int r1, c1, r2, c2; };  // a fence from (r1, c1) to (r2, c2)

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << "usage: gen n m density planted|broken|random seed\n";
        return 1;
    }
    int n = atoi(argv[1]), m = atoi(argv[2]);
    double density = atof(argv[3]);
    string type = argv[4];
    mt19937 rng((unsigned)atoi(argv[5]));
    uniform_real_distribution<double> coin(0.0, 1.0);
    if (n < 2 || n > 50 || m < 2 || m > 50) {
        cerr << "n and m must be between 2 and 50\n";
        return 1;
    }

    // choose which cells hold a connector
    vector<vector<int>> has(n, vector<int>(m, 0));
    for (int r = 0; r < n; r++)
        for (int c = 0; c < m; c++) has[r][c] = coin(rng) < density;

    vector<vector<int>> degree(n, vector<int>(m, 0));
    if (type == "random") {
        uniform_int_distribution<int> pick(1, 4);
        for (int r = 0; r < n; r++)
            for (int c = 0; c < m; c++)
                if (has[r][c]) degree[r][c] = pick(rng);
    } else {
        // all segments between neighbouring connectors
        vector<Segment> segments;
        for (int r = 0; r < n; r++) {
            int prev = -1;
            for (int c = 0; c < m; c++) {
                if (!has[r][c]) continue;
                if (prev >= 0) segments.push_back({r, prev, r, c});
                prev = c;
            }
        }
        for (int c = 0; c < m; c++) {
            int prev = -1;
            for (int r = 0; r < n; r++) {
                if (!has[r][c]) continue;
                if (prev >= 0) segments.push_back({prev, c, r, c});
                prev = r;
            }
        }
        shuffle(segments.begin(), segments.end(), rng);

        // used[r][c]: 0 free, 1 horizontal fence, 2 vertical fence
        vector<vector<int>> used(n, vector<int>(m, 0));
        for (const Segment &s : segments) {
            if (coin(rng) > 0.55) continue;  // keep about half of the segments
            bool flat = (s.r1 == s.r2);
            int mine = flat ? 1 : 2, other = flat ? 2 : 1;
            // skip it if it would cross a fence at an empty cell
            bool ok = true;
            for (int r = s.r1; r <= s.r2; r++)
                for (int c = s.c1; c <= s.c2; c++)
                    if (!has[r][c] && used[r][c] == other) ok = false;
            if (!ok) continue;
            for (int r = s.r1; r <= s.r2; r++)
                for (int c = s.c1; c <= s.c2; c++)
                    if (!has[r][c]) used[r][c] = mine;
            degree[s.r1][s.c1]++;
            degree[s.r2][s.c2]++;
        }
        // a connector with no fence just becomes an empty cell

        if (type == "broken") {
            // move 1 degree from connector b to connector a
            for (int tries = 0; tries < 1000; tries++) {
                int ra = rng() % n, ca = rng() % m, rb = rng() % n, cb = rng() % m;
                if (ra == rb && ca == cb) continue;
                if (degree[ra][ca] >= 1 && degree[ra][ca] < 4 && degree[rb][cb] > 1) {
                    degree[ra][ca]++;
                    degree[rb][cb]--;
                    break;
                }
            }
        } else if (type != "planted") {
            cerr << "unknown type " << type << "\n";
            return 1;
        }
    }

    // print the grid in the input format
    cout << n << ' ' << m << '\n';
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < m; c++) cout << (c ? " " : "") << degree[r][c];
        cout << '\n';
    }
    return 0;
}
