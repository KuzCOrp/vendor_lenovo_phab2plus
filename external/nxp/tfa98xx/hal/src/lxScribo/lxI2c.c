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


#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include "dbgprint.h"
#include "lxScribo.h"
#include "Tfa98xx.h"    // for i2c slave address
#include "NXP_I2C.h" /* for the error codes */

int i2c_trace=0;
extern int tfa98xxI2cSlave;
static   char filename[FILENAME_MAX];

#ifndef I2C_SLAVE
#define I2C_SLAVE    0x0703    /* dummy address for building API    */
#endif

static void hexdump(int num_write_bytes, const unsigned char * data)
{
    int i;

    for(i=0;i<num_write_bytes;i++)
    {
        PRINT("0x%02x ", data[i]);
    }

}

void  lxI2cSlave(int fd, int slave)
{
    // open the slave
    int res = ioctl(fd, I2C_SLAVE, slave);
    if ( res < 0 ) {
        /* TODO ERROR HANDLING; you can check errno to see what went wrong */
        ERRORMSG("Can't open i2c slave:0x%02x\n", slave);
        _exit(res);
    }

    if (i2c_trace) PRINT("I2C slave=0x%02x\n", slave);

}

/* SC42158
 *  Remove slave checking and use the address from the transaction.
 */
int lxI2cWriteRead(int fd, int NrOfWriteBytes, const uint8_t * WriteData,
        int NrOfReadBytes, uint8_t * ReadData, unsigned int *pError) {
    int ln = -1;

    lxI2cSlave(fd, WriteData[0]>>1 );

    if (NrOfWriteBytes & i2c_trace) {
        PRINT("W %d:", NrOfWriteBytes);
        hexdump (NrOfWriteBytes, WriteData);
        PRINT("\n");
    }

    if (NrOfWriteBytes > 2)

        ln = write(fd, &WriteData[1],  NrOfWriteBytes - 1);

    if (NrOfReadBytes) { // bigger
        //if ( (ReadData[0]>1) != (WriteData[0]>1) ) // if block read is different
        //        write(fd, &ReadData[0],  1);
        ln = write(fd, &WriteData[1],1); //write sub address
    if ( ln < 0 ) {
      *pError = NXP_I2C_NoAck; /* treat all errors as nack */
    } else {
      ln = read(fd,  &ReadData[1], NrOfReadBytes-1);
    }
  }

    if (NrOfReadBytes & i2c_trace) {
        PRINT("R %d:", NrOfReadBytes);
        hexdump (NrOfReadBytes, ReadData);
        PRINT("\n");
    }

    if ( ln < 0 ) {
        *pError = NXP_I2C_NoAck; /* treat all errors as nack */
        perror("i2c slave error");
    } else {
        *pError = NXP_I2C_Ok;
    }

    return ln+1;
}

int lxI2cWrite(int fd, int size, uint8_t *buffer, unsigned int *pError)
{
    return lxI2cWriteRead( fd, size, buffer, 0, NULL, pError);

}
/*
 * SC42158
 *  Slave is set in the transaction call.
 */
int lxI2cInit(char *devname)
{
    static int fd=-1;

    if ( devname )
        strcpy(filename, devname);

//    if ( lxScribo_verbose ) TODO
//        printf("Opening serial Scribo connection on %d\n", filename);

    if (fd != -1 )
        if (close(fd) )
            _exit(1);

    fd = open(filename, O_RDWR | O_NONBLOCK | O_EXCL, 0);
    if (fd < 0) {
        ERRORMSG("Can't open i2c bus:%s\n", filename);
        _exit(1);
    }

    return fd;
}

int lxI2cVersion(char *buffer, int fd)
{
        PRINT("Not implemented! \n");
    return 0;
}
