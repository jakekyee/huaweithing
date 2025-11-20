# Huawei Research Challenge #2 - Automatic Decision for Re-computation (Ai Compilers)

## This challenge as now been resolved. I placed 3rd out of 778 participants as the highest ranked solo participant. Thank you to Huawei for making this possible!

<img width="1213" height="313" alt="image" src="https://github.com/user-attachments/assets/74e4bc0b-ba59-4074-be2a-aa63e007992b" />

Once again, big thanks to Huawei for hosting the challenge. I had a great time visiting the Vancouver Office and meeting the people!

The relevant snippet of the challenge is displayed below.

```
Based on the above background, we propose a generalized problem. Given the input-output
relationships of operator nodes within an AI model, the output operator node of the entire
model, and the computation time, process, and memory size occupied by the results of each
operator, how should the execution order of operators be arranged under a limited total
memory size and the ability to recompute any one or more nodes, in order to minimize the time
required to produce the final output? For simplicity, the problem is based on the following
assumptions:
1. There is no memory fragmentation;
2. For each test case, the input operator names for each type of operator are fixed.
Data structure of the operator node:
class Node {
private:
std::string name_; // The name of the operator node, such as Add, Mul, etc.
std::vector<Node> inputs_; // All input nodes of the operator node
int run_mem_; // Memory required for the operator's computation process
int output_mem_; // Memory occupied by the operator's computation result
int time_cost_; // Time taken for the operator's computation
}
You are going to design a re-computation algorithm that can arrange the execution order of
operators under a limited total memory size and has the ability to recompute any one or
more nodes, in order to minimize the time required to produce the final output.
```

# High level ideas

Uses a bunch of heuristics to minimize both actual and simulated computation time. 

https://arxiv.org/abs/1905.11722

# To compile

g++ -O3 .\final_submission.cpp


# To run

.\a.exe .\test_out\example1.txt



# Results

This is optimized for speed. Although the algorithm does not necessarily produce an optimal result, the algorithm itself has been aggesively optimized for speed.


For example, the final test file with over 200000+ operations runs in under 10 seconds with my algorithm, producing a valid sequence of operations.

```
PS C:\Users\jy\Desktop\school\huaweithing\huaweithing> .\a.exe .\test_out\example7.txt
Topologically sorted nodes:
Total cost: 1788851676
Peak memory usage: 42949672960 units
Count : 3596351 count
Time taken: 7 seconds
```

