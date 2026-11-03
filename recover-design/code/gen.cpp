// Input generator for Project 3 (Recover the Design).
//
// Usage: ./gen n m density type seed > case.in
//   n, m     grid size (2..50)
//   density  chance that a cell holds a connector, for example 0.6
//   type     planted : put random straight fences that never cross at an
//                      empty cell, then write the degree of every connector.
//                      So this input surely has a solution.
//            broken  : make a planted grid, then add 1 to one connector and
//                      take 1 from another. The degree sum does not change, so
//                      the quick check often can not reject it and the solver
//                      has to search. It may or may not have a solution.
//            random  : every connector gets a random degree from 1 to 4.
//                      Most of these grids have no solution.
//   seed     random seed, the same seed always gives the same grid

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
        // list all segments between two neighbouring connectors
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

        // used[r][c]: 0 = free, 1 = a horizontal fence passes, 2 = a vertical fence passes
        vector<vector<int>> used(n, vector<int>(m, 0));
        for (const Segment &s : segments) {
            if (coin(rng) > 0.55) continue;  // keep about half of the segments
            bool flat = (s.r1 == s.r2);
            int mine = flat ? 1 : 2, other = flat ? 2 : 1;
            // a fence can not cross another fence at an empty cell
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
        // a connector with no fence becomes an empty cell; the planted fences stay valid
        // because no fence ends there or passes through it

        if (type == "broken") {
            // move one unit of degree from connector b to connector a
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

    // print the grid in the input format of the problem
    cout << n << ' ' << m << '\n';
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < m; c++) cout << (c ? " " : "") << degree[r][c];
        cout << '\n';
    }
    return 0;
}
