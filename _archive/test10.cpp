#include <bits/stdc++.h>
#include <chrono>
using namespace std;

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

// --------------------------- File ingestion ---------------------------
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
        vector<string> parts{istream_iterator<string>{iss}, istream_iterator<string>{}};
        if (parts.size() < 4) continue;

        long long number = stoll(parts[0]);
        string name = parts[1];
        long long runmem = stoll(parts[parts.size() - 3]);
        long long outputmem = stoll(parts[parts.size() - 2]);
        long long timecost = stoll(parts[parts.size() - 1]);

        vector<long long> inputs;
        for (size_t i = 2; i < parts.size() - 3; ++i) {
            long long inp = stoll(parts[i]);
            if (inp != number) inputs.push_back(inp);
        }

        nodes.emplace_back(number, name, inputs, runmem, outputmem, timecost);
    }

    return {total_memory, nodes};
}

// --------------------------- Topological sort ---------------------------
vector<Node> topoSort(const vector<Node> &nodes) {
    unordered_map<long long, long long> indeg;
    unordered_map<long long, vector<long long>> adj;
    unordered_map<long long, Node> id_to_node;

    for (const auto &n : nodes) {
        indeg[n.number] = 0;
        id_to_node[n.number] = n;
    }

    for (const auto &n : nodes) {
        for (long long in : n.inputs) {
            adj[in].push_back(n.number);
            indeg[n.number]++;
        }
    }

    queue<long long> q;
    for (const auto &[id, deg] : indeg)
        if (deg == 0) q.push(id);

    vector<Node> order;
    while (!q.empty()) {
        long long u = q.front(); q.pop();
        order.push_back(id_to_node[u]);
        for (long long v : adj[u])
            if (--indeg[v] == 0) q.push(v);
    }

    if (order.size() != nodes.size())
        cerr << "Warning: topoSort incomplete, possible cycles.\n";

    return order;
}

// --------------------------- Memory management ---------------------------
void remove_mem(vector<Node> &memory, long long &current_mem,
                const unordered_set<long long> &safe_nodes,
                long long max_mem, unordered_set<long long> &memory2) 
{
    vector<Node*> removable;
    for (auto &n : memory)
        if (!safe_nodes.count(n.number)) removable.push_back(&n);

    sort(removable.begin(), removable.end(),
         [](Node* a, Node* b) { return a->outputmem > b->outputmem; });

    for (Node* n : removable) {
        if (current_mem <= max_mem) break;

        auto it = find_if(memory.begin(), memory.end(),
                          [&](const Node &m){ return m.number == n->number; });

        if (it != memory.end()) {
            current_mem -= it->outputmem;
            memory2.erase(it->number);
            memory.erase(it);
        }
    }
}

void add_mem(vector<Node> &memory, long long &current_mem, const Node &nodetoadd,
             const unordered_map<long long, Node> &id_to_node, long long max_mem,
             unordered_set<long long> &safe_nodes, vector<Node> &run_order,
             unordered_set<long long> &memory2)
{
    vector<long long> added_safes;

    // Mark inputs as safe and add recursively
    for (long long inp_id : nodetoadd.inputs) {
        if (!safe_nodes.count(inp_id)) { 
            safe_nodes.insert(inp_id);
            added_safes.push_back(inp_id);
        }

        if (!memory2.count(inp_id)) {
            auto node_it = id_to_node.find(inp_id);
            if (node_it != id_to_node.end()) {
                add_mem(memory, current_mem, node_it->second, id_to_node,
                        max_mem, safe_nodes, run_order, memory2);
            } else {
                cerr << "Error: input node " << inp_id << " not found!\n";
            }
        }
    }

    if (current_mem + nodetoadd.runmem > max_mem)
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2);

    current_mem += nodetoadd.runmem;
    memory.push_back(nodetoadd);
    memory2.insert(nodetoadd.number);
    run_order.push_back(nodetoadd);

    current_mem = current_mem - nodetoadd.runmem + nodetoadd.outputmem;

    if (current_mem > max_mem)
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2);

    for (long long id : added_safes) safe_nodes.erase(id);
}

// --------------------------- Main ---------------------------
int main() {
    auto start = chrono::high_resolution_clock::now();

    // string input_file = "test_out/example1.txt"; // Replace with your file
    std::string input_file = "diytest_out/diytest2.txt"; // replace with your file

    auto [max_mem, nodes] = ingestNodes(input_file);

    long long current_mem = 0;
    vector<Node> memory;
    unordered_set<long long> memory2;
    vector<Node> run_order;

    vector<Node> sorted_nodes = topoSort(nodes);
    unordered_map<long long, Node> id_to_node;
    for (const auto &n : sorted_nodes) id_to_node[n.number] = n;

    for (const auto &node : sorted_nodes) {
        unordered_set<long long> safe_nodes;
        add_mem(memory, current_mem, node, id_to_node, max_mem, safe_nodes, run_order, memory2);
    }

    long long total_time = 0;
    for (const auto &node : run_order) {
        total_time += node.timecost;
        cout << "Ran node " << node.number << " (" << node.name << ") timecount: " << total_time << "\n";
    }
    cout << "Total cost: " << total_time << "\n";

    auto end = chrono::high_resolution_clock::now();
    cout << "Time taken: " << chrono::duration_cast<chrono::microseconds>(end - start).count()
         << " microseconds\n";

    return 0;
}
