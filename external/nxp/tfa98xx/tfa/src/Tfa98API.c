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

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#else
#include <assert.h>
#include <string.h>
#include <math.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "Tfa98xx_internals.h"
#include "Tfa98xx_Registers.h"
#include "Tfa98API.h"
#include "Tfa98xx.h"

#include "initTfa9890.h"

/**
 * Return the maximum nr of devices (SC39786)
 */
int Tfa98xx_MaxDevices(void)
{
    return tfa98xx_max_devices();
}

int handle_is_open(Tfa98xx_handle_t h)
{
    return tfa98xx_handle_is_open(h);
}

/* the patch contains a header with the following
 * IC revision register: 1 byte, 0xFF means don't care
 * XMEM address to check: 2 bytes, big endian, 0xFFFF means don't care
 * XMEM value to expect: 3 bytes, big endian
 */
static Tfa98xx_Error_t
checkICROMversion(Tfa98xx_handle_t handle, const unsigned char patchheader[])
{
    return tfa98xx_check_ic_rom_version(handle, patchheader);
}

Tfa98xx_Error_t
Tfa98xx_DspGetSwFeatureBits(Tfa98xx_handle_t handle, int features[2])
{
    return tfa98xx_dsp_get_sw_feature_bits(handle, features);
}

Tfa98xx_Error_t
Tfa98xx_Open(unsigned char slave_address, Tfa98xx_handle_t *pHandle)
{
    return tfa98xx_open(slave_address, pHandle);
}

Tfa98xx_Error_t Tfa98xx_Close(Tfa98xx_handle_t handle)
{
    return tfa98xx_close(handle);
}

/* Tfa98xx_DspConfigParameterCount
 * Yields the number of parameters to be used in Tfa98xx_DspWriteConfig()
 */
Tfa98xx_Error_t Tfa98xx_DspConfigParameterCount(Tfa98xx_handle_t handle,
                        int *pParamCount)
{
    return tfa98xx_dsp_config_parameter_count(handle, pParamCount);
}

/* Tfa98xx_DspReset
 *  set or clear DSP reset signal
 */
 Tfa98xx_Error_t Tfa98xx_DspReset(Tfa98xx_handle_t handle, int state)
{
    return tfa98xx_dsp_reset(handle, state);
}

/* Tfa98xx_DspSystemStable
 *  return: *ready = 1 when clocks are stable to allow DSP subsystem access
 */
Tfa98xx_Error_t Tfa98xx_DspSystemStable(Tfa98xx_handle_t handle, int *ready)
{
    int dspstatus;
    enum Tfa98xx_Error error;

    error = tfa98xx_dsp_system_stable(handle, &dspstatus);
    *ready = dspstatus;
    return error;
}

/* return the device revision id
 */
unsigned short Tfa98xx_GetDeviceRevision(Tfa98xx_handle_t handle)
{
    /* local function. Caller must make sure handle is valid */
    return tfa98xx_get_device_revision(handle);
}

Tfa98xx_Error_t Tfa98xx_Init(Tfa98xx_handle_t handle)
{
    return tfa98xx_init(handle);
}

static Tfa98xx_Error_t
processPatchFile(Tfa98xx_handle_t handle, int length,
         const unsigned char *bytes)
{
    return tfa98xx_process_patch_file(handle, length, bytes);
}

/*
 * write a 16 bit subaddress
 */
Tfa98xx_Error_t
Tfa98xx_WriteRegister16(Tfa98xx_handle_t handle,
            unsigned char subaddress, unsigned short value)
{
    return tfa98xx_write_register16(handle, subaddress, value);
}

Tfa98xx_Error_t
Tfa98xx_DspSupportFramework(Tfa98xx_handle_t handle, int *pbSupportFramework)
{

    /* *pbSupportFramework only changed when error == Tfa98xx_Error_Ok */
    return tfa98xx_dsp_support_framework(handle, pbSupportFramework);
}

/*
 * Write all the bytes specified by num_bytes and data
 */
Tfa98xx_Error_t
Tfa98xx_WriteData(Tfa98xx_handle_t handle,
          unsigned char subaddress, int num_bytes,
          const unsigned char data[])
{
    return tfa98xx_write_data(handle,
          subaddress, num_bytes,
          data);
}

Tfa98xx_Error_t
Tfa98xx_ReadRegister16(Tfa98xx_handle_t handle,
               unsigned char subaddress, unsigned short *pValue)
{
    return tfa98xx_read_register16(handle, subaddress, pValue);
}

Tfa98xx_Error_t
Tfa98xx_ReadData(Tfa98xx_handle_t handle,
         unsigned char subaddress, int num_bytes, unsigned char data[])
{
    return tfa98xx_read_data(handle, subaddress, num_bytes, data);
}

Tfa98xx_Error_t Tfa98xx_CheckDeviceFeatures(Tfa98xx_handle_t handle)
{
    return tfa98xx_check_device_features(handle);
}

Tfa98xx_Error_t Tfa98xx_Powerdown(Tfa98xx_handle_t handle, int powerdown)
{
    return tfa98xx_powerdown(handle, powerdown);
}

Tfa98xx_Error_t Tfa98xx_ResolveIncident(Tfa98xx_handle_t handle,
                       int incidentlevel)
{
    return tfa98xx_resolve_incident(handle, incidentlevel);
}

Tfa98xx_Error_t Tfa98xx_SetConfigured(Tfa98xx_handle_t handle)
{
    return tfa98xx_set_configured(handle);
}

Tfa98xx_Error_t
Tfa98xx_SelectAmplifierInput(Tfa98xx_handle_t handle,
                 Tfa98xx_AmpInputSel_t input_sel)
{
    return tfa98xx_select_amplifier_input(handle, input_sel);
}
Tfa98xx_Error_t
Tfa98xx_SelectI2SOutputLeft(Tfa98xx_handle_t handle,
                Tfa98xx_OutputSel_t output_sel)
{
    return tfa98xx_select_i2s_output_left(handle, output_sel);
}


Tfa98xx_Error_t
Tfa98xx_SelectI2SOutputRight(Tfa98xx_handle_t handle,
                 Tfa98xx_OutputSel_t output_sel)
{
    return tfa98xx_select_i2s_output_right(handle, output_sel);
}

Tfa98xx_Error_t
Tfa98xx_SelectStereoGainChannel(Tfa98xx_handle_t handle,
                Tfa98xx_StereoGainSel_t gain_sel)
{
    return tfa98xx_select_stereo_gain_channel(handle, gain_sel);
}


Tfa98xx_Error_t
Tfa98xx_SelectChannel(Tfa98xx_handle_t handle, Tfa98xx_Channel_t channel)
{
    return tfa98xx_select_channel(handle, channel);
}

Tfa98xx_Error_t
Tfa98xx_SelectMode(Tfa98xx_handle_t handle, Tfa98xx_Mode_t mode)
{
    return tfa98xx_select_mode(handle, mode);
}

Tfa98xx_Error_t
Tfa98xx_EnableAECOutput(Tfa98xx_handle_t handle)
{
    return tfa98xx_enable_aecoutput(handle);
}

Tfa98xx_Error_t
Tfa98xx_DisableAECOutput(Tfa98xx_handle_t handle)
{
    return tfa98xx_disable_aecoutput(handle);
}

Tfa98xx_Error_t Tfa98xx_SetSampleRate(Tfa98xx_handle_t handle, int rate)
{
    return tfa98xx_set_sample_rate(handle, rate);
}

Tfa98xx_Error_t Tfa98xx_GetSampleRate(Tfa98xx_handle_t handle, int *pRate)
{
    return tfa98xx_get_sample_rate(handle, pRate);
}


Tfa98xx_Error_t
Tfa98xx_DspWriteSpeakerParameters(Tfa98xx_handle_t handle,
                  int length,
                  const unsigned char *pSpeakerBytes)
{

    return tfa98xx_dsp_write_speaker_parameters(handle,
                  length, pSpeakerBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspWriteSpeakerParametersMultiple(int handle_cnt,
                      Tfa98xx_handle_t
                      handles[],
                      int length,
                      const unsigned char *pSpeakerBytes)
{
    return tfa98xx_dsp_write_speaker_parameters_multiple(handle_cnt,
                      handles,
                      length, pSpeakerBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspReadSpeakerParameters(Tfa98xx_handle_t handle,
                 int length, unsigned char *pSpeakerBytes)
{
    return tfa98xx_dsp_read_spkr_params(handle,
                     SB_PARAM_GET_LSMODEL, length,
                     pSpeakerBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspReadExcursionModel(Tfa98xx_handle_t handle,
                  int length, unsigned char *pSpeakerBytes)
{
    return tfa98xx_dsp_read_spkr_params(handle,
                     SB_PARAM_GET_XMODEL, length,
                     pSpeakerBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspReadSpkrParams(Tfa98xx_handle_t handle,
              unsigned char paramId,
              int length, unsigned char *pSpeakerBytes)
{
    return tfa98xx_dsp_read_spkr_params(handle,
              paramId,
              length, pSpeakerBytes);
}

Tfa98xx_Error_t Tfa98xx_DspSupporttCoef(Tfa98xx_handle_t handle,
                    int *pbSupporttCoef)
{
    return tfa98xx_dsp_support_tcoef(handle, pbSupporttCoef);
}


Tfa98xx_Error_t
Tfa98xx_DspWriteConfig(Tfa98xx_handle_t handle, int length,
               const unsigned char *pConfigBytes)
{
    return tfa98xx_dsp_write_config(handle, length, pConfigBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspWriteConfigMultiple(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   int length, const unsigned char *pConfigBytes)
{
    return tfa98xx_dsp_write_config_multiple(handle_cnt,
                   handles,
                   length, pConfigBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspWritePreset(Tfa98xx_handle_t handle, int length,
               const unsigned char *pPresetBytes)
{
    return tfa98xx_dsp_write_preset(handle, length, pPresetBytes);
}

Tfa98xx_Error_t Tfa98xx_DspReadConfig(Tfa98xx_handle_t handle, int length,
                      unsigned char *pConfigBytes)
{
    return tfa98xx_dsp_read_config(handle, length,
                      pConfigBytes);
}

Tfa98xx_Error_t Tfa98xx_DspReadPreset(Tfa98xx_handle_t handle, int length,
                      unsigned char *pPresetBytes)
{

    return tfa98xx_dsp_read_preset(handle, length,
                      pPresetBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspWritePresetMultiple(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   int length, const unsigned char *pPresetBytes)
{
    return tfa98xx_dsp_write_preset_multiple(handle_cnt,
                   handles,
                   length, pPresetBytes);
}

Tfa98xx_Error_t Tfa98xx_SetVolume(Tfa98xx_handle_t handle, FIXEDPT voldB)
{
    return tfa98xx_set_volume(handle, voldB);
}

Tfa98xx_Error_t Tfa98xx_SetVolumeLevel(Tfa98xx_handle_t handle, unsigned short vollevel)
{
    return tfa98xx_set_volume_level(handle, vollevel);
}

Tfa98xx_Error_t Tfa98xx_GetVolume(Tfa98xx_handle_t handle, FIXEDPT *pVoldB)
{
    return tfa98xx_get_volume(handle, pVoldB);
}

Tfa98xx_Error_t Tfa98xx_SetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t mute)
{
    return tfa98xx_set_mute(handle, mute);
}

Tfa98xx_Error_t Tfa98xx_GetMute(Tfa98xx_handle_t handle, Tfa98xx_Mute_t *pMute)
{
    return tfa98xx_get_mute(handle, pMute);
}

Tfa98xx_Error_t Tfa98xx_SupportedDAI(Tfa98xx_handle_t handle, Tfa98xx_DAI_t *daimap)
{
    return tfa98xx_supported_dai(handle, daimap);
}

void Tfa98xx_ConvertData2Bytes(int num_data, const int data[],
                   unsigned char bytes[])
{
    tfa98xx_convert_data2bytes(num_data, data,
                   bytes);

}

/*
 * Floating point calculations must be done in user-space
 */
static Tfa98xx_Error_t
calcBiquadCoeff(float b0, float b1, FIXEDPT b2,
        float a1, float a2, unsigned char bytes[BIQUAD_COEFF_SIZE * 3])
{
    float max_coeff;
    int headroom;
    int coeff_buffer[BIQUAD_COEFF_SIZE];
    /* find max value in coeff to define a scaler */
#ifdef __KERNEL__
    float mask;
    max_coeff = abs(b0);
    if (abs(b1) > max_coeff)
        max_coeff = abs(b1);
    if (abs(b2) > max_coeff)
        max_coeff = abs(b2);
    if (abs(a1) > max_coeff)
        max_coeff = abs(a1);
    if (abs(a2) > max_coeff)
        max_coeff = abs(a2);
    /* round up to power of 2 */
    mask = 0x0040000000000000;
    for (headroom = 23; headroom > 0; headroom--) {
        if (max_coeff & mask)
            break;
        mask >>= 1;
    }
#else
    max_coeff = (float)fabs(b0);
    if (fabs(b1) > max_coeff)
        max_coeff = (float)fabs(b1);
    if (fabs(b2) > max_coeff)
        max_coeff = (float)fabs(b2);
    if (fabs(a1) > max_coeff)
        max_coeff = (float)fabs(a1);
    if (fabs(a2) > max_coeff)
        max_coeff = (float)fabs(a2);
    /* round up to power of 2 */
    headroom = (int)ceil(log(max_coeff + pow(2.0, -23)) / log(2.0));
#endif
    /* some sanity checks on headroom */
    if (headroom > 8)
        return Tfa98xx_Error_Bad_Parameter;
    if (headroom < 0)
        headroom = 0;
    /* set in correct order and format for the DSP */
    coeff_buffer[0] = headroom;
#ifdef __KERNEL__
    coeff_buffer[1] = (int) TO_INT(-a2 * (1 << (23 - headroom)));
    coeff_buffer[2] = (int) TO_INT(-a1 * (1 << (23 - headroom)));
    coeff_buffer[3] = (int) TO_INT(b2 * (1 << (23 - headroom)));
    coeff_buffer[4] = (int) TO_INT(b1 * (1 << (23 - headroom)));
    coeff_buffer[5] = (int) TO_INT(b0 * (1 << (23 - headroom)));
#else
    coeff_buffer[1] = (int) (-a2 * pow(2.0, 23 - headroom));
    coeff_buffer[2] = (int) (-a1 * pow(2.0, 23 - headroom));
    coeff_buffer[3] = (int) (b2 * pow(2.0, 23 - headroom));
    coeff_buffer[4] = (int) (b1 * pow(2.0, 23 - headroom));
    coeff_buffer[5] = (int) (b0 * pow(2.0, 23 - headroom));
#endif

    /* convert to fixed point and then bytes suitable for
     * transmission over I2C */
    tfa98xx_convert_data2bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
    return Tfa98xx_Error_Ok;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeff(Tfa98xx_handle_t handle,
               int biquad_index, FIXEDPT b0,
               FIXEDPT b1, FIXEDPT b2, FIXEDPT a1, FIXEDPT a2)
{
    enum Tfa98xx_Error error;
    unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;
    error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
    if (error == Tfa98xx_Error_Ok) {
        error =
            tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (BIQUAD_COEFF_SIZE * 3), bytes);
    }
    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeffMultiple(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   int biquad_index, FIXEDPT b0,
                   FIXEDPT b1, FIXEDPT b2, FIXEDPT a1,
                   FIXEDPT a2)
{
    enum Tfa98xx_Error error;
    unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;
    error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
    if (error == Tfa98xx_Error_Ok) {
        error = tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                    MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (BIQUAD_COEFF_SIZE * 3), bytes);
    }
    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetAllCoeff(Tfa98xx_handle_t handle,
                  FIXEDPT b[TFA98XX_BIQUAD_NUM * 5])
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int i;
    unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
    for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
         ++i) {
        error =
            calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
                    b[i * 5 + 3], b[i * 5 + 4],
                    bytes + i * BIQUAD_COEFF_SIZE * 3);
    }
    if (error == Tfa98xx_Error_Ok) {
        error =
            tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                    0 /* program all at once */ ,
                    (unsigned char)(BIQUAD_COEFF_SIZE *
                        TFA98XX_BIQUAD_NUM * 3),
                    bytes);
    }
    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
                      Tfa98xx_handle_t
                      handles[],
                      FIXEDPT b[TFA98XX_BIQUAD_NUM * 5])
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int i;
    unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
    for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
         ++i) {
        error =
            calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
                    b[i * 5 + 3], b[i * 5 + 4],
                    bytes + i * BIQUAD_COEFF_SIZE * 3);
    }
    if (error == Tfa98xx_Error_Ok) {
        error =
            tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                        MODULE_BIQUADFILTERBANK,
                        0 /* program all at once */ ,
                        (unsigned
                         char)(BIQUAD_COEFF_SIZE *
                               TFA98XX_BIQUAD_NUM * 3),
                        bytes);
    }
    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeffBytes(Tfa98xx_handle_t handle,
                int biquad_index,
                const unsigned char *pBiquadBytes)
{
    return tfa98xx_dsp_biquad_set_coeff_bytes(handle,
                biquad_index, pBiquadBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_SetCoeffMultipleBytes(int handle_cnt,
                    Tfa98xx_handle_t handles[],
                    int biquad_index,
                    const unsigned char *pBiquadBytes)
{
    return tfa98xx_dsp_biquad_set_coeff_multiple_bytes(handle_cnt,
                    handles, biquad_index,
                    pBiquadBytes);
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_Disable(Tfa98xx_handle_t handle, int biquad_index)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int coeff_buffer[BIQUAD_COEFF_SIZE];
    unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;
    /* set in correct order and format for the DSP */
    coeff_buffer[0] = (int) - 8388608;    /* -1.0f */
    coeff_buffer[1] = 0;
    coeff_buffer[2] = 0;
    coeff_buffer[3] = 0;
    coeff_buffer[4] = 0;
    coeff_buffer[5] = 0;
    /* convert to fixed point and then bytes suitable for
     * transmaission over I2C */
    tfa98xx_convert_data2bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
    error =
        tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                (unsigned char)biquad_index,
                (unsigned char)(BIQUAD_COEFF_SIZE * 3), bytes);
    return error;
}

Tfa98xx_Error_t
Tfa98xx_DspBiquad_DisableMultiple(int handle_cnt,
                  Tfa98xx_handle_t handles[], int biquad_index)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int coeff_buffer[BIQUAD_COEFF_SIZE];
    unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;

    /* set in correct order and format for the DSP */
    coeff_buffer[0] = (int) - 8388608;    /* -1.0f */
    coeff_buffer[1] = 0;
    coeff_buffer[2] = 0;
    coeff_buffer[3] = 0;
    coeff_buffer[4] = 0;
    coeff_buffer[5] = 0;
    /* convert to fixed point and then bytes suitable for
     * transmaission over I2C */
    Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
    /*
    error =
        Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
                    MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (unsigned char)BIQUAD_COEFF_SIZE * 3,
                    bytes);
    */
    error = tfa98xx_dsp_biquad_disable_multiple(handle_cnt, handles,
                    biquad_index, bytes);
    return error;
}

#define PATCH_HEADER_LENGTH 6
Tfa98xx_Error_t
Tfa98xx_DspPatch(Tfa98xx_handle_t handle, int patchLength,
         const unsigned char *patchBytes)
{
    return tfa98xx_dsp_patch(handle, patchLength, patchBytes);
}

/* Execute RPC protocol to write something to the DSP */
Tfa98xx_Error_t
Tfa98xx_DspSetParamVarWait(Tfa98xx_handle_t handle,
               unsigned char module_id,
               unsigned char param_id, int num_bytes,
               const unsigned char data[], int waitRetryCount)
{
    return tfa98xx_dsp_set_param_var_wait(handle,
               module_id,
               param_id, num_bytes,
               data, waitRetryCount);
}

/* Execute RPC protocol to write something to the DSP */
Tfa98xx_Error_t
Tfa98xx_DspSetParam(Tfa98xx_handle_t handle,
            unsigned char module_id,
            unsigned char param_id, int num_bytes,
            const unsigned char data[])
{
    /* Use small WaitResult retry count */
    return tfa98xx_dsp_set_param_var_wait(handle, module_id, param_id,
                      num_bytes, data,
                      TFA98XX_WAITRESULT_NTRIES);
}

/* Execute RPC protocol to write something to all the DSPs interleaved,
 * stop at the first error. optimized to minimize the latency between the
 * execution point on the various DSPs */
Tfa98xx_Error_t
Tfa98xx_DspSetParamMultipleVarWait(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   unsigned char module_id,
                   unsigned char param_id,
                   int num_bytes, const unsigned char data[],
                   int waitRetryCount)
{
    return tfa98xx_dsp_set_param_multiple_var_wait(handle_cnt,
                   handles,
                   module_id, param_id,
                   num_bytes, data,
                   waitRetryCount);
}

/* Execute RPC protocol to write something to all the DSPs interleaved,
 * stop at the first error
 * optimized to minimize the latency between the execution point on the
 * various DSPs.
 * Uses small retry count.
 */
Tfa98xx_Error_t
Tfa98xx_DspSetParamMultiple(int handle_cnt,
                Tfa98xx_handle_t handles[],
                unsigned char module_id,
                unsigned char param_id,
                int num_bytes, const unsigned char data[])
{
    return tfa98xx_dsp_set_param_multiple_var_wait(handle_cnt, handles,
                          module_id, param_id,
                          num_bytes, data,
                          TFA98XX_WAITRESULT_NTRIES);
}

/* Execute RPC protocol to read something from the DSP */
Tfa98xx_Error_t
Tfa98xx_DspGetParam(Tfa98xx_handle_t handle,
            unsigned char module_id,
            unsigned char param_id, int num_bytes, unsigned char data[])
{
    return tfa98xx_dsp_get_param(handle,
            module_id,
            param_id, num_bytes, data);
}

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
void Tfa98xx_ConvertBytes2Data(int num_bytes, const unsigned char bytes[],
                   int data[])
{
    tfa98xx_convert_bytes2data(num_bytes, bytes, data);
}

Tfa98xx_Error_t
Tfa98xx_DspGetStateInfo(Tfa98xx_handle_t handle, Tfa98xx_StateInfo_t *pInfo)
{

    return tfa98xx_dsp_get_state_info(handle, pInfo);
}

Tfa98xx_Error_t
Tfa98xx_DspGetCalibrationImpedance(Tfa98xx_handle_t handle, FIXEDPT *pRe25)
{
    return tfa98xx_dsp_get_calibration_impedance(handle, pRe25);
}

Tfa98xx_Error_t
Tfa98xx_DspSetCalibrationImpedance(Tfa98xx_handle_t handle, const unsigned char *bytes)
{
    return tfa98xx_dsp_set_calibration_impedance(handle, bytes);
}

Tfa98xx_Error_t
Tfa98xx_DspReadMem(Tfa98xx_handle_t handle,
           unsigned short start_offset, int num_words, int *pValues)
{
    return tfa98xx_dsp_read_mem(handle,
           start_offset, num_words, pValues);
}

Tfa98xx_Error_t
Tfa98xx_DspWriteMem(Tfa98xx_handle_t handle, unsigned short address, int value, int memtype)
{

    return tfa98xx_dsp_write_mem(handle, address, value, memtype);
}

/* Execute generic RPC protocol that has both input and output parameters */
Tfa98xx_Error_t
Tfa98xx_DspExecuteRpc(Tfa98xx_handle_t handle,
              unsigned char module_id,
              unsigned char param_id, int num_inbytes,
              unsigned char indata[], int num_outbytes,
              unsigned char outdata[])
{
    return tfa98xx_dsp_execute_rpc(handle,
              module_id,
              param_id, num_inbytes,
              indata, num_outbytes,
              outdata);
}

Tfa98xx_Error_t
Tfa98xx_DspReadMemory(Tfa98xx_handle_t handle, Tfa98xx_DMEM_e which_mem,
              unsigned short start_offset, int num_words, int *pValues)
{

    return tfa98xx_dsp_read_memory(handle, which_mem,
              start_offset, num_words, pValues);
}

Tfa98xx_Error_t Tfa98xx_DspWriteMemory(Tfa98xx_handle_t handle,
                       Tfa98xx_DMEM_e which_mem,
                       unsigned short start_offset,
                       int num_words, int *pValues)
{
    return tfa98xx_dsp_write_memory(handle,
                       which_mem,
                       start_offset,
                       num_words, pValues);
}



Tfa98xx_Error_t Tfa98xx_SetToneDetectionOff(Tfa98xx_handle_t handle)
{
        return tfa98xx_set_tone_detection_off(handle);
}


const char* Tfa98xx_GetErrorString(Tfa98xx_Error_t error)
{
  const char* pErrStr;
  char latest_errorstr[64];

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
