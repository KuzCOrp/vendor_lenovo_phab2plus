/*
 * teei_fpc.c
 *
 *  Created on: Apr 15, 2015
 *      Author: napoleonliu
 */
#include <string.h>
#include "fpc_tac.h"
#include "teei_fp.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <android/log.h>
#include <errno.h> 
#include <cutils/properties.h>
#include "fpc_hal.h"
#include <stdlib.h>

#define BUFFER_SIZE 4096
#define CID_LENGTH sizeof(tci_command_id_t)
#define FID_LENGTH sizeof(tci_fpc_cmd_t)
#define TEEI_RESULT_LENGTH sizeof(fpc_def_return_t)
#define DATA_SIZE_LENGTH sizeof(uint32_t)
#define DATA_PTR_OFFSET CID_LENGTH+FID_LENGTH+TEEI_RESULT_LENGTH+DATA_SIZE_LENGTH
#define TEMPLATE_UPDATE_WORKAROUND

bool _fpc_tac_open = false;
static fpc_lib_identify_data_t last_identify_result;
static fpc_def_return_t _fpc_tac_send_cmd(tci_command_id_t cid, tci_fpc_cmd_t fid, uint32_t *data);
static fpc_def_return_t _fpc_tac_transfer_data(tci_fpc_cmd_t fid,
		uint32_t *data, void* data_struct, uint32_t* data_size);
static fpc_def_return_t _fpc_tac_store_templates(void);
static fpc_def_return_t _fpc_tac_load_templates(void);
static bool fpc_tac_check_connection();
static int teei_ready();
uint32_t _fpc_tac_tac_error_count = 0;

void *buffer_data;
int teei_fp_fd = 0;
const char * TEEI_NODE = "/dev/teei_client";
/******************************************************************************/
fpc_def_return_t fpc_tac_init() {
	LOG_I("%s start", __func__);
    fpc_def_return_t ret;
    uint32_t data = 0;
    if(teei_ready() != 0){
        return FPC_DEF_ERROR_TAC;
    }
    if (!fpc_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
    }
    buffer_data = malloc(BUFFER_SIZE);
    LOG_I("try open file");
    if(teei_fp_fd == 0){
        teei_fp_fd = open("/dev/teei_fp", O_RDWR);
    }
	if(!teei_fp_fd){
		LOG_E("%s open teei_fp_fd failed", __func__);
		ret = FPC_DEF_ERROR_TAC;
	} else {
		LOG_I("file open success");
		if(!buffer_data){
			LOG_E("%s Failed to alloc buffer memory", __func__);
			ret = FPC_DEF_ERROR_TAC;
			close(teei_fp_fd);
            teei_fp_fd = 0;
		} else {
			ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_INIT, &data);
			if (ret != FPC_DEF_OK) {
				LOG_E("%s Failed to send init command, %d", __func__, ret);
			    close(teei_fp_fd);
                teei_fp_fd = 0;
                free(buffer_data);
            } else {
            	ret = _fpc_tac_load_templates();
			}
		}
	}

    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_deinit() {
    fpc_def_return_t ret;
    uint32_t data;
    if (!fpc_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_DEINIT,&data);
    if (ret != FPC_DEF_OK) {
        LOG_E("%s Failed to send deinit command, %d", __func__, ret);
    }
    if(teei_fp_fd){
    	close(teei_fp_fd);
        teei_fp_fd = 0;
    }
    if(buffer_data){
    	free(buffer_data);
    }
    return ret;
}

/******************************************************************************/
void fpc_tac_close(void) {}

/******************************************************************************/
fpc_def_return_t fpc_tac_open(void) {
	fpc_def_return_t result;
    return FPC_DEF_OK;
}
/******************************************************************************/
static bool fpc_tac_check_connection() {
    fpc_def_return_t ret;
    if (!_fpc_tac_open) {
        LOG_I("%s TAC not started restart.", __func__);
        if (_fpc_tac_tac_error_count > 10) {
			return false;
		}
		_fpc_tac_tac_error_count++;
        ret = fpc_tac_open();
        if (ret != FPC_DEF_OK) {
            LOG_E("%s TAC failed to start.", __func__);
            sleep(5);
            fpc_tac_close();
            teei_fp_fd = 0;
            return false;
        } else {
			_fpc_tac_open = true;

        }
    }
    _fpc_tac_tac_error_count = 0;
    LOG_I("%s",  __func__);
    return true;
}
//TODO where is MC_DRV_OK
#define MC_DRV_OK 0
/******************************************************************************/
fpc_def_return_t fpc_tac_begin_enroll() {
	fpc_def_return_t ret;
	uint32_t data;
	LOG_I("-->%s", __func__);
	if (!fpc_tac_check_connection()) {
		LOG_E("%s No connection to TA, end command", __func__);
		return FPC_DEF_ERROR_TAC;
	}

	ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_BEGIN_ENROLL, &data);
	if (ret != FPC_DEF_OK) {
		LOG_E("%s, send command failed with 0x%02X, closing TAC"
				, __func__, ret);
		//fpc_tac_close();
		return FPC_DEF_ERROR_GENERAL;
	}

	LOG_I("<--%s returns %d", __func__, ret);
	return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_wakeup_setup() {
    fpc_def_return_t ret;

    LOG_I("-->%s", __func__);
    if (!fpc_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FPC_DEF_ERROR_TAC;
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_WAKEUP_SETUP,0);
    if (ret != FPC_DEF_OK) {
        LOG_E("%s, send command failed with 0x%02X", __func__, ret);
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_wakeup_qualification() {
    fpc_def_return_t ret;

    LOG_I("-->%s", __func__);
    if (!fpc_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FPC_DEF_ERROR_TAC;
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_WAKEUP_QUALIFICATION,0);
    if (ret == FPC_DEF_ERROR_TAC) {
        LOG_E("%s, send command failed with 0x%02X", __func__, ret);
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}


/******************************************************************************/
fpc_def_return_t fpc_tac_enroll(fpc_lib_enroll_data_t* data) {
    fpc_def_return_t ret;
    uint32_t data_size = sizeof(fpc_lib_enroll_data_t);
    LOG_I("-->%s", __func__);

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_ENROLL, 0, data, &data_size);
    if (ret != FPC_DEF_OK) {
        LOG_E("<--%s send command failed with 0x%02X, closing TAC.",
                __func__, ret);
        //fpc_tac_close();
        return FPC_DEF_ERROR_GENERAL;
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}


/******************************************************************************/
fpc_def_return_t fpc_tac_end_enroll(uint32_t * id) {
    fpc_def_return_t ret;
    uint32_t data;
    LOG_I("-->%s", __func__);

    LOG_I("End enroll called");

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_END_ENROLL,&data);
    if (ret != FPC_DEF_OK) {
        LOG_E("%s: failed", __func__);
        return ret;
    }
    *id = data;
    ret = _fpc_tac_store_templates();
    if (ret != FPC_DEF_OK) {
        LOG_E("%s: store templates failed", __func__);
        return ret;
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_get_extended_enroll(fpc_lib_extended_enroll_t* data) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);
    uint32_t data_size = data->size;
    LOG_I("Extend Enroll data called");

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_EXTENDED_ENROLL_DATA, 0, data->data,
            &data_size);

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_init_finger_detect(uint32_t* data) {
    fpc_def_return_t ret;
    LOG_I("-->%s fid is %d", __func__, TCI_FPC_CMD_INIT_FINGER_DETECT);
    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_INIT_FINGER_DETECT, data);
    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_start_finger_detect(uint32_t* data) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);

    LOG_I("Start finger detect called");

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_START_FINGER_DETECT, data);
    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_check_finger_present(uint32_t* data) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);

    LOG_I("Check finger present called");

    if (!fpc_tac_check_connection()) {
        LOG_I("<--%s No communication setup", __func__);
        return FPC_DEF_ERROR_GENERAL;
    }
    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_CHECK_FINGER_PRESENT,data);
    if (ret == FPC_DEF_ERROR_GENERAL) {
        LOG_E("<--%s communication error %d", __func__, ret);
        return ret;
    }
    //TODO
    //*data = tlTci->rsp.cmd_ret;
    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_check_finger_lost(uint32_t* data) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);

    if (!fpc_tac_check_connection()) {
        LOG_E("<--%s No connection to TA %d", __func__, ret);
        return FPC_DEF_ERROR_GENERAL;
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_CHECK_FINGER_LOST,data);
    if (ret == FPC_DEF_ERROR_GENERAL) {
        LOG_E("<--%s communication error %d", __func__, ret);
        return ret;
    }
    LOG_I("<--%s returns %d data %d", __func__, ret, *data);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_capture_image() {
    fpc_def_return_t ret;
    uint32_t data;
    LOG_I("-->%s", __func__);

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_CAPTURE_IMAGE, &data);

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}


/******************************************************************************/
fpc_def_return_t fpc_tac_remove_template(uint32_t id) {
    fpc_def_return_t ret;
    char template_name[100];
    int error;
    uint32_t index;
    LOG_I("-->%s", __func__);

    LOG_I("Remove template %d", id);

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_DELETE_TEMPLATE, &id);
    if (ret != FPC_DEF_OK) {
        LOG_E("Error when removing template, id = %d", id);
    }
    //sprintf(template_name,"/data/template%d.db", tlTci->data);

    /*error = remove(template_name);
    if (error != 0) {
        LOG_I("%s, remove file failed %s\n",
                __func__, template_name);
    }*/
    _fpc_tac_store_templates();
    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

fpc_def_return_t fpc_tac_remove_all_templates() {
	fpc_def_return_t ret;
    int error;
    uint32_t index;
    LOG_I("-->%s", __func__);

    LOG_I("Remove all template");

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_DELETE_ALL_TEMPLATES,0);
    if (ret != FPC_DEF_OK) {
        LOG_E("Error when removing all template. (TCI_FPC_CMD_DELETE_ALL_TEMPLATES)");
    }

    /* Save the new database */
    _fpc_tac_store_templates();

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_begin_identify(uint32_t* ids, uint32_t length) {
    fpc_def_return_t ret;
    uint32_t size;
    LOG_I("-->%s", __func__);

    memset(&last_identify_result,0,sizeof(last_identify_result));

    if (!fpc_tac_check_connection()) {
		LOG_E("<--%s No connection to TA %d", __func__, ret);
		return FPC_DEF_ERROR_GENERAL;
	}

    size = length * sizeof(uint32_t);
    LOG_I("Begin identify called");

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_BEGIN_IDENTIFY, 0, ids, &size);
    memcpy(&last_identify_result, ids, sizeof(last_identify_result));
    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_identify(fpc_lib_identify_data_t* data) {
    fpc_def_return_t ret;
    uint32_t size;
    LOG_I("-->%s", __func__);

    size = sizeof(fpc_lib_identify_data_t);

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_IDENTIFY, 0, data, &size);
    if (ret != FPC_DEF_OK) {
        LOG_I("<--%s Returns %d %d %d %d", __func__, ret,
                data->result, data->index, data->score);
    } else {
        LOG_I("<--%s Returns %d", __func__, ret);
    }
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_end_identify() {
    fpc_def_return_t ret;
    uint32_t data;
    LOG_I("-->%s", __func__);


    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_END_IDENTIFY, &data);
    if (ret != FPC_DEF_OK) {
        LOG_I("<--%s returns %d", __func__, ret);
        return ret;
    }
    //TODO
#ifdef TEMPLATE_UPDATE_WORKAROUND
    if(last_identify_result.result == FPC_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE
       && last_identify_result.score > 280)
#endif
    {
       LOG_I("%s updating template", __func__);
    ret = _fpc_tac_store_templates();
    }

    LOG_I("<--%s returns %d", __func__, ret);
    return ret;
}


/******************************************************************************/
fpc_def_return_t fpc_tac_debug_retrieve_image(uint8_t* image, uint32_t* size) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);

    LOG_I("Debug retrieve image");

    ret =
        _fpc_tac_transfer_data(TCI_FPC_CMD_DEBUG_RETRIEVE_IMAGE, 0, image, size);

    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_debug_inject_image(uint8_t* image, uint32_t size) {
    fpc_def_return_t ret;
    uint32_t len = size;

    LOG_I("-->%s size %d", __func__, len);

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_DEBUG_INJECT_IMAGE, 0, image, &len);

    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_get_ids(uint32_t* ids, uint32_t* len) {
    fpc_def_return_t ret;
    uint32_t original_size = *len;
    LOG_I("-->%s len %d", __func__, *len);
    if (!fpc_tac_check_connection()) {
        LOG_E("<--%s connection to TA failed.", __func__);
        return FPC_DEF_ERROR_GENERAL;
    }

    LOG_I("Get Ids called.");
    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_GET_IDS, 0, ids, len);

    if (*len > original_size) {
        LOG_E("Number of ids in the database larger than expected,"
                " ask for length %d, got length %d", *len, original_size);
        return FPC_DEF_ERROR_MEM;
    }
    LOG_I("<--%s", __func__);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_get_ids_count(uint32_t* count) {
    fpc_def_return_t ret;
    LOG_I("-->%s count %d", __func__, *count);
    if (!fpc_tac_check_connection()) {
        LOG_E("<--%s connection to TA failed.", __func__);
        return FPC_DEF_ERROR_GENERAL;
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_GET_IDS_COUNT, count);
    /*if (ret != FPC_DEF_ERROR_GENERAL) {
        *count = *((uint32_t*)(buffer_data + DATA_PTR_OFFSET));
    }*/

    LOG_I("<--%s returnts %d count %d", __func__, ret, *count);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_deep_sleep(void) {
    fpc_def_return_t ret;
    LOG_I("-->%s", __func__);
    if (!fpc_tac_check_connection()) {
        LOG_E("<--%s connection to TA failed.", __func__);
        return FPC_DEF_ERROR_GENERAL;
    }

    ret = _fpc_tac_send_cmd(TCI_CMD_SEND_CMD, TCI_FPC_CMD_DEEP_SLEEP,0);

    LOG_I("<--%s returnts %d", __func__, ret);
    return ret;
}

/******************************************************************************/
fpc_def_return_t fpc_tac_deadpixel_test(int32_t* nDeadPixels) {
    fpc_def_return_t ret;
    uint32_t len = sizeof(int32_t);

    LOG_I("-->%s", __func__);
    if (!fpc_tac_check_connection()) {
        LOG_E("%s No connection to TA, end command", __func__);
        return FPC_DEF_ERROR_TAC;
    }

    ret = _fpc_tac_transfer_data(TCI_FPC_CMD_DEADPIXEL_TEST, 0, nDeadPixels, &len);
    if (ret != MC_DRV_OK) {
        LOG_E("%s, send command failed with 0x%02X, closing TAC"
                , __func__, ret);
        return FPC_DEF_ERROR_GENERAL;
    }

    LOG_I("<--%s returns %d,nDeadPixels=%d", __func__, ret, *nDeadPixels);
    return ret;
}

/******************************************************************************/
static fpc_def_return_t _fpc_tac_load_templates(void) {
    fpc_def_return_t ret = FPC_DEF_OK;
    LOG_I("Enter %s", __func__);
	ret = _fpc_tac_transfer_data(TCI_FPC_CMD_LOAD_ONE_TEMPLATE, 0, 0, 0);
    return ret;
}

/******************************************************************************/
static fpc_def_return_t _fpc_tac_store_templates(void) {
	fpc_def_return_t ret = FPC_DEF_OK;
	ret = _fpc_tac_transfer_data(TCI_FPC_CMD_STORE_ONE_TEMPLATE,0,0,0);
	return ret;
}


/******************************************************************************/
static fpc_def_return_t _fpc_tac_send_cmd(tci_command_id_t cid, tci_fpc_cmd_t fid, uint32_t *data) {
	LOG_I("%s start" , __func__);
	fpc_def_return_t result;
	//fpc_hal_stop_standby();
    result = TEEI_InvokeCommand(cid, fid, data, 0,0);
    //fpc_hal_standby();
    LOG_I("_tac_fpc_send_cmd result is %d" , result);
    return result;
}

/******************************************************************************/
static fpc_def_return_t _fpc_tac_transfer_data(
        tci_fpc_cmd_t fid,
        uint32_t* data,
        void* data_struct,
        uint32_t* data_struct_size) {
	fpc_def_return_t result;
    result = TEEI_InvokeCommand(TCI_CMD_TRANSFER_DATA, fid, data, data_struct, data_struct_size);
    return result;
}

fpc_def_return_t TEEI_InvokeCommand(tci_command_id_t cid, tci_fpc_cmd_t fid, uint32_t *data ,void* data_struct, uint32_t* data_size){
	fpc_def_return_t ret;
	uint32_t offset = 0;
	uint32_t len;
	memset(buffer_data, 0x0, BUFFER_SIZE);

	memcpy(buffer_data,&cid, CID_LENGTH);
	offset += CID_LENGTH;
	memcpy(buffer_data + offset ,&fid, FID_LENGTH);
	offset += FID_LENGTH;

	if(data){
		memcpy(buffer_data + offset, data, sizeof(uint32_t));
	}
	offset += sizeof(uint32_t);

	if(data_size){
		memcpy(buffer_data + offset, data_size, DATA_SIZE_LENGTH);
		if(*data_size > 0 && data_struct){
			offset += DATA_SIZE_LENGTH;
			memcpy(buffer_data + offset, data_struct, *data_size);
		}
	}

	LOG_I("Command info: %p", buffer_data);
	LOG_I("Command info cid: %d", *((tci_command_id_t *)buffer_data));
	LOG_I("Command info fid: %d", *((tci_fpc_cmd_t *)(buffer_data + CID_LENGTH)));
	LOG_I("Command info data: %d",*((uint32_t *)(buffer_data + CID_LENGTH + FID_LENGTH)));
	LOG_I("Command info data_length: %d", *((uint32_t *)(buffer_data + CID_LENGTH + FID_LENGTH + sizeof(uint32_t))));
	//LOG_I("Command info data_struct: %p", buffer_data);
#define FP_INVOKE  _IO(0x775A777E,0x2)
	//invoke api
	if(teei_fp_fd){
		LOG_I("use ioctl send buffer_data %p", buffer_data);
		int ioresult = ioctl(teei_fp_fd, FP_INVOKE, buffer_data);
		LOG_I("use ioctl send buffer_data result is %d ", ioresult);
		if(ioresult != 0){
			LOG_I("errno:\t\t%s\n",strerror(errno));
		}
		ret = *((fpc_def_return_t *)(buffer_data));
		//get result back
		if(data){
			*data = *((uint32_t *)(buffer_data + CID_LENGTH + FID_LENGTH));
		}
		if(data_size && data_struct){
			//copy result back to data_struct
			memcpy(data_struct, buffer_data + DATA_PTR_OFFSET, *data_size);
		}
	} else {
		//fp fd doesn't open
		ret = FPC_DEF_ERROR_TAC;
	}
    LOG_I("Command result: %x", ret);
	return ret;
}
#define TEEI_CLIENT_FULL_PATH_DEV_NAME "/dev/teei_client"
#define TEEI_CLIENT_IOC_MAGIC 0x775B777F /* "TEEI Client" */
#define TEEI_GET_TEEI_CONFIG_STAT \
    _IO(TEEI_CLIENT_IOC_MAGIC, 0x1001)
#if 1 
static int teei_ready() {
    LOG_I("Checking if TEEI is ready!!!!!!!!!!!!!!!!!!!!!!!!!");
    int client_fd = -1;
    int teei_ready = 0;
    while(!teei_ready){
        if(client_fd < 0){
            client_fd = open(TEEI_CLIENT_FULL_PATH_DEV_NAME, O_RDONLY);
        }
        if(client_fd < 0){
            LOG_I("teei_client node has not created, waiting");
            sleep(2);
        } else {
            teei_ready = ioctl(client_fd, TEEI_GET_TEEI_CONFIG_STAT, 0);
            if(teei_ready){
                LOG_I("TEEI has be ready!");
                break;
            }else {
                LOG_I("TEEI has not ready, waiting");
                sleep(2);
            }
        }
    }
    if(client_fd >=0){
        close(client_fd);
    }
    return 0;
}
#else
static int teei_ready() {
    LOG_I("Checking if TEEI is ready!!!!!!!!!!!!!!!!!!!!!!!!!");
    int client_fd = -1;
    int teei_ready = 0;
    char result_buf[PROPERTY_VALUE_MAX];
    int property_ret;
    while(!teei_ready){
        property_ret = property_get("soter.teei.init", result_buf, "DEFAULT_VALUE");
        if(property_ret <= 0){
            LOG_I("Get property failed when checking TEEI !!!!!!!!!!!!!!!!!!!!!!!!!");
            return -1;
        } else if(property_ret == 7) {
            LOG_I("TEEI has be ready!");
            teei_ready = 1;
        } else {
            LOG_I("TEEI has not ready, waiting result is %s", result_buf);
            sleep(2);
        }
    }
    return 0;
}
#endif
