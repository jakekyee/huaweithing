#include <bits/stdc++.h>
#include <chrono>
using namespace std;

// Node structure
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


//Quick test for debugging
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


// Ingest the files
pair<long long, vector<Node>> ingestNodes(const string &input_file) {
    ifstream file(input_file);
    string line;
    long long total_memory = 0;
    vector<Node> nodes;

    if (!file.is_open()) {
        cerr << "Failed to open file: " << input_file << endl;
        return {0, {}};
    }

    // First line: "MemoryLimit <value>"
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


// Toposort
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


// Remove memory
void remove_mem(vector<Node*> &memory, long long &current_mem,
                const unordered_set<long long> &safe_nodes,
                long long max_mem, unordered_set<long long> &memory2,
                const unordered_map<long long, Node*> &id_to_node,
                const vector<Node> &sorted_nodes, long long current_idx);
{

    //Hardcoded for speed
    // Could not do it if I wanted to
    const int LOOKAHEAD = 100;
    unordered_set<long long> future_needed;

    for (int i = current_idx; i < min((size_t)(current_idx + LOOKAHEAD), sorted_nodes.size()); i++) {
        const Node& future_node = sorted_nodes[i];
        for (long long inp : future_node.inputs) {
            future_needed.insert(inp);
        }
    }

    int i = 0;
    while (current_mem > max_mem && i < (int)memory.size()) {
        Node* n = memory[i];
        if (!safe_nodes.count(n->number) && !future_needed.count(n->number)) {
            current_mem -= n->outputmem;
            memory2.erase(n->number);
            memory[i] = memory.back();
            memory.pop_back();
        } else {
            ++i;
        }
    }

    i = 0;
    while (current_mem > max_mem && i < (int)memory.size()) {
        Node* n = memory[i];
        if (!safe_nodes.count(n->number)) {
            current_mem -= n->outputmem;
            memory2.erase(n->number);
            memory[i] = memory.back();
            memory.pop_back();
        } else {
            ++i;
        }
    }
}


// Add memory
void add_mem(vector<Node*> &memory, long long &current_mem, long long &peak_mem, Node* nodetoadd,  // <-- NEW: added peak_mem
             const unordered_map<long long, Node*> &id_to_node,
             long long max_mem, unordered_set<long long> &safe_nodes,
             vector<Node> &run_order, unordered_set<long long> &memory2,
             long long &timecount, const vector<Node> &sorted_nodes, long long current_idx)
{
    vector<long long> added_safes;

    for (long long inp_id : nodetoadd->inputs) {
        if (!safe_nodes.count(inp_id)) {
            safe_nodes.insert(inp_id);
            added_safes.push_back(inp_id);
        }

        if (!memory2.count(inp_id)) {
            auto it = id_to_node.find(inp_id);
            if (it != id_to_node.end()) {
                add_mem(memory, current_mem, peak_mem, it->second, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount, sorted_nodes, current_idx);
            }
        }
    }
    current_mem += nodetoadd->runmem;

    if (current_mem + nodetoadd->runmem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2, id_to_node, sorted_nodes, current_idx);
    }

    if (current_mem > peak_mem) peak_mem = current_mem;  // <-- NEW

    memory.push_back(nodetoadd);
    memory2.insert(nodetoadd->number);
    timecount += nodetoadd->timecost;
    run_order.push_back(*nodetoadd);

    current_mem -= nodetoadd->runmem;
    current_mem += nodetoadd->outputmem;

    if (current_mem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2, id_to_node, sorted_nodes, current_idx);
    }
    if (current_mem > peak_mem) peak_mem = current_mem;  // <-- NEW

    for (long long id : added_safes) safe_nodes.erase(id);
}


// Execute order
vector<Node> ExecuteOrder(const vector<Node> &all_nodes, const std::string &output_name, long long total_memory) {
    auto start = std::chrono::high_resolution_clock::now();

    long long max_mem = total_memory;
    long long current_mem = 0;
    long long peak_mem = 0;  // <-- NEW
    long long timecount = 0;

    vector<Node> sorted_nodes = topoSort(all_nodes);

    unordered_map<long long, Node*> id_to_node;
    for (auto &n : sorted_nodes) id_to_node[n.number] = &n;

    vector<Node*> memory;
    unordered_set<long long> memory2;
    vector<Node> run_order;

    cout << "Topologically sorted nodes:\n";
    for (size_t idx = 0; idx < sorted_nodes.size(); ++idx) {
        auto &node = sorted_nodes[idx];
        unordered_set<long long> safe_nodes;
        add_mem(memory, current_mem, peak_mem, &node, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount, sorted_nodes, (long long)idx);
    }

    cout << "Total cost: " << timecount << "\n";
    cout << "Peak memory usage: " << peak_mem << " units\n";  // <-- NEW
    int count = 0;
    for (const auto &node : run_order) {
        count++;
        // cout << "O Ran node " << node.number << " (" << node.name << ")"  << "\n";
    }
    cout << "Count : " << count << " count" << endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds" << endl;

    return run_order;
}


// Main
int main() {
    std::string input_file = "test_out/example5.txt"; // replace with your file
    auto [max_mem, nodes] = ingestNodes(input_file);

    ExecuteOrder(nodes, input_file, max_mem);
    return 0;
}
