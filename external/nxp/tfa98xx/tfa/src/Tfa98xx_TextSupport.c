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
#include "Tfa98xx.h"

/* support for error code translation into text */

static char latest_errorstr[64];

const char* tfa98xx_get_error_string(enum Tfa98xx_Error error)
{
  const char* pErrStr;

  switch (error)
  {
  case Tfa98xx_Error_Ok:
    pErrStr = "Ok";
    break;
  case Tfa98xx_Error_DSP_not_running:
    pErrStr = "DSP_not_running";
    break;
  case Tfa98xx_Error_Bad_Parameter:
    pErrStr = "Bad_Parameter";
    break;
  case Tfa98xx_Error_NotOpen:
    pErrStr = "NotOpen";
    break;
  case Tfa98xx_Error_OutOfHandles:
    pErrStr = "OutOfHandles";
    break;
  case Tfa98xx_Error_RpcBusy:
    pErrStr = "RpcBusy";
    break;
  case Tfa98xx_Error_RpcModId:
    pErrStr = "RpcModId";
    break;
  case Tfa98xx_Error_RpcParamId:
    pErrStr = "RpcParamId";
    break;
  case Tfa98xx_Error_RpcInfoId:
    pErrStr = "RpcInfoId";
    break;
  case Tfa98xx_Error_RpcNotAllowedSpeaker:
    pErrStr = "RpcNotAllowedSpeaker";
    break;
  case Tfa98xx_Error_Not_Supported:
    pErrStr = "Not_Supported";
    break;
  case Tfa98xx_Error_I2C_Fatal:
    pErrStr = "I2C_Fatal";
    break;
  case Tfa98xx_Error_I2C_NonFatal:
    pErrStr = "I2C_NonFatal";
    break;
  case Tfa98xx_Error_StateTimedOut:
    pErrStr = "WaitForState_TimedOut";
    break;
  default:
    sprintf(latest_errorstr, "Unspecified error (%d)", (int)error);
    pErrStr = latest_errorstr;
  }
  return pErrStr;
}
