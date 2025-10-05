#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <set>
#include <climits>

struct Node {
    long long number;
    std::string name;
    std::vector<long long> inputs;
    long long runmem;
    long long outputmem;
    long long timecost;

    Node(long long num = 0, const std::string &n = "", const std::vector<long long> &inp = {},
         long long run = 0, long long out = 0, long long time = 0)
        : number(num), name(n), inputs(inp), runmem(run), outputmem(out), timecost(time) {}
};

// Function to ingest nodes from file
std::pair<long long, std::vector<Node>> ingestNodes(const std::string &input_file) {
    std::ifstream file(input_file);
    std::string line;
    long long total_memory = 0;
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

        long long number = std::stoll(parts[0]);
        std::string name = parts[1];
        long long runmem = std::stoll(parts[parts.size() - 3]);
        long long outputmem = std::stoll(parts[parts.size() - 2]);
        long long timecost = std::stoll(parts[parts.size() - 1]);

        std::vector<long long> inputs;
        for (size_t i = 2; i < parts.size() - 3; ++i) {
            long long inp = std::stoll(parts[i]);
            if (inp != number) inputs.push_back(inp); // skip self-dependency
        }

        nodes.emplace_back(number, name, inputs, runmem, outputmem, timecost);
    }

    return {total_memory, nodes};
}

// Function to perform topological sort
std::vector<long long> topoSort(const std::vector<Node> &nodes) {

    std::unordered_map<long long, std::vector<long long>> graph;
    std::unordered_map<long long, long long> indegree;

    std::unordered_map<long long, bool> node_exists;
    for (const auto &node : nodes) node_exists[node.number] = true;

    for (const auto &node : nodes) {
        for (long long inp : node.inputs) {
            if (node_exists[inp]) {
                graph[inp].push_back(node.number);
                indegree[node.number]++;
            }
        }
        if (indegree.find(node.number) == indegree.end()) {
            indegree[node.number] = 0;
        }
    }

    std::queue<long long> q;
    for (const auto &pair : node_exists) {
        if (indegree[pair.first] == 0) q.push(pair.first);
    }

    std::vector<long long> sorted_nodes;
    while (!q.empty()) {
        long long u = q.front();
        q.pop();
        sorted_nodes.push_back(u);

        for (long long v : graph[u]) {
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

std::vector<long long> ExecuteOrder(const std::vector<Node> &nodes, const std::vector<long long> &topo_order, long long total_memory) {

    std::unordered_map<long long, Node> node_map;
    for (auto &node : nodes) node_map[node.number] = node;

    struct DPState {
        long long time_cost;
        long long mem_used;
        std::vector<long long> sequence;
    };

    std::unordered_map<long long, DPState> dp;

    for (long long node_num : topo_order) {
        const Node &node = node_map[node_num];
        DPState best_state;
        best_state.time_cost = LLONG_MAX;

        if (node.inputs.empty()) {
            if (node.runmem + node.outputmem <= total_memory) {
                best_state.time_cost = node.timecost;
                best_state.mem_used = node.runmem + node.outputmem;
                best_state.sequence.push_back(node.number);
            }
        } else {
            std::set<long long> current_sequence;
            long long extra_time = 0;
            long long memory_used = node.runmem + node.outputmem;

            for (long long inp : node.inputs) {
                if (dp.find(inp) != dp.end()) {
                    DPState &inp_state = dp[inp];
                    extra_time += inp_state.time_cost;
                    memory_used += node_map[inp].outputmem;
                    for (long long n : inp_state.sequence) current_sequence.insert(n);
                } else {
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
                DPState state;
                state.time_cost = 0;
                state.mem_used = 0;
                state.sequence.clear();

                for (long long inp : node.inputs) {
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

    long long last_node_num = topo_order.back();
    return dp[last_node_num].sequence;
}

int main() {
    // std::string input_file = "test_out/example7.txt"; // replace with your file

    std::string input_file = "diytest_out/diytest1.txt"; 
    auto [memory, nodes] = ingestNodes(input_file);

    std::vector<long long> order = topoSort(nodes);

    std::cout << "Topological order: ";
    for (long long n : order) std::cout << n << " ";
    std::cout << "\n";

    std::vector<long long> exec_sequence = ExecuteOrder(nodes, order, memory);

    std::cout << "\nExecution sequence respecting memory constraints:\n";
    long long total_time = 0;
    for (long long n : exec_sequence) {
        auto &node = nodes[n];
        std::cout << node.name << node.timecost << "(" << n << ") ";
        total_time += node.timecost;
    }
    std::cout << "\n";

    std::cout << "\nTotal execution time (sum of node timecosts): " 
              << total_time << "\n";

    return 0;
}
