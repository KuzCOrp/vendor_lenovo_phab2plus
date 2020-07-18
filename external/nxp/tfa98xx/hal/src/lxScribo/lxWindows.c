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

#undef __cplusplus

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include "Scribo.h"

//Link  with Scribo.lib. Make sure the Scribo.lib can be found by the linker
#ifdef _X64
#pragma comment (lib, "Scribo64.lib")
#else
#pragma comment (lib, "Scribo.lib")
#endif

/* implementation of the NXP_I2C API on Windows */
#include "NXP_I2C.h"
#include "dbgprint.h"
#include <stdio.h>
#include <assert.h>

int lxWindows_verbose = 0;

void lxWindowsVerbose(int level)
{
    lxWindows_verbose = level;
}
/*
    called during target device registry
     this function formerly was in NXP_I2C_Windows.c
*/
int lxWindowsInit(char *this)
{
#define bufSz 1024
    char buffer[bufSz];
    unsigned char ma, mi, bma, bmi;
    unsigned short sz = bufSz;
    unsigned long busFreq;

    this = 0; /* Remove unreferenced formal parameter warning */
    //NXP_I2C_Error_t error = NXP_I2C_NoInterfaceFound;

    //Show some info about Scribo client and server
    if (ClientVersion(&ma, &mi, &bma, &bmi)) {
        if (lxWindows_verbose)
            PRINT("Scribo client version: %d.%d.%d.%d\r\n", ma, mi, bma, bmi);
    } else {
        PRINT("Failed to get Scribo client version\r\n");
    }

    if (ServerVersion(&ma, &mi, &bma, &bmi)) {
        if (lxWindows_verbose)
            PRINT("Scribo server version: %d.%d.%d.%d\r\n", ma, mi, bma, bmi);
    } else {
        PRINT("Failed to get Scribo server version\r\n");
    }

    //If Scribo isn't connected yet, try to connect
    if (!ConnectionValid()) Connect();

    if (ConnectionValid())
    {
        //We are connected, so print some info
        if (VersionStr(buffer, &sz)) {
            //just to be safe
            if (sz < bufSz) buffer[sz] = '\0';
            if (lxWindows_verbose)
                printf_s("Remote target: %s\r\n", buffer);
        }
        else {
            PRINT("Remote target didn't return version string\r\n");
        }

        busFreq = GetSpeed();
        if (busFreq > 0) {
            if ( lxWindows_verbose)
                PRINT("I2C bus frequency: %d\r\n", busFreq);
        }
        else {
            PRINT("I2C bus frequency unknown\r\n");
        }
    }

    return ConnectionValid();
}

// TODO fix duplication from NXP_I2C
typedef struct
{
  char msg[32];
} NXP_I2C_ErrorStr_t;

static const NXP_I2C_ErrorStr_t errorStr[NXP_I2C_ErrorMaxValue] =
{
  { "UnassignedErrorCode" },
  { "Ok" },
  { "NoAck" },
  { "SclStuckAtOne" },
  { "SdaStuckAtOne" },
  { "SclStuckAtZero" },
  { "SdaStuckAtZero" },
  { "TimeOut" },
  { "ArbLost" },
  { "NoInit" },
  { "Disabled" },
  { "UnsupportedValue" },
  { "UnsupportedType" },
  { "NoInterfaceFound" },
  { "NoPortnumber" }
};

static NXP_I2C_Error_t translate_error(uint32_t error)
{
  NXP_I2C_Error_t retval;

  switch (error)
  {
    case eNone: retval=NXP_I2C_Ok;
      break;
    case eI2C_DATA_NACK:
    case eI2C_SLA_NACK: retval=NXP_I2C_NoAck;
      break;
    case eI2C_UNSPECIFIED: retval=NXP_I2C_ArbLost;
      break;
    case eNoResponse: retval=NXP_I2C_TimeOut;
      break;
    case eNoService: retval=NXP_I2C_NoInit;
      break;
   case eInvalidPinMode: retval=NXP_I2C_UnsupportedValue;
      break;
    case eNotAnInteger: retval=NXP_I2C_UnsupportedType;
      break;
    case eBufferOverRun: retval=NXP_I2C_BufferOverRun;
        break;
    case eNoConnect: retval=NXP_I2C_NoInterfaceFound;
      break;
    case eInvalidPinNumber: retval=NXP_I2C_NoPortnumber;
      break;
    default:
      /* error without appropriate code */
         retval=NXP_I2C_UnassignedErrorCode;
  }

  if (error != eNone)
    {
    PRINT_ERROR("I2C error %d (%s)\n", error, (char*)&errorStr[retval].msg);
    }
  return retval;
}

int lxWindowsWrite(int fd, int num_write_bytes, unsigned char *buffer, unsigned int *pError)
{
    unsigned char sla = buffer[0];
    unsigned char *data = &buffer[1];
    fd = 0; /* Remove unreferenced formal parameter warning */

    num_write_bytes--;

    *pError = NXP_I2C_Ok;
    if (!Write(sla >> 1, data, ((unsigned short)num_write_bytes & 0xFFFF))) {
            *pError = LastError();
            return 0;
    }

    return num_write_bytes;
}

int lxWindowsWriteRead(int fd, int NrOfWriteBytes, const uint8_t *WriteData,
             int NrOfReadBytes, uint8_t *ReadData, uint32_t *pError)
{
    uint8_t sla = (uint8_t)WriteData[0];
    uint8_t *write_data = (uint8_t*)&WriteData[1];
    uint16_t rCnt = ((uint16_t)NrOfReadBytes-1);
    fd = 0; /* Remove unreferenced formal parameter warning */

    *pError = NXP_I2C_Ok;

    if (!WriteRead(sla >> 1, write_data, ((unsigned short)NrOfWriteBytes-1), &ReadData[1], &rCnt)) {
            *pError = LastError();
            return 0;
    }

    return rCnt + 1;
}

int lxWindowsVersion(char *buffer, int fd)
{
        unsigned short sz = bufSz;

        VersionStr(buffer, &sz);

        return sz;
}