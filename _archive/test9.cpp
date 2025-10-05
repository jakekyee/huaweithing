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



    // Greedy removal: remove nodes with largest outputmem first (excluding safe_nodes)
    void remove_mem(vector<Node> &memory, long long &current_mem,
                    const unordered_set<long long> &safe_nodes, long long max_mem) 
    {
        // Collect removable nodes (not safe)
        vector<Node*> removable;
        for (auto &n : memory) {
            if (!safe_nodes.count(n.number)) {
                removable.push_back(&n);
            }
        }

        // Sort by outputmem descending
        sort(removable.begin(), removable.end(), [](Node* a, Node* b) {
            return a->outputmem > b->outputmem;
        });

        // Remove greedily until memory fits
        for (Node* n : removable) {
            if (current_mem <= max_mem) break;

            // Find node in memory and erase
            auto it = find_if(memory.begin(), memory.end(), [&](const Node &m) {
                return m.number == n->number;
            });

            if (it != memory.end()) {
                current_mem -= it->outputmem;
                memory.erase(it);
            }
        }
    }

    // // Pass in memory, currentmem, safe nodes, max_mem - NEWEST to OLDEST
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

    // Add a node and its inputs to memory, recursively if needed
    // void add_mem(vector<Node> &memory, long long &current_mem, const Node &nodetoadd,
    //             const unordered_map<long long, Node> &id_to_node, long long max_mem,
    //              unordered_set<long long> &safe_nodes, vector<Node> &run_order)
    // {
    //     // Add this node's inputs to the safe set
    //     for (long long inp_id : nodetoadd.inputs) {
    //         // safe_nodes.insert(inp_id);
    //         safe_nodes.insert(inp_id);

    //         // If input not in memory, add it first
    //         auto it = find_if(memory.begin(), memory.end(), [&](const Node &n){ return n.number == inp_id; });
    //         if (it == memory.end()) {
    //             auto node_it = id_to_node.find(inp_id);
    //             if (node_it != id_to_node.end()) {
    //                 add_mem(memory, current_mem, node_it->second, id_to_node, max_mem, safe_nodes, run_order);
    //             } else {
    //                 cerr << "Error: input node " << inp_id << " not found in DAG!\n";
    //             }
    //         }
    //     }

    //     // Ensure there is enough memory for running node
    //     if (current_mem + nodetoadd.runmem > max_mem) {
    //         remove_mem(memory, current_mem, safe_nodes, max_mem);
    //     }

    //     // Add node running memory
    //     // cout << nodetoadd.name << " :\n";

    //     current_mem += nodetoadd.runmem;
    //     memory.push_back(nodetoadd);
    //     run_order.push_back(nodetoadd);

    //     // After running, convert running memory to output memory
    //     current_mem -= nodetoadd.runmem;
    //     current_mem += nodetoadd.outputmem;

    //     // Remove extra memory if over limit
    //     if (current_mem > max_mem) {
    //         remove_mem(memory, current_mem, safe_nodes, max_mem);
    //     }
    // }



void add_mem(vector<Node> &memory, long long &current_mem, const Node &nodetoadd,
              const unordered_map<long long, Node> &id_to_node, long long max_mem,
              unordered_set<long long> &safe_nodes, vector<Node> &run_order)
{
    vector<long long> added_safes; // ✅ track which nodes we add as safe

    // Mark inputs as safe for this operation
    for (long long inp_id : nodetoadd.inputs) {
        if (!safe_nodes.count(inp_id)) { // only track if newly added
            safe_nodes.insert(inp_id);
            added_safes.push_back(inp_id);
        }

        // If input not in memory, add it recursively
        auto it = find_if(memory.begin(), memory.end(),
                          [&](const Node &n){ return n.number == inp_id; });
        if (it == memory.end()) {
            auto node_it = id_to_node.find(inp_id);
            if (node_it != id_to_node.end()) {
                add_mem(memory, current_mem, node_it->second, id_to_node, max_mem, safe_nodes, run_order);
            } else {
                cerr << "Error: input node " << inp_id << " not found in DAG!\n";
            }
        }
    }

    // Ensure enough memory
    if (current_mem + nodetoadd.runmem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem);
    }

    // Simulate running node
    current_mem += nodetoadd.runmem;
    memory.push_back(nodetoadd);
    run_order.push_back(nodetoadd);

    // Convert runmem → outputmem
    current_mem -= nodetoadd.runmem;
    current_mem += nodetoadd.outputmem;

    // Clean up extra memory if still over limit
    if (current_mem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem);
    }


    for (long long id : added_safes) {
        safe_nodes.erase(id);
    }
}






    // --------------------------- Main ---------------------------
    int main() {
        std::string input_file = "test_out/example1.txt";     // replace with your file
        // std::string input_file = "test_out/example2.txt"; // replace with your file
        // std::string input_file = "test_out/example3.txt"; // replace with your file
        // std::string input_file = "test_out/example4.txt"; // replace with your file
        // std::string input_file = "diytest_out/diytest1.txt"; // replace with your file
        // std::string input_file = "test_out/example7.txt"; // replace with your file
        auto [max_mem, nodes] = ingestNodes(input_file);
        long long current_mem = 0;
        
        vector<Node> sorted_nodes = topoSort(nodes);
        vector<Node> memory;
        vector<Node> run_order;
        cout << "Topologically sorted nodes:\n";
        unordered_map<long long, Node> id_to_node;
        for (const auto &n : sorted_nodes) id_to_node[n.number] = n;

        for (const auto &node : sorted_nodes) {
            unordered_set<long long> safe_nodes;

            add_mem(memory, current_mem, node, id_to_node, max_mem, safe_nodes, run_order);
            // cout << "sdlfjlaksfjd" << "\n";
            // cout << "X Ran node " << node.number << " (" << node.name << "), current memory: " << current_mem << "\n";
        }    

        long long timecount = 0;
        for (const auto &node : run_order) {
            timecount = timecount + node.timecost;
            cout << "O Ran node " << node.number << " (" << node.name << ") timecount :" << timecount << "\n";
        }
        cout << "TOtal cost " << timecount << "\n";

        return 0;
    }
