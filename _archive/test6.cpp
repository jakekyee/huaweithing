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

// --------------------------- Lookahead memory removal ---------------------------
void remove_mem_lookahead(vector<Node> &memory, long long &current_mem,
                          const unordered_set<long long> &safe_nodes,
                          long long max_mem,
                          const unordered_map<long long, vector<long long>> &usage_map,
                          size_t current_step)
{
    vector<Node*> removable;
    for (auto &n : memory) {
        if (!safe_nodes.count(n.number)) removable.push_back(&n);
    }

    sort(removable.begin(), removable.end(), [&](Node* a, Node* b) {
        auto &ua = usage_map.find(a->number) != usage_map.end() ? usage_map.at(a->number) : vector<long long>{};
        auto &ub = usage_map.find(b->number) != usage_map.end() ? usage_map.at(b->number) : vector<long long>{};

        long long next_a = -1, next_b = -1;
        for (long long idx : ua) if (idx > (long long)current_step) { next_a = idx; break; }
        for (long long idx : ub) if (idx > (long long)current_step) { next_b = idx; break; }

        if (next_a == -1) return true;
        if (next_b == -1) return false;
        return next_a > next_b;
    });

    for (Node* n : removable) {
        if (current_mem <= max_mem) break;
        auto it = find_if(memory.begin(), memory.end(), [&](const Node &m) { return m.number == n->number; });
        if (it != memory.end()) {
            current_mem -= it->outputmem;
            memory.erase(it);
        }
    }
}

// --------------------------- Lookahead add memory ---------------------------
void add_mem_lookahead(vector<Node> &memory, long long &current_mem, const Node &nodetoadd,
                       const unordered_map<long long, Node> &id_to_node,
                       long long max_mem,
                       vector<Node> &run_order,
                       const unordered_map<long long, vector<long long>> &usage_map,
                       size_t current_step,
                       unordered_set<long long> &safe_nodes,
                       unordered_set<long long> &memory_set)
{
    // Recursively load inputs
    for (long long inp_id : nodetoadd.inputs) {
        safe_nodes.insert(inp_id);
        if (!memory_set.count(inp_id)) {
            auto it = id_to_node.find(inp_id);
            if (it != id_to_node.end()) {
                add_mem_lookahead(memory, current_mem, it->second, id_to_node, max_mem, run_order, usage_map, current_step, safe_nodes, memory_set);
            } else {
                cerr << "Error: input node " << inp_id << " not found in DAG!\n";
            }
        }
    }

    // Check memory before running
    if (current_mem + nodetoadd.runmem > max_mem) {
        remove_mem_lookahead(memory, current_mem, safe_nodes, max_mem, usage_map, current_step);
    }

    // Add node if not already in memory
    if (!memory_set.count(nodetoadd.number)) {
        current_mem += nodetoadd.runmem;
        memory.push_back(nodetoadd);
        run_order.push_back(nodetoadd);
        memory_set.insert(nodetoadd.number);

        current_mem -= nodetoadd.runmem;
        current_mem += nodetoadd.outputmem;

        if (current_mem > max_mem) {
            remove_mem_lookahead(memory, current_mem, safe_nodes, max_mem, usage_map, current_step);
        }
    }
}

// --------------------------- Main ---------------------------
int main() {
    string input_file = "test_out/example1.txt";
    auto [max_mem, nodes] = ingestNodes(input_file);
    long long current_mem = 0;

    vector<Node> sorted_nodes = topoSort(nodes);
    vector<Node> memory;
    vector<Node> run_order;
    unordered_set<long long> memory_set; // track nodes in memory

    unordered_map<long long, Node> id_to_node;
    for (const auto &n : sorted_nodes) id_to_node[n.number] = n;

    // Precompute usage map for lookahead
    unordered_map<long long, vector<long long>> usage_map;
    for (size_t i = 0; i < sorted_nodes.size(); ++i) {
        for (long long inp : sorted_nodes[i].inputs) {
            usage_map[inp].push_back(i);
        }
    }

    unordered_set<long long> safe_nodes; // passed by reference

    // Execute nodes with lookahead memory management
    for (size_t i = 0; i < sorted_nodes.size(); ++i) {
        add_mem_lookahead(memory, current_mem, sorted_nodes[i], id_to_node, max_mem, run_order, usage_map, i, safe_nodes, memory_set);
    }

    long long total_time = 0;
    for (const auto &node : run_order) {
        total_time += node.timecost;
        cout << "O Ran node " << node.number << " (" << node.name << ") mem: " << total_time << "\n";
    }
    cout << "Total cost: " << total_time << "\n";

    return 0;
}
