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



#define LOG_TAG "tfa89xx"
#define LOG_NDEBUG 0

#include <stdio.h>
#include <assert.h>
#if defined(WIN32) || defined(_X64)
#include <Windows.h>
#elif defined(__REDLIB__)
#else
#include <unistd.h>
#include <libgen.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "dbgprint.h"
#include "NXP_I2C.h"
#include "Tfa98API.h"
#include "Tfa98xx.h"
#include "nxpTfa98xx.h"
#include "tfa98xxCalibration.h"
#include "tfa98xxRuntime.h"
#include "tfaContainer.h"
#include "Tfa98xx_Registers.h"

#define false 0
#define true  1
/*
 * module globals
 */

static   char *paramsName[]={  "patch",
        "speaker",
        "preset",
        "config",
        "equalizer",
        "drc"};
nxpTfa98xxParameters_t tfaParams = {0};  // TODO do we allow tfaParams to be global (needed for speaker file update?

int tfa98xx_trace = 0;
int tfa98xx_verbose = 0;
int tfa98xx_quiet = 0;

#define TRACEIN  if(tfa98xx_trace) PRINT("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xx_trace) PRINT("Leave %s\n", __FUNCTION__);

#define MAXDEV ( (int) (sizeof(handles)/sizeof(Tfa98xx_handle_t)) )
static int idx = 0;         // TODO cleanup for single device

unsigned char  tfa98xxI2cSlave=TFA_I2CSLAVEBASE; // global for i2c access
#define I2C(idx) ((tfa98xxI2cSlave+idx)*2)
#define I2C_CHUNKSIZE (256) // max i2c xfer length
#define HAVE_ATOF 0 //1


#define BIQUAD_COEFF_SIZE       6
#define Tfa98xx_BIQUAD_LENGTH (1+(BIQUAD_COEFF_SIZE*3)) // TODO put this in API def?
static char latest_errorstr[64];

void tfa_trace(int level) {
    tfa98xx_trace = level;
}

void tfa_verbose(int level) {
    tfa98xx_verbose = level;
}

void tfa_quiet(int level) {
    tfa98xx_quiet = level;
}

void tfa98_I2cSlave(char level) {
    tfa98xxI2cSlave = level;
}

Tfa98xx_Error_t
nxpTfa98xx_Open(Tfa98xx_handle_t *pHandle)
{
    return Tfa98xx_Open(tfa98xxI2cSlave*2, pHandle);
}

const char* nxpTfa98xxGetErrorString(tfa_srv_api_error_t error) {
    const char* pErrStr;

    switch (error) {
    case tfa_srv_api_error_Ok:
        pErrStr = "Ok";
        break;
    case tfa_srv_api_error_Fail:
        pErrStr = "generic failure";
        break;
    case tfa_srv_api_error_NoClock:
        pErrStr = "No I2S Clock";
        break;
    case tfa_srv_api_error_LowAPI:
        pErrStr = "error returned from lower API level";
        break;
    case tfa_srv_api_error_BadParam:
        pErrStr = "wrong parameter";
        break;
    case tfa_srv_api_error_AmpOn:
        pErrStr = "amp is still running";
        break;
    case tfa_srv_api_error_DSP_not_running:
        pErrStr = "DSP is not running";
        break;

    default:
        sprintf(latest_errorstr, "Unspecified error (%d)", (int) error);
        pErrStr = latest_errorstr;
    }
    return pErrStr;
}

/*
 *  int registers and coldboot dsp
 */
int tfa98xxReset( Tfa98xx_handle_t *handlesIn, unsigned char i2cAddrs, int idx )
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    TRACEIN

    if( handlesIn[idx] == -1 )
    {
        err = Tfa98xx_Open(i2cAddrs*2, &handlesIn[idx] );
        PRINT_ASSERT( err) ;
    }

    if( handlesIn[idx] != -1 )
    {
        err = tfaRunStartup(handlesIn[idx]);
        PRINT_ASSERT(err);
        if (err)
            return err;

        /* force cold boot */
        err = tfaRunColdboot(handlesIn[idx], 1); // set ACS
        PRINT_ASSERT(err);
    }
    /* reset all i2C registers to default */
#if !(defined(WIN32) || defined(_X64))
    err = tfa98xx_write_register16(handlesIn[idx], TFA98XX_SYS_CTRL,
                    TFA98XX_SYS_CTRL_I2CR_MSK);
#else
    err = Tfa98xx_WriteRegister16(handlesIn[idx], TFA98XX_SYS_CTRL,
                    TFA98XX_SYS_CTRL_I2CR_MSK);
#endif
    PRINT_ASSERT(err);

    TRACEOUT

    return err;
}

/*
 * close all devices
 */
Tfa98xx_Error_t tfa98xxClose( Tfa98xx_handle_t *handlesIn )
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Other;
    TRACEIN

    err = Tfa98xx_Close(handlesIn[idx] );
    PRINT_ASSERT( err );
    if (err) return err;

    TRACEOUT
    return err;
}


/*
 * volume control
 *
 *  note that the get only operates on the selected device
 */
int nxpTfa98xxGetVolume( Tfa98xx_handle_t *handlesIn, float *getVol )
{
   Tfa98xx_Error_t err;
   float vol = 0;

   TRACEIN
    if (handlesIn[idx] == -1)
    {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
        PRINT_ASSERT( err);
        if (err) {
            return err;
        }
    }
    // call API
    err = Tfa98xx_GetVolume(handlesIn[idx], &vol);
    PRINT_ASSERT( err);
    if (err) {
        return err;
    }
    vol = vol * 10;
    *getVol = vol;
    TRACEOUT

    return err;
}


/*
 * note that this only operates on the selected device
 */
int tfa98xxGetCalibrationImpedance( FIXEDPT* re0, Tfa98xx_handle_t *handlesIn, int idx)
{
    Tfa98xx_Error_t err;

    TRACEIN

    if (handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
        PRINT_ASSERT( err);
        if ( err ) return err;

    }
    // call API
    err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], re0);
    PRINT_ASSERT( err);
    return err;

    TRACEOUT

    return err;
}

/*
 * Set the re0 in the MTP. The value must be in range [4-10] ohms
 * Note that this only operates on the selected device
 */
int tfa98xxSetCalibrationImpedance( float re0, Tfa98xx_handle_t *handlesIn )
{
    Tfa98xx_Error_t err = tfa_srv_api_error_Not_Supported;
    int re0_xmem = 0;

    unsigned char bytes[3];
    if ( re0 > 10.0 || re0 < 4.0 ) {
        err = tfa_srv_api_error_BadParam;
    } else {
        if (handlesIn[idx] == -1) {
            err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
            PRINT_ASSERT( err);
            if ( err ) return err;
        }
        /* multiply with 2^14 */
        re0_xmem = (int)(re0 * (1<<14));
        /* convert to bytes */
        Tfa98xx_ConvertData2Bytes(1, (int *) &re0_xmem, bytes);
        /* call the TFA layer, which is also expected to reset the DSP */
        err = Tfa98xx_DspSetCalibrationImpedance(handlesIn[idx], bytes);
        if (err == Tfa98xx_Error_Not_Supported)
            err = tfa_srv_api_error_Not_Supported;
    }
    return err;
}



/*
 * select device for testing and individual addressing
 *
 * this is for read/write
 */
int nxpTfa98xxSetIdx( int idxIn )
{
    int err = 0;

    if ( idxIn > 2) {
        PRINT_ERROR("%s Error: index %i bigger then max devices %i\n", __FUNCTION__, idx, 2) ;
        err = 1;
        return err;
    }
    idx = idxIn;

    return err;
}

/*
 * Bypass DSP handling
 */
int nxpTfa98xxBypassDSP(Tfa98xx_handle_t *handlesIn)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;
   unsigned short i2SRead = 0;
   unsigned short sysRead = 0;
   unsigned short sysCtrlRead = 0;

   TRACEIN

   if( handlesIn[idx] == -1)
    {
          err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
          if (err) return err;
   }

    /* power on the sub system */
    err = Tfa98xx_Powerdown(handlesIn[idx], 0);
    PRINT_ASSERT(err);

   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_I2SREG, &i2SRead);
   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_I2S_SEL_REG, &sysRead);
   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_SYS_CTRL, &sysCtrlRead);

   i2SRead &= ~(TFA98XX_I2SREG_CHSA_MSK);
   sysRead &= ~(TFA98XX_I2S_SEL_REG_DCFG_MSK);
   sysRead |= TFA98XX_I2S_SEL_REG_SPKR_MSK;
   sysCtrlRead &= ~(TFA98XX_SYS_CTRL_DCA_MSK);
   sysCtrlRead &= ~(TFA98XX_SYS_CTRL_CFE_MSK);

   /* Set CHSA to bypass DSP */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_I2SREG, i2SRead);
   /* Set DCDC compensation to off and set impedance as 8ohm */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_I2S_SEL_REG, sysRead);
   /* Set DCDC to follower mode and disable Coolflux */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_SYS_CTRL, sysCtrlRead);

   TRACEOUT

   return err;
}

/*
 * Unbypassed the DSP
 */
int nxpTfa98xxUnBypassDSP(Tfa98xx_handle_t *handlesIn)
{
   Tfa98xx_Error_t err = Tfa98xx_Error_Other;
   unsigned short i2SRead = 0;
   unsigned short sysRead = 0;
   unsigned short sysCtrlRead = 0;

   TRACEIN

   if( handlesIn[idx] == -1)
   {
          err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
          if (err) return err;
   }

   /* power on the sub system */
   err = Tfa98xx_Powerdown(handlesIn[idx], 0);
   PRINT_ASSERT(err);

   // basic settings for quickset
   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_I2SREG, &i2SRead);
   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_I2S_SEL_REG, &sysRead);
   err = Tfa98xx_ReadRegister16( handlesIn[idx], TFA98XX_SYS_CTRL, &sysCtrlRead);

   i2SRead |= TFA98XX_I2SREG_CHSA;

#define TFA98XX_I2S_SEL_REG_POR    0x3ec3
   sysRead |= TFA98XX_I2S_SEL_REG_POR;
   sysRead &= ~(TFA98XX_I2S_SEL_REG_SPKR_MSK);
   sysCtrlRead |= TFA98XX_SYS_CTRL_DCA_MSK;
   sysCtrlRead |= TFA98XX_SYS_CTRL_CFE_MSK;

   /* Set CHSA to Unbypass DSP */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_I2SREG, i2SRead );
   /* Set I2S SEL REG to set DCDC compensation to default 100% and
      Set impedance to be defined by DSP */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_I2S_SEL_REG, sysRead);
   /* Set DCDC to active mode and enable Coolflux */
   err = Tfa98xx_WriteRegister16( handlesIn[idx], TFA98XX_SYS_CTRL, sysCtrlRead);

   TRACEOUT

   return err;
}

/*
 * operates only on selected dev
 */
int tfa98xxGetState(float* resistance, float* temperature, Tfa98xx_handle_t *handlesIn )
{
    Tfa98xx_Error_t err;
    Tfa98xx_StateInfo_t info;

    TRACEIN

    if (handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx]);
        PRINT_ASSERT( err);
        if (err) return err;
    }

    err = Tfa98xx_DspGetStateInfo(handlesIn[idx], &info);
    PRINT_ASSERT( err);

    if (err == Tfa98xx_Error_Ok) {
        *resistance = info.Re;
        *temperature = (float)(info.T);
    }
    TRACEOUT

    return err;
}

void tfa98xxI2cSetSlave(unsigned char slave) {
    tfa98xxI2cSlave = slave;
}

/*
 * read a register
 */
Tfa98xx_Error_t tfa98xxReadRegister( Tfa98xx_handle_t *handlesIn, unsigned char offset, unsigned short *value )
{
    Tfa98xx_Error_t error;
    unsigned short val= *value;

    TRACEIN

    if( handlesIn[idx] == -1) {
        error = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PRINT_ASSERT( error);
        if (error) return error;
    }

    error = Tfa98xx_ReadRegister16( handlesIn[idx] , offset, &val);
    *value = val;
    PRINT_ASSERT( error);

    TRACEOUT

    return error;
}
/*
 * read a register
 */
unsigned int tfa98xxReadXmem(unsigned char offset, Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;
    int value;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PRINT_ASSERT( err);
        if (err) return err;

    }
    err = Tfa98xx_DspReadMem(handlesIn[idx] , offset, 1, &value);
    PRINT_ASSERT( err);

    TRACEOUT

    return value & 0xffffff;

}
/*
 * write a register
 */
Tfa98xx_Error_t tfa98xxWriteRegister( unsigned char offset, unsigned short value,
                           Tfa98xx_handle_t *handlesIn   )
{
    Tfa98xx_Error_t err;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PRINT_ASSERT( err);
        if (err) return err;
    }

    err = Tfa98xx_WriteRegister16( handlesIn[idx] , offset, value);
    PRINT_ASSERT( err);

    TRACEOUT

    return err;
}
/*
 * DSP mem access
 *
 *  the msb 0xM0000 , M is the DSP mem region
 */
Tfa98xx_Error_t
tfa98xx_DspRead( unsigned int offset, Tfa98xx_handle_t *handlesIn, int *value)
{
     Tfa98xx_handle_t handle; // TODO cleanup handle shuffle
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
        unsigned short cf_ctrl; /* the value to sent to the CF_CONTROLS register */
        unsigned char bytes[4];
        int idx=0; //TODO I2C
        int dmem = (offset>>16) & 0xf;
        int val = *value;
        int nr_bytes = (dmem == Tfa98xx_DMEM_PMEM) ? 4 : 3;
//        Tfa98xx_DMEM_PMEM=0,
//        Tfa98xx_DMEM_XMEM=1,
//        Tfa98xx_DMEM_YMEM=2,
//        Tfa98xx_DMEM_IOMEM=3
        if( handlesIn[0] == -1) {
            error = Tfa98xx_Open(I2C(idx), &handlesIn[0] );
            assert( error==Tfa98xx_Error_Ok);
        }

        handle = handlesIn[0];
        /* first set DMEM and AIF, leaving other bits intact */
        error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }
        cf_ctrl &= ~0x000E;     /* clear AIF & DMEM */
        cf_ctrl |= (dmem << 1);    /* set DMEM, leave AIF cleared for autoincrement */
        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }

        error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, offset & 0xffff);
        if (error != Tfa98xx_Error_Ok) {
                return error;
        }

        error = Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, nr_bytes, bytes);
        if (error != Tfa98xx_Error_Ok) {
            PRINT("DSP mem read error\n");
                return -1;
        }

        if (dmem == Tfa98xx_DMEM_PMEM) {
            val = (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
        } else {
            Tfa98xx_ConvertBytes2Data(3, bytes, &val);
        }
        *value = val;

        return error;
}

/*
 * get tag
 *
 */
static tfa_srv_api_error_t nxpTfa98xx_GetDspTag(Tfa98xx_handle_t *handlesIn, char *string, int *size)
{
    Tfa98xx_Error_t err87;
    tfa_srv_api_error_t err = tfa_srv_api_error_Ok;
    int i;
    unsigned char tag[TFA98XX_MAXTAG];

    *size = 0;

    // interface should already be opened
    err87 = Tfa98xx_DspGetParam(handlesIn[idx], 1 /*MODULE_SPEAKERBOOST*/ , 0xFF,
            TFA98XX_MAXTAG, tag);

    PRINT_ASSERT( err87);

    if (err87 == Tfa98xx_Error_Ok) {
        // the characters are in every 3rd byte
        for ( i=2 ; i<TFA98XX_MAXTAG ; i+=3)
        {
                if ( isprint(tag[i]) ) {
                    *string++ =  tag[i];    // only printable chars
                    (*size)++;
                }
        }
         *string = '\0';
    }
    if (err87 == Tfa98xx_Error_DSP_not_running)
        return tfa_srv_api_error_DSP_not_running;
    else if (err87 == Tfa98xx_Error_Bad_Parameter)
        return tfa_srv_api_error_BadParam;

    return err;
}

/*
 * return version strings
 */
tfa_srv_api_error_t nxpTfa98xxVersions( Tfa98xx_handle_t *handlesIn, char *strings, int maxlength )
{
    tfa_srv_api_error_t err = tfa_srv_api_error_Ok;
    char str[NXPTFA_MAXLINE],str1[NXPTFA_MAXLINE];
    int length=0, i, totalBufferLength = 0;
    unsigned short reg;

    // API rev
    sprintf(str, "nxpTfa API rev: %d.%d.%d\n", NXPTFA_APP_REV_MAJOR, NXPTFA_APP_REV_MINOR, NXPTFA_APP_REV_REVISION);
    length = (int)(strlen(str));
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

    strcpy(strings, str);
    strings += length;

    // tfa device and hal layer rev
    {
        int M,m,mR,H,h,hR;
        tfa98xx_rev(&M, &m, &mR);
        NXP_I2C_rev(&H, &h, &hR);
        sprintf(str, "Tfa98xx API rev: %d.%d.%d\n" "Tfa98xx HAL rev: %d.%d.%d\n", M, m, mR, H, h, hR);
    }

    length = (int)(strlen(str));
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

    strcpy(strings, str);
    strings += length;
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

    // chip rev
    err = tfa98xxReadRegister(handlesIn, 0x03, &reg); //TODO define a rev somewhere
    PRINT_ASSERT(err);
    sprintf(str, "Tfa98xx HW  rev: 0x%04x\n", reg);

    length = (int)(strlen(str));
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

    strcpy(strings, str);
    strings += length;
    length  += length;
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

        // coolflux ROM rev
        err = nxpTfa98xx_GetDspTag( handlesIn, str, &i);
        if ( err != tfa_srv_api_error_Ok)
                return err;
        length += i;
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

        sprintf(str1, "DSP revstring: \"%s\"\n", str);
        strcpy(strings, str1);
        strings += strlen(str1);
        length  += (int)(strlen(str1));
        totalBufferLength += length;

    if (  totalBufferLength > maxlength ) {
            PRINT_ERROR("Buffer size too short \n");
                return tfa_srv_api_error_BadParam; // max length too short
        }

    *strings = '\0';

        return err;
}

/*
 * read DSP parameters
 */
tfa_srv_api_error_t tfa98xxDspGetParam( unsigned char module_id, unsigned char param_id,
                                       int num_bytes, unsigned char data[],
                                       Tfa98xx_handle_t *handlesIn    )
{
    Tfa98xx_Error_t err87;
    tfa_srv_api_error_t err = tfa_srv_api_error_Ok;

    TRACEIN

    if( handlesIn[idx] == -1) {
        err87 = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PRINT_ASSERT( err87);
        if (err87) return err87;
    }

    err87 = Tfa98xx_DspGetParam(handlesIn[idx], module_id, param_id, num_bytes, data );
    PRINT_ASSERT( err87);

    if (err87 ==  Tfa98xx_Error_DSP_not_running)
        err = tfa_srv_api_error_DSP_not_running;
    else if (err87 == Tfa98xx_Error_Bad_Parameter)
        err = tfa_srv_api_error_BadParam;

    TRACEOUT

    return err;

}
/*
 * file type names for enum type nxpTfa98xxParamsType for user-friendly messages
 */
static const char *filetypeName[] = {
        "patch file" ,
        "speaker file" ,
        "preset file" ,
        "config file" ,
        "equalizer file" ,
        "drc file" ,
        "vstep file" ,
        "unknown file"
};
/*
 * save dedicated device files. Depends on the file extension
 */
int tfa98xxSaveFileWrapper(Tfa98xx_handle_t *handlesIn, char *filename)
{
    char *ext;
    nxpTfa98xxParamsType_t ftype = tfa_no_params;

    // get filename extension
    ext = strrchr(filename, '.'); // point to filename extension

    if ( ext == NULL ) {
        PRINT("Cannot find file %s type requested.\n" , filename);
        PRINT("Example:<filename>.eq.bin or <filename>.speaker.bin.\n");
        return Tfa98xx_Error_Other; // no '.'
    } else {
        if ( strcmp(ext, ".bin")==0 )
        {
            /* now look for supported type */
            /* todo: make it case-insensitive */
            if ( strstr(filename, ".patch.bin") != NULL ) {
                ftype = tfa_patch_params;
            }
            else if ( strstr(filename, ".speaker.bin") != NULL ) {
                ftype = tfa_speaker_params;
            }
            else if ( strstr(filename, ".preset.bin") != NULL ) {
                ftype = tfa_preset_params;
            }
            else if ( strstr(filename, ".config.bin") != NULL ) {
                ftype = tfa_config_params;
            }
            else if ( strstr(filename, ".eq.bin") != NULL ) {
                ftype = tfa_equalizer_params;
            }
            else {
                ftype = tfa_no_params;
            }
        } else {
            PRINT("Cannot find file %s type requested.\n" , filename);
            PRINT("Example:<filename>.eq.bin or <filename>.speaker.bin.\n");
            return Tfa98xx_Error_Other; // no '.bin'
        }
    }

    if ( ftype != tfa_no_params )
    {
        ftype = tfa98xxSaveFile( handlesIn, filename, ftype);
        return ftype;
    }
    else
        return Tfa98xx_Error_Other;

}
/*
 * save dedicated params
 */
int tfa98xxSaveFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params)
{
    FILE *f;
    unsigned char buffer[TFA98XX_SPEAKERPARAMETER_LENGTH*2]; //TODO use malloc or move
    unsigned char eqbuffer[60*3]; //TODO use malloc or move
    int c=0, i=0, test;
    int fileLength = 0;
    int nxpTfaCurrentProfile = tfa98xx_get_profile();
    Tfa98xx_Error_t err;

    if( handlesIn[idx] == -1)
    {
        err = Tfa98xx_Open(I2C(idx), &handlesIn[idx] );
        PRINT_ASSERT( err);
        if (err) return err;
    }

    /*
    * call the specific setter functions
    *  rely on the API for error checking
    */
    switch ( params )
    {
        case tfa_patch_params:
            PRINT("Patch file cannot be saved from the device.\n");
            break;
        case tfa_speaker_params:
            err = Tfa98xx_DspReadSpeakerParameters(handlesIn[idx], TFA98XX_SPEAKERPARAMETER_LENGTH, buffer);
            if (err)
            {
            PRINT("DSP error\n");
            return err;
            }
            PRINT_ASSERT(err);
            f = fopen( filename, "wb");
            if (!f)
            {
                PRINT("Unable to open %s\n", filename);
                err = Tfa98xx_Error_Other;
                return err;
            }
            c = (int)(fwrite( buffer, TFA98XX_SPEAKERPARAMETER_LENGTH , 1, f ));
            fclose(f);
            break;
        case tfa_config_params:
            err = tfa98xx_dsp_config_parameter_count(handlesIn[idx], &fileLength);
            fileLength *= 3; // 3bytes

            err = Tfa98xx_DspReadConfig(handlesIn[idx], fileLength, buffer);
            if (err)
            {
                PRINT("DSP error\n");
                return err;
            }
            PRINT_ASSERT(err);
            f = fopen( filename, "wb");
            if (!f)
            {
                PRINT("Unable to open %s\n", filename);
                err = Tfa98xx_Error_Other;
                return err;
            }
            c = (int)(fwrite( buffer, fileLength , 1, f ));
            fclose(f);
            break;
        case tfa_preset_params:
            err = tfa98xx_dsp_config_parameter_count(handlesIn[idx], &fileLength);
            fileLength *= 3; // 3bytes
            err = Tfa98xx_DspReadPreset(  handlesIn[idx],
                    fileLength+TFA98XX_PRESET_LENGTH, buffer);
            if (err)
            {
                PRINT("DSP error\n");
                return err;
            }
            PRINT_ASSERT(err);
            f = fopen( filename, "wb");
            if (!f)
            {
                PRINT("Unable to open %s\n", filename);
                err = Tfa98xx_Error_Other;
                return err;
            }
            c = (int)(fwrite( buffer, TFA98XX_PRESET_LENGTH , 1, f ));
            fclose(f);
            break;
        case tfa_equalizer_params:
            err = tfa98xxDspReadEq( handlesIn[idx], 60, (void*)&eqbuffer );
            if (err)
            {
                PRINT("DSP error\n");
                return err;
            }

            PRINT_ASSERT(err);
            f = fopen( filename, "wb");
            if (!f)
            {
                PRINT("Unable to open %s\n", filename);
                err = Tfa98xx_Error_Other;
                return err;
            }
            c = (int)(fwrite( (void*)&eqbuffer, 60*3 , 1, f ));
            fclose(f);
            break;
        default:
            PRINT_ERROR("%s Error: bad parameter:%d\n", __FUNCTION__, params) ;
            break;
    }

    if (c != 1)
    {
        PRINT("Unable to handle the file %s\n", filename);
        err = Tfa98xx_Error_Other;
        return err;
    }

   return Tfa98xx_Error_Ok;
}

/* load the EQ parameters from the device and save it to a file */
Tfa98xx_Error_t tfa98xxDspReadEq(Tfa98xx_handle_t handle,
                                    int length, unsigned char *pEqBytes)
{
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

    int i = 0;
    int input[3];
    unsigned char input_bytes[3 * 3];
    /* EQ by 60 words*/
    unsigned char output_bytes[60 * 3] = {0};

    /* Read first part of DRC, limited by 80 words */
    input[0] = Tfa98xx_DMEM_YMEM;

    input[1] = 361;

    input[2] = 60;        /* EQ contents are 60 bytes */

    Tfa98xx_ConvertData2Bytes(3, input, input_bytes);

    error =
    Tfa98xx_DspExecuteRpc(handle, 0 /* moduleId */ , 5 /* paramId */ ,
                  sizeof(input_bytes), input_bytes,
                  length * 3, output_bytes);

    /* Assigning to output EQ buffer */
    for (i=0; i<(length*3); i++)
    {
        pEqBytes[i] = output_bytes[i];
    }

    return error;
}


#if !(defined(WIN32) || defined(_X64))

/*
 * dump the params into files
 */
static int outFile(int length, void *buffer, char *basename, char *ext) {

  char outfile[80];
  FILE *out;

  strncpy(outfile, basename, sizeof(outfile));
  strcat(outfile, ext);
  out = fopen( outfile, "wb");
  if(out==0) {
    PRINT("%s: can't open:%s\n", __FUNCTION__, outfile);
    return 0;
  }
  if ( length ) {
      fwrite(buffer, length, 1, out);
      fclose(out);
      PRINT("created %s (%d bytes)\n", outfile,
          length);
  }    \
  return length;
}
#endif
