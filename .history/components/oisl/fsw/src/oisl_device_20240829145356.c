/*******************************************************************************
** File: oisl_device.c
**
** Purpose:
**   This file contains the source code for the OISL device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "oisl_device.h"
#include <stdio.h>
#include <math.h>


extern struct __cmdline cmdline;

/* 
** Generic read data from device
*/
int32_t OISL_ReadData(uart_info_t* device, uint8_t* read_data, uint8_t data_length)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    int32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(device);
    while((bytes_available < data_length) && (ms_timeout_counter < OISL_CFG_MS_TIMEOUT))
    {   
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(device);
    }

    if (ms_timeout_counter < OISL_CFG_MS_TIMEOUT)
    {   
        /* Limit bytes available */
        if (bytes_available > data_length)
        {
            bytes_available = data_length;
        }
        
        /* Read data */
        bytes = uart_read_port(device, read_data, bytes_available);
        if (bytes != bytes_available)
        {   
            OS_printf("  OISL_ReadData: Bytes read != to requested! \n");
            #ifdef OISL_CFG_DEBUG
                OS_printf("  OISL_ReadData: Bytes read != to requested! \n");
            #endif
            status = OS_ERROR;
        } /* uart_read */
    }
    else
    {   OS_printf("Errore qui in OISL_ReadData.");
        status = OS_ERROR;
    } /* ms_timeout_counter */

    return status;
}


/* 
** Generic command to device
** Note that confirming the echoed response is specific to this implementation
*/
int32_t OISL_CommandDevice(uart_info_t* device, uint8_t cmd_code, uint32_t payload)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    uint8_t write_data[OISL_DEVICE_CMD_SIZE];
    uint8_t read_data[OISL_DEVICE_DATA_SIZE];

    /* Prepare command */
    write_data[0] = OISL_DEVICE_HDR_0;
    write_data[1] = OISL_DEVICE_HDR_1;
    write_data[2] = cmd_code;
    write_data[3] = payload >> 24;
    write_data[4] = payload >> 16;
    write_data[5] = payload >> 8;
    write_data[6] = payload;
    write_data[7] = OISL_DEVICE_TRAILER_0;
    write_data[8] = OISL_DEVICE_TRAILER_1;

    /* Flush any prior data */
    status = uart_flush(device);
    if (status == UART_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(device, write_data, OISL_DEVICE_CMD_SIZE);
        #ifdef OISL_CFG_DEBUG
            OS_printf("  OISL_CommandDevice[%d] = ", bytes);
            for (uint32_t i = 0; i < OISL_DEVICE_CMD_SIZE; i++)
            {
                OS_printf("%02x", write_data[i]);
            }
            OS_printf("\n");
        #endif
        if (bytes == OISL_DEVICE_CMD_SIZE)
        {
            status = OISL_ReadData(device, read_data, OISL_DEVICE_CMD_SIZE);
            if (status == OS_SUCCESS)
            {
                /* Confirm echoed response */
                bytes = 0;
                while ((bytes < (int32_t) OISL_DEVICE_CMD_SIZE) && (status == OS_SUCCESS))
                {
                    if (read_data[bytes] != write_data[bytes])
                    {
                        status = OS_ERROR;
                    }
                    bytes++;
                }
            } /* OISL_ReadData */
            else
            {
                #ifdef OISL_CFG_DEBUG
                    OS_printf("OISL_CommandDevice - OISL_ReadData returned %d \n", status);
                #endif
            }
        } 
        else
        {
            #ifdef OISL_CFG_DEBUG
                OS_printf("OISL_CommandDevice - uart_write_port returned %d, expected %d \n", bytes, OISL_DEVICE_CMD_SIZE);
            #endif
        } /* uart_write */
    } /* uart_flush*/
    return status;
}


/*
** Request housekeeping command
*/
int32_t OISL_RequestHK(uart_info_t* device, OISL_Device_HK_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[OISL_DEVICE_HK_SIZE];

    /* Command device to send HK */
    status = OISL_CommandDevice(device, OISL_DEVICE_REQ_HK_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = OISL_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            #ifdef OISL_CFG_DEBUG
                OS_printf("  OISL_RequestHK = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            /* Verify data header and trailer */
            if ((read_data[0]  == OISL_DEVICE_HDR_0)     && 
                (read_data[1]  == OISL_DEVICE_HDR_1)     && 
                (read_data[14] == OISL_DEVICE_TRAILER_0) && 
                (read_data[15] == OISL_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];

                data->DeviceConfig  = read_data[6] << 24;
                data->DeviceConfig |= read_data[7] << 16;
                data->DeviceConfig |= read_data[8] << 8;
                data->DeviceConfig |= read_data[9];

                data->DeviceStatus  = read_data[10] << 24;
                data->DeviceStatus |= read_data[11] << 16;
                data->DeviceStatus |= read_data[12] << 8;
                data->DeviceStatus |= read_data[13];

                #ifdef OISL_CFG_DEBUG
                    OS_printf("  Header  = 0x%02x%02x  \n", read_data[0], read_data[1]);
                    OS_printf("  Counter = 0x%08x      \n", data->DeviceCounter);
                    OS_printf("  Config  = 0x%08x      \n", data->DeviceConfig);
                    OS_printf("  Status  = 0x%08x      \n", data->DeviceStatus);
                    OS_printf("  Trailer = 0x%02x%02x  \n", read_data[14], read_data[15]);
                #endif
            }
            else
            {
                #ifdef OISL_CFG_DEBUG
                    OS_printf("  OISL_RequestHK: OISL_ReadData reported error %d \n", status);
                #endif 
                status = OS_ERROR;
            }
        } /* OISL_ReadData */
    }
    else
    {
        #ifdef OISL_CFG_DEBUG
            OS_printf("  OISL_RequestHK: OISL_CommandDevice reported error %d \n", status);
        #endif 
    }
    return status;
}


/*
** Request data command
*/
int32_t OISL_RequestData(uart_info_t* device, OISL_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    // uint8_t read_data[OISL_DEVICE_DATA_SIZE]; //62
    uint8_t read_data[10]; // the 6 doubles (48 bytes) are not considered.


    /* Command device to send HK */
    status = OISL_CommandDevice(device, OISL_DEVICE_REQ_DATA_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = OISL_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {   

            #ifdef OISL_CFG_DEBUG
                OS_printf("  OISL_RequestData = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            /* Verify data header and trailer */
            if ((read_data[0]  == OISL_DEVICE_HDR_0)     && 
                (read_data[1]  == OISL_DEVICE_HDR_1)     && 
                (read_data[8] == OISL_DEVICE_TRAILER_0) && 
                (read_data[9] == OISL_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];
            }
        } 
        else
        {   
            OS_printf("  OISL_RequestData: Invalid data read! \n");
            #ifdef OISL_CFG_DEBUG
                OS_printf("  OISL_RequestData: Invalid data read! \n");
            #endif 
            status = OS_ERROR;
        } /* OISL_ReadData */
    }
    else
    {   OS_printf("  OISL_RequestData: OISL_CommandDevice reported error %d \n", status);
        #ifdef OISL_CFG_DEBUG
            OS_printf("  OISL_RequestData: OISL_CommandDevice reported error %d \n", status);
        #endif 
    }

    // Call the new submethod to get the ISL vectors
    double backward_isl_vector[3];
    double forward_isl_vector[3];
    get_isl_vectors(forward_isl_vector, backward_isl_vector);

    // OS_printf("Backward ISL Vector: %f, %f, %f\n", backward_isl_vector[0], backward_isl_vector[1], backward_isl_vector[2]);
    // OS_printf("Forward ISL Vector: %f, %f, %f\n", forward_isl_vector[0], forward_isl_vector[1], forward_isl_vector[2]);

    data->BACKWARD_ISL_X = backward_isl_vector[0];
    data->BACKWARD_ISL_Y = backward_isl_vector[1];
    data->BACKWARD_ISL_Z = backward_isl_vector[2];
    data->FORWARD_ISL_X = forward_isl_vector[0];
    data->FORWARD_ISL_Y = forward_isl_vector[1];
    data->FORWARD_ISL_Z = forward_isl_vector[2];

    /* Read Forward Alignment */
    data->ForwardAlignment = read_data[6];

    /* Read Backward Alignment */
    data->BackwardAlignment = read_data[7];

    // OS_printf("FOrward alignment: %u und backward alignment: %u\n", data->ForwardAlignment, data->BackwardAlignment);

    return status;
}

// Submethod to get the forward and backward OISL vectors
void get_isl_vectors(double* forward_ISL_vector, double* backward_ISL_vector)
{
    // Call the Python script and get the ECI position vector of the forward and backward satellites
    FILE* fp;
    char buffer[128];
    char command[256];
    const char* tle_file_path_forward = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/forward_sat.txt";
    const char* tle_file_path_backward = "/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/backward_sat.txt";
    const char* tle_files [] = {tle_file_path_backward, tle_file_path_forward};
    // Variable to store the position vectors
    double eci_position[2][3]; // 2 satellites, each with a 3D position vector, back and for


    // Retrieve current SIM time
    CFE_TIME_SysTime_t nowT = CFE_TIME_GetTime();
    uint32 seconds = nowT.Seconds;
    uint32 subseconds = nowT.Subseconds;

    // Convert the time to double
    double pd = (double)seconds + ((double)subseconds / 4294967296.0); // 4294967296.0 = 2^32
    // OS_printf("TOH TIME PD %f\n", pd);

    if (pd <)

    for (int i=0; i<2; i++) {
        // Construct the command with arguments
        snprintf(command, sizeof(command),
             "python3 /home/jstar/Desktop/github-nos3/components/oisl/fsw/src/orbital_propagation.py %s %f",
             tle_files[i], pd);

        // Run the Python script and capture its output
        fp = popen(command, "r");
        if (fp == NULL) {
            fprintf(stderr, "Failed to run Python script\n");
        }
    
        // Read the output and parse the position vector
        while (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
            if (sscanf(buffer, "ECI propagated Position: [%lf, %lf, %lf]", &eci_position[i][0], &eci_position[i][1], &eci_position[i][2]) == 3) { 
                // Successfully parsed the position vector
                // OS_printf("Got the propagation");
            } else {
                // Print other output lines if any
                printf("%s", buffer);
            }
        }
        pclose(fp);
    }
    double backward_satellite [] = {eci_position[0][0], eci_position[0][1], eci_position[0][2]};
    double forward_satellite [] = {eci_position[1][0], eci_position[1][1], eci_position[1][2]};

    // Retrieve the ECI GPS position of this satellite
    FILE *file_in = fopen("/home/jstar/Desktop/github-nos3/components/oisl/fsw/src/ECI_position.txt", "r");
    double x, y, z;
    fscanf(file_in, "%lf %lf %lf", &x, &y, &z);
    fclose(file_in);
    double central_sat [] = {x/1000, y/1000, z/1000};
    // OS_printf("Central Sat ECI Position: [%f, %f, %f]\n", central_sat[0], central_sat[1], central_sat[2]);
    // OS_printf("FOrward Sat Position: [%f, %f, %f]\n", forward_satellite[0], forward_satellite[1], forward_satellite[2]);
    // OS_printf("Backward Sat Position: [%f, %f, %f]\n", backward_satellite[0], backward_satellite[1], backward_satellite[2]);

    // Calculate the ISL vector with the forward satellite
    double ISL_vector_forward[] = {forward_satellite[0] - central_sat[0], forward_satellite[1] - central_sat[1], forward_satellite[2] - central_sat[2]};
    double ISL_vector_backward[] = {backward_satellite[0] - central_sat[0] , backward_satellite[1] - central_sat[1], backward_satellite[2] - central_sat[2]};
    // OS_printf("OISL vector is [%f, %f, %f]\n", OISL_vector[0], OISL_vector[1], OISL_vector[2]);

    // OS_printf("DIstanza tra satelliti in KM: %f", sqrt(ISL_vector_forward[0]*ISL_vector_forward[0]+ISL_vector_forward[1]*ISL_vector_forward[1]+ISL_vector_forward[2]*ISL_vector_forward[2])); // 1758km

    // Normalize the difference vector to get the unit vector
    UNITV2(ISL_vector_backward);
    UNITV2(ISL_vector_forward);

    // Print the normalized vector
    // OS_printf("Normalized ISL vector forward: [%f, %f, %f]\n", ISL_vector_forward[0], ISL_vector_forward[1], ISL_vector_forward[2]);
    // OS_printf("Normalized ISL vector backward: [%f, %f, %f]\n", ISL_vector_backward[0], ISL_vector_backward[1], ISL_vector_backward[2]);

    // Assign the normalized vectors to the output parameters
    backward_ISL_vector[0] = ISL_vector_backward[0];
    backward_ISL_vector[1] = ISL_vector_backward[1];
    backward_ISL_vector[2] = ISL_vector_backward[2];

    forward_ISL_vector[0] = ISL_vector_forward[0];
    forward_ISL_vector[1] = ISL_vector_forward[1];
    forward_ISL_vector[2] = ISL_vector_forward[2];


}


/*  Normalize a 3-vector if it is non-zero.                           */
void UNITV2(double V[3])
{
      double A;

      A=sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2]);
      if (A > 0.0) {
         V[0]/=A;
         V[1]/=A;
         V[2]/=A;
      }
}
