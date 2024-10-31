#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_PDU.h"
#include "oisl_app.h"
#include "CFDP_Luca.h"
#include <time.h>

#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/platform_inc/generic_adcs_msgids.h"
#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/src/generic_adcs_msg.h"
#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/src/generic_adcs_adac.h"
#include "cfe.h"

const int networkDelay = 7000; // 7 ms ONE TRIP
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps

static const char* fileSent_confirmation = "/home/jstar/Desktop/github-nos3/file_sent.txt";


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
    printf("\"%.*s\"\n", (int)(CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t)), receivedContent->data);

    if (pduValid) {
        printf("PDU #%d is valid. Sending ACK.\n", segmentNumber);
        return 1; // Indicates the PDU is valid and ready for ACK
    } else {
        printf("PDU #%d is invalid. No ACK will be sent.\n", segmentNumber);
        return 0; // Indicates the PDU is invalid
    }
}

CF_CFDP_PduAck_t createAck(CF_CFDP_FileDirective_t dir_code, CF_CFDP_ConditionCode_t cc, int segmentNumber) {
    CF_CFDP_PduAck_t ack;
    ack.directive_and_subtype_code.octets[0] = (uint8)((dir_code << 4) | 1); // Directive and subtype code
    ack.cc_and_transaction_status.octets[0] = (uint8)((cc << 4) | CF_CFDP_TransactionStatus_SUCCESS); // Condition code and transaction status

    // For simplicity, print ACK details
    printf("Created ACK for PDU #%d\n", segmentNumber);
    printf("Directive and subtype code: %u\n", (unsigned int)ack.directive_and_subtype_code.octets[0]);
    printf("Condition code and transaction status: %u\n", (unsigned int)ack.cc_and_transaction_status.octets[0]);

    return ack;
}

void sendAck(CF_CFDP_PduAck_t *ack) {
    // Simulate sending ACK (in reality, you'd send this over the network)
    printf("Sending ACK with directive and subtype code: %d, condition code and transaction status: %d\n",
           ack->directive_and_subtype_code.octets[0], ack->cc_and_transaction_status.octets[0]);
}

int receiveAck(int segmentNumber, CF_CFDP_PduAck_t *ack) {
    // TODO: This must come from the other satellite
    // TODO: IMOPLEMENT A TIMEOUT. IF THE ACK IS NOT RECEIVED WITHIN TIME OUT, ASSUME FAILURE
    // Simulate a random chance of ACK delay or loss
    // Simulate verifying the ACK
    if (ack->cc_and_transaction_status.octets[0] == CF_CFDP_TransactionStatus_SUCCESS) {
        printf("Received valid ACK for PDU #%d\n", segmentNumber);
        return 1; // Valid ACK received
    } else {
        printf("Received invalid or error ACK for PDU #%d\n", segmentNumber);
        return 0; // Invalid ACK, might need to retransmit
    }
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

void createSentFile(const char *fileContent) {
    // TODO source and dest must be defined by the file content. 
    FILE *sentFile = fopen(fileSent_confirmation, "w");
    if (sentFile != NULL) {
        // Write the entire file content to the confirmation file
        fprintf(sentFile, "%s", fileContent);
        fclose(sentFile);
        printf("File 'file_sent_confirmation.txt' created with the content copied from fileContent.\n");
    } else {
        printf("Error creating 'file_sent_confirmation.txt' file.\n");
    }
}

// Function to extract the ground station name directly from the file content
void extractOGSName(const char *fileContent, char *OGS_name, size_t max_len) {
    char command[1000];
    FILE *fp;
    char buffer[500];

    FILE *temp_file = fopen("/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/ftemp.json", "w");
    if (temp_file != NULL) {
        fputs(fileContent, temp_file);
        fclose(temp_file);
        snprintf(command, sizeof(command), 
                "python3 /home/jstar/Desktop/github-nos3/components/oisl/fsw/src/OGS_name.py /home/jstar/Desktop/github-nos3/components/oisl/fsw/src/ftemp.json");
    }

    // // Prepare the command to call the Python script
    // snprintf(command, sizeof(command), 
    //          "python3 /home/jstar/Desktop/github-nos3/components/oisl/fsw/src/OGS_name.py '%s'", 
    //          fileContent);

    printf("Command: %s\n", command);

    // Run the Python script and capture its output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run Python script\n");
    }

    // Read the output and parse the position vector
    if (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        if (sscanf(buffer, "OGS Name: %s", OGS_name) == 1) { 
            // Successfully parsed the position vector
            printf("Got the propagation");
        } else {
            // Print other output lines if any
            printf("FAILED.\n");
            printf("%s", buffer);
        }
    }
    pclose(fp);
}

// Function to check if a given time is within a visibility window
int is_visible(time_t current_time, const char *vis_start, const char *vis_end) {
    struct tm tm_start, tm_end;
    time_t start_time, end_time;

    // Parse visibility start and end times
    strptime(vis_start, "%Y-%m-%dT%H:%M:%S", &tm_start);
    strptime(vis_end, "%Y-%m-%dT%H:%M:%S", &tm_end);

    // Convert to time_t format for comparison
    start_time = mktime(&tm_start);
    end_time = mktime(&tm_end);

    return (current_time >= start_time && current_time <= end_time);
}

// Function to check if Sat_1_2 is visible sooner
int routing_Sat(FILE *vis_file, time_t current_time) {
    char line[256];
    char sat_id[35], vis_start[35], vis_end[35]; //todo improve time format in vis files and use 30 here
    double duration;
    time_t earliest_visibility = 0;
    int recommend_forward = 0;

    // Reset file pointer to the beginning for a fresh search
    rewind(vis_file);

    while (fgets(line, sizeof(line), vis_file) != NULL) {
        // Parse satellite ID, visibility start and end times
        if (sscanf(line, "%35[^,],%35[^,],%35[^,],%lf", sat_id, vis_start, vis_end, &duration) == 4) {
            printf("Line 212");
            struct tm tm_start;
            time_t start_time;

            // Parse start time for comparison
            strptime(vis_start, "%Y-%m-%dT%H:%M:%S", &tm_start);
            start_time = mktime(&tm_start);

            // If the entry is for Sat_1_2 and has an upcoming visibility
            if (strcmp(sat_id, "Sat_1_2") == 0 && start_time > current_time) {
                printf("Line 222");
                if (earliest_visibility == 0 || start_time < earliest_visibility) {
                    earliest_visibility = start_time;
                    recommend_forward = 1;
                    break;
                }
            }
            // If Sat_1_1 has an earlier visibility window, do not recommend forward
            else if (strcmp(sat_id, "Sat_1_1") == 0 && start_time > current_time && start_time < earliest_visibility) {
                recommend_forward = 0;
                break;
            }
        }
    }

    if (recommend_forward) {
        printf("Recommendation: Better to connect with forward satellite (Sat_1_2) for earlier visibility.\n");
    } else {
        printf("No recommendation to connect with forward satellite.\n");
    }

    return recommend_forward;
}

// CFDP-like file sending function
void sendFile(const char *fileContent, const size_t fileSize) {
    // TODO: EXTEND TO INCLUDE POSSIBLER FILES CONTAINIGN BOTH TM AND COMMANDS. (IF REALISTIC)
    int segmentNumber = 0;
    const int segmentSize = CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t) - CF_CFDP_MIN_HEADER_SIZE;
    /* If each PDU can carry 504 bytes of data and you can send a maximum of 1000 PDUs, the maximum file size in bytes is: Max File Size (bytes)=504×1000=504000 bytes = 504 kB*/
    CF_CFDP_PduFileDataHeader_t *headers = NULL;
    CF_CFDP_PduFileDataContent_t *contents = NULL;
    int segmentCount = segmentFileIntoPDUs(fileContent, fileSize, &headers, &contents, segmentSize);
    double transferTime = estimateTransferTime(fileSize, segmentCount);

    uint8 *connection_establishment;
    if (OISL_AppData.CFDP.Target == 0) {
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.BackwardConnection;
    }
    else if (OISL_AppData.CFDP.Target == 1)
    {
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
    }
    else if (OISL_AppData.CFDP.Target == 2)
    {   
        char OGS_name[64]; // Buffer to hold the extracted ground station name
        // Extract the ground station name from the file content
        extractOGSName(fileContent, OGS_name, sizeof(OGS_name));
        // Print the OGS name as confirmation
        printf("CFDP: File received. OGS specified for the DL: %s\n", OGS_name);
        // Open the visibility file 
        char vis_file_name[200];
        snprintf(vis_file_name, sizeof(vis_file_name), "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/OGS_visibilities/%s_vis_prediction.txt", OGS_name);
        FILE *vis_file = fopen(vis_file_name, "r");
        if (vis_file == NULL) {
            perror("Failed to open visibility prediction file");
        }
        // Base time for the simulation (2025-10-18 08:30:00 UTC)
        struct tm base_time = { .tm_year = 2025 - 1900, .tm_mon = 10 - 1, .tm_mday = 18,
                                .tm_hour = 8, .tm_min = 30, .tm_sec = 0, .tm_isdst = -1 };
        time_t base_timestamp = mktime(&base_time);
        // Retrieve current simulation time in seconds and subseconds
        CFE_TIME_SysTime_t nowT = CFE_TIME_GetTime();
        uint32 seconds = nowT.Seconds;
        uint32 subseconds = nowT.Subseconds;
        // Convert to full timestamp in seconds
        double sim_seconds = (double)seconds + ((double)subseconds / 4294967296.0);
        time_t current_time = base_timestamp + (time_t)sim_seconds;

        char line[256];
        int is_visible_now = 0;

        // ADCS MODE will be changed
        Generic_ADCS_Mode_cmd_t cmd8;
        CFE_MSG_Init(CFE_MSG_PTR(cmd8.CmdHeader), CFE_SB_ValueToMsgId(GENERIC_ADCS_CMD_MID), sizeof(Generic_ADCS_Mode_cmd_t)); //TODO then move these lines above
        CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&cmd8, GENERIC_ADCS_SET_MODE_CC);

        while (fgets(line, sizeof(line), vis_file) != NULL) {
            printf("Line 291\n");
            char sat_id[35], vis_start[35], vis_end[35];
            double duration;

            // Ensure line ends at '\n' and doesn't include any hidden characters 
            line[strcspn(line, "\r\n")] = 0;
            int items = sscanf(line, "%35[^,],%35[^,],%35[^,], %lf", sat_id, vis_start, vis_end, &duration); //TODO: REMOVE THE 00+00 and improve time format in the visibility file so that you can reduce froom 35 to 30
            printf("Sat id: %s then vis start: %s \n vis end: %s \n duration %f \n", sat_id, vis_start, vis_end,  duration);
            if (items == 4) { 
                printf("Line 297\n");
                if (strcmp(sat_id, "Sat_1_1") == 0 && is_visible(current_time, vis_start, vis_end)) {
                    printf("Satellite %s is currently visible from OGS.\n", sat_id);
                    is_visible_now = 1;
                    break;
                }
            }
            else { 
                // Print to debug what was read 
                printf("Parsed %d items: %s\n", items, line); 
            }
        }

        if (!is_visible_now) {
            printf("Satellite Sat_1_1 is not visible from OGS at this time.\n");
            int routing = routing_Sat(vis_file, current_time); // Call routing_Sat to check visibility of Sat_1_2 TODO generalize
            if (routing == 1) {
                connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
                cmd8.Mode = OISL_MODE_F;
            }
            else {
                cmd8.Mode = OISL_MODE_OGS;
            }
        }
        else {
        cmd8.Mode = OISL_MODE_OGS;
        }

        // Transmit the MSG to modify ADCS mode
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&cmd8);
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&cmd8, true);

        // Close the visibility file
        fclose(vis_file);
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection; // TODO wait for what to align? maybe wait for the visibility? if now > start visibility then connection established and go
    }
    else {
        printf("Unknown taget to align with, or method not yet impemented for target %u, default to forward", OISL_AppData.CFDP.Target);
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
    }
    
    printf("OISL FILE CFDP: Estimated Transfer time including network delay: %f s.\n", transferTime);

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;
        int retries = 0;
        const int maxRetries = 5;

        while (!sent && retries < maxRetries) {
            if (*connection_establishment == 1) {
                sent = sendPDU(&headers[segmentNumber], &contents[segmentNumber], segmentNumber, fileContent, segmentSize);
                if (sent == 1) {
                    simulateNetworkDelay();

                    // Simulate the other satellite receiving the PDU TODO THIS MUST COME FROM OTHER SAT
                    if (receivePDU(&headers[segmentNumber], &contents[segmentNumber], segmentNumber)) {
                        CF_CFDP_PduAck_t ack = createAck(CF_CFDP_FileDirective_ACK, CF_CFDP_ConditionCode_NO_ERROR, segmentNumber);
                        sendAck(&ack);

                        // Simulate the sender receiving the ACK
                        if (!receiveAck(segmentNumber, &ack)) {
                            sent = 0;
                            retries++;
                            printf("PDU #%d not acknowledged, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                        }
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
            break;
        }
    }

    if (segmentNumber == segmentCount) {
        printf("All PDUs sent successfully. File transmission is over \n");
        createSentFile(fileContent);
    } 
    else {
        printf("File transmission incomplete.\n");
    }

    // Free dynamically allocated memory
    free(headers);
    free(contents);
}