/*
Copyright 2014 NXP Semiconductors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/*
 * main_container.c
 *
 */
#include <Tfa98xx.h>
#include <assert.h>
#include <string.h>
#if defined(WIN32) || defined(_X64)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <NXP_I2C.h>
#ifndef WIN32
// need PIN access
#include <inttypes.h>
#include <lxScribo.h>
#ifdef Android
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "tfa98xx", __VA_ARGS__)
#endif
#endif

#include "nxpTfa98xx.h"
#include "tfa98xxRuntime.h"
#include "tfa98xxCalibration.h"
#include "tfaContainer.h"
#include "Tfa98API.h"

#include "exTfa98xx.h"

#ifndef WIN32

#define Sleep(ms) usleep((ms)*1000)
#define _fileno fileno
#define _GNU_SOURCE   /* to avoid link issues with sscanf on NxDI? */
#ifdef Android
pthread_mutex_t pa_logfile_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

#define MAX_DEVICES 2
/*#define CNT_FILENAME "Tfa98xx.cnt"*/

#define exTfa98xx_STATUSREG_WDS             (0x1<<13)
#define exTfa98xx_STATUSREG_ACS             (0x1<<11)
#define exTfa98xx_STATUSREG_OCDS            (0x1<<5)
#define exTfa98xx_STATUSREG_PLL             (0x1<<1)

/* global parameter cache */
extern nxpTfa98xxParameters_t tfaParams;
static exTfa98xx_audio_mode_t setmode = Audio_Mode_Music_Normal;
static int mLeftvolume = 0;
static int mRightvolume = 0;

int cli_verbose=0;    /* verbose flag */
extern regdef_t regdefs[];
int NXP_I2C_verbose=0;
static FIXEDPT Imped25[]= {0.0, 0.0};

/* global parameter cache */
extern nxpTfa98xxParameters_t tfaParams;

static void exTfa98xx_statusmonitor( int dev )
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handles[] = {-1,-1};
    unsigned short status;
    int profile[MAX_DEVICES];
    int vsteps[MAX_DEVICES]={0,0};

    vsteps[0] = mLeftvolume;
    vsteps[1] = mRightvolume;
    profile[0] = setmode;
    profile[1] = setmode;

    nxpTfa98xxSetIdx(dev);
    handles[dev] = dev;
    /* check status ACS bit to set */
    err = tfa98xxReadRegister(handles, 0x00, &status); // this will also call open
#ifdef Android
    LOGD("handles %d\n", handles[dev]);
    LOGD("[NXP] exTfa98xx status reg is 0x%04x\n", status);
#endif
    if ( !(status & exTfa98xx_STATUSREG_PLL))
    {
#ifdef Android
        LOGD("[NXP] exTfa98xx has no clock input\n");
#endif
        return;
    }
    if ( (status & exTfa98xx_STATUSREG_ACS) |
         (status & exTfa98xx_STATUSREG_WDS) |
         (status & exTfa98xx_STATUSREG_OCDS) )
    {
#ifdef Android
        LOGD("error detected, need recovery\n");
#endif
        err = tfaRunSpeakerBoost(handles[dev], 1);
        if ( err != Tfa98xx_Error_Ok)
        {
#ifdef Android
            LOGD("[NXP] exTfa98xx recover failed\n");
#endif
        }
        else
        {
#ifdef Android
            LOGD("[NXP] exTfa98xx system recovered\n");
#endif
            err = tfaContWriteFilesProf(handles[dev], profile[dev], vsteps[dev]);
        }
    }

    if (err)
    {
#ifdef Android
        LOGD("close handle failed\n");
#endif
    }
    else
    {
#ifdef Android
        LOGD("status checking ok\n");
#endif
     }
    return;
}

void exTfa98xx_LR_Switch( int i32Ori )
{
    int i32dev = 0;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handles[] = {-1,-1};
    unsigned short u16status = 0, u16CHS12 = 0;
#ifdef Android
    pthread_mutex_lock(&mutex);
#endif
    for( i32dev=0; i32dev < tfa98xx_cnt_max_device(); i32dev++) {
        err = tfaContOpen(i32dev);
        nxpTfa98xxSetIdx(i32dev);
        handles[i32dev] = i32dev;
        /* check status ACS bit to set */
        err = tfa98xxReadRegister(handles, 0x4, &u16status);
        if(i32Ori!=0) {//Original
            u16CHS12 = i32dev?2:1;
        } else {
            u16CHS12 = i32dev?1:2;
        }
        u16status = u16status & ~0x18;
        u16status |= u16CHS12;
        tfa98xxWriteRegister(0x4, u16status, handles);
    }
    for( i32dev=0; i32dev < tfa98xx_cnt_max_device(); i32dev++) {
        err = tfaContClose(i32dev);
    }
#ifdef Android
    pthread_mutex_unlock(&mutex);
#endif
    return;
}

int exTfa98xx_speakeron( exTfa98xx_audio_mode_t mode)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    int profile[MAX_DEVICES];
    int vsteps[MAX_DEVICES]={0,0};
    int dev;

#ifdef Android
    pthread_mutex_lock(&mutex);
#endif
    vsteps[0] = mLeftvolume;
    vsteps[1] = mRightvolume;
    profile[0] = mode;
    profile[1] = mode;

#ifdef Android
    LOGD("exTfa98xx_speakeron: mode %d, setmode %d", mode, setmode);
#endif
    /* load the container file to switch to new profile */
    if (!tfa98xx_cnt_loadfile(LOCATION_FILES CNT_FILENAME, 0) )  { /* read params */
#ifdef Android
        LOGD("Load container failed\n");
        pthread_mutex_unlock(&mutex);
#endif
        return -1;
    } else {
#ifdef Android
        LOGD("Loaded container file speaker_on %s.\n", CNT_FILENAME);
#endif
    }

#ifdef Android
        LOGD("tfa start with volume %d.\n", vsteps[0]);
#endif
    err = tfa98xx_start(profile[0], vsteps, tfa98xx_cnt_max_device());
    if (err)
    {
#ifdef Android
        LOGD("start use case failed\n");
        LOGD("error is number %d\n", err);
#endif
        for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
            err = tfaContOpen(dev);
            exTfa98xx_statusmonitor(dev);
        }
        for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
            err = tfaContClose(dev);
        }
        err = tfa98xx_stop();
#ifdef Android
        LOGD("start failed then stopping device\n");
#endif
        if (err)
           {
#ifdef Android
            LOGD("stop failed error is number %d\n", err);
#endif
        }
#ifdef Android
        pthread_mutex_unlock(&mutex);
#endif
        return -1;
       }
#ifdef Android
    LOGD("start use case success\n");
#endif
    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        err = tfaContOpen(dev);
        exTfa98xx_statusmonitor(dev);
    }
    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        err = tfaContClose(dev);
    }

    if (err)
    {
#ifdef Android
        LOGD("tfa start failed error : %d\n", err);
        pthread_mutex_unlock(&mutex);
#endif
           return -1;
    }
    else
    {
        setmode = mode;
#ifdef Android
        LOGD("exTfa98xx start use case success\n");
#endif
    }
#ifdef Android
    pthread_mutex_unlock(&mutex);
#endif
    return 0;
}

void exTfa98xx_speakeroff()
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
#ifdef Android
    pthread_mutex_lock(&mutex);
#endif

    err = tfa98xx_stop();
    if (err)
    {
#ifdef Android
        LOGD("stopping device failed error : %d\n", err);
#endif
    }
    else
    {
#ifdef Android
        LOGD("device goes to stop successfully\n");
#endif
    }

   Sleep(5);
#ifdef Android
   pthread_mutex_unlock(&mutex);
#endif
   return;
}

void exTfa98xx_setvolumestep(int leftvolume, int rightvolume)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
       int profile[MAX_DEVICES];
       int vsteps[MAX_DEVICES]={0,0};
    int dev;

#ifdef Android
    pthread_mutex_lock(&mutex);
#endif
    vsteps[0] = leftvolume;
    vsteps[1] = rightvolume;
    profile[0] = setmode;
    profile[1] = setmode;
    mLeftvolume = leftvolume;
    mRightvolume = rightvolume;
    /* load the container file to switch to new profile */

#ifdef Android
    LOGD("[NXP] exTfa98xx mode is set for %d in set volume step file\n", profile[0]);
#endif
    err = tfa98xx_start(profile[0], vsteps, tfa98xx_cnt_max_device());

    if (err)
    {
#ifdef Android
        LOGD("tfa set volume failed\n");
#endif
    }
    else
    {
#ifdef Android
        LOGD("tfa set volume successful\n");
#endif
    }
#ifdef Android
    pthread_mutex_unlock(&mutex);
#endif
    return;
}

/*
 * calling exTfa98xx_getImped25, after exTfa98xx_calibration
 *
 * @param device index
 * @return Imped25 of the device
 *
 */
FIXEDPT exTfa98xx_getImped25(int dev)
{
    FIXEDPT imped = 0.0;
    pthread_mutex_lock(&mutex);
    if (dev < MAX_DEVICES) imped = Imped25[dev];
    pthread_mutex_unlock(&mutex);
    return imped;
}

int exTfa98xx_calibration(int bManual)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handlesIn[] ={-1, -1};
    int dev;
    FIXEDPT Re25 = 0.0;
#ifdef Android
    pthread_mutex_lock(&mutex);

    LOGD("[NXP] %s ENTER",__func__);
#endif
    if (!tfa98xx_cnt_loadfile(LOCATION_FILES CNT_FILENAME, 0) )  { // read params
#ifdef Android
        LOGD("Load container failed\n");
        pthread_mutex_unlock(&mutex);
#endif
        return -1;
    }
    else {
#ifdef Android
        LOGD("Loaded container file calibration %s.\n", CNT_FILENAME);
#endif
    }

    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        nxpTfa98xxSetIdx(dev);
        err = tfaContOpen(dev);
        handlesIn[dev] = dev;
        if (err)
        {
#ifdef Android
            LOGD("Open failed\n");
            pthread_mutex_unlock(&mutex);
#endif
            return -1;
        }
        err = tfa98xxCalibrationEx(handlesIn, dev, 1, bManual, &Re25);
        if (err)
        {
            for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
                nxpTfa98xxSetIdx(dev);
                err = tfa98xxClose(handlesIn);
            }
            tfa98xx_stop();
#ifdef Android
            LOGD("calibration failed error: %d\n", err);
            pthread_mutex_unlock(&mutex);
#endif
            return -1;
        } else {
            Imped25[dev] = Re25;
        }
    }
#if 0
    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        nxpTfa98xxSetIdx(dev);
        err = tfa98xxClose(handlesIn);
    }
    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
                err = tfaContOpen(dev);
                exTfa98xx_statusmonitor(dev);
    }
    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        err = tfaContClose(dev);
    }
#else
    /* after calibration is done turned down all of the devices */
    for(dev=0; dev <  tfa98xx_cnt_max_device(); dev++) {
        if (tfa98xx_runtime_verbose)
            LOGD("Stopping device [%s]\n", tfaContDeviceName(dev));
        exTfa98xx_statusmonitor(dev);
        /* tfaRunSpeakerBoost (called by start) implies unmute */
        /* mute + SWS wait */
        err = tfaRunMuteAmplifier( dev );
        if (err != Tfa98xx_Error_Ok)
            continue;
        /* powerdown CF */
        err = Tfa98xx_Powerdown(dev, 1 );
        if (err != Tfa98xx_Error_Ok)
            continue;
        tfaContClose(dev);
   }
#endif
#ifdef Android
    LOGD("[NXP] %s END Calibration is done!",__func__);

    pthread_mutex_unlock(&mutex);
#endif
    return 0;
}

void exTfa98xx_factorytest(int bManual)
{
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    Tfa98xx_handle_t handlesIn[] ={-1, -1};
    int dev;
    FIXEDPT Re25 = 0.0;
#ifdef Android
    pthread_mutex_lock(&mutex);

    LOGD("[NXP] %s ENTER",__func__);
#endif
    if (!tfa98xx_cnt_loadfile(LOCATION_FILES CNT_FILENAME, 0) )  { // read params
#ifdef Android
        LOGD("Load container failed\n");
        pthread_mutex_unlock(&mutex);
#endif
        return;
    }
    else {
#ifdef Android
        LOGD("Loaded container file calibration %s.\n", CNT_FILENAME);
#endif
    }

    for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
        nxpTfa98xxSetIdx(dev);
        err = tfaContOpen(dev);
        handlesIn[dev] = dev;
        if (err)
        {
#ifdef Android
            LOGD("Open failed\n");
            pthread_mutex_unlock(&mutex);
#endif
            return;
        }
        err = tfa98xxCalibrationEx(handlesIn, dev, 1, bManual, &Re25);
        if (err)
        {
            for( dev=0; dev < tfa98xx_cnt_max_device(); dev++) {
                nxpTfa98xxSetIdx(dev);
                err = tfa98xxClose(handlesIn);
            }
            tfa98xx_stop();
#ifdef Android
            LOGD("calibration failed error: %d\n", err);
            pthread_mutex_unlock(&mutex);
#endif
            return;
        } else {
            Imped25[dev] = Re25;
        }
    }

#ifdef Android
    LOGD("[NXP] %s END Calibration is done!",__func__);

    pthread_mutex_unlock(&mutex);
#endif
}

int main(int argc, char* argv[])
{
    Tfa98xx_Error_t err;
    char target[FILENAME_MAX];

    /* Get the container file from command line */
    if ( argc == 1 ) {
        printf("Please supply a container file as command line argument.\n");
        return 0;
    }
    if ( argc == 3 ) {
        /* target specified */
        strcpy(target, argv[2]);
    } else {
        /* no target specified, use default*/
#ifdef Android
        strcpy(target, "/dev/ttyACM0");
#else
        strcpy(target, "scribo");
#endif
    }
    tfa_cnt_verbose( argc==4);

    printf ("Device started with cold start up or calibration if tfa is not calibrated.\n");

    err = exTfa98xx_calibration(0);

    if (!err)
    {
        printf ("Device started with cold start up or calibration successful.\n");
    }
    else
    {
        printf ("Device cold start failed, err: %d", err);
    }

    printf ("Press enter to start next step.\n");
    scanf ("%*[^\n]");

    exTfa98xx_speakeroff();

    printf ("Device first stop and then continue first time speaker on with mode music.\n");

    err = exTfa98xx_speakeron(Audio_Mode_Music_Normal);

    if (!err)
    {
        printf ("Device started with warm start up successful.\n");
    }
    else
    {
        printf ("Device warm start failed, err: %d", err);
    }

    printf ("Press enter to start next step.\n");
    scanf ("%*[^\n]");

    exTfa98xx_setvolumestep(6,6);

    if (!err)
    {
        printf ("Device started with warm start up successful.\n");
    }
    else
    {
        printf ("Device warm start failed, err: %d", err);
    }

    printf ("Device switch to voice mode.\n");

    exTfa98xx_setvolumestep(0,0);

    err = exTfa98xx_speakeron(Audio_Mode_Voice);

    if (!err)
    {
        printf ("Device switch to voice successful.\n");
    }
    else
    {
        printf ("Device warm start failed, err: %d", err);
    }

    printf ("Press enter to stop device.\n");
    scanf ("%*[^\n]");

    exTfa98xx_speakeroff();
    return 0;
}
