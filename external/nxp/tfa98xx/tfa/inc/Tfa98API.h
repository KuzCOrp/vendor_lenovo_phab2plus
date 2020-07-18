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

#ifndef TFA98API_H
#define TFA98API_H
#include "Tfa98xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* the number of biquads supported */
#define TFA98XX_BIQUAD_NUM              10

typedef enum Tfa98xx_Error Tfa98xx_Error_t;
typedef enum Tfa98xx_AmpInputSel Tfa98xx_AmpInputSel_t;
typedef enum Tfa98xx_OutputSel Tfa98xx_OutputSel_t;
typedef enum Tfa98xx_StereoGainSel Tfa98xx_StereoGainSel_t;
typedef enum Tfa98xx_DAI Tfa98xx_DAI_t;
typedef enum Tfa98xx_Channel Tfa98xx_Channel_t;
typedef enum Tfa98xx_Mode Tfa98xx_Mode_t;

typedef enum Tfa98xx_Mute Tfa98xx_Mute_t;
typedef enum Tfa98xx_SpeakerBoostStatusFlags Tfa98xx_SpeakerBoostStatusFlags_t;


typedef struct Tfa98xx_StateInfo Tfa98xx_StateInfo_t;

/* possible memory values for DMEM in CF_CONTROLs */
typedef enum Tfa98xx_DMEM Tfa98xx_DMEM_e;

/* register definition structure */
typedef struct regdef regdef_t;

/* for speaker parameters */
#define TFA98XX_SPEAKERPARAMETER_LENGTH   423
typedef unsigned char
 Tfa98xx_SpeakerParameters_t[TFA98XX_SPEAKERPARAMETER_LENGTH];

//typedef int Tfa98xx_handle_t;
#define Tfa98xx_handle_t int


/**
 * Open an instance to a TFA98XX IC.
 * When successful a created instance handle is also returned.
 * This handle is to be passed as first parameter to the other calls.
 * @param slave_address can be 0x68, 0x6A, 0x6C, 0x6E
 * @param *pHandle:
 * @return instance handle when successful
 */
Tfa98xx_Error_t Tfa98xx_Open(unsigned char slave_address,
                 Tfa98xx_handle_t *pHandle);
/**
 * Check if device is opened.
 */
int handle_is_open(Tfa98xx_handle_t h);

/**
 * Ensures the I2C registers are loaded with good default values for optimal operation.
 * @param handle to opened instance
 */
Tfa98xx_Error_t Tfa98xx_Init(Tfa98xx_handle_t handle);

/**
 * This will return the maximum number of devices addressable by the API.
 * @return Integer with number of devices
 */
int Tfa98xx_MaxDevices(void);

/**
 * Close the instance when no longer needed.
 * @param handle to opened instance
 */
Tfa98xx_Error_t Tfa98xx_Close(Tfa98xx_handle_t handle);

/**
 * Take the TFA98xx out of, or into power down state.
 * @param handle to opened instance
 * @param powerdown must be 1 or 0,
 *   1: The TFA98xx will be brought in power down state where it consumes minimal power. DSP is not running.
 *   0: The TFA98xx is allowed to start, provided an I2S clock is running.
 */
Tfa98xx_Error_t Tfa98xx_Powerdown(Tfa98xx_handle_t handle,
                  int powerdown);

/**
 * Resolved the incident reported from service layer.
 * For 97 a DSP reset is performed.
 * for all other device perform power down.
 * @param handle to opened instance
 * @param incidentlevel must be 1 or 0
 */
Tfa98xx_Error_t Tfa98xx_ResolveIncident(Tfa98xx_handle_t handle,
                       int incidentlevel);

/**
 * Tell the DSP that all parameters are set or normal operation mode can start.
 * This only has to be done once after a cold boot.
 * @param handle to opened instance
 */
Tfa98xx_Error_t Tfa98xx_SetConfigured(Tfa98xx_handle_t handle);

/**
 * Use this to select what had to be sent to the class D amplifier and speaker output.
 * The TFA98xx defaults to TFA98xx_AmpInputSel_DSP.
 * @param handle to opened instance
 * @param input_sel: channel selection, see Tfa98xx_AmpInputSel_t type in the header file
 *   -TFA98xx_AmpInputSel_I2SLeft: Use I2S left channel, bypassing DSP.
 *   -TFA98xx_AmpInputSel_I2SRight: Use I2S right channel, bypassing DSP.
 *   -TFA98xx_AmpInputSel_DSP: Use DSP output
 */
Tfa98xx_Error_t Tfa98xx_SelectAmplifierInput(Tfa98xx_handle_t handle,
                Tfa98xx_AmpInputSel_t
                input_sel);

/**
 * Select the signal to be put on the left channel of the I2S output.
 * The TFA98xx defaults to TFA98xx_I2SOutputSel_Amp.
 * @param handle to opened instance
 * @param output_sel: channel selection, see Tfa98xx_OutputSel_t in the header file
 *   -TFA98xx_I2SOutputSel_CurrentSense,
 *   -TFA98xx_I2SOutputSel_DSP_Gain,
 *   -TFA98xx_I2SOutputSel_DSP_AEC,
 *   -TFA98xx_I2SOutputSel_Amp,
 *   -TFA98xx_I2SOutputSel_DataI3R,
 *   -TFA98xx_I2SOutputSel_DataI3L,
 *   -TFA98xx_I2SOutputSel_DcdcFFwdCur
 */
Tfa98xx_Error_t Tfa98xx_SelectI2SOutputLeft(Tfa98xx_handle_t handle,
                        Tfa98xx_OutputSel_t
                        output_sel);

/**
 * Select the signal to be put on the right channel of the I2S output.
 * The TFA98xx defaults to TFA98xx_I2SOutputSel_CurrentSense.
 * @param handle to opened instance
 * @param output_sel: channel selection, see Tfa98xx_OutputSel_t in the header file
 *   -TFA98xx_I2SOutputSel_CurrentSense,
 *   -TFA98xx_I2SOutputSel_DSP_Gain,
 *   -TFA98xx_I2SOutputSel_DSP_AEC,
 *   -TFA98xx_I2SOutputSel_Amp,
 *   -TFA98xx_I2SOutputSel_DataI3R,
 *   -TFA98xx_I2SOutputSel_DataI3L,
 *   -TFA98xx_I2SOutputSel_DcdcFFwdCur
 */
Tfa98xx_Error_t Tfa98xx_SelectI2SOutputRight(Tfa98xx_handle_t handle,
                         Tfa98xx_OutputSel_t
                         output_sel);

/**
 * For stereo support, the 2 TFA98xx ICs need to communicate the gain applied with each other to ensure both remain similar.
 * This gain channel can be selected on the I2S DATAO using the TFA98xx_SelectI2SOutputLeft and Right functions.
 * This DATAO pin must be connected to the DATAI2 pin of the other IC.
 * This function is then used to specify on which channel of the DATAI2 pin the gain of the other IC is.
 * @param handle to opened instance
 * @param gain_sel: channel selection, see Tfa98xx_StereoGainSel_t in the header file
 *   -TFA98xx_StereoGainSel_left
 *   -TFA98xx_StereoGainSel_Right
 */
Tfa98xx_Error_t Tfa98xx_SelectStereoGainChannel(Tfa98xx_handle_t handle,
                        Tfa98xx_StereoGainSel_t
                        gain_sel);

/**
 * Control the volume of the DSP (deprecated).
 * A volume smaller than -127dB has the same effect as SetMute(TFA98xx_Mute_Digital).
 * The TFA98xx defaults to 0dB.
 * @param handle to opened instance
 * @param voldB Float value, in dB. Must be smaller or equal to 0dB
 */
Tfa98xx_Error_t Tfa98xx_SetVolume(Tfa98xx_handle_t handle,
                  FIXEDPT voldB);

/**
 * Control the volume I2C register.
 * @param handle to opened instance
 * @param vollevel volume in level.  must be between 0 and 255
 */
Tfa98xx_Error_t Tfa98xx_SetVolumeLevel(Tfa98xx_handle_t handle,
                unsigned short vollevel);

/**
 * Read the currently set volume.
 * @param handle to opened instance
 * @param *pVoldB: The volume in dB.
 */
Tfa98xx_Error_t Tfa98xx_GetVolume(Tfa98xx_handle_t handle,
                  FIXEDPT * pVoldB);

/**
 * Default sample rate of the TFA98xx is 48kHz. Another sample rate can be chosen.
 * Note that this only notifies the TFA98xx from the sample rate via I2C.
 * The I2S clock must match the set sample rate always when running.
 * @param handle to opened instance
 * @param rate Samplerate in Hz.  must be 32000, 44100 or 48000
 */
Tfa98xx_Error_t Tfa98xx_SetSampleRate(Tfa98xx_handle_t handle,
                      int rate);

/**
 * Returns the programmed sample rate.
 * @param handle to opened instance
 * @param pRate: Pointer, The sample rate in Hz i.e 32000, 44100 or 48000
 */
Tfa98xx_Error_t Tfa98xx_GetSampleRate(Tfa98xx_handle_t handle,
                      int *pRate);

/**
 * Select the I2S input channel the DSP should process.
 * The TFA98xx defaults to TFA98xx_Channel_L.
 * @param handle to opened instance
 * @param channel, channel selection: see Tfa98xx_Channel_t in the header file
 *   - TFA98xx_Channel_L: I2S left channel
 *   - TFA98xx_Channel_R: I2S right channel
 *   - TFA98xx_Channel_L_R: I2S (left + right channel)/2
 */
Tfa98xx_Error_t Tfa98xx_SelectChannel(Tfa98xx_handle_t handle,
                      Tfa98xx_Channel_t channel);

/**
 * Select the device mode.
 * The TFA98xx defaults to TFA98xx_Mode_Normal.
 * @param handle to opened instance
 * @param mode, mode selection: see Tfa98xx_Mode_t in the header file
 *   - TFA98xx_Mode_Normal: Normal operating mode
 *   - TFA98xx_Mode_RCV: Receiver operating mode
 */
Tfa98xx_Error_t Tfa98xx_SelectMode(Tfa98xx_handle_t handle,
                      Tfa98xx_Mode_t mode);

/**
 * Enabling the AEC output.
 */
Tfa98xx_Error_t
Tfa98xx_EnableAECOutput(Tfa98xx_handle_t handle);

/**
 * Disabling the AEC output to prevent leakage current.
 */
Tfa98xx_Error_t
Tfa98xx_DisableAECOutput(Tfa98xx_handle_t handle);

/**
 * A soft mute/unmute is executed, the duration of which depends on an advanced parameter.
 * NOTE: before going to powerdown mode, the amplifier should be stopped first to ensure clean transition without artifacts.
 * Digital silence or mute is not sufficient.
 * The TFA98xx defaults to TFA98xx_Mute_Off.
 * @param handle to opened instance
 * @param mute: see Tfa98xx_Mute_t type in the header file
 *   - TFA98xx_Mute_Off: leave the muted state.
 *   - TFA98xx_Mute_Digital: go to muted state, but the amplifier keeps running.
 *   - TFA98xx_Mute_Amplifier: go to muted state and stop the amplifier.  This will consume lowest power.
 *     This is also the mute state to be used when powering down or changing the sample rate.
 */
Tfa98xx_Error_t Tfa98xx_SetMute(Tfa98xx_handle_t handle,
                Tfa98xx_Mute_t mute);

Tfa98xx_Error_t Tfa98xx_GetMute(Tfa98xx_handle_t handle,
                Tfa98xx_Mute_t *pMute);

/**
 * Supported Digital Audio Interfaces.
 * @param handle to opened instance
 * @param *daimap: Pointer to the bitmap of the supported Digital Audio Interfaces
 * @return error code.
 */
Tfa98xx_Error_t Tfa98xx_SupportedDAI(Tfa98xx_handle_t handle, Tfa98xx_DAI_t *daimap);

/**
 * Yields the number of parameters to be used in Tfa98xx_DspWriteConfig().
 * @param handle to opened instance
 * @param *pParamCount: Pointer to parameter count.
 * @return error code. Only assigned if return value == Tfa98xx_Error_Ok
 */
Tfa98xx_Error_t Tfa98xx_DspConfigParameterCount(Tfa98xx_handle_t handle,
                        int *pParamCount);

/**
 * Set or clear DSP reset signal.
 * @param handle to opened instance
 * @param state: Requested state of the reset signal
 */
Tfa98xx_Error_t Tfa98xx_DspReset(Tfa98xx_handle_t handle, int state);

/**
 * Check the state of the DSP subsystem to determine if the subsystem is ready for access.
 * Normally this function is called after a powerup or reset when the higher layers need to assure that the subsystem can be safely accessed.
 * @param handle to opened instance
 * @param ready pointer to state flag
 * @return Non-zero if stable
 */
Tfa98xx_Error_t Tfa98xx_DspSystemStable(Tfa98xx_handle_t handle,
                    int *ready);

/* The following functions can only be called when the DSP is running
 * - I2S clock must be active,
 * - IC must be in operating mode
 */

/**
 * The TFA98XX has provision to patch the ROM code. This API function provides a means to do this.
 * When a patch is needed, NXP will provide a file. The contents of that file have to be passed this this API function.
 * As patching requires access to the DSP, this is only possible when the TFA98xx has left powerdown mode.
 * @param handle to opened instance
 * @param patchLength: size of the patch file
 * @param *patchBytes: Pointer to Array of bytes, the contents of the patch file
 */
Tfa98xx_Error_t Tfa98xx_DspPatch(Tfa98xx_handle_t handle,
                 int patchLength,
                 const unsigned char *patchBytes);

/**
 * Check whether the DSP expects tCoef or tCoefA as last parameter in the speaker parameters.
 * @param handle to opened instance
 * @param *pbSupporttCoef: =1 when DSP expects tCoef,
 *           *pbSupporttCoef  =0 when it expects tCoefA (and the elaborate workaround to calculate tCoefA from tCoef on the host)
 */
Tfa98xx_Error_t Tfa98xx_DspSupporttCoef(Tfa98xx_handle_t handle,
                    int *pbSupporttCoef);

/**
 * Return the device revision id.
 * @param handle to opened instance
 */
unsigned short Tfa98xx_GetDeviceRevision(Tfa98xx_handle_t handle);


/**
 * The config contains the one time setup parameters. It must be called once after initialization.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pConfigBytes: Opaque array of Length bytes.
 * e.g. read from a .config file that was generated by the Pico GUI.
 * The config actually contains takes system parameters that don’t require runtime changing.
 * The parameters are sample rate independent, so when changing sample rates it is no required to load a different config.
 * For more details about the config parameters, see §0.
 */
Tfa98xx_Error_t Tfa98xx_DspWriteConfig(Tfa98xx_handle_t handle,
                       int length, const unsigned char
                       *pConfigBytes);

/**
 * Get the system wide parameters.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pConfigBytes: Opaque array of Length bytes.
 */
Tfa98xx_Error_t Tfa98xx_DspReadConfig(Tfa98xx_handle_t handle,
                      int length,
                      unsigned char *pConfigBytes);

/**
 * Use this for a free speaker or when saving the output of the online estimation process across cold starts (see use case in chapter 4).
 * The exact content is considered advanced parameters and detailed knowledge of this is not needed to use the API.
 * NXP will give support for determining this for 3rd party speakers.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pSpeakerBytes: Opaque array of Length bytes, read from a .speaker file that was generated by the Pico GUI
 */
Tfa98xx_Error_t Tfa98xx_DspWriteSpeakerParameters(
                Tfa98xx_handle_t handle,
                int length,
                const unsigned char *pSpeakerBytes);

/**
 * Optional: can be used to read and save the current speaker model, to reload later with the TFA98xx_WriteSpeakerParameters function.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pSpeakerBytes: Pointer to a buffer of Length bytes
 * @return Speaker parameters filled into the buffer
 */
Tfa98xx_Error_t Tfa98xx_DspReadSpeakerParameters(
                Tfa98xx_handle_t handle,
                int length,
                unsigned char *pSpeakerBytes);

/**
 * This will return the actual speaker excursion model.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pSpeakerBytes: Pointer to a buffer of Length bytes
 * @return the actual speaker excursion model raw data
*/
Tfa98xx_Error_t Tfa98xx_DspReadExcursionModel(
                Tfa98xx_handle_t handle,
                int length,
                unsigned char *pSpeakerBytes);

/**
 * This API function loading a predefined preset from a file.
 * The parameters are sample rate independent, so when changing sample rates it is no required to load a different preset.
 * It is allowed to be called while audio is playing, so not needed to mute.
 * For more details about the preset parameters, see §6.1.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pPresetBytes: Opaque array of Length bytes, e.g. read from a .preset file that was generated by the Pico GUI
 */
Tfa98xx_Error_t Tfa98xx_DspWritePreset(Tfa98xx_handle_t handle,
                       int length, const unsigned char
                       *pPresetBytes);
/**
 * Get the system wide parameters.
 * @param handle to opened instance
 * @param length: the size of the array in bytes
 * @param *pPresetBytes: Opaque array of Length bytes
 */
Tfa98xx_Error_t Tfa98xx_DspReadPreset(Tfa98xx_handle_t handle,
                      int length,
                      unsigned char *pPresetBytes);


/**
 * Set the biquad coefficient for the indicated biquad filter (index).
 * This function is supported for kernel.
 * @param handle to opened instance
 * @param biquad_index
 * @param *pBiquadBytes
 */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeffBytes(
                Tfa98xx_handle_t handle,
                int biquad_index,
                const unsigned char *pBiquadBytes);

/**
 * @param handle_cnt
 * @param handles[]
 * @param biquad_index
 * @param *pBiquadBytes
 */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeffMultipleBytes(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int biquad_index,
                const unsigned char *pBiquadBytes);

/**
 * Disable a certain biquad.
 * @param handle to opened instance
 * @param biquad_index: 1-10 of the biquad that needs to be adressed
*/
Tfa98xx_Error_t Tfa98xx_DspBiquad_Disable(Tfa98xx_handle_t handle,
                      int biquad_index);

/**
 * @param handle_cnt
 * @param handles[]
 * @param biquad_index
 */
Tfa98xx_Error_t Tfa98xx_DspBiquad_DisableMultiple(int handle_cnt,
                          Tfa98xx_handle_t
                          handles[],
                          int biquad_index);

/**
 * This will read out the calibrated speaker resistance, which is the DC resistance at 25 degrees Celsius.
 * @param handle to opened instance
 * @param *pRe25:
 * @return The calibration resistance in Ohm
 * When no calibration results (yet), the returned resistance is 0.0 Ohm.
*/
Tfa98xx_Error_t Tfa98xx_DspGetCalibrationImpedance(Tfa98xx_handle_t
                           handle,
                           FIXEDPT *pRe25);

/**
 * This will read out the actual values of the gain, output level, speaker temperature, speaker excursion, speaker resistance and various flags.
 * It is used for instance to fill in the bulk of the traces and flags in the Pico GUI.
 * @param handle to opened instance
 * @param *pInfo: Pointer to the buffer where to store the StateInfo
 * @return The state info, of type TFA98xx_StateInfo_t, see §6.2.
*/
Tfa98xx_Error_t Tfa98xx_DspGetStateInfo(
                Tfa98xx_handle_t handle,
                Tfa98xx_StateInfo_t *pInfo);

/**
 * @param handle to opened instance
 * @param *bytes
 */
Tfa98xx_Error_t Tfa98xx_DspSetCalibrationImpedance(
                Tfa98xx_handle_t handle,
                const unsigned char *bytes);

/**
 * Optimized SetParam type of functions that allows writing the same parameters to multiple device with minimal delay between the devices.
 */
Tfa98xx_Error_t Tfa98xx_DspWriteConfigMultiple(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int length,
                const unsigned char *pConfigBytes);
Tfa98xx_Error_t Tfa98xx_DspWriteSpeakerParametersMultiple(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int length,
                const unsigned char *pSpeakerBytes);
Tfa98xx_Error_t Tfa98xx_DspWritePresetMultiple(
                int handle_cnt,
                Tfa98xx_handle_t
                handles[], int length,
                const unsigned char *pPresetBytes);


Tfa98xx_Error_t Tfa98xx_SetToneDetectionOff(Tfa98xx_handle_t handle);

/**
 * DSP Biquad use floating point - which is not supported in kernel.
 * Use ifndef to bypass this API and consider using Tfa98xx_DspBiquad_SetCoeffBytes.
 *
 */

/**
 * Load all biquads.  The coefficients are in the same order as in the Tfa98xx_DspBiquad_SetCoeff function b0, b1, ...
 * The TFA98XX contains 10 fully programmable cascaded second-order biquad Infinite Impulse Response (IIR) filter sections have been integrated into the DSP.
 * The biquad filtering is implemented in 24 bit precision according to the following equation:
 *
 * \f$H(z) = (b_0+b_1Z^-1+b_2Z^-2)/(1+a_1Z^-1+a_2Z^-2)\f$
 *
 * By default the filters are disabled (don’t consume CPU cycles).
 * Processing occurs on blocks of 32 samples for performance reasons (same as SpeakerBoost protection).
 * Using the biquads introduces no additional buffering delay in the TFA98XX.
 * The host processor is also responsible for including headroom in the coefficients such that 0dBFS is not exceeded during the filtering.
 * @param handle to opened instance
 * @param biquad_index: 1-10 of the biquad that needs te be addressed
 * @param b0: b0 coefficient, floating point value
 * @param b1
 * @param b2
 * @param a1
 * @param a2
 *
 * This will be converted to fixed point coefficients for the DSP,
 * including a coefficient scaling value to allow coefficients > 1.0.
 *
 * Note: the biquads numbered 1 and 2 are calculated in double precision on the DSP and as such are recommended for the low frequency filters.
 */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeff(Tfa98xx_handle_t handle,
                       int biquad_index, FIXEDPT b0,
                       FIXEDPT b1, FIXEDPT b2,
                       FIXEDPT a1, FIXEDPT a2);

/**
 * Use this function to set all 10 biquads in 1 RPC call, which is faster if multiple biquads are to be changed.
 * @param handle to opened instance
 * @param coef: array of 10*5 parameters in the order b0, b1, b2, a1, a2, for each of the 10 biquads
 */
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetAllCoeff(Tfa98xx_handle_t handle,
                          FIXEDPT
                          coef[TFA98XX_BIQUAD_NUM *
                           5]);

Tfa98xx_Error_t Tfa98xx_DspBiquad_SetCoeffMultiple(int handle_cnt,
                           Tfa98xx_handle_t
                           handles[],
                           int biquad_index,
                           FIXEDPT b0,
                           FIXEDPT b1,
                           FIXEDPT b2,
                           FIXEDPT a1,
                           FIXEDPT a2);
Tfa98xx_Error_t Tfa98xx_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
                              Tfa98xx_handle_t
                              handles[],
            FIXEDPT coef[TFA98XX_BIQUAD_NUM * 5]);

/* low level routines, not part of official API and might be removed in the future */

/**
 * This API function allows reading an arbitrary I2C register of the TFA98xx.
 * @param handle to opened instance
 * @param subaddress: 8 bit subaddress of the I2C register
 * @param *pValue
 * @return 16 bit value that was stored in the selected I2C register
 */
Tfa98xx_Error_t Tfa98xx_ReadRegister16(Tfa98xx_handle_t handle,
                       unsigned char subaddress,
                       unsigned short *pValue);

/**
 * This API function allows writing an arbitrary I2C register of the TFA98XX.
 * @param handle to opened instance
 * @param subaddress: 8 bit subaddress of the I2C register
 * @param value: 16 bit value to be stored in the selected I2C register
 */
Tfa98xx_Error_t Tfa98xx_WriteRegister16(Tfa98xx_handle_t handle,
                    unsigned char subaddress,
                    unsigned short value);

Tfa98xx_Error_t Tfa98xx_DspReadMem(Tfa98xx_handle_t handle,
                   unsigned short start_offset,
                   int num_words, int *pValues);

Tfa98xx_Error_t Tfa98xx_DspWriteMem(Tfa98xx_handle_t handle,
                    unsigned short address,
                                    int value, int memtype);

/**
 * Allow any Set RPC command to any module, typically used for lower level control of the DSP (diagnostic purposes).
 * @param handle to opened instance
 * @param module_id: id of the module to be addressed
 * @param param_id: id of the function call within the module
 * @param num_bytes: number of bytes data, depends on the module_id and the param_id
 * @param data: array of num_bytes bytes to be send
 */
Tfa98xx_Error_t Tfa98xx_DspSetParam(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id,
                    int num_bytes,
                    const unsigned char data[]);

Tfa98xx_Error_t Tfa98xx_DspSetParamMultiple(int handle_cnt,
                        Tfa98xx_handle_t handles[],
                        unsigned char module_id,
                        unsigned char param_id,
                        int num_bytes,
                        const unsigned char data[]);

/**
 * Allow any Get RPC command to any module, typically used for reading internal data of the DSP (e.g. traces).
 * @param handle to opened instance
 * @param module_id: id of the module to be addressed
 * @param param_id: id of the function call within the module
 * @param num_bytes: number of bytes data, depends on the module_id and the param_id
 * @param data: buffer for the array of num_bytes bytes that will be read
 * Cannot be used when handle refers to a device opened with generic I2C slave address.
 * This function will return TFA98xx_Error_Bad_Parameter in that case.
 */
Tfa98xx_Error_t Tfa98xx_DspGetParam(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id,
                    int num_bytes,
                    unsigned char data[]);

Tfa98xx_Error_t Tfa98xx_DspExecuteRpc(Tfa98xx_handle_t handle,
              unsigned char module_id,
              unsigned char param_id, int num_inbytes,
              unsigned char indata[], int num_outbytes,
              unsigned char outdata[]);

Tfa98xx_Error_t Tfa98xx_ReadData(Tfa98xx_handle_t handle,
                 unsigned char subaddress,
                 int num_bytes, unsigned char data[]);

Tfa98xx_Error_t Tfa98xx_WriteData(Tfa98xx_handle_t handle,
                  unsigned char subaddress,
                  int num_bytes,
                  const unsigned char data[]);

Tfa98xx_Error_t Tfa98xx_DspWriteMemory(Tfa98xx_handle_t handle,
                       enum Tfa98xx_DMEM which_mem,
                       unsigned short start_offset,
                       int num_words, int *pValues);

Tfa98xx_Error_t Tfa98xx_DspReadMemory(Tfa98xx_handle_t handle,
                      enum Tfa98xx_DMEM which_mem,
                      unsigned short start_offset,
                      int num_words, int *pValues);

Tfa98xx_Error_t Tfa98xx_CheckDeviceFeatures(Tfa98xx_handle_t handle);

/**
 * If needed, this API function can be used to get a text version of the error code
 * @param error: An error code of type TFA98xx_Error_t
 * @return Pointer to a text version of the error code
 */
const char *Tfa98xx_GetErrorString(Tfa98xx_Error_t error);

#ifdef __cplusplus
}
#endif
#endif                /* TFA98API_H */
