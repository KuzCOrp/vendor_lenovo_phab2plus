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



#ifndef TFA98XXRUNTIME_H_
#define TFA98XXRUNTIME_H_

#include "tfa98xxParameters.h"

/**
 * Start/Restart the SpeakerBoost on all devices.
 *
 * This can only be called when the audio input clock is active.
 * When the device is in coldstart-state (ACS=1) then a full initialization
 * will be performed.
 * In case of a warm start only a power-on and un-mute will be executed.
 *
 * @param profile the profile to load, if -1 then don't change profile
 * @param vsteps the volume step selections for each channel, if -1 then softmute
 *                        0 sets the maximum volume
 * @param channels the nr of channels to which the vsteps array apply
 * @return enum Tfa98xx_Error
 */
Tfa98xx_Error_t tfa98xx_start(int profile, int *vstep, int channels);
/**
 * Stop SpeakerBoost on all devices.
 *
 * This the notification of the audio clock to be taken away by the host.
 * Note that the function will block until the amplifiers are actually switched
 * off unless timed-out.
 *
 * @return enum Tfa98xx_Error
 */
Tfa98xx_Error_t tfa98xx_stop(void);
/*
 * accounting globals
 */
int gTfaRun_useconds;
int gTfRun_i2c_writes;
int gTfRun_i2c_reads;
int tfa98xx_runtime_verbose;
int gTfaRun_timingVerbose;

int gNXP_i2c_writes;
int gNXP_i2c_reads;

Tfa98xx_Error_t tfaRunColdboot(Tfa98xx_handle_t handle, int state);
Tfa98xx_Error_t tfaRunMuteAmplifier(Tfa98xx_handle_t handle);
Tfa98xx_Error_t tfaRunMute(Tfa98xx_handle_t handle);
Tfa98xx_Error_t tfaRunUnmute(Tfa98xx_handle_t handle);

Tfa98xx_Error_t tfa98xxRunWaitCalibration(Tfa98xx_handle_t handle, int *calibrateDone);

/*
 * set verbosity level
 */
void tfaRunVerbose(int level);

/*
 * set TimingVerbose level
 */
void gTfaRunTimingVerbose(int level);

void Run_useconds(int level);
void Run_i2c_writes(int level);
void Run_i2c_reads(int level);
int gTfaRun_useconds;
int gTfRun_i2c_writes;
int gTfRun_i2c_reads;


void TfaCurrentProfile(int level);
int tfa98xx_set_profile(int level);
int tfa98xx_get_profile(void);
int tfa98xx_set_vstep(int level);
int tfa98xx_get_vstep(void);
void i2c_writes(int level);
void i2c_reads(int level);

/*
 * start the maximus speakerboost algorithm
 *  this implies a full system startup when the system was not already started
 *
 */
Tfa98xx_Error_t tfaRunSpeakerBoost(Tfa98xx_handle_t handle, int force);
/*
 *  this will load the patch witch will implicitly start the DSP
 *   if no patch is available the DPS is started immediately
 */
Tfa98xx_Error_t tfaRunStartDSP(Tfa98xx_handle_t handle);
/*
 * start the clocks and wait until the AMP is switching
 *  on return the DSP sub system will be ready for loading
 */
Tfa98xx_Error_t tfaRunStartup(Tfa98xx_handle_t handle);

/*
 * startup all devices. all step until patch loading is handled
 */
int tfaRunStartupAll(Tfa98xx_handle_t *handles);
/*
 * powerup the coolflux subsystem and wait for it
 */
Tfa98xx_Error_t tfaRunCfPowerup(Tfa98xx_handle_t handle);
/*
 * send the incident severty to the TFA layer to resolved based on device type
 */
Tfa98xx_Error_t tfaRunResolveIncident(Tfa98xx_handle_t handle, int incidentlevel);
/*
 * shutdown the clocks and power down single or all devices
 */
int tfaRunShutDown(Tfa98xx_handle_t handle);
int tfaRunShutDownAll(Tfa98xx_handle_t *handles);
/*
 * true if cold-started
 */
int tfaRunIsCold(Tfa98xx_handle_t handle);
int tfaRunIsPwdn(Tfa98xx_handle_t handle);

/*
 * run the startup/init sequence and set ACS bit
 */
Tfa98xx_Error_t tfaRunColdStartup(Tfa98xx_handle_t handle);
/*
 * clear the DSP reset counter
 */
Tfa98xx_Error_t tfaRunResetCountClear(Tfa98xx_handle_t handle);
/*
 * read the DSP reset counter
 */
int tfaRunResetCount(Tfa98xx_handle_t handle);

void tfaRun_Sleepus(int us);
void tfaRun_SleepusTrace(int us, char *file, int line);
#define tfaRun_Sleepus(t) tfaRun_SleepusTrace(t, __FILE__, __LINE__);
/*
 * validate the parameter cache
 *  if not loaded fix defaults
 */
int tfaRunNameToParam( char *fullname, nxpTfa98xxParamsType_t type, nxpTfa98xxParameters_t *parms);
void tfaRunShowParameters( nxpTfa98xxParameters_t *parms);
/*
 * first test implementation for handling OCP events
 */
int tfaRunCheckEvents(unsigned short regval);
void tfaRunStatusCheck(Tfa98xx_handle_t handle);

enum Tfa98xx_Error tfa98xx_writebf(nxpTfaBitfield_t bf);

/*
 * check for algorythm corruption
 */
int tfaRunCheckAlgo(Tfa98xx_handle_t handle);
Tfa98xx_Error_t tfaRunPowerCycleCF(Tfa98xx_handle_t handle);

enum Tfa98xx_Error tfa98xx_set_tone_detection(Tfa98xx_handle_t handle, int state);


#endif /* TFA98XXRUNTIME_H_ */
