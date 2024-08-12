import numpy as np

# Load data (assuming data is in two columns: timestamp and vector components)
# Replace 'your_velocity_file.csv' and 'your_isl_file.csv' with your actual filenames
velocity_data = np.loadtxt('your_velocity_file.csv', delimiter=',')
isl_data = np.loadtxt('your_isl_file.csv', delimiter=',')

# Normalize velocity vectors
velocity_vectors = velocity_data[:, 1:]  # Assuming columns 1-3 are the velocity vectors
velocity_magnitudes = np.linalg.norm(velocity_vectors, axis=1, keepdims=True)
normalized_velocity_vectors = velocity_vectors / velocity_magnitudes

# Extract ISL vectors (already normalized)
isl_vectors = isl_data[:, 1:]  # Assuming columns 1-3 are the ISL vectors

# Check if the timestamps match, then compare the directions
matching_indices = []
similarities = []

for i in range(len(velocity_data)):
    if np.isclose(velocity_data[i, 0], isl_data[i, 0]):  # Assuming the first column is the timestamp
        matching_indices.append(i)
        similarity = np.dot(normalized_velocity_vectors[i], isl_vectors[i])
        similarities.append(similarity)

# Check if the vectors are pointing in the same direction
threshold = 0.99  # Threshold for determining similarity (can be adjusted)
direction_similarity = np.array(similarities) > threshold

# Output the results
print("Indices with matching timestamps:", matching_indices)
print("Cosine Similarities:", similarities)
print("Direction Similarity (True if similar):", direction_similarity)
