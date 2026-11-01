#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex { int r, c, degree; vector<int> edges; };
struct Edge { int a, b, da, db; vector<int> conflicts; };

// Conflict-driven Boolean search; literals 2*v and 2*v+1 mean v and not v.
class BooleanSolver {
    vector<vector<int>> clauses, watches;
    vector<int> values, levels, reasons, trail, boundaries, phase;
    vector<double> activity;
    size_t head = 0;
    double increment = 1.0;
    bool consistent = true;

    // Return -1 for unknown, otherwise the truth value of a literal.
    int value(int literal) const {
        int v = values[literal / 2];
        return v < 0 ? -1 : v ^ (literal & 1);
    }
    // Record an implication and the clause that justifies it.
    bool enqueue(int literal, int reason) {
        if (value(literal) >= 0) return value(literal) == 1;
        int v = literal / 2;
        values[v] = 1 ^ (literal & 1); phase[v] = values[v];
        levels[v] = (int)boundaries.size(); reasons[v] = reason;
        trail.push_back(literal); return true;
    }
    // Only clauses watching a newly false literal need to be inspected.
    int propagate() {
        while (head < trail.size()) {
            int false_literal = trail[head++] ^ 1;
            auto &watch = watches[false_literal];
            size_t i = 0;
            while (i < watch.size()) {
                int id = watch[i]; auto &clause = clauses[id];
                if (clause[0] == false_literal) swap(clause[0], clause[1]);
                if (value(clause[0]) == 1) { ++i; continue; }
                size_t k = 2;
                while (k < clause.size() && value(clause[k]) == 0) ++k;
                if (k < clause.size()) {
                    swap(clause[1], clause[k]);
                    watches[clause[1]].push_back(id);
                    watch[i] = watch.back(); watch.pop_back();
                } else {
                    ++i;
                    if (!enqueue(clause[0], id)) return id;
                }
            }
        }
        return -1;
    }
    // Undo decisions above the target level, retaining root-level implications.
    void backtrack(int level) {
        if ((int)boundaries.size() <= level) return;
        size_t mark = boundaries[level];
        for (size_t i = mark; i < trail.size(); ++i) {
            int v = trail[i] / 2; values[v] = -1; reasons[v] = -1;
        }
        trail.resize(mark); boundaries.resize(level); head = min(head, mark);
    }
    // Resolve reasons backwards to the first unique implication point.
    vector<int> analyze(int conflict, int &back_level) {
        vector<int> learned(1, -1);
        vector<bool> seen(values.size(), false);
        int open = 0, p = -1, index = (int)trail.size() - 1;
        do {
            for (int q : clauses[conflict]) {
                int v = q / 2;
                if ((p >= 0 && v == p / 2) || seen[v] || levels[v] == 0) continue;
                seen[v] = true; activity[v] += increment;
                if (activity[v] > 1e100) {
                    for (double &score : activity) score *= 1e-100;
                    increment *= 1e-100;
                }
                if (levels[v] == (int)boundaries.size()) ++open;
                else learned.push_back(q);
            }
            while (!seen[trail[index] / 2]) --index;
            p = trail[index--]; seen[p / 2] = false; --open;
            conflict = reasons[p / 2];
        } while (open > 0);
        learned[0] = p ^ 1; back_level = 0;
        for (size_t i = 1; i < learned.size(); ++i)
            if (levels[learned[i] / 2] > back_level) {
                back_level = levels[learned[i] / 2]; swap(learned[1], learned[i]);
            }
        increment /= 0.95;
        return learned;
    }
public:
    // Allocate fixed variable-indexed storage; all variables initially prefer selection.
    explicit BooleanSolver(int count) : watches(2 * count), values(count, -1),
        levels(count), reasons(count, -1), phase(count, 1), activity(count, 0) {}

    // Store a non-tautological clause; single literals are root-level requirements.
    int add_clause(const vector<int> &clause) {
        int id = (int)clauses.size(); clauses.push_back(clause);
        if (clause.empty()) consistent = false;
        else if (clause.size() == 1) consistent = enqueue(clause[0], id) && consistent;
        else {
            watches[clause[0]].push_back(id); watches[clause[1]].push_back(id);
        }
        for (int p : clause) activity[p / 2] += 1.0 / max(size_t(1), clause.size());
        return id;
    }
    // Learn from each conflict and jump directly to its relevant earlier decision.
    bool solve() {
        if (!consistent) return false;
        int conflicts_since_restart = 0, restart_limit = 100;
        while (true) {
            int conflict = propagate();
            if (conflict >= 0) {
                if (boundaries.empty()) return false;
                int back_level;
                vector<int> learned = analyze(conflict, back_level);
                backtrack(back_level);
                int id = add_clause(learned);
                if (!consistent || !enqueue(learned[0], id)) return false;
                if (++conflicts_since_restart >= restart_limit) {
                    backtrack(0); conflicts_since_restart = 0;
                    restart_limit = min(10000, restart_limit + restart_limit / 2);
                }
                continue;
            }
            int best = -1;
            for (int v = 0; v < (int)values.size(); ++v)
                if (values[v] == -1 && (best < 0 || activity[v] > activity[best])) best = v;
            if (best < 0) return true;
            boundaries.push_back((int)trail.size());
            enqueue(2 * best + (phase[best] ? 0 : 1), -1);
        }
    }
    // Expose the selected-edge assignment only after successful solving.
    bool selected(int v) const { return values[v] == 1; }
};

class Solver {
    int n, m;
    vector<Vertex> vertices;
    vector<Edge> edges;
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

    // Candidate edges must supply an even degree sum within each component.
    bool feasible_components() const {
        vector<int> color(vertices.size(), -1);
        vector<int> queue;
        for (int root = 0; root < (int)vertices.size(); ++root) {
            if (color[root] != -1) continue;
            queue.clear(); queue.push_back(root); color[root] = 0;
            int sum[2] = {0, 0};
            bool bipartite = true;
            for (size_t i = 0; i < queue.size(); ++i) {
                int v = queue[i];
                sum[color[v]] += vertices[v].degree;
                for (int e : vertices[v].edges) {
                    int w = edges[e].a ^ edges[e].b ^ v;
                    if (color[w] == -1) {
                        color[w] = color[v] ^ 1; queue.push_back(w);
                    } else if (color[w] == color[v]) bipartite = false;
                }
            }
            // Color the actual graph: long segments need not change grid parity.
            if ((sum[0] + sum[1]) % 2 || (bipartite && sum[0] != sum[1])) return false;
        }
        return true;
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
        return true;
    }
    // Print one legal solution in row-major order, or the required failure text.
    void solve(ostream &out) {
        if (!feasible_components()) { out << "No Solution\n"; return; }
        BooleanSolver solver((int)edges.size());
        for (const auto &v : vertices) {
            int count = (int)v.edges.size();
            if (v.degree > count) { out << "No Solution\n"; return; }
            // Forbid every subset that would contain too many selected/excluded edges.
            for (int mask = 1; mask < (1 << count); ++mask) {
                int bits = 0;
                for (int i = 0; i < count; ++i) bits += (mask >> i) & 1;
                for (int excluded = 0; excluded < 2; ++excluded) {
                    int forbidden = excluded ? count - v.degree + 1 : v.degree + 1;
                    if (bits != forbidden) continue;
                    vector<int> clause;
                    for (int i = 0; i < count; ++i) if ((mask >> i) & 1)
                        clause.push_back(2 * v.edges[i] + (excluded ? 0 : 1));
                    solver.add_clause(clause);
                }
            }
        }
        for (int e = 0; e < (int)edges.size(); ++e)
            for (int f : edges[e].conflicts) if (e < f)
                solver.add_clause({2 * e + 1, 2 * f + 1});
        if (!solver.solve()) { out << "No Solution\n"; return; }
        for (const auto &v : vertices) {
            array<int, 4> directions{};
            int id = ids[v.r][v.c];
            for (int e : v.edges) if (solver.selected(e))
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
    return 0;
}
