#include <bits/stdc++.h>
using namespace std;

struct Node {
    int number;
    string name;
    vector<int> inputs;
    int runmem;
    int outputmem;
    int timecost;

    Node(int num = 0, const string &n = "", const vector<int> &inp = {},
         int run = 0, int out = 0, int time = 0)
        : number(num), name(n), inputs(inp), runmem(run), outputmem(out), timecost(time) {}
};




// struct Node {
//     int number;
//     string name;
//     vector<int> inputs;
//     int runmem;
//     int outputmem;
//     int timecost;

//     Node(int num = 0, const string &n = "", const vector<int> &inp = {},
//          int run = 0, int out = 0, int time = 0)
//         : number(num), name(n), inputs(inp), runmem(run), outputmem(out), timecost(time) {}
// };



// --------------------------- File ingestion ---------------------------
pair<long, vector<Node>> ingestNodes(const string &input_file) {
    ifstream file(input_file);
    string line;
    long total_memory = 0;
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

        int number = stoi(parts[0]);
        string name = parts[1];
        int runmem = stoi(parts[parts.size() - 3]);
        int outputmem = stoi(parts[parts.size() - 2]);
        int timecost = stoi(parts[parts.size() - 1]);

        vector<int> inputs;
        for (size_t i = 2; i < parts.size() - 3; ++i) {
            int inp = stoi(parts[i]);
            if (inp != number) inputs.push_back(inp);
        }

        nodes.emplace_back(number, name, inputs, runmem, outputmem, timecost);
    }

    return {total_memory, nodes};
}

// --------------------------- Topological sort ---------------------------
vector<Node> topoSort(const vector<Node> &nodes) {
    unordered_map<int, int> indeg;
    unordered_map<int, vector<int>> adj;
    unordered_map<int, Node> id_to_node;  // map from ID to Node
    unordered_set<int> all_ids;

    for (const auto &n : nodes) {
        all_ids.insert(n.number);
        id_to_node[n.number] = n;  // store the Node
        if (!indeg.count(n.number)) indeg[n.number] = 0;
    }

    for (const auto &n : nodes) {
        for (int in : n.inputs) {
            if (!indeg.count(in)) indeg[in] = 0;
            adj[in].push_back(n.number);
            indeg[n.number]++;
        }
    }

    queue<int> q;
    for (int id : all_ids)
        if (indeg[id] == 0) q.push(id);

    vector<Node> order;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(id_to_node[u]);  // push the actual Node
        for (int v : adj[u])
            if (--indeg[v] == 0)
                q.push(v);
    }

    if (order.size() != all_ids.size())
        cerr << "Warning: topoSort incomplete, check for cycles.\n";

    return order;
}

// // /////////////////////
// // Pass in memory, currentmem, safe nodes, max_mem
// void remove_mem(){

//     // While currentmem > max_mem
//     // for thing in memory
//     // if thing in safe
//     // go next
//     // otherwise remove
//     // and check if still too much

// }

// // Pass in memory, currentmem, safenodes, nodetoadd, max_mem
// void add_mem(){
//     // make safenodes = nodetoadd.inputs
//     // check if safenodes in memory
//     // If not
//     // addmem(safenodes.copy) pass in safenodes
//     // otherwise
//     // If currentmemory + nodetoadd.runmem > max_mem
//     // then call remove mem and pass in safenodes
//     // Otherwise
//     // Add to memory
//     // Then if currentmemory + nodetoadd.outputmem > max_mem
//     // remove_mem(memory, currentmem, )

// }
// Pass in memory, currentmem, safe nodes, max_mem
// Remove memory nodes not in safe set
void remove_mem(vector<Node> &memory, int &current_mem, const unordered_set<int> &safe_nodes, int max_mem) {
    auto it = memory.begin();
    while (current_mem > max_mem && it != memory.end()) {
        if (safe_nodes.count(it->number)) {
            ++it; // skip safe nodes
        } else {
            current_mem -= it->outputmem;
            it = memory.erase(it);
        }
    }
}

// Add a node and its inputs to memory, recursively if needed
void add_mem(vector<Node> &memory, int &current_mem, const Node &nodetoadd,
             const unordered_map<int, Node> &id_to_node, int max_mem,
             vector<Node> &run_order, unordered_set<int> safe_nodes = {})
             {
    // Add this node's inputs to the safe set
    for (int inp_id : nodetoadd.inputs) {
        safe_nodes.insert(inp_id);

        // If input not in memory, add it first
        auto it = find_if(memory.begin(), memory.end(), [&](const Node &n){ return n.number == inp_id; });
        if (it == memory.end()) {
            auto node_it = id_to_node.find(inp_id);
            if (node_it != id_to_node.end()) {
                add_mem(memory, current_mem, node_it->second, id_to_node, max_mem,run_order, safe_nodes);
            } else {
                cerr << "Error: input node " << inp_id << " not found in DAG!\n";
            }
        }
    }

    // Ensure there is enough memory for running node
    if (current_mem + nodetoadd.runmem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem);
    }

    // Add node running memory
    cout << nodetoadd.name << " :\n";

    current_mem += nodetoadd.runmem;
    memory.push_back(nodetoadd);
    run_order.push_back(nodetoadd);

    // After running, convert running memory to output memory
    current_mem -= nodetoadd.runmem;
    current_mem += nodetoadd.outputmem;

    // Remove extra memory if over limit
    if (current_mem > max_mem) {
        remove_mem(memory, current_mem, safe_nodes, max_mem);
    }
}


// --------------------------- Main ---------------------------
int main() {
    // std::string input_file = "diytest_out/diytest1.txt"; // replace with your file
    std::string input_file = "test_out/example1.txt"; // replace with your file
    auto [max_mem, nodes] = ingestNodes(input_file);
    int current_mem = 0;
    
    vector<Node> sorted_nodes = topoSort(nodes);
    vector<Node> memory;
    vector<Node> run_order;
    cout << "Topologically sorted nodes:\n";
    unordered_map<int, Node> id_to_node;
    for (const auto &n : sorted_nodes) id_to_node[n.number] = n;

    for (const auto &node : sorted_nodes) {
        vector<Node> tempsafe;

        add_mem(memory, current_mem, node, id_to_node, max_mem, run_order);

        // run_order.push_back(node);
        cout << "Ran node " << node.number << " (" << node.name << "), current memory: " << current_mem << "\n";
    }    
    int memorycountthing = 0;
    for (const auto &node : run_order) {
        memorycountthing = memorycountthing + node.timecost;
        cout << "Ran node " << node.number << " (" << node.name << "\n";
    }
     cout << "TOtal cost " << memorycountthing << "\n";


    return 0;
}


