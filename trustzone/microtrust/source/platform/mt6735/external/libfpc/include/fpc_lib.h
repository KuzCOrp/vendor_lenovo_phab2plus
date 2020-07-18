#ifndef _FPC_LIB_H_
#define _FPC_LIB_H_

/* Version 0.5 */
/* Used by all fpc_lib functions*/
typedef enum {
    FPC_LIB_OK,
    FPC_LIB_WAIT_EVENT_FINGER_PRESENT,
    FPC_LIB_CAPTURE_DONE,
    FPC_LIB_ENABLE_EVENT_FINGER_PRESENT,
    FPC_LIB_WAIT_TIME,
    FPC_LIB_FINGER_PRESENT,
    FPC_LIB_FINGER_LOST,
    FPC_LIB_ERROR_TOO_FAST,
    FPC_LIB_ERROR_TOO_SLOW,
    FPC_LIB_ERROR_GENERAL,
    FPC_LIB_ERROR_SENSOR,
    FPC_LIB_ERROR_MEMORY,
    FPC_LIB_ERROR_PARAMETER,
} fpc_lib_return_t;

typedef enum {
    FPC_LIB_IDENTIFY_NO_MATCH,
    FPC_LIB_IDENTIFY_MATCH,
    FPC_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE,
} fpc_lib_identify_result_t;

typedef enum {
    FPC_LIB_ENROLL_SUCCESS,
    FPC_LIB_ENROLL_TOO_MANY_ATTEMPTS,
    FPC_LIB_ENROLL_TOO_MANY_FAILED_ATTEMPTS,
    FPC_LIB_ENROLL_FAIL_LOW_QUALITY,
    FPC_LIB_ENROLL_FAIL_LOW_COVERAGE,
    FPC_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE,
} fpc_lib_enroll_result_t;

typedef enum {
    FPC_LIB_SENSOR_OK,
    FPC_LIB_SENSOR_WORKING,
    FPC_LIB_SENSOR_INITIALISING,
    FPC_LIB_SENSOR_OUT_OF_ORDER,
    FPC_LIB_SENSOR_MALFUNCTIONED,
    FPC_LIB_SENSOR_FAILURE,
} fpc_lib_sensor_status_t;

/* 
 * Structure for templates stored in the database
*/
typedef struct {
    /* Pointer to a buffer for template data */
    uint8_t* tpl;
    /* Size of the buffer for template data */
    uint32_t size;
} fpc_lib_template_t;

/* 
 * Struct with data from an enroll attempt
 */
typedef struct {
   /* Progress of the current enroll process in percent */
    uint32_t progress;
    /* Quality for the image*/
    uint32_t quality;
    /* Status of current enroll attempt */
    fpc_lib_enroll_result_t result;
    /* Number of successful enroll attempts so far */
    uint32_t nr_successful;
    /* Number of failed enroll attempts so far */
    uint32_t nr_failed;
    /* Size of the enrolled template */
    uint32_t enrolled_template_size;
    /* Size of the data part in the structure used for extended enroll */
    uint32_t extended_enroll_size;
    /* Coverage of the image*/
    uint32_t coverage;
    /* Used to indicate that touches are too similar */
    int8_t user_touches_too_immobile;
} fpc_lib_enroll_data_t;

/* Structure used for extended enroll */
typedef struct {
    /* Data buffer for extended enroll */
    uint8_t* data;
    /* Size of the extended enroll data buffer */
    uint32_t size;
} fpc_lib_extended_enroll_t;

/* Data from the identification attempt */
typedef struct {
    /* Result of the identification attempt */
    fpc_lib_identify_result_t result;
    /* Matching score */
    uint32_t score;
    /* Index of the identification template */
    uint32_t index;
    /* Size of the update template if one exits */
    uint32_t updated_template_size;
} fpc_lib_identify_data_t;

/* Security levels for the matching function */
typedef enum {
    /* FAR 1/1000 */
    FPC_LIB_SECURITY_LOW,
    /* FAR 1/10,000 */
    FPC_LIB_SECURITY_REGULAR,
    /* FAR 1/50,000 */
    FPC_LIB_SECURITY_HIGH,
    /* FAR 1/100,000 */
    FPC_LIB_SECURITY_VERY_HIGH,
} fpc_lib_security_t;

/*
 * Initialize the library, load the database and check the sensor.
 *
 * @return FPC_OK
 *
 */
fpc_lib_return_t fpc_lib_init(uint8_t* data, uint32_t* size);

uint32_t fpc_lib_init_data_size(void);
/*
 * De-initialize the library.
 *
 * @return FPC_OK
 *
 */
fpc_lib_return_t fpc_lib_deinit(void);

/* 
 * Captures an image from the sensor, and stores it for use with enroll and
 * identify.
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_SENSOR - if the image capture failed because of the
 *         communication with the sensor
 *         FPC_LIB_ERROR_MEMORY - if memory allocation failed during the capture
 *         FPC_LIB_ERROR_GENERAL - if other error occurred
*/
fpc_lib_return_t fpc_lib_capture_image(void);

/*
 * Begin the enrollment of a new finger.
 *
 * @return FPC_OK
 *
 */
fpc_lib_return_t fpc_lib_begin_enroll(void);

/*
 * Use the currently stored image for enrollment.
 *
 * @param[out] data, the data from the enrollment.
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if data is NULL
 *         FPC_LIB_ERROR_MEMORY - if memory allocation failed during the enrollment
 */
fpc_lib_return_t fpc_lib_enroll(fpc_lib_enroll_data_t* data);

/*
 * Get information for the extended enroll processes. The size of the data
 * structure were given in the previous fpc_lib_enroll(void) call.
 *
 * @param[out] data, the data used for extend enroll.
 * @param[in/out] size, in:size is the size of the allocated data structure,
 *  out:size is the size of the used data structure. If out:size is larger than
 *  in:size call has to be redone with the larger value.
 *
 *  @return FPC_OK
 *          FPC_LIB_ERROR_PARAMETER - if the struct pointer is null
 *          FPC_LIB_ERROR_MEM - if the size is too small
*/
fpc_lib_return_t fpc_lib_get_extended_enroll_data(fpc_lib_extended_enroll_t* data,
        uint32_t* size);

/*
 * Ends the enrollment and returns the enrolled finger template.
 *
 * @param[out] tpl, a buffer with the template data for the enrolled finger.
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if template is NULL or the size of the 
 *         buffer is too small
 *         FPC_LIB_ERROR_MEMORY - if the template extraction failed
 */
fpc_lib_return_t fpc_lib_end_enroll(fpc_lib_template_t* tpl);

/* 
 * Begin identify for the given template candidates.
 * 
 * @param[in] candidates, the templates of the candidates to verify against
 * @param[in] size, size of the candidate array.
 * @param[in] security, the security level from fpc_lib_security_t
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if candidates is a NULL pointer, size is 
 *         zero or security is an invalid number
 *         FPC_LIB_ERROR_MEMORY - if memory allocation fails during
 *         the identification process
*/
fpc_lib_return_t fpc_lib_begin_identify(fpc_lib_template_t* candidates,
        uint32_t size, fpc_lib_security_t security);

/* 
 * Use the currently stored image for identification.
 * 
 * @param data data from the identification process.
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if data is a null pointer
 *         FPC_LIB_ERROR_MEMORY - if the memory allocation failed during the
 *         identification
*/
fpc_lib_return_t fpc_lib_identify(fpc_lib_identify_data_t* data);

/* 
 * End the started identification. If there was a template update during the
 * identification process, the given template struct will be filled with
 * template data. If a null pointer is given the update will be discarded.
 *
 * @param[in/out] updated_template, a buffer for the updated template 
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if template is NULL or the size of the buffer
 *         is too small, (the needed size is returned in the template data)
 */
fpc_lib_return_t fpc_lib_end_identify(fpc_lib_template_t* updated_template);

/* 
 * Check if a finger is present on the sensor
 * 
 *    while (ret != FPC_FINGER_PRESENT || abort != TRUE) {
 *       ret = fpc_finger_present(void)
 *       switch (ret) {
 *           case FPC_ENABLE_EVENT_FINGER_PRESENT:
 *               driver_init_finger_detect_event(void)
 *               break;
 *           case FPC_WAIT_FOR_EVENT:
 *               driver_wait_for_event(void);
 *               break;
 *           case FPC_ENABLE_EVENT_TIME:
 *               sleep(FINGER_PRESENT_WAIT_TIME);
 *               break;
 *           case FPC_FINGER_LOST:
 *               break;
 *           default:
 *               ret = fpc_end_identify(void);
 *               return -1;
 *       } 
 * @param[out] wait_time, the time in ms to wait.
 *
 * @return FPC_LIB_ENABLE_EVENT_FINGER_PRESENT - start the wait for finger
 *         present 
 *         FPC_LIB_WAITING_FOR_EVENT - if a finger present event is needed
 *         before checking
 *         FPC_LIB_ENABLE_EVENT_TIME - all qualification for finger present
 *         not fullfilled, wait and try again,
 *         FPC_LIB_FINGER_PRESENT - finger is present on the sensor 
 *         FPC_LIB_FINGER_LOST - finger is removed from the sensor
*/
fpc_lib_return_t fpc_lib_finger_present(uint32_t* wait_time);

/* 
 * Check if the finger is removed from the sensor
 *
 * @return FPC_LIB_FINGER_LOST - if finger is not on the sensor
 *         FPC_LIB_ENABLE_EVENT_TIME - finger is on sensor, sleep and try later
 */
fpc_lib_return_t fpc_lib_check_finger_lost(uint32_t* wait_time);

/* 
 * Injects an image to use for enroll or identify.
 * ONLY for debugging purpose
 *
 * @param img, a pointer to the image data.
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if the image has the wrong format or other error
*/
fpc_lib_return_t fpc_lib_debug_inject_image(uint8_t* img, uint32_t size);

/* 
 * Retrieves the image to use for enroll or identify.
 * ONLY for debugging purpose.
 *
 * @param[out] img an pointer to a mem area with the size of an image. 
 *
 * @return FPC_OK
 *         FPC_LIB_ERROR_PARAMETER - if image is a NULL pointer or size is 0
*/
fpc_lib_return_t fpc_lib_debug_retrieve_image(uint8_t* img, uint32_t size);
/* 
 * Used when phone "wake up" is triggered by the sensor. If qualification 
 * passes the phone should be woken up.
 *
 * @return FPC_FINGER_PRESENT - if the wake qualification passed
 *         FPC_FINGER_LOST - if the qualification fails
*/
fpc_lib_return_t fpc_lib_wakeup_qualification(void);

/*
 * Returns status from the sensor module.
 *
 * @param[out] status, status from the sensor module
 *
 * @return FPC_LIB_OK
 *         FPC_LIB_ERROR_PARAMETER - if status is a NULL pointer.
*/
fpc_lib_return_t fpc_lib_get_sensor_status(fpc_lib_sensor_status_t* status);

#endif
