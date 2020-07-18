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



#ifndef TFA98XXDIAGNOSTICS_H_
#define TFA98XXDIAGNOSTICS_H_

/*
 * the  following is directly from tfaRuntime
 *  diag should avoid external type dependency
 */
void tfaRun_Sleepus(int us);
void tfaRun_SleepusTrace(int us, char *file, int line);

#define NXPTFA_DIAG_REV_MAJOR    (1)         // major API rev
#define NXPTFA_DIAG_REV_MINOR    (0)         // minor

extern int tfa98xxDiag_trace;
extern int tfa98xxDiag_verbose;

#define DIAGTRACE

/**
 * Diagnostic test description structure.
 */
enum tfa_diag_group {
    tfa_diag_all,         /**< all tests, container needed, destructive */
    tfa_diag_i2c,         /**< I2C register writes, no container needed, destructive */
    tfa_diag_dsp,     /**< dsp interaction, container needed, destructive */
    tfa_diag_sb,         /**< SpeakerBoost interaction, container needed, not destructive */
    tfa_diag_func,     /**< functional tests,  destructive */
    tfa_diag_pins      /**< Pin tests, no container needed, destructive */
};
typedef struct tfa98xxDiagTest {
        int (*function) (int);          /**< The function pointer of the test */
        const char *description;/**< single line description */
        enum tfa_diag_group group;    /**< test group : I2C ,DSP, SB, PINS */
} tfa98xxDiagTest_t;

/**
 * container needs to be loaded
 * @param test number
 * @return 1 if needed
 */
int tfa_diag_need_cnt(int nr) ;

/**
 * run a testnumber
 *
 *  All the diagnostic test functions will return 0 if passed
 *  on failure the return value may contain extra info.
 * @param slave I2C slave under test
 * @param testnumber diagnostic test number
 * @return if testnumber is too big an empty string is returned
 */
int tfa98xxDiag(int slave, int testnumber);

/**
 * translate group name argument to enum
 * @param groupname
 * @return enum tfa_diag_group
 */
enum tfa_diag_group tfa98xxDiagGroupname(char *arg);

int tfa98xxDiagGroup(int slave,  enum tfa_diag_group group);
/**
 * run all tests in the group
 *
 *  All the diagnostic test functions will return 0 if passed
 *  on failure the return value may contain extra info.
 * @param slave I2C slave under test
 * @param test group
 * @return 0
 * @return > 0 test  failure
 * @return < 0 config data missing
 */


/**
 *  print supported device features
 *
 */
void tfa98xxDiagPrintFeatures(int devidx, char *strings);

/******************************************************************************
 *
 *    the test functions
 *
 *****************************************************************************/
/**
 * list all tests descriptions
 * @param slave I2C slave under test
 * @return 0 passed
 */
int tfa_diag_help(int slave);

/**
 * read test of DEVID register bypassing the tfa98xx API
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_register_read(int slave);

/**
 * write/read test of  register 0x71
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_register_write_read(int slave);

/**
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no match
 */
int tfa_diag_check_device_features(int slave);

/**
 * check PLL status after poweron
 *  - powerdown
 *  - powerup
 *  - wait for system stable
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 power-on timed out
 */
int tfa_diag_clock_enable(int slave);

/**
 * write/read test of  xmem locations
 * - ensure clock is on
 * - put DSP in reset
 * - write count into all xmem locations
 * - verify this count by reading back xmem
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 xmem expected read value mismatch
 */
int tfa_diag_xmem_access(int slave);

/**
 * set ACS bit via iomem, read via status reg
 * - ensure clock is on
 * - clear ACS via CF_CONTROLS
 * - check ACS is clear in status reg
 * - set ACS via CF_CONTROLS
 * - check ACS is set in status reg
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 no control over ACS via iomem
 */
int tfa_diag_ACS_via_iomem(int slave);

/**
 * write xmem, read with i2c burst
 * - ensure clock is on
 * - put DSP in reset
 * - write testpattern
 * - burst read testpattern
 * - verify data
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 xmem expected read value mismatch
 */
int tfa_diag_xmem_burst_read(int slave);

/**
 * write/read full xmem in i2c burst
 * - ensure clock is on
 * - put DSP in reset
 * - burst write testpattern
 * - burst read testpattern
 * - verify data
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 xmem expected read value mismatch
 */
int tfa_diag_xmem_burst_write(int slave);

/**
 * verify dsp response to reset toggle
 * - ensure clock is on
 * - put DSP in reset
 * - write count_boot in xmem to 1
 * - release  DSP reset
 * - verify that count_boot incremented to 2
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no proper DSP response to RST
 */
int tfa_diag_dsp_reset(int slave);

/**
 * check battery voltage level
 * - force i2c reset, clock on and cfe on
 * - check that level is between
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 battery level not within bounds
 */
int tfa_diag_battery_level(int slave);

/**
 * verify dsp response to reset toggle
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_dsp_reset(int slave);

/**
 * check battery voltage level
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_battery_level(int slave);

/**
 * load a patch and verify patch version number
 * - ensure clock is on
 * - clear xmem patch version storage
 * - load patch from container
 * - check xmem patch version storage
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 patch version no updated
 */
int tfa_diag_patch_load(int slave);


/**
 * DSP framework interaction
 */
/**
 * load the config to bring up SpeakerBoost
 * - ensure clock is on
 * - start with forced coldflag
 * - verify that the ACS flag cleared
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 DSP did not clear ACS
 */
int tfa_diag_start_speakerboost(int slave);

/**
 * read back the ROM version tag
 * - ensure clock is on
 * - read ROMID TAG
 * - check 1st character to be '<'
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 invalid patch version tag format
 */
int tfa_diag_read_version_tag(int slave);

/**
 * read back to verify that all parameters are loaded
 * - ensure clock is on
 * - assure ACS is set
 * - verify that SB parameters are correctly loaded
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 DSP is not configured
 * @return 4 speaker parameters error
 * @return 5 config parameters error
 * @return 6 preset parameters error
 */
int tfa_diag_verify_parameters(int slave);

/**
 * run a calibration and check for success
 * - ensure clock is on
 * - assure ACS is set
 * - run calibration
 * - check R for non-zero
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 DSP is not configured
 * @return 4 calibration call failed
 * @return 5 calibration failed, returned value is 0
 */
int tfa_diag_calibrate_always(int slave);

/**
 * verify that the speaker impedance is within range
 * - ensure clock is on
 * - assure ACS is set
 * - check calibration done
 * - get Rapp from config
 * - get Rtypical from speakerfile
 * - compare result with expected value
 * Assume the speakerfile header holds the typical value of the active speaker
 * the range is +/- 15% Rtypical + Rapp
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 DSP is not configured
 * @return 4 calibration was not done
 * @return 5 calibration is not within expected range
 */
int tfa_diag_speaker_impedance(int slave);

/**
 * verify all the live data can be read back properly
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_record_livedata(int slave);

/**
 * check the speakertemperature
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_speaker_temperature(int slave);

/**
 * playback audio and check speakertemp
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_audio_continuous_play(int slave);

/**
 * playback audio with clock stops,check speakertemp
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_audio_interval_play(int slave);

/**
 * check the recording functionality
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa_diag_audio_record(int slave);

/**
 * check the interrupt hardware functionality
 * @return 0 passed
 * @return 1 no device found
 * @return 2 no clock
 * @return 3 interrupt bit(s) did not clear
 */
int tfa_diag_irq_cold(int slave);
int tfa_diag_irq_warm(int slave);

int tfa_diag_power_1v8(int slave);
int tfa_diag_reset(int slave);
int tfa_diag_interrupt(int slave);
int tfa_diag_LEDs(int slave);
int tfa_diag_board_ADCs(int slave);
/********************* OLD functions *************************/
/**
 * read I2C  ID register
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 no device found
 */
int tfa98xxDiagI2cRdId(int slave);

/**
 * write/read test of a register that has no risk of causing damage
 *  - check for default value
 *  - write pattern and read verify
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 RW pattern mismatch
 * @return 2 pwron default wrong
 */
int tfa98xxDiagI2cRw(int slave);

/**
 * check status register flags and assume coldstart (or fail)
 *  status register errorbits checked for 1
 *  - OCDS
 *  status register errorbits checked for 0
 *  - VDDS UVDS OVDS OTDS
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 error bit found active
 * @return 2 not cold power on
 * @return 3 I2C slave not found or other internal errors
 */
int tfa98xxDiagStatusCold(int slave);

/**
 * verify default state of relevant registers
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 error bit found active
 * @return 2 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagRegisterDefaults(int slave);

/**
 * enable clocks in bypass
 *  - verify clock running
 *  - check error status bits
 *  .
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 clock not running
 * @return 2 error bit found active
 * @return 3 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagClock(int slave);

/**
 * start dsp and verify (by reading ROM tag and check status)
 * @param slave I2C slave under test
 * @return 1 DSP failure
 * @return 2 wrong DSP revtag
 * @return 3 I2C slave not found or other internal errors
 *
 */
int tfa98xxDiagDsp(int slave);

/**
 * load configuration settings and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadConfig(int slave);

/**
 * load preset values and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadPreset(int slave);

/**
 * load speaker parameters and verify by reading back
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 DSP parameters mismatch
 * @return 2 I2C slave not found or other internal errors
 */
int tfa98xxDiagLoadSpeaker(int slave);

/**
 * check battery level to be above 2Volts
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 battery level too low
 * @return 2 clock not running
 * @return 3 error bit found active
 * @return 4 I2C slave not found or other internal errors
 */
int tfa98xxDiagBattery(int slave);

/**
 * verify the presence of the speaker by checking the resistance for non-0
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 speaker not detected
 * @return 2 speaker parameters could not be loaded
 * @return 3 I2C slave not found or other internal errors
 */
int tfa98xxDiagSpeakerPresence(int slave);

/**
 * calibrate speaker and verify speaker presence check R range ,  verifies power from Vbat via DCDC to amplifier
 * @param slave I2C slave under test
 */
int tfa98xxDiagCalibration(int slave);

/**
 * assume I2S input and verify signal activity
 *  - initialize DSP
 *  - load speaker file
 *  - select I2S input
 *  - get DspSpeakerBoost StateInfo
 *  - check for audio active bit
 * @param slave I2C slave under test
 * @return 0 passed
 * @return 1 audio not detected
 * @return 2 speaker parameters could not be loaded
 * @return 3 I2C slave not found or other internal errors

 */
int tfa98xxDiagI2sInput(int slave);
/**
 * for testing the I2S output an external receiver should acknowlege data presence
 * @param slave I2C slave under test
 */
int tfa98xxDiagI2sOutput(int slave);

/******************************************************************************
 *
 *    generic support functions
 *
 *****************************************************************************/

/**
 * dump all known registers
 *   returns:
 *     0 if slave can't be opened
 *     nr of registers displayed
 * @param slave I2C slave under test
 */
int tfa98xxDiagRegisterDump(int slave);

/**
 * dump all TDM registers
 * @param slave I2C slave under test
 *  @return
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDumpTdm(int slave);

/**
 * dump  Interrupt registers
 * @param slave I2C slave under test
 *  @return
 *     0 if slave can't be opened
 *     nr of registers displayed
 */
int tfa98xxDiagRegisterDumpInt(int slave);


/**
 * run all tests
 * @param slave I2C slave under test
 * @return last error code
 */
int tfa98xxDiagAll(int slave);

/**
 * return the number of the test that was executed last
 * @param slave I2C slave under test
 * @return latest testnumber
 */
int tfa98xxDiagGetLatest(void);

/**
 * return the errorcode of the test that was executed last
 * @return last error code
 */
int tfa98xxDiagGetLastError(void);

/**
 * return the errorstring of the test that was executed last
 * @return last error string
 */
char *tfa98xxDiagGetLastErrorString(void);

/**
 * Return the single line test description.
 * @param testnumber
 * @return testname string
 * @return if testnumber is too big an empty string is returned
 */
const char *tfa98xxDiagGetTestNameString(int testnumber);

#endif                          /* TFA98XXDIAGNOSTICS_H_ */
