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

#ifndef TFA98XX_H
#define TFA98XX_H

#ifdef __cplusplus
extern "C" {
#endif

#define TFA98XX_API_STR1(s) #s
#define TFA98XX_API_STR(s) TFA98XX_API_STR1(s)

#if !defined(SVN_BUILD_REV) || (SVN_BUILD_REV+1 == 1)
  #undef SVN_BUILD_REV
  #define SVN_BUILD_REV 0
#endif

#define TFA98XX_API_REV_MAJOR        1           /* major API rev */
#define TFA98XX_API_REV_MINOR        4           /* minor */
#define TFA98XX_API_REV_REVISION     1          /* revision */

#define TFA98XX_API_REV_STR TFA98XX_API_STR(TFA98XX_API_REV_MAJOR) "." \
                            TFA98XX_API_STR(TFA98XX_API_REV_MINOR) "." \
                            TFA98XX_API_STR(TFA98XX_API_REV_REVISION)

#ifdef __KERNEL__
    typedef long long FIXEDPT;
#define TO_LONG_LONG(x)  ((FIXEDPT)(x)<<32)
#define TO_INT(x)    ((x)>>32)
#else
    /* can only be done in user-space */
    typedef float FIXEDPT;
#define TO_INT(x)    (x)
#endif

/* Maximum number of retries for DSP result
 * Keep this value low!
 * If certain calls require longer wait conditions, the
 * application should poll, not the API
 * The total wait time depends on device settings. Those
 * are application specific.
 */
#define TFA98XX_WAITRESULT_NTRIES          40
#define TFA98XX_WAITRESULT_NTRIES_LONG   2000

/* following lengths are in bytes */
#define TFA98XX_PRESET_LENGTH              87

/* make full the default */

#define TFA98XX_CONFIG_LENGTH           165

/*
MUST BE CONSISTANT: either one uses opaque arrays of bytes, or not!!!
*/
typedef unsigned char Tfa98xx_Config_t[TFA98XX_CONFIG_LENGTH];
typedef unsigned char Tfa98xx_Preset_t[TFA98XX_PRESET_LENGTH];

/* Type containing all the possible errors that can occur
 *
 */
enum Tfa98xx_Error {
    Tfa98xx_Error_Ok = 0,
    Tfa98xx_Error_DSP_not_running,    /* communication with the DSP failed */
    Tfa98xx_Error_Bad_Parameter,
    Tfa98xx_Error_NotOpen,    /* the given handle is not open */
    Tfa98xx_Error_OutOfHandles,    /* too many handles */
    /* the expected response did not occur within the expected time */
    Tfa98xx_Error_StateTimedOut,
    Tfa98xx_Error_RpcBase = 100,
    Tfa98xx_Error_RpcBusy = 101,
    Tfa98xx_Error_RpcModId = 102,
    Tfa98xx_Error_RpcParamId = 103,
    Tfa98xx_Error_RpcInfoId = 104,
    Tfa98xx_Error_RpcNotAllowedSpeaker = 105,

    Tfa98xx_Error_Not_Implemented,
    Tfa98xx_Error_Not_Supported,
    Tfa98xx_Error_I2C_Fatal,    /* Fatal I2C error occurred */
    /* Nonfatal I2C error, and retry count reached */
    Tfa98xx_Error_I2C_NonFatal,
    Tfa98xx_Error_Other = 1000
};
/*
 * possible Digital Audio Interfaces bitmap
 */
enum Tfa98xx_DAI {
    Tfa98xx_DAI_I2S   =  0x01,
    Tfa98xx_DAI_TDM  =  0x02,
    Tfa98xx_DAI_PDM  =  0x04,
};
/*
 * config file subtypes
 */
enum Tfa98xx_config_type {
    Tfa98xx_config_generic,
    Tfa98xx_config_sub1,
    Tfa98xx_config_sub2,
    Tfa98xx_config_sub3,
};

enum Tfa98xx_AmpInputSel {
    Tfa98xx_AmpInputSel_I2SLeft,
    Tfa98xx_AmpInputSel_I2SRight,
    Tfa98xx_AmpInputSel_DSP
};

enum Tfa98xx_OutputSel {
    Tfa98xx_I2SOutputSel_CurrentSense,
    Tfa98xx_I2SOutputSel_DSP_Gain,
    Tfa98xx_I2SOutputSel_DSP_AEC,
    Tfa98xx_I2SOutputSel_Amp,
    Tfa98xx_I2SOutputSel_DataI3R,
    Tfa98xx_I2SOutputSel_DataI3L,
    Tfa98xx_I2SOutputSel_DcdcFFwdCur,
};

enum Tfa98xx_StereoGainSel {
    Tfa98xx_StereoGainSel_Left,
    Tfa98xx_StereoGainSel_Right
};

#define TFA98XX_MAXPATCH_LENGTH (3*1024)

/* the number of biquads supported */
#define TFA98XX_BIQUAD_NUM              10

enum Tfa98xx_Channel {
    Tfa98xx_Channel_L,
    Tfa98xx_Channel_R,
    Tfa98xx_Channel_L_R,
    Tfa98xx_Channel_Stereo
};

enum Tfa98xx_Mode {
    Tfa98xx_Mode_Normal = 0,
    Tfa98xx_Mode_RCV
};

enum Tfa98xx_Mute {
    Tfa98xx_Mute_Off,
    Tfa98xx_Mute_Digital,
    Tfa98xx_Mute_Amplifier
};

enum Tfa98xx_SpeakerBoostStatusFlags {
    Tfa98xx_SpeakerBoost_Activity = 0,    /* Input signal activity. */
    Tfa98xx_SpeakerBoost_S_Ctrl,    /* S Control triggers the limiter */
    Tfa98xx_SpeakerBoost_Muted,    /* 1 when signal is muted */
    Tfa98xx_SpeakerBoost_X_Ctrl,    /* X Control triggers the limiter */
    Tfa98xx_SpeakerBoost_T_Ctrl,    /* T Control triggers the limiter */
    Tfa98xx_SpeakerBoost_NewModel,    /* New model is available */
    Tfa98xx_SpeakerBoost_VolumeRdy,    /* 0:stable vol, 1:still smoothing */
    Tfa98xx_SpeakerBoost_Damaged,    /* Speaker Damage detected  */
    Tfa98xx_SpeakerBoost_SignalClipping    /* input clipping detected */
};
/*
 * the order matches the ACK bits order in TFA98XX_CF_STATUS
 */
enum tfa_fw_event { /* not all available on each device */
    tfa_fw_i2c_cmd_ack,
    tfa_fw_reset_start,
    tfa_fw_short_on_mips,
    tfa_fw_soft_mute_ready,
    tfa_fw_volume_ready,
    tfa_fw_error_damage,
    tfa_fw_calibrate_done,
    tfa_fw_max
};


struct Tfa98xx_StateInfo {
    /* SpeakerBoost State */
    float agcGain;    /* Current AGC Gain value */
    float limGain;    /* Current Limiter Gain value */
    float sMax;    /* Current Clip/Lim threshold */
    int T;        /* Current Speaker Temperature value */
    int statusFlag;    /* Masked bit word */
    float X1;    /* estimated excursion caused by Spkrboost gain ctrl */
    float X2;    /* estimated excursion caused by manual gain setting */
    float Re;    /* Loudspeaker blocked resistance */
    /* Framework state */
    /* increments each time a MIPS problem is detected on the DSP */
    int shortOnMips;
};

/* possible memory values for DMEM in CF_CONTROLs */
enum Tfa98xx_DMEM {
    Tfa98xx_DMEM_PMEM = 0,
    Tfa98xx_DMEM_XMEM = 1,
    Tfa98xx_DMEM_YMEM = 2,
    Tfa98xx_DMEM_IOMEM = 3,
};

/**
 *  register definition structure
 */
struct regdef {
    unsigned char offset; /**< subaddress offset */
    unsigned short pwronDefault;
                  /**< register contents after poweron */
    unsigned short pwronTestmask;
                  /**< mask of bits not test */
    char *name;          /**< short register name */
};

#define Tfa98xx_handle_t int

/**
 * The TFA9887 slave address can be 0x68, 6A, 6C or 6E
 */
enum Tfa98xx_Error tfa98xx_open(unsigned char slave_address,
                 Tfa98xx_handle_t *pHandle);
/**
 * Check if device is opened.
 */
int tfa98xx_handle_is_open(Tfa98xx_handle_t h);

/**
 * Load the default HW settings in the device
 */
enum Tfa98xx_Error tfa98xx_init(Tfa98xx_handle_t handle);

/**
 * Return the tfa revision
 */
void tfa98xx_rev(int *major, int *minor, int *revision);

/**
 * Return the maximum nr of devices
 */
int tfa98xx_max_devices(void);

/**
 * Close the instance handle
 */
enum Tfa98xx_Error tfa98xx_close(Tfa98xx_handle_t handle);

/* control the powerdown bit of the TFA9887
 * @param powerdown must be 1 or 0
 */
enum Tfa98xx_Error tfa98xx_powerdown(Tfa98xx_handle_t handle,
                  int powerdown);

/* resolved the incident reported from service layer
 * for 97 a DSP reset is performed
 * for all other device perform power down
 * @param incidentlevel must be 1 or 0
 */
enum Tfa98xx_Error tfa98xx_resolve_incident(Tfa98xx_handle_t handle,
                       int incidentlevel);

/* Notify the DSP that all parameters have been loaded.
 * @param configured must be 1 or 0
 */
enum Tfa98xx_Error tfa98xx_set_configured(Tfa98xx_handle_t handle);

/* control the input_sel bits of the TFA9887, to indicate */
/* what is sent to the amplfier and speaker
 * @param input_sel, see Tfa98xx_AmpInputSel_t
 */
enum Tfa98xx_Error tfa98xx_select_amplifier_input(Tfa98xx_handle_t handle,
                         enum Tfa98xx_AmpInputSel
                         input_sel);

/* control the I2S left output of the TFA9887
 * @param output_sel, see Tfa98xx_OutputSel_t
 */
enum Tfa98xx_Error tfa98xx_select_i2s_output_left(Tfa98xx_handle_t handle,
                        enum Tfa98xx_OutputSel
                        output_sel);

/* control the I2S right output of the TFA9887
 * @param output_sel, see Tfa98xx_OutputSel_t
 */
enum Tfa98xx_Error tfa98xx_select_i2s_output_right(Tfa98xx_handle_t handle,
                         enum Tfa98xx_OutputSel
                         output_sel);

/* indicates on which channel of DATAI2 the gain from the IC is set
 * @param gain_sel, see Tfa98xx_StereoGainSel_t
 */
enum Tfa98xx_Error tfa98xx_select_stereo_gain_channel(Tfa98xx_handle_t handle,
                        enum Tfa98xx_StereoGainSel
                        gain_sel);

/* control the volume of the DSP. This is depricated due to use of floats.
 * @param voldB volume in dB.  must be between 0 and -inf
 */
enum Tfa98xx_Error tfa98xx_set_volume(Tfa98xx_handle_t handle,
                  FIXEDPT voldB);

/* control the volume of the DSP
 * @param vol volume in bit field. It must be between 0 and 255
 */
enum Tfa98xx_Error tfa98xx_set_volume_level(Tfa98xx_handle_t handle,
                  unsigned short vol);

/* read the currently set volume
 * @param voldB volume in dB.
 */
enum Tfa98xx_Error tfa98xx_get_volume(Tfa98xx_handle_t handle,
                  FIXEDPT *pVoldB);

/* notify the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param rate in Hz.  must be 32000, 44100 or 48000
 */
enum Tfa98xx_Error tfa98xx_set_sample_rate(Tfa98xx_handle_t handle,
                      int rate);

/* read the TFA9887 of the sample rate of the I2S bus that will be used.
 * @param pRate pointer to rate in Hz i.e 32000, 44100 or 48000
 */
enum Tfa98xx_Error tfa98xx_get_sample_rate(Tfa98xx_handle_t handle,
                      int *pRate);

/* set the input channel to use
 * @param channel see Tfa98xx_Channel_t enumeration
 */
enum Tfa98xx_Error tfa98xx_select_channel(Tfa98xx_handle_t handle,
                        enum Tfa98xx_Channel channel);

/* set the mode for normal or receiver mode
 * @param mode see Tfa98xx_Mode enumeration
 */
enum Tfa98xx_Error tfa98xx_select_mode(Tfa98xx_handle_t handle,
                                        enum Tfa98xx_Mode mode );

/* get the mode of the device e.g. normal or receiver mode
 * @param mode see Tfa98xx_Mode enumeration
 */
enum Tfa98xx_Error tfa98xx_get_mode(Tfa98xx_handle_t handle,
                                        enum Tfa98xx_Mode *mode);

/* enabling the AEC output
 */
enum Tfa98xx_Error tfa98xx_enable_aecoutput(Tfa98xx_handle_t handle);

/* disabling the AEC output to prevent leakage current
 */
enum Tfa98xx_Error tfa98xx_disable_aecoutput(Tfa98xx_handle_t handle);

/* mute/unmute the audio
 * @param mute see Tfa98xx_Mute_t enumeration
 */
enum Tfa98xx_Error tfa98xx_set_mute(Tfa98xx_handle_t handle,
                enum Tfa98xx_Mute mute);

enum Tfa98xx_Error tfa98xx_get_mute(Tfa98xx_handle_t handle,
                enum Tfa98xx_Mute *pMute);
/*
 * tfa98xx_supported_dai
 *  returns the bitmap of the supported Digital Audio Interfaces
 * @param dai bitmap enum pointer
 *  @return error code
 */
enum Tfa98xx_Error tfa98xx_supported_dai(Tfa98xx_handle_t handle, enum Tfa98xx_DAI *daimap);

/* Yields the number of parameters to be used in tfa98xx_dsp_write_config()
 * @pointer to parameter count.
 * @return error code. Only assigned if return value == Tfa98xx_Error_Ok
 */
enum Tfa98xx_Error tfa98xx_dsp_config_parameter_count(Tfa98xx_handle_t handle,
                        int *pParamCount);

/* Tfa98xx_DspConfigParameterType
 *   Returns the config file subtype
 */
enum Tfa98xx_Error tfa98xx_dsp_config_parameter_type(Tfa98xx_handle_t handle,
        enum Tfa98xx_config_type *ptype);

/* load the tables to the DSP
 *   called after patch load is done
 *   @return error code
 */
enum Tfa98xx_Error tfa98xx_dsp_write_tables(int handle);

/* set or clear DSP reset signal
 * @param new state
 * @return error code
 */
enum Tfa98xx_Error tfa98xx_dsp_reset(Tfa98xx_handle_t handle, int state);

/* check the state of the DSP subsystem
 * return ready = 1 when clocks are stable to allow safe DSP subsystem access
 * @param pointer to state flag, non-zero if clocks are not stable
 * @return error code
 */
//#if (!defined(TFA9890)  && !defined(TFA98XX_FULL)) /* default function */
enum Tfa98xx_Error tfa98xx_dsp_system_stable(Tfa98xx_handle_t handle,
                        int *ready);
//#elif defined(TFA98XX_FULL)
/* This is the clean, default static _function that is called
 * from Tfa98xx_<func>() in the switch case. The function name needs to be
 * changed in this case.
 */
enum Tfa98xx_Error _tfa98xx_dsp_system_stable(Tfa98xx_handle_t handle,
                        int *ready);
//#endif

/* The following functions can only be called when the DSP is running
 * - I2S clock must be active,
 * - IC must be in operating mode
 */
/* patch the ROM code of the DSP */
enum Tfa98xx_Error tfa98xx_dsp_patch(Tfa98xx_handle_t handle,
                 int patchLength,
                 const unsigned char *patchBytes);

/* Check whether the DSP expects tCoef or tCoefA as last parameter in
 * the speaker parameters
 * *pbSupporttCoef=1 when DSP expects tCoef,
 * *pbSupporttCoef=0 when it expects tCoefA (and the elaborate workaround
 * to calculate tCoefA from tCoef on the host)
 */
enum Tfa98xx_Error tfa98xx_dsp_support_tcoef(Tfa98xx_handle_t handle,
                    int *pbSupporttCoef);

/* return the device revision id
 */
unsigned short Tfa98xx_get_device_revision(Tfa98xx_handle_t handle);


/* load the system wide parameters */
enum Tfa98xx_Error tfa98xx_dsp_write_config(Tfa98xx_handle_t handle,
                       int length, const unsigned char
                       *pConfigBytes);
/* Get the system wide parameters */
enum Tfa98xx_Error tfa98xx_dsp_read_config(Tfa98xx_handle_t handle,
                      int length,
                      unsigned char *pConfigBytes);

/* load explicitly the speaker parameters in case of free speaker, */
/* or when using a saved speaker model */
enum Tfa98xx_Error tfa98xx_dsp_write_speaker_parameters(
                Tfa98xx_handle_t handle,
                int length,
                const unsigned char *pSpeakerBytes);

/* read the speaker parameters as used by the SpeakerBoost processing */
enum Tfa98xx_Error tfa98xx_dsp_read_speaker_parameters(
                Tfa98xx_handle_t handle,
                int length,
                unsigned char *pSpeakerBytes);

/* read the speaker excursion model as used by SpeakerBoost processing */
enum Tfa98xx_Error tfa98xx_dsp_read_excursion_model(
                Tfa98xx_handle_t handle,
                int length,
                unsigned char *pSpeakerBytes);

/* load all the parameters for a preset from a file */
enum Tfa98xx_Error tfa98xx_dsp_write_preset(Tfa98xx_handle_t handle,
                       int length, const unsigned char
                       *pPresetBytes);
/* Get the system wide parameters */
enum Tfa98xx_Error tfa98xx_dsp_read_preset(Tfa98xx_handle_t handle,
                      int length,
                      unsigned char *pPresetBytes);


/* set the biquad coeeficient for the indicated biquad filter (index)
 * This function is provided for kernel integration.
 */
enum Tfa98xx_Error tfa98xx_dsp_biquad_set_coeff(
                Tfa98xx_handle_t handle,
                int biquad_index,
                const unsigned char *pBiquadBytes);
enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_coeff_bytes(Tfa98xx_handle_t handle,
                int biquad_index,
                const unsigned char *pBiquadBytes);

enum Tfa98xx_Error tfa98xx_dsp_biquad_set_coeff_multiple_bytes(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int biquad_index,
                const unsigned char *pBiquadBytes);
/* disable a certain biquad */
enum Tfa98xx_Error tfa98xx_dsp_biquad_disable(Tfa98xx_handle_t handle,
                    int biquad_index,
                    const unsigned char *bytes);
enum Tfa98xx_Error tfa98xx_dsp_biquad_disable_multiple(int handle_cnt,
                    Tfa98xx_handle_t
                    handles[],
                    int biquad_index,
                    const unsigned char *bytes);

/* Get the calibration result */
enum Tfa98xx_Error tfa98xx_dsp_get_calibration_impedance(Tfa98xx_handle_t
                           handle,
                           FIXEDPT *pRe25);
/* Set the calibration impedance.
 * DSP reset is done in this function so that the new re0
 * value to take effect, */
enum Tfa98xx_Error tfa98xx_dsp_set_calibration_impedance(Tfa98xx_handle_t
                           handle,
                            const unsigned char *pBytes);

/* read the current status of the DSP, typically used for development, */
/* not essential to be used in a product                               */
enum Tfa98xx_Error tfa98xx_dsp_get_state_info(
                Tfa98xx_handle_t handle,
                struct Tfa98xx_StateInfo *pInfo);

/* optimized SetParam type of functions that allows writing the same    */
/* parameters to multiple device with minimal delay between the devices */
enum Tfa98xx_Error tfa98xx_dsp_write_config_multiple(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int length,
                const unsigned char *pConfigBytes);
enum Tfa98xx_Error tfa98xx_dsp_write_speaker_parameters_multiple(
                int handle_cnt,
                Tfa98xx_handle_t handles[],
                int length,
                const unsigned char *pSpeakerBytes);
enum Tfa98xx_Error tfa98xx_dsp_write_preset_multiple(
                int handle_cnt,
                Tfa98xx_handle_t
                handles[], int length,
                const unsigned char *pPresetBytes);


/* DSP Biquad use floating point - which is not supported in kernel
 * Use the #ifndef to bypass this API and consider using
 * Tfa98xx_DspBiquad_SetCoeffBytes.
 */
enum Tfa98xx_Error tfa98xx_dsp_biquad_set_all_coeff(Tfa98xx_handle_t handle,
                  const unsigned char *bytes);

enum Tfa98xx_Error tfa98xx_dsp_biquad_set_all_coeff_multiple(int handle_cnt,
                Tfa98xx_handle_t handles[],
                const unsigned char *bytes);

/* low level routines, not part of official API and might be removed */
/* in the future */
enum Tfa98xx_Error tfa98xx_read_register16(Tfa98xx_handle_t handle,
                       unsigned char subaddress,
                       unsigned short *pValue);
enum Tfa98xx_Error tfa98xx_write_register16(Tfa98xx_handle_t handle,
                    unsigned char subaddress,
                    unsigned short value);
enum Tfa98xx_Error tfa98xx_dsp_read_mem(Tfa98xx_handle_t handle,
                   unsigned short start_offset,
                   int num_words, int *pValues);
enum Tfa98xx_Error tfa98xx_dsp_write_mem(Tfa98xx_handle_t handle,
                    unsigned short address, int value, int memtype);
enum Tfa98xx_Error tfa98xx_dsp_set_param(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id,
                    int num_bytes,
                    const unsigned char data[]);
enum Tfa98xx_Error tfa98xx_dsp_set_paramMultiple(int handle_cnt,
                        Tfa98xx_handle_t handles[],
                        unsigned char module_id,
                        unsigned char param_id,
                        int num_bytes,
                        const unsigned char data[]);
enum Tfa98xx_Error tfa98xx_dsp_get_param(Tfa98xx_handle_t handle,
                    unsigned char module_id,
                    unsigned char param_id,
                    int num_bytes,
                    unsigned char data[]);
enum Tfa98xx_Error tfa98xx_dsp_execute_rpc(Tfa98xx_handle_t handle,
              unsigned char module_id,
              unsigned char param_id, int num_inbytes,
              unsigned char indata[], int num_outbytes,
              unsigned char outdata[]);
enum Tfa98xx_Error tfa98xx_read_data(Tfa98xx_handle_t handle,
                 unsigned char subaddress,
                 int num_bytes, unsigned char data[]);
enum Tfa98xx_Error tfa98xx_write_data(Tfa98xx_handle_t handle,
                  unsigned char subaddress,
                  int num_bytes,
                  const unsigned char data[]);
enum Tfa98xx_Error tfa98xx_read_versions(char *buffer);

enum Tfa98xx_Error tfa98xx_dsp_write_memory(Tfa98xx_handle_t handle,
                       enum Tfa98xx_DMEM which_mem,
                       unsigned short start_offset,
                       int num_words, int *pValues);
enum Tfa98xx_Error tfa98xx_dsp_read_memory(Tfa98xx_handle_t handle,
                      enum Tfa98xx_DMEM which_mem,
                      unsigned short start_offset,
                      int num_words, int *pValues);

/* support for converting error codes into text */
const char *tfa98xx_get_error_string(enum Tfa98xx_Error error);

enum Tfa98xx_Error tfa98xx_execute_param(Tfa98xx_handle_t handle);
void tfa98xx_convert_data2bytes(int num_data, const int data[],
                   unsigned char bytes[]);
enum Tfa98xx_Error
tfa98xx_check_size(enum Tfa98xx_DMEM which_mem, int num_bytes);

//enum Tfa98xx_Error tfa98xx_classify_i2c_error(enum NXP_I2C_Error i2c_error);

enum Tfa98xx_Error
tfa98xx_check_ic_rom_version(Tfa98xx_handle_t handle, const unsigned char patchheader[]);
enum Tfa98xx_Error
tfa98xx_dsp_get_sw_feature_bits(Tfa98xx_handle_t handle, int features[2]);
unsigned short tfa98xx_get_device_revision(Tfa98xx_handle_t handle);
enum Tfa98xx_Error
tfa98xx_process_patch_file(Tfa98xx_handle_t handle, int length,
         const unsigned char *bytes);
enum Tfa98xx_Error
tfa98xx_dsp_support_framework(Tfa98xx_handle_t handle, int *pbSupportFramework);

enum Tfa98xx_Error
tfa98xx_dsp_read_spkr_params(Tfa98xx_handle_t handle,
              unsigned char paramId,
              int length, unsigned char *pSpeakerBytes);

enum Tfa98xx_Error
tfa98xx_dsp_set_param_multiple(int handle_cnt,
                Tfa98xx_handle_t handles[],
                unsigned char module_id,
                unsigned char param_id,
                int num_bytes, const unsigned char data[]);

enum Tfa98xx_Error
tfa98xx_check_rpc_status(Tfa98xx_handle_t handle, int *pRpcStatus);

enum Tfa98xx_Error
tfa98xx_write_parameter(Tfa98xx_handle_t handle,
           unsigned char module_id,
           unsigned char param_id,
           int num_bytes, const unsigned char data[]);

enum Tfa98xx_Error
tfa98xx_wait_result(Tfa98xx_handle_t handle, int waitRetryCount);

enum Tfa98xx_Error
tfa98xx_dsp_set_param_var_wait(Tfa98xx_handle_t handle,
               unsigned char module_id,
               unsigned char param_id, int num_bytes,
               const unsigned char data[], int waitRetryCount);

enum Tfa98xx_Error
tfa98xx_dsp_set_param_multiple(int handle_cnt,
                Tfa98xx_handle_t handles[],
                unsigned char module_id,
                unsigned char param_id,
                int num_bytes, const unsigned char data[]);

void tfa98xx_convert_bytes2data(int num_bytes, const unsigned char bytes[],
                   int data[]);

enum Tfa98xx_Error
tfa98xx_dsp_set_param_multiple_var_wait(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   unsigned char module_id,
                   unsigned char param_id,
                   int num_bytes, const unsigned char data[],
                   int waitRetryCount);

enum Tfa98xx_Error tfa98xx_dsp_msg(int handle, int length, const char *buf, int *msg_status);
enum Tfa98xx_Error tfa98xx_dsp_msg_write(int handle, int length, const char *buf);
enum Tfa98xx_Error tfa98xx_dsp_msg_read(int handle,int length, unsigned char *bytes);
enum Tfa98xx_Error tfa98xx_dsp_msg_status(Tfa98xx_handle_t handle, int *pRpcStatus);
enum Tfa98xx_Error tfa98xx_reg_setfield(Tfa98xx_handle_t handle,  unsigned int bf_enum, unsigned short  value);
enum Tfa98xx_Error tfa98xx_reg_getfield(Tfa98xx_handle_t handle,  int bf_enum, unsigned short *value);
enum Tfa98xx_Error tfa98xx_reg_setfield_named(Tfa98xx_handle_t handle,  char *bf, unsigned short  value) ;
enum Tfa98xx_Error tfa98xx_reg_getfield_named(Tfa98xx_handle_t handle, char *bf , unsigned short *value);
enum Tfa98xx_Error tfa98xx_dsp_mem_write(int handle, enum Tfa98xx_DMEM which_mem,
        unsigned short start_offset,
        int length, const unsigned char *bytedata) ;
enum Tfa98xx_Error tfa98xx_dsp_mem_read(int handle, enum Tfa98xx_DMEM which_mem,
        unsigned short start_offset,
        int length, const unsigned char *bytedata) ;
enum Tfa98xx_Error tfa98xx_set_tone_detection_off(Tfa98xx_handle_t handle);
enum Tfa98xx_Error tfa98xx_check_device_features(Tfa98xx_handle_t handle);

#ifdef __cplusplus
}
#endif
#endif                /* TFA98XX_H */
