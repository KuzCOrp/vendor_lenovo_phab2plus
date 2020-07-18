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



#ifndef LXSCRIBO_H_
#define LXSCRIBO_H_
#include "../../../tfa98xx_cust.h"

#define LXSCRIBO_VERSION     1.0

/** the default target device */
//#if defined(WIN32) || defined(x64)
//#define TFA_I2CDEVICE        "scribo"            /* windows, pseudo device */
//#else
//#define TFA_I2CDEVICE        "/dev/ttyACM0" /* linux default */
//#endif

#define LXSOCKET             "9887"         // note: in ascii for api alignment with serial

/*
 * devkit pin assignments
 */

enum devkit_pinset {
    pinset_gpo_en_ana,
    pinset_gpo_en_i2s1,
    pinset_gpo_en_spdif,
    pinset_gpo_en_lpc,
    pinset_power_1v8, // 4
    pinset_led1,
    pinset_led2,
    pinset_led3,
    pinset_power_1v8_also, //8
    pinset_tfa_rst_l, // 9
    pinset_tfa_rst_r
};
enum devkit_pinget {
    pinget_gpi_det_i2s1,
    pinget_gpi_det_spdif,
    pinget_gpi_det_ana,
    pinget_gpi_det_i2s2,
    pinget_tfa_int_l, //4
    pinget_tfa_int_r
};
void  lxScriboVerbose(int level);        // set verbose level.
int lxScriboRegister(char *dev);    // register target and return opened file desc.
int lxScriboGetFd(void);            // return active file desc.
char * lxScriboGetName(void);    // target name

int lxScriboVersion(char *buffer, int fd);
int lxScriboWrite(int fd, int size, unsigned char *buffer, unsigned int *pError);
int lxScriboWriteRead(int fd, int wsize, const unsigned char *wbuffer
                                           , int rsize, unsigned char *rbuffer, unsigned int *pError);
int lxScriboPrintTargetRev(int fd);
int lxScriboSerialInit(char *dev);
int lxScriboSocketInit(char *dev);
void lxScriboSocketExit(int status);
int lxScriboSetPin(int fd, int pin, int value);

#ifdef HAL_HID
int lxHidInit(char *dev);
int lxHidWrite(int fd, int size, unsigned char *buffer, unsigned int *pError);
int lxHidWriteRead(int fd, int wsize, const unsigned char *wbuffer
                                           , int rsize, unsigned char *rbuffer, unsigned int *pError);
int lxHidVersion(char *buffer, int fd);

#endif
int lxI2cInit(char *dev);
int lxI2cWrite(int fd, int size, unsigned char *buffer, unsigned int *pError);
int lxI2cWriteRead(int fd, int wsize, const unsigned char *wbuffer
                                           , int rsize, unsigned char *rbuffer, unsigned int *pError);
int lxI2cVersion(char *buffer, int fd);

void lxDummyVerbose(int level);

int lxDummyInit(char *dev);
int lxDummyWrite(int fd, int size, unsigned char *buffer, unsigned int *pError);
int lxDummyWriteRead(int fd, int wsize, const unsigned char *wbuffer
                                           , int rsize, unsigned char *rbuffer, unsigned int *pError);
int lxDummyVersion(char *buffer, int fd);

int lxHtcInit(char *dev);
int lxHtcWrite(int fd, int size, unsigned char *buffer, unsigned int *pError);
int lxHtcWriteRead(int fd, int wsize, const unsigned char *wbuffer
                                           , int rsize, unsigned char *rbuffer, unsigned int *pError);

int lxWindowsInit(char *this);
int lxWindowsWrite(int fd, int num_write_bytes, unsigned char *buffer, unsigned int *pError);
int lxWindowsWriteRead(int fd, int NrOfWriteBytes, const unsigned char *WriteData,
             int NrOfReadBytes, unsigned char *ReadData, unsigned int *pError);
int lxWindowsVersion(char *buffer, int fd);

int lxScriboGetRev(int fd, char *str);
int lxScriboGetPin(int fd, int pin);

extern int i2c_trace;
extern int NXP_I2C_verbose;

// for dummy
#define rprintf printf

//from gpio.h
#define I2CBUS 0
//from gui.h
/* IDs of menu items */
typedef enum
{
  ITEM_ID_VOLUME = 100,
  ITEM_ID_PRESET,
  ITEM_ID_EQUALIZER,

  ITEM_ID_STANDALONE,

  ITEM_ID_ENABLESCREENDUMP,
} menuElement_t;

#endif /* LXSCRIBO_H_ */
