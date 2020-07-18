/*
 *Copyright 2014 NXP Semiconductors
 *
 *Licensed under the Apache License, Version 2.0 (the "License");
 *you may not use this file except in compliance with the License.
 *You may obtain a copy of the License at
 *
 *http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS,
 *WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lxScribo.h"
#include "NXP_I2C.h"

int i2c_GetSpeed(int bus);
void i2c_SetSpeed(int bus, int bitrate);

int i2c_WriteRead(int bus, int addrWr, void* dataWr, int sizeWr, int* nWr, void* dataRd, int sizeRd, int* nRd);
_Bool i2c_Write(int bus, int addrWr, void* dataWr, int sizeWr);
_Bool i2c_Read(int bus, int addr, void* data, int size, int* nr);
_Bool gui_GetValue(menuElement_t item, int* val);
_Bool gui_SetValue(menuElement_t item, int* val);


