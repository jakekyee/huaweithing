int main() {
    std::string input_file = "test_out/example1.txt"; // replace with your file
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
