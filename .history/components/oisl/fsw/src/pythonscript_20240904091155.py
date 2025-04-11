import matplotlib.pyplot as plt
import numpy as np
import math

# Function to read and process the files
def process_files(file42, file2):
    # Initialize lists to store the processed data
    central_sat = []
    forward_sat = []

    # Process both files simultaneously
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        for i, (line1, line2) in enumerate(zip(f1, f2)):
            # Skip every other line
            if i % 2 == 0:
                # Extract and convert the numbers
                coords1 = line1.split()
                coords2 = line2.split()
                
                # Append the first coordinate of each set
                central_sat.append(float(coords1[0]))
                forward_sat.append(float(coords2[0]))

    return central_sat, forward_sat

def calculate_orbital_period(altitude_km):
    # Constants
    earth_radius_km = 6371  # Radius of the Earth in kilometers
    gravitational_constant = 6.674 * 10**-11  # Gravitational constant in N*m^2/kg^2
    earth_mass_kg = 5.972 * 10**24  # Mass of the Earth in kilograms

    # Calculate orbital period
    orbital_period_seconds = 2 * math.pi * math.sqrt((((earth_radius_km + altitude_km)*1000)**3) / (gravitational_constant * earth_mass_kg))

    return orbital_period_seconds

# Main execution
if __name__ == "__main__":
    # File names
    file42 = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/FORWARD_position_42.txt'
    filePROP = '/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/FORWARD_position_PROPAGATED.txt'

    # Read and process the files
    forward_42, forward_PROP = process_files(file42, filePROP)
    print(calculate_orbital_period(400))

    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.plot(range(len(central_sat)), central_sat, label='Central SAT')
    plt.plot([i + 231 for i in range(len(forward_sat))], forward_sat, label='Forward SAT')

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
    
