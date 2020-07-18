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
#include "NXP_I2C.h"
#include "dbgprint.h"

#include "initTfa9890.h"

#ifdef __KERNEL__
#define _ASSERT(e) do { if ((e)) pr_err("PrintAssert:%s (%s:%d)\n",\
        __func__, __FILE__, __LINE__); } while (0)
#else
#define _ASSERT(e)  assert(e)
#endif

#define OPTIMIZED_RPC

/* 4 possible I2C addresses
 */
#define MAX_HANDLES 4
static struct Tfa98xx_handle_private handlesLocal[MAX_HANDLES];

/**
 * return revision
 */
void tfa98xx_rev(int *major, int *minor, int *rev)
{
    int H,h,r;
    *major = TFA98XX_API_REV_MAJOR;
    *minor = TFA98XX_API_REV_MINOR;
    *rev   = TFA98XX_API_REV_REVISION;

    NXP_I2C_rev(&H,&h,&r);
    PRINT("Tfa98xx HAL rev: %d.%d.%d\n", H, h, r);
}
/**
 * Return the maximum nr of devices (SC39786)
 */
int tfa98xx_max_devices(void)
{
    return MAX_HANDLES;
}

int tfa98xx_handle_is_open(Tfa98xx_handle_t h)
{
    int retval = 0;

    if ((h >= 0) && (h < MAX_HANDLES))
        retval = handlesLocal[h].in_use != 0;

    return retval;
}

/* translate a I2C driver error into an error for Tfa9887 API */
static enum Tfa98xx_Error tfa98xx_classify_i2c_error(enum NXP_I2C_Error i2c_error)
{
    switch (i2c_error) {
        case NXP_I2C_Ok:
            return Tfa98xx_Error_Ok;
        case NXP_I2C_NoAck:
        case NXP_I2C_ArbLost:
        case NXP_I2C_TimeOut:
            return Tfa98xx_Error_I2C_NonFatal;
        default:
            return Tfa98xx_Error_I2C_Fatal;
    }
}

/* the patch contains a header with the following
 * IC revision register: 1 byte, 0xFF means don't care
 * XMEM address to check: 2 bytes, big endian, 0xFFFF means don't care
 * XMEM value to expect: 3 bytes, big endian
 */
enum Tfa98xx_Error
tfa98xx_check_ic_rom_version(Tfa98xx_handle_t handle, const unsigned char patchheader[])
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short checkrev;
    unsigned short checkaddress;
    int checkvalue;
    int value = 0;
    int status;
    checkrev = patchheader[0];
    if ((checkrev != 0xFF) && (checkrev != handlesLocal[handle].rev))
        return Tfa98xx_Error_Not_Supported;

    checkaddress = (patchheader[1] << 8) + patchheader[2];
    checkvalue =
        (patchheader[3] << 16) + (patchheader[4] << 8) + patchheader[5];
    if (checkaddress != 0xFFFF) {
        /* before reading XMEM, check if we can access the DSP */
        error = tfa98xx_dsp_system_stable(handle, &status);
        if (error == Tfa98xx_Error_Ok) {
            if (!status) {
                /* DSP subsys not running */
                error = Tfa98xx_Error_DSP_not_running;
            }
        }
        /* read register to check the correct ROM version */
        if (error == Tfa98xx_Error_Ok) {
            error =
            tfa98xx_dsp_read_mem(handle, checkaddress, 1, &value);
        }
        if (error == Tfa98xx_Error_Ok) {
            if (value != checkvalue)
                error = Tfa98xx_Error_Not_Supported;
        }
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_get_sw_feature_bits(Tfa98xx_handle_t handle, int features[2])
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned char bytes[3 * 2];

    error =
        tfa98xx_dsp_get_param(handle, MODULE_FRAMEWORK,
                FW_PARAM_GET_FEATURE_BITS, sizeof(bytes),
                bytes);
    if (error != Tfa98xx_Error_Ok) {
        /* old ROM code may respond with Tfa98xx_Error_RpcParamId */
        return error;
    }
    tfa98xx_convert_bytes2data(sizeof(bytes), bytes, features);

    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_get_state_info_size(Tfa98xx_handle_t handle, int *pSize)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    *pSize = sizeof(struct Tfa98xx_StateInfo);
    return error;
}

enum Tfa98xx_Error
tfa98xx_open(unsigned char slave_address, Tfa98xx_handle_t *pHandle)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_OutOfHandles;
    unsigned short rev, status;
    int i;
    _ASSERT(pHandle != (Tfa98xx_handle_t *) 0);
    *pHandle = -1;
    /* find free handle */

    for (i = 0; i < MAX_HANDLES; ++i) {
        if (!handlesLocal[i].in_use) {
            handlesLocal[i].in_use = 1;
            handlesLocal[i].slave_address = slave_address;
            handlesLocal[i].supportDrc = supportNotSet;
            handlesLocal[i].supportFramework = supportNotSet;
            error = tfa98xx_read_register16(i,
                            TFA98XX_REVISIONNUMBER,
                            &rev);
            if (Tfa98xx_Error_Ok != error) {
                handlesLocal[i].in_use = 0;
                break; /* error, exit the for-loop */
            }
            handlesLocal[i].rev = rev & 0xff;
            handlesLocal[i].subrev = (rev>>8) & 0xff;
            *pHandle = i;
            error = Tfa98xx_Error_Ok;
            break;    /* handle is assigned, exit the for-loop */
        }
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_close(Tfa98xx_handle_t handle)
{
    if (tfa98xx_handle_is_open(handle)) {
        handlesLocal[handle].in_use = 0;
        return Tfa98xx_Error_Ok;
    } else {
        return Tfa98xx_Error_NotOpen;
    }
}

/* Tfa98xx_DspConfigParameterCount
 * Yields the number of parameters to be used in tfa98xx_dsp_write_config()
 */
enum Tfa98xx_Error tfa98xx_dsp_config_parameter_count(Tfa98xx_handle_t handle,
                        int *pParamCount)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

        *pParamCount = 55;

    return error;
}
/* Tfa98xx_DspConfigParameterType
 *   Returns the config file subtype
 */
enum Tfa98xx_Error tfa98xx_dsp_config_parameter_type(Tfa98xx_handle_t handle,
        enum Tfa98xx_config_type *ptype)
{
enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

*ptype = Tfa98xx_config_generic;
    if (!tfa98xx_handle_is_open(handle)) return Tfa98xx_Error_NotOpen;
        *ptype = Tfa98xx_config_sub1;

    return error;
}

/*
 * tfa98xx_supported_dai
 *  returns the bitmap of the supported Digital Audio Interfaces
 */
enum Tfa98xx_Error tfa98xx_supported_dai(Tfa98xx_handle_t handle, enum Tfa98xx_DAI *daimap)
{
    if (tfa98xx_handle_is_open(handle)) {
#ifdef TFA9891
        *daimap = ( Tfa98xx_DAI_PDM | Tfa98xx_DAI_I2S );
#else
        *daimap = Tfa98xx_DAI_I2S;        /* all others */
#endif /* TFA9891 */
    } else
        return Tfa98xx_Error_NotOpen;

    return Tfa98xx_Error_Ok;
}

/* Tfa98xx_DspReset
 *  set or clear DSP reset signal
 */
/* the wrapper for DspReset, for tfa9890 */
enum Tfa98xx_Error tfa98xx_dsp_reset(Tfa98xx_handle_t handle, int state)
{

    return tfa9890_dsp_reset(handle, state);    /* tfa9890 function */
}

/* Tfa98xx_DspSystemStable
 *  return: *ready = 1 when clocks are stable to allow DSP subsystem access
 */
enum Tfa98xx_Error tfa98xx_dsp_system_stable(Tfa98xx_handle_t handle, int *ready)
{
    return tfa9890_dsp_system_stable(handle, ready);    /* tfa9890 function */
}

/* return the device revision id
 */
unsigned short tfa98xx_get_device_revision(Tfa98xx_handle_t handle)
{
    /* local function. Caller must make sure handle is valid */
    return handlesLocal[handle].rev;
}

enum Tfa98xx_Error tfa98xx_init(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error error;

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    /* reset all i2C registers to default */
    error = tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL,
                    TFA98XX_SYS_CTRL_I2CR_MSK);

    /* some other registers must be set for optimal amplifier behaviour
     * This is implemented in a file specific for the type number
     */
    error = tfa9890_specific(handle);

    return error;
}

enum Tfa98xx_Error
tfa98xx_process_patch_file(Tfa98xx_handle_t handle, int length,
         const unsigned char *bytes)
{
    unsigned short size;
    int index;
    enum NXP_I2C_Error i2c_error = NXP_I2C_Ok;
    /* expect following format in patchBytes:
     * 2 bytes length of I2C transaction in little endian, then the bytes,
     * excluding the slave address which is added from the handle
     * This repeats for the whole file
     */

    index = 0;
    while (index < length) {
        /* extract little endian length */
        size = bytes[index] + bytes[index + 1] * 256;
        index += 2;
        if ((index + size) > length) {
            /* outside the buffer, error in the input data */
            return Tfa98xx_Error_Bad_Parameter;
        }
        if (size > NXP_I2C_BufferSize()) {
            /* too big, must fit buffer */
            return Tfa98xx_Error_Bad_Parameter;
        }
        i2c_error =
            NXP_I2C_Write(handlesLocal[handle].slave_address, size,
                  bytes + index);
        if (i2c_error != NXP_I2C_Ok)
            break;
        index += size;
    }
    return tfa98xx_classify_i2c_error(i2c_error);
}

/*
 * write a 16 bit subaddress
 */
enum Tfa98xx_Error
tfa98xx_write_register16(Tfa98xx_handle_t handle,
            unsigned char subaddress, unsigned short value)
{
    enum NXP_I2C_Error i2c_error;
    int bytes2write = 3;    /* subaddress and 2 bytes of the value */
    unsigned char write_data[3];
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    write_data[0] = subaddress;
    write_data[1] = (value >> 8) & 0xFF;
    write_data[2] = value & 0xFF;

    i2c_error = NXP_I2C_Write(handlesLocal[handle].slave_address, bytes2write, write_data);

    return tfa98xx_classify_i2c_error(i2c_error);
}

enum Tfa98xx_Error
tfa98xx_dsp_support_framework(Tfa98xx_handle_t handle, int *pbSupportFramework)
{
    int featureBits[2] = { 0, 0 };
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    _ASSERT(pbSupportFramework != 0);

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    if (handlesLocal[handle].supportFramework != supportNotSet) {
        *pbSupportFramework = handlesLocal[handle].supportFramework;
    } else {
        error = tfa98xx_dsp_get_sw_feature_bits(handle, featureBits);
        if (error == Tfa98xx_Error_Ok) {
            *pbSupportFramework = 1;
            handlesLocal[handle].supportFramework = supportYes;
        } else if (error == Tfa98xx_Error_RpcParamId) {
            *pbSupportFramework = 0;
            handlesLocal[handle].supportFramework = supportNo;
            error = Tfa98xx_Error_Ok;
        }
    }

    /* *pbSupportFramework only changed when error == Tfa98xx_Error_Ok */
    return error;
}

/*
 * Write all the bytes specified by num_bytes and data
 */
enum Tfa98xx_Error
tfa98xx_write_data(Tfa98xx_handle_t handle,
          unsigned char subaddress, int num_bytes,
          const unsigned char data[])
{
    enum NXP_I2C_Error i2c_error;
    /* subaddress followed by data */
    int bytes2write = num_bytes + 1;

    unsigned char write_data[MAX_PARAM_SIZE];
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (bytes2write > MAX_PARAM_SIZE)
        return Tfa98xx_Error_Bad_Parameter;
    if (bytes2write > NXP_I2C_BufferSize())
        return Tfa98xx_Error_Bad_Parameter;

    write_data[0] = subaddress;
    memcpy(write_data + 1, data, num_bytes);
    i2c_error =
        NXP_I2C_Write(handlesLocal[handle].slave_address, bytes2write,
              write_data);
    return tfa98xx_classify_i2c_error(i2c_error);
}

enum Tfa98xx_Error
tfa98xx_read_register16(Tfa98xx_handle_t handle,
               unsigned char subaddress, unsigned short *pValue)
{
    enum NXP_I2C_Error i2c_error;
    const int bytes2write = 1;    /* subaddress */
    /* 2 bytes that will contain the data of the register */
    const int bytes2read = 2;
    unsigned char write_data[1];
    unsigned char read_buffer[2];
    _ASSERT(pValue != (unsigned short *)0);
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    write_data[0] = subaddress;
    read_buffer[0] = read_buffer[1] = 0;
    i2c_error =
        NXP_I2C_WriteRead(handlesLocal[handle].slave_address, bytes2write,
                  write_data, bytes2read, read_buffer);
    if (tfa98xx_classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok) {
        return tfa98xx_classify_i2c_error(i2c_error);
    } else {
        *pValue = (read_buffer[0] << 8) + read_buffer[1];
        return Tfa98xx_Error_Ok;
    }
}

enum Tfa98xx_Error tfa98xx_read_versions(char *buffer)
{
        enum NXP_I2C_Error i2c_error;

        i2c_error = NXP_I2C_Version(buffer);

        return tfa98xx_classify_i2c_error(i2c_error);
}

enum Tfa98xx_Error
tfa98xx_read_data(Tfa98xx_handle_t handle,
         unsigned char subaddress, int num_bytes, unsigned char data[])
{
    enum NXP_I2C_Error i2c_error;
    const int bytes2write = 1;    /* subaddress */
    unsigned char write_data[1];
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (num_bytes > NXP_I2C_BufferSize())
        return Tfa98xx_Error_Bad_Parameter;
    write_data[0] = subaddress;
    i2c_error =
        NXP_I2C_WriteRead(handlesLocal[handle].slave_address, bytes2write,
                  write_data, num_bytes, data);
    if (tfa98xx_classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok)
        return tfa98xx_classify_i2c_error(i2c_error);
    else
        return Tfa98xx_Error_Ok;
}

enum Tfa98xx_Error tfa98xx_powerdown(Tfa98xx_handle_t handle, int powerdown)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_SYS_CTRL, &value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    switch (powerdown) {
    case 1:
        value |= TFA98XX_SYS_CTRL_PWDN_MSK;
        break;
    case 0:
        value &= ~(TFA98XX_SYS_CTRL_PWDN_MSK);
        break;
    default:
        return Tfa98xx_Error_Bad_Parameter;
    }
    error = tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL, value);
    return error;
}

enum Tfa98xx_Error tfa98xx_resolve_incident(Tfa98xx_handle_t handle,
                       int incidentlevel)
{
    enum Tfa98xx_Error error;
    //unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

        /* for TFA98xx need power cycle of CF */
        switch (incidentlevel) {
                case 1:
                    error = tfa98xx_powerdown(handle, 1);
                    if (error == Tfa98xx_Error_Ok)
                        error = tfa98xx_powerdown(handle, 0);

                    break;
                default:
                    return Tfa98xx_Error_Bad_Parameter;
        }
    return error;
}


enum Tfa98xx_Error tfa98xx_set_configured(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_SYS_CTRL, &value);
    if (error != Tfa98xx_Error_Ok)
        return error;

    value |= TFA98XX_SYS_CTRL_SBSL_MSK;
    error = tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL, value);
    return error;
}

enum Tfa98xx_Error
tfa98xx_select_amplifier_input(Tfa98xx_handle_t handle,
                 enum Tfa98xx_AmpInputSel input_sel)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 2 bits first */
        value &= ~(0x3 << TFA98XX_I2SREG_CHSA_POS);
        switch (input_sel) {
        case Tfa98xx_AmpInputSel_I2SLeft:
            value |= (0x0 << TFA98XX_I2SREG_CHSA_POS);
            break;
        case Tfa98xx_AmpInputSel_I2SRight:
            value |= (0x1 << TFA98XX_I2SREG_CHSA_POS);
            break;
        case Tfa98xx_AmpInputSel_DSP:
            value |= (0x2 << TFA98XX_I2SREG_CHSA_POS);
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_write_register16(handle, TFA98XX_I2SREG, value);

    return error;
}

enum Tfa98xx_Error
tfa98xx_select_i2s_output_left(Tfa98xx_handle_t handle,
                enum Tfa98xx_OutputSel output_sel)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the I2S SEL register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2S_SEL_REG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 3 bits first */
        value &= ~(0x7 << TFA98XX_I2S_SEL_REG_DOLS_POS);
        switch (output_sel) {
        case Tfa98xx_I2SOutputSel_CurrentSense:
        case Tfa98xx_I2SOutputSel_DSP_Gain:
        case Tfa98xx_I2SOutputSel_DSP_AEC:
        case Tfa98xx_I2SOutputSel_Amp:
        case Tfa98xx_I2SOutputSel_DataI3R:
        case Tfa98xx_I2SOutputSel_DataI3L:
        case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
            /* enum definition matches the HW registers */
            value |= (output_sel << TFA98XX_I2S_SEL_REG_DOLS_POS);
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok) {
        error = tfa98xx_write_register16(
            handle, TFA98XX_I2S_SEL_REG, value);
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_select_i2s_output_right(Tfa98xx_handle_t handle,
                 enum Tfa98xx_OutputSel output_sel)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the I2S SEL register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2S_SEL_REG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 3 bits first */
        value &= ~(0x7 << TFA98XX_I2S_SEL_REG_DORS_POS);
        switch (output_sel) {
        case Tfa98xx_I2SOutputSel_CurrentSense:
        case Tfa98xx_I2SOutputSel_DSP_Gain:
        case Tfa98xx_I2SOutputSel_DSP_AEC:
        case Tfa98xx_I2SOutputSel_Amp:
        case Tfa98xx_I2SOutputSel_DataI3R:
        case Tfa98xx_I2SOutputSel_DataI3L:
        case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
            /* enum definition matches the HW registers */
            value |= (output_sel << TFA98XX_I2S_SEL_REG_DORS_POS);
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok) {
        error =
            tfa98xx_write_register16(handle, TFA98XX_I2S_SEL_REG,
                        value);
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_select_stereo_gain_channel(Tfa98xx_handle_t handle,
                enum Tfa98xx_StereoGainSel gain_sel)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the I2S Control register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the bits first */
        value &= ~(0x1 << TFA98XX_I2SREG_CHS3_POS);
        switch (gain_sel) {
        case Tfa98xx_StereoGainSel_Left:
        case Tfa98xx_StereoGainSel_Right:
            /* enum definition matches the HW registers */
            value |= (gain_sel << TFA98XX_I2SREG_CHS3_POS);
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_write_register16(handle, TFA98XX_I2SREG, value);

    return error;
}

enum Tfa98xx_Error
tfa98xx_select_channel(Tfa98xx_handle_t handle, enum Tfa98xx_Channel channel)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 2 bits first */
        value &= ~(0x3 << TFA98XX_I2SREG_CHS12_POS);
        switch (channel) {
        case Tfa98xx_Channel_L:
            value |= (0x1 << TFA98XX_I2SREG_CHS12_POS);
            break;
        case Tfa98xx_Channel_R:
            value |= (0x2 << TFA98XX_I2SREG_CHS12_POS);
            break;
        case Tfa98xx_Channel_L_R:
            value |= (0x3 << TFA98XX_I2SREG_CHS12_POS);
            break;
        case Tfa98xx_Channel_Stereo:
            /* real stereo on 1 DSP not yet supported */
            error = Tfa98xx_Error_Not_Supported;
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_write_register16(handle, TFA98XX_I2SREG, value);

    return error;
}
enum Tfa98xx_Error
tfa98xx_enable_aecoutput(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
    unsigned short status;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    switch (handlesLocal[handle].rev) {
        case 0x97:
        {
            /* 97 powerdown already handled this internally by disabling TDM interface */
            return err;
        }
        default:
        {
            /* now wait for the amplifier to turn off */
            err = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &status);
            if (err != Tfa98xx_Error_Ok)
            {
                return err;
            }

            status |= TFA98XX_I2SREG_I2SDOE;

            err = tfa98xx_write_register16(handle, TFA98XX_I2SREG, status);

            if (err != Tfa98xx_Error_Ok)
            {
                return err;
            }
        }
    }
    return err;
}

enum Tfa98xx_Error
tfa98xx_disable_aecoutput(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
    unsigned short status;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    switch (handlesLocal[handle].rev) {
        case 0x97:
        {
            /* 97 powerdown already handled this internally by disabling TDM interface */
            return err;
        }
        default:
        {
            /* now wait for the amplifier to turn off */
            err = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &status);
            if (err != Tfa98xx_Error_Ok)
            {
                return err;
            }

            status &= ~TFA98XX_I2SREG_I2SDOE;

            err = tfa98xx_write_register16(handle, TFA98XX_I2SREG, status);

            if (err != Tfa98xx_Error_Ok)
            {
                return err;
            }
        }
    }
    return err;
}

enum Tfa98xx_Error
tfa98xx_select_mode(Tfa98xx_handle_t handle, enum Tfa98xx_Mode mode )
{
        enum Tfa98xx_Error error = Tfa98xx_Error_Bad_Parameter;
        unsigned short i2s_value;
        unsigned short sysctrl_value;
        unsigned short bat_volt;
        unsigned short tmp_value=0;
        int timeoutloop = 100;

        if (!tfa98xx_handle_is_open(handle))
            return Tfa98xx_Error_NotOpen;

        switch (handlesLocal[handle].rev) {
                case 0x97:
                {
                        /* read the SystemControl register,
                         *  modify the bit and write again */
                        error = tfa98xx_read_register16(handle,
                                                        TFA98XX_I2SREG,
                                                        &i2s_value);
                        error = tfa98xx_read_register16(handle,
                                                        TFA98XX_SYS_CTRL,
                                                        &sysctrl_value);
                        if (error == Tfa98xx_Error_Ok) {
                                switch (mode) {
                                        case Tfa98xx_Mode_Normal:
                                        {
                                                /* clear the 2 bits RCV */
                                                i2s_value &= ~(TFA98XX_AUDIOREG_RCV_MSK);
                                                sysctrl_value |= (TFA98XX_SYS_CTRL_DCA_MSK);
                                                error =
                                                tfa98xx_write_register16(
                                                        handle,
                                                        TFA98XX_I2SREG,
                                                        i2s_value);
                                                error =
                                                tfa98xx_write_register16(
                                                        handle,
                                                        TFA98XX_SYS_CTRL,
                                                        sysctrl_value);
                                        }
                                                break;
                                        case Tfa98xx_Mode_RCV:
                                        {
                                                /* decrement first, or the expresion after the loop gives a wrong result */
                                                while(--timeoutloop && error == Tfa98xx_Error_Ok) /* escape when we can't read the ADC's */
                                                {
                                                        error =
                                                        tfa98xx_read_register16(
                                                                handle,
                                                                TFA98XX_TEMPERATURE,
                                                                &tmp_value);
                                                        /* wait until th ADC's are up, 0x100 means not ready yet */
                                                        if (tmp_value<0x100)
                                                                break;
                                                }
                                                /* last iteration can still be valid and the loop may end due to read errors
                                                 * timeoutloop value seems redundant, but distuinguishes between read errors and timeout.
                                                 */
                                                if (timeoutloop==0 && tmp_value>=0x100)
                                                {
                                                        PRINT(" ADC's startup timed out\n");
                                                        error = Tfa98xx_Error_StateTimedOut;
                                                }
                                                if (error == Tfa98xx_Error_Ok) {
                                                    error =
                                                    tfa98xx_read_register16(
                                                                    handle,
                                                                    TFA98XX_BATTERYVOLTAGE,
                                                                    &bat_volt);
                                                    if (bat_volt < 838)
                                                    {
                                                            i2s_value |= (TFA98XX_AUDIOREG_RCV_MSK);
                                                            sysctrl_value &= ~(TFA98XX_SYS_CTRL_DCA_MSK);
                                                            error =
                                                            tfa98xx_write_register16(handle, TFA98XX_I2SREG, i2s_value);
                                                            error =
                                                            tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL, sysctrl_value);
                                                    }
                                                    else
                                                    {
                                                            PRINT(" Battery voltage too high\n");
                                                            break;
                                                    }
                                                } else {
                                                    printf(" Failed to read ADC's");
                                                    break;
                                                }

                                        }
                                        break;
                                        default:
                                                error = Tfa98xx_Error_Bad_Parameter;
                                }
                        }
                }
                break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

enum Tfa98xx_Error tfa98xx_get_mode( Tfa98xx_handle_t handle, enum Tfa98xx_Mode *mode )
{
        enum Tfa98xx_Error error = Tfa98xx_Error_Bad_Parameter;
        unsigned short audio_reg;
        if (!tfa98xx_handle_is_open(handle))
            return Tfa98xx_Error_NotOpen;
        if (NULL == mode) return Tfa98xx_Error_Bad_Parameter;
        *mode = Tfa98xx_Mode_Normal;    /* default is Normal */
        switch (handlesLocal[handle].rev) {
                case 0x97:
                {
                        error = tfa98xx_read_register16(handle,
                                                TFA98XX_AUDIOREG, &audio_reg);
                        if (error == Tfa98xx_Error_Ok )
                                *mode = (audio_reg & TFA98XX_AUDIOREG_RCV_MSK);
                }
                break;
                default:
                        error = Tfa98xx_Error_Bad_Parameter;
        }
        return error;
}

enum Tfa98xx_Error tfa98xx_set_sample_rate(Tfa98xx_handle_t handle, int rate)
{
    enum Tfa98xx_Error error;
    unsigned short value = 0;
   unsigned short index = 0;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 4 bits first */
        value &= (~(0xF << TFA98XX_I2SREG_I2SSR_POS));
        switch (rate) {
        case 48000:
            value |= TFA98XX_I2SCTRL_RATE_48000;
         index = (TFA98XX_I2SCTRL_RATE_48000 >> TFA98XX_I2SREG_I2SSR_POS);
         break;
        case 44100:
            value |= TFA98XX_I2SCTRL_RATE_44100;
         index = (TFA98XX_I2SCTRL_RATE_44100 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 32000:
            value |= TFA98XX_I2SCTRL_RATE_32000;
         index = (TFA98XX_I2SCTRL_RATE_32000 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 24000:
            value |= TFA98XX_I2SCTRL_RATE_24000;
         index = (TFA98XX_I2SCTRL_RATE_24000 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 22050:
            value |= TFA98XX_I2SCTRL_RATE_22050;
         index = (TFA98XX_I2SCTRL_RATE_22050 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 16000:
            value |= TFA98XX_I2SCTRL_RATE_16000;
         index = (TFA98XX_I2SCTRL_RATE_16000 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 12000:
            value |= TFA98XX_I2SCTRL_RATE_12000;
         index = (TFA98XX_I2SCTRL_RATE_12000 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 11025:
            value |= TFA98XX_I2SCTRL_RATE_11025;
         index = (TFA98XX_I2SCTRL_RATE_11025 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        case 8000:
            value |= TFA98XX_I2SCTRL_RATE_08000;
         index = (TFA98XX_I2SCTRL_RATE_08000 >> TFA98XX_I2SREG_I2SSR_POS);
            break;
        default:
            error = Tfa98xx_Error_Bad_Parameter;
        }
    }
    if (error == Tfa98xx_Error_Ok)    /*set sample rate */
        error = tfa98xx_write_register16(handle, TFA98XX_I2SREG, value);

    return error;
}

enum Tfa98xx_Error tfa98xx_get_sample_rate(Tfa98xx_handle_t handle, int *pRate)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    _ASSERT(pRate != 0);
    /* read the SystemControl register, modify the bit and write again */
    error = tfa98xx_read_register16(handle, TFA98XX_I2SREG, &value);
    if (error == Tfa98xx_Error_Ok) {
        /* clear the 4 bits first */
        value = value & (0xF << TFA98XX_I2SREG_I2SSR_POS);
        switch (value) {
        case TFA98XX_I2SCTRL_RATE_48000:
            *pRate = 48000;
            break;
        case TFA98XX_I2SCTRL_RATE_44100:
            *pRate = 44100;
            break;
        case TFA98XX_I2SCTRL_RATE_32000:
            *pRate = 32000;
            break;
        case TFA98XX_I2SCTRL_RATE_24000:
            *pRate = 24000;
            break;
        case TFA98XX_I2SCTRL_RATE_22050:
            *pRate = 22050;
            break;
        case TFA98XX_I2SCTRL_RATE_16000:
            *pRate = 16000;
            break;
        case TFA98XX_I2SCTRL_RATE_12000:
            *pRate = 12000;
            break;
        case TFA98XX_I2SCTRL_RATE_11025:
            *pRate = 11025;
            break;
        case TFA98XX_I2SCTRL_RATE_08000:
            *pRate = 8000;
            break;
        default:
            /* cannot happen, only 9 cases possible */
            _ASSERT(0);
        }
    }
    return error;
}


enum Tfa98xx_Error
tfa98xx_dsp_write_speaker_parameters(Tfa98xx_handle_t handle,
                  int length,
                  const unsigned char *pSpeakerBytes)
{
    enum Tfa98xx_Error error;
    if (pSpeakerBytes != 0) {
        /* by design: keep the data opaque and no
         * interpreting/calculation */
        /* Use long WaitResult retry count */
        error = tfa98xx_dsp_set_param_var_wait(
                    handle,
                    MODULE_SPEAKERBOOST,
                    SB_PARAM_SET_LSMODEL, length,
                    pSpeakerBytes,
                    TFA98XX_WAITRESULT_NTRIES_LONG);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }

    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_write_speaker_parameters_multiple(int handle_cnt,
                      Tfa98xx_handle_t
                      handles[],
                      int length,
                      const unsigned char *pSpeakerBytes)
{
    enum Tfa98xx_Error error;
    if (pSpeakerBytes != 0) {
        /* by design: keep the data opaque and no
         * interpreting/calculation */
        /* Use long WaitResult retry count */
        error = tfa98xx_dsp_set_param_multiple_var_wait(
                    handle_cnt, handles,
                    MODULE_SPEAKERBOOST,
                    SB_PARAM_SET_LSMODEL,
                    length,
                    pSpeakerBytes,
                    TFA98XX_WAITRESULT_NTRIES_LONG);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }

    return error;
}


enum Tfa98xx_Error
tfa98xx_dsp_read_spkr_params(Tfa98xx_handle_t handle,
              unsigned char paramId,
              int length, unsigned char *pSpeakerBytes)
{
    enum Tfa98xx_Error error;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (pSpeakerBytes != 0) {
        error =
            tfa98xx_dsp_get_param(handle, MODULE_SPEAKERBOOST,
                    paramId, length, pSpeakerBytes);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_dsp_support_tcoef(Tfa98xx_handle_t handle,
                    int *pbSupporttCoef)
{
    int featureBits[2];
    unsigned short featureBits1;
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    _ASSERT(pbSupporttCoef != 0);

    error = tfa98xx_dsp_get_sw_feature_bits(handle, featureBits);
    if (error == Tfa98xx_Error_Ok) {
        /* easy case: new API available => not the 87 */
        /* bit=0 means tCoef expected */
        *pbSupporttCoef = (featureBits[0] & FEATURE1_TCOEF) == 0;
    } else if (error == Tfa98xx_Error_RpcParamId) {
        /* feature bits available in MTP directly */
        error =
            tfa98xx_read_register16(handle, TFA98XX_MTP + 6,
                       &featureBits1);
        if (error == Tfa98xx_Error_Ok) {
            /* bit=0 means tCoef expected */
            *pbSupporttCoef = (featureBits1 & FEATURE1_TCOEF) == 0;
        }

    }
    /* else some other error, return transparantly */

    /* pbSupporttCoef only changed when error == Tfa98xx_Error_Ok */
    return error;
}


enum Tfa98xx_Error
tfa98xx_dsp_write_config(Tfa98xx_handle_t handle, int length,
               const unsigned char *pConfigBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    error =
        tfa98xx_dsp_set_param(handle, MODULE_SPEAKERBOOST,
                SB_PARAM_SET_CONFIG, length, pConfigBytes);


    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_write_config_multiple(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   int length, const unsigned char *pConfigBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    error =
        tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                    MODULE_SPEAKERBOOST,
                    SB_PARAM_SET_CONFIG, length,
                    pConfigBytes);

    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_write_preset(Tfa98xx_handle_t handle, int length,
               const unsigned char *pPresetBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    if (pPresetBytes != 0) {
        /* by design: keep the data opaque and no
         * interpreting/calculation */
        error =
            tfa98xx_dsp_set_param(handle, MODULE_SPEAKERBOOST,
                    SB_PARAM_SET_PRESET, length,
                    pPresetBytes);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_dsp_read_config(Tfa98xx_handle_t handle, int length,
                      unsigned char *pConfigBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    if (pConfigBytes != 0) {
        /* Here one can keep it simple by reading only the first
         * length bytes from DSP memory */
        error = tfa98xx_dsp_get_param(handle, MODULE_SPEAKERBOOST,
                        SB_PARAM_GET_CONFIG_PRESET, length,
                        pConfigBytes);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_dsp_read_preset(Tfa98xx_handle_t handle, int length,
                      unsigned char *pPresetBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    /* Here one cannot keep it simple as in Tfa98xx_DspReadConfig(), */
    /* since we are interested by the LAST length bytes from DSP memory */
    unsigned char temp[MAX_PARAM_SIZE];
    int configlength;

    if (pPresetBytes != 0) {
        error = tfa98xx_dsp_config_parameter_count(handle, &configlength);
        _ASSERT(error == Tfa98xx_Error_Ok);        /* an error should not happen */
        configlength *= 3; /* word to bytes count */
        error = tfa98xx_dsp_get_param(handle, MODULE_SPEAKERBOOST,
                        SB_PARAM_GET_CONFIG_PRESET,
                        (configlength + TFA98XX_PRESET_LENGTH), temp);
        if (error == Tfa98xx_Error_Ok) {
            int i;
            for (i = 0; i < length; i++) {
                pPresetBytes[i] =
                    temp[configlength + i];
            }
        }
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }

    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_write_preset_multiple(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   int length, const unsigned char *pPresetBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    if (pPresetBytes != 0) {
        /* by design: keep the data opaque and no
         * interpreting/calculation */
        error =
            tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                        MODULE_SPEAKERBOOST,
                        SB_PARAM_SET_PRESET, length,
                        pPresetBytes);
    } else
        error = Tfa98xx_Error_Bad_Parameter;

    return error;
}

enum Tfa98xx_Error tfa98xx_set_volume(Tfa98xx_handle_t handle, FIXEDPT voldB)
{
    /* This function is depricated, as floating point is not allowed in TFA
     * layer code.
     */
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    if (voldB > 0.0) {
        error = Tfa98xx_Error_Bad_Parameter;
    }
    /* 0x00 -> 0.0 dB
        * 0x01 -> -0.5 dB
        * ...
        * 0xFE -> -127dB
        * 0xFF -> muted
    */
    if (error == Tfa98xx_Error_Ok) {
        int volume_value;
#ifdef __KERNEL__
        volume_value = TO_INT(voldB * -2);
#else
        volume_value = (int)(voldB / (-0.5f));
#endif
        if (volume_value > 255)    /* restricted to 8 bits */
            volume_value = 255;

        error = tfa98xx_set_volume_level(handle, (unsigned short)volume_value);
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_set_volume_level(Tfa98xx_handle_t handle, unsigned short vol)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (vol > 255)    /* restricted to 8 bits */
        vol = 255;
    if ( (vol >= 0) && (vol <= 255) ) {
        /* 0x00 -> 0.0 dB
         * 0x01 -> -0.5 dB
         * ...
         * 0xFE -> -127dB
         * 0xFF -> muted
         */
        error =
            tfa98xx_read_register16(handle, TFA98XX_AUDIO_CTR, &value);
    } else {
        error = Tfa98xx_Error_Bad_Parameter;
    }
    if (error == Tfa98xx_Error_Ok) {
        /* volume value is in the top 8 bits of the register */
        value = (value & 0x00FF) | (unsigned short)(vol << 8);
        error =
            tfa98xx_write_register16(handle, TFA98XX_AUDIO_CTR, value);
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_get_volume(Tfa98xx_handle_t handle, FIXEDPT *pVoldB)
{
    enum Tfa98xx_Error error;
    unsigned short value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    _ASSERT(pVoldB != 0);
    error = tfa98xx_read_register16(handle, TFA98XX_AUDIO_CTR, &value);
    if (error == Tfa98xx_Error_Ok) {
        value >>= 8;
#ifdef __KERNEL__
        *pVoldB = TO_FIXED(value) / -2;
#else
        *pVoldB = (-0.5f) * value;
#endif
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_set_mute(Tfa98xx_handle_t handle, enum Tfa98xx_Mute mute)
{
    enum Tfa98xx_Error error;
    unsigned short audioctrl_value;
    unsigned short sysctrl_value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    error =
        tfa98xx_read_register16(
        handle, TFA98XX_AUDIO_CTR, &audioctrl_value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    error =
        tfa98xx_read_register16(handle, TFA98XX_SYS_CTRL, &sysctrl_value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    switch (mute) {
    case Tfa98xx_Mute_Off:
        /* previous state can be digital or amplifier mute,
         * clear the cf_mute and set the enbl_amplifier bits
         *
         * To reduce PLOP at power on it is needed to switch the
         * amplifier on with the DCDC in follower mode
         * (enbl_boost = 0 ?).
         * This workaround is also needed when toggling the
         * powerdown bit!
         */
        audioctrl_value &= ~(TFA98XX_AUDIO_CTR_CFSM_MSK);
        sysctrl_value |= (TFA98XX_SYS_CTRL_AMPE_MSK |
                 TFA98XX_SYS_CTRL_DCA_MSK);
        break;
    case Tfa98xx_Mute_Digital:
        /* expect the amplifier to run */
        /* set the cf_mute bit */
        audioctrl_value |= TFA98XX_AUDIO_CTR_CFSM_MSK;
        /* set the enbl_amplifier bit */
        sysctrl_value |= (TFA98XX_SYS_CTRL_AMPE_MSK);
        /* clear active mode */
        sysctrl_value &= ~(TFA98XX_SYS_CTRL_DCA_MSK);
        break;
    case Tfa98xx_Mute_Amplifier:
        /* clear the cf_mute bit */
        audioctrl_value &= ~TFA98XX_AUDIO_CTR_CFSM_MSK;
        /* clear the enbl_amplifier bit and active mode */
        sysctrl_value &=
            ~(TFA98XX_SYS_CTRL_AMPE_MSK | TFA98XX_SYS_CTRL_DCA_MSK);
        break;
    default:
        error = Tfa98xx_Error_Bad_Parameter;
    }
    if (error != Tfa98xx_Error_Ok)
        return error;
    error =
        tfa98xx_write_register16(handle, TFA98XX_AUDIO_CTR,
                    audioctrl_value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    error =
        tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL, sysctrl_value);
    return error;
}

enum Tfa98xx_Error
tfa98xx_get_mute(Tfa98xx_handle_t handle, enum Tfa98xx_Mute *pMute)
{
    enum Tfa98xx_Error error;
    unsigned short audioctrl_value;
    unsigned short sysctrl_value;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    _ASSERT(pMute != 0);
    error =
        tfa98xx_read_register16(
        handle, TFA98XX_AUDIO_CTR, &audioctrl_value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    error =
        tfa98xx_read_register16(handle, TFA98XX_SYS_CTRL, &sysctrl_value);
    if (error != Tfa98xx_Error_Ok)
        return error;
    if (sysctrl_value & TFA98XX_SYS_CTRL_AMPE_MSK) {
        /* amplifier is enabled */
        if (audioctrl_value & (TFA98XX_AUDIO_CTR_CFSM_MSK))
            *pMute = Tfa98xx_Mute_Digital;
        else
            *pMute = Tfa98xx_Mute_Off;
    } else {
        /* amplifier disabled. */
        *pMute = Tfa98xx_Mute_Amplifier;
    }
    return Tfa98xx_Error_Ok;
}

/**
 convert signed 24 bit integers to 32bit aligned bytes
   input:   data contains "num_bytes/3" int24 elements
   output:  bytes contains "num_bytes" byte elements
*/

void tfa98xx_convert_data2bytes(int num_data, const int data[],
                   unsigned char bytes[])
{
    int i;            /* index for data */
    int k;            /* index for bytes */
    int d;
    /* note: cannot just take the lowest 3 bytes from the 32 bit
     * integer, because also need to take care of clipping any
     * value > 2&23 */
    for (i = 0, k = 0; i < num_data; ++i, k += 3) {
        if (data[i] >= 0)
            d = MIN(data[i], (1 << 23) - 1);
        else {
            /* 2's complement */
            d = (1 << 24) - MIN(-data[i], 1 << 23);
        }
        _ASSERT(d >= 0);
        _ASSERT(d < (1 << 24));    /* max 24 bits in use */
        bytes[k] = (d >> 16) & 0xFF;    /* MSB */
        bytes[k + 1] = (d >> 8) & 0xFF;
        bytes[k + 2] = (d) & 0xFF;    /* LSB */
    }
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_coeff(Tfa98xx_handle_t handle,
               int biquad_index, const unsigned char *bytes)
{
    return tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (int)(BIQUAD_COEFF_SIZE * 3), bytes);
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_all_coeff(Tfa98xx_handle_t handle,
                  const unsigned char *bytes)
{
    return tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                    0 /* program all at once */ ,
                    (unsigned char)(BIQUAD_COEFF_SIZE *
                        TFA98XX_BIQUAD_NUM * 3),
                    bytes);
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_all_coeff_multiple(int handle_cnt, Tfa98xx_handle_t handles[],
                  const unsigned char *bytes)
{
    return tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                        MODULE_BIQUADFILTERBANK,
                        0 /* program all at once */ ,
                        (unsigned
                         char)(BIQUAD_COEFF_SIZE *
                               TFA98XX_BIQUAD_NUM * 3),
                        bytes);
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_coeff_bytes(Tfa98xx_handle_t handle,
                int biquad_index,
                const unsigned char *pBiquadBytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;
    if (pBiquadBytes == 0)
        return Tfa98xx_Error_Bad_Parameter;

    if (error == Tfa98xx_Error_Ok) {
        error = tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                        (unsigned char)biquad_index,
                        (BIQUAD_COEFF_SIZE * 3),
                        pBiquadBytes);
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_set_coeff_multiple_bytes(int handle_cnt,
                    Tfa98xx_handle_t handles[],
                    int biquad_index,
                    const unsigned char *pBiquadBytes)
{
    enum Tfa98xx_Error error;
    if (biquad_index > TFA98XX_BIQUAD_NUM)
        return Tfa98xx_Error_Bad_Parameter;
    if (biquad_index < 1)
        return Tfa98xx_Error_Bad_Parameter;
    if (pBiquadBytes == 0)
        return Tfa98xx_Error_Bad_Parameter;
    error =
        tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                    MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (BIQUAD_COEFF_SIZE * 3),
                    pBiquadBytes);
    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_disable(Tfa98xx_handle_t handle,
                    int biquad_index,
                    const unsigned char *bytes)
{
    return
        tfa98xx_dsp_set_param(handle, MODULE_BIQUADFILTERBANK,
                (unsigned char)biquad_index,
                (unsigned char)BIQUAD_COEFF_SIZE * 3, bytes);
}

enum Tfa98xx_Error
tfa98xx_dsp_biquad_disable_multiple(int handle_cnt,
                  Tfa98xx_handle_t handles[], int biquad_index,
                            const unsigned char *bytes)
{
    return tfa98xx_dsp_set_param_multiple(handle_cnt, handles,
                    MODULE_BIQUADFILTERBANK,
                    (unsigned char)biquad_index,
                    (unsigned char)BIQUAD_COEFF_SIZE * 3,
                    bytes);
}

#define PATCH_HEADER_LENGTH 6
enum Tfa98xx_Error
tfa98xx_dsp_patch(Tfa98xx_handle_t handle, int patchLength,
         const unsigned char *patchBytes)
{
    enum Tfa98xx_Error error;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (patchLength < PATCH_HEADER_LENGTH)
        return Tfa98xx_Error_Bad_Parameter;
    error = tfa98xx_check_ic_rom_version(handle, patchBytes);
    if (Tfa98xx_Error_Ok != error)
        return error;
    error =
        tfa98xx_process_patch_file(handle, patchLength - PATCH_HEADER_LENGTH,
                 patchBytes + PATCH_HEADER_LENGTH);
    return error;
}

/* read the return code for the RPC call */
enum Tfa98xx_Error
tfa98xx_check_rpc_status(Tfa98xx_handle_t handle, int *pRpcStatus)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    /* the value to sent to the * CF_CONTROLS register: cf_req=00000000,
     * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
    unsigned short cf_ctrl = 0x0002;
    /* memory address to be accessed (0: Status, 1: ID, 2: parameters) */
    unsigned short cf_mad = 0x0000;
    unsigned char mem[3];    /* for the status read from DSP memory */

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    if (pRpcStatus == 0)
        return Tfa98xx_Error_Bad_Parameter;
#ifdef OPTIMIZED_RPC
    {
        /* minimize the number of I2C transactions by making use
         * of the autoincrement in I2C */
        unsigned char buffer[4];
        /* first the data for CF_CONTROLS */
        buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
        buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
        /* write the contents of CF_MAD which is the subaddress
         * following CF_CONTROLS */
        buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
        buffer[3] = (unsigned char)(cf_mad & 0xFF);
        error =
            tfa98xx_write_data(handle, TFA98XX_CF_CONTROLS,
                      sizeof(buffer), buffer);
    }
#else /* OPTIMIZED_RPC */
    /* 1) write DMEM=XMEM to the DSP XMEM */
    if (error == Tfa98xx_Error_Ok) {
        error =
            tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS,
                        cf_ctrl);
    }

    if (error == Tfa98xx_Error_Ok) {
        /* write the address in XMEM where to read */
        error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, cf_mad);
    }
#endif /* OPTIMIZED_RPC */
    if (error == Tfa98xx_Error_Ok) {
        /* read 1 word (24 bit) from XMEM */
        error =
            tfa98xx_read_data(handle, TFA98XX_CF_MEM,
                        3 /*sizeof(mem) */ , mem);
    }
    if (error == Tfa98xx_Error_Ok)
        *pRpcStatus = (mem[0] << 16) | (mem[1] << 8) | mem[2];
    return error;
}

/* check that num_byte matches the memory type selected */
enum Tfa98xx_Error
tfa98xx_check_size(enum Tfa98xx_DMEM which_mem, int num_bytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int modulo_size = 1;
    switch (which_mem) {
    case Tfa98xx_DMEM_PMEM:
        /* 32 bit PMEM */
        modulo_size = 4;
        break;
    case Tfa98xx_DMEM_XMEM:
    case Tfa98xx_DMEM_YMEM:
    case Tfa98xx_DMEM_IOMEM:
        /* 24 bit MEM */
        modulo_size = 3;
        break;
    default:
        error = Tfa98xx_Error_Bad_Parameter;
    }
    if (error == Tfa98xx_Error_Ok) {
        if ((num_bytes % modulo_size) != 0)
            error = Tfa98xx_Error_Bad_Parameter;

    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_write_parameter(Tfa98xx_handle_t handle,
           unsigned char module_id,
           unsigned char param_id,
           int num_bytes, const unsigned char data[])
{
    enum Tfa98xx_Error error;
    unsigned char buffer[7];

    /* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
     * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
    unsigned short cf_ctrl = 0x0002;
    /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters)*/
    unsigned short cf_mad = 0x0001;

    error = tfa98xx_check_size(Tfa98xx_DMEM_XMEM, num_bytes);
    if (error == Tfa98xx_Error_Ok) {
        if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE))
            error = Tfa98xx_Error_Bad_Parameter;
    }


        /* minimize the number of I2C transactions by making use of
         * the autoincrement in I2C */

        /* first the data for CF_CONTROLS */
        buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
        buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
        /* write the contents of CF_MAD which is the subaddress
         * following CF_CONTROLS */
        buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
        buffer[3] = (unsigned char)(cf_mad & 0xFF);
        /* write the module and RPC id into CF_MEM, which
         * follows CF_MAD */
        buffer[4] = 0;
        buffer[5] = module_id + 128;
        buffer[6] = param_id;
        error =
            tfa98xx_write_data(handle, TFA98XX_CF_CONTROLS,
                      sizeof(buffer), buffer);


    if (error == Tfa98xx_Error_Ok) {
        int offset = 0;
        int chunk_size =
            ROUND_DOWN(NXP_I2C_BufferSize(), 3);  /* XMEM word size */
        int remaining_bytes = num_bytes;
        /* due to autoincrement in cf_ctrl, next write will happen at
         * the next address */
        while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
            if (remaining_bytes < chunk_size)
                chunk_size = remaining_bytes;
            /* else chunk_size remains at initialize value above */
            error =
                tfa98xx_write_data(handle, TFA98XX_CF_MEM,
                          chunk_size, data + offset);
            remaining_bytes -= chunk_size;
            offset += chunk_size;
        }
    }
    return error;
}

enum Tfa98xx_Error tfa98xx_execute_param(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error error;
    /* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
     * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
    unsigned short cf_ctrl = 0x0002;
    cf_ctrl |= (1 << 8) | (1 << 4);    /* set the cf_req1 and cf_int bit */
    error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
    return error;
}

enum Tfa98xx_Error
tfa98xx_wait_result(Tfa98xx_handle_t handle, int waitRetryCount)
{
    enum Tfa98xx_Error error;
    unsigned short cf_status; /* the contents of the CF_STATUS register */
    int tries = 0;
    do {
        error =
            tfa98xx_read_register16(handle, TFA98XX_CF_STATUS,
                       &cf_status);
        tries++;
    }
    /* don't wait forever, DSP is pretty quick to respond (< 1ms) */
    while ((error == Tfa98xx_Error_Ok) && ((cf_status & 0x0100) == 0)
            && (tries < waitRetryCount));
    if (tries >= waitRetryCount) {
        /* something wrong with communication with DSP */
        error = Tfa98xx_Error_DSP_not_running;
    }
    return error;
}

/* Execute RPC protocol to write something to the DSP */
enum Tfa98xx_Error
tfa98xx_dsp_set_param_var_wait(Tfa98xx_handle_t handle,
               unsigned char module_id,
               unsigned char param_id, int num_bytes,
               const unsigned char data[], int waitRetryCount)
{
    enum Tfa98xx_Error error;
    int rpcStatus = STATUS_OK;
    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* 1) write the id and data to the DSP XMEM */
    error = tfa98xx_write_parameter(handle, module_id, param_id, num_bytes, data);
    /* 2) wake up the DSP and let it process the data */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_execute_param(handle);
    /* check the result when addressed an IC uniquely */
    if (handlesLocal[handle].slave_address !=
                TFA98XX_GENERIC_SLAVE_ADDRESS) {
        /* 3) wait for the ack */
        if (error == Tfa98xx_Error_Ok)
            error = tfa98xx_wait_result(handle, waitRetryCount);
        /* 4) check the RPC return value */
        if (error == Tfa98xx_Error_Ok)
            error = tfa98xx_check_rpc_status(handle, &rpcStatus);
        if (error == Tfa98xx_Error_Ok) {
            if (rpcStatus != STATUS_OK) {
                /* DSP RPC call returned an error */
                error =
                    (enum Tfa98xx_Error) (rpcStatus +
                               Tfa98xx_Error_RpcBase);
            }
        }
    }
    return error;
}

/* Execute RPC protocol to write something to the DSP */
enum Tfa98xx_Error
tfa98xx_dsp_set_param(Tfa98xx_handle_t handle,
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
enum Tfa98xx_Error
tfa98xx_dsp_set_param_multiple_var_wait(int handle_cnt,
                   Tfa98xx_handle_t handles[],
                   unsigned char module_id,
                   unsigned char param_id,
                   int num_bytes, const unsigned char data[],
                   int waitRetryCount)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int i;
    int rpcStatus = STATUS_OK;
    for (i = 0; i < handle_cnt; ++i) {
        if (!tfa98xx_handle_is_open(handles[i]))
            return Tfa98xx_Error_NotOpen;
    }
    /* from here onward, any error will fall through without executing the
     * following for loops */
    /* 1) write the id and data to the DSP XMEM */
    for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
        error =
            tfa98xx_write_parameter(handles[i], module_id, param_id, num_bytes,
                   data);
    }
    /* 2) wake up the DSP and let it process the data */
    for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
        error = tfa98xx_execute_param(handles[i]);
    }
    /* 3) wait for the ack */
    for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
        error = tfa98xx_wait_result(handles[i], waitRetryCount);
    }
    /* 4) check the RPC return value */
    for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
        error = tfa98xx_check_rpc_status(handles[i], &rpcStatus);
        if (rpcStatus != STATUS_OK) {
            /* DSP RPC call returned an error */
            error =
                (enum Tfa98xx_Error) (rpcStatus +
                           Tfa98xx_Error_RpcBase);
            /* stop at first error */
            return error;
        }
    }
    return error;
}

/* Execute RPC protocol to write something to all the DSPs interleaved,
 * stop at the first error
 * optimized to minimize the latency between the execution point on the
 * various DSPs.
 * Uses small retry count.
 */
enum Tfa98xx_Error
tfa98xx_dsp_set_param_multiple(int handle_cnt,
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
enum Tfa98xx_Error tfa98xx_dsp_get_param(Tfa98xx_handle_t handle,
            unsigned char module_id,
            unsigned char param_id,
            int num_bytes, unsigned char data[])
{
    enum Tfa98xx_Error error;
    /* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
     * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
    unsigned short cf_ctrl = 0x0002;
    /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters)*/
    unsigned short cf_mad = 0x0001;
    /* the contents of the CF_STATUS register */
    unsigned short cf_status;
    int rpcStatus = STATUS_OK;

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;
    /* allow generic access in multi debugging only */
    if (handlesLocal[handle].slave_address ==
                TFA98XX_GENERIC_SLAVE_ADDRESS) {
        /* cannot read */
        return Tfa98xx_Error_Bad_Parameter;
    }
    error = tfa98xx_check_size(Tfa98xx_DMEM_XMEM, num_bytes);
    if (error == Tfa98xx_Error_Ok) {
        if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE))
            error = Tfa98xx_Error_Bad_Parameter;
        }

    {
        /* minimize the number of I2C transactions by making use of
         * the autoincrement in I2C */
        unsigned char buffer[7];
        /* first the data for CF_CONTROLS */
        buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
        buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
        /* write the contents of CF_MAD which is the subaddress
         * following CF_CONTROLS */
        buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
        buffer[3] = (unsigned char)(cf_mad & 0xFF);
        /* write the module and RPC id into CF_MEM,
         * which follows CF_MAD */
        buffer[4] = 0;
        buffer[5] = module_id + 128;
        buffer[6] = param_id;

        error = tfa98xx_write_data(handle, TFA98XX_CF_CONTROLS, sizeof(buffer), buffer);
    }

    /* 2) wake up the DSP and let it process the data */
    if (error == Tfa98xx_Error_Ok) {
        /* set the cf_req1 and cf_int bit */
        cf_ctrl |= (1 << 8) | (1 << 4);
        error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
    }

    /* 3) wait for the ack */
    if (error == Tfa98xx_Error_Ok) {
        int tries = 0;
        do {
            error = tfa98xx_read_register16(handle, TFA98XX_CF_STATUS, &cf_status);
            tries++;
        }

        /* don't wait forever, DSP is pretty quick to respond (< 1ms)*/
        while ((error == Tfa98xx_Error_Ok)
            && ((cf_status & 0x0100) == 0)
            && (tries < TFA98XX_WAITRESULT_NTRIES))
            ;

        if (tries >= TFA98XX_WAITRESULT_NTRIES) {
            /* something wrong with communication with DSP */
            return Tfa98xx_Error_DSP_not_running;
        }
    }

    /* 4) check the RPC return value */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_check_rpc_status(handle, &rpcStatus);

    if (error == Tfa98xx_Error_Ok) {
        if (rpcStatus != STATUS_OK) {
            /* DSP RPC call returned an error */
            error = (enum Tfa98xx_Error) (rpcStatus + Tfa98xx_Error_RpcBase);
        }
    }

    /* 5) read the resulting data */
    if (error == Tfa98xx_Error_Ok) {
        /* memory address to be accessed (0: Status,
         * 1: ID, 2: parameters) */
        cf_mad = 0x0002;
        error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, cf_mad);
    }

    if (error == Tfa98xx_Error_Ok) {
        int offset = 0;
        int chunk_size = ROUND_DOWN(NXP_I2C_BufferSize(), 3 /* XMEM word size */ );
        int remaining_bytes = num_bytes;
        /* due to autoincrement in cf_ctrl, next write will happen at
         * the next address */
        while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
            if (remaining_bytes < NXP_I2C_BufferSize())
                chunk_size = remaining_bytes;

            /* else chunk_size remains at initialize value above */
            error = tfa98xx_read_data(handle, TFA98XX_CF_MEM, chunk_size, data + offset);
            remaining_bytes -= chunk_size;
            offset += chunk_size;
        }
    }

    return error;
}

/**
 convert memory bytes to signed 24 bit integers
   input:  bytes contains "num_bytes" byte elements
   output: data contains "num_bytes/3" int24 elements
*/
void tfa98xx_convert_bytes2data(int num_bytes, const unsigned char bytes[],
                   int data[])
{
    int i;            /* index for data */
    int k;            /* index for bytes */
    int d;
    int num_data = num_bytes / 3;
    _ASSERT((num_bytes % 3) == 0);
    for (i = 0, k = 0; i < num_data; ++i, k += 3) {
        d = (bytes[k] << 16) | (bytes[k + 1] << 8) | (bytes[k + 2]);
        _ASSERT(d >= 0);
        _ASSERT(d < (1 << 24));    /* max 24 bits in use */
        if (bytes[k] & 0x80)    /* sign bit was set */
            d = -((1 << 24) - d);

        data[i] = d;
    }
}

enum Tfa98xx_Error
tfa98xx_dsp_get_state_info(Tfa98xx_handle_t handle,
            struct Tfa98xx_StateInfo *pInfo)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    int data[FW_STATE_MAX_SIZE];    /* allocate worst case */
    unsigned char bytes[FW_STATE_MAX_SIZE * 3];
    /* contains the actual amount of parameters transferred,
     * depends on IC and ROM code version */
    int stateSize = FW_STATE_SIZE;
    int i;
    int bSupportFramework = 0;


    _ASSERT(pInfo != 0);

    /* init to default value to have sane values even when
     * some features aren't supported */
    for (i = 0; i < FW_STATE_MAX_SIZE; i++)
        data[i] = 0;


    error = Tfa98xx_DspSupportFramework(handle, &bSupportFramework);
    if (error != Tfa98xx_Error_Ok)
        return error;

    if (bSupportFramework) {
        error =
            tfa98xx_dsp_get_param(handle, MODULE_FRAMEWORK,
                    FW_PARAM_GET_STATE, 3 * stateSize,
                    bytes);
    } else {
        /* old ROM code, ask SpeakerBoost and only do first portion */
        stateSize = 8;
        error =
            tfa98xx_dsp_get_param(handle, MODULE_SPEAKERBOOST,
                    SB_PARAM_GET_STATE, 3 * stateSize,
                    bytes);
    }
    tfa98xx_convert_bytes2data(3 * stateSize, bytes, data);

        pInfo->agcGain = (FIXEDPT)data[0] / (1 << (23 - SPKRBST_AGCGAIN_EXP));     /* /2^23*2^(SPKRBST_AGCGAIN_EXP) */
        pInfo->limGain = (FIXEDPT)data[1] / (1 << (23 - SPKRBST_LIMGAIN_EXP));     /* /2^23*2^(SPKRBST_LIMGAIN_EXP) */
        pInfo->sMax =    (FIXEDPT)data[2] / (1 << (23 - SPKRBST_HEADROOM));        /* /2^23*2^(SPKRBST_HEADROOM)    */
        pInfo->T =              data[3] / (1 << (23 - SPKRBST_TEMPERATURE_EXP)); /* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
        pInfo->statusFlag =     data[4];
        pInfo->X1 =      (FIXEDPT)data[5] / (1 << (23 - SPKRBST_HEADROOM));        /* /2^23*2^(SPKRBST_HEADROOM)        */
        /* deal with type specific variations */
        pInfo->X2 =      (FIXEDPT)data[6] / (1 << (23 - SPKRBST_HEADROOM));        /* /2^23*2^(SPKRBST_HEADROOM)        */
        pInfo->Re =      (FIXEDPT)data[7] / (1 << (23 - SPKRBST_TEMPERATURE_EXP)); /* /2^23*2^(SPKRBST_TEMPERATURE_EXP) */
        pInfo->shortOnMips = data[8];


    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_get_calibration_impedance(Tfa98xx_handle_t handle, FIXEDPT *pRe25)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned char bytes[3];
    int data[1];
    int calibrateDone;

    _ASSERT(pRe25 != 0);
    *pRe25 = 0.0f;        /* default 0.0 */
    error = tfa98xx_dsp_read_mem(handle, 231, 1, &calibrateDone);
    if (error == Tfa98xx_Error_Ok) {
        if (!calibrateDone) {
            /* return the default */
            return error;
        }
        error = tfa98xx_dsp_get_param(handle, MODULE_SPEAKERBOOST,
                        SB_PARAM_GET_RE0, 3, bytes);
    }
    if (error == Tfa98xx_Error_Ok) {
        tfa98xx_convert_bytes2data(3, bytes, data);
        /* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
#ifdef __KERNEL__
        *pRe25 = TO_FIXED(data[0]) / (1 << (23 -
                        SPKRBST_TEMPERATURE_EXP));
#else
        *pRe25 = (FIXEDPT) data[0] / (1 << (23 -
                        SPKRBST_TEMPERATURE_EXP));
#endif
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_set_calibration_impedance(Tfa98xx_handle_t handle, const unsigned char *bytes)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Not_Supported;

    /* Set new Re0. */
    error = tfa98xx_dsp_set_param_var_wait(handle, MODULE_SETRE,
                SB_PARAM_SET_RE0, 3, bytes, TFA98XX_WAITRESULT_NTRIES_LONG);
    if (error == Tfa98xx_Error_Ok) {
        /* reset the DSP to take the newly set re0*/
        error = tfa98xx_dsp_reset(handle, 1);
        if (error == Tfa98xx_Error_Ok)
            error = tfa98xx_dsp_reset(handle, 0);
    }
    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_read_mem(Tfa98xx_handle_t handle,
           unsigned short start_offset, int num_words, int *pValues)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short cf_ctrl;    /* to sent to the CF_CONTROLS register */
    unsigned char bytes[MAX_PARAM_SIZE];
    int burst_size;        /* number of words per burst size */
    int bytes_per_word = 3;
    int num_bytes;
    int *p;
    /* first set DMEM and AIF, leaving other bits intact */
    error = tfa98xx_read_register16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;

    cf_ctrl &= ~0x000E;    /* clear AIF & DMEM */
    /* set DMEM, leave AIF cleared for autoincrement */
    cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);
    error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;

    error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, start_offset);
    if (error != Tfa98xx_Error_Ok)
        return error;

    num_bytes = num_words * bytes_per_word;
    p = pValues;
    for (; num_bytes > 0;) {
        burst_size = ROUND_DOWN(NXP_I2C_BufferSize(), bytes_per_word);
        if (num_bytes < burst_size)
            burst_size = num_bytes;

        _ASSERT(burst_size <= sizeof(bytes));
        error =
            tfa98xx_read_data(
            handle, TFA98XX_CF_MEM, burst_size, bytes);
        if (error != Tfa98xx_Error_Ok)
            return error;

        tfa98xx_convert_bytes2data(burst_size, bytes, p);
        num_bytes -= burst_size;
        p += burst_size / bytes_per_word;
    }
    return Tfa98xx_Error_Ok;
}

enum Tfa98xx_Error
tfa98xx_dsp_write_mem(Tfa98xx_handle_t handle, unsigned short address, int value, int memtype)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short cf_ctrl;    /* to send to the CF_CONTROLS register */
    unsigned char bytes[3];
    /* first set DMEM and AIF, leaving other bits intact */
    error = tfa98xx_read_register16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;

    cf_ctrl &= ~0x000E;    /* clear AIF & DMEM */
    /* set DMEM, leave AIF cleared for autoincrement */

    switch(memtype) {
        case Tfa98xx_DMEM_PMEM:
                cf_ctrl |= (Tfa98xx_DMEM_PMEM << 1);
                break;
        case Tfa98xx_DMEM_XMEM:
                cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);
                break;
        case Tfa98xx_DMEM_YMEM:
                cf_ctrl |= (Tfa98xx_DMEM_YMEM << 1);
                break;
        case Tfa98xx_DMEM_IOMEM:
                cf_ctrl |= (Tfa98xx_DMEM_IOMEM << 1);
                break;
    }

    error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;
    error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, address);
    if (error != Tfa98xx_Error_Ok)
        return error;

    tfa98xx_convert_data2bytes(1, &value, bytes);
    error = tfa98xx_write_data(handle, TFA98XX_CF_MEM, 3, bytes);
    if (error != Tfa98xx_Error_Ok)
        return error;

    return Tfa98xx_Error_Ok;
}

/* Execute generic RPC protocol that has both input and output parameters */
enum Tfa98xx_Error
tfa98xx_dsp_execute_rpc(Tfa98xx_handle_t handle,
              unsigned char module_id,
              unsigned char param_id, int num_inbytes,
              unsigned char indata[], int num_outbytes,
              unsigned char outdata[])
{
    enum Tfa98xx_Error error;
    int rpcStatus = STATUS_OK;
    int i;

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    /* 1) write the id and data to the DSP XMEM */
    error = tfa98xx_write_parameter(handle, module_id, param_id, num_inbytes, indata);

    /* 2) wake up the DSP and let it process the data */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_execute_param(handle);

    /* 3) wait for the ack, but not too long */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_wait_result(handle, TFA98XX_WAITRESULT_NTRIES);

    /* 4) check the RPC return value */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_check_rpc_status(handle, &rpcStatus);

    if (error == Tfa98xx_Error_Ok) {
        if (rpcStatus != STATUS_OK) {
            /* DSP RPC call returned an error */
            error = (enum Tfa98xx_Error) (rpcStatus + Tfa98xx_Error_RpcBase);
        }
    }

    /* 5) read the resulting data */
    error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, 2 /*start_offset */ );

    if (error != Tfa98xx_Error_Ok)
        return error;

    /* read in chunks, limited by max I2C length */
    for (i = 0; i < num_outbytes;) {
        int burst_size = ROUND_DOWN(NXP_I2C_BufferSize(), 3);  /*bytes_per_word */
        if ((num_outbytes - i) < burst_size)
            burst_size = num_outbytes - i;

        error = tfa98xx_read_data(handle, TFA98XX_CF_MEM, burst_size, outdata + i);
        if (error != Tfa98xx_Error_Ok)
            return error;

        i += burst_size;
    }

    return error;
}

enum Tfa98xx_Error
tfa98xx_dsp_read_memory(Tfa98xx_handle_t handle, enum Tfa98xx_DMEM which_mem,
              unsigned short start_offset, int num_words, int *pValues)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    int input[3];
    unsigned char input_bytes[3 * 3];
    unsigned char output_bytes[80 * 3];

    /* want to read all in 1 RPC call, so limit the max size */
    if (num_words > 80)
        return Tfa98xx_Error_Bad_Parameter;

    input[0] = which_mem;
    input[1] = start_offset;
    input[2] = num_words;
    tfa98xx_convert_data2bytes(3, input, input_bytes);

    error =
        tfa98xx_dsp_execute_rpc(handle, 0 /* moduleId */ , 5 /* paramId */ ,
                  sizeof(input_bytes), input_bytes,
                  num_words * 3, output_bytes);

    tfa98xx_convert_bytes2data(num_words * 3, output_bytes, pValues);

    return error;
}

enum Tfa98xx_Error tfa98xx_dsp_write_memory(Tfa98xx_handle_t handle,
                       enum Tfa98xx_DMEM which_mem,
                       unsigned short start_offset,
                       int num_words, int *pValues)
{
    int output[3];
    unsigned char output_bytes[83 * 3];
    int num_bytes;

    /* want to read all in 1 RPC call, so limit the max size */
    if (num_words > 80)
        return Tfa98xx_Error_Bad_Parameter;

    output[0] = which_mem;
    output[1] = start_offset;
    output[2] = num_words;
    tfa98xx_convert_data2bytes(3, output, output_bytes);
    tfa98xx_convert_data2bytes(num_words, pValues, output_bytes + 3 * 3);

    num_bytes = 3 * (num_words + 3);

    return tfa98xx_dsp_set_param(handle, 0 /* framework */ , 4 /* param */ ,
                   num_bytes, output_bytes);
}

enum Tfa98xx_Error
    tfa98xx_after_patch_hook(int handle_cnt, int parm) {
        enum Tfa98xx_Error error;
        unsigned char buf[]={00,00,00,
                             00,00,00,
                             00,00,00,
                             00,00,00,
                             00,00,00,00,00,00,00,00,00,00,00,00,00,00,01};

        PRINT("%s : %d\n", __FUNCTION__, parm);
        error =
            tfa98xx_dsp_set_param ( handle_cnt, 0, 3, 27, buf);

        return  error;
}
enum Tfa98xx_Error
    tfa98xx_after_calibration_hook(int handle_cnt, int parm) {
        enum Tfa98xx_Error error;
        PRINT("%s : %d\n", __FUNCTION__, parm);
        error = tfa98xx_dsp_write_mem(handle_cnt, 1951 , 0, Tfa98xx_DMEM_XMEM); // lsMod1StTap need to be set after configured
        return  error;
}
/*
 * load the tables to the DSP, called after patch load is done
 */
enum Tfa98xx_Error tfa98xx_dsp_write_tables(int handle){
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    return error;
}
/*
 * write/read raw msg functions :
 *  the buffer is provided in little endian format, each word occupying 3 bytes, length is in bytes.
 *  The functions will return immediately and do not not wait for DSP reponse.
 */
#define MAX_WORDS (300)
enum Tfa98xx_Error tfa98xx_dsp_msg(int handle, int length, const char *buf, int *msg_status){
    enum Tfa98xx_Error error;
    int tries;

    /* write the message and notify the DSP */
    error = tfa98xx_dsp_msg_write(handle, length, buf);
    if( error != Tfa98xx_Error_Ok)
        return error;

    /* get the result from the DSP (polling) */
    for(tries=TFA98XX_WAITRESULT_NTRIES; tries>0;tries--) {
        error = tfa98xx_dsp_msg_status(handle, msg_status);
        if (error ==Tfa98xx_Error_Ok )
            break;
    }

    return error;
}
enum Tfa98xx_Error tfa98xx_dsp_msg_write(int handle, int length, const char *buf){
    unsigned char buffer[2*2 + MAX_WORDS*3]; /* all data for single i2c burst */
    int offset = 0;
    int chunk_size = ROUND_DOWN(NXP_I2C_BufferSize(), 3);  /* XMEM word size */
    int remaining_bytes = length+4;
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    buffer[0] = 0x00;
    buffer[1] = 0x02; /* set cf ctl to DMEM */
    buffer[2] = 0x00;
    buffer[3] = 0x01; /* xmem[1] is start of message */
    memcpy( &buffer[4], buf,  length);

    /* due to autoincrement in cf_ctrl, next write will happen at
     * the next address */
    while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
        if (remaining_bytes < chunk_size)
            chunk_size = remaining_bytes;
        /* else chunk_size remains at initialize value above */
        error =
            tfa98xx_write_data(handle, TFA98XX_CF_CONTROLS,
                      chunk_size, buffer + offset);
        remaining_bytes -= chunk_size;
        offset += chunk_size;
    }

    /* notify the DSP */
    if (error == Tfa98xx_Error_Ok)
        error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, 0x112);

    return error;
}

enum Tfa98xx_Error tfa98xx_dsp_msg_read(int handle,int length, unsigned char *bytes){
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short cf_ctrl;    /* to sent to the CF_CONTROLS register */
    int burst_size;        /* number of words per burst size */
    int bytes_per_word = 3;
    int num_bytes;

    unsigned short start_offset=2; /* msg starts @xmem[2] ,[1]=cmd */

    if ( length > MAX_PARAM_SIZE)
        return Tfa98xx_Error_Bad_Parameter;

    /* first set DMEM and AIF, leaving other bits intact */
    error = tfa98xx_read_register16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;

    cf_ctrl &= ~0x000E;    /* clear AIF & DMEM */
    /* set DMEM, leave AIF cleared for autoincrement */
    cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);
    error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
    if (error != Tfa98xx_Error_Ok)
        return error;

    error = tfa98xx_write_register16(handle, TFA98XX_CF_MAD, start_offset);
    if (error != Tfa98xx_Error_Ok)
        return error;

    num_bytes = length; /* input param */
//    p = pValues;
    for (; num_bytes > 0;) {
        burst_size = ROUND_DOWN(NXP_I2C_BufferSize(), bytes_per_word);
        if (num_bytes < burst_size)
            burst_size = num_bytes;
        error =
            tfa98xx_read_data(handle, TFA98XX_CF_MEM, burst_size, bytes);
        if (error != Tfa98xx_Error_Ok)
            return error;

//        tfa98xx_convert_bytes2data(burst_size, bytes, p);
        num_bytes -= burst_size;

    }
    return Tfa98xx_Error_Ok;
}

/*
* status function to retrieve command/msg status:
* return a <0 status of the DSP did not ACK.
*/
enum Tfa98xx_Error tfa98xx_dsp_msg_status(Tfa98xx_handle_t handle, int *pRpcStatus) {
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

    error = tfa98xx_wait_result(handle, 2); /* 2 is only one try */
    if (error == Tfa98xx_Error_DSP_not_running) {
        *pRpcStatus = -1;
        return Tfa98xx_Error_Ok;
    }

        else if (error != Tfa98xx_Error_Ok)
                return error;

        error = tfa98xx_check_rpc_status(handle, pRpcStatus);

        return error;
}

int tfa98xx_Run_Is_Cold(Tfa98xx_handle_t handle)
{
        enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
        unsigned short status;

        if (!tfa98xx_handle_is_open(handle)) {
                PRINT("error: %s \n", error);
                if (error) return error;
        }

        error = tfa98xx_read_register16(handle, TFA98XX_STATUSREG, &status);

        return (status & TFA98XX_STATUSREG_ACS)!=0;
}

enum Tfa98xx_Error tfa98xx_set_tone_detection_off(Tfa98xx_handle_t handle) {
        enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

        switch (handlesLocal[handle].rev) {
                case 0x12: /* TFA9895 */
                        error = tfa98xx_dsp_write_mem(handle, 0x0f98, 0, Tfa98xx_DMEM_XMEM);
                        error = tfa98xx_dsp_write_mem(handle, 0x0f99, 0, Tfa98xx_DMEM_XMEM);
                        break;
                case 0x97: /* TFA9897 */
                        error = tfa98xx_dsp_write_mem(handle, 0x06b7, 0, Tfa98xx_DMEM_XMEM);
                        error = tfa98xx_dsp_write_mem(handle, 0x06b8, 0, Tfa98xx_DMEM_XMEM);
                        break;
                default:
                        PRINT("Error: Only available for tfa9895 and tfa9897! \n");
                        error = Tfa98xx_Error_Bad_Parameter;
                        break;
        }

        return error;
}

enum Tfa98xx_Error tfa98xx_check_device_features(Tfa98xx_handle_t handle) {
         enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
         int featureBits[2], SupportDrc = 0;


        return error;
}

#ifdef __KERNEL__
module_init();
module_exit();
#endif
