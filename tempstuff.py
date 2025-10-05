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
class CurrentState:
    def __init__(self, number=0, name="", inputs=None, runmem=0, outputmem=0, timecost=0):
        self.mem = []
        self.mem_max = name
        self.inputs = inputs if inputs is not None else []
        self.runmem = runmem
        self.outputmem = outputmem
        self.timecost = timecost



# ---------------------------
# Topological sort
# ---------------------------
# def topo_sort(nodes):
#     print(len(nodes))
#     indegree = defaultdict(int)
#     graph = defaultdict(list)
#     for node in nodes:
#         for inp in node.inputs:
#             graph[inp].append(node.number)
#             indegree[node.number] += 1
#     queue = deque([node.number for node in nodes if indegree[node.number] == 0])
#     order = []
#     while queue:
#         u = queue.popleft()
#         order.append(u)
#         for v in graph[u]:
#             indegree[v] -= 1
#             if indegree[v] == 0:
#                 queue.append(v)

#     print(len(order))
    
#     return order


def topo_sort(nodes):
    indegree = defaultdict(int)
    graph = defaultdict(list)
    
    # Map node numbers to node objects
    node_dict = {node.number: node for node in nodes}
    
    for node in nodes:
        for inp in node.inputs:
            graph[inp].append(node.number)
            indegree[node.number] += 1
    
    queue = deque([node.number for node in nodes if indegree[node.number] == 0])
    order = []
    
    while queue:
        u = queue.popleft()
        order.append(node_dict[u])  # append the Node object instead of the number
        for v in graph[u]:
            indegree[v] -= 1
            if indegree[v] == 0:
                queue.append(v)
    
    return order

def ingestfile(input_file):
    
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

            inputs = [int(x) for x in parts[3:-3]]  # skip self-dependency


            node = Node(number=number, name=name, inputs=inputs,
                        runmem=runmem, outputmem=outputmem, timecost=timecost)
            nodes.append(node)
    return total_memory, nodes





# remove from memory with greedy-ish algorithm
def remove_mem(topo_nodes, memory, safe, count=1, lookahead=1):
    # check ahead a couple nodes
    # for memory (pick arbiraty(in order))
    for i in memory:
        if i.number in safe:
            pass
        else:
            # for j in range(0, lookahead):
            #     if 
            # memory.remove(topo_nodes, safe, count, lookahead)
            memory.remove(i)
            current_mem = current_mem - i.outputmem
    pass
# adds to memory
def add_mem(node, topo_nodes, memory, max_mem, safe, runorder, current_mem, count=1, lookahead=1):

    i = node

    for j in i.inputs:
        safe.append(j)


    while (i.runmem + current_mem > max_mem):
        remove_mem(topo_nodes, safe, 1, 1)
    

    for j in i.inputs:
        if j in memory:
            pass
        else:
            add_mem(j, topo_nodes, max_mem, safe, runorder, 1,1)
    
    memory.append(i)
    current_mem = current_mem + i.outputmem
    # if (current_mem > highest_mem):
    #     highest_mem = current_mem

    runorder.append(i)
    return current_mem

def main():
    
    memory = []
    memorycount = 0
    current_mem = 0
    max_mem = 0
    runorder = []
    highest_mem = 0
    # input_file = "diytest_out/diytest1.txt"
    input_file = "test_out/example1.txt"
    max_mem, nodes = ingestfile(input_file)
    for i in nodes:
        print(i.number)
    topo_nodes = topo_sort(nodes)
    for i in topo_nodes:
        print(i.number)


    for i in topo_nodes:
        # check running mem
        # if running mem + cur mem > max mem:
            # remove something
        while (i.runmem + current_mem > max_mem):
            remove_mem(topo_nodes, memory, i.inputs, 1, 1)

        safe = []
        for j in i.inputs:
            safe.append(j)
        for j in i.inputs:
            if j in memory:
                pass
            else:
                add_mem(i, topo_nodes,memory, max_mem, safe,current_mem, 1, 1)
    

    for i in runorder:
        print(i.number)

    print(highest_mem)
    




main()