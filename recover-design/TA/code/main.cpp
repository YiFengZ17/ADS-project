// Project 3: Recover the Design
//
// Main idea of this program:
// 1. Every connector is a vertex. If two connectors are in the same row (or
//    column) and there are only empty cells between them, a straight fence can
//    join them. We call such a pair a candidate edge.
// 2. Every candidate edge gets a 0/1 variable: 1 means we build this fence.
// 3. The rules of the problem are written as clauses. A clause is a list of
//    conditions and at least one of them must be true.
//      - A connector of degree k must have exactly k chosen edges.
//      - A horizontal edge and a vertical edge can not cross at an empty cell.
// 4. Before searching, we check the degree sums of every connected part of the
//    candidate graph. Many impossible inputs are rejected here very quickly.
// 5. A small SAT solver gives every variable a value. It searches with
//    backtracking and learns a new clause from every conflict, so it does not
//    make the same mistake again.
// 6. Finally we print the four directions of every connector.

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
using namespace std;

// Index of each direction, in the same order as the output "up down left right".
const int UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3;

// A connector on the grid.
struct Vertex {
    int r, c;           // position, counted from 0
    int degree;         // number of fences this connector needs
    vector<int> edges;  // ids of the candidate edges that touch it
};

// A candidate edge between two neighbouring connectors a and b.
struct Edge {
    int a, b;               // vertex ids of the two ends
    int da, db;             // direction of the edge seen from a, and seen from b
    vector<int> conflicts;  // other edges that cross this edge at an empty cell
};

// A literal is a variable or its negation. We store variable v as two numbers:
// 2*v means "v is true" and 2*v+1 means "v is false".
// So literal ^ 1 is always the opposite literal, and literal / 2 is the variable.
int yes(int v) { return 2 * v; }
int no(int v) { return 2 * v + 1; }

// A simple CDCL (conflict driven clause learning) SAT solver.
class BooleanSolver {
    vector<vector<int>> clauses;  // all clauses, given ones and learned ones
    vector<vector<int>> watches;  // watches[lit] = ids of clauses that watch lit
    vector<int> values;           // -1 = no value yet, otherwise 0 or 1
    vector<int> levels;           // decision level when the variable got its value
    vector<int> reasons;          // clause that forced the value, -1 for a decision
    vector<int> trail;            // literals made true, in the order they were set
    vector<int> boundaries;       // boundaries[i] = trail size when decision i+1 was made
    vector<int> phase;            // last value of each variable, reused by decisions
    vector<double> activity;      // score of each variable, higher = decide earlier
    size_t head = 0;              // trail[head..] still need to be propagated
    double increment = 1.0;       // how much a conflict adds to a score
    bool consistent = true;       // false if the given clauses already contradict

    // Return the value of a literal: -1 unknown, 0 false, 1 true.
    int value(int literal) const {
        int v = values[literal / 2];
        if (v < 0) return -1;
        return v ^ (literal & 1);
    }

    // Make a literal true and remember why. Return false if it is already false.
    bool enqueue(int literal, int reason) {
        if (value(literal) >= 0) return value(literal) == 1;
        int v = literal / 2;
        values[v] = 1 ^ (literal & 1);
        phase[v] = values[v];
        levels[v] = (int)boundaries.size();  // current decision level
        reasons[v] = reason;
        trail.push_back(literal);
        return true;
    }

    // Unit propagation. Every clause with 2 or more literals watches two of
    // them: clause[0] and clause[1]. When a watched literal becomes false we
    // look for another literal to watch. If there is none, clause[0] must be
    // true. Return the id of a clause that became false, or -1 if no conflict.
    int propagate() {
        while (head < trail.size()) {
            int false_literal = trail[head] ^ 1;  // this literal just became false
            head++;
            vector<int> &watch = watches[false_literal];
            size_t i = 0;
            while (i < watch.size()) {
                int id = watch[i];
                vector<int> &clause = clauses[id];
                // keep the false literal at clause[1]
                if (clause[0] == false_literal) swap(clause[0], clause[1]);
                // the other watched literal is true, so this clause is already fine
                if (value(clause[0]) == 1) {
                    i++;
                    continue;
                }
                // look for a literal that is not false
                size_t k = 2;
                while (k < clause.size() && value(clause[k]) == 0) k++;
                if (k < clause.size()) {
                    // watch the new literal and remove the clause from this list
                    swap(clause[1], clause[k]);
                    watches[clause[1]].push_back(id);
                    watch[i] = watch.back();
                    watch.pop_back();
                } else {
                    // all other literals are false, so clause[0] must be true
                    i++;
                    if (!enqueue(clause[0], id)) return id;  // clause[0] is false too
                }
            }
        }
        return -1;
    }

    // Go back to decision level `level`: clear every value set after it.
    void backtrack(int level) {
        if ((int)boundaries.size() <= level) return;
        size_t mark = boundaries[level];  // trail size before decision level+1
        for (size_t i = mark; i < trail.size(); i++) {
            int v = trail[i] / 2;
            values[v] = -1;
            reasons[v] = -1;
        }
        trail.resize(mark);
        boundaries.resize(level);
        head = min(head, mark);
    }

    // Conflict analysis. Start from the false clause. Each literal from the
    // current level is replaced by the clause that forced it, until only one
    // literal of the current level is left (the "first UIP"). Literals from
    // earlier levels go straight into the learned clause. back_level is set to
    // the level we should jump back to.
    vector<int> analyze(int conflict, int &back_level) {
        vector<int> learned(1, -1);  // learned[0] is filled in at the end
        vector<bool> seen(values.size(), false);
        int open = 0;       // seen variables of the current level not handled yet
        int p = -1;         // the literal we just replaced by its reason
        int index = (int)trail.size() - 1;
        do {
            for (int q : clauses[conflict]) {
                int v = q / 2;
                // skip p itself, variables already seen, and level 0 facts
                if ((p >= 0 && v == p / 2) || seen[v] || levels[v] == 0) continue;
                seen[v] = true;
                // variables in conflicts get a higher score
                activity[v] += increment;
                if (activity[v] > 1e100) {
                    // scale all scores down so the numbers do not overflow
                    for (double &score : activity) score *= 1e-100;
                    increment *= 1e-100;
                }
                if (levels[v] == (int)boundaries.size()) open++;  // handle it later
                else learned.push_back(q);                        // keep it in the clause
            }
            // go back along the trail to the latest variable we have seen
            while (!seen[trail[index] / 2]) index--;
            p = trail[index];
            index--;
            seen[p / 2] = false;
            open--;
            conflict = reasons[p / 2];  // next we look at the reason of p
        } while (open > 0);
        learned[0] = p ^ 1;  // the last current-level literal, negated
        // jump back to the highest level among the other literals,
        // and move that literal to learned[1] so it is watched
        back_level = 0;
        for (size_t i = 1; i < learned.size(); i++) {
            if (levels[learned[i] / 2] > back_level) {
                back_level = levels[learned[i] / 2];
                swap(learned[1], learned[i]);
            }
        }
        increment /= 0.95;  // later conflicts count a bit more
        return learned;
    }

public:
    // Create a solver for `count` variables. Every variable has no value yet
    // and first tries the value 1 (edge chosen).
    explicit BooleanSolver(int count) : watches(2 * count), values(count, -1),
        levels(count), reasons(count, -1), phase(count, 1), activity(count, 0) {}

    // Add a clause and return its id. An empty clause means no solution.
    // A clause with one literal is set to true at once.
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
        // variables in short clauses start with a slightly higher score
        for (int p : clause) activity[p / 2] += 1.0 / max(size_t(1), clause.size());
        return id;
    }

    // Main search loop. Return true if every clause can be satisfied.
    bool solve() {
        if (!consistent) return false;
        int conflicts_since_restart = 0, restart_limit = 100;
        while (true) {
            int conflict = propagate();
            if (conflict >= 0) {
                // a conflict with no decision made means there is no solution
                if (boundaries.empty()) return false;
                int back_level;
                vector<int> learned = analyze(conflict, back_level);
                backtrack(back_level);
                int id = add_clause(learned);
                // after the jump, learned[0] is the only literal that can be true
                if (!consistent || !enqueue(learned[0], id)) return false;
                // restart from level 0 sometimes; learned clauses are kept
                if (++conflicts_since_restart >= restart_limit) {
                    backtrack(0);
                    conflicts_since_restart = 0;
                    restart_limit = min(10000, restart_limit + restart_limit / 2);
                }
                continue;
            }
            // no conflict: choose the unassigned variable with the highest score
            int best = -1;
            for (int v = 0; v < (int)values.size(); v++) {
                if (values[v] == -1 && (best < 0 || activity[v] > activity[best])) best = v;
            }
            if (best < 0) return true;  // every variable has a value: done
            // start a new decision level and use the value it had last time
            boundaries.push_back((int)trail.size());
            enqueue(phase[best] ? yes(best) : no(best), -1);
        }
    }

    // After solve() returns true: is edge v chosen?
    bool selected(int v) const { return values[v] == 1; }
};

// Reads the grid, builds the candidate graph and prints the answer.
class Solver {
    int n, m;
    vector<Vertex> vertices;
    vector<Edge> edges;
    vector<vector<int>> ids;         // ids[r][c] = vertex id, or -1 for an empty cell
    vector<vector<int>> horizontal;  // horizontal edge passing an empty cell, or -1
    vector<vector<int>> vertical;    // vertical edge passing an empty cell, or -1

    // Add an edge from vertex a to vertex b, and mark the empty cells it passes.
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

    // Quick check before searching. Every chosen edge adds 1 to the degree of
    // both ends, so in each connected part the degree sum must be even. If the
    // part is bipartite (2-colorable), every edge has one end of each color, so
    // the two colors must have the same degree sum.
    bool feasible_components() const {
        vector<int> color(vertices.size(), -1);
        vector<int> queue;
        for (int root = 0; root < (int)vertices.size(); root++) {
            if (color[root] != -1) continue;
            // BFS over one connected part and color it with 0 and 1
            queue.clear();
            queue.push_back(root);
            color[root] = 0;
            int sum[2] = {0, 0};  // degree sum of each color
            bool bipartite = true;
            for (size_t i = 0; i < queue.size(); i++) {
                int v = queue[i];
                sum[color[v]] += vertices[v].degree;
                for (int e : vertices[v].edges) {
                    int w = edges[e].a ^ edges[e].b ^ v;  // the other end of edge e
                    if (color[w] == -1) {
                        color[w] = color[v] ^ 1;
                        queue.push_back(w);
                    } else if (color[w] == color[v]) {
                        bipartite = false;  // found an odd cycle
                    }
                }
            }
            // We color the real graph, not the grid (r + c) parity, because a
            // long edge can join two cells with the same parity.
            if ((sum[0] + sum[1]) % 2 != 0) return false;
            if (bipartite && sum[0] != sum[1]) return false;
        }
        return true;
    }

    // Add clauses so that vertex v has exactly `degree` chosen edges.
    // Let d be its number of candidate edges and k its degree:
    //   - any k+1 of its edges can not all be chosen   (one of them is false)
    //   - any d-k+1 of its edges can not all be unused (one of them is true)
    // d is at most 4, so we simply try every subset with a bit mask.
    void add_degree_clauses(BooleanSolver &solver, const Vertex &v) {
        int d = (int)v.edges.size();
        for (int mask = 1; mask < (1 << d); mask++) {
            int size = 0;  // number of edges in this subset
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
    // Read the grid. Then join neighbouring connectors in every row and column,
    // and record the pairs of edges that cross at an empty cell.
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
        // each row: join every connector with the next one on its right
        for (int r = 0; r < n; r++) {
            int prev = -1;
            for (int c = 0; c < m; c++) {
                if (ids[r][c] < 0) continue;
                if (prev >= 0) add_edge(prev, ids[r][c], RIGHT, LEFT);
                prev = ids[r][c];
            }
        }
        // each column: join every connector with the next one below it
        for (int c = 0; c < m; c++) {
            int prev = -1;
            for (int r = 0; r < n; r++) {
                if (ids[r][c] < 0) continue;
                if (prev >= 0) add_edge(prev, ids[r][c], DOWN, UP);
                prev = ids[r][c];
            }
        }
        // an empty cell passed by both a horizontal and a vertical edge
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

    // Build all clauses, run the SAT solver and print the result.
    void solve(ostream &out) {
        if (!feasible_components()) {
            out << "No Solution\n";
            return;
        }
        BooleanSolver solver((int)edges.size());
        for (const Vertex &v : vertices) {
            // not enough candidate edges for this degree
            if (v.degree > (int)v.edges.size()) {
                out << "No Solution\n";
                return;
            }
            add_degree_clauses(solver, v);
        }
        // two crossing edges can not both be chosen
        for (int e = 0; e < (int)edges.size(); e++) {
            for (int f : edges[e].conflicts) {
                if (e < f) solver.add_clause({no(e), no(f)});
            }
        }
        if (!solver.solve()) {
            out << "No Solution\n";
            return;
        }
        // vertices were created in row-major order, so printing them in order
        // gives the required output order
        for (const Vertex &v : vertices) {
            array<int, 4> directions{};  // up, down, left, right
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

// Read one test case and print one valid design or "No Solution".
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver solver;
    if (!solver.read(cin)) {
        cerr << "Invalid input\n";  // bad input format, not the same as no solution
        return 1;
    }
    solver.solve(cout);
    return 0;
}
