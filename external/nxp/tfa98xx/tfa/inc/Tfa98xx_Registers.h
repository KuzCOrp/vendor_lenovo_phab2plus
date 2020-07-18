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

#ifndef TFA98XX_REGISTERS_H
#define TFA98XX_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Tfa98xx_genregs.h"

/* some shorthands for readability */
#define TFA98XX_MTP                TFA98XX_KEY2_PROTECTED_SPKR_CAL_MTP
#define TFA98XX_MTP_CTRL    TFA98XX_KEY1_PROTECTED_MTP_CTRL_REG3

/* MTP bits */
/*  */
/* copy */
#define TFA98XX_MTP_CTRL_CIMTP_MSK TFA98XX_KEY1_PROTECTED_MTP_CTRL_REG3_CIMTP_MSK
/* one time calibration */
#define TFA98XX_MTP_MTPOTC_MSK  TFA98XX_KEY2_PROTECTED_SPKR_CAL_MTP_MTPOTC_MSK /*bit0*/
/* one time calibration done */
#define TFA98XX_MTP_MTPEX_MSK   TFA98XX_KEY2_PROTECTED_SPKR_CAL_MTP_MTPEX_MSK /*bit1*/
/* sample rates */
/* I2S_CONTROL bits */
#define TFA98XX_I2SCTRL_RATE_08000 (0<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_11025 (1<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_12000 (2<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_16000 (3<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_22050 (4<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_24000 (5<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_32000 (6<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_44100 (7<<TFA98XX_I2SREG_I2SSR_POS)
#define TFA98XX_I2SCTRL_RATE_48000 (8<<TFA98XX_I2SREG_I2SSR_POS)

/* TODO the POR values are for the tfa9890n1c2 */

#ifdef __cplusplus
}
#endif
#endif                /* TFA98XX_REGISTERS_H */
