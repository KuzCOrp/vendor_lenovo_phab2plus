/*
 * lxHid.c
 *
 *  Created on: Oct 16, 2014
 *      Author: wim
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

// the USGIO API
#define TB_DEBUG 1
#include "usbgio_api.h"
// map Windows TCHAR functions and macros to normal C-Runtime library functions on Linux
#define _ftprintf fprintf
#define _T(x)     x

// handle to the device
static USBGIODeviceHandle gDeviceHandle = USBGIO_INVALID_HANDLE;

static hid_i2c_slave=-1;

extern int i2c_trace;
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

void  lxHidSlave(int fd, int slave)
{
    // set the slave
    hid_i2c_slave = slave;
#if www
    int res = ioctl(fd, I2C_SLAVE, slave);
    if ( res < 0 ) {
        /* TODO ERROR HANDLING; you can check errno to see what went wrong */
        ERRORMSG("Can't open i2c slave:0x%02x\n", slave);
        _exit(res);
    }
#endif
    if (i2c_trace) PRINT("I2C slave=0x%02x\n", slave);

}

/* SC42158
 *  Remove slave checking and use the address from the transaction.
 */
int lxHidWriteRead(int fd, int NrOfWriteBytes, const uint8_t * WriteData,
        int NrOfReadBytes, uint8_t * ReadData, unsigned int *pError) {
    int ln = -1;
      TSTATUS st;
      int rdcount;
      uint8_t *rdbuf=NULL;

      if(NrOfReadBytes) {
          rdcount= NrOfReadBytes-1;
          rdbuf=&ReadData[1];
          ln=rdcount;
      }
      else {
          rdcount=0;
          ln = NrOfWriteBytes-1;
      }

    lxHidSlave(fd, WriteData[0]>>1 );

    if (NrOfWriteBytes & i2c_trace) {
        PRINT("W %d:", NrOfWriteBytes);
        hexdump (NrOfWriteBytes, WriteData);
        PRINT("\n");
    }

//hid+++
      // gIntArgs[0] is the address of the register to read
//      unsigned char regAddr = (unsigned char)gIntArgs[0];
//      unsigned char readData[2];
      st = USBGIO_I2cTransaction(gDeviceHandle,                 // USBGIODeviceHandle deviceHandle,
                                 0,                             // unsigned int i2cBus,
                                  (400 * 1000),                 // unsigned int i2cSpeed,
                                 WriteData[0] >> 1,    // unsigned int i2cSlaveAddress,
                                 &WriteData[1],                      // const unsigned char* writeData,
                                 NrOfWriteBytes-1,               // unsigned int bytesToWrite,
                                 rdbuf,                       // unsigned char* readBuffer,
                                 rdcount,              // unsigned int bytesToRead,
                                 1000                           // unsigned int timeoutInterval
                                 );
      if (TSTATUS_SUCCESS != st) {
        _ftprintf(stderr, _T("USBGIO_I2cTransaction failed, status = 0x%08X\n"), st);
        return -1;
      }


//hid---
#if www
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
#endif
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

int lxHidWrite(int fd, int size, uint8_t *buffer, unsigned int *pError)
{
    return lxHidWriteRead( fd, size, buffer, 0, NULL, pError);

}
///+++

static
TSTATUS
OpenDevice()
{
  TSTATUS st;

  // enumerate available devices
  _ftprintf(stdout, _T("Enumerate available devices...\n"));
  unsigned int devCnt;
  st = USBGIO_EnumerateDevices(&devCnt);
  if (TSTATUS_SUCCESS != st) {
    _ftprintf(stderr, _T("USBGIO_EnumerateDevices failed, status = 0x%08X\n"), st);
    return st;
  }

  if (0 == devCnt) {
    _ftprintf(stderr, _T("No devices connected, please connect a device.\n"));
    return TSTATUS_NO_DEVICES;
  }

  _ftprintf(stdout, _T("Number of connected devices: %u\n"), devCnt);

  // open the first device
  _ftprintf(stdout, _T("Open device with index 0...\n"));
  T_UNICHAR devInst[512];
  st = USBGIO_GetDeviceInstanceId(0,                    // unsigned int deviceIndex,
                                  devInst,              // T_UNICHAR* instanceIdString,
                                  sizeof(devInst)       // unsigned int stringBufferSize
                                  );
  if (TSTATUS_SUCCESS != st) {
    _ftprintf(stderr, _T("USBGIO_GetDeviceInstanceId failed, status = 0x%08X\n"), st);
    return st;
  }

  st = USBGIO_OpenDevice(devInst,         // const T_UNICHAR* instanceIdString,
                         &gDeviceHandle   // USBGIODeviceHandle* deviceHandle
                         );
  if (TSTATUS_SUCCESS != st) {
    _ftprintf(stderr, _T("USBGIO_OpenDevice failed, status = 0x%08X\n"), st);
    return st;
  }

  return TSTATUS_SUCCESS;
} // OpenDevice
//----
/*
 *
 *  Slave is set in the transaction call.
 */
int lxHidInit(char *devname)
{
    static int fd=-1;

    if ( devname )
        strcpy(filename, devname);

//    if ( lxScribo_verbose ) TODO
//        printf("Opening serial Scribo connection on %d\n", filename);
#if www
    if (fd != -1 )
        if (close(fd) )
            _exit(1);

    fd = open(filename, O_RDWR | O_NONBLOCK | O_EXCL, 0);
    if (fd < 0) {
        ERRORMSG("Can't open i2c bus:%s\n", filename);
        _exit(1);
    }
#endif
    return OpenDevice() == TSTATUS_SUCCESS ? 1 : -1;
}


