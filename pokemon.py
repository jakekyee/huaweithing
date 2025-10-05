def sum_last_numbers(filename):
    """
    Returns the sum of the last number from every line in a file, excluding the first line.
    Returns integer if all numbers are integers, otherwise float.
    """
    total = 0
    all_integers = True
    
    try:
        with open(filename, 'r') as file:
            # Skip the first line
            next(file)
            
            for line in file:
                parts = line.strip().split()
                if parts:
                    last_element = parts[-1]
                    try:
                        # Try to convert to float
                        num = float(last_element)
                        total += num
                        
                        # Check if it's an integer
                        if all_integers and not num.is_integer():
                            all_integers = False
                            
                    except ValueError:
                        continue
                        
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return 0
    except StopIteration:
        # File had only one line
        return 0
    except Exception as e:
        print(f"Error reading file: {e}")
        return 0
    
    # Return integer if all numbers were integers, otherwise float
    return int(total) if all_integers else total




result = sum_last_numbers('test_out\example1.txt')
# result = sum_last_numbers('diytest_out\diytest1.txt')
print(result)