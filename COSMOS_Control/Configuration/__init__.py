import numpy as np
from scipy.spatial.transform import Rotation
import math

def euler_213_to_quaternion(roll, pitch, yaw):
    """
    Convert Euler angles (Pitch, Roll, Yaw) to quaternion.

    Parameters:
        roll (float): Roll angle in radians.
        pitch (float): Pitch angle in radians.
        yaw (float): Yaw angle in radians.

    Returns:
        quaternion (numpy.ndarray): Quaternion representing the rotation.
    """
    qx = np.sin(roll / 2) * np.cos(pitch / 2) * np.cos(yaw / 2) - np.cos(roll / 2) * np.sin(pitch / 2) * np.sin(yaw / 2)
    qy = np.cos(roll / 2) * np.sin(pitch / 2) * np.cos(yaw / 2) + np.sin(roll / 2) * np.cos(pitch / 2) * np.sin(yaw / 2)
    qz = np.cos(roll / 2) * np.cos(pitch / 2) * np.sin(yaw / 2) - np.sin(roll / 2) * np.sin(pitch / 2) * np.cos(yaw / 2)
    qw = np.cos(roll / 2) * np.cos(pitch / 2) * np.cos(yaw / 2) + np.sin(roll / 2) * np.sin(pitch / 2) * np.sin(yaw / 2)

    return np.array([qx, qy, qz, qw])


def quaternion_to_euler_213(quaternion):
    """
    Convert quaternion to Euler angles (Pitch, Roll, Yaw).

    Parameters:
        quaternion (numpy.ndarray): Quaternion representing the rotation.

    Returns:
        euler_angles (numpy.ndarray): Euler angles in radians in the form (pitch, roll, yaw).
    """
    w, x, y, z = quaternion

    # Ensure quaternion is normalized
    norm = np.linalg.norm(quaternion)
    if norm == 0:
        return np.zeros((3,))

    # Calculate Euler angles
    theta = 2 * np.arccos(w)
    phi = np.arctan2(-x * y + w * z, w * w + x * x - y * y - z * z)
    psi = np.arctan2(x * z + y * w, x * x - y * y - z * z + w * w)

    # Adjusting for "213" configuration
    pitch = np.degrees(theta)
    roll = np.degrees(phi)
    yaw = np.degrees(psi)

    return np.array([pitch, roll, yaw])

# euler_angles_start = np.array([60,40,20])
#
# # Convert Euler angles to quaternions
# rotation = Rotation.from_euler('YXZ', euler_angles_start, degrees=True)
# st_quaternion = rotation.as_quat()
#
# #print(f"Quaternions: {st_quaternion}")
#
# # Convert quaternions back to Euler angles
# euler_angles_back = rotation.as_euler('YXZ', degrees=True)
#
# #print(f"Euler angles (back): {euler_angles_back}")
#
# #print("Quaternion:", euler_213_to_quaternion(np.radians(60), np.radians(40), np.radians(20)))
#
#
# st_quat_nos3 = [3.732862e-01, 4.112740e-01, -2.709756e-02 , 8.311299e-01]
# #print('ST QUAT NOS3:', st_quat_nos3)
# rot=Rotation.from_quat(st_quat_nos3)
# print('NOS3 start EUler:', rot.as_euler('YXZ', degrees=True))
#
# test_DT = [-0.05, 0.6, -0.77, 0.215]
# rot=Rotation.from_quat(test_DT)
# print('EUler DT:', rot.as_euler('YXZ', degrees=True))
# test_R = [-0.12, 0.8, -0.55, 0.19]
# rot=Rotation.from_quat(test_R)
# print('Euler real', rot.as_euler('YXZ', degrees=True))


