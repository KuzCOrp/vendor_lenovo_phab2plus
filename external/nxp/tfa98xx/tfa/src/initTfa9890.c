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
#include <linux/kernel.h>
#define _ASSERT(e) do { if ((e)) pr_err("PrintAssert:%s (%s:%d)\n",\
        __func__, __FILE__, __LINE__); } while (0)
#else
#include <assert.h>
#define _ASSERT(e)  assert(e)
#endif
#include "Tfa98xx.h"
#include "Tfa98xx_Registers.h"

enum Tfa98xx_Error tfa9890_specific(Tfa98xx_handle_t handle)
{
    enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
    unsigned short regRead = 0;

    if (!tfa98xx_handle_is_open(handle))
        return Tfa98xx_Error_NotOpen;

    /* all i2C registers are already set to default for N1C2 */

    /* some PLL registers must be set optimal for amplifier behaviour
     */
    error = tfa98xx_write_register16(handle, 0x40, 0x5a6b);
    error = tfa98xx_read_register16(handle, 0x59, &regRead);

    regRead |= 0x3;

    error = tfa98xx_write_register16(handle, 0x59, regRead);
    error = tfa98xx_write_register16(handle, 0x40, 0x0000);

    /* Added according to datasheet */
    error = tfa98xx_write_register16(handle, 0x47, 0x7BE1);

    return error;
}

/*
 * Tfa9890_DspSystemStable will compensate for the wrong behavior of CLKS
 * to determine if the DSP subsystem is ready for patch and config loading.
 *
 * A MTP calibration register is checked for non-zero.
 *
 * Note: This only works after i2c reset as this will clear the MTP contents.
 * When we are configured then the DSP communication will synchronize access.
 *
 */
enum Tfa98xx_Error tfa9890_dsp_system_stable(Tfa98xx_handle_t handle, int *ready)
{
    enum Tfa98xx_Error error;
    unsigned short status, mtp0;
    int tries;

    /* check the contents of the STATUS register */
    error = tfa98xx_read_register16(handle, TFA98XX_STATUSREG, &status);
    if (error)
        goto errorExit;

    /* if AMPS is set then we were already configured and running
     *   no need to check further
     */
    *ready = (status & TFA98XX_STATUSREG_AMPS_MSK) == (TFA98XX_STATUSREG_AMPS_MSK);
    if (*ready)        /* if  ready go back */
        return error;    /* will be Tfa98xx_Error_Ok */

    /* check AREFS and CLKS: not ready if either is clear */
    *ready =
        (status &
         (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK))
        == (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK);
    if (!*ready)        /* if not ready go back */
        return error;    /* will be Tfa98xx_Error_Ok */

    /* check MTPB
     *   mtpbusy will be active when the subsys copies MTP to I2C
     *   2 times retry avoids catching this short mtpbusy active period
     */
    for (tries = 2; tries > 0; tries--) {
        error =
            tfa98xx_read_register16(handle, TFA98XX_STATUSREG, &status);
        if (error)
            goto errorExit;
        /* check the contents of the STATUS register */
        *ready = (status & TFA98XX_STATUSREG_MTPB_MSK) == 0;
        if (*ready)    /* if ready go on */
            break;
    }
    if (tries == 0)        /* ready will be 0 if retries exausted */
        return Tfa98xx_Error_Ok;

    /* check the contents of  MTP register for non-zero,
     *  this indicates that the subsys is ready  */
    error = tfa98xx_read_register16(handle, 0x84, &mtp0);
    if (error)
        goto errorExit;

    *ready = (mtp0 != 0);    /* The MTP register written? */

    return error;

errorExit:
    *ready = 0;
    _ASSERT(error);        /* an error here can be considered
                   to be fatal */
    return error;
}

/*
 * The CurrentSense4 register is not in the datasheet, define local
 */
#define TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF (1<<2)
#define TFA98XX_CURRENTSENSE4 0x49
/*
 * Disable clock gating
 */
enum Tfa98xx_Error tfa9890_clockgating(Tfa98xx_handle_t handle, int on)
{
    enum Tfa98xx_Error error;
    unsigned short value49, value;

    /* for TFA9890 temporarily disable clock gating when dsp reset is used */
    error = tfa98xx_read_register16(handle, TFA98XX_CURRENTSENSE4, &value49);
    if (error) return error;

    if (Tfa98xx_Error_Ok == error) {
        if( on )  /* clock gating on - clear the bit */
                value = value49 & ~TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF;
        else  /* clock gating off - set the bit */
            value = value49 | TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF;

        error = tfa98xx_write_register16(handle, TFA98XX_CURRENTSENSE4, value);
    }

    return error;
}

/*
 * Tfa9890_DspReset will deal with clock gating control in order
 * to reset the DSP for warm state restart
 */
enum Tfa98xx_Error tfa9890_dsp_reset(Tfa98xx_handle_t handle, int state)
{
    enum Tfa98xx_Error error;
    unsigned short value49, value;

    /* for TFA9890 temporarily disable clock gating
       when dsp reset is used */
    error = tfa98xx_read_register16(
        handle, TFA98XX_CURRENTSENSE4, &value49);
    if (error)
        return error;

    if (Tfa98xx_Error_Ok == error) {
        /* clock gating off */
        value = value49 | TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF;
        error =
            tfa98xx_write_register16(handle, TFA98XX_CURRENTSENSE4,
                        value);
        if (error)
            return error;
    }

    error = tfa98xx_read_register16(handle, TFA98XX_CF_CONTROLS, &value);
    if (error)
        return error;

    /* set requested the DSP reset signal state */
    value = state ? (value | TFA98XX_CF_CONTROLS_RST_MSK) :
        (value & ~TFA98XX_CF_CONTROLS_RST_MSK);

    error = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS, value);

    /* clock gating restore */
    error = tfa98xx_write_register16(handle, 0x49, value49);
    return error;
}
