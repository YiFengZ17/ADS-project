#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;
using i64 = int64_t;
static constexpr i64 INF = numeric_limits<i64>::max()/4;
struct Edge { int to, weight; };
struct Graph { vector<vector<Edge>> adj; int64_t arcs=0; };

// Read the directed, nonnegative DIMACS shortest-path graph.
Graph read_graph(const string& path) {
    ifstream in(path); if (!in) throw runtime_error("cannot open graph: " + path);
    Graph g; string line; bool has_header=false; int64_t count=0;
    while (getline(in,line)) {
        istringstream record(line); string tag;
        if (!(record >> tag)) continue;
        if (tag == "c") continue;
        string extra;
        if (tag == "p") {
            // A second header would discard or reinterpret previously read arcs.
            if (has_header) throw runtime_error("duplicate DIMACS header");
            string kind; int n; int64_t m;
            if (!(record >> kind >> n >> m) || (record >> extra) || kind != "sp" || n <= 0 || m < 0)
                throw runtime_error("invalid DIMACS header");
            g.adj.resize(n); g.arcs=m; has_header=true;
        } else if (tag == "a") {
            // Parse each line independently so a missing field cannot consume the next record.
            if (!has_header) throw runtime_error("arc before DIMACS header");
            int64_t u,v,w;
            if (!(record >> u >> v >> w) || (record >> extra))
                throw runtime_error("malformed arc record");
            if (u<1 || v<1 || u>(int64_t)g.adj.size() || v>(int64_t)g.adj.size() ||
                w<0 || w>numeric_limits<int>::max()) throw runtime_error("invalid arc");
            g.adj[u-1].push_back({static_cast<int>(v-1),static_cast<int>(w)});
            if (++count > g.arcs) throw runtime_error("arc count mismatch");
        } else throw runtime_error("unknown DIMACS record");
    }
    if (in.bad()) throw runtime_error("graph read failure");
    if (!has_header) throw runtime_error("missing DIMACS header");
    if (count != g.arcs) throw runtime_error("arc count mismatch");
    return g;
}

// Standard lazy binary heap: stale copies are discarded on extraction.
i64 binary_dijkstra(const Graph& g, int source, int target) {
    vector<i64> dist(g.adj.size(),INF);
    priority_queue<pair<i64,int>,vector<pair<i64,int>>,greater<pair<i64,int>>> heap;
    dist[source]=0; heap.push({0,source});
    while (!heap.empty()) {
        auto [d,u]=heap.top(); heap.pop();
        if (d!=dist[u]) continue;
        if (u==target) return d;
        for (auto e:g.adj[u]) if (d+e.weight<dist[e.to]) {
            dist[e.to]=d+e.weight; heap.push({dist[e.to],e.to});
        }
    }
    return INF;
}

// Intrusive Fibonacci heap nodes; one node per discovered vertex.
struct Node { i64 key=INF; int vertex=-1, degree=0; bool mark=false;
    Node *parent=nullptr,*child=nullptr,*left=this,*right=this; };
class FibHeap {
    Node* minimum=nullptr; size_t count=0;
    // Remove one node from its circular sibling ring.
    static void detach(Node* x) { x->left->right=x->right; x->right->left=x->left; x->left=x->right=x; }
    // Splice a singleton after a node already in a ring.
    static void insert_ring(Node* at, Node* x) { x->right=at->right; x->left=at; at->right->left=x; at->right=x; }
    // Add a tree root and refresh the minimum pointer.
    void add_root(Node* x) { x->parent=nullptr; x->mark=false;
        if (!minimum) minimum=x; else { insert_ring(minimum,x); if (x->key<minimum->key) minimum=x; } }
    // Make the larger-key root a child of the smaller-key root.
    void link(Node* child,Node* parent) { detach(child); child->parent=parent; child->mark=false;
        if (!parent->child) parent->child=child; else insert_ring(parent->child,child); ++parent->degree; }
    // Coalesce equal-degree roots after extracting the minimum.
    void consolidate() {
        // Snapshot roots before detach/link changes their circular list.
        vector<Node*> roots; Node* p=minimum;
        do { roots.push_back(p); p=p->right; } while(p!=minimum);
        vector<Node*> by_degree(64,nullptr); minimum=nullptr;
        for (Node* x:roots) {
            // A root may have been linked under another root earlier in this pass.
            if (x->parent) continue;
            detach(x);
            while (true) {
                if ((size_t)x->degree>=by_degree.size()) by_degree.resize(x->degree+1,nullptr);
                Node* y=by_degree[x->degree]; if (!y) break;
                by_degree[x->degree]=nullptr;
                if (y->key<x->key) swap(x,y);
                link(y,x);
            }
            by_degree[x->degree]=x;
        }
        for (Node* x:by_degree) if (x) add_root(x);
    }
    // Move a violating child to the root list.
    void cut(Node* x,Node* parent) {
        if (parent->child==x) parent->child=(x->right==x ? nullptr : x->right);
        detach(x); --parent->degree; add_root(x);
    }
    // Apply the mark rule and continue cuts toward the root when needed.
    void cascading_cut(Node* x) { Node* p=x->parent; if (!p) return;
        if (!x->mark) x->mark=true; else { cut(x,p); cascading_cut(p); } }
public:
    // Report whether any unsettled vertex remains in the queue.
    bool empty() const { return minimum==nullptr; }
    // Insert a newly discovered vertex.
    void push(Node* x) { add_root(x); ++count; }
    // Extract and return the current minimum vertex.
    Node* pop() {
        Node* z=minimum;
        if (!z) return nullptr;
        if (z->child) {
            // Snapshot children before moving them into the root ring.
            vector<Node*> children; Node* p=z->child;
            do { children.push_back(p); p=p->right; } while(p!=z->child);
            for(Node* x:children) { detach(x); add_root(x); }
            z->child=nullptr;
        }
        Node* next=(z->right==z ? nullptr : z->right); detach(z); --count;
        minimum=next; if (minimum) consolidate();
        return z;
    }
    // Lower one vertex's tentative distance and restore heap order.
    void decrease(Node* x,i64 key) {
        if (key>x->key) throw runtime_error("increase-key requested");
        x->key=key; Node* p=x->parent;
        if (p && x->key<p->key) { cut(x,p); cascading_cut(p); }
        if (x->key<minimum->key) minimum=x;
    }
};

// Decrease an existing vertex key instead of creating duplicate heap entries.
i64 fib_dijkstra(const Graph& g,int source,int target) {
    vector<i64> dist(g.adj.size(),INF);
    // State 0: unseen; 1: active in heap; 2: settled. The fixed-size node array
    // keeps pointers stored in the heap stable for the entire query.
    vector<Node> nodes(g.adj.size()); vector<uint8_t> state(g.adj.size(),0);
    FibHeap heap; nodes[source].key=0; nodes[source].vertex=source;
    dist[source]=0; state[source]=1; heap.push(&nodes[source]);
    while (!heap.empty()) {
        Node* cur=heap.pop(); int u=cur->vertex; state[u]=2;
        if (u==target) return dist[u];
        for (auto e:g.adj[u]) {
            if (state[e.to]==2) continue;
            i64 candidate=dist[u]+e.weight;
            if (candidate<dist[e.to]) {
                dist[e.to]=candidate;
                if (state[e.to]==0) { Node* x=&nodes[e.to]; x->vertex=e.to; x->key=candidate;
                    state[e.to]=1; heap.push(x); }
                else heap.decrease(&nodes[e.to],candidate);
            }
        }
    }
    return INF;
}

// Verify hand-constructed edge cases before loading any benchmark graph.
void self_test() {
    Graph g; g.adj={{{1,3},{2,10}},{{2,4}},{{3,0}}, {}, {}};
    for (auto f:{binary_dijkstra,fib_dijkstra}) {
        if (f(g,0,3)!=7 || f(g,0,0)!=0 || f(g,0,4)!=INF || f(g,2,3)!=0)
            throw runtime_error("self-test failed");
    }
    Graph large; large.adj={{{1,2000000000}},{{2,2000000000}}, {}};
    for (auto f:{binary_dijkstra,fib_dijkstra})
        if (f(large,0,2)!=4000000000LL) throw runtime_error("64-bit distance failed");
    mt19937 rng(42);
    for(int trial=0;trial<100;trial++) {
        Graph a; a.adj.resize(30);
        for(int i=0;i<200;i++) {int u=rng()%30,v=rng()%30,w=rng()%50; a.adj[u].push_back({v,w});}
        for(int j=0;j<20;j++) {int s=rng()%30,t=rng()%30;
            if(binary_dijkstra(a,s,t)!=fib_dijkstra(a,s,t)) throw runtime_error("random graph mismatch");}
    }
}
// Run correctness tests or time both queues on the same generated query list.
int main(int argc,char** argv) {
    try {
        if (argc==2 && string(argv[1])=="--self-test") {self_test(); cout<<"PASS: edge cases and 2000 random pairs\n"; return 0;}
        if (argc==3 && string(argv[1])=="--validate") {
            Graph g=read_graph(argv[2]); cout<<g.adj.size()<<" vertices, "<<g.arcs<<" arcs\n"; return 0;
        }
        if (argc!=4) {cerr<<"usage: benchmark graph.gr queries seed\n"; return 2;}
        Graph g=read_graph(argv[1]); int q=stoi(argv[2]); unsigned seed=stoul(argv[3]);
        if(q<1000) throw runtime_error("at least 1000 queries required");
        mt19937 rng(seed); uniform_int_distribution<int> pick(0,(int)g.adj.size()-1);
        vector<pair<int,int>> pairs; pairs.reserve(q);
        for(int i=0;i<q;i++) pairs.push_back({pick(rng),pick(rng)});
        cout<<"method,query,source,target,distance,seconds,nodes,arcs,seed\n";
        vector<i64> expected(q);
        for(int method=0;method<2;method++) for(int i=0;i<q;i++) {
            auto [s,t]=pairs[i]; auto start=chrono::steady_clock::now();
            i64 d=method==0 ? binary_dijkstra(g,s,t) : fib_dijkstra(g,s,t);
            double seconds=chrono::duration<double>(chrono::steady_clock::now()-start).count();
            if(method==0) expected[i]=d;
            else if(d!=expected[i]) throw runtime_error("distance mismatch at query " + to_string(i));
            cout<<(method==0?"binary":"fibonacci")<<','<<i<<','<<s+1<<','<<t+1<<',';
            if(d==INF) cout<<"INF"; else cout<<d;
            cout<<','<<seconds<<','<<g.adj.size()<<','<<g.arcs<<','<<seed<<'\n';
        }
    } catch(const exception& e) {cerr<<e.what()<<'\n'; return 1;}
}
