#include <bits/stdc++.h>
#include <chrono>
using namespace std;

// ===== Node structure =====
struct Node {
    long long number;
    string name;
    vector<long long> inputs;
    long long runmem;
    long long outputmem;
    long long timecost;

    Node(long long num = 0, const string &n = "", const vector<long long> &inp = {},
         long long run = 0, long long out = 0, long long time = 0)
        : number(num), name(n), inputs(inp), runmem(run), outputmem(out), timecost(time) {}
};

// ===== Quick debug print =====
void printNodes(long long total_memory, const vector<Node>& nodes) {
    cout << "MemoryLimit: " << total_memory << endl;
    for (const auto& node : nodes) {
        cout << "Node " << node.number << " (" << node.name << "): ";
        cout << "Inputs: ";
        for (const auto& inp : node.inputs) cout << inp << " ";
        cout << "| RunMem: " << node.runmem
             << " | OutputMem: " << node.outputmem
             << " | TimeCost: " << node.timecost << endl;
    }
}

// ===== File ingestion =====
pair<long long, vector<Node>> ingestNodes(const string &input_file) {
    ifstream file(input_file);
    string line;
    long long total_memory = 0;
    vector<Node> nodes;

    if (!file.is_open()) {
        cerr << "Failed to open file: " << input_file << endl;
        return {0, {}};
    }

    if (getline(file, line)) {
        istringstream iss(line);
        string tmp;
        iss >> tmp >> total_memory;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        vector<string> parts;
        string part;
        while (iss >> part) parts.push_back(part);
        if (parts.size() < 4) continue;

        long long number = stoll(parts[0]);
        string name = parts[1];
        long long runmem = stoll(parts[parts.size() - 3]);
        long long outputmem = stoll(parts[parts.size() - 2]);
        long long timecost = stoll(parts[parts.size() - 1]);

        vector<long long> inputs;
        for (size_t i = 3; i < parts.size() - 3; ++i) {
            long long inp = stoll(parts[i]);
            if (inp != number) inputs.push_back(inp);
        }

        nodes.emplace_back(number, name, inputs, runmem, outputmem, timecost);
    }

    return {total_memory, nodes};
}

// ===== Topological sort =====
vector<Node> topoSort(const vector<Node> &nodes) {
    unordered_map<long long, long long> indeg;
    unordered_map<long long, vector<long long>> adj;
    unordered_map<long long, Node> id_to_node;
    unordered_set<long long> all_ids;

    for (const auto &n : nodes) {
        all_ids.insert(n.number);
        id_to_node[n.number] = n;
        if (!indeg.count(n.number)) indeg[n.number] = 0;
    }

    for (const auto &n : nodes) {
        for (long long in : n.inputs) {
            if (!indeg.count(in)) indeg[in] = 0;
            adj[in].push_back(n.number);
            indeg[n.number]++;
        }
    }

    queue<long long> q;
    for (long long id : all_ids)
        if (indeg[id] == 0) q.push(id);

    vector<Node> order;
    while (!q.empty()) {
        long long u = q.front(); q.pop();
        order.push_back(id_to_node[u]);
        for (long long v : adj[u])
            if (--indeg[v] == 0)
                q.push(v);
    }

    if (order.size() != all_ids.size())
        cerr << "Warning: topoSort incomplete, check for cycles.\n";

    return order;
}

// ===== Helpers for ExecuteOrder =====
struct ExecNode {
    Node* node;
    size_t idx_in_sorted;
};

double recompute_score(Node* n, long long current_mem, long long total_memory) {
    long long mem_saved = n->outputmem;
    if (current_mem + n->runmem <= total_memory) return 0.0;
    return (double)n->timecost / mem_saved;
}

bool all_inputs_ready(Node* n, const unordered_set<long long>& memory2) {
    for (long long inp : n->inputs)
        if (!memory2.count(inp)) return false;
    return true;
}

void remove_mem_future(vector<Node*>& memory, long long& current_mem, long long max_mem,
                       const unordered_map<long long, size_t>& next_use) {
    sort(memory.begin(), memory.end(), [&](Node* a, Node* b) {
        size_t next_a = next_use.count(a->number) ? next_use.at(a->number) : SIZE_MAX;
        size_t next_b = next_use.count(b->number) ? next_use.at(b->number) : SIZE_MAX;
        return next_a < next_b;
    });

    size_t i = memory.size();
    while (current_mem > max_mem && i-- > 0) {
        Node* n = memory[i];
        current_mem -= n->outputmem;
        memory.erase(memory.begin() + i);
    }
}

void add_node_memory(Node* nodetoadd, vector<Node*>& memory, long long& current_mem,
                     long long& peak_mem, long long max_mem) {
    current_mem += nodetoadd->runmem;
    if (current_mem > peak_mem) peak_mem = current_mem;
    current_mem -= nodetoadd->runmem;
    current_mem += nodetoadd->outputmem;
    memory.push_back(nodetoadd);
    if (current_mem > peak_mem) peak_mem = current_mem;
}

// ===== ExecuteOrder =====
vector<Node> ExecuteOrder(const vector<Node> &all_nodes, const string &output_name, long long total_memory) {
    auto start = chrono::high_resolution_clock::now();

    vector<Node> sorted_nodes = topoSort(all_nodes);
    unordered_map<long long, Node*> id_to_node;
    for (auto &n : sorted_nodes) id_to_node[n.number] = &n;

    unordered_map<long long, size_t> next_use;
    for (size_t i = 0; i < sorted_nodes.size(); ++i)
        for (long long inp : sorted_nodes[i].inputs)
            next_use[inp] = i;

    vector<Node*> memory;
    unordered_set<long long> memory2;
    vector<Node> run_order;
    long long current_mem = 0;
    long long peak_mem = 0;
    long long timecount = 0;

    auto cmp = [&](ExecNode a, ExecNode b) {
        return recompute_score(a.node, current_mem, total_memory) > recompute_score(b.node, current_mem, total_memory);
    };
    priority_queue<ExecNode, vector<ExecNode>, decltype(cmp)> pq(cmp);

    for (size_t i = 0; i < sorted_nodes.size(); ++i) {
        Node* n = &sorted_nodes[i];
        if (n->inputs.empty()) pq.push({n, i});
    }

    unordered_set<long long> executed;
    while (!pq.empty()) {
        ExecNode en = pq.top(); pq.pop();
        Node* n = en.node;

        for (long long inp : n->inputs) {
            if (!memory2.count(inp)) {
                Node* in_node = id_to_node[inp];
                add_node_memory(in_node, memory, current_mem, peak_mem, total_memory);
                timecount += in_node->timecost;
                memory2.insert(inp);
                run_order.push_back(*in_node);
            }
        }

        if (current_mem + n->runmem > total_memory) {
            remove_mem_future(memory, current_mem, total_memory, next_use);
        }

        add_node_memory(n, memory, current_mem, peak_mem, total_memory);
        timecount += n->timecost;
        run_order.push_back(*n);
        memory2.insert(n->number);
        executed.insert(n->number);

        for (size_t i = 0; i < sorted_nodes.size(); ++i) {
            Node* candidate = &sorted_nodes[i];
            if (executed.count(candidate->number)) continue;
            if (all_inputs_ready(candidate, memory2)) pq.push({candidate, i});
        }
    }

    cout << "Total cost: " << timecount << "\n";
    cout << "Peak memory usage: " << peak_mem << "\n";
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds\n";
    return run_order;
}

// ===== Main =====
int main() {
    string input_file = "test_out/example1.txt"; // replace with your file
    auto [max_mem, nodes] = ingestNodes(input_file);
    ExecuteOrder(nodes, input_file, max_mem);
    return 0;
}
