#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
using namespace std;

const int UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3;  // same order as the output

struct Vertex {
    int r, c, degree;
    vector<int> edges;  // candidate edges touching this connector
};

struct Edge {
    int a, b;    // the two connectors
    int da, db;  // direction of the edge seen from a and from b
    vector<int> conflicts;  // edges crossing this one at an empty cell
};

// literal 2*v means "edge v is used", 2*v+1 means "edge v is not used"
int yes(int v) { return 2 * v; }
int no(int v) { return 2 * v + 1; }

// SAT solver: search with backtracking, and learn a new clause after each conflict
class BooleanSolver {
    vector<vector<int>> clauses, watches;
    vector<int> values, levels, reasons, trail, boundaries, phase;
    vector<double> activity;
    size_t head = 0;
    double increment = 1.0;
    bool consistent = true;

    // value of a literal: -1 unknown, 0 false, 1 true
    int value(int literal) const {
        int v = values[literal / 2];
        if (v < 0) return -1;
        return v ^ (literal & 1);
    }

    // set a literal to true and remember which clause forced it
    bool enqueue(int literal, int reason) {
        if (value(literal) >= 0) return value(literal) == 1;
        int v = literal / 2;
        values[v] = 1 ^ (literal & 1);
        phase[v] = values[v];
        levels[v] = (int)boundaries.size();
        reasons[v] = reason;
        trail.push_back(literal);
        return true;
    }

    // unit propagation: each clause watches two literals, and we only check a
    // clause when one of its watched literals becomes false
    int propagate() {
        while (head < trail.size()) {
            int false_literal = trail[head] ^ 1;
            head++;
            vector<int> &watch = watches[false_literal];
            size_t i = 0;
            while (i < watch.size()) {
                int id = watch[i];
                vector<int> &clause = clauses[id];
                if (clause[0] == false_literal) swap(clause[0], clause[1]);
                if (value(clause[0]) == 1) {
                    i++;
                    continue;
                }
                size_t k = 2;
                while (k < clause.size() && value(clause[k]) == 0) k++;
                if (k < clause.size()) {
                    // found another literal to watch
                    swap(clause[1], clause[k]);
                    watches[clause[1]].push_back(id);
                    watch[i] = watch.back();
                    watch.pop_back();
                } else {
                    // clause[0] must be true; if it is false we get a conflict
                    i++;
                    if (!enqueue(clause[0], id)) return id;
                }
            }
        }
        return -1;
    }

    // undo all values set after the given decision level
    void backtrack(int level) {
        if ((int)boundaries.size() <= level) return;
        size_t mark = boundaries[level];
        for (size_t i = mark; i < trail.size(); i++) {
            int v = trail[i] / 2;
            values[v] = -1;
            reasons[v] = -1;
        }
        trail.resize(mark);
        boundaries.resize(level);
        head = min(head, mark);
    }

    // find the cause of a conflict and build a new clause from it;
    // back_level is the level we go back to
    vector<int> analyze(int conflict, int &back_level) {
        vector<int> learned(1, -1);
        vector<bool> seen(values.size(), false);
        int open = 0, p = -1;
        int index = (int)trail.size() - 1;
        do {
            for (int q : clauses[conflict]) {
                int v = q / 2;
                if ((p >= 0 && v == p / 2) || seen[v] || levels[v] == 0) continue;
                seen[v] = true;
                activity[v] += increment;  // variables in conflicts are tried earlier
                if (activity[v] > 1e100) {
                    for (double &score : activity) score *= 1e-100;
                    increment *= 1e-100;
                }
                if (levels[v] == (int)boundaries.size()) open++;
                else learned.push_back(q);
            }
            // replace the latest literal of this level by its reason
            while (!seen[trail[index] / 2]) index--;
            p = trail[index];
            index--;
            seen[p / 2] = false;
            open--;
            conflict = reasons[p / 2];
        } while (open > 0);
        learned[0] = p ^ 1;
        back_level = 0;
        for (size_t i = 1; i < learned.size(); i++) {
            if (levels[learned[i] / 2] > back_level) {
                back_level = levels[learned[i] / 2];
                swap(learned[1], learned[i]);
            }
        }
        increment /= 0.95;
        return learned;
    }

public:
    explicit BooleanSolver(int count) : watches(2 * count), values(count, -1),
        levels(count), reasons(count, -1), phase(count, 1), activity(count, 0) {}

    // add a clause; a clause with one literal is set at once
    int add_clause(const vector<int> &clause) {
        int id = (int)clauses.size();
        clauses.push_back(clause);
        if (clause.empty()) {
            consistent = false;
        } else if (clause.size() == 1) {
            consistent = enqueue(clause[0], id) && consistent;
        } else {
            watches[clause[0]].push_back(id);
            watches[clause[1]].push_back(id);
        }
        for (int p : clause) activity[p / 2] += 1.0 / max(size_t(1), clause.size());
        return id;
    }

    // main search loop, returns true if all clauses can be satisfied
    bool solve() {
        if (!consistent) return false;
        int conflicts_since_restart = 0, restart_limit = 100;
        while (true) {
            int conflict = propagate();
            if (conflict >= 0) {
                if (boundaries.empty()) return false;  // conflict without any guess
                int back_level;
                vector<int> learned = analyze(conflict, back_level);
                backtrack(back_level);
                int id = add_clause(learned);
                if (!consistent || !enqueue(learned[0], id)) return false;
                // restart sometimes, but keep the learned clauses
                if (++conflicts_since_restart >= restart_limit) {
                    backtrack(0);
                    conflicts_since_restart = 0;
                    restart_limit = min(10000, restart_limit + restart_limit / 2);
                }
                continue;
            }
            // guess a value for the free variable with the highest score
            int best = -1;
            for (int v = 0; v < (int)values.size(); v++) {
                if (values[v] == -1 && (best < 0 || activity[v] > activity[best])) best = v;
            }
            if (best < 0) return true;
            boundaries.push_back((int)trail.size());
            enqueue(phase[best] ? yes(best) : no(best), -1);
        }
    }

    bool selected(int v) const { return values[v] == 1; }
};

class Solver {
    int n, m;
    vector<Vertex> vertices;
    vector<Edge> edges;
    vector<vector<int>> ids, horizontal, vertical;

    // add an edge between connectors a and b, and mark the empty cells it passes
    void add_edge(int a, int b, int da, int db) {
        int e = (int)edges.size();
        edges.push_back({a, b, da, db, {}});
        vertices[a].edges.push_back(e);
        vertices[b].edges.push_back(e);
        const Vertex &u = vertices[a];
        const Vertex &v = vertices[b];
        if (u.r == v.r) {
            for (int c = u.c + 1; c < v.c; c++) horizontal[u.r][c] = e;
        } else {
            for (int r = u.r + 1; r < v.r; r++) vertical[r][u.c] = e;
        }
    }

    // degree sum check for each connected part: the sum must be even, and if
    // the part is bipartite the two colors must have the same sum
    bool feasible_components() const {
        vector<int> color(vertices.size(), -1);
        vector<int> queue;
        for (int root = 0; root < (int)vertices.size(); root++) {
            if (color[root] != -1) continue;
            queue.clear();
            queue.push_back(root);
            color[root] = 0;
            int sum[2] = {0, 0};
            bool bipartite = true;
            for (size_t i = 0; i < queue.size(); i++) {
                int v = queue[i];
                sum[color[v]] += vertices[v].degree;
                for (int e : vertices[v].edges) {
                    int w = edges[e].a ^ edges[e].b ^ v;
                    if (color[w] == -1) {
                        color[w] = color[v] ^ 1;
                        queue.push_back(w);
                    } else if (color[w] == color[v]) {
                        bipartite = false;
                    }
                }
            }
            if ((sum[0] + sum[1]) % 2 != 0) return false;
            if (bipartite && sum[0] != sum[1]) return false;
        }
        return true;
    }

    // clauses for "exactly degree edges are used" at connector v:
    // no degree+1 edges are all used, and no d-degree+1 edges are all unused
    void add_degree_clauses(BooleanSolver &solver, const Vertex &v) {
        int d = (int)v.edges.size();
        for (int mask = 1; mask < (1 << d); mask++) {
            int size = 0;
            for (int i = 0; i < d; i++) size += (mask >> i) & 1;
            if (size == v.degree + 1) {
                vector<int> clause;
                for (int i = 0; i < d; i++)
                    if ((mask >> i) & 1) clause.push_back(no(v.edges[i]));
                solver.add_clause(clause);
            }
            if (size == d - v.degree + 1) {
                vector<int> clause;
                for (int i = 0; i < d; i++)
                    if ((mask >> i) & 1) clause.push_back(yes(v.edges[i]));
                solver.add_clause(clause);
            }
        }
    }

public:
    // read the grid and build the candidate edges and crossing pairs
    bool read(istream &in) {
        if (!(in >> n >> m) || n <= 1 || n > 50 || m <= 1 || m > 50) return false;
        ids.assign(n, vector<int>(m, -1));
        horizontal = ids;
        vertical = ids;
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < m; c++) {
                int k;
                if (!(in >> k) || k < 0 || k > 4) return false;
                if (k > 0) {
                    ids[r][c] = (int)vertices.size();
                    vertices.push_back({r, c, k, {}});
                }
            }
        }
        // join neighbouring connectors in each row, then in each column
        for (int r = 0; r < n; r++) {
            int prev = -1;
            for (int c = 0; c < m; c++) {
                if (ids[r][c] < 0) continue;
                if (prev >= 0) add_edge(prev, ids[r][c], RIGHT, LEFT);
                prev = ids[r][c];
            }
        }
        for (int c = 0; c < m; c++) {
            int prev = -1;
            for (int r = 0; r < n; r++) {
                if (ids[r][c] < 0) continue;
                if (prev >= 0) add_edge(prev, ids[r][c], DOWN, UP);
                prev = ids[r][c];
            }
        }
        // an empty cell with both a horizontal and a vertical edge is a crossing
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < m; c++) {
                int h = horizontal[r][c], v = vertical[r][c];
                if (h >= 0 && v >= 0) {
                    edges[h].conflicts.push_back(v);
                    edges[v].conflicts.push_back(h);
                }
            }
        }
        return true;
    }

    // build the clauses, run the SAT solver and print the answer
    void solve(ostream &out) {
        if (!feasible_components()) {
            out << "No Solution\n";
            return;
        }
        BooleanSolver solver((int)edges.size());
        for (const Vertex &v : vertices) {
            if (v.degree > (int)v.edges.size()) {
                out << "No Solution\n";
                return;
            }
            add_degree_clauses(solver, v);
        }
        for (int e = 0; e < (int)edges.size(); e++) {
            for (int f : edges[e].conflicts) {
                if (e < f) solver.add_clause({no(e), no(f)});  // not both crossing edges
            }
        }
        if (!solver.solve()) {
            out << "No Solution\n";
            return;
        }
        // vertices are stored in row-major order, which is the output order
        for (const Vertex &v : vertices) {
            array<int, 4> directions{};
            int id = ids[v.r][v.c];
            for (int e : v.edges) {
                if (!solver.selected(e)) continue;
                if (edges[e].a == id) directions[edges[e].da] = 1;
                else directions[edges[e].db] = 1;
            }
            out << v.r + 1 << ' ' << v.c + 1;
            for (int d : directions) out << ' ' << d;
            out << '\n';
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver solver;
    if (!solver.read(cin)) {
        cerr << "Invalid input\n";
        return 1;
    }
    solver.solve(cout);
    return 0;
}
