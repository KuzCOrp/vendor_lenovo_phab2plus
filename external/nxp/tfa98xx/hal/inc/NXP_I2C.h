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

#ifndef NXP_I2C_H
#define NXP_I2C_H

#define TFA98XX_HAL_STR1(s) #s
#define TFA98XX_HAL_STR(s) TFA98XX_HAL_STR1(s)

//Version
#define TFA98XX_HAL_REV_MAJOR    1
#define TFA98XX_HAL_REV_MINOR    5
#define TFA98XX_HAL_REV_REVISION 0

#define TFA98XX_HAL_REV_STR TFA98XX_HAL_STR(TFA98XX_HAL_REV_MAJOR) "." \
                            TFA98XX_HAL_STR(TFA98XX_HAL_REV_MINOR) "." \
                            TFA98XX_HAL_STR(TFA98XX_HAL_REV_REVISION)


#ifdef __cplusplus
extern "C" {
#endif

/* A glue layer.
 * The NXP components will use the functions defined in this API to do the actual calls to I2C
 * Make sure you implement this to use your I2C access functions (which are SoC and OS dependent)
 */

enum NXP_I2C_Error
{
  NXP_I2C_UnassignedErrorCode,
  NXP_I2C_Ok,
  NXP_I2C_NoAck,
  NXP_I2C_SclStuckAtOne,
  NXP_I2C_SdaStuckAtOne,
  NXP_I2C_SclStuckAtZero,
  NXP_I2C_SdaStuckAtZero,
  NXP_I2C_TimeOut,
  NXP_I2C_ArbLost,
  NXP_I2C_NoInit,
  NXP_I2C_Disabled,
  NXP_I2C_UnsupportedValue,
  NXP_I2C_UnsupportedType,
  NXP_I2C_NoInterfaceFound,
  NXP_I2C_NoPortnumber,
  NXP_I2C_BufferOverRun,
  NXP_I2C_ErrorMaxValue
};

typedef enum NXP_I2C_Error NXP_I2C_Error_t;

/*
 * use tracefile fo output
 *  args:
 *      0      = stdout
 *      "name" = filename
 *      -1     = close file
 *  return fd or -1 on error
 */
void NXP_I2C_Trace_file(char *filename);

/**
 * return HAL revision
 */
void NXP_I2C_rev(int *major, int *minor, int *revision);

/* The maximum I2C message size allowed for read and write buffers, incl the slave address */
#define NXP_I2C_MAX_SIZE 254
/* The maximum I2C burst size, transaction will be split into smaller chunks */
//#define NXP_I2C_MAX_BURST 32 //if defined then NXP_I2C_Write() will enable this, note that the read has not been done yet

/* Execute an I2C write transaction
   @sla = slave address
   @num_write_bytes = size of data[]
   @data[] = byte array of data to write
*/
NXP_I2C_Error_t NXP_I2C_Write(  unsigned char sla,
                int num_write_bytes,
                                const unsigned char data[] );

/* Execute a write, followed by I2C restart and a read of num_read_bytes bytes.
   The read_buffer must be big enough to contain num_read_bytes.
   @sla = slave address
   @num_write_bytes = size of data[]
   @write_data[] = byte array of data to write
   @num_read_bytes = size of read_buffer[] and number of bytes to read
   @read_buffer[] = byte array to receive the read data
*/
NXP_I2C_Error_t NXP_I2C_WriteRead(  unsigned char sla,
                int num_write_bytes,
                const unsigned char write_data[],
                int num_read_bytes,

                unsigned char read_buffer[] );

/* Read back the version info */
NXP_I2C_Error_t NXP_I2C_Version(char *data);

/* Returns the number of bytes that can be transfered in one transaction */
int NXP_I2C_BufferSize();
/* enable/disable trace */
void NXP_I2C_Trace(int on);
/*
 * use tracefile fo output
 *  args:
 *      0      = stdout
 *      "name" = filename
 *      -1     = close file
 *  return fd or -1 on error
 */
void NXP_I2C_Trace_file(char *filename);
/* fill the interface */
int  NXP_I2C_Interface(char  *target,
        int (*init)(char *dev),
        int (*write)(int fd, int size, unsigned char *buffer, unsigned int *pError),
        int (*write_read)(int fd, int wsize, const unsigned char *wbuffer,
                int rsize, unsigned char *rbuffer, unsigned int *pError),
                int (*version_str)(char *buffer, int fd));

#if (defined(WIN32) || defined(_X64))
NXP_I2C_Error_t init_I2C();
#endif

#ifdef __cplusplus
}
#endif

#endif // NXP_I2C_H
