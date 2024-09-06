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

/**
 *  @brief Max PDU size.
 *
 *  @par Description:
 *       The max PDU size across all channels in the system. Keep in mind that
 *       the max filedata pdu will be smaller than this. This size includes
 *       the PDU headers and everything. While this is the max value for all
 *       channels, the outgoing_file_chunk_size in the configuration table
 *       is different for each channel so a smaller size can be used.
 *
 *  @par Limits:
 *
 */
/* CF_MAX_PDU_SIZE must be the max possible PDU for any channel. Channels can be configured with a smaller max. */
#define CF_MAX_PDU_SIZE (512)

/**
 * @brief Minimum encoded size of a CFDP PDU header
 *
 * Per the blue book, the size of the Entity ID and Sequence Number must be at least 1 byte.
 */
#define CF_CFDP_MIN_HEADER_SIZE \
    (sizeof(CF_CFDP_PduHeader_t) + (3 * sizeof(CF_CFDP_uint8_t))) /* 1 byte for each variable item */

/**
 * @brief PDU file data content
 *
 * To serve as a sanity check, this should accommodate the largest data block possible.
 * In that light, it should be sized based on the minimum encoded header size, rather than
 * the maximum, as that case leaves the most space for data.
 */
typedef struct CF_CFDP_PduFileDataContent
{
    uint8 data[CF_MAX_PDU_SIZE - sizeof(CF_CFDP_PduFileDataHeader_t) - CF_CFDP_MIN_HEADER_SIZE];
} CF_CFDP_PduFileDataContent_t;

/**
 * @brief PDU file data header
 */
typedef struct CF_CFDP_PduFileDataHeader
{
    /**
     * NOTE: while this is the only fixed/required field in the data PDU, it may
     * have segment metadata prior to this, depending on how the fields in the
     * base header are set
     */
    CF_CFDP_uint32_t offset;
} CF_CFDP_PduFileDataHeader_t;

/**
 * @brief Structure representing CFDP Acknowledge PDU
 *
 * Defined per section 5.2.4 / table 5-8 of CCSDS 727.0-B-5
 */
typedef struct CF_CFDP_PduAck
{
    CF_CFDP_uint8_t directive_and_subtype_code;
    CF_CFDP_uint8_t cc_and_transaction_status;
} CF_CFDP_PduAck_t;

/**
 * @brief Structure representing CFDP Segment Request
 *
 * Defined per section 5.2.6 / table 5-11 of CCSDS 727.0-B-5
 */
typedef struct CF_CFDP_SegmentRequest
{
    CF_CFDP_uint32_t offset_start;
    CF_CFDP_uint32_t offset_end;
} CF_CFDP_SegmentRequest_t;

/**
 * @brief Structure representing CFDP Non-Acknowledge PDU
 *
 * Defined per section 5.2.6 / table 5-10 of CCSDS 727.0-B-5
 */
typedef struct CF_CFDP_PduNak
{
    CF_CFDP_uint32_t scope_start;
    CF_CFDP_uint32_t scope_end;
} CF_CFDP_PduNak_t;


#endif /* MY_PDU_H */
