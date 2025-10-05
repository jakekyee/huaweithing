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
    unordered_map<long long, Node> id_to_node;  // map from ID to Node
    unordered_set<long long> all_ids;

    for (const auto &n : nodes) {
        all_ids.insert(n.number);
        id_to_node[n.number] = n;  // store the Node
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
        order.push_back(id_to_node[u]);  // push the actual Node
        for (long long v : adj[u])
            if (--indeg[v] == 0)
                q.push(v);
    }

    if (order.size() != all_ids.size())
        cerr << "Warning: topoSort incomplete, check for cycles.\n";

    return order;
}

// void remove_mem(vector<Node*> &memory, long long &current_mem,
//                 const unordered_set<long long> &safe_nodes,
//                 long long max_mem, unordered_set<long long> &memory2) 
// {
//     // Iterate from oldest to newest (beginning to end) - LRU removal
//     int i = 0;
//     while (current_mem > max_mem && i < memory.size()) {
//         Node* n = memory[i];
//         if (!safe_nodes.count(n->number)) {
//             current_mem -= n->outputmem;
//             memory2.erase(n->number);
//             // Swap with last and pop (O(1))
//             memory[i] = memory.back();
//             memory.pop_back();
//             // Don't increment i since we swapped a new element into position i
//         } else {
//             // Only increment if we didn't remove the current element
//             ++i;
//         }
//     }
// }



// Add node with inputs recursively
void add_mem(vector<Node*> &memory, long long &current_mem, Node* nodetoadd,
             const unordered_map<long long, Node*> &id_to_node,
             long long max_mem, unordered_set<long long> &safe_nodes,
             vector<Node*> &run_order, unordered_set<long long> &memory2,
             long long &timecount)
{
    vector<long long> added_safes;

    // 1️⃣ Mark inputs as safe and ensure they are in memory
    for (long long inp_id : nodetoadd->inputs) {
        if (!safe_nodes.count(inp_id)) {
            safe_nodes.insert(inp_id);
            added_safes.push_back(inp_id);
        }

        if (!memory2.count(inp_id)) {
            auto it = id_to_node.find(inp_id);
            if (it != id_to_node.end()) {
                add_mem(memory, current_mem, it->second, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount);
            }
        }
    }

    // 2️⃣ Ensure enough memory
    if (current_mem + nodetoadd->runmem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2);
    }

    // 3️⃣ Run node
    current_mem += nodetoadd->runmem;
    memory.push_back(nodetoadd);
    memory2.insert(nodetoadd->number);
    timecount += nodetoadd->timecost;
    run_order.push_back(nodetoadd);

    // 4️⃣ Convert runmem -> outputmem
    current_mem -= nodetoadd->runmem;
    current_mem += nodetoadd->outputmem;

    // 5️⃣ Cleanup if over memory
    if (current_mem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2);
    }

    // 6️⃣ Restore safe_nodes
    for (long long id : added_safes) safe_nodes.erase(id);
}


// --------------------------- Main ---------------------------
int main() {
    auto start = std::chrono::high_resolution_clock::now();
    // std::string input_file = "test_out/example5.txt";     // replace with your file
    std::string input_file = "test_out/example2.txt"; // replace with your file
    // std::string input_file = "test_out/example2.txt"; // replace with your file
    // std::string input_file = "test_out/example3.txt"; // replace with your file
    // std::string input_file = "diytest_out/example5.txt"; // replace with your file
    // std::string input_file = "diytest_out/diytest2.txt"; // replace with your file
    // std::string input_file = "test_out/example5.txt"; // replace with your file
    // std::string input_file = "diytest_out/diytest1.txt"; // replace with your file
    // std::string input_file = "test_out/example7.txt"; // replace with your file
    auto [max_mem, nodes] = ingestNodes(input_file);

    long long current_mem = 0;
    long long timecount = 0;

    // Topologically sort the nodes
    vector<Node> sorted_nodes = topoSort(nodes);

    // Convert nodes to pointers for faster memory management
    unordered_map<long long, Node*> id_to_node;
    for (auto &n : sorted_nodes) id_to_node[n.number] = &n;

    vector<Node*> memory;
    unordered_set<long long> memory2;  // tracks node IDs in memory
    vector<Node*> run_order;

    cout << "Topologically sorted nodes:\n";
    for (auto &node : sorted_nodes) {
        unordered_set<long long> safe_nodes;
        add_mem(memory, current_mem, &node, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount);
    }

    cout << "Total cost: " << timecount << "\n";
    for (const auto &node : run_order) {
            // timecount = timecount + node.timecost;
            cout << "O Ran node " << node->number << " (" << node->name << ") timecount :" << timecount << "\n";
        }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds" << endl;

    return 0;
}
