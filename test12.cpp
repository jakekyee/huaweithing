#include <bits/stdc++.h>
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

// --------------------------- Memory management ---------------------------
void remove_mem(deque<Node> &memory, long long &current_mem,
                const unordered_set<long long> &safe_nodes, long long max_mem) 
{
    while (current_mem > max_mem && !memory.empty()) {
        if (safe_nodes.count(memory.front().number)) {
            memory.push_back(memory.front());
            memory.pop_front();
        } else {
            current_mem -= memory.front().outputmem;
            memory.pop_front();
        }
    }
}

void add_mem(deque<Node> &memory, long long &current_mem, const Node &node,
             const unordered_map<long long, Node> &id_to_node, long long max_mem,
             unordered_set<long long> &safe_nodes, vector<Node> &run_order)
{
    for (long long inp_id : node.inputs) {
        if (!safe_nodes.count(inp_id)) {
            safe_nodes.insert(inp_id);
            auto it = id_to_node.find(inp_id);
            if (it != id_to_node.end())
                add_mem(memory, current_mem, it->second, id_to_node, max_mem, safe_nodes, run_order);
        }
    }

    if (current_mem + node.runmem > max_mem)
        remove_mem(memory, current_mem, safe_nodes, max_mem);

    current_mem += node.runmem;
    memory.push_back(node);
    run_order.push_back(node);
    current_mem = current_mem - node.runmem + node.outputmem;

    if (current_mem > max_mem)
        remove_mem(memory, current_mem, safe_nodes, max_mem);
}

// --------------------------- Main ---------------------------
int main() {
    auto start = chrono::high_resolution_clock::now();
    // string input_file = "diytest_out/example5.txt";
    
    // std::string input_file = "test_out/example5.txt";     // replace with your file
    std::string input_file = "test_out/example2.txt"; // replace with your file
    // std::string input_file = "test_out/example3.txt"; // replace with your file
    // std::string input_file = "diytest_out/example5.txt"; // replace with your file
    // std::string input_file = "diytest_out/diytest2.txt"; // replace with your file
    // std::string input_file = "test_out/example4.txt"; // replace with your file
    // std::string input_file = "diytest_out/diytest1.txt"; // replace with your file
    // std::string input_file = "test_out/example7.txt"; // replace with your file
    auto [max_mem, nodes] = ingestNodes(input_file);

    vector<Node> sorted_nodes = topoSort(nodes);
    unordered_map<long long, Node> id_to_node;
    for (const auto &n : sorted_nodes) id_to_node[n.number] = n;

    deque<Node> memory;
    long long current_mem = 0;
    vector<Node> run_order;

    for (const auto &node : sorted_nodes) {
        unordered_set<long long> safe_nodes;
        add_mem(memory, current_mem, node, id_to_node, max_mem, safe_nodes, run_order);
    }

    long long timecount = 0;
    for (auto &node : run_order) timecount += node.timecost;
    cout << "Total cost: " << timecount << "\n";

    auto end = chrono::high_resolution_clock::now();
    cout << "Time taken: " 
         << chrono::duration_cast<chrono::microseconds>(end - start).count() 
         << " microseconds\n";

    return 0;
}
