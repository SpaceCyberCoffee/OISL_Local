#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_PDU.h"
#include "oisl_app.h"

const int networkDelay = 100000; // 100 ms
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps

// Simulated network functions
int sendSegment(CF_CFDP_PduFileDataHeader_t *header, CF_CFDP_PduFileDataContent_t *content, int segmentNumber) {
    // Simulate network sending
    printf("Sending PDU #%d with offset: %zu\n", segmentNumber, header->offset);
    // Here you would add the code to actually send the PDU over the network
    return 1; // Assume it always succeeds for this example
}

int receiveAck(int segmentNumber) {
    // Simulate receiving an ACK
    // TODO: This must come from the other satellite
    printf("Received ACK for PDU #%d\n", segmentNumber);
    return 1;
}

void simulateNetworkDelay(void) {
    // TODO: Maybe this is a problem
    usleep(networkDelay); // Simulate network delay (100 ms)
    // TODO: How to implement this as 100ms sim time, instead of real time?
}

// Function to segment the file content into PDUs
int segmentFileIntoPDUs(const char *fileContent, size_t fileSize, CF_CFDP_PduFileDataHeader_t headers[], CF_CFDP_PduFileDataContent_t contents[], int segmentSize) {
    int segmentCount = 0;
    int length = fileSize;
    int i;

    for (i = 0; i < length; i += segmentSize) {
        // Fill the header with the correct offset
        headers[segmentCount].offset.octets[0] = (i >> 24) & 0xFF;
        headers[segmentCount].offset.octets[1] = (i >> 16) & 0xFF;
        headers[segmentCount].offset.octets[2] = (i >> 8) & 0xFF;
        headers[segmentCount].offset.octets[3] = i & 0xFF;

        // Copy the segment data into the content structure
        strncpy((char *)contents[segmentCount].data, &fileContent[i], segmentSize);
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
    const int segmentSize = CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t) - CF_CFDP_MIN_HEADER_SIZE;
    CF_CFDP_PduFileDataHeader_t headers[1000]; // Assuming a max of 1000 PDUs
    CF_CFDP_PduFileDataContent_t contents[1000];
    int segmentCount = segmentFileIntoPDUs(fileContent, fileSize, headers, contents, segmentSize);
    double transferTime = estimateTransferTime(fileSize, segmentCount);
    
    printf("OISL FILE CFDP: Estimated Transfer time including network delay: %f s.\n", transferTime);

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;

        while (!sent) {
            // TODO IMPLEMENT ALIGNMENT CHECK
            if (OISL_AppData.DevicePkt.Oisl.ForwardAlignment == 1) {
                sent = sendSegment(&headers[segmentNumber], &contents[segmentNumber], segmentNumber);
                simulateNetworkDelay();

                if (receiveAck(segmentNumber)) {
                    sent = 1;
                } else {
                    printf("Resending PDU #%d\n", segmentNumber);
                }
            }
            else {
                printf("Wait for re-alignment\n");
                // TODO GET SIM TIME TO SLEEP
                sleep(10);           
            }
        }
    }
}
