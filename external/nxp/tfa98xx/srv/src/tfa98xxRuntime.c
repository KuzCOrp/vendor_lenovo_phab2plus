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
#if defined(WIN32) || defined(_X64)
#include <Windows.h>
#elif defined(__REDLIB__)
#else
#include <unistd.h>
#include <libgen.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dbgprint.h"
#include "Tfa98API.h"
#include "nxpTfa98xx.h"
#include "tfa98xxRuntime.h"
#include "Tfa98xx_Registers.h"
#include "tfaContainer.h"
#include "tfaOsal.h"

// retry values
#define AREFS_TRIES 100
#define CFSTABLE_TRIES 100

extern unsigned char  tfa98xxI2cSlave;  // global for i2c access
#define I2C(idx) ((tfa98xxI2cSlave+idx)*2)

int tfa98xxRun_trace=0;
#define TRACEIN  if(tfa98xxRun_trace) PRINT("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxRun_trace) PRINT("Leave %s\n", __FUNCTION__);

#if !(defined(WIN32) || defined(_X64) || defined(__REDLIB__))
/************************
 * time measurement
 */
#include <sys/time.h>
#include <sys/resource.h>

typedef struct tag_time_measure
{
  struct timeval startTimeVal;
  struct timeval stopTimeVal;

  struct rusage startTimeUsage;
  struct rusage stopTimeUsage;
} time_measure;

time_measure * startTimeMeasuring()
{
  time_measure * tu = malloc(sizeof(time_measure));
  if(!tu)
    exit(1);

  getrusage(RUSAGE_SELF, &tu->startTimeUsage);
  gettimeofday(&tu->startTimeVal,0);

  return tu;
}

void stopTimeMeasuring(time_measure * tu)
{
  getrusage(RUSAGE_SELF, &tu->stopTimeUsage);
  gettimeofday(&tu->stopTimeVal,0);
}

void printMeasuredTime(time_measure * tu)
{
  struct timeval elapsedVal;
  struct timeval userVal;
  struct timeval systemVal;

  double elapsed_millis = 0.0f;
  double user_millis = 0.0f;
  double system_millis = 0.0f;

  timersub(&tu->stopTimeVal, &tu->startTimeVal, &elapsedVal);
  timersub(&tu->stopTimeUsage.ru_utime, &tu->startTimeUsage.ru_utime, &userVal);
  timersub(&tu->stopTimeUsage.ru_stime, &tu->startTimeUsage.ru_stime, &systemVal);

  elapsed_millis = elapsedVal.tv_sec * 1000 + (double) elapsedVal.tv_usec / 1000;
  user_millis = userVal.tv_sec * 1000 + (double) userVal.tv_usec / 1000;
  system_millis = systemVal.tv_sec * 1000 + (double) systemVal.tv_usec / 1000;

  PRINT("-execution times [ms]:");
  PRINT("Total: %f, User: %f,System: %f\n", elapsed_millis, user_millis, system_millis);
}
#endif
/*
 *
 */
// global param cache
extern nxpTfa98xxParameters_t tfaParams;
/*
 * accounting globals
 */
int gTfaRun_useconds=0;
/*
 * sleep requested amount of micro seconds
 */
#ifdef tfaRun_Sleepus
void _tfaRun_Sleepus(int us)
#else
void tfaRun_Sleepus(int us)
#endif
{
#if (defined(WIN32) || defined(_X64))
    int rest;

    rest = us%1000;
    if (rest)
        us += 1000; // round up for windows TODO check usec in windows
    Sleep(us/1000); // is usinterval
#else
    usleep(us); // is usinterval
#endif
    gTfaRun_useconds += us;
}

static int nxpTfaCurrentVolstep=0;
static int nxpTfaCurrentProfile=-1;
void TfaCurrentProfile(int level) {
    nxpTfaCurrentProfile = level;
}
int tfa98xx_set_profile(int level) {
    int old = nxpTfaCurrentProfile;

    nxpTfaCurrentProfile = level;
    return old;
}
int tfa98xx_get_profile(void) {
    return nxpTfaCurrentProfile;
}

int tfa98xx_set_vstep(int level) {
    int old = nxpTfaCurrentVolstep;

    nxpTfaCurrentVolstep = level;
    return old;
}
int tfa98xx_get_vstep(void) {
    return nxpTfaCurrentVolstep;
}

void i2c_writes(int level) {
    gNXP_i2c_writes = level;
}

void i2c_reads(int level) {
    gNXP_i2c_reads = level;
}

void Run_useconds(int level) {
    gTfaRun_useconds = level;
}

void Run_i2c_writes(int level) {
    gTfRun_i2c_writes = level;
}

void Run_i2c_reads(int level) {
    gTfRun_i2c_reads = level;
}

/*
 * trace wrapper for tfaRun_Sleepus
 */
void tfaRun_SleepusTrace(int us, char *file, int line){
    if (gTfaRun_timingVerbose)
        PRINT("sleep %d us @%s:%d\n", us, file, line);
    _tfaRun_Sleepus(us);
}
/*
 * powerup the coolflux subsystem and wait for it
 */
Tfa98xx_Error_t tfaRunCfPowerup(Tfa98xx_handle_t handle) {
    Tfa98xx_Error_t err;
    int tries, status;

    /* power on the sub system */
    err = Tfa98xx_Powerdown(handle, 0);
    PRINT_ASSERT(err);
    // wait until everything is stable, in case clock has been off
    if (tfa98xx_runtime_verbose)
        PRINT("Waiting for DSP system stable...\n");
    for ( tries=CFSTABLE_TRIES; tries > 0; tries-- ) {
        err = Tfa98xx_DspSystemStable(handle, &status);
        assert(err == Tfa98xx_Error_Ok);
        if ( status )
            break;
    }
    if (tries==0) {// timedout
        PRINT("DSP subsystem start timed out\n");
        return Tfa98xx_Error_StateTimedOut;
    }

    return err;
}
/*
 * start the maximus speakerboost algorithm
 *  this implies a full system startup when the system was not already started
 *
 */
Tfa98xx_Error_t tfaRunSpeakerBoost(Tfa98xx_handle_t handle, int force)
{
    Tfa98xx_Error_t err;

    if ( force ) {
        err= tfaRunColdStartup(handle);
        if ( err ) return err;
        // DSP is running now
    }

    if ( tfaRunIsCold(handle)) {
        int calibrateDone;

        PRINT_ERROR("coldstart%s\n", force? " (forced)":"");

        if ( !force ) { // in case of force CF already runnning
        err = tfaRunStartup(handle);
        PRINT_ASSERT(err);
        if ( err )
            return err;

        err = tfaRunStartDSP(handle);
        PRINT_ASSERT(err);
        if ( err )
            return err;
        }
        // DSP is running now
        //
        // NOTE that ACS may be active
        //  no DSP reset/sample rate may be done until configured (SBSL)

        // soft mute
        err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Digital);
        PRINT_ASSERT(err);
        if ( err )
            return err;

        // For the first configuration the DSP expects at least
        // the speaker, config and a preset.
        // Therefore all files from the device list as well as the file
        // from the default profile are loaded before SBSL is set.
        //
        // Note that the register settings were already done before loading the patch
        //
        // write all the files from the device list (typically spk and config)
        err = tfaContWriteFiles(handle);
        if (err)
            return err;

        // write all the files from the profile list (typically preset)
        err = tfaContWriteFilesProf(handle, tfa98xx_get_profile(), 0); // use volumestep 0
        PRINT_ASSERT(err);
        if (err != Tfa98xx_Error_Ok)
        {
            return err;
        }

        // tell DSP it's loaded
        err = Tfa98xx_SetConfigured(handle);
        PRINT_ASSERT(err);
        if ( err )
            return err;

        // await calibration, this should return ok
        tfa98xxRunWaitCalibration(handle, &calibrateDone);
        if (!calibrateDone) {
            PRINT("Calibration not done!\n");
            return Tfa98xx_Error_StateTimedOut;
        }
    } else { // already warm, so just pwr on
        err = tfaRunCfPowerup(handle);
        PRINT_ASSERT(err);
    }

    return err;
}

/*
 * Set the debug option
 */
void tfaRunVerbose(int level) {
    tfa98xx_runtime_verbose = level;
}

/*
 * verbose enable
 */
void gTfaRunTimingVerbose(int level) {
    gTfaRun_timingVerbose = level;
}

#define TFA98XX_XMEM_CALIBRATION_DONE 231 //0xe7
#define TFA98XX_XMEM_COUNT_BOOT          161 //0xa1
/*
 * Check if calibration is done
 */
int tfaRunIsCaldone(Tfa98xx_handle_t handle) {
    int calibrateDone;

    Tfa98xx_DspReadMem(handle, TFA98XX_XMEM_CALIBRATION_DONE, 1, &calibrateDone);

    return calibrateDone;
}
/*
 *
 */
Tfa98xx_Error_t tfaRunResetCountClear(Tfa98xx_handle_t handle) {

  return Tfa98xx_DspWriteMem(handle,TFA98XX_XMEM_COUNT_BOOT , 0, Tfa98xx_DMEM_XMEM);

}
/*
 *
 */
int tfaRunResetCount(Tfa98xx_handle_t handle) {
    int count;

    Tfa98xx_DspReadMem(handle, TFA98XX_XMEM_COUNT_BOOT, 1, &count);

  return count;

}
/*
 * report if we are in cold state
 *   return true if it's cold booted
 */
int tfaRunIsCold(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;

    /* check status ACS bit to set */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    PRINT_ASSERT(err);
    return (status & TFA98XX_STATUSREG_ACS)!=0;
}
/*
 * report if we are in powerdown state
 *  use AREFS from the status register iso the actual PWDN bit
 *   return true if powered down
 */
int tfaRunIsPwdn(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;

    /* check if PWDN bit is clear by looking at AREFS */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);

    PRINT_ASSERT(err);
    return (status & TFA98XX_STATUSREG_AREFS)==0;
}
/*
 *
 */
int tfaRunIsAmpRunning(Tfa98xx_handle_t handle)
{
    unsigned short status;
    Tfa98xx_Error_t err;

    /* check status SWS bit to set */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    PRINT_ASSERT(err);

    return (status & TFA98XX_STATUSREG_SWS_MSK)!=0;
}
/*
 * wait for a status bit condition
 *  return 0 if set within maxTries
 *          else timedout
 */
int tfaRunStatusWait(Tfa98xx_handle_t handle, unsigned short bitmask,
            int value, int maxTries)
{
    unsigned short status;
    int tries, set=0;
    Tfa98xx_Error_t err;

    for ( tries=1; tries < maxTries; tries++ ) {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        assert(err == Tfa98xx_Error_Ok);
        set = (status & bitmask) != 0; // bit set or clear?
        if ( set  == value ) {
            if (tfa98xx_runtime_verbose) PRINT("%s: %d tries\n",__FUNCTION__, tries);
            return 0;
        }
    }

    return -1;

}
Tfa98xx_Error_t tfaRunColdboot(Tfa98xx_handle_t handle, int state)
{
#define CF_CONTROL 0x8100
    Tfa98xx_Error_t err=Tfa98xx_Error_Ok;
    int tries = 10;

    /* repeat set ACS bit until set as requested */
    while ( state == !tfaRunIsCold(handle) ) {
        /* set colstarted in CF_CONTROL to force ACS */
                err = Tfa98xx_DspWriteMem(handle, CF_CONTROL, state, Tfa98xx_DMEM_IOMEM);
        PRINT_ASSERT(err);

        if (tries-- == 0) {
            PRINT("coldboot (ACS) did not %s\n", state ? "set":"clear");
            return Tfa98xx_Error_Other;
        }
    }

    return err;
}


/*
 * selection input PLL for lock
 *  0 = BCK [default]
 *  1 = WS
 *  */
Tfa98xx_Error_t tfaRunSelectPLLInput( Tfa98xx_handle_t handle, int input )
{
    Tfa98xx_Error_t error;
    unsigned short value;

    error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
    if (error) return error;

    input-=1; // reg

    value &= ~TFA98XX_SYS_CTRL_IPLL;
    value |= (input<<TFA98XX_SYS_CTRL_IPLL_POS);
    error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

    return error;
}
/*
 * set CFE and AMPE
 */
Tfa98xx_Error_t tfaRunEnableCF( Tfa98xx_handle_t handle, int state )
{
    Tfa98xx_Error_t error;
    unsigned short value;

    error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
    if (error) return error;

    value &= ~TFA98XX_SYS_CTRL_AMPE_MSK; //enable amp
    value |= (state<<TFA98XX_SYS_CTRL_AMPE_POS);
    value &= ~TFA98XX_SYS_CTRL_CFE_MSK;
    value |= (state<<TFA98XX_SYS_CTRL_CFE_POS);
    error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);

    return error;

}

/*
 * load the patch if any
 *   else tell no loaded
 */
Tfa98xx_Error_t tfaRunLoadPatch(Tfa98xx_handle_t handle) {

    return tfaContWritePatch(handle);
}
/*
 *  this will load the patch witch will implicitly start the DSP
 *   if no patch is available the DPS is started immediately
 */
Tfa98xx_Error_t tfaRunStartDSP(Tfa98xx_handle_t handle) {
    Tfa98xx_Error_t err;

    err = tfaRunLoadPatch(handle);
    PRINT_ASSERT(err);
    err = tfa98xx_dsp_write_tables(handle);
    PRINT_ASSERT(err);

    return err;
}

/*
 * start the clocks and wait until the AMP is switching
 *  on return the DSP sub system will be ready for loading
 */
Tfa98xx_Error_t tfaRunStartup(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    int tries, status;

    /* load the optimal TFA98XX in HW settings */
    err = Tfa98xx_Init(handle);
    PRINT_ASSERT(err);

    /* I2S settings to define the audio input properties
     *  these must be set before the subsys is up */
    // this will run the list until a non-register item is encountered
    err = tfaContWriteRegsDev(handle); // write device register settings
    PRINT_ASSERT(err);
    // also write register the settings from the default profile
    // NOTE we may still have ACS=1 so we can switch sample rate here
    err = tfaContWriteRegsProf(handle, tfa98xx_get_profile());
    PRINT_ASSERT(err);

    /* power on the sub system */
    err = Tfa98xx_Powerdown(handle, 0);
    PRINT_ASSERT(err);

    /*  powered on
     *    - now it is allowed to access DSP specifics
     * */
    //     * - put DSP in reset
    err = Tfa98xx_DspReset(handle, 1);
    PRINT_ASSERT(err);

    /*  wait until the DSP subsystem hardware is ready
     *    note that the DSP CPU is not running (RST=1) */
    if (tfa98xx_runtime_verbose)
        PRINT("Waiting for DSP system stable...\n");
    for ( tries=1; tries < CFSTABLE_TRIES; tries++ ) {
        err = Tfa98xx_DspSystemStable(handle, &status);
        assert(err == Tfa98xx_Error_Ok);
        if ( status )
            break;
    }
    if (tries == CFSTABLE_TRIES) {
        if (tfa98xx_runtime_verbose) PRINT("Timed out\n");
        return Tfa98xx_Error_StateTimedOut;
    }  else
        if (tfa98xx_runtime_verbose) PRINT(" OK (tries=%d)\n", tries);

    /* the CF subsystem is enabled */

    if (tfa98xx_runtime_verbose) PRINT("reset count:0x%x\n", tfaRunResetCount(handle));

    return err;
}
/*
 * run the startup/init sequence and set ACS bit
 */
Tfa98xx_Error_t tfaRunColdStartup(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;

    err = tfaRunStartup(handle);
    PRINT_ASSERT(err);
    if (err)
        return err;

    /* force cold boot */
    err = tfaRunColdboot(handle, 1); // set ACS
    PRINT_ASSERT(err);
    if (err)
        return err;

    /* start */
    err = tfaRunStartDSP(handle);
    PRINT_ASSERT(err);

    return err;
}
/*
 *
 */
Tfa98xx_Error_t tfaRunMuteAmplifier(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;
   int tries = 0;

    /* signal the TFA98XX to mute plop free and turn off the amplifier */
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }

    /* now wait for the amplifier to turn off */
#if 0
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
    while ( ((status & TFA98XX_STATUSREG_SWS) == TFA98XX_STATUSREG_SWS) && (tries < 1000))
    {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
      tries++;
        if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
    }
   if (tries == 1000)
   {
      /*The amplifier is always switching*/
      return Tfa98xx_Error_Other;
   }
#else
    do {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if (err != Tfa98xx_Error_Ok) {
            ALOGE("FUNC: %s, LINE: %u err = 0x%04x", __func__, __LINE__, err);
            break;
        }
        if ( (status & TFA98XX_STATUSREG_SWS_MSK) == TFA98XX_STATUSREG_SWS_MSK) {
            if (++tries >= TFA98XX_WAITRESULT_NTRIES) {
                break;
            }
            tfaRun_Sleepus(5000);
        } else {
            break;
        }
    } while (tries < TFA98XX_WAITRESULT_NTRIES);

    if (tries == TFA98XX_WAITRESULT_NTRIES) {
        /*The amplifier is always switching*/
        err = Tfa98xx_Error_Other;
    }
    ALOGD("FUNC: %s, LINE: %u status = 0x%04x", __func__, __LINE__, status);
#endif
   return err;
}
/*
 *
 */
Tfa98xx_Error_t tfaRunMute(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;
   int tries = 0;

    /* signal the TFA98XX to mute  */
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Amplifier);
    if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }

    /* now wait for the amplifier to turn off */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
    if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }
    while ( ((status & TFA98XX_STATUSREG_SWS) == TFA98XX_STATUSREG_SWS) && (tries < 1000))
    {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
      tries++;
        if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }
    }

    if ( tfa98xx_runtime_verbose )
        PRINT("-------------------- muted --------------------\n");

   if (tries == 1000)
   {
      /*The amplifier is always switching*/
      return Tfa98xx_Error_Other;
   }

   return err;
}
/*
 *
 */
Tfa98xx_Error_t tfaRunUnmute(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    int tries = 0;

    /* signal the TFA98XX to mute  */
    err = Tfa98xx_SetMute(handle, Tfa98xx_Mute_Off);

    if ( tfa98xx_runtime_verbose )
        PRINT("-------------------unmuted ------------------\n");

    return err;
}

/*
 *
 */
// status register errors to check for not 1
#define TFA98XX_STATUSREG_ERROR1_SET_MSK (  \
        TFA98XX_STATUSREG_OCDS  )
// status register errors to check for not 0
#define TFA98XX_STATUSREG_ERROR1_CLR_MSK ( \
        TFA98XX_STATUSREG_UVDS  |  \
        TFA98XX_STATUSREG_OVDS  |  \
        TFA98XX_STATUSREG_OTDS    )
#define TFA98XX_STATUSREG_ERROR2_SET_MSK (  \
        TFA98XX_STATUSREG_ACS |   \
        TFA98XX_STATUSREG_WDS )

/*
 * return of 0 means ok, else the system needs to be configured
 */
int tfaRunCheckAlgo(Tfa98xx_handle_t handle)
{
    //Tfa98xx_Error_t err;
    static int ampTempCheck=0;// correct value after startup
    //int ampTemp; // newly read value

#ifdef TFA_XMEM_AMPCHECK
    if ( ampTempCheck==0) { // first time
        if (!tfaRunIsCold(handle)) {
            err = Tfa98xx_DspReadMem(handle, TFA_XMEM_AMPCHECK, 1, &ampTempCheck); //get initial value
            if (err)
                return 2; //other error
        }
        else
            return 1; // need to be (re-)configured
    }
    err = Tfa98xx_DspReadMem(handle, TFA_XMEM_AMPCHECK, 1, &ampTemp);
    if (err)
        return 2; //other error

    if ( ampTemp != ampTempCheck) {
        PRINT("ampTemp mismatch!!!=0x%04x, expected=0x%04x\n", ampTemp, ampTempCheck);
        return 1; // !! error need to be (re-)configured
    }
#endif /* TFA_XMEM_AMPCHECK */

    return 0; //ok
}


void tfaRunStatusCheck(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short status;

   /* Check status from register 0*/
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
   if (status & TFA98XX_STATUSREG_WDS_MSK)
   {
      PRINT("DSP watchDog triggerd");
      return;
   }
}

int tfaRunCheckEvents(unsigned short regval) {
    int severity=0;

    //TODO see if all alarms are similar
    if ( regval & TFA98XX_STATUSREG_OCDS) //
        severity = 1;
    // next will overwrite if set
    if ( regval & TFA98XX_STATUSREG_ERROR2_SET_MSK )
        severity = 2;

    return severity;
}

Tfa98xx_Error_t tfaRunPowerCycleCF(Tfa98xx_handle_t handle){
    Tfa98xx_Error_t err;

    TRACEIN

    err = Tfa98xx_Powerdown(handle, 1);
    PRINT_ASSERT(err);
    err = Tfa98xx_Powerdown(handle, 0);
    PRINT_ASSERT(err);

    return err;
}

Tfa98xx_Error_t tfaRunResolveIncident(Tfa98xx_handle_t handle, int incidentlevel){
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    TRACEIN
    switch(incidentlevel) {//idx=0
    case 1:
        err = Tfa98xx_ResolveIncident(handle, incidentlevel);
        PRINT_ASSERT(err);
        break;
    case 2:
        err = tfaRunSpeakerBoost(handle, 1); //force a reload
        break;
    default:    //nop
        break;
    }

    return err;
}

enum Tfa98xx_Error tfa98xx_writebf(nxpTfaBitfield_t bf) {
    Tfa98xx_Error_t err;
    int dev, devcount = tfa98xx_cnt_max_device();

    if ( devcount == 0 ) {
        PRINT_ERROR("No or wrong container file loaded\n");
        return    Tfa98xx_Error_Bad_Parameter;
    }

    for( dev=0; dev < devcount; dev++) {
        err = tfaContOpen(dev);
        if ( err != Tfa98xx_Error_Ok)
            goto error_exit;
        if ( tfa98xx_runtime_verbose )
            PRINT("bf device [%s]\n", tfaContDeviceName(dev));
        err = tfaRunWriteBitfield(dev, bf);
        if ( err != Tfa98xx_Error_Ok)
            goto error_exit;

    }

 error_exit:
    for( dev=0; dev < devcount; dev++)
    tfaContClose(dev); /* close it */
    return err;
}

/*
 * wait for calibrateDone
 */
#if 0 
#define TFA98XX_API_WAITRESULT_NTRIES TFA98XX_WAITRESULT_NTRIES_LONG // defined in API
#else
#define TFA98XX_API_WAITRESULT_NTRIES TFA98XX_WAITRESULT_NTRIES // defined in API
#endif
Tfa98xx_Error_t tfa98xxRunWaitCalibration(Tfa98xx_handle_t handle, int *calibrateDone)
{
    Tfa98xx_Error_t err;
    int tries = 0;
    unsigned short mtp;

    *calibrateDone = 0;

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);

    /* in case of calibrate once wait for MTPEX */
#if 0
    if ( mtp & TFA98XX_MTP_MTPOTC_MSK) {
        while ( (*calibrateDone == 0) && (tries < TFA98XX_API_WAITRESULT_NTRIES))
        {    // TODO optimise with wait estimation
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
            *calibrateDone = ( mtp & TFA98XX_MTP_MTPEX_MSK);    /* check MTP bit1 (MTPEX) */
            tries++;
        }
    } else /* poll xmem for calibrate always */
    {
        while ((*calibrateDone == 0) && (tries<TFA98XX_API_WAITRESULT_NTRIES) )
        {    // TODO optimise with wait estimation
            err = Tfa98xx_DspReadMem(handle, 231, 1, calibrateDone);
            tries++;
        }
    }
#else
    if ( mtp & TFA98XX_MTP_MTPOTC_MSK) {
        while (tries < TFA98XX_API_WAITRESULT_NTRIES)
        {    // TODO optimise with wait estimation
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
            *calibrateDone = ( mtp & TFA98XX_MTP_MTPEX_MSK);    /* check MTP bit1 (MTPEX) */
            if(*calibrateDone == 0) {
                tfaRun_Sleepus(50000);
            } else break;
            tries++;
        }
    } else /* poll xmem for calibrate always */
    {
        while (tries < TFA98XX_API_WAITRESULT_NTRIES)
        {    // TODO optimise with wait estimation
            err = Tfa98xx_DspReadMem(handle, 231, 1, calibrateDone);
            if(*calibrateDone == 0) {
                tfaRun_Sleepus(50000);
            } else break;
            tries++;
        }
    }
#endif
    if(tries==TFA98XX_API_WAITRESULT_NTRIES) {
        PRINT("!!calibrateDone timedout!!\n");
        err = Tfa98xx_Error_StateTimedOut;
    }
    return err;
}

/*
 * start up the all devices
 */
int tfaRunStartupAll(Tfa98xx_handle_t *handles)
{
    Tfa98xx_Error_t err87;
    unsigned int handleCount = sizeof(handles)/sizeof(handles[0]);
    unsigned int idx = 0;
    for (idx = 0; idx < (handleCount+1); idx++)
    {
        /* create handle */
        err87 = Tfa98xx_Open(I2C(idx), &handles[idx]);
        assert(err87 == Tfa98xx_Error_Ok);

        /* load the optimal TFA9887 in HW settings */
        err87 = Tfa98xx_Init(handles[idx]);
        assert(err87 == Tfa98xx_Error_Ok);

        err87 = Tfa98xx_SetSampleRate(handles[idx], 48000);
        assert(err87 == Tfa98xx_Error_Ok);
        err87 = Tfa98xx_SelectAmplifierInput(handles[idx], Tfa98xx_AmpInputSel_DSP);
        assert(err87 == Tfa98xx_Error_Ok);

        /* start at -6 dB */
        err87 = Tfa98xx_SetVolumeLevel(handles[idx], 0x0c);
        assert(err87 == Tfa98xx_Error_Ok);

        err87 = Tfa98xx_Powerdown(handles[idx], 0);
        assert(err87 == Tfa98xx_Error_Ok);
    }
   return err87;
}

/*
 * shutdown the clocks and power down for single device
 */
int tfaRunShutDown(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err87;
    err87 = Tfa98xx_Powerdown(handle, 1);
    assert(err87 == Tfa98xx_Error_Ok);
    err87 = Tfa98xx_Close(handle);
    assert(err87 == Tfa98xx_Error_Ok);
    return err87;
}

/*
 * shutdown the clocks and power down for all devices
 */
int tfaRunShutDownAll(Tfa98xx_handle_t *handles)
{
    Tfa98xx_Error_t err87;
    unsigned int handleCount = sizeof(handles)/sizeof(handles[0]);
    unsigned int idx = 0;
    for (idx = 0; idx < (handleCount+1); idx++)
    {
        err87 = tfaRunShutDown(handles[idx]);
        assert(err87 == Tfa98xx_Error_Ok);
    }
   return err87;

}
/*
 * store zero terminated filename to cached params
 */
int tfaRunNameToParam( char *fullname, nxpTfa98xxParamsType_t type, nxpTfa98xxParameters_t *parms)
{
    char *name;
    name = basename(fullname);

    switch(type) {
    case tfa_patch_params:
        strncpy ( tfaParams.patchFile, name, TFA_MAX_FILENAME);
        break;
    case tfa_speaker_params:
        strncpy ( tfaParams.speakerFile, name, TFA_MAX_FILENAME);
        break;
    case tfa_preset_params:
        strncpy ( tfaParams.profile[nxpTfaCurrentProfile].filename , name, TFA_MAX_FILENAME);
        break;
    case tfa_config_params:
        strncpy ( tfaParams.configFile, name, TFA_MAX_FILENAME);
        break;
    case tfa_equalizer_params:
        strncpy ( tfaParams.profile[nxpTfaCurrentProfile].Eqfilename , name, TFA_MAX_FILENAME);
        break;

        break;
    default:
        return 1;
    }
    return 0;
}
/*
 * display parameters
 */
void tfaRunShowParameters( nxpTfa98xxParameters_t *parms)
{
    int  p;

    PRINT("parameters:\n");
    // show commons

    if( parms->patchLength ) {
        PRINT(" patch: %s (%d bytes)", parms->patchFile, parms->patchLength);
    } else
        PRINT(" no patch");
    PRINT("\n");
    if( parms->configLength ) {
        PRINT(" config: %s (%d bytes)", parms->configFile, parms->configLength);
    } else
        PRINT(" no config");
    PRINT("\n");
    if( parms->speakerLength ) {
        PRINT(" speaker: %s (%d bytes)", parms->speakerFile, parms->speakerLength);
    } else
        PRINT(" no speaker");
    PRINT("\n");

    // show profiles
    for (p=0;p<TFA_MAX_PROFILES;p++) {
        PRINT(" profile[%d]:", p);
        if ( parms->profile[p].valid ){
            PRINT(" %s", parms->profile[p].filename);
            PRINT(" vsteps:%d", parms->profile[p].vsteps);
            PRINT(", sampleRate:%d", parms->profile[p].sampleRate);
            PRINT(", i2sIn%d", parms->profile[p].i2sRoute);
            PRINT(", pllin: %s", parms->profile[p].ipll ? "WS" : "BCK");
            if ( parms->profile[p].EqValid )
                PRINT("\n             %s", parms->profile[p].Eqfilename);
        } else
            PRINT(" not valid");
        PRINT("\n");
    }
}


enum Tfa98xx_Error tfa98xx_start(int next_profile, int *vstep, int channels)
{
    Tfa98xx_Error_t err;
    int dev, devcount = tfa98xx_cnt_max_device();
    int active_profile;
    int active_vstep;

    if ( devcount < 1 ) {
        PRINT_ERROR("No or wrong container file loaded\n");
        return    Tfa98xx_Error_Bad_Parameter;
    }

    /* set the current profile
     *     in case they get written during cold start
     */
    active_profile = tfa98xx_set_profile(next_profile);

    for( dev=0; dev < devcount; dev++) {
        err = tfaContOpen(dev);
        if ( err != Tfa98xx_Error_Ok)
            goto error_exit;

        if (tfa98xx_runtime_verbose)
            PRINT("Starting device [%s]\n", tfaContDeviceName(dev));
        if ( tfaRunIsCold(dev))
        {
            active_vstep = tfa98xx_set_vstep(vstep[dev]);
            /* cold start up without unmute*/
            err = tfaRunSpeakerBoost(dev, 0);
        }
        else
        {
            /* set the current vsteps
            *     in case they get written during cold start
            */
            active_vstep = tfa98xx_set_vstep(vstep[dev]);
            if ( next_profile != active_profile) {/* was it not done already */
                err = tfaContWriteProfile(dev, next_profile, vstep[dev]);
                if (err!=Tfa98xx_Error_Ok) /* if error, set to original profile*/
                {
                    tfa98xx_set_profile(active_profile);
                }
            }
            else
            {
                if (tfaRunIsPwdn(dev))
                {
                    err = tfaRunSpeakerBoost(dev, 0);
                }
                if ( err != Tfa98xx_Error_Ok)
                {
                    PRINT_ASSERT(err);
                    break;
                }
                if ( vstep[dev] != active_vstep)
                {
                    err = tfaContWriteFilesVstep(dev, next_profile, vstep[dev]);
                    if ( err != Tfa98xx_Error_Ok)
                    {
                        tfa98xx_set_vstep(active_vstep);
                        PRINT_ASSERT(err);
                    }
                }
            }
        }
    }
    if ( err == Tfa98xx_Error_Ok)
    {
        for( dev=0; dev < devcount; dev++) {
            err = Tfa98xx_EnableAECOutput(dev);
            err = tfaRunUnmute(dev);
        }
    }

error_exit:
    for( dev=0; dev < devcount; dev++)
        tfaContClose(dev); /* close all of them */
    return err;
}

enum Tfa98xx_Error tfa98xx_stop(void) {
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    int dev, devcount = tfa98xx_cnt_max_device();

    if ( devcount == 0 ) {
        PRINT_ERROR("No or wrong container file loaded\n");
        return    Tfa98xx_Error_Bad_Parameter;
    }

    for( dev=0; dev < devcount; dev++) {
        err = tfaContOpen(dev);
        if ( err != Tfa98xx_Error_Ok) {
            PRINT("Open device [%s] failed\n", tfaContDeviceName(dev));
            continue;
        }
        if (tfa98xx_runtime_verbose)
            PRINT("Stopping device [%s]\n", tfaContDeviceName(dev));
        /* tfaRunSpeakerBoost (called by start) implies unmute */
        /* mute + SWS wait */
        err = tfaRunMuteAmplifier( dev );
        if ( err != Tfa98xx_Error_Ok) {
            PRINT("tfaRunMuteAmplifier [%s] failed\n", tfaContDeviceName(dev));
            continue;
        }
        /* powerdown CF */
        err = Tfa98xx_Powerdown(dev, 1 );
        if ( err != Tfa98xx_Error_Ok) {
            PRINT("Tfa98xx_Powerdown [%s] failed\n", tfaContDeviceName(dev));
            continue;
        }

        err = Tfa98xx_DisableAECOutput(dev);
        if ( err != Tfa98xx_Error_Ok) {
            PRINT("Tfa98xx_DisableAECOutput [%s] failed\n", tfaContDeviceName(dev));
            continue;
        }
    }

error_exit:
    for( dev=0; dev < devcount; dev++)
        tfaContClose(dev); /* close all of them */
    return err;
}

enum Tfa98xx_Error tfa98xx_set_tone_detection(Tfa98xx_handle_t handle, int state)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

        /* check state: 1 is on, 0 is off*/
        if(state) {
                err = tfaRunColdStartup(handle);
                if(err == Tfa98xx_Error_Ok)
                        PRINT("Tone detection is reset \n");
        }
        else {
                err = Tfa98xx_SetToneDetectionOff(handle);

                if(err == Tfa98xx_Error_Ok)
                        PRINT("Tone detection is turned off \n");
        }

    return err;
}
