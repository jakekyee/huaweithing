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



def ingestnodes(input_file):
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
    return total_memory, nodes

def toposort(nodes):
    from collections import defaultdict, deque

    # Build graph and indegree count
    graph = defaultdict(list)       # node_number -> list of dependent node_numbers
    indegree = defaultdict(int)     # node_number -> number of unsatisfied dependencies

    node_numbers = set(node.number for node in nodes)

    for node in nodes:
        for inp in node.inputs:
            if inp in node_numbers:        # ignore missing nodes
                graph[inp].append(node.number)
                indegree[node.number] += 1

        # Ensure node is in indegree dict
        if node.number not in indegree:
            indegree[node.number] = 0

    # Queue of nodes with zero indegree (no dependencies)
    queue = deque([n for n in node_numbers if indegree[n] == 0])

    sorted_nodes = []

    while queue:
        u = queue.popleft()
        sorted_nodes.append(u)

        for v in graph[u]:
            indegree[v] -= 1
            if indegree[v] == 0:
                queue.append(v)

    # Check for cycles
    if len(sorted_nodes) != len(nodes):
        print("Warning: cycle detected or missing dependencies! Topo sort incomplete.")

    return sorted_nodes

def main():
    input_file = "test_out/example1.txt"  # Replace with your filename
    memory, nodes = ingestnodes(input_file)

    print(memory)
    for node in nodes:
        print(f"Node {node.number} | Name: {node.name} | Inputs: {node.inputs} | "
            f"RunMem: {node.runmem} | OutputMem: {node.outputmem} | TimeCost: {node.timecost}")

    order = toposort(nodes)
    print("\nTopological order of nodes:", order)

    # If you want full Node info in topo order:
    node_dict = {node.number: node for node in nodes}
    print("\nTopo order with names:")
    for n in order:
        node = node_dict[n]
        print(f"{node.number}: {node.name}")

if __name__ == "__main__":
    main()
