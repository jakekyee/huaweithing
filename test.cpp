#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>

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

int main() {
    std::string input_file = "test_out/example1.txt"; // replace with your file
    auto [memory, nodes] = ingestNodes(input_file);

    std::cout << "Total memory: " << memory << "\n";
    for (const auto &node : nodes) {
        std::cout << "Node " << node.number << " | Name: " << node.name
                  << " | Inputs: ";
        for (int inp : node.inputs) std::cout << inp << " ";
        std::cout << "| RunMem: " << node.runmem
                  << " | OutputMem: " << node.outputmem
                  << " | TimeCost: " << node.timecost << "\n";
    }

    std::vector<int> order = topoSort(nodes);
    std::cout << "\nTopological order of nodes: ";
    for (int n : order) std::cout << n << " ";
    std::cout << "\n";

    std::unordered_map<int, Node> node_map;
    for (const auto &node : nodes) node_map[node.number] = node;

    std::cout << "\nTopo order with names:\n";
    for (int n : order) {
        std::cout << n << ": " << node_map[n].name << "\n";
    }

    return 0;
}
