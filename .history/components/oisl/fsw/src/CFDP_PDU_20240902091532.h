#ifndef MY_PDU_H
#define MY_PDU_H

#include <stdint.h>

/**
 * @brief Encoded 8-bit value in the PDU
 */
typedef struct
{
    uint8_t octets[1];
} MyPDU_uint8_t;

/**
 * @brief Encoded 16-bit value in the PDU
 */
typedef struct
{
    uint8_t octets[2];
} MyPDU_uint16_t;

/**
 * @brief Encoded 32-bit value in the PDU
 */
typedef struct
{
    uint8_t octets[4];
} MyPDU_uint32_t;

/**
 * @brief Encoded 64-bit value in the PDU
 */
typedef struct
{
    uint8_t octets[8];
} MyPDU_uint64_t;

/**
 * @brief Structure representing base PDU header
 *
 * This header appears at the beginning of all PDUs, of all types.
 */
typedef struct MyPDU_Header
{
    MyPDU_uint8_t  flags;           /**< \brief Flags indicating the PDU type, direction, mode, etc */
    MyPDU_uint16_t length;          /**< \brief Length of the entire PDU, in octets */
    MyPDU_uint8_t  eid_tsn_lengths; /**< \brief Lengths of the Entity ID and Transaction Sequence Number data */
    // Variable-length fields for EID and TSN are not directly included here
} MyPDU_Header_t;

/**
 * @brief Structure representing a File Directive PDU
 */
typedef struct MyPDU_FileDirective
{
    MyPDU_uint8_t directive_code;  /**< \brief The directive code (e.g., EOF, NAK, etc.) */
    // Additional directive-specific fields go here
} MyPDU_FileDirective_t;

/**
 * @brief Structure representing Metadata PDU
 */
typedef struct MyPDU_Metadata
{
    MyPDU_uint8_t  segmentation_control; /**< \brief Segmentation control byte */
    MyPDU_uint32_t size;                 /**< \brief Total size of the file being transferred */
    // Followed by optional variable-length LV (length-value) pairs for filenames, etc.
} MyPDU_Metadata_t;

/**
 * @brief Structure representing a PDU for file data
 */
typedef struct MyPDU_FileData
{
    MyPDU_uint32_t offset;               /**< \brief Offset within the file */
    uint8_t data[];                      /**< \brief File data (variable length) */
} MyPDU_FileData_t;

/**
 * @brief Union representing any PDU type
 */
typedef union MyPDU
{
    MyPDU_Header_t        header;
    MyPDU_FileDirective_t file_directive;
    MyPDU_Metadata_t      metadata;
    MyPDU_FileData_t      file_data;
} MyPDU_t;

#endif /* MY_PDU_H */
