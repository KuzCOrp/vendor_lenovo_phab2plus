/*
 * teei_fp.h
 *
 *  Created on: Apr 15, 2015
 *      Author: napoleonliu
 */

#ifndef TEEI_FP_H_
#define TEEI_FP_H_
#include "fpc_tac.h"
typedef struct{
	tci_command_id_t cid;
	tci_command_id_t fid;
	void* data;
	unsigned int data_size;
}TEEI_Operation;
fpc_def_return_t TEEI_InvokeCommand(tci_command_id_t cid, tci_fpc_cmd_t fid, uint32_t *data, void* data_struct, uint32_t* data_size);
#endif /* TEEI_FP_H_ */
