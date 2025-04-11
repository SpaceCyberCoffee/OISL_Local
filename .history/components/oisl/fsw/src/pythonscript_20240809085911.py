import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.cm as cm

# Function to read vector components from a text file
def read_vectors_from_file_ISL(filename):
    vectors = []
    i = 0
    count = 50
    with open(filename, 'r') as file:
        for line in file:
            start = [0,0,0]
            components = list(map(float, line.strip().split()))
            if i == 50:
                vectors.append(start + components + [count])
                i = 0
                count += 50
            i += 1
    return vectors

def read_vectors_from_file_SUN(filename):
    vectors = []
    i = 0
    count = 50
    with open(filename, 'r') as file:
        for line in file:
            start = [0,0,1]
            components = list(map(float, line.strip().split()))
            if i == 500:
                vectors.append(start + components + [count])
                i = 0
                count += 50
            i += 1
    return vectors

quat = np.array(-0.5510871, -0.4059919, -0.5108982, 0.5200544)
rot = Rotation.from_quat(quat)
rot_euler = rot.as_euler('YXZ', degrees=True) # 213 INTRINSIC CAUSE THE EULER ANGLES ARE IN BODY FRAME

# Read the vectors from the file
filename_ISL = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/testchanges_ISL.txt'
vector_components_ISL = read_vectors_from_file_ISL(filename_ISL)
# Read the vectors from the file
filename_SUN = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/testchanges_sun.txt'
vector_components_SUN = read_vectors_from_file_SUN(filename_SUN)

vector_components = vector_components_ISL + vector_components_SUN

# Unpack the direction components
X, Y, Z, U, V, W, labels = zip(*vector_components)

# Generate a list of colors for each set of vectors
num_vectors_ISL = len(vector_components_ISL)
num_vectors_SUN = len(vector_components_SUN)
print(num_vectors_ISL, num_vectors_SUN)

colors_ISL = cm.rainbow(np.linspace(0, 1, num_vectors_ISL))
colors_SUN = cm.rainbow(np.linspace(0, 1, num_vectors_SUN))
colors = np.concatenate((colors_ISL, colors_SUN), axis=0)

# Create a figure and a 3D subplot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Offset for labels
label_offset = 0.3

# Plot the vectors with different colors and labels
for i in range(len(vector_components)):
    ax.quiver(X[i], Y[i], Z[i], U[i], V[i], W[i], color=colors[i])
    # Move the label away from the vector slightly
    ax.text(X[i] + U[i] + label_offset, Y[i] + V[i] + label_offset, Z[i] + W[i] + label_offset, 
            f'{int(labels[i])}s', color=colors[i])

# Set the axes limits
ax.set_xlim([-1, 2])
ax.set_ylim([-2, 2])
ax.set_zlim([-1, 1])

# Show the plot
plt.show()
plt.close()
