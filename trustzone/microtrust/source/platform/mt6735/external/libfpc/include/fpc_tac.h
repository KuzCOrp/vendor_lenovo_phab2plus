/*
 * teei_fp_lib.h
 *
 *  Created on: Apr 15, 2015
 *      Author: napoleonliu
 */

#ifndef TEEI_FP_LIB_H_
#define TEEI_FP_LIB_H_
#include "tci.h"
#include "fpc_lib.h"
#include <android/log.h>
#include <unistd.h>
struct TEEI_SharedMemory
{
/*! The pointer to the block of shared memory. */
    void*   buffer;

/*! The length of the shared memory block in bytes. Should not be zero */
    unsigned int size;
};
typedef struct TEEI_SharedMemory TEEI_SharedMemory;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

#define LOG_E(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "fpc_test_lib",fmt, ##__VA_ARGS__); \
						__android_log_print(ANDROID_LOG_ERROR, "fpc_test_lib","\n")

#define LOG_I(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "fpc_test_lib", fmt, ##__VA_ARGS__); \
						__android_log_print(ANDROID_LOG_INFO, "fpc_test_lib","\n")

fpc_def_return_t fpc_tac_open(void);

fpc_def_return_t fpc_tac_init(void);
fpc_def_return_t fpc_tac_deinit(void);
fpc_def_return_t fpc_tac_begin_identify(uint32_t* ids, uint32_t length);
fpc_def_return_t fpc_tac_end_identify();

/*
 * Identify a fingerprint with one of the template id on the list of id
 * associated with the client id.
 *
 * @param client_id - the id of the caller
 * @param id - an array of candidate ids
 * @param length - the length of the candidate list
 *
 * @return
 */
fpc_def_return_t fpc_tac_identify(fpc_lib_identify_data_t* data);

/*
 * Begin the enrol process.
 *
 * @param tid - the id of the template to enroll
 */
fpc_def_return_t fpc_tac_begin_enroll();

/*
 * Starts the enrol process.
 *
 */
fpc_def_return_t fpc_tac_enroll(fpc_lib_enroll_data_t* data);

fpc_def_return_t fpc_tac_get_extended_enroll(fpc_lib_extended_enroll_t* data);

/*
 * End the enrol process.
 *
 */
fpc_def_return_t fpc_tac_end_enroll(uint32_t * id);

/*
 * Gets the enrolled ids from the driver.
 *
 * @param ids - an array to fill with ids
 * @param len - length of the array
 */
fpc_def_return_t fpc_tac_get_ids(uint32_t* ids, uint32_t* len);

fpc_def_return_t fpc_tac_get_ids_count(uint32_t* len);
fpc_def_return_t fpc_tac_deadpixel_test(int* nDeadPixels);

fpc_def_return_t fpc_tac_debug_inject_image(uint8_t* image, uint32_t size);
fpc_def_return_t fpc_tac_debug_retrieve_image(uint8_t* image, uint32_t* size);
fpc_def_return_t fpc_tac_init_finger_detect(uint32_t* data);
fpc_def_return_t fpc_tac_start_finger_detect(uint32_t* data);
fpc_def_return_t fpc_tac_check_finger_present(uint32_t* data);
fpc_def_return_t fpc_tac_check_finger_lost(uint32_t* data);
fpc_def_return_t fpc_tac_capture_image();
fpc_def_return_t fpc_tac_remove_template(uint32_t id);
fpc_def_return_t fpc_tac_remove_all_templates();
fpc_def_return_t fpc_tac_spidrv_open(void);
fpc_def_return_t fpc_tac_wakeup_setup();
fpc_def_return_t fpc_tac_wakeup_qualification();
fpc_def_return_t fpc_tac_deep_sleep(void);
void fpc_tac_spidrv_close(void);

void fpc_tac_close(void);

#endif /* TEEI_FP_LIB_H_ */
