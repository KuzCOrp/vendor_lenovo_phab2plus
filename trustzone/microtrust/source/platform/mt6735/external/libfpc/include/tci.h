/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * @file   tci.h
 * @brief  Contains TCI (Trustlet Control
 * Interface) definitions and data structures
 *
 */

#ifndef TCI_H_
#define TCI_H_

#include "fpc_def.h"

/**
 * Trustlet UUID.
 */
#define TIC_TA_FPC_UUID { { 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


/**< Responses have bit 31 set */
#define TCI_RSP_ID_MASK (1U << 31)
#define TCI_RSP_ID(cmdId) (((uint32_t)(cmdId)) | TCI_RSP_ID_MASK)
#define TCI_IS_CMD(cmdId) ((((uint32_t)(cmdId)) & TCI_RSP_ID_MASK) == 0)
#define TCI_IS_RSP(cmdId) ((((uint32_t)(cmdId)) & TCI_RSP_ID_MASK) == TCI_RSP_ID_MASK)

/**
 * Return codes.
 */
#define TCI_RET_OK               0     /**< Set, if processing is error free */
#define TCI_RET_ERR_UNKNOWN_CMD  1     /**< Unknown command */
#define TCI_RET_CUSTOM_START     2
#define TCI_RET_ERR_MAP          3
#define TCI_RET_ERR_UNMAP        4
#define TCI_RET_ERR_RETURN_CODE  5
#define TCI_RET_ERR_NOTIFICATION 6
#define TCI_RET_ERR_TCI          7

/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */

/* Send a fpc command with no buffer */
#define TCI_CMD_SEND_CMD                0x1
/* Send a fpc command requiring an allocated buffer. */
#define TCI_CMD_TRANSFER_DATA           0x2 


typedef enum {
	 /* Start the initialize the TA */
	TCI_FPC_CMD_INIT = 0x00,
	TCI_FPC_CMD_DEINIT,
	TCI_FPC_CMD_DEBUG_INJECT_IMAGE,
	TCI_FPC_CMD_DEBUG_RETRIEVE_IMAGE,
	TCI_FPC_CMD_BEGIN_ENROLL,
	TCI_FPC_CMD_ENROLL,
	TCI_FPC_CMD_END_ENROLL,
	TCI_FPC_CMD_EXTENDED_ENROLL_DATA,
	TCI_FPC_CMD_BEGIN_IDENTIFY,
	TCI_FPC_CMD_IDENTIFY,
	TCI_FPC_CMD_END_IDENTIFY,
	TCI_FPC_CMD_CHECK_FINGER_PRESENT,
	TCI_FPC_CMD_CHECK_FINGER_LOST,
	TCI_FPC_CMD_WAKEUP_QUALIFICATION,
	TCI_FPC_CMD_WAKEUP_SETUP,
	TCI_FPC_CMD_GET_IDS,
	TCI_FPC_CMD_GET_IDS_COUNT,
	TCI_FPC_CMD_DELETE_TEMPLATE,
	TCI_FPC_CMD_LOAD_ONE_TEMPLATE,
	TCI_FPC_CMD_STORE_ONE_TEMPLATE,
	TCI_FPC_CMD_GET_TEMPLATE_SIZE,
	TCI_FPC_CMD_INIT_FINGER_DETECT,
	TCI_FPC_CMD_START_FINGER_DETECT,
	TCI_FPC_CMD_CAPTURE_IMAGE,
	TCI_FPC_CMD_DEADPIXEL_TEST,
	TCI_FPC_CMD_DELETE_ALL_TEMPLATES,
	TCI_FPC_CMD_DEEP_SLEEP,
} tci_fpc_cmd_t;

#define fpc_debug(...) \
    tlApiLogPrintf("[TA FPC] " __VA_ARGS__ ); \
    tlApiLogPrintf("\n");

/**
 * Termination codes
 */
#define TCI_EXIT_ERROR                      ((uint32_t)(-1))

/**
 * Maximum data length.
 */
#define TCI_MAX_DATA_LEN 1024

typedef uint32_t tci_command_id_t;
typedef uint32_t tci_response_id_t;
typedef uint32_t tci_return_code_t;
typedef uint32_t fpc_template_id_t;
/**
 * Command structure Trusted Connector -> Trusted Application message.
 */

/**
 * TCI command header.
 */
typedef struct{
    tci_command_id_t id; /**< Command ID */
    tci_fpc_cmd_t fpc_cmd;
    uint32_t client_id; 
} tci_command_header_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */

/**
 * TCI response header.
 */
typedef struct{
     /**< Response ID (must be command ID | RSP_ID_MASK )*/
    tci_response_id_t     id;
     /**< Return code of command */
    tci_return_code_t     code;
    fpc_def_return_t    cmd_ret;
} tci_response_header_t;

/**
 * TCI message data.
 */
/*typedef struct {
	union {
	    tci_command_header_t  cmd;				*< Command header
	    tci_response_header_t rsp;				*< Response header
    };
    uint32_t data;
    mcBulkMap_t data_struct;
    uint32_t data_size;
} tci_message_t;*/

#endif // TCI_H_
