#ifndef CFDP_H
#define CFDP_H

#include "CFDP_PDU.h"

/**
 * @brief Simulate network delay based on a specified delay time.
 */
void simulateNetworkDelay(void);

/**
 * @brief Sends a CFDP PDU (Protocol Data Unit).
 *
 * @param header Pointer to the PDU header.
 * @param content Pointer to the PDU content.
 * @param segmentNumber The number of the segment being sent.
 * @param fileContent The content of the file being sent.
 * @param segmentSize The size of the segment being sent.
 * @return int Returns 1 if the PDU was sent successfully, 0 otherwise.
 */
int sendPDU(CF_CFDP_PduFileDataHeader_t *header, CF_CFDP_PduFileDataContent_t *content, int segmentNumber, const char *fileContent, int segmentSize);

/**
 * @brief Receives a CFDP PDU and verifies its integrity.
 *
 * @param receivedHeader Pointer to the received PDU header.
 * @param receivedContent Pointer to the received PDU content.
 * @param segmentNumber The number of the segment being received.
 * @return int Returns 1 if the PDU is valid, 0 otherwise.
 */
int receivePDU(CF_CFDP_PduFileDataHeader_t *receivedHeader, CF_CFDP_PduFileDataContent_t *receivedContent, int segmentNumber);

/**
 * @brief Creates an ACK (Acknowledgment) PDU.
 *
 * @param dir_code The directive code of the PDU.
 * @param cc The condition code of the PDU.
 * @param segmentNumber The segment number for which the ACK is created.
 * @return CF_CFDP_PduAck_t Returns the created ACK PDU.
 */
CF_CFDP_PduAck_t createAck(CF_CFDP_FileDirective_t dir_code, CF_CFDP_ConditionCode_t cc, int segmentNumber);

/**
 * @brief Sends an ACK (Acknowledgment) PDU.
 *
 * @param ack Pointer to the ACK PDU to be sent.
 */
void sendAck(CF_CFDP_PduAck_t *ack);

/**
 * @brief Receives an ACK (Acknowledgment) PDU and verifies its integrity.
 *
 * @param segmentNumber The segment number associated with the ACK.
 * @param ack Pointer to the received ACK PDU.
 * @return int Returns 1 if the ACK is valid, 0 otherwise.
 */
int receiveAck(int segmentNumber, CF_CFDP_PduAck_t *ack);

/**
 * @brief Segments the file content into PDUs (Protocol Data Units).
 *
 * @param fileContent Pointer to the file content to be segmented.
 * @param fileSize The size of the file content.
 * @param headers Pointer to the array of PDU headers to be populated.
 * @param contents Pointer to the array of PDU contents to be populated.
 * @param segmentSize The size of each segment.
 * @return int Returns the number of segments created.
 */
int segmentFileIntoPDUs(const char *fileContent, size_t fileSize, CF_CFDP_PduFileDataHeader_t **headers, CF_CFDP_PduFileDataContent_t **contents, int segmentSize);

/**
 * @brief Estimates the total transfer time for a file.
 *
 * @param fileSize The size of the file in bytes.
 * @param segmentCount The total number of segments to be sent.
 * @return double Returns the estimated transfer time in seconds.
 */
double estimateTransferTime(size_t fileSize, int segmentCount);

/**
 * @brief Sends a file using a CFDP-like protocol, handling segmentation, transmission, and acknowledgment.
 *
 * @param fileContent Pointer to the file content to be sent.
 * @param fileSize The size of the file content.
 */
void sendFile(const char *fileContent, const size_t fileSize);

/**
 * @brief Creates a confirmation file after sending data. Used to signal the server to move the file in the specified directory.
 *
 * This function writes a timestamp along with the satellite name (Sat_Name) to the first line 
 * of the confirmation file 'file_sent_confirmation.txt', but only if the destination is "OGS". 
 * Then, it writes the provided file content.
 *
 * @param fileContent The content to be written in the confirmation file.
 * @param fileMemn A string to identify if the file is being sent to an OGS ("OGS") or not.
 */
void createSentFile(const char *fileContent, const char *fileMemn);

/**
 * @brief Creates a confirmation file after sending data, with direction-aware source/destination. USED IN CASE OF ROUTING OF LARGE FILES
 *
 * This version supports inter-satellite links (forward/backward) and downlink to OGS.
 * Based on the `direction`, it writes a source-destination pair or timestamp as the first line,
 * followed by the full content of the file.
 *
 * @param fileContent The content to be written in the confirmation file.
 * @param direction Integer code indicating transmission direction:
 *        1 = forward link, 2 = backward link, any other = downlink to OGS.
 */
void createSentFile2(const char *fileContent, const int direction);

// Structure to track memory status
typedef struct
{
    size_t totalSize;
    size_t currentUsed;
    bool isAvailable;
} MemoryStatus;

extern MemoryStatus *memoryInfo; // declare as external so it can be accessed from device.c
extern uint8_t *transferActive;  // declare as external so it can be accessed from device.c

/**
 * @brief Determines the relative direction of a satellite with respect to a central satellite.
 *
 * This function calculates whether a satellite is in the forward, backward,
 * or central position relative to a given central satellite index, accounting
 * for wrap-around in a circular constellation (e.g., ring topology).
 *
 * @param sat_index Index of the satellite to evaluate.
 * @param central_index Index of the reference (central) satellite.
 * @param total_satellites Total number of satellites in the constellation.
 * 
 * @return int Direction:
 *         0 = central satellite,
 *         1 = forward (moving in increasing index order),
 *         2 = backward (moving in decreasing index order).
 */
int determine_direction(int sat_index, int central_index, int total_satellites);

void receiveMemoryInfo(const char *filename);

int sendIteration(const char *fileContent, size_t fileSize, CF_CFDP_PduFileDataHeader_t *headers, CF_CFDP_PduFileDataContent_t *contents, 
                int segmentCount, int segmentSize, uint8_t *connection_establishment, const char *filename_memory, double fake_duration);

#endif
