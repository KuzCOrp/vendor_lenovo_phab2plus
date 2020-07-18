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

extern int gTargetFd;     //TODO make this cleaner: interface filedescr
static  int i2cserver_verbose;
void i2cserver_set_verbose(int val) {
    i2cserver_verbose = val;
}
#define VERBOSE if (i2cserver_verbose)

static int i2c_Speed=0;
int i2c_GetSpeed(int bus) {
    return i2c_Speed;
}
void i2c_SetSpeed(int bus, int bitrate) {
    i2c_Speed=bitrate;
}

static void hexdump(int num_write_bytes, const unsigned char * data)
{
    int i;

    for(i=0;i<num_write_bytes;i++)
    {
        printf("0x%02x ", data[i]);
    }

}

int i2c_WriteRead(int bus, int addrWr, void* dataWr, int sizeWr, int* nWr, void* dataRd, int sizeRd, int* nRd)
{
    NXP_I2C_Error_t err;

    err = NXP_I2C_WriteRead(addrWr<<1, sizeWr, dataWr, sizeRd, dataRd);

    if ( err == NXP_I2C_Ok) {
        *nRd=sizeRd; //actual
    } else
        *nRd=0;

    return err == NXP_I2C_Ok;
}

_Bool i2c_Write(int bus, int addrWr, void* dataWr, int sizeWr)
{
    return  ( NXP_I2C_Ok == NXP_I2C_Write(addrWr<<1, sizeWr,dataWr )) ?  1 : 0 ;
}

_Bool i2c_Read(int bus, int addr, void* data, int size, int* nr)
{
    return i2c_WriteRead(bus, addr, NULL, 0, NULL, data, size, nr);
}



