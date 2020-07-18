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



#ifndef NXPTFA98XX_H_
#define NXPTFA98XX_H_

#ifdef __ANDROID__
#include <utils/Log.h>
#else
#define LOGV if (0/*tfa98xx_verbose*/) printf //TODO improve logging
#endif

#if defined(WIN32) || defined(x64)
//suppress warnings for unsafe string routines.
#pragma warning(disable : 4996)
#endif

#include "Tfa98xx.h"
#include "Tfa98API.h"
#include "Tfa98xx_internals.h"
#include "../../../tfa98xx_cust.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define NXPTFA_APP_REV_MAJOR     1               // major API rev
#define NXPTFA_APP_REV_MINOR     4               // minor
#define NXPTFA_APP_REV_REVISION  1                // revision

/**
 * Type containing all the possible errors that can be returned
 *
 */
typedef enum tfa_srv_api_error {
    // Error code value mapped on TFA API
    tfa_srv_api_error_Ok = 0,
    tfa_srv_api_error_DSP_not_running, // dsp is not running
    tfa_srv_api_error_BadParam,  // wrong parameter
    // Error code specific to srv
    tfa_srv_api_error_Fail,        // generic failure, avoid mislead message
    tfa_srv_api_error_NoClock,   // no clock detected
    tfa_srv_api_error_LowAPI,    // error returned from lower API level
    tfa_srv_api_error_AmpOn,     // amp is still running
    tfa_srv_api_error_Not_Supported, // feature is not supported
    tfa_srv_api_error_File_Write, // file not written properly
    tfa_srv_api_error_Other
} tfa_srv_api_error_t;

/*
 * profiles & volumesteps
 *
 */
#define TFA_MAX_FILENAME        (64) // clip filename if longer
#define TFA_MAX_PROFILES        (3)
#define TFA_MAX_VSTEPS          (16)
#define TFA_FILE_MAX_VSTEPS     (64)
#define TFA_ISMUTE(v)           ((v)>=TFA_MAX_VSTEPS)   // mute if vstep is bigger than max step

/** the default target slave address */
//#define TFA_I2CSLAVEBASE        (0x36)              // tfa device slave address of 1st (=left) device

#define TFA_PROFILE0_SAMPLERATE    (44100) //TODO remove when cnt is active

extern unsigned char  tfa98xxI2cSlave; // global for i2c access
void tfa98_I2cSlave(char level);

#define TFA98XX_EQ_LENGTH                 (10*128) // TODO properly implement eq ;=max10 lines

#pragma pack (push, 1) //TODO double check alignments

typedef struct nxpTfa98xxVolumeStep {
    float attenuation;              // IEEE single float
    unsigned char CF[TFA98XX_PRESET_LENGTH];
} nxpTfa98xxVolumeStep_t;

#define NXPTFA_MAXLINE              (256)       // maximum string length

#define TFA_VSTEP_ID         "VP1_00"
#define TFA_MAXPATCH_LENGTH (40*1024) // (3*1024)    // TODO move this into low API

#define TFA98XX_MAXTAG              (138)        // TODO move to Tfa98xx api


typedef struct nxpTfa98xxVolumeStepFile {
    char id[6];                     // 6 bytes ID = "VP1_00"
    unsigned short size;            // 16 bit integer data size (not including header)
    char crc[4];              //  CRC value for data after header. Uses 0xEDB88320 as polynomial (same as zip). TODO assure 32bits int
    //
    nxpTfa98xxVolumeStep_t vstep[TFA_FILE_MAX_VSTEPS];
} nxpTfa98xxVolumeStepFile_t;
#pragma pack (pop)

typedef struct nxpTfa98xxProfile {
    int valid;
    int i2sRoute;   // TODO define routing
    int ipll;
    int sampleRate;
    int vsteps;     // total amount of vsteps in this profile
    char filename[TFA_MAX_FILENAME];
    //TODO more profile params?
    nxpTfa98xxVolumeStep_t vstep[TFA_MAX_VSTEPS];
    // TODO add eq/biquad floats
    char EqValid;
    char Eqfilename[TFA_MAX_FILENAME];
    const char EQ[TFA98XX_EQ_LENGTH];
} nxpTfa98xxProfile_t;


/*
 * the DSP parameters that are loaded after poweron
 */
typedef struct nxpTfa98xxParameters {
    int patchLength;
    char patchFile[TFA_MAX_FILENAME];
    const unsigned char patchBuffer[TFA_MAXPATCH_LENGTH];
    int speakerLength;
    char speakerFile[TFA_MAX_FILENAME];
    unsigned char speakerBuffer[TFA98XX_SPEAKERPARAMETER_LENGTH];
    int configLength;
    char configFile[TFA_MAX_FILENAME];
    unsigned char configBuffer[TFA98XX_CONFIG_LENGTH];
    // profiles
    nxpTfa98xxProfile_t profile[TFA_MAX_PROFILES];

} nxpTfa98xxParameters_t;

/*
 * the DSP parameters that are loaded after poweron
 */
typedef struct nxpTfa98xxGetParameters {
    int patchLength;
    unsigned char patchBuffer[TFA_MAXPATCH_LENGTH];
    int speakerLength;
    unsigned char speakerBuffer[TFA98XX_SPEAKERPARAMETER_LENGTH];
    int configLength;
    unsigned char configBuffer[TFA98XX_CONFIG_LENGTH];
    // profiles
    nxpTfa98xxProfile_t profile[TFA_MAX_PROFILES];

} nxpTfa98xxGetParameters_t;
/*
 * buffer types for setting parameters
 */
typedef enum nxpTfa98xxParamsType {
    tfa_patch_params,
    tfa_speaker_params,
    tfa_preset_params,
    tfa_config_params,
    tfa_equalizer_params,
    tfa_drc_params,
    tfa_vstep_params,
    tfa_cnt_params,
    tfa_msg_params,
//    tfa_mdrc_params,
    tfa_no_params
} nxpTfa98xxParamsType_t;

/*
 * nxpTfa98xx API calls
 */
tfa_srv_api_error_t nxpTfa98xxStoreParameters(Tfa98xx_handle_t *handlesIn, nxpTfa98xxParamsType_t params, void *data, int length); // provides all loadable parameters
tfa_srv_api_error_t nxpTfa98xxStoreProfile(int profile, void *data, int length); // provide profile
tfa_srv_api_error_t nxpTfa98xxSetVolume(Tfa98xx_handle_t *handlesIn, int profile, int step); // apply volume step, or mute

int nxpTfa98xxGetVolume(Tfa98xx_handle_t *handlesIn, float *getVol); /* get volume for selected idx */
int tfa98xxSaveParamsFile(Tfa98xx_handle_t *handlesIn, char *filename );
int tfa98xxLoadParamsFile(Tfa98xx_handle_t *handlesIn, char *filename );
int nxpTfa98xxBypassDSP(Tfa98xx_handle_t *handlesIn);
int nxpTfa98xxUnBypassDSP(Tfa98xx_handle_t *handlesIn);
/* select device for testing and individual addressing functions */
int nxpTfa98xxSetIdx( int idxIn ); /* default = left */
/* notify the Tfa98xx of the sample rate of the I2S bus that will be used.
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
tfa_srv_api_error_t nxpTfa98xxSetSampleRate(Tfa98xx_handle_t *handlesIn, int rate);

/* read the Tfa98xx of the sample rate of the I2S bus that will be used.
 * @param pRate pointer to rate in Hz. Will be one of 32000, 44100 or 48000 if successful
 */
tfa_srv_api_error_t nxpTfa98xxGetSampleRate(Tfa98xx_handle_t *handlesIn, int* pRate);

tfa_srv_api_error_t nxpTfa98xxVersions(Tfa98xx_handle_t *handlesIn, char *strings, int maxlength); // return all version
const char* nxpTfa98xxGetErrorString(tfa_srv_api_error_t error);
int tfa98xxSaveFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params);
int tfa98xxLoadFile(Tfa98xx_handle_t *handlesIn, char *filename, nxpTfa98xxParamsType_t params);
/*
 * return current tCoef , set new value if arg!=0
 */
float tfa98xxTcoef(float tCoef);

int tfa98xxReset( Tfa98xx_handle_t *handlesIn, unsigned char i2cAddrs, int idx ); /* init registers and coldboot dsp */
/*
 *  lower levels generic API
 */
int tfa98xx_trace, tfa98xx_verbose, tfa98xx_quiet;

void tfa_trace(int level);
void tfa_verbose(int level);
void tfa_quiet(int level);

Tfa98xx_Error_t nxpTfa98xx_Open(Tfa98xx_handle_t *pHandle);

/*
 * write all cached parameters to the DSP
 */
Tfa98xx_Error_t tfa98xxLoadParams(Tfa98xx_handle_t handle);
/* load parameter buffer */
int  tfa98xxSetParams(  nxpTfa98xxParamsType_t params, void *data, int length,
                        Tfa98xx_handle_t handle);
/* load parameter multi buffer */
int  tfa98xxSetParamsMultiple( nxpTfa98xxParamsType_t params, void *data, int length,
                        Tfa98xx_handle_t handle[], int handle_cnt);
/* initialize and enable */
int tfa98xxInit( Tfa98xx_handle_t handle);
Tfa98xx_Error_t tfa98xxClose( Tfa98xx_handle_t *handlesIn ); /* close all devices */

/* Get parameter buffer */
tfa_srv_api_error_t tfa98xxDspGetParam( unsigned char module_id, unsigned char param_id,
                                       int num_bytes, unsigned char data[],
                                       Tfa98xx_handle_t *handlesIn    );
/* calibration */
/* wait until calibration impedance is ok */
int tfa98xxWaitCalibration(Tfa98xx_handle_t *handlesIn ); /* 0 return is ok, else timed-out */
/* wait until calibration impedance is ok */
int waitCalibration(Tfa98xx_handle_t handlesIn, int *calibrateDone); /* 0 return is ok, else timed-out */
int tfa98xxGetCalibrationImpedance( FIXEDPT* re0,
                                    Tfa98xx_handle_t *handlesIn, int idx); /* if return==0  DC impedance in re0 */
/* set calibration impedance for selected idx */
int tfa98xxSetCalibrationImpedance( FIXEDPT re0, Tfa98xx_handle_t *handlesIn);

/*
 * DSP mem access
 *
 *  the msb 0xM0000 , M is the DSP mem region
 */
Tfa98xx_Error_t tfa98xx_DspRead( unsigned int offset, Tfa98xx_handle_t *handlesIn, int *value);

/*
 * save dedicated device files. Depends on the file extension
 */
int tfa98xxSaveFileWrapper(Tfa98xx_handle_t *handlesIn, char *filename);

/*
 * tfa98xxGetState of selected idx:
 *      Current Loudspeaker blocked resistance
 *      Current Speaker Temperature value
 *  TODO is this enough ?
 */
int             tfa98xxGetState(float* resistance, float* temperature,
                                Tfa98xx_handle_t *handlesIn);
void            tfa98xxPowerUp(Tfa98xx_handle_t *handlesIn);
void            tfa98xxPowerdown(Tfa98xx_handle_t *handlesIn);
void            tfa98xxI2cSetSlave(unsigned char);
/* r/w for selected idx */
Tfa98xx_Error_t  tfa98xxReadRegister(Tfa98xx_handle_t *handlesIn,
            unsigned char offset, unsigned short *value);
Tfa98xx_Error_t  tfa98xxWriteRegister(unsigned char offset, unsigned short value,
                                       Tfa98xx_handle_t *handlesIn);
void ClearCalMtp(Tfa98xx_handle_t handle);

Tfa98xx_Error_t tfa98xxDspReadEq(Tfa98xx_handle_t handle,
                        int length, unsigned char *pEqBytes);


#if defined(__cplusplus)
}  /* extern "C" */
#endif
#endif /* TFA98XX_H_ */
