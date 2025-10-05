#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <set>

struct Node {
    int number;
    std::string name;
    std::vector<int> inputs;
    int runmem;
    int outputmem;
    int timecost;

    Node(int num = 0, const std::string &n = "", const std::vector<int> &inp = {},
         int run = 0, int out = 0, int time = 0)
        : number(num), name(n), inputs(inp), runmem(run), outputmem(out), timecost(time) {}
};

// Function to ingest nodes from file
std::pair<int, std::vector<Node>> ingestNodes(const std::string &input_file) {
    std::ifstream file(input_file);
    std::string line;
    int total_memory = 0;
    std::vector<Node> nodes;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << input_file << std::endl;
        return {0, {}};
    }

    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string tmp;
        iss >> tmp >> total_memory;  // Assume first line format: "MemoryLimit <value>"
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> parts;
        std::string part;

        while (iss >> part) {
            parts.push_back(part);
        }

        if (parts.size() < 4) continue;

        int number = std::stoi(parts[0]);
        std::string name = parts[1];
        int runmem = std::stoi(parts[parts.size() - 3]);
        int outputmem = std::stoi(parts[parts.size() - 2]);
        int timecost = std::stoi(parts[parts.size() - 1]);

        std::vector<int> inputs;
        for (size_t i = 2; i < parts.size() - 3; ++i) {
            int inp = std::stoi(parts[i]);
            if (inp != number) inputs.push_back(inp); // skip self-dependency
        }

        nodes.emplace_back(number, name, inputs, runmem, outputmem, timecost);
    }

    return {total_memory, nodes};
}

// Function to perform topological sort
std::vector<int> topoSort(const std::vector<Node> &nodes) {
    std::unordered_map<int, std::vector<int>> graph;
    std::unordered_map<int, int> indegree;

    std::unordered_map<int, bool> node_exists;
    for (const auto &node : nodes) node_exists[node.number] = true;

    for (const auto &node : nodes) {
        for (int inp : node.inputs) {
            if (node_exists[inp]) {
                graph[inp].push_back(node.number);
                indegree[node.number]++;
            }
        }
        if (indegree.find(node.number) == indegree.end()) {
            indegree[node.number] = 0;
        }
    }

    std::queue<int> q;
    for (const auto &pair : node_exists) {
        if (indegree[pair.first] == 0) q.push(pair.first);
    }

    std::vector<int> sorted_nodes;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        sorted_nodes.push_back(u);

        for (int v : graph[u]) {
            indegree[v]--;
            if (indegree[v] == 0) q.push(v);
        }
    }

    if (sorted_nodes.size() != nodes.size()) {
        std::cerr << "Warning: cycle detected or missing dependencies! Topo sort incomplete.\n";
    }

    return sorted_nodes;
}

// ---------------- Sliding Window DP for Memory-Constrained Execution ----------------
std::vector<int> ExecuteOrder(const std::vector<Node> &nodes, const std::vector<int> &topo_order, long total_memory) {
    std::unordered_map<int, Node> node_map;
    for (auto &node : nodes) node_map[node.number] = node;

    struct DPState {
        long time_cost;
        long mem_used;
        std::vector<int> sequence;
    };

    // Map node number to its best DP state
    std::unordered_map<int, DPState> dp;

    for (int node_num : topo_order) {
        const Node &node = node_map[node_num];
        DPState best_state;
        best_state.time_cost = LONG_MAX;

        // Consider all previous DP states that include some or all dependencies
        if (node.inputs.empty()) {
            // No dependencies: initial node
            if (node.runmem + node.outputmem <= total_memory) {
                best_state.time_cost = node.timecost;
                best_state.mem_used = node.runmem + node.outputmem;
                best_state.sequence.push_back(node.number);
            }
        } else {
            // Dependencies exist
            // We collect union of sequences of all inputs
            std::set<int> current_sequence;
            long extra_time = 0;
            long memory_used = node.runmem + node.outputmem;

            for (int inp : node.inputs) {
                // If input already has a DP state
                if (dp.find(inp) != dp.end()) {
                    DPState &inp_state = dp[inp];
                    extra_time += inp_state.time_cost;
                    memory_used += node_map[inp].outputmem;
                    for (int n : inp_state.sequence) current_sequence.insert(n);
                } else {
                    // Should not happen if topo_order is valid
                    memory_used += node_map[inp].outputmem;
                    extra_time += node_map[inp].timecost;
                    current_sequence.insert(inp);
                }
            }

            if (memory_used <= total_memory) {
                DPState state;
                state.time_cost = extra_time + node.timecost;
                state.mem_used = memory_used;
                state.sequence.assign(current_sequence.begin(), current_sequence.end());
                state.sequence.push_back(node.number);

                best_state = state;
            } else {
                // Memory exceeded: naive recompute strategy
                DPState state;
                state.time_cost = 0;
                state.mem_used = 0;
                state.sequence.clear();

                // Recompute all inputs
                for (int inp : node.inputs) {
                    state.time_cost += node_map[inp].timecost;
                    state.mem_used += node_map[inp].outputmem;
                    state.sequence.push_back(inp);
                }
                state.time_cost += node.timecost;
                state.mem_used += node.runmem + node.outputmem;
                state.sequence.push_back(node.number);

                best_state = state;
            }
        }

        dp[node_num] = best_state;
    }

    // Build final sequence from DP of last node
    int last_node_num = topo_order.back();
    return dp[last_node_num].sequence;
}

int main() {
    std::string input_file = "test_out/example7.txt"; // replace with your file
    auto [memory, nodes] = ingestNodes(input_file);

    std::vector<int> order = topoSort(nodes);

    std::cout << "Topological order: ";
    for (int n : order) std::cout << n << " ";
    std::cout << "\n";

    std::vector<int> exec_sequence = ExecuteOrder(nodes, order, memory);

    std::cout << "\nExecution sequence respecting memory constraints:\n";
    long total_time = 0; // <-- add this
    for (int n : exec_sequence) {
        auto &node = nodes[n];
        std::cout << node.name << "(" << n << ") ";
        total_time += node.timecost; // <-- sum up timecost
    }
    std::cout << "\n";

    std::cout << "\nTotal execution time (sum of node timecosts): " 
              << total_time << "\n"; // <-- print total time

    return 0;
}
