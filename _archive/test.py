from collections import defaultdict, deque

# ---------------------------
# Node class
# ---------------------------
class Node:
    def __init__(self, number=0, name="", inputs=None, runmem=0, outputmem=0, timecost=0):
        self.number = number
        self.name = name
        self.inputs = inputs if inputs is not None else []
        self.runmem = runmem
        self.outputmem = outputmem
        self.timecost = timecost

# ---------------------------
# Topological sort
# ---------------------------
def topo_sort(nodes):
    print(len(nodes))
    indegree = defaultdict(int)
    graph = defaultdict(list)
    for node in nodes:
        for inp in node.inputs:
            graph[inp].append(node.number)
            indegree[node.number] += 1
    queue = deque([node.number for node in nodes if indegree[node.number] == 0])
    order = []
    while queue:
        u = queue.popleft()
        order.append(u)
        for v in graph[u]:
            indegree[v] -= 1
            if indegree[v] == 0:
                queue.append(v)

    print(len(order))
    
    return order

# ---------------------------
# Sliding DP function
# ---------------------------
def sliding_dp(nodes, total_memory, window_size=12):
    node_dict = {node.number: node for node in nodes}
    topo_order = topo_sort(nodes)
    
    execution_sequence = []
    memory_nodes = set()  # nodes currently in memory
    memory_used = 0
    time_elapsed = 0
    print(len(topo_order))
    
    for w_start in range(0, len(topo_order), window_size):
        window_nodes = topo_order[w_start:w_start+window_size]
        window_set = set(window_nodes)
        
        dp = dict()
        dp[frozenset(memory_nodes)] = (time_elapsed, [])
        print("test")
        
        for _ in range(len(window_nodes)):
            new_dp = dict()
            for mem_nodes, (curr_time, seq) in dp.items():
                mem_nodes_set = set(mem_nodes)
                # ready_nodes = [n for n in window_nodes if n not in mem_nodes_set
                #                and all(inp in mem_nodes_set or inp in memory_nodes for inp in node_dict[n].inputs)]
                
                ready_nodes = [n for n in window_nodes if n not in mem_nodes_set
               and (n == 0 or all(inp in mem_nodes_set or inp in memory_nodes for inp in node_dict[n].inputs))]

                print("test")
                for node in ready_nodes:
                    print(f"Node {node.number} | Name: {node.name} | Inputs: {node.inputs} | "
                        f"RunMem: {node.runmem} | OutputMem: {node.outputmem} | TimeCost: {node.timecost}")

                for n in ready_nodes:
                    node = node_dict[n]
                    required_mem = node.runmem + node.outputmem
                    available_mem = total_memory - sum(node_dict[m].outputmem for m in mem_nodes_set)
                    evict_candidates = list(mem_nodes_set)
                    best_eviction = []
                    if required_mem > available_mem:
                        evict_candidates.sort(key=lambda x: node_dict[x].timecost / max(1, node_dict[x].outputmem))
                        freed = 0
                        evict_list = []
                        for e in evict_candidates:
                            freed += node_dict[e].outputmem
                            evict_list.append(e)
                            if freed + available_mem >= required_mem:
                                break
                        best_eviction = evict_list
                        available_mem += sum(node_dict[e].outputmem for e in best_eviction)
                    if required_mem <= available_mem:
                        new_mem_nodes = (mem_nodes_set - set(best_eviction)) | {n}
                        new_time = curr_time + node.timecost
                        new_seq = seq + [n]
                        fs = frozenset(new_mem_nodes)
                        if fs not in new_dp or new_time < new_dp[fs][0]:
                            new_dp[fs] = (new_time, new_seq)
            dp = new_dp
        
        if dp:
            best_fs, (time_elapsed, seq) = min(dp.items(), key=lambda x: x[1][0])
            execution_sequence += seq
            memory_nodes = set(best_fs)
    
    return execution_sequence, time_elapsed

# ---------------------------
# Main function
# ---------------------------
def main():
    input_file = "test_out/example1.txt"  # Replace with your filename
    total_memory = 1000  # Replace with the memory limit from the first line of the file




    nodes = []
    with open(input_file, "r") as f:
        first_line = f.readline().strip()
        # total_memory = int(first_line)
        total_memory = int(first_line.split(" ")[1])  # use memory limit from file

        for line in f:
            parts = line.strip().split()
            if len(parts) < 4:
                continue
            number = int(parts[0])
            name = parts[1]
            runmem = int(parts[-3])
            outputmem = int(parts[-2])
            timecost = int(parts[-1])
            # inputs = [int(x) for x in parts[2:-3]]  # everything between name and runmem

            inputs = [int(x) for x in parts[2:-3] if int(x) != number]  # skip self-dependency


            node = Node(number=number, name=name, inputs=inputs,
                        runmem=runmem, outputmem=outputmem, timecost=timecost)
            nodes.append(node)

    # print("Nodes loaded from file:")
    # for node in nodes:
    #     print(f"Node {node.number} | Name: {node.name} | Inputs: {node.inputs} | "
    #         f"RunMem: {node.runmem} | OutputMem: {node.outputmem} | TimeCost: {node.timecost}")

    
    execution_seq, total_time = sliding_dp(nodes, total_memory, window_size=12)
    
    print("Execution sequence:", execution_seq)
    print("Total simulated time:", total_time)

# ---------------------------
# Run main
# ---------------------------
if __name__ == "__main__":
    main()
