#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_Luca.h"
#include "oisl_app.h"

const int networkDelay = 100000; // 100 ms
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps

// Simulated network functions
int sendSegment(const char *segment, int segmentNumber) {
    // Simulate network sending
    printf("Sending segment #%d: %s\n", segmentNumber, segment);
    return 1; // Assume it always succeeds for this example
}

int receiveAck(int segmentNumber) {
    // Simulate receiving an ACK
    // TODO: THIS MUST COME FROM THE OTHER SATELLITE
    printf("Received ACK for segment #%d\n", segmentNumber);
    return 1;
}

void simulateNetworkDelay(void) {
    usleep(networkDelay); // Simulate network delay (100 ms)
    // TODO: How to implement this as 100ms sim time, instead of real time?
}

// Function to segment the file content
int segmentFile(const char *fileContent, char segments[][5], int segmentSize) {
    int segmentCount = 0;
    int length = strlen(fileContent);
    int i;

    for (i = 0; i < length; i += segmentSize) {
        strncpy(segments[segmentCount], &fileContent[i], segmentSize);
        segments[segmentCount][segmentSize] = '\0'; // Ensure null termination
        segmentCount++;
    }

    return segmentCount;
}

double estimateTransferTime(size_t fileSize, int segmentCount) {
    // Estimate transfer time
    double fileSizeBits = fileSize * 8.0; // Convert file size to bits
    double transferSpeedBps = transferSpeedMbps * 1e6; // Convert Mbps to bps
    double estimatedTransferTimeSeconds = fileSizeBits / transferSpeedBps; // Time in seconds
    // Get segmentCount to take into consideration the Network delay
    double delay = segmentCount * (networkDelay / 1e6); // seconds
    return estimatedTransferTimeSeconds + delay;
}

// CFDP-like file sending function
void sendFile(const char *fileContent, const size_t fileSize) {
    int segmentNumber = 0;
    const int segmentSize = 4;
    char segments[1000][5]; // Assuming a max of 1000 segments, each up to 4 chars + '\0'
    int segmentCount = segmentFile(fileContent, segments, segmentSize);
    double transferTime = estimateTransferTime(fileSize, segmentCount);
    
    printf("OISL FILE CFDP: Estimated Transfer time including network delay: %f s.", transferTime);

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;

        while (!sent) {
            // TODO IMPLEMENT ALIGNMENT CHECK
            if (OISL_AppData.DevicePkt.Oisl.ForwardAlignment == 1) {
                sent = sendSegment(segments[segmentNumber], segmentNumber);
                simulateNetworkDelay();

                if (receiveAck(segmentNumber)) {
                    sent = 1;
                } else {
                    printf("Resending segment #%d\n", segmentNumber);
                }
            }
            else {
                printf("Wait for re-alignment");
                // TODO GET SIM TIME TO SLEEP
                sleep(5);           
            }
        }
    }
}

