#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_Luca.h"

// Simulated network functions
int sendSegment(const char *segment, int segmentNumber) {
    // Simulate network sending
    printf("Sending segment #%d: %s\n", segmentNumber, segment);
    return 1; // Assume it always succeeds for this example
}

int receiveAck(int segmentNumber) {
    // Simulate receiving an ACK
    printf("Received ACK for segment #%d\n", segmentNumber);
    return 1;
}

void simulateNetworkDelay() {
    usleep(100000); // Simulate network delay (100 ms)
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

// CFDP-like file sending function
void sendFile(const char *fileContent) {
    int segmentNumber = 0;
    const int segmentSize = 4;
    char segments[100][5]; // Assuming a max of 100 segments, each up to 4 chars + '\0'
    int segmentCount = segmentFile(fileContent, segments, segmentSize);

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;

        while (!sent) {
            sent = sendSegment(segments[segmentNumber], segmentNumber);
            simulateNetworkDelay();

            if (receiveAck(segmentNumber)) {
                sent = 1;
            } else {
                printf("Resending segment #%d\n", segmentNumber);
            }
        }
    }
}

