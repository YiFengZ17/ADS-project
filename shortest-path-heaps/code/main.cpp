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

// Counters describing the work a Fibonacci heap performs. They are only
// collected when a pointer is supplied, so the normal benchmark path is
// unchanged.
struct HeapStats {
    long long pushes=0, pops=0, decreases=0, consolidations=0, links=0,
                cuts=0, cascading_cuts=0, root_ring_total=0, root_ring_max=0;
    // Time spent inside consolidate(), which rebuilds the whole root list.
    double consolidate_seconds=0;
};

// Intrusive Fibonacci heap nodes; one node per discovered vertex.
// A node is linked into the heap through these pointers, so the node array
// must not be resized while the heap is in use.
struct Node { i64 key=INF; int vertex=-1, degree=0; bool mark=false;
    // left and right form the circular sibling list; a lone node points at itself.
    Node *parent=nullptr,*child=nullptr,*left=this,*right=this; };
class FibHeap {
    Node* minimum=nullptr;
    HeapStats* stats=nullptr;   // null unless counting is requested.
    // Remove one node from its circular sibling ring.
    static void detach(Node* x) { x->left->right=x->right; x->right->left=x->left; x->left=x->right=x; }
    // Splice a singleton after a node already in a ring.
    static void insert_ring(Node* at, Node* x) { x->right=at->right; x->left=at; at->right->left=x; at->right=x; }
    // Add a tree root and refresh the minimum pointer.
    void add_root(Node* x) { x->parent=nullptr; x->mark=false;
        if (!minimum) minimum=x; else { insert_ring(minimum,x); if (x->key<minimum->key) minimum=x; } }
    // Make the larger-key root a child of the smaller-key root.
    void link(Node* child,Node* parent) { detach(child); child->parent=parent; child->mark=false;
        if (!parent->child) parent->child=child; else insert_ring(parent->child,child); ++parent->degree;
        if (stats) stats->links++; }
    // Coalesce equal-degree roots after extracting the minimum.
    void consolidate() {
        auto started = chrono::steady_clock::now();
        if (stats) { stats->consolidations++; }
        // Snapshot the roots first: detach and link rewrite the circular list,
        // so walking it while modifying it would skip nodes.
        vector<Node*> roots; Node* p=minimum;
        do { roots.push_back(p); p=p->right; } while(p!=minimum);
        if (stats) { stats->root_ring_total += (long long)roots.size();
                     stats->root_ring_max = max(stats->root_ring_max,(long long)roots.size()); }
        // A Fibonacci heap node has degree O(log_phi n), so 64 slots cover every
        // graph used here; the resize below keeps it correct for any input.
        vector<Node*> by_degree(64,nullptr); minimum=nullptr;
        for (Node* x:roots) {
            // A root may have been linked under another root earlier in this pass.
            if (x->parent) continue;
            detach(x);
            while (true) {
                if ((size_t)x->degree>=by_degree.size()) by_degree.resize(x->degree+1,nullptr);
                Node* y=by_degree[x->degree]; if (!y) break;
                by_degree[x->degree]=nullptr;
                // Keep the smaller key as the parent so the heap order holds.
                if (y->key<x->key) swap(x,y);
                link(y,x);
            }
            by_degree[x->degree]=x;
        }
        for (Node* x:by_degree) if (x) add_root(x);
        if (stats) stats->consolidate_seconds +=
            chrono::duration<double>(chrono::steady_clock::now()-started).count();
    }
    // Move a violating child to the root list.
    void cut(Node* x,Node* parent) {
        if (parent->child==x) parent->child=(x->right==x ? nullptr : x->right);
        detach(x); --parent->degree; add_root(x);
        if (stats) stats->cuts++;
    }
    // Apply the mark rule and continue cuts toward the root when needed.
    // An unmarked node that loses a child is marked; a marked node that loses
    // another child is cut from its own parent and the rule continues upward.
    void cascading_cut(Node* x) { Node* p=x->parent; if (!p) return;
        if (!x->mark) x->mark=true; else { cut(x,p); cascading_cut(p); if (stats) stats->cascading_cuts++; } }
public:
    explicit FibHeap(HeapStats* s=nullptr):stats(s) {}
    // Report whether any unsettled vertex remains in the queue.
    bool empty() const { return minimum==nullptr; }
    // Insert a newly discovered vertex.
    void push(Node* x) { add_root(x); if (stats) stats->pushes++; }
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
        Node* next=(z->right==z ? nullptr : z->right); detach(z);
        minimum=next; if (minimum) consolidate();
        if (stats) stats->pops++;
        return z;
    }
    // Lower one vertex's tentative distance and restore heap order.
    void decrease(Node* x,i64 key) {
        if (key>x->key) throw runtime_error("increase-key requested");
        x->key=key; Node* p=x->parent;
        // A smaller key than the parent's breaks the heap order, so cut.
        if (p && x->key<p->key) { cut(x,p); cascading_cut(p); }
        // After the cut x is a root and may now hold the smallest key.
        if (x->key<minimum->key) minimum=x;
        if (stats) stats->decreases++;
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

// Indexed binary heap: one array slot per vertex, so a shorter path updates the
// key in place instead of leaving a stale duplicate in the queue. This is the
// standard textbook binary heap, and it is the fairest comparison against the
// Fibonacci heap because both then offer a real decrease-key operation.
class IndexedHeap {
    vector<int> heap;      // Vertex ids ordered by key.
    vector<int> pos;       // pos[v] is the slot of v, or -1 when v is absent.
    vector<i64>& key;      // Current tentative distance of each vertex.
    // Restore the heap property upwards after a key at slot i got smaller.
    // The parent of slot i is (i-1)/2, so the path to the root is short.
    void sift_up(size_t i) {
        while (i>0) { size_t p=(i-1)/2; if (key[heap[p]]<=key[heap[i]]) break;
            std::swap(heap[p],heap[i]); pos[heap[p]]=p; pos[heap[i]]=i; i=p; }
    }
    // Restore the heap property downwards after the root was replaced.
    // The children of slot i are 2*i+1 and 2*i+2.
    void sift_down(size_t i) {
        for (;;) { size_t l=2*i+1, r=l+1, m=i;
            if (l<heap.size() && key[heap[l]]<key[heap[m]]) m=l;
            if (r<heap.size() && key[heap[r]]<key[heap[m]]) m=r;
            if (m==i) break;
            std::swap(heap[m],heap[i]); pos[heap[m]]=m; pos[heap[i]]=i; i=m; }
    }
public:
    IndexedHeap(vector<i64>& k,size_t n):pos(n,-1),key(k) { heap.reserve(n); }
    bool empty() const { return heap.empty(); }
    // Insert a vertex, or lower the key of a vertex already in the heap.
    // Because pos[] records the slot, a decrease-key touches one entry only.
    void push_or_decrease(int v) {
        if (pos[v]==-1) { pos[v]=(int)heap.size(); heap.push_back(v); }
        sift_up((size_t)pos[v]);
    }
    // Remove and return the vertex with the smallest key.
    int pop() {
        int top=heap[0], last=heap.back(); heap.pop_back(); pos[top]=-1;
        if (!heap.empty()) { heap[0]=last; pos[last]=0; sift_down(0); }
        return top;
    }
};

// Dijkstra with the indexed binary heap. Same graph, relaxation rule and
// early-stop condition as the other two solvers, so all three agree.
i64 idx_dijkstra(const Graph& g,int source,int target) {
    vector<i64> dist(g.adj.size(),INF);
    vector<char> settled(g.adj.size(),0);
    IndexedHeap heap(dist,g.adj.size());
    dist[source]=0; heap.push_or_decrease(source);
    while (!heap.empty()) {
        int u=heap.pop(); settled[u]=1;
        if (u==target) return dist[u];
        for (auto e:g.adj[u]) {
            if (settled[e.to]) continue;
            i64 candidate=dist[u]+e.weight;
            if (candidate<dist[e.to]) { dist[e.to]=candidate; heap.push_or_decrease(e.to); }
        }
    }
    return INF;
}

// Verify hand-constructed edge cases before loading any benchmark graph.
void self_test() {
    Graph g; g.adj={{{1,3},{2,10}},{{2,4}},{{3,0}}, {}, {}};
    for (auto f:{binary_dijkstra,fib_dijkstra,idx_dijkstra}) {
        if (f(g,0,3)!=7 || f(g,0,0)!=0 || f(g,0,4)!=INF || f(g,2,3)!=0)
            throw runtime_error("self-test failed");
    }
    Graph large; large.adj={{{1,2000000000}},{{2,2000000000}}, {}};
    for (auto f:{binary_dijkstra,fib_dijkstra,idx_dijkstra})
        if (f(large,0,2)!=4000000000LL) throw runtime_error("64-bit distance failed");
    mt19937 rng(42);
    for(int trial=0;trial<100;trial++) {
        Graph a; a.adj.resize(30);
        for(int i=0;i<200;i++) {int u=rng()%30,v=rng()%30,w=rng()%50; a.adj[u].push_back({v,w});}
        for(int j=0;j<20;j++) {int s=rng()%30,t=rng()%30;
            if(binary_dijkstra(a,s,t)!=fib_dijkstra(a,s,t) ||
               binary_dijkstra(a,s,t)!=idx_dijkstra(a,s,t)) throw runtime_error("random graph mismatch");}
    }
}
// Attribute the measured gap between the two queues to concrete causes.
// This mode separates per-query initialization from the search itself, counts
// the work each queue performs, and reports a third solver that uses an
// indexed binary heap, so the report can state where the difference comes
// from instead of only listing plausible contributors.
void attribute_report(const string& path,int q,int repeats=1) {
    Graph g=read_graph(path);
    mt19937 rng(20261016); uniform_int_distribution<int> pick(0,(int)g.adj.size()-1);
    vector<pair<int,int>> pairs; pairs.reserve(q);
    for(int i=0;i<q;i++) pairs.push_back({pick(rng),pick(rng)});

    // One measurement of all three solvers for a single query.
    struct Sample { double b_init,b_search,f_init,f_search,i_init,i_total;
                     long long b_pushes,b_pops,b_stale;
                     HeapStats stats; };
    auto measure=[&](int s,int t)->Sample {
        Sample r{};
        // ---- lazy binary heap: initialization, then search ----
        auto t0=chrono::steady_clock::now();
        vector<i64> dist(g.adj.size(),INF);
        auto t1=chrono::steady_clock::now();
        priority_queue<pair<i64,int>,vector<pair<i64,int>>,greater<pair<i64,int>>> heap;
        dist[s]=0; heap.push({0,s});
        while (!heap.empty()) {
            auto [d,u]=heap.top(); heap.pop();
            r.b_pops++; if (d!=dist[u]) { r.b_stale++; continue; }
            if (u==t) break;
            for (auto e:g.adj[u]) if (d+e.weight<dist[e.to]) {
                dist[e.to]=d+e.weight; heap.push({dist[e.to],e.to}); r.b_pushes++; }
        }
        auto t2=chrono::steady_clock::now();
        // ---- Fibonacci heap: initialization, then search ----
        auto t3=chrono::steady_clock::now();
        vector<i64> fdist(g.adj.size(),INF);
        vector<Node> nodes(g.adj.size()); vector<uint8_t> state(g.adj.size(),0);
        auto t4=chrono::steady_clock::now();
        FibHeap fh(&r.stats); nodes[s].key=0; nodes[s].vertex=s;
        fdist[s]=0; state[s]=1; fh.push(&nodes[s]);
        while (!fh.empty()) {
            Node* cur=fh.pop(); int u=cur->vertex; state[u]=2;
            if (u==t) break;
            for (auto e:g.adj[u]) {
                if (state[e.to]==2) continue;
                i64 candidate=fdist[u]+e.weight;
                if (candidate<fdist[e.to]) {
                    fdist[e.to]=candidate;
                    if (state[e.to]==0) { Node* x=&nodes[e.to]; x->vertex=e.to; x->key=candidate;
                        state[e.to]=1; fh.push(x); }
                    else fh.decrease(&nodes[e.to],candidate);
                }
            }
        }
        auto t5=chrono::steady_clock::now();
        // ---- indexed binary heap: initialization, then search ----
        auto t6=chrono::steady_clock::now();
        vector<i64> idist(g.adj.size(),INF);
        vector<char> isettled(g.adj.size(),0);
        IndexedHeap iheap(idist,g.adj.size());
        auto t7=chrono::steady_clock::now();
        idist[s]=0; iheap.push_or_decrease(s);
        while (!iheap.empty()) {
            int u=iheap.pop(); isettled[u]=1;
            if (u==t) break;
            for (auto e:g.adj[u]) {
                if (isettled[e.to]) continue;
                i64 candidate=idist[u]+e.weight;
                if (candidate<idist[e.to]) { idist[e.to]=candidate; iheap.push_or_decrease(e.to); }
            }
        }
        auto t8=chrono::steady_clock::now();
        if (idist[t]!=INF && fdist[t]!=idist[t]) throw runtime_error("attribute: indexed heap disagrees");
        r.b_init=chrono::duration<double>(t1-t0).count();
        r.b_search=chrono::duration<double>(t2-t1).count();
        r.f_init=chrono::duration<double>(t4-t3).count();
        r.f_search=chrono::duration<double>(t5-t4).count();
        r.i_init=chrono::duration<double>(t7-t6).count();
        r.i_total=chrono::duration<double>(t8-t6).count();
        return r;
    };

    double b_init=0,b_search=0,f_init=0,f_search=0,i_init=0,i_total=0;
    HeapStats stats; double f_consolidate=0;
    long long b_pushes=0,b_pops=0,b_stale=0;

    for (auto [s,t]:pairs) {
        // Repeat the measurement and keep the median run, so that one unlucky
        // scheduling or paging event does not dominate the reported average.
        vector<Sample> runs; runs.reserve(repeats);
        for(int r=0;r<repeats;r++) runs.push_back(measure(s,t));
        auto by=[&](double Sample::*field){
            vector<double> v; v.reserve(runs.size());
            for (auto& x:runs) v.push_back(x.*field);
            sort(v.begin(),v.end());
            return v[v.size()/2];
        };
        b_init+=by(&Sample::b_init); b_search+=by(&Sample::b_search);
        f_init+=by(&Sample::f_init); f_search+=by(&Sample::f_search);
        i_init+=by(&Sample::i_init); i_total+=by(&Sample::i_total);
        // The operation counts are deterministic, so any run gives the same.
        const Sample& chosen=runs[runs.size()/2];
        b_pushes+=chosen.b_pushes; b_pops+=chosen.b_pops; b_stale+=chosen.b_stale;
        stats.pushes+=chosen.stats.pushes; stats.pops+=chosen.stats.pops;
        stats.decreases+=chosen.stats.decreases; stats.consolidations+=chosen.stats.consolidations;
        stats.links+=chosen.stats.links; stats.cuts+=chosen.stats.cuts;
        stats.cascading_cuts+=chosen.stats.cascading_cuts;
        stats.root_ring_total+=chosen.stats.root_ring_total;
        stats.root_ring_max=max(stats.root_ring_max,chosen.stats.root_ring_max);
        f_consolidate+=chosen.stats.consolidate_seconds;
    }

    double n=(double)q;
    double gap=(f_init+f_search)-(b_init+b_search);
    cout<<"dataset="<<path<<" vertices="<<g.adj.size()<<" arcs="<<g.arcs<<" queries="<<q<<"\n";
    cout<<"phase,init_ms,search_ms,total_ms\n";
    cout<<"binary,"<<b_init*1000/n<<","<<b_search*1000/n<<","<<(b_init+b_search)*1000/n<<"\n";
    cout<<"fibonacci,"<<f_init*1000/n<<","<<f_search*1000/n<<","<<(f_init+f_search)*1000/n<<"\n";
    cout<<"indexed_binary,"<<i_init*1000/n<<","<<(i_total-i_init)*1000/n<<","<<i_total*1000/n<<"\n";
    cout<<"ratio_fibonacci_over_binary,"<<(f_init+f_search)/(b_init+b_search)<<"\n";
    cout<<"ratio_indexed_over_binary,"<<i_total/(b_init+b_search)<<"\n";
    cout<<"gap_from_initialization_ms,"<<(f_init-b_init)*1000/n<<"\n";
    cout<<"gap_from_search_ms,"<<(f_search-b_search)*1000/n<<"\n";
    cout<<"gap_initialization_share,"<<(f_init-b_init)/gap<<"\n";
    cout<<"gap_search_share,"<<(f_search-b_search)/gap<<"\n";
    cout<<"heap_stat,value\n";
    cout<<"fib_pushes,"<<stats.pushes<<"\n";
    cout<<"fib_pops,"<<stats.pops<<"\n";
    cout<<"fib_decrease_key,"<<stats.decreases<<"\n";
    cout<<"fib_consolidations,"<<stats.consolidations<<"\n";
    cout<<"fib_links,"<<stats.links<<"\n";
    cout<<"fib_cuts,"<<stats.cuts<<"\n";
    cout<<"fib_cascading_cuts,"<<stats.cascading_cuts<<"\n";
    cout<<"fib_root_ring_avg,"<<(double)stats.root_ring_total/max(1LL,stats.consolidations)<<"\n";
    cout<<"fib_root_ring_max,"<<stats.root_ring_max<<"\n";
    cout<<"fib_consolidate_share,"<<f_consolidate/(f_init+f_search)<<"\n";
    cout<<"binary_pushes,"<<b_pushes<<"\n";
    cout<<"binary_pops,"<<b_pops<<"\n";
    cout<<"binary_stale_pops,"<<b_stale<<"\n";
}

// Run correctness tests or time both queues on the same generated query list.
int main(int argc,char** argv) {
    try {
        if (argc==2 && string(argv[1])=="--self-test") {self_test(); cout<<"PASS: edge cases and 2000 random pairs\n"; return 0;}
        if (argc==3 && string(argv[1])=="--validate") {
            Graph g=read_graph(argv[2]); cout<<g.adj.size()<<" vertices, "<<g.arcs<<" arcs\n"; return 0;
        }
        if (argc>=4 && string(argv[1])=="--attribute") {
            int rep=1;
            for(int i=4;i<argc;i++) if (string(argv[i])=="--repeat" && i+1<argc) rep=stoi(argv[++i]);
            attribute_report(argv[2],stoi(argv[3]),rep); return 0;
        }
        // Optional --repeat N times each query and keep the median, which
        // absorbs a single unlucky scheduling or paging event.
        int repeats=1;
        vector<string> rest;
        for(int i=1;i<argc;i++) {
            if (string(argv[i])=="--repeat" && i+1<argc) { repeats=stoi(argv[++i]); continue; }
            rest.push_back(argv[i]);
        }
        if (rest.size()!=3) {cerr<<"usage: benchmark graph.gr queries seed [--repeat N]\n"; return 2;}
        Graph g=read_graph(rest[0]); int q=stoi(rest[1]); unsigned seed=stoul(rest[2]);
        if(q<1000) throw runtime_error("at least 1000 queries required");
        if(repeats<1) throw runtime_error("repeat count must be positive");
        mt19937 rng(seed); uniform_int_distribution<int> pick(0,(int)g.adj.size()-1);
        vector<pair<int,int>> pairs; pairs.reserve(q);
        for(int i=0;i<q;i++) pairs.push_back({pick(rng),pick(rng)});
        cout<<"method,query,source,target,distance,seconds,nodes,arcs,seed,repeats\n";
        vector<i64> expected(q);
        for(int method=0;method<2;method++) for(int i=0;i<q;i++) {
            auto [s,t]=pairs[i];
            // Time the same query repeatedly and keep the median. The answer
            // is identical every time, so only the elapsed time varies.
            vector<double> samples; samples.reserve(repeats);
            i64 d=INF;
            for(int r=0;r<repeats;r++) {
                auto start=chrono::steady_clock::now();
                d=method==0 ? binary_dijkstra(g,s,t) : fib_dijkstra(g,s,t);
                double seconds=chrono::duration<double>(chrono::steady_clock::now()-start).count();
                samples.push_back(seconds);
            }
            sort(samples.begin(),samples.end());
            double seconds=samples[samples.size()/2];
            if(method==0) expected[i]=d;
            else if(d!=expected[i]) throw runtime_error("distance mismatch at query " + to_string(i));
            cout<<(method==0?"binary":"fibonacci")<<','<<i<<','<<s+1<<','<<t+1<<',';
            if(d==INF) cout<<"INF"; else cout<<d;
            cout<<','<<seconds<<','<<g.adj.size()<<','<<g.arcs<<','<<seed<<','<<repeats<<'\n';
        }
    } catch(const exception& e) {cerr<<e.what()<<'\n'; return 1;}
}
