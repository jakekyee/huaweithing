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

    // // Pass in memory, currentmem, safe nodes, max_mem
    // void remove_mem(vector<Node> &memory, long long &current_mem, const unordered_set<long long> &safe_nodes, long long max_mem) {
    //     auto it = memory.begin();
    //     while (current_mem > max_mem && it != memory.end()) {
    //         if (safe_nodes.count(it->number)) {
    //             ++it; // skip safe nodes
    //         } else {
    //             current_mem -= it->outputmem;
    //             it = memory.erase(it);
    //         }
    //     }
    // }
    // void remove_mem(vector<Node> &memory, long long &current_mem, const unordered_set<long long> &safe_nodes, long long max_mem) {
    //     auto it = memory.begin();
    //     while (current_mem > max_mem && it != memory.end()) {
    //         if (safe_nodes.count(it->number)) {
    //             ++it; // skip safe nodes
    //         } else {
    //             current_mem -= it->outputmem;
    //             it = memory.erase(it);
    //         }
    //     }
    // }

    // Pass in memory, currentmem, safe nodes, max_mem - NEWEST to OLDEST
    // void remove_mem(vector<Node> &memory, long long &current_mem, const unordered_set<long long> &safe_nodes, long long max_mem) {
    //     auto it = memory.rbegin(); // reverse iterator starting from the end
    //     while (current_mem > max_mem && it != memory.rend()) {
    //         if (safe_nodes.count(it->number)) {
    //             ++it; // skip safe nodes (moves toward beginning)
    //         } else {
    //             current_mem -= it->outputmem;
    //             // For reverse iterators, we need to use base() to erase
    //             memory.erase((++it).base());
    //             // After erase, 'it' is already advanced to next element
    //         }
    //     }
    // }
// Eviction-aware memory removal: LRU + largest outputmem
void remove_mem(vector<Node> &memory, long long &current_mem,
                const unordered_set<long long> &safe_nodes, long long max_mem,
                unordered_set<long long> &memory2) 
{
    while (current_mem > max_mem) {
        // Find the oldest node that is not safe
        auto it = find_if(memory.begin(), memory.end(), [&](const Node &n) {
            return !safe_nodes.count(n.number);
        });

        if (it == memory.end()) break; // nothing left to evict

        current_mem -= it->outputmem;
        memory2.erase(it->number);
        memory.erase(it);
    }
}

// Recursive addition with eviction awareness
void add_mem(vector<Node> &memory, long long &current_mem, const Node &nodetoadd,
             const unordered_map<long long, Node> &id_to_node, long long max_mem,
             unordered_set<long long> &safe_nodes, vector<Node> &run_order,
             unordered_set<long long> &memory2, long long &timecount)
{
    vector<long long> added_safes;

    // 1️⃣ Mark inputs as safe and ensure they are in memory
    for (long long inp_id : nodetoadd.inputs) {
        if (!safe_nodes.count(inp_id)) {
            safe_nodes.insert(inp_id);
            added_safes.push_back(inp_id);
        }

        if (!memory2.count(inp_id)) {
            auto it = id_to_node.find(inp_id);
            if (it != id_to_node.end()) {
                add_mem(memory, current_mem, it->second, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount);
            } else {
                cerr << "Error: input node " << inp_id << " not found in DAG!\n";
            }
        }
    }

    // 2️⃣ Ensure enough memory for running node
    if (current_mem + nodetoadd.runmem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem, memory2);
    }

    // 3️⃣ Run node
    current_mem += nodetoadd.runmem;
    memory.push_back(nodetoadd);
    memory2.insert(nodetoadd.number);
    timecount = timecount + nodetoadd.timecost;
    run_order.push_back(nodetoadd);

    // 4️⃣ Convert runmem → outputmem
    current_mem -= nodetoadd.runmem;
    current_mem += nodetoadd.outputmem;

    // 5️⃣ Cleanup memory if over limit
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
        // std::string input_file = "test_out/example2.txt"; // replace with your file
        // std::string input_file = "test_out/example3.txt"; // replace with your file
        std::string input_file = "diytest_out/example5.txt"; // replace with your file
        // std::string input_file = "diytest_out/diytest2.txt"; // replace with your file
        // std::string input_file = "test_out/example4.txt"; // replace with your file
        // std::string input_file = "diytest_out/diytest1.txt"; // replace with your file
        // std::string input_file = "test_out/example7.txt"; // replace with your file
        auto [max_mem, nodes] = ingestNodes(input_file);
        long long current_mem = 0;
        long long timecount = 0;

        
        vector<Node> sorted_nodes = topoSort(nodes);
        vector<Node> memory;
        // unordered_map<long long, Node> memory2;
        unordered_set<long long> memory2;

        vector<Node> run_order;
        cout << "Topologically sorted nodes:\n";
        unordered_map<long long, Node> id_to_node;
        for (const auto &n : sorted_nodes) id_to_node[n.number] = n;
        
        for (const auto &node : sorted_nodes) {
            unordered_set<long long> safe_nodes;

            add_mem(memory, current_mem, node, id_to_node, max_mem, safe_nodes, run_order, memory2, timecount);
            // cout << "sdlfjlaksfjd" << "\n";
            // cout << "X Ran node " << node.number << " (" << node.name << "), current memory: " << current_mem << "\n";
        }    

        // for (const auto &node : run_order) {
        //     timecount = timecount + node.timecost;
        //     // cout << "O Ran node " << node.number << " (" << node.name << ") timecount :" << timecount << "\n";
        // }
        cout << "TOtal cost " << timecount << "\n";
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;
        return 0;
    }
