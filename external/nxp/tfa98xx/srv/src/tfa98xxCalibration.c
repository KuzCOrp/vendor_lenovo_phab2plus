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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "dbgprint.h"
#include "Tfa98API.h"
#include "nxpTfa98xx.h"
#include "tfa98xxRuntime.h"
#include "tfaContainer.h"
#include "tfa98xxCalibration.h"
#include "Tfa98xx_Registers.h"
#include "tfaOsal.h"
#include "tfa98xx_cust.h"

/* global parameter cache */
extern nxpTfa98xxParameters_t tfaParams;
extern int nxpTfaCurrentProfile;

/* for verbosity */
int tfa98xx_cal_verbose;

/*
 * Set the debug option
 */
void tfa98xxCalVerbose(int level) {
    tfa98xx_cal_verbose = level;
}

int tfa98xxCalDspSupporttCoef(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    int bSupporttCoef;

    err = Tfa98xx_DspSupporttCoef(handle, &bSupporttCoef);
    assert(err == Tfa98xx_Error_Ok);

    return bSupporttCoef;
}
/*
 * return the speakerimpedence tCoef
 */
float tfa98xxCalGetTcoef(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    float re25;
    int Tcal; /* temperature at which the calibration happened */
    int calibrateDone = 0;

    if( handle == -1)
    {
        err = Tfa98xx_Open( tfa98xxI2cSlave*2, &handle );
        PRINT_ASSERT( err);
        if (err != Tfa98xx_Error_Ok)
        {
            return err;
        }
    }

    tfa98xxRunWaitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
        err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    }
    else
    {
        re25 = 0;
    }
    err = Tfa98xx_DspReadMem(handle, 232, 1, &Tcal);
    if (err != Tfa98xx_Error_Ok)
    {
        return err;
    }
    //PRINT("Calibration value is %2.2f ohm @ %d degrees\n", re25, Tcal);
    return re25;
}
/*
 * return the tcoefA
 */
FIXEDPT tfa98xxGetTcoefA(Tfa98xx_handle_t *handlesIn)
{
    Tfa98xx_Error_t err;
    FIXEDPT tCoefA, tCoef;
    FIXEDPT re25;
    int Tcal; /* temperature at which the calibration happened */
    int T0;
    int calibrateDone = 0;
    Tfa98xx_handle_t handle;
    uint8_t *speakerbuffer;

    speakerbuffer = tfacont_speakerbuffer(handlesIn[0]);
    if(speakerbuffer==0) {
        PRINT("No speaker data found\n");
        return Tfa98xx_Error_Bad_Parameter;
    }

    if( handlesIn[0] == -1)
    {
        err = Tfa98xx_Open( tfa98xxI2cSlave*2, &handlesIn[0] );
        PRINT_ASSERT( err);
        if (err != Tfa98xx_Error_Ok)
        {
            return err;
        }
    }

    handle = handlesIn[0] ;

    tfa98xxRunWaitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
        err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    }
    else
    {
        re25 = 0;
    }
    err = Tfa98xx_DspReadMem(handle, 232, 1, &Tcal);
    if (err != Tfa98xx_Error_Ok)
    {
        return err;
    }
    PRINT("Calibration value is %2.2f ohm @ %d degrees\n", re25, Tcal);
    tCoef = tfa98xxCaltCoefFromSpeaker(speakerbuffer);
    /* calculate the tCoefA */
    T0 = 25; /* definition of temperature for Re0 */
    tCoefA = tCoef * re25 / (tCoef * (Tcal - T0)+1); /* TODO: need Rapp influence */
    PRINT(" Final tCoefA %1.5f\n", tCoefA);

    return tCoefA;
}
/*
 *  calculate a new tCoefA and put the result into the loaded Speaker params
 */
Tfa98xx_Error_t tfa98xxCalComputeSpeakertCoefA(  Tfa98xx_handle_t handle,
                                                Tfa98xx_SpeakerParameters_t loadedSpeaker,
                                                float tCoef )
{
    Tfa98xx_Error_t err;
    float tCoefA;
    FIXEDPT re25;
    int Tcal; /* temperature at which the calibration happened */
    int T0;
    int calibrateDone = 0;
    int nxpTfaCurrentProfile = tfa98xx_get_profile();

    /* make sure there is no valid calibration still present */
    err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    PRINT_ASSERT(err);
    assert(fabs(re25) < 0.1);
    PRINT(" re25 = %2.2f\n", re25);

    /* use dummy tCoefA, also eases the calculations, because tCoefB=re25 */
    tfa98xxCaltCoefToSpeaker(loadedSpeaker, 0.0f);

    // write all the files from the device list (typically spk and config)
    err = tfaContWriteFiles(handle);
    if (err) return err;

    // write all the files from the profile list (typically preset)
    tfaContWriteFilesProf(handle, nxpTfaCurrentProfile, 0); // use volumestep 0


    /* start calibration and wait for result */
    err = Tfa98xx_SetConfigured(handle);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }
    if (tfa98xx_cal_verbose)
        PRINT(" ----- Configured (for tCoefA) -----\n");

    tfa98xxRunWaitCalibration(handle, &calibrateDone);
    if (calibrateDone)
    {
      err = Tfa98xx_DspGetCalibrationImpedance(handle, &re25);
    }
    else
    {
       re25 = 0;
    }
    err = Tfa98xx_DspReadMem(handle, 232, 1, &Tcal);
    if (err != Tfa98xx_Error_Ok)
    {
       return err;
    }
    PRINT("Calibration value is %2.2f ohm @ %d degrees\n", re25, Tcal);
    /* calculate the tCoefA */
    T0 = 25; /* definition of temperature for Re0 */
    tCoefA = tCoef * re25 / (tCoef * (Tcal - T0)+1); /* TODO: need Rapp influence */
    PRINT(" Final tCoefA %1.5f\n", tCoefA);

    /* update the speaker model */
    tfa98xxCaltCoefToSpeaker(loadedSpeaker, tCoefA);

    /* !!! the host needs to save this loadedSpeaker as it is needed after the next cold boot !!! */

    return err;
}
float tfa98xxCaltCoefFromSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes)
{
    int iCoef;

    /* tCoef(A) is the last parameter of the speaker */
    iCoef = (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3]<<16) + (speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2]<<8) + speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1];

    return (float)iCoef/(1<<23);
}

void tfa98xxCaltCoefToSpeaker(Tfa98xx_SpeakerParameters_t speakerBytes, float tCoef)
{
    int iCoef;

    iCoef =(int)(tCoef*(1<<23));

    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-3] = (iCoef>>16)&0xFF;
    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-2] = (iCoef>>8)&0xFF;
    speakerBytes[TFA98XX_SPEAKERPARAMETER_LENGTH-1] = (iCoef)&0xFF;
}

Tfa98xx_Error_t tfa98xxCalSetCalibrateOnce(Tfa98xx_handle_t handle)
{
        Tfa98xx_Error_t err;
        unsigned short mtp, status;
        int tries = 0;

       /* Read MTP Register  */
       err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
       if (err != Tfa98xx_Error_Ok)
      {
         return err;
      }

       /* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

       /* Verify if device is already set to calibrate once.*/
       if ( (mtp & TFA98XX_MTP_MTPOTC_MSK ) == 0)
       {
              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTPKEY2_REG, 0x5A); /* unlock key2 */
              if (err != Tfa98xx_Error_Ok)
               {
                  goto EXIT;
               }

              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP,  TFA98XX_MTP_MTPOTC_MSK); /* MTPOTC=1, MTPEX=0 */
              if (err != Tfa98xx_Error_Ok)
               {
                  goto EXIT;
               }
              err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP_CTRL, TFA98XX_MTP_CTRL_CIMTP_MSK); /* CIMTP=1 */
              if (err != Tfa98xx_Error_Ok)
               {
                  goto EXIT;
               }
              // polling status for MTP busy clear
#if 0
              for(;;){
                  tfaRun_Sleepus(10000); // wait 1ms for mtp to complete
                  err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
                  if ( status & TFA98XX_STATUSREG_MTPB_MSK)
                      ;//PRINT("0=%0x\n",status);
                  else break;
              }
#else
            do {
                err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
                if (err != Tfa98xx_Error_Ok) {
                    PRINT("%s (%s:%u) %s\n",__FUNCTION__,__FILE__,__LINE__,
                        Tfa98xx_GetErrorString(err));
                    break;
                }
                if (status & TFA98XX_STATUSREG_MTPB_MSK) {
                    if (++tries >=TFA98XX_WAITRESULT_NTRIES) {
                        break;
                    }
                    tfaRun_Sleepus(10000);
                } else {
                   break;
                }
            } while (tries < TFA98XX_WAITRESULT_NTRIES);
#endif
       }
EXIT:
       err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x0); /* lock key2 */
       return err;
}
Tfa98xx_Error_t tfa98xxCalSetCalibrationAlways(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
    unsigned short mtp, status;
    int tries = 0;

    err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTPKEY2_REG, 0x5A); /* unlock key2 */
    if (err != Tfa98xx_Error_Ok)
    {
       goto EXIT;
    }

    /* Read MTP Register  */
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    if (err != Tfa98xx_Error_Ok)
    {
       goto EXIT;
    }

    // if mtp=0 then it already set to calibration always, so skip
    if ( mtp!=0)
    {
        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 0); /* MTPOTC=0, MTPEX=0 */
        if (err != Tfa98xx_Error_Ok)
         {
            goto EXIT;
         }
        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP_CTRL, TFA98XX_MTP_CTRL_CIMTP_MSK); /* CIMTP=1 */
        if (err != Tfa98xx_Error_Ok)
         {
            goto EXIT;
         }
        // note writing MPT to 0 takes long
        tfaRun_Sleepus(140000); // wait 140ms for mtp to complete
        // polling status for MTP busy clear
#if 0
        for(;;){
            tfaRun_Sleepus(10000); // wait 10ms for mtp to complete
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
            if ( status & (1<<8))
                ;//PRINT("0=%0x\n",status);
            else break;
        }
#else
        do {
            err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
            if (err != Tfa98xx_Error_Ok) {
                PRINT("%s (%s:%u) %s\n",__FUNCTION__,__FILE__,__LINE__,
                    Tfa98xx_GetErrorString(err));
                break;
            }
            if (status & TFA98XX_STATUSREG_MTPB_MSK) {
                if (++tries >= TFA98XX_WAITRESULT_NTRIES) {
                    break;
                }
                tfaRun_Sleepus(10000); // wait 10ms for mtp to complete
            } else {
               break;
            }
        } while (tries < TFA98XX_WAITRESULT_NTRIES);
#endif
   }
EXIT:
   err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x0); /* lock key2 */
   return err;

}

/*
 *
 */
Tfa98xx_Error_t tfa98xxCalibration(Tfa98xx_handle_t *handlesIn, int idx, int once )
{
    float tCoef = 0;
    FIXEDPT re25;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    int calibrateDone = 0;
    uint8_t *speakerbuffer = NULL;

    PRINT("calibrate %s\n", once ? "once" : "always" );
    /* Added default profile as 0 for calibration*/
    tfa98xx_set_profile(0);

    err = tfaRunColdStartup(handlesIn[idx]);

    if (err)
        return err;

    if (once)
    {
        tfa98xxCalSetCalibrateOnce(handlesIn[idx]);
    }
    else
    {
        tfa98xxCalSetCalibrationAlways(handlesIn[idx]);
        /* check Re,
         *  in case the old value is still there reset it
        */
        err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &re25);

        PRINT_ASSERT(err);
        if (fabs(re25) > 0.1)
        {
            PRINT(" cleaning up old Re (=%2.2f)\n", re25);
            /* run startup again to clean up old calibration */
            err = tfaRunColdStartup(handlesIn[idx]);
            if (err)
                return err;
        }
    }
    if(tfa98xxCalCheckMTPEX(handlesIn[idx]) == 0)
    {
        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);

        if (!tfa98xxCalDspSupporttCoef(handlesIn[idx]))
        {
            PRINT(" 2 step calibration\n");
            speakerbuffer = tfacont_speakerbuffer(handlesIn[idx]);
            if(speakerbuffer==0) {
                PRINT("No speaker data found\n");
                return Tfa98xx_Error_Bad_Parameter;
            }
            tCoef = tfa98xxCaltCoefFromSpeaker(speakerbuffer);
            PRINT(" tCoef = %1.5f\n", tCoef);

            err = tfa98xxCalComputeSpeakertCoefA(handlesIn[idx], speakerbuffer, tCoef);
            assert(err == Tfa98xx_Error_Ok);

            /* if we were in one-time calibration (OTC) mode, clear the calibration results
            from MTP so next time 2nd calibartion step can start. */
            tfa98xxCalResetMTPEX(handlesIn[idx]);

            /* force recalibration now with correct tCoefA */
            tfaRunMuteAmplifier(handlesIn[idx]); /* clean shutdown to avoid plop */
            tfaRunColdStartup(handlesIn[idx]);
        }
    }
    else
    {
        PRINT("DSP already calibrated.\n Calibration skipped, previous results loaded from MTP.\n");
    }

    // For the first configuration the DSP expects at least
    // the speaker, config and a preset.
    // Therefore all files from the device list as well as the file
    // from the default profile are loaded before SBSL is set.
    //
    // Note that the register settings were already done before loading the patch
    //
    // write all the files from the device list (typically spk and config)
    err = tfaContWriteFiles(handlesIn[idx]);
    if (err)
        return err;

    // write all the files from the profile list (typically preset)
    err = tfaContWriteFilesProf(handlesIn[idx], tfa98xx_get_profile(), 0); // use volumestep 0
    PRINT_ASSERT(err);

    if (err != Tfa98xx_Error_Ok)
    {
        return err;
    }


    // tell DSP it's loaded
    err = Tfa98xx_SetConfigured(handlesIn[idx]);
    PRINT_ASSERT(err);

    if (err != Tfa98xx_Error_Ok)
    {
        return err;
    }

    err = tfa98xxRunWaitCalibration(handlesIn[idx], &calibrateDone);
    if(err) goto errorExit;

    if (calibrateDone)
    {
        err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx],&re25);
    }
    else
    {
        re25 = 0;
    }
    PRINT("Calibration value is \t\t\t\t\t%2.2f ohm\n", re25);
    /* Unmute after calibration */
    Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);

    if(tCoef != 0) {
        if (!tfa98xxCalDspSupporttCoef(handlesIn[idx]))
               tfa98xxCaltCoefToSpeaker(speakerbuffer, tCoef);
    }

errorExit:
    return err;
}
/*
 *
 */
Tfa98xx_Error_t tfa98xxCalibrationEx(Tfa98xx_handle_t *handlesIn, int idx, int once, int bManual, FIXEDPT *Imped25 )
{
    float tCoef = 0;
    FIXEDPT re25 = 0;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    int calibrateDone = 0;
    uint8_t *speakerbuffer;

    //PRINT("calibrate %s\n", once ? "once" : "always" );
    /* Added default profile as 0 for calibration*/
    tfa98xx_set_profile(0);

    err = tfaRunColdStartup(handlesIn[idx]);

    if (err) goto errorExit;

    if (once)
    {
        if(bManual) {
            tfa98xxCalResetMTPEX(handlesIn[idx]);
            err = tfaRunColdStartup(handlesIn[idx]);
            if (err) {
                ALOGD("%s %d err = %x", __func__, __LINE__, err);
            }
        }
        tfa98xxCalSetCalibrateOnce(handlesIn[idx]);
        err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &re25);
        if ((re25 < TFA98XX_NOMINAL_IMPEDANCE_MIN)
            || (re25 > TFA98XX_NOMINAL_IMPEDANCE_MAX)) {
            ALOGD("Calibration Value error in device %d, reset MtpEx and recalibration", idx);
            err = tfa98xxCalResetMTPEX(handlesIn[idx]);
            if (err) {
                ALOGD("LINE: %u reset calibration state failed error: 0x%04x\n", __LINE__, err);
            }
        } else {
            ALOGD("DSP has calibrated before, %2.2f ohm\n", re25);
        }
    }
    else
    {
        tfa98xxCalSetCalibrationAlways(handlesIn[idx]);
        /* check Re,
         *  in case the old value is still there reset it
        */
        err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &re25);

        PRINT_ASSERT(err);
        if (fabs(re25) > 0.1)
        {
            PRINT(" cleaning up old Re (=%2.2f)\n", re25);
            /* run startup again to clean up old calibration */
            err = tfaRunColdStartup(handlesIn[idx]);
            if (err) goto errorExit;
        }
    }
    if(tfa98xxCalCheckMTPEX(handlesIn[idx]) == 0)
    {
        /* ensure no audio during special calibration */
        err = Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Digital);
        assert(err == Tfa98xx_Error_Ok);

        if (!tfa98xxCalDspSupporttCoef(handlesIn[idx]))
        {
            PRINT(" 2 step calibration\n");
            speakerbuffer = tfacont_speakerbuffer(handlesIn[idx]);
            if(speakerbuffer==0) {
                PRINT("No speaker data found\n");
                err = Tfa98xx_Error_Bad_Parameter;
                goto errorExit;
            }
            tCoef = tfa98xxCaltCoefFromSpeaker(speakerbuffer);
            PRINT(" tCoef = %1.5f\n", tCoef);

            err = tfa98xxCalComputeSpeakertCoefA(handlesIn[idx], speakerbuffer, tCoef);
            assert(err == Tfa98xx_Error_Ok);

            /* if we were in one-time calibration (OTC) mode, clear the calibration results
            from MTP so next time 2nd calibartion step can start. */
            tfa98xxCalResetMTPEX(handlesIn[idx]);

            /* force recalibration now with correct tCoefA */
            tfaRunMuteAmplifier(handlesIn[idx]); /* clean shutdown to avoid plop */
            tfaRunColdStartup(handlesIn[idx]);
        }
    }
    else
    {
        PRINT("DSP already calibrated.\n Calibration skipped, previous results loaded from MTP.\n");
    }

    // For the first configuration the DSP expects at least
    // the speaker, config and a preset.
    // Therefore all files from the device list as well as the file
    // from the default profile are loaded before SBSL is set.
    //
    // Note that the register settings were already done before loading the patch
    //
    // write all the files from the device list (typically spk and config)
    err = tfaContWriteFiles(handlesIn[idx]);
    if (err) goto errorExit;

    // write all the files from the profile list (typically preset)
    err = tfaContWriteFilesProf(handlesIn[idx], tfa98xx_get_profile(), 0); // use volumestep 0
    PRINT_ASSERT(err);

    if (err != Tfa98xx_Error_Ok)
    {
        goto errorExit;
    }


    // tell DSP it's loaded
    err = Tfa98xx_SetConfigured(handlesIn[idx]);
    PRINT_ASSERT(err);

    if (err != Tfa98xx_Error_Ok)
    {
        goto errorExit;
    }

    err = tfa98xxRunWaitCalibration(handlesIn[idx], &calibrateDone);
    if(err) goto errorExit;

    if (calibrateDone)
    {
        err = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx],&re25);
        if ((re25 < TFA98XX_NOMINAL_IMPEDANCE_MIN)
            || (re25 > TFA98XX_NOMINAL_IMPEDANCE_MAX)) {
            ALOGD("Calibration Value error, reset MtpEx and, do not open device %d", idx);
            err = tfa98xxCalResetMTPEX(handlesIn[idx]);
            if (err) {
                ALOGD("LINE: %u reset calibration state failed error: 0x%04x\n", __LINE__, err);
            }
            re25 = 0;
        }
    }
    else
    {
        re25 = 0;
    }
    PRINT("%2.2f\n", re25);
    /* Unmute after calibration */
    Tfa98xx_SetMute(handlesIn[idx], Tfa98xx_Mute_Off);

    if(tCoef != 0) {
        if (!tfa98xxCalDspSupporttCoef(handlesIn[idx]))
               tfa98xxCaltCoefToSpeaker(speakerbuffer, tCoef);
    }

errorExit:
    if(NULL != Imped25) *Imped25 = re25;
    return err;
}

/*
 *
 *
 */
int tfa98xxCalCheckMTPEX(Tfa98xx_handle_t handle)
{
    unsigned short mtp;
    Tfa98xx_Error_t err;
    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
   if (err != Tfa98xx_Error_Ok)
   {
      return err;
   }

   if ( mtp & TFA98XX_MTP_MTPEX_MSK)    /* check MTP bit1 (MTPEX) */
        return 1;                    /* MTPEX is 1, calibration is done */
    else
        return 0;                    /* MTPEX is 0, calibration is not done yet */
}
/*
 *
 */
Tfa98xx_Error_t tfa98xxCalResetMTPEX(Tfa98xx_handle_t handle)
{
    Tfa98xx_Error_t err;
   unsigned short mtp;
    unsigned short status;
    int tries = 0;

    assert( handle != -1);

    err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
    if (err != Tfa98xx_Error_Ok)
    {
        return err;
    }
    /* all settings loaded, signal the DSP to start calibration, only needed once after cold boot */

    /* reset MTPEX bit if needed */
    if ( (mtp & TFA98XX_MTP_MTPOTC_MSK) && (mtp & TFA98XX_MTP_MTPEX_MSK))
    {
        err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A); /* unlock key2 */
        if (err != Tfa98xx_Error_Ok)
        {
            goto EXIT;
        }

        err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP, 1); /* MTPOTC=1, MTPEX=0 */
        if (err != Tfa98xx_Error_Ok)
        {
            goto EXIT;
        }
        err = Tfa98xx_WriteRegister16(handle, 0x62, 1<<11); /* CIMTP=1 */
        if (err != Tfa98xx_Error_Ok)
        {
            goto EXIT;
        }
    }
#if 0
    do {
        tfaRun_Sleepus(10000);
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if (err != Tfa98xx_Error_Ok) {
            return err;
        }
    } while ((status & TFA98XX_STATUSREG_MTPB) == TFA98XX_STATUSREG_MTPB);
    if (err != Tfa98xx_Error_Ok) {
        return err;
    }
#else
    do {
        err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
        if (err != Tfa98xx_Error_Ok) {
            PRINT("%s (%s:%u) %s\n",__FUNCTION__,__FILE__,__LINE__,
                Tfa98xx_GetErrorString(err));
            break;
        }
        if((status & TFA98XX_STATUSREG_MTPB) == TFA98XX_STATUSREG_MTPB) {
            if (++tries >=TFA98XX_WAITRESULT_NTRIES) {
                break;
            }
            tfaRun_Sleepus(10000);
        } else {
           break;
        }
    } while (tries < TFA98XX_WAITRESULT_NTRIES);

    if ( (status & TFA98XX_STATUSREG_MTPB)!=0 ) {
         PRINT("Waiting for TFA98XX_STATUSREG_MTPB timeouts!");
    }
#endif
EXIT:
   err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x0); /* lock key2 */
   return err;
}

/*
 *  Set DSP is configured for calibration
 */
int tfa98xxCalSetConfigured(Tfa98xx_handle_t *handlesIn )
{
   Tfa98xx_Error_t err87;
    unsigned int handleCount = sizeof(handlesIn)/sizeof(handlesIn[0]);
    unsigned int idx = 0;
    for (idx = 0; idx < (handleCount+1); idx++)
    {
        err87 = Tfa98xx_SetConfigured(handlesIn[0]);
        assert(err87 == Tfa98xx_Error_Ok);
        err87 = Tfa98xx_SetConfigured(handlesIn[1]);
        assert(err87 == Tfa98xx_Error_Ok);
    }
   return err87;
}
