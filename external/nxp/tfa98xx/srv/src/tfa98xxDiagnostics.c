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



#include <tfa98xxDiagnostics.h>

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dbgprint.h"

// TFA98XX API
#include "Tfa98API.h"
#include "Tfa98xx.h"
#include "Tfa98xx_Registers.h"

// srv
#include "tfaContainer.h"
#include "tfa98xxRuntime.h"
#include "tfaFieldnames.h"
#include "tfa98xxCalibration.h"

// for NXP_I2C_MAX_SIZE
#ifndef I2C_MAX_SIZE
#include "NXP_I2C.h"
#define I2C_MAX_SIZE NXP_I2C_MAX_SIZE
#endif

#define TFA_XMEM_PATCHVERSION 0x12bf            // TODO properly define
#define XMEM_MAX (0x0d7f)
#define TFA9897_XMEM_PATCHVERSION XMEM_MAX
#define XMEM_COUNT_BOOT 0xa1

#ifdef noDIAGTRACE
#include <stdio.h>
#define TRACEIN  if(tfa98xxDiag_trace) PRINT("Enter %s\n", __FUNCTION__);
#define TRACEOUT if(tfa98xxDiag_trace) PRINT("Leave %s\n", __FUNCTION__);
#else
#define TRACEIN
#define TRACEOUT
#endif
/* *INDENT-OFF* */
/*
 * test functions array
 */
tfa98xxDiagTest_t DiagTest[] = {
//           {tfa98xxDiagAll,"run all tests"},
           {tfa_diag_help,"list all tests descriptions"},
//#ifndef  WIN32
//       {tfa98xxDiagI2cSpeed,"measure i2c performance"},
//       {tfa98xxDiagI2cBurst,"test maximum transfer size"},
//#endif
//       {tfa_diag_register_read, "read test of DEVID register"},
//       {tfa_diag_register_write_read, "write/read test of register 0x71"},
//       {tfa98xxDiagStatusCold, "check status register flags and assume coldstart (or fail)"},
//       {tfa98xxDiagRegisterDefaults, "verify default state of relevant registers"},
//       {tfa98xxDiagClock, "enable clocks in bypass and verify status"},
//       {tfa98xxDiagDsp, "start DSP and verify (by reading ROM tag and check status)"},
//       {tfa98xxDiagLoadConfig, "load configuration settings and verify readback"},
//       {tfa98xxDiagLoadPreset, "load preset values and verify readback"},
//       {tfa98xxDiagLoadSpeaker, "load speaker parameters and verify readback"},
//       {tfa98xxDiagBattery, "check battery level"},
//       {tfa98xxDiagSpeakerPresence, "verify speaker presence by checking the resistance"},
//       {tfa98xxDiagI2sInput, "assume I2S input and verify signal activity"},
//       {tfa98xxDiagLoadPresetsDouble, "load presets L+R 2x single API"},
//       {tfa98xxDiagLoadPresetsMultiple, "load presets L+R multiple API"}//,
//TBD?       {tfa98xxDiagCalibration, "calibrate speaker and verify speaker presence and DCDC"},
//       {tfa98xxDiagI2sOutput, "for testing the I2S output an external receiver should acknowlege data presence (TBD)"}
          {tfa_diag_register_read           ,"read test of DEVID register", tfa_diag_i2c},
          {tfa_diag_register_write_read      ,"write/read test of  register 0x71", tfa_diag_i2c},
          {tfa_diag_clock_enable          ,"check PLL status after poweron", tfa_diag_i2c},
          {tfa_diag_xmem_access          ,"write/read test of  all xmem locations", tfa_diag_i2c},
          {tfa_diag_ACS_via_iomem          ,"set ACS bit via iomem, read via status reg", tfa_diag_i2c},
          {tfa_diag_xmem_burst_read       ,"write full xmem, read with i2c burst", tfa_diag_i2c},
          {tfa_diag_xmem_burst_write      ,"write/read full xmem in i2c burst", tfa_diag_i2c},
          {tfa_diag_dsp_reset          ,"verify dsp response to reset toggle", tfa_diag_i2c},
          {tfa_diag_battery_level          ,"check battery voltage level", tfa_diag_i2c},
          {tfa_diag_patch_load          ,"load a patch and verify version number", tfa_diag_dsp },
          {tfa_diag_read_version_tag      ,"read back the ROM version tag", tfa_diag_dsp },
        {tfa_diag_check_device_features   ,"check that the features match the device", tfa_diag_dsp},
          {tfa_diag_start_speakerboost      ,"load the configuration to bring up SpeakerBoost", tfa_diag_dsp },
          {tfa_diag_verify_parameters      ,"read back to verify that all parameters are loaded", tfa_diag_sb },
          {tfa_diag_calibrate_always      ,"run a calibration and check for success", tfa_diag_sb },
          {tfa_diag_speaker_impedance      ,"verify that the speaker impedance is within range", tfa_diag_sb },
          {tfa_diag_irq_warm                ,"test interrupt ACS & ACK bits", tfa_diag_func },
         {tfa_diag_irq_cold                ,"low level test interrupt bits", tfa_diag_func }
//          {tfa_diag_record_livedata       ,"verify all the live data can be read back properly", tfa_diag_sb },
//          {tfa_diag_speaker_temperature      ,"check the speakertemperature", tfa_diag_sb},
//          {tfa_diag_audio_continuous_play      ,"playback audio and check speakertemp", tfa_diag_sb},
//          {tfa_diag_audio_interval_play      ,"playback audio with clock stops,check speakertemp", tfa_diag_sb},
//          {tfa_diag_audio_record          ,"check the recording functionality", tfa_diag_sb},
//          {tfa_diag_power_1v8          ,"verify that the device can be powered off", tfa_diag_pins},
//          {tfa_diag_reset              ,"test the response to a reset toggle", tfa_diag_pins},
//          {tfa_diag_interrupt          ,"read back the int pin status (use ACS toggle)", tfa_diag_pins},
//          {tfa_diag_LEDs              ,"run a blinking pattern over the LEDs", tfa_diag_pins},
//          {tfa_diag_board_ADCs          ,"read back and check board voltages", tfa_diag_pins}
};
/* *INDENT-ON* */

/* *INDENT-OFF* */
regdef_t regdefs[] = {
        { 0x00, 0x081d, 0xfeff, "statusreg"}, //ignore MTP busy bit
        { 0x01, 0x0, 0x0, "batteryvoltage"},
        { 0x02, 0x0, 0x0, "temperature"},
        { 0x03, 0x0012, 0xffff, "revisionnumber"},
        { 0x04, 0x888b, 0xffff, "i2sreg"},
        { 0x05, 0x13aa, 0xffff, "bat_prot"},
        { 0x06, 0x001f, 0xffff, "audio_ctr"},
        { 0x07, 0x0fe6, 0xffff, "dcdcboost"},
        { 0x08, 0x0800, 0x3fff, "spkr_calibration"}, //NOTE: this is a software fix to 0xcoo
        { 0x09, 0x041d, 0xffff, "sys_ctrl"},
        { 0x0a, 0x3ec3, 0x7fff, "i2s_sel_reg"},
        { 0x40, 0x0, 0x00ff, "hide_unhide_key"},
        { 0x41, 0x0, 0x0, "pwm_control"},
        { 0x46, 0x0, 0x0, "currentsense1"},
        { 0x47, 0x0, 0x0, "currentsense2"},
        { 0x48, 0x0, 0x0, "currentsense3"},
        { 0x49, 0x0, 0x0, "currentsense4"},
        { 0x4c, 0x0, 0xffff, "abisttest"},
        { 0x62, 0x0, 0, "mtp_copy"},
        { 0x70, 0x0, 0xffff, "cf_controls"},
        { 0x71, 0x0, 0, "cf_mad"},
        { 0x72, 0x0, 0, "cf_mem"},
        { 0x73, 0x00ff, 0xffff, "cf_status"},
        { 0x80, 0x0, 0, "mtp"},
        { 0x83, 0x0, 0, "mtp_re0"},
        { 0xff, 0,0, NULL}
};
regdef_t tdm_regdefs[] = {
        { 0x10, 0x0220, 0, "tdm_config_reg0"},
        { 0x11, 0xc1f1, 0, "tdm_config_reg1"},
        { 0x12, 0x0020, 0, "tdm_config_reg2"},
        { 0x13, 0x0000, 0, "tdm_config_reg3"},
        { 0x14, 0x2000, 0, "tdm_config_reg4"},
        { 0x15, 0x0000, 0, "tdm_status_reg"},
        { 0xff, 0,0, NULL}
};
regdef_t int_regdefs[] = {
        { TFA98XX_INTERRUPT_OUT_REG1, 0, 0, "int_reg_out1"},
        { 0x21, 0, 0, "int_reg_out2"},
        { TFA98XX_INTERRUPT_OUT_REG3, 0, 0, "int_reg_out3"},
        { TFA98XX_INTERRUPT_IN_REG1, 0, 0, "int_reg_in1"},
        { 0x24, 0, 0, "int_reg_in2"},
        { TFA98XX_INTERRUPT_IN_REG3, 0, 0, "int_reg_in3"},
        { TFA98XX_INTERRUPT_ENABLE_REG1,0x0001 , 0, "int_ena1"},
        { 0x27, 0, 0, "int_ena2"},
        { TFA98XX_INTERRUPT_ENABLE_REG3, 0, 0, "int_ena3"},
        { TFA98XX_STATUS_POLARITY_REG1, 0xf5e2, 0, "int_pol1"},
        { 0x2a, 0, 0, "int_pol2"},
        { TFA98XX_STATUS_POLARITY_REG3, 0x0003, 0, "int_pol3"},
        { 0xff, 0,0, NULL}
};
/* *INDENT-ON* */
#define MAXREGS ((sizeof(regdefs)/sizeof(regdef_t))-1)
#define MAXTDMREGS ((sizeof(tdm_regdefs)/sizeof(regdef_t))-1)
#define MAXINTREGS ((sizeof(int_regdefs)/sizeof(regdef_t))-1)

// status register errors to check for not 1
#define TFA98XX_STATUSREG_ERRORS_SET_MSK (  \
        TFA98XX_STATUSREG_OCDS  )
// status register errors to check for not 0
#define TFA98XX_STATUSREG_ERRORS_CLR_MSK (  TFA98XX_STATUSREG_VDDS  |\
        TFA98XX_STATUSREG_UVDS  |  \
        TFA98XX_STATUSREG_OVDS  |  \
        TFA98XX_STATUSREG_OTDS    )
//      TFA98XX_STATUSREG_DCCS   ) TODO check bit

// register used in rw diag
#define RWTEST_REG TFA98XX_CF_MAD

/*
 * check status register flags and do not check coldstart
 */
static int tfa98xxDiagStatus(Tfa98xx_handle_t handle, unsigned short mask,
                      unsigned short clearmask);

// globals
int tfa98xxDiag_trace = 1;
int tfa98xxDiag_verbose = 1;
static Tfa98xx_handle_t gHandle=-1; // for nesting
static int lastTest = -1;
static int lastError = -1;
static char lastErrorString[256] = "";

static Tfa98xx_Error_t lastApiError;

#if !(defined(WIN32) || defined(_X64))
/************************
 * time measurement
 */
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
typedef struct tag_time_measure
{
  struct timeval startTimeVal;
  struct timeval stopTimeVal;

  struct rusage startTimeUsage;
  struct rusage stopTimeUsage;
} time_measure;
time_measure * startTimeMeasuring();
void stopTimeMeasuring(time_measure * tu);
void printMeasuredTime(time_measure * tu);

int tfa98xxDiagI2cSpeed(int slave); //TODO tfa98xxDiagI2cSpeed
int tfa98xxDiagI2cBurst(int slave); //TODO tfa98xxDiagI2cBurst
#endif
/*
 * TODO make this permanent?
 */
int tfa98xxDiagLoadPresetsDouble(int slave);
int tfa98xxDiagLoadPresetsMultiple(int slave);


#define DIAG_CFSTABLE_TRIES 10
static int tfa_diag_pwron_wait(Tfa98xx_handle_t handle)
{
    int tries;
    int status = 0;
    enum Tfa98xx_Error error;

    for (tries = 1; tries < DIAG_CFSTABLE_TRIES; tries++)
    {
        error = Tfa98xx_DspSystemStable(handle, &status);
        assert (Tfa98xx_Error_Ok == error);
        if (status)
            return 0;
    }
    return DIAG_CFSTABLE_TRIES;
}

int tfa_diag_need_cnt(int nr) {
    return (DiagTest[nr].group == tfa_diag_all ||
            DiagTest[nr].group == tfa_diag_dsp ||
            DiagTest[nr].group == tfa_diag_sb );

}
static const char *diag_groupname[]= {
        "", "i2c", "dsp", "sb", "pins","func"
};
/*
 *  the functions will return 0 if passed
 *  else fail and the return value may contain extra info
 */
 /*
  * run a testnr
  */
int tfa98xxDiag(int slave, int testnr)
{
    int maxtest = sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t);
    int result;

    if (testnr > maxtest) {
            PRINT_ERROR("%s: test number:%d too high, max %d \n",
                    __FUNCTION__, testnr, maxtest);
            return -1;
    }

    if (testnr>0) {
            PRINT("test %d [%s]: %s\n", testnr,  diag_groupname[DiagTest[testnr].group], DiagTest[testnr].description);
    }

    if ( tfa_diag_need_cnt(testnr) ) {
        if (tfa98xx_get_cnt() == NULL ) {
            PRINT_ERROR("No container loaded! (test %d : %s)\n", testnr, DiagTest[testnr].description);
            return -1;
        }

    }
    result = DiagTest[testnr].function(slave);

    if (testnr != 0)        // don't if 0 that's for all tests
            lastTest = testnr;      // remember the last test for reporting

    if (result==1)
            sprintf(lastErrorString, "No i2c slave 0x%0x", slave);
    else if (result==2)
        sprintf(lastErrorString, "No clock");
    else if (result==-1)
        sprintf(lastErrorString, "Something wrong with container and/or configuration");
    else if (result==1000)
        sprintf(lastErrorString, "------------test not yet implemented--------------");

    return result;
}

/*
 * list all tests descriptions
 */
int tfa_diag_help(int slave) {
    int i,  maxtest =
        sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t) - 1;


    // run what is in this group
    for (i = 1; i <= maxtest; i++) {
        PRINT("\tdiag=%d  : %s , [%s]\n ", i , DiagTest[i].description,diag_groupname[ DiagTest[i].group]);
    }

    return 0;
}
// run all
int tfa98xxDiagAll(int slave)
{
        enum tfa_diag_group groups[]={tfa_diag_i2c, tfa_diag_dsp, tfa_diag_sb};
        int i, result = 0, maxtest =
            sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t) - 1;

        TRACEIN;
        for (i = 0; i < 3; i++) {
//            result = tfa98xxDiag(slave, i);
            result = tfa98xxDiagGroup(slave, groups[i]);
                if (result != 0)
                        break;
        }
        TRACEOUT;
        return result;
}
/* translate group name argument to enum */
enum tfa_diag_group tfa98xxDiagGroupname(char *arg) {

    if (arg==NULL)
        return tfa_diag_all;         /**< all tests, container needed, destructive */

    if ( strcmp(arg, "i2c")==0 )
        return tfa_diag_i2c;         /**< I2C register writes, no container needed, destructive */
    else if (strcmp(arg, "dsp")==0 )
        return tfa_diag_dsp;     /**< dsp interaction, container needed, destructive */
    else if (strcmp(arg, "sb")==0 )
        return     tfa_diag_sb;         /**< SpeakerBoost interaction, container needed, not destructive */
    else if (strcmp(arg, "pins")==0 )
        return tfa_diag_pins;      /**< Pin tests, no container needed, destructive */

    PRINT_ERROR("Unknown groupname argument: %s\n", arg);

    return tfa_diag_all;         /**< all tests, container needed, destructive */

}

// run group
int tfa98xxDiagGroup(int slave,  enum tfa_diag_group group) {
    int i, result = 0, maxtest =
        sizeof(DiagTest) / sizeof(tfa98xxDiagTest_t) - 1;

    if (group == tfa_diag_all)         /**< all tests, container needed, destructive */
        return tfa98xxDiagAll(slave);

    // run what is in this group
    for (i = 1; i <= maxtest; i++) {
        if ( DiagTest[i].group == group) {
            result = tfa98xxDiag(slave, i);
            if (result != 0)
                break;
        }
    }

    return result;
}
/*
 * print supported device features
 */
void tfa98xxDiagPrintFeatures(int devidx, char *strings) {
    Tfa98xx_Error_t error;
    enum Tfa98xx_DAI daimap;
    int status;
    char str[NXPTFA_MAXLINE];
    int length=0;

    sprintf(str, "device features:");
    length = (int)(strlen(str));
    strcpy(strings, str);
    strings += length;

    error = tfa98xx_supported_dai(devidx, &daimap);
    assert (error==Tfa98xx_Error_Ok);

    if ( daimap & Tfa98xx_DAI_I2S )    {
        sprintf(str, " I2S");
        length = (int)(strlen(str));
        strcpy(strings, str);
        strings += length;
    }
    if ( daimap & Tfa98xx_DAI_TDM )    {
        sprintf(str, " TDM");
        length = (int)(strlen(str));
        strcpy(strings, str);
        strings += length;
    }
    if ( daimap & Tfa98xx_DAI_PDM ) {
        sprintf(str, " PDM");
        length = (int)(strlen(str));
        strcpy(strings, str);
        strings += length;
    }

    error = tfa98xx_dsp_system_stable(devidx, &status);
    assert (error==Tfa98xx_Error_Ok);
    if (status) {
            /* only possible if DSP subsys  running */
        error = tfa98xx_dsp_support_tcoef(devidx, &status);
        assert (error==Tfa98xx_Error_Ok);
        sprintf(str, ", %s calibration", status ? "normal":"2-step");
        length = (int)(strlen(str));
        strcpy(strings, str);
        strings += length;
        }
    sprintf(str, "\n");
    length = (int)(strlen(str));
    strcpy(strings, str);
    strings += length;
    *strings = '\0';
}

/*
 * return latest testnr
 */
int tfa98xxDiagGetLatest(void)
{
        return lastTest;
}

/*
 * return last error code
 */
int tfa98xxDiagGetLastError(void)
{
        return lastError;
}

/*
 * return last error string
 */
char *tfa98xxDiagGetLastErrorString(void)
{
        return lastErrorString;
}

/*
 * return testname string
 *  if testnr is too big an empty string is returned
 *
 */
const char *tfa98xxDiagGetTestNameString(int testnr)
{
        if (testnr < MAXREGS)
                return DiagTest[testnr].description;
        else
                return "";
}
#if !(defined(WIN32) || defined(_X64))
/*
 *
 */
int tfa98xxDiagI2cSpeed(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        time_measure * tu;
        unsigned short testreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);

        if (lastApiError != Tfa98xx_Error_Ok) {
                sprintf(lastErrorString, "can't find i2c slave 0x%0x", slave);
        }

        // read
        tu = startTimeMeasuring();
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        stopTimeMeasuring(tu);
        assert(lastApiError == Tfa98xx_Error_Ok);
        printMeasuredTime(tu);
        //write
        tu = startTimeMeasuring();
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, 0x1234);    //
        stopTimeMeasuring(tu);
        assert(lastApiError == Tfa98xx_Error_Ok);
        printMeasuredTime(tu);

        lastError = result;
        TRACEOUT;

        free(tu);
        return result;
}
/*
 *
 */
int tfa98xxDiagI2cBurst(int slave)
{
        Tfa98xx_handle_t handle;
        int i,result = 0;         // 1 is failure
        time_measure * tu;
        int tsize=I2C_MAX_SIZE/3;
        int testbuf[I2C_MAX_SIZE/3],
                       checkbuf[I2C_MAX_SIZE/3];

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);

        if (lastApiError != Tfa98xx_Error_Ok) {
                sprintf(lastErrorString, "can't find i2c slave 0x%0x", slave);
        }

        // create test patterns
        for(i=0;i< tsize;i++){
            testbuf[i] = i;
        }
        // limit max a to 80 for now
        if (tsize>80) tsize=80;
        // write
        tu = startTimeMeasuring();

        lastApiError = Tfa98xx_DspWriteMemory(handle,
                Tfa98xx_DMEM_XMEM, 0 /* offset */, tsize, testbuf);
        stopTimeMeasuring(tu);
        //ignore assert(lastApiError == Tfa98xx_Error_Ok);
        printMeasuredTime(tu);

        tu = startTimeMeasuring();
        lastApiError = Tfa98xx_DspReadMemory(handle,
                Tfa98xx_DMEM_XMEM, 0 /* offset */, tsize /* number words to read */, checkbuf);
        stopTimeMeasuring(tu);
        assert(lastApiError == Tfa98xx_Error_Ok);
        printMeasuredTime(tu);

        // note readback payload data start @word 3
        if (0 != memcmp(testbuf, &checkbuf[3], tsize) )
            PRINT("data compare of I2C burst of %d bytes failed\n", I2C_MAX_SIZE);

        lastError = result;
        TRACEOUT;

        free(tu);
        return result;
}
#endif
/*
 * direct read of register
 */
static int tfa_read(int slave,
               unsigned char subaddress, unsigned short *pValue)
{
    NXP_I2C_Error_t i2c_error;
    const int bytes2write = 1;    /* subaddress size */
    /* 2 bytes that will contain the data of the register */
    const int bytes2read = 2;
    unsigned char write_data[1];
    unsigned char read_buffer[2];

    write_data[0] = subaddress;
    read_buffer[0] = read_buffer[1] = 0;
    i2c_error =
        NXP_I2C_WriteRead( slave<<1, bytes2write, write_data, bytes2read, read_buffer);

    if ( i2c_error != NXP_I2C_Ok) {
        return 1;
    } else {
        *pValue = (read_buffer[0] << 8) + read_buffer[1];
        return 0;
    }
}
/*
 *read test of DEVID register
 *
 */
int tfa_diag_register_read(int slave)
{
        //Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        unsigned short value;

        TRACEIN;

        if (tfa_read(slave, 0x03, &value) ==0) {
            result = Tfa98xx_Error_Ok;
        }
        else {
                result = 1;     // non-0 if fail
                sprintf(lastErrorString, "can't find i2c slave 0x%0x", slave);
        }

        lastError = result;
        TRACEOUT;

        return result;
}


/**
 * write/read test of  register 0x71
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_register_write_read(int slave)
{
        Tfa98xx_handle_t handle;
        //int testregoffset;
        int result = 0;      // 1 is failure
        unsigned short testreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 1;

        // write 0x1234
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, 0x1234);     //
        assert(lastApiError == Tfa98xx_Error_Ok);
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        assert(lastApiError == Tfa98xx_Error_Ok);

        // restore default, else pwrdefault may fail
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, regdefs[RWTEST_REG].pwronDefault);   //
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (0x1234 != testreg) {
                sprintf(lastErrorString,
                        "read back value mismatch: (testreg=0x%02x), exp:0x%04x rcv:0x%04x\n",
                        RWTEST_REG, 0x1234, testreg);
                result = 1;
        }

// stop:
        Tfa98xx_Close(handle);

        lastError = result;
        TRACEOUT;
        return result;
}

int tfa_diag_check_device_features(int slave)
{
        Tfa98xx_handle_t handle;
    int result = 0;

    TRACEIN;
    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }

        lastApiError = Tfa98xx_CheckDeviceFeatures(handle);
        assert(lastApiError == Tfa98xx_Error_Ok);
        if (lastApiError != Tfa98xx_Error_Ok) {
        result = -1;
        goto stop;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_clock_enable(int slave) {
    Tfa98xx_handle_t handle;
    int ready, result = 0, loop=50;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test  :
     *   - powerdown
     *  - powerup
     *  - wait for system stable */
    lastApiError = Tfa98xx_Init(handle);
    assert(lastApiError == Tfa98xx_Error_Ok);
    lastApiError = Tfa98xx_Powerdown(handle, 0);
    assert(lastApiError == Tfa98xx_Error_Ok);

    do {
        lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
        assert(lastApiError == Tfa98xx_Error_Ok);
        if (ready)
            break;
    } while (loop--);

    /* report result */
    if (!ready) {
        sprintf(lastErrorString, "power-on timed out\n");
        result = 2;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

//     * write/read test of  all xmem locations
int tfa_diag_xmem_access(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int ready, i,data;
    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
//     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
//     * - put DSP in reset
    lastApiError = Tfa98xx_DspReset(handle, 1);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - write count into xmem locations
    for(i=0;i <= XMEM_MAX;i+=100) {
        lastApiError = tfa98xx_dsp_write_mem(handle, i, i, Tfa98xx_DMEM_XMEM);
        assert(lastApiError == Tfa98xx_Error_Ok);
    }
//     * - verify this count by reading back xmem
    for(i=0;i <= XMEM_MAX;i+=100) {
        lastApiError = tfa98xx_dsp_read_mem(handle, i,1,&data);
        assert(lastApiError == Tfa98xx_Error_Ok);
        if (data != i) {
            result = 3;
            break;
        }
    }

    /* report result */
    if (result == 3) {
        sprintf(lastErrorString, "xmem expected read value mismatch; xmem[%d] exp:%d , actual %d",i, i, data);
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

//     * set ACS bit via iomem, read via status reg
int tfa_diag_ACS_via_iomem(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int ready; // i,data;
    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

//     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
//     * - clear ACS via CF_CONTROLS
//     * - check ACS is clear in status reg
    if (Tfa98xx_Error_Ok == tfaRunColdboot(handle, 0) ){
        //     * - set ACS via CF_CONTROLS
        //     * - check ACS is set in status reg
        if (Tfa98xx_Error_Ok != tfaRunColdboot(handle, 1) ){
            result = 3;
        }
    } else
        result = 3;


    /* report result */
    if (result==3) {
        sprintf(lastErrorString, "no control over ACS via iomem\n");

    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}
//     * interrupt tests


/*
 * trusted functions:
 *  target assumed open
 *  no error checking for register io
 */
enum Tfa98xx_Error tfa98xx_irq_clear(Tfa98xx_handle_t handle, enum tfa_irq bit) {
    int reg;
    /* make bitfield enum */
    if ( bit == tfa_irq_all) {
        /* operate on all bits */
        for(reg=TFA98XX_INTERRUPT_IN_REG1; reg<TFA98XX_INTERRUPT_IN_REG1+3; reg++)
            tfa98xx_write_register16(handle, reg, 0xffff); /* all bits */
    } else if (bit < tfa_irq_max) {
            reg = TFA98XX_INTERRUPT_IN_REG1 + (bit>>4);
            tfa98xx_write_register16(handle, reg, 1<<(bit & 0x0f)); /* only this bit */
    } else
        return Tfa98xx_Error_Bad_Parameter;

    return Tfa98xx_Error_Ok;
}
enum Tfa98xx_Error tfa98xx_irq_ena(Tfa98xx_handle_t handle, enum tfa_irq bit, int state) {
    uint16_t value;
    int reg, mask;
    /* */
    if ( bit == tfa_irq_all) {
        /* operate on all bits */
        for(reg=TFA98XX_INTERRUPT_ENABLE_REG1; reg<TFA98XX_INTERRUPT_ENABLE_REG1+3; reg++)
            tfa98xx_write_register16(handle, reg, state ? 0xffff : 0); /* all bits */
    } else if (bit < tfa_irq_max) {
         /* only this bit */
            reg = TFA98XX_INTERRUPT_ENABLE_REG1 + (bit>>4);
            mask = 1<<(bit & 0x0f);
            tfa98xx_read_register16(handle, reg, &value);
            if (state) //set
                value |=  mask;
            else         // clear
                value &= ~mask;
            tfa98xx_write_register16(handle, reg, value);
    } else
        return Tfa98xx_Error_Bad_Parameter;

    return Tfa98xx_Error_Ok;
}
/*
 * return state of irq or -1 if illegal bit
 */
int tfa98xx_irq_get(Tfa98xx_handle_t handle, enum tfa_irq bit){
    uint16_t value;
    int reg, mask;

    if (bit < tfa_irq_max) {
        /* only this bit */
        reg = TFA98XX_INTERRUPT_OUT_REG1 + (bit>>4);
        mask = 1<<(bit & 0x0f);
        tfa98xx_read_register16(handle, reg, &value);
    } else
        return -1;

    return (value & mask) !=0 ;
}
/*
 * return state of irq or -1 if illegal bit
 */
int tfa98xx_irq_pol(Tfa98xx_handle_t handle, enum tfa_irq bit, int state){
    uint16_t value;
    int reg, mask;

    if (bit < tfa_irq_max) {
        /* only this bit */
        reg = TFA98XX_STATUS_POLARITY_REG1 + (bit>>4);
        mask = 1<<(bit & 0x0f);
        tfa98xx_read_register16(handle, reg, &value);
        if (state) //set
            value |=  mask;
        else         // clear
            value &= ~mask;
        tfa98xx_write_register16(handle, reg, value);
    } else
        return -1;

    return 0;
}

Tfa98xx_Error_t tfaRunBitfieldSet(Tfa98xx_handle_t handle,enum nxpTfaBfEnumList name, uint16_t value ) {
     nxpTfaBitfield_t bf;
     bf.field = name;
     bf.value = value;
     return tfaRunWriteBitfield(handle, bf);
}

//
int tfa_diag_irq_cold(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int ready, i; //data;
    //uint16_t value;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    // power on
    lastApiError = tfaRunStartup(handle);
    assert(lastApiError == Tfa98xx_Error_Ok);
    // set to bypass, is enough for 97 N1A to enable irq
//    tfaRunBitfieldSet(handle, bfCFE, 0);
//     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }

    //     * - put DSP in reset
        lastApiError = Tfa98xx_DspReset(handle, 1);
        assert(lastApiError == Tfa98xx_Error_Ok);

    /* force interrupt */
        /* disable all irq's */
        tfa98xx_irq_ena(handle, tfa_irq_all, 0);
        /* invert the ones with active input after init*/
        tfa98xx_irq_pol(handle, 1, 0);
        tfa98xx_irq_pol(handle, 6, 0);
        tfa98xx_irq_pol(handle, 9, 1);
        tfa98xx_irq_pol(handle, 11, 1); // TODO is ACS supposed to be active low?
        tfa98xx_irq_pol(handle, 12, 0);
        tfa98xx_irq_pol(handle, 14, 0);
        tfa98xx_irq_pol(handle, 15, 0);
        tfa98xx_irq_pol(handle, 19, 0);
        tfa98xx_irq_pol(handle, 26, 0);
        tfa98xx_irq_pol(handle, 31, 0);
        tfa98xx_irq_pol(handle, tfa_irq_cfma_err, 0);
        /* clear all */
        tfa98xx_irq_clear(handle, tfa_irq_all);
        /* check for clear */
        for(i=0;i<tfa_irq_max;i++) {
            result = tfa98xx_irq_get(handle, i);
            assert (result!=-1);
            if(result) {
                PRINT_ERROR("%s: interrupt bit %d did not clear\n", __FUNCTION__, i);
            }
        }
        /* report result not cleared */
        if (result) {
            result = 3;
            sprintf(lastErrorString, "interrupt bit(s) did not clear\n");

        }
        /* bit 0 tfa_irq_vdds not clear is fatal */
        if(  tfa98xx_irq_get(handle, tfa_irq_vdds) ) {
            result = 4;
            sprintf(lastErrorString, "VDDS interrupt did not clear\n");
            goto stop;
        }
        /* bit 0 tfa_irq_vdds not clear is fatal */
        if(  tfa98xx_irq_get(handle, tfa_irq_cfma_ack) ) {
            result = 5;
            sprintf(lastErrorString, "ACK interrupt did not clear\n");
            goto stop;
        }
        /* invert polarity and expect int */
        tfa98xx_irq_pol(handle, tfa_irq_vdds, 1);
        if(  !tfa98xx_irq_get(handle, tfa_irq_vdds) ) {
            result = 6;
            sprintf(lastErrorString, "VDDS interrupt did not set\n");
            goto stop;
        }

    stop:
    lastApiError = Tfa98xx_DspReset(handle, 0) /* release DSP */;
    assert(lastApiError == Tfa98xx_Error_Ok);

    Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}
//
Tfa98xx_Error_t tfa_diag_make_event(Tfa98xx_handle_t handle, enum tfa_fw_event event) {
    Tfa98xx_Error_t error =Tfa98xx_Error_Ok;
    int features[2];

    switch (event) {
    case tfa_fw_i2c_cmd_ack:
        error = tfa98xx_dsp_get_sw_feature_bits(handle, features);
        break;
    case tfa_fw_reset_start:
        Tfa98xx_DspReset(handle, 1) /* set rst DSP */;
        error = Tfa98xx_DspReset(handle, 0) /* release DSP */;
        break;
    case tfa_fw_short_on_mips:
        /* force it if needed, skip for now */
        break;
    case tfa_fw_soft_mute_ready:
        //error = tfa98xx_set_volume_level(handle, 0xff);
        error = tfa98xx_set_mute(handle, Tfa98xx_Mute_Digital);
        break;
    default:
        PRINT_ERROR("%s is called with unknown firmware event:%d\n", __FUNCTION__, event);
        assert(0);
        break;
    }

    return error;
}
int tfa_diag_cfma_ack (Tfa98xx_handle_t handle, enum tfa_fw_event fw_event) {
int result;

    tfa98xx_irq_clear(handle, tfa_irq_cfma_ack);
    if(  tfa98xx_irq_get(handle, tfa_irq_cfma_ack) ) {
        result = 3;
        sprintf(lastErrorString, "ACK interrupt did not clear\n");
        return result;
    }
    /* generate the firmware event*/
    tfa_diag_make_event(handle, fw_event);

    if(  !tfa98xx_irq_get(handle, tfa_irq_cfma_ack) ) {
        result = 4;
        sprintf(lastErrorString, "ACK interrupt did not set\n");
        return result;
    }
    return 0;
}
int tfa_diag_irq_warm(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int ready; // i,data;
    //uint16_t value;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
        lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
        assert(lastApiError == Tfa98xx_Error_Ok);
        if (!ready) {
            result = 2;
            goto stop;
        }
    //     * - put DSP in reset
        lastApiError = Tfa98xx_DspReset(handle, 1);
        assert(lastApiError == Tfa98xx_Error_Ok);
        /* assume CF up and running/calibrated so bypass is not enough */

        /* ACS bit test */
        if (tfaRunIsCold(handle) ) {
            result = 3;
            sprintf(lastErrorString, "ACS is not clear (coldboot state)\n");
            goto stop;
        }
        tfa98xx_irq_pol(handle, tfa_irq_acs, 1);
        tfa98xx_irq_clear(handle, tfa_irq_acs);
        if(  tfa98xx_irq_get(handle, tfa_irq_acs) ) {
            result = 3;
            sprintf(lastErrorString, "ACS interrupt did not clear\n");
            goto stop;
        }
        tfaRunColdboot(handle, 1);
        if(  !tfa98xx_irq_get(handle, tfa_irq_acs) ) {
            result = 4;
            sprintf(lastErrorString, "ACS interrupt did not set\n");
            goto stop;
        }
        tfaRunColdboot(handle, 0);

        /* clear all ACKs by toggling all REQs */
        tfaRunBitfieldSet(handle, bfREQ, 0xff);
        tfaRunBitfieldSet(handle, bfREQ, 0);

        /* check msg ACK */
        if ( tfa_diag_cfma_ack (handle ,tfa_fw_i2c_cmd_ack)) {
            result = 5;
            sprintf(lastErrorString, "interrupt for tfa_fw_i2c_cmd_ack ACK failed\n");
            goto stop;
        }

        if ( tfa_diag_cfma_ack (handle ,tfa_fw_reset_start)) {
            result = 6;
            sprintf(lastErrorString, "interrupt for tfa_fw_reset_start ACK failed\n");
            goto stop;
        }

#if 0
        if ( tfa_diag_cfma_ack (handle ,tfa_fw_soft_mute_ready)) {
            result = 7;
            sprintf(lastErrorString, "interrupt for tfa_fw_soft_mute_ready ACK failed\n");
            goto stop;
        }
#else
        PRINT("%s tfa_fw_soft_mute_ready: irq test  disabled\n", __FUNCTION__);
#endif
            /* TODO tfa_fw_short_on_mips if we can generate it */


    stop:

    lastApiError = Tfa98xx_DspReset(handle, 0) /* release DSP */;
    assert(lastApiError == Tfa98xx_Error_Ok);

    Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

#define XMEM_STRIDE 100 // shorten test time
//     * write xmem, read with i2c burst
int tfa_diag_xmem_burst_read(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int i, ready, data[ XMEM_MAX + 4]; // test buffer
    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
    //     * - put DSP in reset
    lastApiError = Tfa98xx_DspReset(handle, 1);
    assert(lastApiError == Tfa98xx_Error_Ok);
    //     * - write testpattern
    for (i = 0; i < XMEM_MAX; i += XMEM_STRIDE) {
        lastApiError = tfa98xx_dsp_write_mem(handle, i, i, Tfa98xx_DMEM_XMEM);
        assert(lastApiError == Tfa98xx_Error_Ok);
    }
    //     * - burst read testpattern
    lastApiError = tfa98xx_dsp_read_mem(handle, 0, XMEM_MAX, data);
    assert(lastApiError == Tfa98xx_Error_Ok);
    //     * - verify data
    for (i = 0; i < XMEM_MAX; i += XMEM_STRIDE) {
        if (i != data[i]) {
            result = 3;
            break;
        }
    }
    /* report result */
    if (result == 3) {
        sprintf(lastErrorString, "xmem expected read value mismatch\n");
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

// write/read full xmem in i2c burst
int tfa_diag_xmem_burst_write(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int i, ready;
    int worddata[XMEM_MAX + 4]; // test buffers
    unsigned char bytedata[sizeof(worddata)*3];
    //unsigned short regval;
    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
    //     * - put DSP in reset
    lastApiError = Tfa98xx_DspReset(handle, 1);
    assert(lastApiError == Tfa98xx_Error_Ok);
    //     * - create & burst write testpattern
    for (i = 0; i < XMEM_MAX; i ++) {
        worddata[i]=i;
    }
    tfa98xx_convert_data2bytes(XMEM_MAX, worddata, bytedata);
//    lastApiError = Tfa98xx_DspWriteMemory(    handle, Tfa98xx_DMEM_XMEM, 0, XMEM_MAX,data);
//    assert(lastApiError == Tfa98xx_Error_Ok);
    // set mem ctl
    lastApiError = tfa98xx_write_register16(handle, TFA98XX_CF_CONTROLS,
             Tfa98xx_DMEM_XMEM << TFA98XX_CF_CONTROLS_DMEM_POS |  // xmem
            /* TFA98XX_CF_CONTROLS_AIF_MSK | */TFA98XX_CF_CONTROLS_RST_MSK  );
    assert(lastApiError == Tfa98xx_Error_Ok);
    tfa98xx_write_register16(handle, TFA98XX_CF_MAD,0);
    assert(lastApiError == Tfa98xx_Error_Ok);

#define ROUND_DOWN(a,n) (((a)/(n))*(n))
    if (lastApiError == Tfa98xx_Error_Ok) {
        int offset = 0;
        int chunk_size =
            ROUND_DOWN(NXP_I2C_BufferSize(), 3);  /* XMEM word size */
        int remaining_bytes = XMEM_MAX*3;

        /* due to autoincrement in cf_ctrl, next write will happen at
         * the next address */
        while ((lastApiError == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
            if (remaining_bytes < chunk_size)
                chunk_size = remaining_bytes;
            /* else chunk_size remains at initialize value above */
            lastApiError =
                tfa98xx_write_data(handle, TFA98XX_CF_MEM,
                          chunk_size, bytedata + offset);
            remaining_bytes -= chunk_size;
            offset += chunk_size;
        }
    }
    for (i = 0; i < XMEM_MAX; i ++) {
        worddata[i]=0xdeadbeef; // clean read buffer
    }
    //     * - burst read testpattern
    lastApiError = tfa98xx_dsp_read_mem(handle, 0, XMEM_MAX, worddata);
    assert(lastApiError == Tfa98xx_Error_Ok);
    //     * - verify data
    for (i = 0; i < XMEM_MAX; i++) {
        if (i != worddata[i]) {
            result = 3;
            break;
        }
    }
    /* report result */
    if (result == 3) {
        sprintf(lastErrorString, "xmem expected read value mismatch\n");
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}
////*
//{
//
//    err = tfaRunStartup(handlesIn[idx]);
//    PRINT_ASSERT(err);
//    if (err)
//        return err;
//
//    /* force cold boot */
//    err = tfaRunColdboot(handlesIn[idx], 1); // set ACS
//    PRINT_ASSERT(err);
//
//}
///* reset all i2C registers to default */
//err = tfa98xx_write_register16(handlesIn[idx], TFA98XX_SYS_CTRL,
//                TFA98XX_SYS_CTRL_I2CR_MSK);
//PRINT_ASSERT(err);
//*/

/*
 * load a patch and verify patch version number
 */
int tfa_diag_patch_load(int slave) {
    Tfa98xx_handle_t handle;
    int ready, result = 0;      // 1 is failure
    int xm_version, idx, data;
    nxpTfaBitfield_t id;
    id.field = bfREV;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    // reset i2c
    lastApiError = tfa98xx_write_register16(handle, TFA98XX_SYS_CTRL,
            TFA98XX_SYS_CTRL_I2CR_MSK);
    assert(lastApiError == Tfa98xx_Error_Ok);
    // power on
    lastApiError = tfaRunStartup(handle);
    assert(lastApiError == Tfa98xx_Error_Ok);
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);

    // report as error
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
    // set cold boot flag
    lastApiError = tfaRunColdboot(handle, 1);
    assert(lastApiError == Tfa98xx_Error_Ok);

    tfaRunReadBitfield(handle, &id);
    id.value &= 0xff; /* mask off high field */
    xm_version = id.value == 0x97 ? TFA9897_XMEM_PATCHVERSION :
                                     TFA_XMEM_PATCHVERSION; // the rest
    /* lookup slave for device index */
    idx = tfa98xx_cnt_slave2idx(slave);
    if (idx<0) {
        result = -1;
        goto stop;
    }
//     * - clear xmem patch version storage
    lastApiError = tfa98xx_dsp_write_mem(idx, xm_version, 0x123456, Tfa98xx_DMEM_XMEM);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - load patch from container
    lastApiError = tfaContWritePatch(idx);
    assert(lastApiError == Tfa98xx_Error_Ok);

//     * - check xmem patch version storage
    lastApiError = tfa98xx_dsp_read_mem(handle, xm_version,1,&data);
    assert(lastApiError == Tfa98xx_Error_Ok);

    /* report result */
    if ( data == 0 || data == 0x123456 ) {
        sprintf(lastErrorString, "patch version not updated");
        result = 3;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_dsp_reset(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    int data, ready;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
    //     * - put DSP in reset
    lastApiError = Tfa98xx_DspReset(handle, 1);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - clear count_boot
    lastApiError = tfaRunResetCountClear(handle);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - release  DSP reset
    lastApiError = Tfa98xx_DspReset(handle, 0);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - verify that count_boot incremented
    data = tfaRunResetCount(handle);
    /* report result */
    if (data != 1) {
        sprintf(lastErrorString, "no proper DSP response to RST\n");
        result = 3;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

// check battery voltage level
int tfa_diag_battery_level(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure
    unsigned short reg;
    //nxpTfaBitfield_t bf;
    int loop=10;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /*   - powerdown by calling init */
    /*  - powerup, by fully clearing r9 */
    /*  - wait for system stable */
    lastApiError = Tfa98xx_Init(handle);
    assert(lastApiError == Tfa98xx_Error_Ok);
    lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0);     // this give us the ADC
    assert(lastApiError == Tfa98xx_Error_Ok);

    if (tfa_diag_pwron_wait(handle)) {
        // assume because of clock missing
        result = 2;
        goto stop;
    }
#define TFA_VBAT(R) (R*5.5/1024)
    // check battery level
    while(loop--) {
        lastApiError = Tfa98xx_ReadRegister16(handle, TFA98XX_TEMPERATURE, &reg);
        assert(lastApiError == Tfa98xx_Error_Ok);
        if (reg<0x100) // wait until th ADC's are up, 0x100 means not ready yet
            break;
    }
    if (loop==0) {
        sprintf(lastErrorString, " ADC's startup timed out\n");
        result = 3;
        goto stop;
    }
    lastApiError =
        Tfa98xx_ReadRegister16(handle, TFA98XX_BATTERYVOLTAGE, &reg);
    assert(lastApiError == Tfa98xx_Error_Ok);
    /* report result */
    if ( TFA_VBAT(reg) < 2.5 || TFA_VBAT(reg) > 5.5 ) {
        sprintf(lastErrorString, " battery level not within bounds (%2.2fV). Reg value is 0x%04x\n", TFA_VBAT(reg), reg);
        result = 3;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}


int tfa_diag_start_speakerboost(int slave) {
    Tfa98xx_handle_t handle;
    int ready, result = 0, channel=0;      // 1 is failure
    int vstep[TFACONT_MAXDEVS]={0,0,0,0};

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }
//     * - start with forced coldflag
    //TODO deal with other device handle
    tfa98xx_set_vstep(vstep[0]);

    lastApiError = tfaRunSpeakerBoost(handle, 1); /* force coldstart */
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * - verify that the ACS flag cleared

    /* report result */
    if (tfaRunIsCold(handle)) {
        sprintf(lastErrorString, "DSP did not clear ACS");
        result = 3;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_read_version_tag(int slave) {
    Tfa98xx_handle_t handle;
    int ready, result = 0;      // 1 is failure
    unsigned char tag[TFA98XX_MAXTAG];

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }

    // * - read ROMID TAG
    lastApiError = Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST*/ , 0xFF,
            TFA98XX_MAXTAG, tag);
    assert(lastApiError == Tfa98xx_Error_Ok);

    /* report result */
    if (tag[2]!='<') {  // * - check 1st character to be '<'
        sprintf(lastErrorString, "invalid patch version tag format");
        result = 3;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_verify_parameters(int slave) {
    Tfa98xx_handle_t handle;
    int i, size, idx, ready, result = 0;      // 1 is failure
    unsigned char buffer[TFA98XX_SPEAKERPARAMETER_LENGTH]; /* biggest size */
    const unsigned char *srcdata;
    nxpTfaSpeakerFile_t *spkr;
    nxpTfaConfig_t *cfg;
    nxpTfaPreset_t *preset;
    nxpTfaVolumeStep2File_t *vstep;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);

    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }

    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);

    assert(lastApiError == Tfa98xx_Error_Ok);

    if (!ready) {
        result = 2;
        goto stop;
    }

//     * - assure ACS is set
    if (tfaRunIsCold(handle))
    {
            result = 3;
            sprintf(lastErrorString, "DSP is not configured");
            goto stop;
    }

    /* lookup slave for device index */
    idx = tfa98xx_cnt_slave2idx(slave);

    if (idx<0) {
        result = -1;
        goto stop;
    }
//     * - verify that SB parameters are correctly loaded

    /************* speaker parameters ************************/
    lastApiError = Tfa98xx_DspReadSpeakerParameters(handle,
                        TFA98XX_SPEAKERPARAMETER_LENGTH, buffer);

    assert(lastApiError == Tfa98xx_Error_Ok);

    /* find the speaker params for this device */
    spkr = (nxpTfaSpeakerFile_t *)tfacont_getfiledata(idx, 0, speakerHdr);

    if (spkr==0) {
        result = -1;
        goto stop;
    }

    srcdata = spkr->data; /*payload*/

    /* fix potentially modified params */
    for(i=0;i<3*128;i++) /* FIR is 128 words */
        buffer[i] = srcdata[i];
    //fRes =136
    for(i=135*3;i<136*3;i++) /* 1 words */
        buffer[i] = srcdata[i];
    // tCoef=141
    for(i=140*3;i<141*3;i++) /* 1 word */
        buffer[i] = srcdata[i];

    /* report result */
    if (memcmp(srcdata, buffer, TFA98XX_SPEAKERPARAMETER_LENGTH)!=0) {
        sprintf(lastErrorString, "speaker parameters error");
        result = 4;
        goto stop;
    }
    /************* config parameters ************************/
    tfa98xx_dsp_config_parameter_count(handle, &size);
    size *=3; //need bytes
    lastApiError = tfa98xx_dsp_read_config(handle, size, buffer);

    assert(lastApiError == Tfa98xx_Error_Ok);
    /* find the speaker params for this device */
    cfg = (nxpTfaConfig_t *)tfacont_getfiledata(idx, 0, configHdr);

    if (cfg==0) {
        result = -1;
        goto stop;
    }
    srcdata = cfg->data; /*payload*/
    /* fix potentially modified params */
    buffer[78] = srcdata[78];
    buffer[79] = srcdata[79];
    buffer[80] = srcdata[80];

    if (memcmp(srcdata, buffer, size)!=0) {
        sprintf(lastErrorString, "config parameters error");
        result = 5;
        goto stop;
    }
    /************* preset parameters ************************/
    /* assume that  profile 0 and vstep 0 are loaded */
    lastApiError = tfa98xx_dsp_read_preset(handle, TFA98XX_PRESET_LENGTH, buffer);
    assert(lastApiError == Tfa98xx_Error_Ok);

    /* find the speaker params for this device */
    preset = (nxpTfaPreset_t *)tfacont_getfiledata(idx, 0, presetHdr);

    if (preset==0) {
        /* maybe we have a vstep here */
        vstep = (nxpTfaVolumeStep2File_t *)tfacont_getfiledata(idx, 0, volstepHdr);

        if (vstep==0) {
            result = -1; /* no vstep either */
            goto stop;
        }

        srcdata = vstep->vstep[0].preset; /*payload for step 0*/
    } else {
        srcdata = preset->data; /*payload*/
    }

    if (memcmp(srcdata, buffer, TFA98XX_PRESET_LENGTH)!=0) {
        sprintf(lastErrorString, "preset parameters error");
        result = 6;
        goto stop;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_calibrate_always(int slave) {
    Tfa98xx_handle_t handle;
    //int i, size;
    int idx, ready, result = 0;      // 1 is failure
    Tfa98xx_handle_t handlesIn[4]={-1,-1,-1,-1}; // TODO fix interface
    float Ractual;

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    handlesIn[0]=handle;
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }

//     * - assure ACS is set
    if (tfaRunIsCold(handle))
     {
            result = 3;
            sprintf(lastErrorString, "DSP is not configured");
            goto stop;
        }
    /* lookup slave for device index */
    idx = tfa98xx_cnt_slave2idx(slave);
    if (idx<0) {
        result = -1;
        goto stop;
    }

    /* execute test */
//     * - run calibration always
    lastApiError = tfa98xxCalibration(handlesIn, 0, 0);
    if (lastApiError != Tfa98xx_Error_Ok) {
        sprintf(lastErrorString, "calibration call failed");
        result = 4;
        goto stop;
    }
    assert(lastApiError == Tfa98xx_Error_Ok);

    //     * -check R for non-zero
    lastApiError = Tfa98xx_DspGetCalibrationImpedance(handle, &Ractual);
    assert(lastApiError == Tfa98xx_Error_Ok);

    /* report result */
    if ( Ractual==0 ) {
        sprintf(lastErrorString, "calibration failed, returned value is 0");
        result = 5;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}
int tfa_diag_speaker_impedance(int slave) {
    Tfa98xx_handle_t handle;
    //int i, size;
    int idx, ready, result = 0;      // 1 is failure
    Tfa98xx_handle_t handlesIn[4]={-1,-1,-1,-1}; // TODO fix interface
    nxpTfaSpeakerFile_t *spkr;
    nxpTfaConfig_t *cfg;
    int Rapp, calibrateDone;
    float Rtypical,Rexpect, Ractual;

    TRACEIN;


    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    handlesIn[0]=handle;
    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    handlesIn[1]=handle;
    /* execute test */
    //     * - ensure clock is on
    lastApiError = Tfa98xx_DspSystemStable(handle, &ready);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if (!ready) {
        result = 2;
        goto stop;
    }

//     * - assure ACS is set
    if (tfaRunIsCold(handle))
     {
            result = 3;
            sprintf(lastErrorString, "DSP is not configured");
            goto stop;
        }
    /* lookup slave for device index */
    idx = tfa98xx_cnt_slave2idx(slave);
    if (idx<0) {
        result = -1;
        goto stop;
    }

    /* execute test */
    lastApiError = tfa98xx_dsp_read_mem(handle, 231, 1, &calibrateDone);
    assert(lastApiError == Tfa98xx_Error_Ok);
    if ( !calibrateDone) {
        result = 4;
        sprintf(lastErrorString, "calibration was not done");
        goto stop;
    }
//     * - get Rapp from config
    cfg = (nxpTfaConfig_t *)tfacont_getfiledata(idx, 0, configHdr);
    if (cfg==0) {
        result = -1;
        goto stop;
    }

    tfa98xx_convert_bytes2data(3, &cfg->data[3*(9-1)], &Rapp);

    Rapp /= 16384;

    //PRINT(">>>>>>>>>>>>>>>>>%d %d %d RAPP=%d\n",cfg->data[3*(9-1)],cfg->data[3*(9-1)+1],cfg->data[3*(9-1)+2], Rapp);
     // * - get Rtypical from speakerfile
    spkr = (nxpTfaSpeakerFile_t *)tfacont_getfiledata(idx, 0, speakerHdr);
    if (spkr==0) {
        result = -1;
        goto stop;
    }

    Rtypical = spkr->ohm;
    if (Rtypical==0) {
        PRINT_ERROR("Warning: Speaker impedance not defined in spkr file, assuming 8 Ohm!\n");
        Rtypical = 8;
    }

    Rexpect = (float)(Rtypical + Rapp);
    //     * - compare result with expected value
    lastApiError = Tfa98xx_DspGetCalibrationImpedance(handlesIn[idx], &Ractual);
    assert(lastApiError == Tfa98xx_Error_Ok);
//     * Assume the speakerfile header holds the typical value of the active speaker
//     * the range is +/- 15% Rtypical + Rapp
    /* report result */

    if ( Ractual < (Rexpect*0.85) || Ractual > (Rexpect*1.15)) {
        sprintf(lastErrorString, "calibration is not within expected range  (%2.2f Ohm)\n", Ractual);
        result = 5;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}


int tfa_diag_record_livedata(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1000;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_speaker_temperature(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1000;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_audio_continuous_play(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1000;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_audio_interval_play(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_audio_record(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_power_1v8(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_reset(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_interrupt(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_LEDs(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}

int tfa_diag_board_ADCs(int slave) {
    Tfa98xx_handle_t handle;
    int result = 0;      // 1 is failure

    TRACEIN;

    lastApiError = Tfa98xx_Open(slave << 1, &handle);
    if (lastApiError != Tfa98xx_Error_Ok) {
        result = 1;
        goto stop;
    }
    /* execute test */

    /* report result */
    if (1) {
        sprintf(lastErrorString, "test %s is not yet implemented!\n",
                __FUNCTION__);
        result = 1;
    }

    stop: Tfa98xx_Close(handle);

    lastError = result;
    TRACEOUT;
    return result;
}


/**************************************************************
 * TODO cleanup old tests
 */
/*
 * write/read test of a register that has no risk of causing damage
 *   return code:
 *       2 :  write value wrong
 *
  */
int tfa98xxDiagI2cRw(int slave)
{
        Tfa98xx_handle_t handle;
        int testregoffset, result = 0;      // 1 is failure
        unsigned short testreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 1;

        // get the index of the testreg
        for ( testregoffset = 0;  testregoffset < MAXREGS;  testregoffset++) {
            if (regdefs[ testregoffset].offset == RWTEST_REG)
                break;
        }

        // powerdown to avoid side effects
        lastApiError = Tfa98xx_Powerdown(handle, 1);
        assert(lastApiError == Tfa98xx_Error_Ok);
        // check pwron default first
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (regdefs[ testregoffset].pwronDefault != (testreg & regdefs[ testregoffset].pwronTestmask)) {
                sprintf(lastErrorString,
                        "poweron default wrong: %s (0x%02x), exp:0x%04x rcv:0x%04x\n",
                        regdefs[ testregoffset].name, regdefs[ testregoffset].offset,
                        regdefs[ testregoffset].pwronDefault, testreg);
                result = 2;
                goto stop;
        }
        // write 0x1234
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, 0x1234);     //
        assert(lastApiError == Tfa98xx_Error_Ok);
        lastApiError = Tfa98xx_ReadRegister16(handle, RWTEST_REG, &testreg);    //
        assert(lastApiError == Tfa98xx_Error_Ok);

        // restore default, else pwrdefault may fail
        lastApiError = Tfa98xx_WriteRegister16(handle, RWTEST_REG, regdefs[RWTEST_REG].pwronDefault);   //
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (0x1234 != testreg) {
                sprintf(lastErrorString,
                        "read back value mismatch: (testreg=0x%02x), exp:0x%04x rcv:0x%04x\n",
                        RWTEST_REG, 0x1234, testreg);
                result = 1;
        }

 stop:
        Tfa98xx_Close(handle);

        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * check status register flags and assume coldstart (or fail)
 *   return code:
 *      1 : error bit
 *       2 : not cold powered on
 *       other internal errors
 */
int tfa98xxDiagStatusCold(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        unsigned short statusreg;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok) {
                return 3;
        }

        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &statusreg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        if (!(statusreg & TFA98XX_STATUSREG_ACS)) {        /* ensure cold booted */
                sprintf(lastErrorString, "not a cold start");
                result = 2;
                goto stop;
        }

        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUSREG_ERRORS_SET_MSK,
             TFA98XX_STATUSREG_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 1;
        }

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * check status register flags for any of the error bits set
 *  return 0 if ok
 *         1 if not ok
 *         other internal errors
 */
int tfa98xxDiagStatus(Tfa98xx_handle_t handle, unsigned short setmask,
                      unsigned short clearmask)
{
        int result = 0;         // 1 is failure
        unsigned short statusreg;

        TRACEIN;

        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &statusreg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        if ((statusreg & setmask))      /* check for any of these bits set */
                return 1;
        if ((~statusreg & clearmask))   /* check for any of these bits clear */
                return 1;

        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * verify default state of relevant registers
 */
int tfa98xxDiagRegisterDefaults(int slave)
{
  Tfa98xx_handle_t handle;
  int i, result = 0;      // 1 is failure
  unsigned short regval;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        for (i = 0; i < MAXREGS; i++) {
                if (regdefs[i].pwronTestmask == 0)
                        continue;
                lastApiError =
                    Tfa98xx_ReadRegister16(handle, regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);

                if (regdefs[i].pwronDefault !=
                    (regval & regdefs[i].pwronTestmask)) {
                        sprintf(lastErrorString,
                                "poweron default wrong: %s (0x%02x), exp:0x%04x rcv:0x%04x\n",
                                regdefs[i].name, regdefs[i].offset,
                                regdefs[i].pwronDefault, regval);
                        result++;
                }
        }
        // set DC-DC peak protection bit
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SPKR_CALIBRATION, 0x0c00);       //
        assert(lastApiError == Tfa98xx_Error_Ok);

// stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}


/**
 * dump all known registers
 *  @return
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDump(int slave)
{
  Tfa98xx_handle_t handle;
  int i;
  unsigned short regval;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 0;

        for (i = 0; i < MAXREGS; i++) {
                lastApiError =
                    Tfa98xx_ReadRegister16(handle, regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);
                PRINT("0x%02x:0x%04x ",
                       regdefs[i].offset, regval);
                if (tfaRunBitfieldDump(stdout, regdefs[i].offset, regval ))
                    PRINT("%s ", regdefs[i].name);
        }
        PRINT("\n");

        Tfa98xx_Close(handle);

        TRACEOUT;
        return i;
}
/**
 * dump all TDM registers
 *  @return
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDumpTdm(int slave)
{
  Tfa98xx_handle_t handle;
  int i = 0;
  unsigned short regval;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 0;

        if ( tfa98xx_get_device_revision(handle) == 0x97 )
            for (i = 0; i < MAXTDMREGS; i++) {
                lastApiError =
                        Tfa98xx_ReadRegister16(handle, tdm_regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);
                PRINT("0x%02x:0x%04x ",
                        tdm_regdefs[i].offset, regval);
                if (tfaRunBitfieldDump(stdout, tdm_regdefs[i].offset, regval ))
                    PRINT("%s ", tdm_regdefs[i].name);
            }
            PRINT("\n");

        Tfa98xx_Close(handle);

        TRACEOUT;
        return i;
}
/**
 * dump  Interrupt registers
 *  @return
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDumpInt(int slave)
{
  Tfa98xx_handle_t handle;
  int i=0;
  unsigned short regval;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 0;

        if ( tfa98xx_get_device_revision(handle) == 0x97 )
            for (i = 0; i < MAXINTREGS; i++) {
                lastApiError =
                        Tfa98xx_ReadRegister16(handle, int_regdefs[i].offset, &regval);
                assert(lastApiError == Tfa98xx_Error_Ok);
                PRINT("0x%02x:0x%04x ",
                        int_regdefs[i].offset, regval);
                if (tfaRunBitfieldDump(stdout, int_regdefs[i].offset, regval ))
                    PRINT("%s ",int_regdefs[i].name);
            }
            PRINT("\n");

        Tfa98xx_Close(handle);

        TRACEOUT;
        return i;
}

/*
 * enable clocks in bypass and verify status
 */
int tfa98xxDiagClock(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        // enable the clock in bypass mode

        // 48 kHz left channel I2S with coolflux in Bypass
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, 0x880b); //
        assert(lastApiError == Tfa98xx_Error_Ok);
        //  PLL=BCK, input 1, power off
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0219);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 1.0 uF coil, PLL=BCK, input 1, power on
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0618);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // give it some time
        tfaRun_Sleepus(14000); // 14ms good for all rates
        // check if clocks are stable and running
        // expect: 0x00 : 0xd85f
        if (tfa98xxDiagStatus
            (handle, 0, TFA98XX_STATUSREG_PLLS | TFA98XX_STATUSREG_CLKS)) {
                sprintf(lastErrorString, "clock not running");
                result = 1;     // fail if any of these is clear
                goto stop;
        }
        // any other errors
        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUSREG_ERRORS_SET_MSK,
             TFA98XX_STATUSREG_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 2;
        }
 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/********************************************************************************
 * TODO check if the DSP should be powered down after every test
 */
static void coldStartup(Tfa98xx_handle_t handle)
{
        Tfa98xx_Error_t err;
       // unsigned short status;

        /* load the optimal TFA9887 in HW settings */
        err = Tfa98xx_Init(handle);
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_SetSampleRate(handle, 44100);
        assert(err == Tfa98xx_Error_Ok);

        err = Tfa98xx_Powerdown(handle, 0);
        assert(err == Tfa98xx_Error_Ok);

}

/*
 * start dsp and verify (by reading ROM tag and check status)
 */
#define TFA98XX_MAXTAG              (138)
#define DSP_revstring        "< Dec 21 2011 - 12:33:16 -  SpeakerBoostOnCF >"

int tfa98xxDiagDsp(int slave)
{
        Tfa98xx_handle_t handle;
        int i, result = 0;      // !0 is failure
        unsigned char tag[TFA98XX_MAXTAG];
        char string[TFA98XX_MAXTAG + 1], *ptr;
        const char *exp = DSP_revstring;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        coldStartup(handle);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0xFF,
                                TFA98XX_MAXTAG, tag);
        if (lastApiError != Tfa98xx_Error_Ok) {
                sprintf(lastErrorString, "DSP failure");
                return 1;
        }

        ptr = string;
        // the characters are in every 3rd byte
        for (i = 2; i < TFA98XX_MAXTAG; i += 3) {
                if (isprint(tag[i])) {
                        *ptr++ = tag[i];        // only printable chars
                }
        }
        *ptr = '\0';

        if (strcmp(exp, string)) {
                sprintf(lastErrorString, "wrong DSP revtag: exp %s rcv:%s\n",
                        exp, string);
                return 2;
        }
// stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * load configuration settings and verify readback
 */
unsigned char settings_Setup87_config[];
int tfa98xxDiagLoadConfig(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 1;         // 1 is failure
        Tfa98xx_Config_t cfg;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        coldStartup(handle);
        lastApiError = Tfa98xx_DspWriteConfig(handle, TFA98XX_CONFIG_LENGTH,
                settings_Setup87_config);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0x80,
                                sizeof(cfg), cfg);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_Setup87_config, cfg, sizeof(Tfa98xx_Config_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

// stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

/*
 * load preset values and verify readback
 */
unsigned char settings_HQ_KS_13X18_DUMBO_preset[];

int tfa98xxDiagLoadPreset(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 1;         // 1 is failure
        Tfa98xx_Config_t cfg;
        unsigned char tstbuf[0x87 + sizeof(cfg)];

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 2;

        coldStartup(handle);
        lastApiError =
            Tfa98xx_DspWritePreset(handle, TFA98XX_PRESET_LENGTH,
                        settings_HQ_KS_13X18_DUMBO_preset);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(handle, 1 /*MODULE_SPEAKERBOOST */ , 0x80,
                                sizeof(tstbuf), tstbuf);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_HQ_KS_13X18_DUMBO_preset, &tstbuf[sizeof(cfg)],
                        sizeof(Tfa98xx_Preset_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

// stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

unsigned char settings_KS_13X18_DUMBO_speaker[];
/*
 * load speaker parameters and verify readback
 *
 *   note: this function can be called from other tests it uses the global handle
 */
int tfa98xxDiagLoadSpeaker(int slave)
{
        int result = 1;         // 1 is failure
        int gbl=0;
        Tfa98xx_SpeakerParameters_t spkr;

        TRACEIN;

        if ( gHandle<0 ) {
            lastApiError = Tfa98xx_Open(slave << 1, &gHandle);
            if (lastApiError != Tfa98xx_Error_Ok)
                return 2;
        } else
            gbl = 1;

        coldStartup(gHandle);
        lastApiError =
            Tfa98xx_DspWriteSpeakerParameters(gHandle, TFA98XX_SPEAKERPARAMETER_LENGTH,
                                              settings_KS_13X18_DUMBO_speaker);
        assert(lastApiError == Tfa98xx_Error_Ok);

        lastApiError =
            Tfa98xx_DspGetParam(gHandle, 1 /*MODULE_SPEAKERBOOST */ , 0x86,
                                sizeof(spkr), spkr);
        assert(lastApiError == Tfa98xx_Error_Ok);

        result =
            0 != memcmp(settings_KS_13X18_DUMBO_speaker, spkr,
                        sizeof(Tfa98xx_SpeakerParameters_t));
        if (result)
                sprintf(lastErrorString, "DSP parameters mismatch");

// stop:
         if ( !gbl ) {
             Tfa98xx_Close(gHandle);
             lastError = result;
             gHandle=-1;
         }
        TRACEOUT;
        return result;

}

/*
 * check battery level to be above 2Volts
 *
 *
 */
int tfa98xxDiagBattery(int slave)
{
        Tfa98xx_handle_t handle;
        int result = 0;         // 1 is failure
        unsigned short reg;

        TRACEIN;
        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 4;

        // enable the clock in bypass mode

        // 48 kHz left channel I2S with coolflux in Bypass
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, 0x880b); //
        assert(lastApiError == Tfa98xx_Error_Ok);
        //  PLL=BCK, input 1, power off
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0219);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 1.0 uF coil, PLL=BCK, input 1, power on
        lastApiError = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, 0x0618);       //
        assert(lastApiError == Tfa98xx_Error_Ok);
        // check if clocks are stable and running
        // give it some time
        tfaRun_Sleepus(14000); // 14ms good for all rates
        // expect: 0x00 : 0xd85f
        if (tfa98xxDiagStatus
            (handle, 0, TFA98XX_STATUSREG_PLLS | TFA98XX_STATUSREG_CLKS)) {
                sprintf(lastErrorString, "clock not running");
                result = 2;     // fail if any of these is clear
                goto stop;
        }
        // check battery level
        lastApiError =
            Tfa98xx_ReadRegister16(handle, TFA98XX_BATTERYVOLTAGE, &reg);
        assert(lastApiError == Tfa98xx_Error_Ok);
        // 2V: 2/(5.5/1024)=372
        if (reg < 372) {
                sprintf(lastErrorString,
                        "battery level too low: exp > 2.0V, rcv=%2.2f",
                        reg * (5.5 / 1024));
                result = 1;
                goto stop;
        }
        // any other errors
        if (tfa98xxDiagStatus
            (handle, TFA98XX_STATUSREG_ERRORS_SET_MSK,
             TFA98XX_STATUSREG_ERRORS_CLR_MSK)) {
                sprintf(lastErrorString, "status errorbit active");
                result = 3;
        }
 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        TRACEOUT;
        return result;
}

///*
// * wait until calibration impedance is ok
// *  operate on all devices that have a handle open
// */
//static void waitCalibration(Tfa98xx_handle_t handle, float *pRe25)
//{
//        Tfa98xx_Error_t err;
//        int calibrateDone, count = 500;
//
//        assert(pRe25 != NULL);
//
//        err = Tfa98xx_DspReadMem(handle, 231, 1, &calibrateDone);
//        assert(err == Tfa98xx_Error_Ok);
//        while ((calibrateDone == 0) && (count-- > 0))   // TODO protect loop with timeout?
//        {
//                err = Tfa98xx_DspReadMem(handle, 231, 1, &calibrateDone);
//                assert(err == Tfa98xx_Error_Ok);
//        }
//        err = Tfa98xx_DspGetCalibrationImpedance(handle, pRe25);
//        assert(err == Tfa98xx_Error_Ok);
//
//}

/*
 * verify speaker presence by checking the resistance
 */
int tfa98xxDiagSpeakerPresence(int slave)
{
        Tfa98xx_handle_t handle;
        int result;
        float re0=0;

        TRACEIN;

        lastApiError = Tfa98xx_Open(slave << 1, &handle);
        if (lastApiError != Tfa98xx_Error_Ok)
                return 3;

        gHandle = handle;
        result = tfa98xxDiagLoadSpeaker(slave);
        if (result) {
                result = 2;
                goto stop;
        }
        lastApiError = Tfa98xx_SetConfigured(handle);
        assert(lastApiError == Tfa98xx_Error_Ok);
  //wwww      waitCalibration(handle, &re0);

        // check R for non-0
        if (re0 == 0) {
                sprintf(lastErrorString, "Speaker not detected");
                result = 1;
        } else
                result = 0;

 stop:
        Tfa98xx_Close(handle);
        lastError = result;
        gHandle = -1;
        TRACEOUT;
        return result;
}

/*
 * calibrate speaker and verify speaker presence check R range ,  verifies power from Vbat via DCDC to amplifier
 */
int tfa98xxDiagCalibration(int slave)
{
        TRACEIN;

        TRACEOUT;
        return 1;
}

/*
 * assume I2S input and verify signal activity
 */
int tfa98xxDiagI2sInput(int slave)
{
        {
                Tfa98xx_handle_t handle;
                int result;
                Tfa98xx_StateInfo_t stateInfo;
                unsigned short sysctrlReg;

                TRACEIN;

                lastApiError = Tfa98xx_Open(slave << 1, &handle);
                if (lastApiError != Tfa98xx_Error_Ok)
                        return 3;

                gHandle = handle; // global prevents reopen
                result = tfa98xxDiagLoadSpeaker(slave);
                if (result) {
                        result = 2;
                        goto stop;
                }
                lastApiError = Tfa98xx_SetConfigured(handle);
                assert(lastApiError == Tfa98xx_Error_Ok);

                //select channel

                lastApiError =
                    Tfa98xx_ReadRegister16(handle, 0x09, &sysctrlReg);
                assert(lastApiError == Tfa98xx_Error_Ok);

                lastApiError = Tfa98xx_WriteRegister16(handle, 0x09, sysctrlReg & ~(0x1 << 13));        // input 1
                //   lastApiError = Tfa98xx_WriteRegister16(handle, 0x09, sysctrlReg | (0x1<<13));  // input 2
                assert(lastApiError == Tfa98xx_Error_Ok);

                lastApiError =
                    Tfa98xx_DspGetStateInfo(handle, &stateInfo);
                assert(lastApiError == Tfa98xx_Error_Ok);

                // check for activity
                if ((stateInfo.statusFlag &
                     (1 << Tfa98xx_SpeakerBoost_Activity)) == 0) {
                        sprintf(lastErrorString, "no audio active on input");
                        result = 1;
                } else
                        result = 0;

 stop:
                Tfa98xx_Close(handle);
                lastError = result;
                gHandle = -1;
                TRACEOUT;
                return result;
        }
}

/*
 * for testing the I2S output an external receiver should acknowlege data presence
 */
int tfa98xxDiagI2sOutput(int slave)
{
        TRACEIN;

        TRACEOUT;
        return 1;
}
/*
 * ************************************ TODO keep these demos?
 */
int tfa98xxDiagLoadPresetsDouble(int slave) {
    Tfa98xx_handle_t hL, hR ;
    int result = 0; // 1 is failure
    //Tfa98xx_Config_t cfg;
    //unsigned char tstbuf[0x87 + sizeof(cfg)];

    TRACEIN;
    lastApiError = Tfa98xx_Open(slave << 1, &hL);
    if (lastApiError != Tfa98xx_Error_Ok)
        return 2;
    lastApiError = Tfa98xx_Open((slave+1) << 1, &hR);
    if (lastApiError != Tfa98xx_Error_Ok)
        return 3;


    lastApiError = Tfa98xx_DspWritePreset(hL, TFA98XX_PRESET_LENGTH,
            settings_HQ_KS_13X18_DUMBO_preset);
    assert(lastApiError == Tfa98xx_Error_Ok);

    lastApiError = Tfa98xx_DspWritePreset(hR, TFA98XX_PRESET_LENGTH,
            settings_HQ_KS_13X18_DUMBO_preset);
    assert(lastApiError == Tfa98xx_Error_Ok);


//  stop:
    Tfa98xx_Close(hL);
    Tfa98xx_Close(hR);
    lastError = result;
    TRACEOUT;
    return result;
}

int tfa98xxDiagLoadPresetsMultiple(int slave)
{
    Tfa98xx_handle_t hL, hR, handles[2] ;
    int result = 0; // 1 is failure
    //Tfa98xx_Config_t cfg;
    //unsigned char tstbuf[0x87 + sizeof(cfg)];

    TRACEIN;
    lastApiError = Tfa98xx_Open(slave << 1, &hL);
    if (lastApiError != Tfa98xx_Error_Ok)
        return 2;
    handles[0]=hL;
    lastApiError = Tfa98xx_Open((slave+1) << 1, &hR);
    if (lastApiError != Tfa98xx_Error_Ok)
        return 3;
    handles[1]=hR;


    lastApiError = Tfa98xx_DspWritePresetMultiple(2, handles, TFA98XX_PRESET_LENGTH,
            settings_HQ_KS_13X18_DUMBO_preset);
    assert(lastApiError == Tfa98xx_Error_Ok);


//  stop:
    Tfa98xx_Close(hL);
    Tfa98xx_Close(hR);
    lastError = result;
    TRACEOUT;
    return result;
}
/*
 * binary buffers
 */
// xxd -i settings/Setup87.config
unsigned char settings_Setup87_config[] = {
        0x09, 0xf3, 0x33, 0x01, 0x3e, 0x66, 0x00, 0x54, 0xcd, 0x00, 0x00, 0x14,
        0x00, 0x00, 0x02, 0x1a, 0xee, 0xb4, 0x1b, 0x49, 0x64, 0x1c, 0x62, 0xc3,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x4b, 0x00, 0x01, 0x4b, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xfa, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x01, 0x40, 0x00,
        0x00, 0x03, 0x47, 0x01, 0x47, 0xae, 0x00, 0x19, 0x9a, 0x00, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x01, 0x05, 0x00, 0x00,
        0x00, 0x80, 0x00, 0x00, 0x0f, 0xff, 0x07, 0xc2, 0x8f, 0x00, 0x03, 0xe8,
        0x08, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x01, 0x01, 0x47, 0xae, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x19, 0x99, 0x9a, 0x00, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x30,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x30, 0xec, 0x00, 0x00, 0x00, 0x03, 0xd7,
        0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00
};

unsigned int settings_Setup87_config_len = 165;

unsigned char settings_HQ_KS_13X18_DUMBO_preset[] = {
        0x00, 0x00, 0x07, 0x00, 0x01, 0x2c, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x1f, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x01, 0x47, 0xae,
        0x00, 0x2b, 0xb1, 0x00, 0x00, 0x9d, 0x00, 0x0d, 0x1b, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x0c, 0xcd, 0x00, 0x40, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x48,
        0x00, 0x01, 0x48, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0c, 0xcd,
        0x00, 0x00, 0x03
};

unsigned int settings_HQ_KS_13X18_DUMBO_preset_len = 87;

unsigned char settings_KS_13X18_DUMBO_speaker[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf4, 0x89, 0xff, 0xf2, 0xf4,
        0xff, 0xf9, 0xff, 0xff, 0xf7, 0xcb, 0xff, 0xfa, 0xc3, 0xff, 0xfa, 0xc8,
        0xff, 0xfb, 0xbc, 0xff, 0xfb, 0xa9, 0xff, 0xfc, 0x57, 0xff, 0xfb, 0x53,
        0xff, 0xfc, 0x38, 0xff, 0xfb, 0x4f, 0xff, 0xfb, 0xd5, 0xff, 0xfb, 0x6b,
        0xff, 0xfc, 0x45, 0xff, 0xfb, 0xb1, 0xff, 0xfc, 0x89, 0xff, 0xfc, 0x8d,
        0xff, 0xfc, 0x61, 0xff, 0xfd, 0x1a, 0xff, 0xfc, 0x30, 0xff, 0xfb, 0xbf,
        0xff, 0xfc, 0x5e, 0xff, 0xfa, 0xf6, 0xff, 0xfc, 0xcd, 0xff, 0xfb, 0x95,
        0xff, 0xfc, 0xa5, 0xff, 0xfb, 0xe5, 0xff, 0xfd, 0x09, 0xff, 0xfc, 0xf7,
        0xff, 0xfc, 0x6d, 0xff, 0xfc, 0xf4, 0xff, 0xfc, 0x4a, 0xff, 0xfc, 0x54,
        0xff, 0xfb, 0xea, 0xff, 0xfb, 0x3f, 0xff, 0xfc, 0xa2, 0xff, 0xfc, 0x50,
        0xff, 0xfb, 0xe9, 0xff, 0xfd, 0x66, 0xff, 0xfd, 0x0f, 0xff, 0xff, 0x42,
        0xff, 0xfe, 0xea, 0xff, 0xff, 0x61, 0xff, 0xff, 0x0e, 0xff, 0xfd, 0x33,
        0xff, 0xfc, 0x00, 0xff, 0xfb, 0x50, 0xff, 0xf8, 0xd9, 0xff, 0xf8, 0xd4,
        0xff, 0xf9, 0xf3, 0xff, 0xf9, 0xab, 0xff, 0xfe, 0xfe, 0x00, 0x00, 0x0c,
        0x00, 0x03, 0xcf, 0x00, 0x04, 0x4b, 0x00, 0x05, 0x07, 0x00, 0x01, 0x39,
        0xff, 0xfd, 0x80, 0xff, 0xf4, 0xff, 0xff, 0xf3, 0x34, 0xff, 0xeb, 0xa4,
        0xff, 0xf1, 0x06, 0xff, 0xf1, 0xaf, 0x00, 0x02, 0x1c, 0x00, 0x08, 0xe5,
        0x00, 0x1c, 0xe1, 0x00, 0x1a, 0x8e, 0x00, 0x22, 0x74, 0x00, 0x0c, 0xcf,
        0x00, 0x00, 0xfd, 0xff, 0xda, 0x59, 0xff, 0xcd, 0x2a, 0xff, 0xb0, 0x80,
        0xff, 0xc2, 0xf7, 0xff, 0xca, 0x40, 0x00, 0x08, 0x63, 0x00, 0x29, 0xd9,
        0x00, 0x7c, 0x85, 0x00, 0x7d, 0xb1, 0x00, 0xa8, 0x44, 0x00, 0x46, 0xe9,
        0x00, 0x2e, 0xcc, 0xff, 0x58, 0x94, 0xff, 0xc5, 0x98, 0x07, 0xa2, 0xd1,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x4a, 0x6b, 0x2a, 0x68, 0xf5, 0xc3, 0x26, 0x66, 0x66,
        0x26, 0x66, 0x66, 0x24, 0xcc, 0xcd, 0x19, 0x99, 0x9a, 0x00, 0x02, 0x8d,
        0x00, 0x02, 0x8d, 0x04, 0x00, 0x00, 0x00, 0x67, 0xae, 0x1c, 0xc0, 0x00,
        0x03, 0x7b, 0x4a
};

unsigned int settings_KS_13X18_DUMBO_speaker_len = 423;
