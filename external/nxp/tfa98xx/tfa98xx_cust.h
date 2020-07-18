/*
 * tfa98xx_cust.h
 *
 *
 *  Created on: Oct 8, 2014
 *  Author: Customer, according to the Platform
 */

#ifndef _TFA98XX_CUST_H_
#define _TFA98XX_CUST_H_

/** the default target device */
#if defined(WIN32) || defined(x64)
#define TFA_I2CDEVICE        "scribo"            /* windows, pseudo device */
#else
//for non-mtk
#define TFA_I2CDEVICE        "/dev/smartpa_i2c"/*"/dev/ttyACM0"*/ /* linux default */
//for mtk
//#define TFA_I2CDEVICE        "/dev/i2c_smartpa"/*"/dev/ttyACM0"*/ /* linux default */
#define LOCATION_FILES       "/etc/"
#endif

#define TFA_I2CSLAVEBASE        (0x34)              // tfa device slave address of 1st (=left) device

#define CNT_FILENAME "mono_mtk.cnt"

#define TFA98XX_NOMINAL_IMPEDANCE         (8)
#define TFA98XX_NOMINAL_IMPEDANCE_MIN     ((float)TFA98XX_NOMINAL_IMPEDANCE*0.8)
#define TFA98XX_NOMINAL_IMPEDANCE_MAX     ((float)TFA98XX_NOMINAL_IMPEDANCE*1.2)

#endif /* _TFA98XX_CUST_H_ */
