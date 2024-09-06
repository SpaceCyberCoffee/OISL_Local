#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_PDU.h"
#include "oisl_app.h"

const int networkDelay = 7000; // 7 ms ONE TRIP
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps


void simulateNetworkDelay(void) {
    usleep(networkDelay); // Simulate network delay for a OISL distance of around 2000km -> 7 ms
    // TODO: How to implement this as 100ms sim time, instead of real time?
}

// Simulated network functions
int sendPDU(CF_CFDP_PduFileDataHeader_t *header, CF_CFDP_PduFileDataContent_t *content, int segmentNumber, const char *fileContent, int segmentSize) {
    // Simulate a random chance of transmission failure (e.g., 10% failure rate)
    double failureRate = 0.1;
    double randomValue = (double)rand() / RAND_MAX;

    if (randomValue < failureRate) {
        return 0; // Indicate failure
    }
    printf("Sending PDU #%d \n", segmentNumber);
    printf("\"%.*s\"\n", segmentSize, content->data);
    // Here you would add the code to actually send the PDU over the network
    return 1; // Assume it always succeeds for this example
}

int receivePDU(CF_CFDP_PduFileDataHeader_t *receivedHeader, CF_CFDP_PduFileDataContent_t *receivedContent, int segmentNumber) {
    // Simulate checking the integrity of the received PDU
    // This can include checking a checksum or other error-detecting mechanism
    int pduValid = 1; // For simplicity, assume the PDU is valid (1 = valid, 0 = invalid)

    // Print received PDU data
    printf("Received PDU #%d\n", segmentNumber);
    printf("\"%.*s\"\n", CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t), receivedContent->data);

    if (pduValid) {
        printf("PDU #%d is valid. Sending ACK.\n", segmentNumber);
        return 1; // Indicates the PDU is valid and ready for ACK
    } else {
        printf("PDU #%d is invalid. No ACK will be sent.\n", segmentNumber);
        return 0; // Indicates the PDU is invalid
    }
}

int receiveAck(int segmentNumber) {
    // TODO: This must come from the other satellite
    // TODO: IMOPLEMENT A TIMEOUT. IF THE ACK IS NOT RECEIVED WITHIN TIME OUT, ASSUME FAILURE
    // Simulate a random chance of ACK delay or loss
    double ackDelayRate = 0.05; // 5% chance of delayed ACK
    double randomValue = (double)rand() / RAND_MAX;

    if (randomValue < ackDelayRate) {
        // Simulate ACK delay
        simulateNetworkDelay(); // Delay the ACK
        printf("ACK for PDU #%d delayed\n", segmentNumber);
    }
    printf("Received ACK for PDU #%d\n", segmentNumber);
    return 1;
}


// Function to segment the file content into PDUs
int segmentFileIntoPDUs(const char *fileContent, size_t fileSize, CF_CFDP_PduFileDataHeader_t **headers, CF_CFDP_PduFileDataContent_t **contents, int segmentSize) {
    int segmentCount = 0;
    int length = fileSize;
    int i;

    // Calculate the number of segments needed
    segmentCount = (fileSize + segmentSize - 1) / segmentSize;

    // Allocate memory for headers and contents dynamically
    *headers = (CF_CFDP_PduFileDataHeader_t *)malloc(segmentCount * sizeof(CF_CFDP_PduFileDataHeader_t));
    *contents = (CF_CFDP_PduFileDataContent_t *)malloc(segmentCount * sizeof(CF_CFDP_PduFileDataContent_t));

    if (*headers == NULL || *contents == NULL) {
    // Handle memory allocation failure
    fprintf(stderr, "Memory allocation failed\n");
    }

    for (i = 0; i < length; i += segmentSize) {
        // Fill the header with the correct offset
        (*headers)[i / segmentSize].offset.octets[0] = (i >> 24) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[1] = (i >> 16) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[2] = (i >> 8) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[3] = i & 0xFF;

        // Copy the segment data into the content structure
        strncpy((char *)(*contents)[i / segmentSize].data, &fileContent[i], segmentSize);
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
    /* If each PDU can carry 504 bytes of data and you can send a maximum of 1000 PDUs, the maximum file size in bytes is: Max File Size (bytes)=504×1000=504000 bytes = 504 kB*/
    CF_CFDP_PduFileDataHeader_t *headers = NULL;
    CF_CFDP_PduFileDataContent_t *contents = NULL;
    int segmentCount = segmentFileIntoPDUs(fileContent, fileSize, &headers, &contents, segmentSize);
    double transferTime = estimateTransferTime(fileSize, segmentCount);
    
    printf("OISL FILE CFDP: Estimated Transfer time including network delay: %f s.\n", transferTime);

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;
        int retries = 0;
        const int maxRetries = 5;

        while (!sent && retries < maxRetries) {
            // Check alignment
            if (OISL_AppData.DevicePkt.Oisl.ForwardAlignment == 0) {
                sent = sendPDU(&headers[segmentNumber], &contents[segmentNumber], segmentNumber, fileContent, segmentSize);
                if (sent == 1) {
                    simulateNetworkDelay();

                    if (!receiveAck(segmentNumber)) {
                        sent = 0;
                        retries++;
                        printf("PDU #%d not acknowledged, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                    } 
                } else {
                    retries++;
                    printf("PDU #%d failed to send, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                }
            } else {
                printf("Wait for re-alignment\n");
                sleep(10);  // Adjust sleep as needed
            }
        }

        if (retries == maxRetries) {
            printf("PDU #%d failed after %d retries, aborting transmission.\n", segmentNumber, maxRetries);
            break;  // or handle failure as needed
        }
    }


    printf("File transmission is over\n");

    // Free dynamically allocated memory
    free(headers);
    free(contents);

}
