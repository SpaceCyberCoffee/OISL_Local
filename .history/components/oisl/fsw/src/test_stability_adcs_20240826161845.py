import matplotlib.pyplot as plt
import numpy as np

# Function to read data from file
def read_data(file_path):
    data = []
    with open(file_path, 'r') as file:
        for line in file:
            # Splitting each line by space and converting to float
            row = [float(x) for x in line.split()]
            data.append(row)
    return np.array(data)

# Function to plot data
def plot_data(data):
    # Plotting the first column against row numbers
    plt.figure(figsize=(10, 5))
    plt.plot(data[:, 0], label='First Column')
    plt.xlabel('Row Number')
    plt.ylabel('Value')
    plt.title('Plot of First Column Values')
    plt.legend()
    # plt.show()

    # Plotting the second column against row numbers
    plt.figure(figsize=(10, 5))
    plt.plot(data[:, 1], label='Second Column')
    plt.xlabel('Row Number')
    plt.ylabel('Value')
    plt.title('Plot of Second Column Values')
    plt.legend()
    # plt.show()
    
    # Plotting the third column against row numbers
    plt.figure(figsize=(10, 5))
    plt.plot(data[:, 2], label='Change of OISL vector Column')
    plt.xlabel('Row Number')
    plt.ylabel('Value')
    plt.title('Plot of Second Column Values')
    plt.legend()
    # plt.show()
    
    # Plotting the second column against row numbers WITHOUT OUTLIERS
    plt.figure(figsize=(10, 5))
    plt.plot([dat for dat in data[100:, 1] if dat>0.989], label='Second Column')
    plt.xlabel('Row Number')
    plt.ylabel('Value')
    plt.axhline(y=0.99863, color='g', linestyle='-') # 3 degree
    plt.axhline(y=0.9998, color='r', linestyle='-')  # 1 degree
    plt.title('Plot of Second Column Values')
    plt.legend()
    # plt.show()
    plt.savefig("/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/2Vectors.png")

if __name__ == "__main__":
    file_path = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/forward_stability_2vectors.txt'  # Path to your data file
    data = read_data(file_path)
    plot_data(data)