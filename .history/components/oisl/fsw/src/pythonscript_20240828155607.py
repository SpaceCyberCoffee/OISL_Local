import matplotlib.pyplot as plt
import numpy as np

# Function to read and process the files
def process_files(file1, file2):
    # Initialize lists to store the processed data
    central_sat = []
    forward_sat = []

    # Process both files simultaneously
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        for i, (line1, line2) in enumerate(zip(f1, f2)):
            # Skip every other line
            if i % 3 == 0:
                # Extract and convert the numbers
                coords1 = line1.split()
                coords2 = line2.split()
                
                # Append the first coordinate of each set
                central_sat.append(coords1[0])
                forward_sat.append(coords2[0])

    return central_sat, forward_sat

# Main execution
if __name__ == "__main__":
    # File names
    file1 = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/central_test.txt'
    file2 = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/forward_test.txt'

    # Read and process the files
    central_sat, forward_sat = process_files(file1, file2)

    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.plot(range(len(central_sat)), central_sat, label='Central SAT')
    plt.plot(range(len(forward_sat)), [float(i) - 237.5 for i in forward_sat], label='Forward SAT')

    # Set up the axes
    plt.xlabel('Raw Number')
    plt.ylabel('Coordinate Value')
    plt.title('Comparison of Central SAT and Forward SAT Coordinates')

    # Add legend and grid
    plt.legend()
    plt.grid(True)

    # Show the plot
    plt.tight_layout()
    plt.show()