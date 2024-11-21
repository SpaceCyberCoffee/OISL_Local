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

static const char* Sat_Name = "Sat_1_1";
// static const char* Sat_For = "Sat_1_2";
// static const char* Sat_Back = "Sat_1_24";
static const double margin  = 60.0;    // THis depends: if the sat is already in OGS Mode the margin is 0. If it has to get into that mode then it might be even higher. TODO add autoregolation based on the mode you are in now. 
static const double margin_routing = 200.0; //time to align with forward + time for the forward to align with OGS


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

time_t get_current_time(void) {
    // Base time for the simulation (2025-10-18 08:30:00 UTC)
    struct tm base_time = { .tm_year = 2025 - 1900, .tm_mon = 10 - 1, .tm_mday = 18,
                            .tm_hour = 8, .tm_min = 30, .tm_sec = 0, .tm_isdst = -1 };
    time_t base_timestamp = mktime(&base_time);

    // Retrieve current simulation time in seconds and subseconds
    CFE_TIME_SysTime_t nowT = CFE_TIME_GetTime();
    uint32_t seconds = nowT.Seconds;
    uint32_t subseconds = nowT.Subseconds;

    // Convert to full timestamp in seconds
    double sim_seconds = (double)seconds + ((double)subseconds / 4294967296.0);
    return base_timestamp + (time_t)sim_seconds;
}

// Function to check if a given time is within a visibility window
int is_visible(time_t current_time, const char *vis_start, const char *vis_end, time_t *vis_start_time, double fileTransferDur, double vis_duration) {
    struct tm tm_start, tm_end;
    time_t start_time, end_time;

     // Ensure tm_isdst is set to -1 to handle DST automatically TODO Prob delete
    tm_start.tm_isdst = -1;
    tm_end.tm_isdst = -1;

    // Temporary buffers to hold truncated visibility start and end times
    char vis_start_trunc[20];
    char vis_end_trunc[20];

    // Copy only the first 19 characters (ignoring fractional seconds)
    strncpy(vis_start_trunc, vis_start, 19);
    vis_start_trunc[19] = '\0';
    strncpy(vis_end_trunc, vis_end, 19);
    vis_end_trunc[19] = '\0';

    // Parse visibility start time
    if (strptime(vis_start, "%Y-%m-%dT%H:%M:%S", &tm_start) == NULL) {
        printf("Failed to parse vis_start: %s\n", vis_start_trunc);
        return 0;
    }

    // Parse visibility end time
    if (strptime(vis_end, "%Y-%m-%dT%H:%M:%S", &tm_end) == NULL) {
        printf("Failed to parse vis_end: %s\n", vis_end_trunc);
        return 0;
    } 

    // Convert to time_t format for comparison
    start_time = mktime(&tm_start);
    end_time = mktime(&tm_end);

    // Check if mktime failed
    if (start_time == -1) {
        printf("mktime failed to convert start_time.\n");
        printf("start_time: %ld, end_time: %ld\n", start_time, end_time);
        return 0;
    }
    if (end_time == -1) {
        printf("mktime failed to convert end_time.\n");
        return 0;
    }

    // Update the next upcoming visibility start time if conditions are met 
    if (start_time > current_time && (*vis_start_time == -1 || start_time < *vis_start_time)) {
        if (vis_duration >= fileTransferDur) {
            *vis_start_time = start_time;
        }
    }

    // Adjust the end time by subtracting margin and file transfer duration
    time_t adjusted_end_time = end_time - (time_t)(margin + fileTransferDur);

    // Check if the current time is within the adjusted visibility window
    return (current_time >= start_time && current_time <= adjusted_end_time);
}
 
int routing_Sat(FILE *vis_file, time_t current_time, time_t *start_visibility, double fileTransferDur) { 
    char line[300];
    char sat_id[20], vis_start[20], vis_end[20];
    double duration;
    time_t earliest_visibility = *start_visibility; // ASSUME FIRST THAT THE EARLIEST VISIBILITY FEASIBLE (CONSIDERING TRANSFER TIME AND VIS DURATION) IS THE NEXT ONE OF THE CENTRAL SAT.
    int recommend_direction = 0;
    int central_index;
    int winning_index = 1; // If no feasible visibility before the next one of this sat, select this sat

    // Extract the index of the current satellite (e.g., from `1_2` → get `2`)
    sscanf(strrchr(Sat_Name, '_') + 1, "%d", &central_index);

    // Reset file pointer to the beginning
    rewind(vis_file);

    //PRINT THE VALUE SAVED IN START_VISIBILITY IN HUMAN READABLE FORMAT DELETE THEN 
    char formatted_start_visibility[20];
    strftime(formatted_start_visibility, sizeof(formatted_start_visibility), "%Y-%m-%d %H:%M:%S", localtime(&(*start_visibility)));
    printf("Start visibility time: %s\n", formatted_start_visibility);

    while (fgets(line, sizeof(line), vis_file) != NULL) {
        if (sscanf(line, "%20[^,],%20[^,],%20[^,],%lf", sat_id, vis_start, vis_end, &duration) == 4) {
            char *trimmed_sat_id = strtok(sat_id, " \t\n\r");

            struct tm tm_start;
            time_t start_time;

            // Ensure tm_isdst is set to -1 to handle DST automatically TODO Prob delete
            tm_start.tm_isdst = -1;

            // Temporary buffers to hold truncated visibility start and end times
            vis_start[19] = '\0';

            // Parse the start time
            strptime(vis_start, "%Y-%m-%dT%H:%M:%S", &tm_start);
            start_time = mktime(&tm_start);

            // Check if mktime failed   
            if (start_time == -1) {
                printf("ROUTING : mktime failed to convert start_time.\n");
            }

            // Check if visibility is upcoming
            if (start_time > current_time + (time_t)margin_routing) {
                int sat_index = atoi(strrchr(trimmed_sat_id, '_') + 1);

                // Determine if the satellite is forward or backward based on central_index
                int direction;
                if (sat_index > central_index && sat_index <= central_index + 12) {
                    direction = 1;  // Forward
                } else if (sat_index < central_index || sat_index > central_index + 12) {
                    direction = 2;  // Backward
                } else {
                    continue;  // Skip the central satellite itself
                }

                // Calculate hops needed and alignment time
                int hops_needed = abs(sat_index - central_index); 
                if (hops_needed > 12) { hops_needed = 24 - hops_needed; };
                
                time_t alignment_time = hops_needed * (time_t)margin_routing;

                // printf("315: start_time: %ld, current_time + margin_routing: %ld\n", start_time, current_time + alignment_time);

                // Check alignment time feasibility
                if (start_time > current_time + alignment_time) {
                    if (duration >= fileTransferDur) { //TODO PROBABLY BETTER ADD A MARGIN TO CONSIDER THE LOST OF ALIGNMENT DURING LARGER FILE TRANSFERS
                        if (start_time < earliest_visibility) {
                            earliest_visibility = start_time;
                            recommend_direction = direction;
                            winning_index = sat_index;
                            *start_visibility = earliest_visibility;
                            printf("ENTERED sat index e hops: %i %i\n", sat_index, hops_needed);
                            printf("323: start_time: %ld, current_time + margin_routing: %ld\n", start_time, current_time + alignment_time);
                        }
                    }
                }
            }
        }
    }
    printf("DECIDED: Sat is %i and earliest visibility: %ld\n", winning_index, earliest_visibility);
    
    return recommend_direction;
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
    // transferTime += 100;
    printf("OISL FILE CFDP: Estimated Transfer time including network delay (modified): %f s.\n", transferTime);

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

        char line[256];
        time_t vis_start_time = -1;
        int is_visible_now = 0;

        // ADCS MODE will be changed
        Generic_ADCS_Mode_cmd_t cmd8;
        CFE_MSG_Init(CFE_MSG_PTR(cmd8.CmdHeader), CFE_SB_ValueToMsgId(GENERIC_ADCS_CMD_MID), sizeof(Generic_ADCS_Mode_cmd_t)); //TODO then move these lines above
        CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&cmd8, GENERIC_ADCS_SET_MODE_CC);

        time_t current_time = get_current_time();

        while (fgets(line, sizeof(line), vis_file) != NULL) {
            //printf("Line 291\n");
            char sat_id[20], vis_start[20], vis_end[20];
            double duration;

            // Ensure line ends at '\n' and doesn't include any hidden characters 
            line[strcspn(line, "\r\n")] = 0;
            int items = sscanf(line, "%20[^,],%20[^,],%20[^,], %lf", sat_id, vis_start, vis_end, &duration); 
            if (items == 4) { 
                if (strcmp(sat_id, Sat_Name) == 0 && is_visible(current_time, vis_start, vis_end, &vis_start_time, transferTime, duration)) {            // vis_start_time stores the earliest visibility of the central sat that is larger than current time
                    printf("Satellite %s is currently visible from OGS.\n", sat_id);
                    char formatted_current[20];
                    strftime(formatted_current, sizeof(formatted_current), "%Y-%m-%d %H:%M:%S", localtime(&(current_time)));
                    printf("CHECK IT: Current time: %s, Visibility start: %s Visibility end: %s\n", formatted_current, vis_start, vis_end);
                    is_visible_now = 1;
                    break;
                }
            }
        }

        if (!is_visible_now) {
            printf("Satellite Sat_1_1 is not visible from OGS at this time.\n");
            int routing = routing_Sat(vis_file, current_time, &vis_start_time, transferTime); // THIS IS THE VALUE PREVIOUSLY UPDATED BY IS VISIBLE!!!
            if (routing == 1) { // Another sat has an earlier visibility --> align to forward and start to route the info. 
                connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
                cmd8.Mode = OISL_MODE_F;
                printf("Routing forward\n");
            }
            else if (routing == 2) { // routing backwards
                connection_establishment = &OISL_AppData.DevicePkt.Oisl.BackwardConnection;
                cmd8.Mode = OISL_MODE_B;
                 printf("Routing backward\n");
            }
            else { // ROuting is not best option --> align with the OGS and wait for the window to start
                // Run a loop until conditions are met
                cmd8.Mode = OISL_MODE_OGS;
                strcpy(cmd8.OGS_Name, OGS_name);
                CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&cmd8);
                CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&cmd8, true);
                while (1) {
                    // Continuously update current time
                    current_time = get_current_time();

                    // Check conditions: OGSAlignment is 1 and current_time >= vis_start_time
                    if (OISL_AppData.DevicePkt.Oisl.OGSAlignment == 1 && current_time >= vis_start_time) {
                        // Conditions are met, establish connection and exit loop
                        uint8 conn_est = 1;
                        connection_establishment = &conn_est;
                        printf("Connection established to OGS.\n");
                        break;
                    }
                    
                    // Sleep briefly to avoid busy-waiting (adjust delay as needed)
                    // Calculate the remaining time until the visibility window starts
                    time_t time_to_vis_start = vis_start_time - current_time;
                    // Print status message with current time and remaining wait time
                    printf("Waiting to enter the visibility window. The visibility start is: %ld and Time until start: %ld seconds\n", (long)vis_start_time, (long)time_to_vis_start);
                    sleep(10); // Add a short delay (e.g., 1 second) to reduce CPU usage TODO regolate the time based on time to vis start
                }
            }
        }
        else { // VISIBLE NOW --> Only condition to check is the OGS alignment. TODO: Include consideration on transfer duration and for how long is the satellite visible 
        strcpy(cmd8.OGS_Name, OGS_name);
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.OGSAlignment; 
        cmd8.Mode = OISL_MODE_OGS;
        }

        // Transmit the MSG to modify ADCS mode
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&cmd8);
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&cmd8, true);

        // Close the visibility file
        fclose(vis_file);
        
    }
    else {
        printf("Unknown taget to align with, or method not yet impemented for target %u, default to forward", OISL_AppData.CFDP.Target);
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
    }

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