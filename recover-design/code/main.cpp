#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex { int r, c, degree; vector<int> edges; };
struct Edge { int a, b, da, db; vector<int> conflicts; };

class Solver {
    int n, m;
    vector<Vertex> vertices;
    vector<Edge> edges;
    vector<int> state, trail;
    vector<vector<int>> ids, horizontal, vertical;

    // Add a segment between consecutive connectors and mark its empty interior.
    void add_edge(int a, int b, int da, int db) {
        int e = (int)edges.size();
        edges.push_back({a, b, da, db, {}});
        vertices[a].edges.push_back(e); vertices[b].edges.push_back(e);
        auto &u = vertices[a]; auto &v = vertices[b];
        if (u.r == v.r)
            for (int c = u.c + 1; c < v.c; ++c) horizontal[u.r][c] = e;
        else
            for (int r = u.r + 1; r < v.r; ++r) vertical[r][u.c] = e;
    }
    // Assign an edge once; the trail supports exact rollback after a failed branch.
    bool assign(int e, int value) {
        if (state[e] != -1) return state[e] == value;
        state[e] = value; trail.push_back(e); return true;
    }
    // Apply degree bounds and crossing exclusions until no new edge is fixed.
    bool propagate() {
        size_t before;
        do {
            before = trail.size();
            for (int e = 0; e < (int)edges.size(); ++e)
                if (state[e] == 1)
                    for (int f : edges[e].conflicts)
                        if (!assign(f, 0)) return false;
            for (const auto &v : vertices) {
                int chosen = 0, unknown = 0;
                for (int e : v.edges) { chosen += state[e] == 1; unknown += state[e] == -1; }
                if (chosen > v.degree || chosen + unknown < v.degree) return false;
                if (unknown && (chosen == v.degree || chosen + unknown == v.degree))
                    for (int e : v.edges)
                        if (state[e] == -1 && !assign(e, chosen < v.degree)) return false;
            }
        } while (trail.size() != before);
        return true;
    }
    // Restore the state at a saved trail boundary.
    void rollback(size_t mark) {
        while (trail.size() > mark) { state[trail.back()] = -1; trail.pop_back(); }
    }
    // Exhaust both values of a constrained edge; a leaf is a fully validated design.
    bool search() {
        if (!propagate()) return false;
        int best = -1, minimum = 5;
        for (const auto &v : vertices) {
            int count = 0, candidate = -1;
            for (int e : v.edges) if (state[e] == -1) { ++count; candidate = e; }
            if (count && count < minimum) { minimum = count; best = candidate; }
        }
        if (best == -1) return true;
        size_t mark = trail.size();
        for (int value : {1, 0}) {
            assign(best, value);
            if (search()) return true;
            rollback(mark);
        }
        return false;
    }
public:
    // Parse the grid, build visible neighbors, then record all forbidden crossings.
    bool read(istream &in) {
        if (!(in >> n >> m) || n <= 1 || n > 50 || m <= 1 || m > 50) return false;
        ids.assign(n, vector<int>(m, -1)); horizontal = vertical = ids;
        for (int r = 0; r < n; ++r) for (int c = 0; c < m; ++c) {
            int k; if (!(in >> k) || k < 0 || k > 4) return false;
            if (k) { ids[r][c] = (int)vertices.size(); vertices.push_back({r, c, k, {}}); }
        }
        for (int r = 0; r < n; ++r) {
            int prev = -1;
            for (int c = 0; c < m; ++c) if (ids[r][c] >= 0) {
                if (prev >= 0) add_edge(prev, ids[r][c], 3, 2);
                prev = ids[r][c];
            }
        }
        for (int c = 0; c < m; ++c) {
            int prev = -1;
            for (int r = 0; r < n; ++r) if (ids[r][c] >= 0) {
                if (prev >= 0) add_edge(prev, ids[r][c], 1, 0);
                prev = ids[r][c];
            }
        }
        for (int r = 0; r < n; ++r) for (int c = 0; c < m; ++c) {
            int h = horizontal[r][c], v = vertical[r][c];
            if (h >= 0 && v >= 0) { edges[h].conflicts.push_back(v); edges[v].conflicts.push_back(h); }
        }
        state.assign(edges.size(), -1); return true;
    }
    // Print one legal solution in row-major order, or the required failure text.
    void solve(ostream &out) {
        if (!search()) { out << "No Solution\n"; return; }
        for (const auto &v : vertices) {
            array<int, 4> directions{};
            int id = ids[v.r][v.c];
            for (int e : v.edges) if (state[e] == 1)
                directions[edges[e].a == id ? edges[e].da : edges[e].db] = 1;
            out << v.r + 1 << ' ' << v.c + 1;
            for (int d : directions) out << ' ' << d;
            out << '\n';
        }
    }
};

// The judge supplies one case; malformed input is reported separately from infeasibility.
int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    Solver solver;
    if (!solver.read(cin)) { cerr << "Invalid input\n"; return 1; }
    solver.solve(cout);
}
