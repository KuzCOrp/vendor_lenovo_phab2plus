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

#if !(defined(WIN32) || defined(_X64))
#include <unistd.h>
#else
#undef __cplusplus

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>


//Link  with Scribo.lib. Make sure the Scribo.lib can be found by the linker
#ifdef _X64
#pragma comment (lib, "Scribo64.lib")
#else
#pragma comment (lib, "Scribo.lib")
#endif

#endif // WINDOWS
/* implementation of the NXP_I2C API on Windows */
#include "NXP_I2C.h"

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "dbgprint.h"
#include "NXP_I2C.h"
#include "lxScribo.h"
#include "Scribo.h"

/* the interface */
static int (*lxInit)(char *dev);
static int (*lxWrite)(int fd, int size, unsigned char *buffer, unsigned int *pError);
static int (*lxWriteRead)(int fd, int wsize, const unsigned char *wbuffer
              , int rsize, unsigned char *rbuffer, unsigned int *pError);
static int (*lxVersionStr)(char *buffer, int fd);

static FILE *traceoutput = NULL;

static int gI2cBufSz=NXP_I2C_MAX_SIZE;
/*
 * accounting globals
 */
int gNXP_i2c_writes=0, gNXP_i2c_reads=0;

#ifndef __BOOL_DEFINED
  typedef unsigned char bool;
  #define true ((bool)(1==1))
  #define false ((bool)(1==0))
#endif

static NXP_I2C_Error_t translate_error(uint32_t error);

typedef struct
{
  char msg[32];
} NXP_I2C_ErrorStr_t;

static bool bInit = false;

static int i2cTargetFd=-1;                         /* file descriptor for target device */

int NXP_I2C_verbose;
extern int lxScriboGetFd(void);
//#define VERBOSE(format, args...) if (NXP_I2C_verbose) PRINT(format, ##args)
#define VERBOSE if (NXP_I2C_verbose)

/**
 * return HAL revision
 */
void NXP_I2C_rev(int *major, int *minor, int *rev)
{
    *major = TFA98XX_HAL_REV_MAJOR;
    *minor = TFA98XX_HAL_REV_MINOR;
    *rev   = TFA98XX_HAL_REV_REVISION;
}
/*
 * use tracefile fo output
 *  args:
 *      0      = stdout
 *      "name" = filename
 *      NULL   = close file
 */
void NXP_I2C_Trace_file(char *filename) {
    if ((traceoutput != NULL) && (traceoutput != stdout)) {
        /* close if open ,except for stdout */
        fclose(traceoutput);
    }
    if ( filename > 0) {
        /* append to file */
        traceoutput = fopen(filename, "a");
        if ( traceoutput == NULL) {
            PRINT_ERROR("Can't open %s\n", filename);
        }
    } else if (filename==0) {
        traceoutput = stdout;
    }
}

/* enable/disable trace */
void NXP_I2C_Trace(int on) {
    if ( traceoutput == NULL )
        traceoutput = stdout;
    NXP_I2C_verbose = on;
}

static void hexdump(int num_write_bytes, const unsigned char * data)
{
    int i;

    for(i=0;i<num_write_bytes;i++)
    {
        PRINT_FILE(traceoutput, "0x%02x ", data[i]);
    }
}
static void i2c_print_trace(char *format, int length, unsigned char *data) {
     PRINT_FILE(traceoutput, format, length);
     hexdump(length, data);
     PRINT_FILE(traceoutput, "\n");
     fflush(traceoutput);
}


static NXP_I2C_Error_t init_if_firsttime(void)
{
    NXP_I2C_Error_t retval = NXP_I2C_Ok;

    if (!bInit)
    {
        int fd = lxScriboRegister(NULL); /* this will register the default target */
        if (fd < 0) { /* invalid file descriptor */
            retval = eI2C_UNSPECIFIED;
        }
        NXP_I2C_Trace_file(0); /* default to stdout */
        bInit = true;
    }

    return retval;
}

static int  recover(void) {
    i2cTargetFd = (*lxInit)(NULL); /* this should reset the connection */
    return i2cTargetFd<0; /* failed if -1 */
}
/* fill the interface and init */
int  NXP_I2C_Interface(char  *target,
        int (*init)(char *dev),
        int (*write)(int fd, int size, unsigned char *buffer, unsigned int *pError),
        int (*write_read)(int fd, int wsize, const unsigned char *wbuffer,
                                   int rsize, unsigned char *rbuffer, unsigned int *pError),
                int (*version_str)(char *buffer, int fd))
{
        lxInit = init;
    lxWrite = write;
    lxWriteRead = write_read;
        lxVersionStr = version_str;
    i2cTargetFd = (*lxInit)(target);
    bInit = true; /* lxScriboRegister() can be called before */
    return i2cTargetFd;
}

NXP_I2C_Error_t NXP_I2C_Write(  unsigned char sla,
                                int num_write_bytes,
                                const unsigned char data[] )
{
  NXP_I2C_Error_t retval;
  uint32_t error;

    if (num_write_bytes > gI2cBufSz)
    {
        PRINT_ERROR("%s: too many bytes: %d\n", __FUNCTION__, num_write_bytes);
        return NXP_I2C_UnsupportedValue;
    }

    retval = init_if_firsttime();

    if (NXP_I2C_Ok == retval)
    {
        unsigned char buffer[NXP_I2C_MAX_SIZE+1];
        buffer[0] = sla;

        memcpy((void*)&buffer[1], (void*)data, num_write_bytes+1); // prepend slave address

        (*lxWrite)(i2cTargetFd, num_write_bytes+1, buffer, &error );

        retval =  error;

        VERBOSE i2c_print_trace("I2C w [%3d]: ", num_write_bytes+1,  buffer);
    }
    return retval;
}

NXP_I2C_Error_t NXP_I2C_WriteRead(  unsigned char sla,
                                    int num_write_bytes,
                                    const unsigned char write_data[],
                                    int num_read_bytes,
                                    unsigned char read_data[] )
{
  NXP_I2C_Error_t retval;
  uint32_t error;

    if (num_write_bytes > gI2cBufSz)
    {
        PRINT_ERROR("%s: too many bytes to write: %d\n", __FUNCTION__, num_write_bytes);
        return NXP_I2C_UnsupportedValue;
    }
    if (num_read_bytes > gI2cBufSz)
    {
        PRINT_ERROR("%s: too many bytes to read: %d\n", __FUNCTION__, num_read_bytes);
        return NXP_I2C_UnsupportedValue;
    }

  retval = init_if_firsttime();
  if (NXP_I2C_Ok==retval)
  {
      unsigned char wbuffer[NXP_I2C_MAX_SIZE], rbuffer[NXP_I2C_MAX_SIZE];

      wbuffer[0] = sla;
      memcpy((void*)&wbuffer[1], (void*)write_data, num_write_bytes); // prepend slave address

      rbuffer[0] = sla|1; //read slave

      VERBOSE i2c_print_trace("I2C W [%3d]: ", num_write_bytes+1,  wbuffer);

      /* num_read_bytes will include the slave byte, so it's incremented by 1 if ok */
      num_read_bytes = (*lxWriteRead)(i2cTargetFd,  num_write_bytes+1, wbuffer,
                                                                                                            num_read_bytes+1, rbuffer, &error);

      retval =  error;

      //    if (!WriteRead(sla >> 1, write_data, ((uint16_t)num_write_bytes), read_data, &rCnt))
        if (num_read_bytes ) {
            VERBOSE i2c_print_trace("I2C R [%3d]: ", num_read_bytes,  rbuffer); // also show slave
            memcpy((void*)read_data, (void*)&rbuffer[1], num_read_bytes-1); // remove slave address
        } else {
            PRINT_ERROR("empty read in %s\n", __FUNCTION__);
            recover();
        }
  }
  return retval;
}

NXP_I2C_Error_t NXP_I2C_Version(char *data)
{
        NXP_I2C_Error_t retval;
        int bufSz = 1024;
        int fd = 0;

    retval = init_if_firsttime();

    if (NXP_I2C_Ok == retval)
    {
        fd = (*lxVersionStr)(data, fd);
    }

        if (fd < bufSz) {
                data[fd] = '\n';
                data[fd+1] = '\0';
        } else
                retval = NXP_I2C_UnassignedErrorCode;

    return retval;
}

int NXP_I2C_BufferSize()
{
    NXP_I2C_Error_t error;
    error = init_if_firsttime();
    if (error == NXP_I2C_Ok) {
        return gI2cBufSz > NXP_I2C_MAX_SIZE ? gI2cBufSz : NXP_I2C_MAX_SIZE - 1;
    }
    return NXP_I2C_MAX_SIZE - 1; //255 is minimum
}
