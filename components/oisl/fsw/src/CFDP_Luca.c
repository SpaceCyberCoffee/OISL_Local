/*******************************************************
** To adapt it to this satellite modify:
** my_src
** for_dest
** back_dest
** Sat_Name
** For large file transfer modify transfer_time_to_add and OGS_ASSUMED
 ********************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "CFDP_PDU.h"
#include "oisl_app.h"
#include "CFDP_Luca.h"
#include <time.h>
#include <math.h>

#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/platform_inc/generic_adcs_msgids.h"
#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/src/generic_adcs_msg.h"
#include "/home/jstar/Desktop/github-nos3/components/generic_adcs/fsw/src/generic_adcs_adac.h"
#include "cfe.h"

const int segmentSize = CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t) - CF_CFDP_MIN_HEADER_SIZE;

static const char* my_src = "/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/nos3_rbt/components/oisl/fsw/src/files_Test/plainText.txt";
static const char* for_dest = "/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/forward_sat/nos3_rbt/COSMOS_Control/Execution/OISL/files_received/plainText.txt";
static const char* back_dest = "/mnt/extras/SSD/NOS3_RBT/nos3_luca_OISL/Backward_Sat/COSMOS_Control/Execution/OISL/files_received/plainText.txt";

const size_t memoryCapacity = 8e9; // 8 GB for payload data
const int    marginDL = 10;        // this is the margin assuming alignment achieved, it is to quanitfy how much data could be transfered during a pass. TODO check if it contrasts with margin.

const double    DLCapacityperSecond = 12.5e6;   // 100Mbps = 12.5e6 Bytes per second
const int       time_to_make_it_realistic = 20;
const double    transfer_time_to_add = 240.0;   // 3GB

const char *OGS_ASSUMED = "Tiflis";

#define MAX_CANDIDATES 24  // Maximum number of satellites in constellation

// Global memory status for receiver
MemoryStatus receiverMemory = {
    .totalSize = 8e9,  // Bytes
    .currentUsed = 0,
    .isAvailable = true
};

MemoryStatus memoryStatusInstance = {
    .totalSize = 8e9,  // Bytes
    .currentUsed = 0,
    .isAvailable = true
};
MemoryStatus *memoryInfo = &memoryStatusInstance;

uint8_t transfering = 0;
uint8_t *transferActive = &transfering;

typedef struct {
    int *direction; // Direction of transfer: 1, 2 or both 
    double *segment_sizes; // Array to hold segment sizes in bytes
    char *sat_indexes;     // Index array.
    size_t num_segments; // how many satellites will be active in DL info
} SplittingInfo;


// New structure to track multiple satellite candidates
typedef struct {
    int sat_index;
    int direction;
    time_t visibility_start;
    double transfer_ratio;
    int hops;
    double dl_capacity;  // Downlink capacity in bytes
    double allocated_bytes; // bytes task to transfer
} SatCandidate;


const int networkDelay = 7000; // 7 ms ONE TRIP
const double transferSpeedMbps = 100.0; // Transfer speed in Mbps

static const char* fileSent_confirmation = "/home/jstar/Desktop/github-nos3/file_sent.txt";

static const char* Sat_Name = "Sat_1_1";
// static const char* Sat_For = "Sat_1_2";
// static const char* Sat_Back = "Sat_1_24";
static const double margin  = 60.0;    // THis depends: if the sat is already in OGS Mode the margin is 0. If it has to get into that mode then it might be even higher. TODO add autoregolation based on the mode you are in now.
static const double margin_routing = 120.0; //time to align with forward + time for the forward to align with OGS MODIFIED 17/1/2025 

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

    // Calculate the number of segments needed for this specific iteration size
    segmentCount = (fileSize + segmentSize - 1) / segmentSize;

    // Allocate memory for headers and contents dynamically
    *headers = (CF_CFDP_PduFileDataHeader_t *)malloc(segmentCount * sizeof(CF_CFDP_PduFileDataHeader_t));
    *contents = (CF_CFDP_PduFileDataContent_t *)malloc(segmentCount * sizeof(CF_CFDP_PduFileDataContent_t));

    if (*headers == NULL || *contents == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        return -1; // Indicate failure
    }

    for (i = 0; i < length; i += segmentSize) {
        // Fill the header with the correct offset
        (*headers)[i / segmentSize].offset.octets[0] = (i >> 24) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[1] = (i >> 16) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[2] = (i >> 8) & 0xFF;
        (*headers)[i / segmentSize].offset.octets[3] = i & 0xFF;

        // Copy the segment data into the content structure
        int currentSegmentSize = (i + segmentSize > length) ? (length - i) : segmentSize; // Handle last segment which might be smaller
        strncpy((char *)(*contents)[i / segmentSize].data, &fileContent[i], currentSegmentSize);
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

void createSentFile2(const char *fileContent, const int direction) { 
    // TODO source and dest must be defined by the file content.
    FILE *sentFile = fopen(fileSent_confirmation, "w");
    if (sentFile != NULL) {
        // Write src and dest as the first line based on direction
        if (direction == 1) { 
            fprintf(sentFile, "%s %s\n", my_src, for_dest); 
        }
        else if (direction == 2) { 
            fprintf(sentFile, "%s %s\n", my_src, back_dest); 
        }
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
            printf("DEBUG: Got the OGS NAME\n");
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
        //if (vis_duration - margin - marginDL >= fileTransferDur) {                                 // Updated: pass duration - alignment with OGS - loss of signal / tracking.
            *vis_start_time = start_time;
        //}
    }

    // Adjust the end time by subtracting margin and file transfer duration
    time_t adjusted_end_time = end_time - (time_t)(margin + fileTransferDur);

    // Check if the current time is within the adjusted visibility window
    return ((current_time >= start_time && current_time <= adjusted_end_time) && (vis_duration - margin - marginDL >= fileTransferDur));
}


// Adjust candidate capacities based on initial file division plan
// Preserves time-based ordering of candidates
int adjust_candidate_capacities_V2(SatCandidate *candidates, int candidate_count, 
                              int central_index, time_t current_time, 
                              double fileTransferDur) {
    printf("\n=== Starting Capacity Adjustment ===\n");
    
    // Step 0: Calculate total bytes to transfer
    double total_bytes = fileTransferDur * DLCapacityperSecond;
    printf("Total bytes to transfer: %.2f GB\n", total_bytes/1e9);
    
    // Step 1: Calculate total GB in each direction
    double total_forward = 0;
    double total_backward = 0;
    for (int i = 0; i < candidate_count; i++) {
        if (candidates[i].direction == 1) {
            total_forward += candidates[i].allocated_bytes;
        } else if (candidates[i].direction == 2) {
            total_backward += candidates[i].allocated_bytes;
        } 
    }
    double total_central = total_bytes - (total_backward + total_forward);
    printf("Initial forward allocation: %.2f GB\n", total_forward/1e9);
    printf("Initial central allocation: %.2f GB\n", total_central/1e9);
    printf("Initial backward allocation: %.2f GB\n", total_backward/1e9);

    int dir_principale = candidates[0].direction;
    
    // Running total of excess bytes that need reallocation
    double excess_bytes = 0;
    
    // Step 2-4: Process each satellite and adjust allocations
    for (int i = 0; i < candidate_count; i++) {
        SatCandidate *curr = &candidates[i];
        printf("\n--- Processing Satellite %d ---\n", curr->sat_index);
        
        if (curr->allocated_bytes <= 0 && excess_bytes <= 0) {
            printf("Satellite has no allocation and no excess bytes, skipping\n");
            continue;
        }
        
        // Calculate total routing time needed
        double time_available = difftime(curr->visibility_start, current_time);
        printf("Time available before visibility start: %.2f s\n", time_available);

        // Compute time needed to reach
        double transfer_bytes = 0;
        double base_routing_time = 0;
        // central case 120 + FORW POIRTION + 12O+ BACK PORTION + ASSSINGED PORTION
        if (curr->direction == 0) {
            printf("Case centrale\n");
            // both forward and backward directions
            if (total_backward >=0 && total_forward >=0)  {
                base_routing_time = 2 * margin_routing;
            }
            // only forward or only backward
            else {
                base_routing_time = margin_routing;
            }
            printf("Base routing %f\n", base_routing_time);
            // get other bytes  
            transfer_bytes = total_forward + total_backward;
            printf("Transfer bytes is %f GB\n", transfer_bytes/1e9);
        }
        // forward case FORWARD ASSIGNED + (FWD - HOP PRIMA) + (FSW - THIS HOP)
        else if (curr->direction == 1 && dir_principale == 1) {
            printf("Case foward main dir\n");
            base_routing_time = curr->hops * margin_routing;  // Alignment time per hop
            printf("Base routing %f\n", base_routing_time);
            transfer_bytes = total_forward; // at least total forward to move from central to forward
            double tot_forward = total_forward;
            int numb_hops_curr = curr->hops; // ex 1 --> stop, 2 take the portion assgined to this, 3 take this plus the forward - previous assigned
            for (int j = 1; j < numb_hops_curr; j++) { // stop at the current numb hops
                // pick the candidate in this direction with hops == j
                for (int i = 0; i < candidate_count; i++) {
                    if (candidates[i].direction == 1 && candidates[i].hops == j) {
                        printf("DEBVUG: found this one\n");
                        transfer_bytes += tot_forward - candidates[i].allocated_bytes;
                        tot_forward -= candidates[i].allocated_bytes;
                    }
                }
            }
            printf("Trasnfer bytes is %f\n", transfer_bytes/1e9);
        }
        // DIR SECONDARY? FORW ASSIGNED + 120 + BACK ASSIGNED + 120 + mia portion if only 1 back. if multiple is an issue
        else if (curr->direction == 2 && dir_principale == 1) {
            printf("Case backward with main dir forward\n");
            base_routing_time = curr->hops * margin_routing + margin_routing;  // Alignment time per hop and marign routing for the first forward transfer
            printf("Base routing %f\n", base_routing_time);
            transfer_bytes = total_backward + total_forward; // at least total backward to move from central to backward and totoal forward because first you route forward!
            double tot_backward = total_backward;
            int numb_hops_curr = curr->hops; // ex 1 --> stop, 2 take the portion assgined to this, 3 take this plus the backward - previous assigned
            for (int j = 1; j < numb_hops_curr; j++) { // stop at the current numb hops
                // pick the candidate in this direction with hops == j
                for (int i = 0; i < candidate_count; i++) {
                    if (candidates[i].direction == 2 && candidates[i].hops == j) {
                        printf("DEBVUG: found this one\n");
                        transfer_bytes += tot_backward - candidates[i].allocated_bytes;
                        tot_backward -= candidates[i].allocated_bytes;
                    }
                }
            }
            printf("Total bytes is %f\n", transfer_bytes/1e9);
        }
        else {
            printf("ERROR: NOT IMPLEMENTED YET BUT SHOULD NOT BE HERE\n");
        }
        double transfer_time = transfer_bytes / DLCapacityperSecond;   
        double time_needed_reach = base_routing_time + transfer_time; // TRANSFER TIME IS TIME TO GET TO TOUCH THIS SATELLITE. IT EXCLUDES THE TRANSFER OF ITS OWN MATERIAL. 

        printf("Time needed to reach this satellite: %.2f s\n", time_needed_reach);

        // REMEMEBER: DL CAPACITY HAS ALSO A MARGIN!
        
        // Check if we need to adjust allocation
    if (time_needed_reach > time_available) { // todo improve: reduce it by the minimum! YOU HAVE EXCESS, BUT EXCESS MUST BE DISTRIBUTED ACVROSS ALL TRANSFERS. SO IF YOU GET RID OF 0.2GB, YU REACH THIS SAT EARLIER
            // It takes longer to reach this satellite. From time available compute how much info can reach this sat before its visibility start.
            double time_loss = time_needed_reach - time_available; // ex: 20 seconds less of info
            // new excess
            double excess = time_loss * DLCapacityperSecond;
            // Not necessarily this is what you delete, because you save time. Maybe start with a portion, like excess / 10. Compute the change in time needed reach.
            // IT DEPENDS ON HOW MANY HOPS. WITH 1 HOP, THEN YOU NEED TO REDUCE IT BY THIS. IF TWO HOPS, LESS! MAYBE HALF?
            int number_hops = fmax(curr->hops, 1);        // how many time this info has to be transferred from central to this one? 
            excess = excess / number_hops;
            // DONE IT.
            excess = fmin(excess, curr->allocated_bytes); // avoid getting negative reduction
            // new bytes to be allocated
            double new_allocated = curr->allocated_bytes - excess;
            
            printf("Reducing allocation from %.2f GB to %.2f GB\n",    
                   curr->allocated_bytes/1e9, new_allocated/1e9);
            
            // Add difference to excess
            curr->allocated_bytes = new_allocated;
            excess_bytes += excess;
            // modify the distribution as well
            if (curr->direction == 1) {total_forward -= excess;}
            else if (curr->direction == 2) {total_backward -= excess;}
            else {total_central-=excess;}
            printf("Debug: now excess is: %f and the total forward is %f\n", excess_bytes, total_forward);
           
        } 
        else if (excess_bytes > 0) {
            // Calculate how many excess bytes this satellite can handle:
            double extra_time = time_available - time_needed_reach;
            double max_extra_bytes = (extra_time * DLCapacityperSecond);
            double bytes_to_add = fmin(excess_bytes, max_extra_bytes);
            // Cannot exceed the DL capacity
            if (bytes_to_add + curr->allocated_bytes > curr->dl_capacity) {
                bytes_to_add = curr->dl_capacity - curr->allocated_bytes;
                if (bytes_to_add < 0) {printf("WTF THIS IS WRONGGGG\n");}
            }
            
            printf("Adding %.2f GB from excess to current allocation of %.2f GB\n",
                   bytes_to_add/1e9, curr->allocated_bytes/1e9);
            
            curr->allocated_bytes += bytes_to_add;
            excess_bytes -= bytes_to_add;
            // modify distributions as well
            if (curr->direction == 1) {total_forward += bytes_to_add;}
            else if (curr->direction == 2) {total_backward += bytes_to_add;}
            else {total_central += bytes_to_add;}
        }
        
        // Update transfer ratio
        curr->transfer_ratio = (curr->allocated_bytes / DLCapacityperSecond) / fileTransferDur;
    }
    
    // Step 6: If there are still excess bytes, try to assign to satellites with zero allocation TODO TEST MIGHT BE WRONG
    if (excess_bytes > 0) {
        printf("\n=== Attempting to allocate remaining %.2f GB ===\n", excess_bytes/1e9);
        for (int i = 0; i < candidate_count && excess_bytes > 0; i++) {
            if (candidates[i].allocated_bytes <= 0) {
                double time_available = difftime(candidates[i].visibility_start, current_time);
                double base_routing_time = candidates[i].hops * margin_routing;
                double usable_time = time_available - base_routing_time;
                
                if (usable_time > 0) {
                    double max_possible = (usable_time * DLCapacityperSecond) / candidates[i].hops;
                    double bytes_to_add = fmin(excess_bytes, max_possible);
                    
                    candidates[i].allocated_bytes = bytes_to_add;
                    candidates[i].transfer_ratio = (bytes_to_add / DLCapacityperSecond) / fileTransferDur;
                    excess_bytes -= bytes_to_add;
                    
                    printf("Allocated %.2f GB to previously empty satellite %d\n",
                           bytes_to_add/1e9, candidates[i].sat_index);
                }
            }
        }
    }
    
    // Remove satellites with zero allocation and count valid ones
    int new_count = 0;
    for (int i = 0; i < candidate_count; i++) {
        if (candidates[i].allocated_bytes > 0) {
            if (i != new_count) {
                candidates[new_count] = candidates[i];
            }
            new_count++;
        }
    }
    
    printf("\n=== Final Allocation State ===\n");
    for (int i = 0; i < new_count; i++) {
        printf("Sat %d: %.2f GB\n", candidates[i].sat_index, 
               candidates[i].allocated_bytes/1e9);
    }
    
    if (excess_bytes > 0) {
        printf("Warning: %.2f GB could not be allocated\n", excess_bytes/1e9);
    }
    
    return new_count;
}

int routing_Sat_V2(FILE *vis_file, time_t current_time, time_t *start_visibility, double fileTransferDur, SplittingInfo *info) {  
    char line[300];
    char sat_id[20], vis_start[20], vis_end[20];
    double duration;
    int recommend_direction = 0;
    int central_index;

    SatCandidate candidates[MAX_CANDIDATES];
    int candidate_count = 0;

    // Extract central satellite index
    sscanf(strrchr(Sat_Name, '_') + 1, "%d", &central_index);
    rewind(vis_file);

    // First pass: Collect all viable candidates  
    while (fgets(line, sizeof(line), vis_file) != NULL) {
        if (sscanf(line, "%20[^,],%20[^,],%20[^,],%lf", sat_id, vis_start, vis_end, &duration) == 4) {  // TODO this would become  == 5 with DL CAPACITY already in the visibility
            char *trimmed_sat_id = strtok(sat_id, " \t\n\r");
            struct tm tm_start;
            time_t start_time;
            tm_start.tm_isdst = -1;
            vis_start[19] = '\0';

            // Calculate downlink capacity for this visibility window
            double dlCapacity = ((duration - margin - marginDL) * DLCapacityperSecond > 0) ?
                              (duration - margin - marginDL) * DLCapacityperSecond : 0;  // dlCapacity in bytes

            if (dlCapacity > 0) {
                strptime(vis_start, "%Y-%m-%dT%H:%M:%S", &tm_start);
                start_time = mktime(&tm_start);

                if (start_time == -1) {
                    printf("ROUTING: mktime failed to convert start_time.\n");
                    continue;
                }

                // if (start_time > current_time + (time_t)margin_routing) {
                    int sat_index = atoi(strrchr(trimmed_sat_id, '_') + 1);

                    // Determine direction
                    int direction;
                    if (sat_index > central_index && sat_index <= central_index + 12) {
                        direction = 1;  // Forward
                    } else if (sat_index < central_index || sat_index > central_index + 12) {
                        direction = 2;  // Backward
                    } else if (sat_index == central_index) {
                        direction = 0;  // Central satellite
                    }

                    // Calculate hops
                    int hops_needed = abs(sat_index - central_index);
                    if (hops_needed > 12) {
                        hops_needed = 24 - hops_needed;
                    }

                    time_t alignment_time = hops_needed * (time_t)margin_routing;

                    // If satellite is reachable in time, add to candidates. SO FAR THE TRANSFER TIME IS NOT TAKEN INTO ACCOUNT
                    if (start_time > current_time + alignment_time) {
                        double transfer_ratio = (duration - marginDL - margin) / fileTransferDur;

                        // Check if the sat_index is already in the candidates list
                        int already_exists = 0;
                        for (int i = 0; i < candidate_count; i++) {
                            if (candidates[i].sat_index == sat_index) {
                                already_exists = 1;  // If the sat_index is already in the list, skip adding it
                                break;
                            }
                        }

                        // Add to candidates array
                        if (!already_exists && candidate_count < MAX_CANDIDATES) {
                            candidates[candidate_count].sat_index = sat_index;
                            candidates[candidate_count].direction = direction;
                            candidates[candidate_count].visibility_start = start_time;
                            candidates[candidate_count].transfer_ratio = transfer_ratio;
                            candidates[candidate_count].hops = hops_needed;
                            candidates[candidate_count].dl_capacity = dlCapacity;
                            candidates[candidate_count].allocated_bytes = 0;
                            candidate_count++;
                        }
                    }
                // }
            }
        }
    }

    // If we have candidates, process them
    if (candidate_count > 0) {
        // Sort candidates by visibility start time
        for (int i = 0; i < candidate_count - 1; i++) {
            for (int j = 0; j < candidate_count - i - 1; j++) {
                if (candidates[j].visibility_start > candidates[j + 1].visibility_start) {
                    SatCandidate temp = candidates[j];
                    candidates[j] = candidates[j + 1];
                    candidates[j + 1] = temp;
                }
            }
        }

        // First check if the sat with earliest visibility can handle the entire file transfer -> situation of small file. Otherwise, file splitting. In this way we avoid doing a dangerous ping pong during large file transfer.
        if (candidates[0].transfer_ratio >= 1.0) {
                // Found a satellite that can handle the entire file
                printf("DEBUG: Found this satellite for the routing to DL data: %d", candidates[0].sat_index);
                *start_visibility = candidates[0].visibility_start;
                return candidates[0].direction;
        }
        

        // If we get here, we need to split the file
        double total_transfer_ratio = 0.0;
        printf("\nFile too large to handle: File splitting analysis:\n");
        printf("Available reachable satellites for partial transfers:\n");

        // Calculate total available capacity
        double total_capacity = 0.0;
        for (int i = 0; i < candidate_count; i++) {
            total_capacity += candidates[i].dl_capacity;
            total_transfer_ratio += candidates[i].transfer_ratio;

            printf("Sat %d: Direction=%d, Start=%ld, Capacity=%.2f GB, TR=%.2f, Hops=%d Allocated bytes=%f\n",
                   candidates[i].sat_index,
                   candidates[i].direction,
                   candidates[i].visibility_start,
                   candidates[i].dl_capacity / 1e9,  // DL CAPACITY IN GB
                   candidates[i].transfer_ratio,
                   candidates[i].hops,
                   candidates[i].allocated_bytes);
        }

        // If combined capacity is sufficient
        if (total_transfer_ratio >= 1.0) {
            // Initialize SplittingInfo structure
            info->direction = malloc(candidate_count * sizeof(int));
            info->segment_sizes = malloc(candidate_count * sizeof(double));
            info->sat_indexes = malloc(candidate_count * sizeof(char)); // Assuming char for simplicity, you can change to int if needed
            info->num_segments = 0;
            printf("\nFile can be split across multiple satellites.\n");
            printf("Total available capacity: %.2f GB\n", total_capacity / 1e9);
            printf("Recommended sequence:\n");

            // Select the first satellite in the sequence as our routing target --> earlier visibility
            *start_visibility = candidates[0].visibility_start;
            recommend_direction = 3;

            // Print the recommended splitting sequence
            double remaining_file = fileTransferDur;
            for (int i = 0; i < candidate_count && remaining_file > 0; i++) {
                double this_transfer = (candidates[i].dl_capacity < remaining_file *DLCapacityperSecond) ? candidates[i].dl_capacity : remaining_file *DLCapacityperSecond;
                candidates[i].allocated_bytes = this_transfer;
                printf("Sat %d: %.2f GB (%.1f%%)\n",
                       candidates[i].sat_index,
                       this_transfer / 1e9,
                       (this_transfer / (fileTransferDur *DLCapacityperSecond)) * 100);
                remaining_file -= this_transfer /DLCapacityperSecond;
            }

            // Before adding the candidate to the Trasnfer program, see if the portion to trasnfer can be transferred in time. REMEMEBER THAT ALSO THE TIMES TO TRANSFER THE FILE SEGMENTS MUST ACCUMULATE: SAT 1_3 HAS TWO HOPS AND TWO TRANSFER TIMES.
            //code.

            // Adjust capacities considering routing constraints
            int updated_count = adjust_candidate_capacities_V2(candidates, candidate_count, 
                                                  central_index, current_time,
                                                  fileTransferDur);
    
            // Update candidate_count with the new count
            candidate_count = updated_count;

            // TODO: RECALCULATE? YES.
            // Recalculate total capacity and transfer ratio
            total_transfer_ratio = 0.0;
            for (int i = 0; i < candidate_count; i++) {
                printf("Debug sat %d this transfer ratio is: %f",candidates[i].sat_index, candidates[i].transfer_ratio);
                total_transfer_ratio += candidates[i].transfer_ratio;
                
            }
            printf("DEBUG Total transfer ratio: %f\n", total_transfer_ratio);
            // Then proceed with your splitting info if still viable
            if (total_transfer_ratio >= 0.99) {
    
                // here the sats are ordered by start time and have updated portions assigned stored in allocateddBYtes
                double remaining_file = fileTransferDur;
                for (int i = 0; i < candidate_count && remaining_file > 0; i++) {
                    double this_transfer = candidates[i].allocated_bytes;
                    printf("Sat %d: %.2f GB (%.1f%%)\n",
                        candidates[i].sat_index,
                        this_transfer / 1e9,
                        (this_transfer / (fileTransferDur *DLCapacityperSecond)) * 100);
                    remaining_file -= this_transfer / DLCapacityperSecond;
                // Fill the SplittingInfo structure with the current satellite data
                    info->sat_indexes[info->num_segments] = candidates[i].sat_index;
                    info->segment_sizes[info->num_segments] = this_transfer; // bytes
                    // Determine direction (forward or backward)
                    info->direction[info->num_segments] = candidates[i].direction;

                    // Increment the number of segments
                    info->num_segments++;
                }
                // Shrink the arrays (optional):
                info->sat_indexes = realloc(info->sat_indexes, info->num_segments * sizeof(char));
                info->segment_sizes = realloc(info->segment_sizes, info->num_segments * sizeof(double));
                info->direction = realloc(info->direction, info->num_segments * sizeof(int));

                
                // DEBUG Optionally print the final results:
                for (size_t i = 0; i < info->num_segments; i++) {
                    printf("Sat %d: Direction=%d, Segment Size=%.2f GB\n",
                    info->sat_indexes[i],
                    info->direction[i],
                    info->segment_sizes[i] / 1e9);  // Convert segment size to GB
                }
            }
            else { printf("Total transfer ratio not enough.\n");}
        
        }
        else {
            printf("\nWarning: Even with splitting, total capacity (%.2f) is insufficient for complete transfer\n",
                   total_transfer_ratio);
        }
    }

    return recommend_direction;
}


// Function to check if receiver has enough memory
bool waitForReceiverMemory(size_t requiredSize, const char* filename) {
    const int MAX_WAIT_TIME = 300;  // Maximum wait time in seconds
    int waitTime = 0;

    while (waitTime < MAX_WAIT_TIME) {
        // In a real implementation, this would be updated via inter-satellite communication
        receiveMemoryInfo(filename);
        if (receiverMemory.currentUsed <= receiverMemory.totalSize && receiverMemory.totalSize - receiverMemory.currentUsed >= requiredSize) {
            return true;
        }
        sleep(10);
        waitTime += 10;
        printf("Waiting for receiver memory to become available: %d/%d seconds\n",
               waitTime, MAX_WAIT_TIME);
    }
    return false;
}

void receiveMemoryInfo(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("ERROR: Filename for memory %s not valid, return\n", filename);
        return;  // Return without value since function is void
    }

    size_t mem_value;
    // int first_value;
    // Read both values properly
    if (fscanf(file, "%*d %zu", &mem_value) != 1) {
        fclose(file);
        printf("ERROR: Failed to read values or negative memory value: %zu\n", mem_value);
        return;
    }

    receiverMemory.currentUsed = mem_value;
    fclose(file);
}

// Function to calculate segment size for each iteration
size_t calculateIterationSize(const size_t fileSize, const int iteration, const int numberOfIterations) {
    if (iteration == numberOfIterations - 1) {
        return fileSize - iteration * memoryCapacity;  // For the last chunk, take the remaining file size
    } else {
        return memoryCapacity;  // For all other chunks, take the full capacity size TODO MAYBE NOT WISE
    }
}

// Structure to hold file portion information
typedef struct {
    char* data;
    size_t size;
    size_t offset;
} FilePortion;

// Helper function to get file portion based on offset and size
FilePortion get_file_portion(const char* fileContent, size_t totalSize, size_t offset, size_t size, double sizePercentage, double offsetPercentage) {  // size percentage is wrong.
    FilePortion portion = {NULL, 0, offset};
    printf("DEBUG: I am about to extract a file portion of %zu from the file with total size: %zu\n", size, totalSize);
    if (offset + size <= totalSize) {
        portion.data = malloc(size);
        if (portion.data) {
            memcpy(portion.data, fileContent + offset, size);
            portion.size = size;
        }
    }
    else {
        // If the requested portion is too large, calculate percentage-based size and offset
        size_t adjustedSize = (size_t)(sizePercentage * totalSize / 100.0);
        size_t adjustedOffset = (size_t)(offsetPercentage * totalSize / 100.0);

        printf("WARNING: Requested size exceeds file size. Extracting %.2f%% of the file (%zu bytes) starting at %.2f%% (%zu bytes).\n",
               sizePercentage, adjustedSize, offsetPercentage, adjustedOffset);

        // Ensure adjustedOffset + adjustedSize does not exceed totalSize
        if (adjustedOffset + adjustedSize > totalSize) {
            adjustedSize = totalSize - adjustedOffset; // Adjust size to fit within bounds
            printf("INFO: Adjusted size to fit within file bounds: %zu bytes.\n", adjustedSize);
        }

        if (adjustedSize > 0) {
            portion.data = malloc(adjustedSize);
            if (portion.data) {
                memcpy(portion.data, fileContent + adjustedOffset, adjustedSize);
                portion.size = adjustedSize;
                portion.offset = adjustedOffset; // Update the offset in the FilePortion struct
            }
        } else {
            printf("ERROR: Invalid percentage-based size calculation.\n");
        }
    }
    
    return portion;
}

// Modified process_direction function
void process_direction(int direction, double size, const uint8 mode, uint8_t *connection, 
                      const char *filename_mem, const char* fileContent, size_t totalSize, size_t offset, SplittingInfo* info, double fake_file_size, time_t vis_start_time) { // size is now in BYTES.
    if (size <= 0) return;
    
    printf("\n=== Processing %d Distribution ===\n", mode);
    printf("Going into mode %d for %.2f GB transfer\n", mode, size/1e9);

    // DEBUG Print satellite sequence
    for (size_t i = 0; i < info->num_segments; i++) {
        if (info->direction[i] == direction) {
            printf("- Sat_%d: %.2f B\n", info->sat_indexes[i], info->segment_sizes[i]);  // segment_sizes is in byte!
        }
    }

    size_t sizeInBytes = (size_t)(size); // Convert double to size_t
    // COMPUTE THE SIZE PERCENTAGE AND ALSO THE OFFSET PERCENTAGE TO PASS TO GET FILE PORTION
    double sizePercentage = (size / fake_file_size) * 100; 
    double offsetPercentage = ((double)offset / fake_file_size)* 100; 
    printf("Debug: size and offset percentages: %f and %f", sizePercentage, offsetPercentage);
    FilePortion portion = get_file_portion(fileContent, totalSize, offset, sizeInBytes, sizePercentage, offsetPercentage);
    
    if (portion.data != NULL) {
        printf("%d portion extracted.\n", mode);

        // Transmit the portion

        // ADCS MODE will be changed
        Generic_ADCS_Mode_cmd_t cmd8;
        CFE_MSG_Init(CFE_MSG_PTR(cmd8.CmdHeader), CFE_SB_ValueToMsgId(GENERIC_ADCS_CMD_MID), sizeof(Generic_ADCS_Mode_cmd_t)); //TODO then move these lines above
        CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&cmd8, GENERIC_ADCS_SET_MODE_CC);
        cmd8.Mode = mode;
        if (mode == 5) {strcpy(cmd8.OGS_Name, OGS_ASSUMED);}  
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&cmd8);  // ERROR: Mode is not transmitted correctly/ 
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&cmd8, true);

        CF_CFDP_PduFileDataHeader_t *headers = NULL;
        CF_CFDP_PduFileDataContent_t *contents = NULL;
        int segmentCount = segmentFileIntoPDUs(portion.data, portion.size, &headers, &contents, segmentSize);

        // In case of central case (aka OGS DL) check visibility to OGS // MISSING: VIS START TIME OF CENTRAL, now it is of the first visible sat!!!!
        if (direction == 0) {
            while (1) {
                // Continuously update current time
                time_t current_time = get_current_time();

                // Check conditions: OGSAlignment is 1 and current_time >= vis_start_time
                if (OISL_AppData.DevicePkt.Oisl.OGSAlignment == 1 && current_time >= vis_start_time) {
                    // Conditions are met, establish connection and exit loop
                    uint8 conn_est = 1;
                    connection = &conn_est;
                    break;
                }
                // Calculate the remaining time until the visibility window starts
                time_t time_to_vis_start = vis_start_time - current_time;
                // Print status message with current time and remaining wait time
                printf("Waiting to enter the visibility window. The visibility start is: %ld and Time until start: %ld seconds\n", (long)vis_start_time, (long)time_to_vis_start);
                sleep((int)time_to_vis_start);
            }
        }

        // Proceed to send the iteration
        // Calculate fake duration of this segment to simulate longer transfers
        double fake_duration = size / DLCapacityperSecond;   // Ex: 1.6GB / 12.5 MB/s
        int confirmation = sendIteration(portion.data, portion.size, headers, contents, 
                                      segmentCount, segmentSize, connection, filename_mem, fake_duration);
        
        if (confirmation != 0) {
            printf("ERROR: Something went wrong during %d transfer\n", mode);
        }
        else {
            printf("Iteration sent. Creating the SentFile\n");
            createSentFile2(portion.data, direction);
        }
        
        free(portion.data);
    }
    else {
        printf("ERROR: Portion of the file not extracted\n");
    }

    printf("Finished transmitting %.2f GB to the %d direction.\n", size/1e9, mode);
}

void handle_splitting(SplittingInfo* info, const char *fileContent, const size_t fileSize, time_t start_visibility) {  // start vis useful for the DL to the OGS of this sat
    if (info == NULL || info->num_segments == 0) {
        printf("Error: Invalid splitting info\n");
        return;
    }
    // Calculate sizes and offsets for each direction
    double forward_size = 0.0;
    double backward_size = 0.0;
    double central_size = 0.0;
    size_t forward_offset = 0;
    size_t backward_offset = 0;
    size_t central_offset = 0;

    // Calculate total size for each direction and determine offsets
    for (size_t i = 0; i < info->num_segments; i++) {
        double size_bytes = info->segment_sizes[i];  // segment sizes are already in bytes

        switch (info->direction[i]) {
            case 1: // Forward
                forward_size += size_bytes;
                break;
            case 2: // Backward
                backward_size += size_bytes;
                break;
            case 0: // Central
                central_size += size_bytes;
                break;
            default:
                printf("Error default case why?\n");
                break;
        }
    }

    printf("\nFile Distribution Analysis:\n");
    printf("Forward: %.2f GB \n", forward_size/1e9);  
    printf("Backward: %.2f GB \n", backward_size/1e9);
    printf("Central: %.2f GB \n", central_size/1e9);

    double fake_file_size = forward_size + central_size + backward_size;    // this is like 3GB

    // Determine processing order based on priority
    int priority = info->direction[0];
    
    if (priority == 1) { // Forward first
        // Offset calculation
        backward_offset = forward_size + central_size;
        central_offset = forward_size;
        process_direction(1, forward_size, OISL_MODE_F, &OISL_AppData.DevicePkt.Oisl.ForwardConnection, 
                        "F_sat_back_alignment.txt", fileContent, fileSize, forward_offset, info, fake_file_size, start_visibility);
        process_direction(2, backward_size, OISL_MODE_B, &OISL_AppData.DevicePkt.Oisl.BackwardConnection, 
                        "B_sat_for_alignment.txt", fileContent, fileSize, backward_offset, info, fake_file_size, start_visibility);
        process_direction(0, central_size, OISL_MODE_OGS, &OISL_AppData.DevicePkt.Oisl.OGSAlignment, 
                        "OGS.txt", fileContent, fileSize, central_offset, info, fake_file_size, start_visibility);
    } else if (priority == 2) { // Backward first
        // Offset calculation
        forward_offset = forward_size + central_size;
        central_offset = backward_size;
        process_direction(2, backward_size, OISL_MODE_B, &OISL_AppData.DevicePkt.Oisl.BackwardConnection, 
                        "B_sat_for_alignment.txt", fileContent, fileSize, backward_offset, info, fake_file_size, start_visibility);
        process_direction(1, forward_size, OISL_MODE_F, &OISL_AppData.DevicePkt.Oisl.ForwardConnection, 
                        "F_sat_back_alignment.txt", fileContent, fileSize, forward_offset, info, fake_file_size, start_visibility);
        process_direction(0, central_size, OISL_MODE_OGS, &OISL_AppData.DevicePkt.Oisl.OGSAlignment, 
                        "OGS.txt", fileContent, fileSize, central_offset, info, fake_file_size, start_visibility);
    } else if (priority == 0) { // Central first
        // Offset calculation
        backward_offset = central_size;
        process_direction(2, backward_size, OISL_MODE_B, &OISL_AppData.DevicePkt.Oisl.BackwardConnection, 
                        "B_sat_for_alignment.txt", fileContent, fileSize, backward_offset, info, fake_file_size, start_visibility);
        process_direction(0, central_size, OISL_MODE_OGS, &OISL_AppData.DevicePkt.Oisl.OGSAlignment, 
                        "OGS.txt", fileContent, fileSize, central_offset, info, fake_file_size, start_visibility);
    }

}

// Helper function to free the splitting info structure
void free_splitting_info(SplittingInfo* info) {
    if (info != NULL) {
        if (info->direction != NULL) free(info->direction);
        if (info->segment_sizes != NULL) free(info->segment_sizes);
        if (info->sat_indexes != NULL) free(info->sat_indexes);
        free(info);
    }
}

int sendIteration(const char *fileContent, size_t fileSize, CF_CFDP_PduFileDataHeader_t *headers, CF_CFDP_PduFileDataContent_t *contents, int segmentCount, int segmentSize, uint8_t *connection_establishment, const char *filename_memory, double fake_duration) {
    // Check memory availability for OISL
    if (strcmp(filename_memory, "OGS") != 0) {
        if (*connection_establishment == 1) {
            receiveMemoryInfo(filename_memory);
            if (receiverMemory.currentUsed > receiverMemory.totalSize || receiverMemory.totalSize - receiverMemory.currentUsed < fileSize) {
                receiverMemory.isAvailable = false;
            }
        }

        if (receiverMemory.isAvailable == false) {
            printf("DEBUG: memory of the receiver not available. %zu. Will wait for the memory to free again. \n", receiverMemory.totalSize - receiverMemory.currentUsed);
            if (!waitForReceiverMemory(fileSize, filename_memory)) {
                printf("Timeout waiting for receiver memory, aborting transfer\n");
                return -1; // Indicate failure
            }
        }
    }

    // Calculate realistic sleep time
    double sleep_time = fake_duration / segmentCount;
    printf("DEbug: I will sleep %f after every PDUs sent to get to a fake duration of %f\n", sleep_time, fake_duration);
    time_t start_segment_time;
    time_t end_segment_time;

    for (int segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {
        int sent = 0;
        int retries = 0;
        const int maxRetries = 5;

        while (!sent && retries < maxRetries) {
            if (*connection_establishment == 1) {
                if (sent == 0) {
                    start_segment_time = get_current_time();
                    end_segment_time = start_segment_time + sleep_time;
                }
                *transferActive = 1;
                sent = sendPDU(&headers[segmentNumber], &contents[segmentNumber], segmentNumber, fileContent, segmentSize);
                if (sent == 1) {
                    simulateNetworkDelay();

                    // Simulate the other satellite receiving the PDU
                    if (receivePDU(&headers[segmentNumber], &contents[segmentNumber], segmentNumber)) {
                        CF_CFDP_PduAck_t ack = createAck(CF_CFDP_FileDirective_ACK, CF_CFDP_ConditionCode_NO_ERROR, segmentNumber);
                        sendAck(&ack);

                        // Simulate the sender receiving the ACK
                        if (!receiveAck(segmentNumber, &ack)) {
                            sent = 0;
                            retries++;
                            printf("PDU #%d not acknowledged, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                        } else {
                            // Update memory usage
                            memoryInfo->currentUsed = (memoryInfo->currentUsed < (size_t)segmentSize) ? 0 : memoryInfo->currentUsed - segmentSize;
                            memoryInfo->isAvailable = (memoryInfo->currentUsed < memoryCapacity) ? 1 : 0;
                            // DEBUG TO SEE MEM CHANGING + TEST STABILITY 
                            printf("DEbug: sleep now\n");
                            while (start_segment_time < end_segment_time) {
                                sleep(2);
                                // update start segment time
                                start_segment_time = get_current_time();
                            }                    
                            printf("DEbug: awake now\n");
                            // update end segment time
                            end_segment_time += sleep_time;
                        }
                    }
                } else {
                    retries++;
                    printf("PDU #%d failed to send, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                }
            } else {
                printf("Wait for re-alignment\n");
                if (*transferActive == 1) {*transferActive = 0;}
                sleep(10); // Adjust sleep as needed
            }
        }

        if (retries == maxRetries) {
            printf("PDU #%d failed after %d retries, aborting transmission.\n", segmentNumber, maxRetries);
            if (*transferActive == 1) {*transferActive = 0;}
            return -1; // Indicate failure
        }
    }

    printf("All PDUs sent successfully.\n");
    if (*transferActive == 1) {*transferActive = 0;}

    // Free dynamically allocated memory
    free(headers);
    free(contents);

    return 0; // Indicate success
}


void sendFile(const char *fileContent, const size_t fileSize) {
    // CHECK MEMORY
    printf("CFDP: Received a file with size: %zu bytes\n", fileSize);
    int segmentNumber = 0;
    /* If each PDU can carry 504 bytes of data and you can send a maximum of 1000 PDUs, the maximum file size in bytes is: Max File Size (bytes)=504�1000=504000 bytes = 504 kB*/
    CF_CFDP_PduFileDataHeader_t *headers = NULL;
    CF_CFDP_PduFileDataContent_t *contents = NULL;
    int segmentCount = segmentFileIntoPDUs(fileContent, fileSize, &headers, &contents, segmentSize); // TODO: probably not here, or mayube segment it again afterr routing...
    double transferTime = estimateTransferTime(fileSize, segmentCount);
    transferTime += transfer_time_to_add; // AKA 3GB of file to DOWNLINK
    printf("OISL FILE CFDP: Estimated (BIGGER) Transfer time including network delay: %f s.\n", transferTime);

    // Update memory information
    memoryInfo->currentUsed = fileSize; // Bytes
    memoryInfo->isAvailable = (memoryInfo->currentUsed < memoryCapacity) ? 1 : 0;

    uint8 *connection_establishment;
    const char* filename_memory;
    if (OISL_AppData.CFDP.Target == 0) {
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.BackwardConnection;
        filename_memory = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/B_sat_for_alignment.txt";
    }
    else if (OISL_AppData.CFDP.Target == 1)
    {
        connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
        filename_memory = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/F_sat_back_alignment.txt";
    }
    else if (OISL_AppData.CFDP.Target == 2)
    {
        char OGS_name[64]; // Buffer to hold the extracted ground station name
        // Extract the ground station name from the file content
        extractOGSName(fileContent, OGS_name, sizeof(OGS_name)); // this receives every time the fileContent entire, so OGS_Name is found always.
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
                    printf("DEBUG: CHECK IT: Current time: %s, Visibility start: %s Visibility end: %s\n", formatted_current, vis_start, vis_end);
                    is_visible_now = 1;
                    break;
                }
            }
        }

        if (!is_visible_now) { 
            time_t my_vis_start = vis_start_time;           // Meaning, either it is not veasible, or the pass duration is too short.
            printf("Satellite Sat is not visible from OGS at this time.\n");
            // Create structure for file segmentation if needed
            // SplittingInfo splitting_info = NULL;
            // First, allocate memory for the SplittingInfo structure itself
            SplittingInfo* info = malloc(sizeof(SplittingInfo));
            if (info == NULL) {
                // Handle allocation failure
                printf("ERROR: ALLOCATION ERROR FOR SPLIOTTING INGO");
                return;
            }
            // Initialize all pointers to NULL
            info->direction = NULL;
            info->segment_sizes = NULL;
            info->sat_indexes = NULL;
            info->num_segments = 0;
            int routing = routing_Sat_V2(vis_file, current_time, &vis_start_time, transferTime, info); // THIS IS THE VALUE PREVIOUSLY UPDATED BY IS VISIBLE!!!
            if (routing == 1) { // Another sat has an earlier visibility --> align to forward and start to route the info.
                connection_establishment = &OISL_AppData.DevicePkt.Oisl.ForwardConnection;
                filename_memory = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/F_sat_back_alignment.txt";
                cmd8.Mode = OISL_MODE_F;
                printf("Routing forward\n");
            }
            else if (routing == 2) { // routing backwards
                connection_establishment = &OISL_AppData.DevicePkt.Oisl.BackwardConnection;
                filename_memory = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/B_sat_for_alignment.txt";
                cmd8.Mode = OISL_MODE_B;
                printf("Routing backward\n");
            }
            else if (routing == 3 && info != NULL) { // file too large so I need also the division into parts and file segmentation.
                handle_splitting(info, fileContent, fileSize, my_vis_start);
                free_splitting_info(info);
                printf("Concluded my work here.\n");
                return;
            }
            else { // ROuting is not best option AND THE FILE SIZE IS PROPER TO BE DL BY THIS SAT ALONE --> align with the OGS and wait for the window to start
                // Run a loop until conditions are met
                cmd8.Mode = OISL_MODE_OGS;
                filename_memory = "OGS";
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

                    // Calculate the remaining time until the visibility window starts
                    time_t time_to_vis_start = vis_start_time - current_time;
                    // Print status message with current time and remaining wait time
                    printf("Waiting to enter the visibility window. The visibility start is: %ld and Time until start: %ld seconds\n", (long)vis_start_time, (long)time_to_vis_start);
                    sleep((int)time_to_vis_start);
                }
            }
        }
        else { // VISIBLE NOW and DL SIZE OKAY--> Only condition to check is the OGS alignment. TODO: Include consideration on transfer duration and for how long is the satellite visible
            strcpy(cmd8.OGS_Name, OGS_name);
            connection_establishment = &OISL_AppData.DevicePkt.Oisl.OGSAlignment;
            filename_memory = "OGS";
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

    // For OISL check first memory of the receiver. For OGS not needed.
    if (strcmp(filename_memory, "OGS") != 0) {
        if (*connection_establishment == 1) {
            receiveMemoryInfo(filename_memory);
            if (receiverMemory.currentUsed > receiverMemory.totalSize || receiverMemory.totalSize - receiverMemory.currentUsed < fileSize) {
                receiverMemory.isAvailable = false;
            }
        }

        if (receiverMemory.isAvailable == false) {
            printf("DEBUG: memory of the receiver not available. %zu. Will wait for the memory to free again. \n", receiverMemory.totalSize - receiverMemory.currentUsed);
            if (!waitForReceiverMemory(fileSize, filename_memory)) {
                printf("Timeout waiting for receiver memory, aborting transfer\n");
                return;
            }
        }
    }

    for (segmentNumber = 0; segmentNumber < segmentCount; segmentNumber++) {   // probably make this a method, so that it can be put here for the majority of thecases, but also called by handle_splitting. 
        int sent = 0;
        int retries = 0;
        const int maxRetries = 5;

        while (!sent && retries < maxRetries) {
            if (*connection_establishment == 1) {
                *transferActive = 1;
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
                        // PDU sent and correctly received --> update memory of this sat
                        sleep(time_to_make_it_realistic); // then delete NOW TO SIMULATE LARGER FILE TRANSFER AND TEST STABILITY
                        memoryInfo->currentUsed = (memoryInfo->currentUsed < (size_t)segmentSize) ? 0 : memoryInfo->currentUsed - segmentSize;
                        memoryInfo->isAvailable = (memoryInfo->currentUsed < memoryCapacity) ? 1 : 0;
                    }
                } else {
                    retries++;
                    printf("PDU #%d failed to send, retrying (%d/%d)\n", segmentNumber, retries, maxRetries);
                }
            } else {
                printf("Wait for re-alignment\n");
                if (*transferActive == 1) {*transferActive = 0;}
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

    if (*transferActive == 1) {*transferActive = 0;}

    // Free dynamically allocated memory
    free(headers);
    free(contents);
    
}