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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !(defined(WIN32) || defined(_X64))
#include <unistd.h>
#include <inttypes.h>    //TODO fix/converge types
#endif
#include <lxScribo.h>

#include "cmdline.h"
#include "climax.h"
#include "dbgprint.h"
#include "nxpTfa98xx.h"
#include "tfa98xxCalibration.h"
#include "tfa98xxDiagnostics.h"
#include "tfa98xxLiveData.h"
#include "tfaContainer.h"
#include "tfa98xxRuntime.h"
#include <limits.h>

#if !(defined(WIN32) || defined(_X64))
/************************
 * time measurement TODO cleanup
 */
#include <sys/time.h>
#include <sys/resource.h>
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

extern int gNXP_i2c_writes, gNXP_i2c_reads; // declared in NXP i2c hal
//extern int gTfaRun_timingVerbose; // in tfaRuntime
#endif

extern struct gengetopt_args_info gCmdLine; /* Globally visible command line args */
static int nxpTfaCurrentProfile;
extern nxpTfa98xxParameters_t tfaParams;
int gNXP_i2c_writes, gNXP_i2c_reads;

/*
 *
 */
static size_t cliFileToBuffer(char *filename, char *buffer, int max)
{
    FILE *f;
    size_t len;

    f = fopen(filename, "rb");;
    if (!f) {
        PRINT_ERROR("can't open %s\n", filename);
        return -1;
    }

    len = fread(buffer, 1, max, f);
    if(cli_verbose) PRINT("%s: %i bytes from %s\n", __FUNCTION__, (int)len, filename);

    fclose(f);

    return len;
}

/*
 * execute the commands
 */
int cliCommands(int targetfd, char *xarg, Tfa98xx_handle_t *handlesIn)
{
    unsigned char buffer[4*1024];    //,byte TODO check size or use malloc
    int i,writes;
    //nxpTfa98xxParamsType_t params;
    int length, maxdev;
    int loopcount=gCmdLine.loop_arg, printloop=1;
    float imp, tCoef;
    tfa_srv_api_error_t error=tfa_srv_api_error_Ok;

    // load cli defaults
    if ( gCmdLine.currentprof_given ) {
        if ( tfa98xx_cnt_max_device() == -1) {
            PRINT("Please supply a container file with profile argument.\n");
            return 0;
        } else {
            nxpTfaCurrentProfile = gCmdLine.currentprof_arg;
            for (i=0; i < tfa98xx_cnt_max_device(); i++ ) {
                if ( nxpTfaCurrentProfile < 0 || nxpTfaCurrentProfile > tfaContMaxProfile(i) ) {
                    PRINT("Incorrect current profile given for device %s. The value must be [0 - %d].\n", tfaContDeviceName(i), (tfaContMaxProfile(i)-1));
                    error = tfa_srv_api_error_BadParam;
                }
            }
            if (error != tfa_srv_api_error_Ok) {    // error with profile number so exit
                PRINT("last status: %d (%s)\n", error, nxpTfa98xxGetErrorString(error));
                return error;
            }
        }
    }

  do {
    if ( gCmdLine.reset_given ) {
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 1;
        }
        for (i=0; i < maxdev; i++ ) {
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
            }
            else {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("Reset Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }
            if( handlesIn[i] == -1) //TODO properly handle open/close dev
            {
                PRINT_ASSERT( Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[i] ));
            }
            error = tfa98xxReset( handlesIn, tfa98xxI2cSlave, i );
            if (error)
                return error;
        }
    }

    if ( gCmdLine.pin_given ) {
            if (xarg) {//set if extra arg
                lxScriboSetPin(targetfd,gCmdLine.pin_arg, atoi(xarg));
                PRINT("pin%d < %d\n", gCmdLine.pin_arg, atoi(xarg));
            }
            else
                PRINT("pin%d : %d\n", gCmdLine.pin_arg,lxScriboGetPin(targetfd,gCmdLine.pin_arg));
    }

    // the param file is a multi arg
    for (i = 0; i < (int)gCmdLine.params_given; ++i)
    {
        if( handlesIn[0] == -1) //TODO properly handle open/close dev
            PRINT_ASSERT( nxpTfa98xx_Open(&handlesIn[0]));

        error = tfaContWriteFileByname(handlesIn[0], gCmdLine.params_arg[i]);

        if ( (int)error != (int)Tfa98xx_Error_Ok) {
            if ((int)error == (int)Tfa98xx_Error_DSP_not_running) {
                PRINT("DSP not running\n");
            } else {
                PRINT("not a valid input file: %s\n", gCmdLine.params_arg[i]);
            }
            _exit(1);
        }
    }
//    /*
//     *  here the parameters are known :::in container now
//     */


    //
    // a write is only performed if there is a register AND a regwrite option
    // the nr of register args determine the total amount of transactions
    //
    writes = gCmdLine.regwrite_given;
    for (i = 0; i < (int)gCmdLine.register_given; ++i)
    {
        unsigned short value;

        if ( !writes ) {// read if no write arg
            error = tfa98xxReadRegister(handlesIn, gCmdLine.register_arg[i], &value);
            PRINT("0x%02x : 0x%04x ", gCmdLine.register_arg[i], value);
        } else {
            value = gCmdLine.regwrite_arg[i];
              PRINT("0x%02x < 0x%04x ", gCmdLine.register_arg[i], gCmdLine.regwrite_arg[i]);
              error = tfa98xxWriteRegister ( gCmdLine.register_arg[i], gCmdLine.regwrite_arg[i], handlesIn);
            writes--; // consumed write arg
        }
        if (cli_verbose)
            tfaRunBitfieldDump(stdout, gCmdLine.register_arg[i], value);
        else
            PRINT("\n");
    }

    // read xmem
    for (i = 0; i < (int)gCmdLine.xmem_given; ++i)
    {
        int value;
        unsigned int offset=gCmdLine.xmem_arg[i]&0xffff /*mask off DMEM*/;
        int count = gCmdLine.count_given? gCmdLine.count_arg-1 :  0;
        enum Tfa98xx_DMEM memtype = (gCmdLine.xmem_arg[i]>>16)&3;

        if ( !writes ) { // read if no write arg
            do {
                    error = tfa98xx_DspRead(gCmdLine.xmem_arg[i], handlesIn, &value); /* memtype handled inside */
                               PRINT("xmem[0x%04x] : 0x%06x\n", offset, value);
                               offset++;
            } while (count--);
        } else {
                        if( handlesIn[0] == -1) //TODO properly handle open/close dev
                        PRINT_ASSERT( nxpTfa98xx_Open(&handlesIn[0]));

                      PRINT("0x%02x < 0x%04x (memtype=%d)\n", offset, gCmdLine.regwrite_arg[i], memtype);
                        error = Tfa98xx_DspWriteMem(handlesIn[0], offset, gCmdLine.regwrite_arg[i], memtype);
                    writes--; // consumed write arg
        }
    }

    /* re0 */
    if ( gCmdLine.re0_given ) {
        error = tfa98xxGetCalibrationImpedance(&imp, handlesIn, 0);
        if ( error==tfa_srv_api_error_Ok )
            PRINT("re0  : %2.2f\n", imp);
        if ( gCmdLine.re0_arg ) {
            error = tfa98xxSetCalibrationImpedance( (gCmdLine.re0_arg), handlesIn);
            if ( error == tfa_srv_api_error_Ok ) {
                PRINT("New re0: %2.2f\n", gCmdLine.re0_arg);
            } else if ( error == tfa_srv_api_error_BadParam ) {
                PRINT("Unable to set re0: %2.2f. \n", gCmdLine.re0_arg);
                PRINT("The allowed range is 4.0 to 10.0 ohm.");
            } else {
                PRINT("Unable to set re0: %2.2f.\n", gCmdLine.re0_arg);
                PRINT("Check the patch or device for errors.\n");
            }
        }
    }

    if ( gCmdLine.resetMtpEx_given ) {
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 1;
        }
        for (i=0; i < maxdev; i++ ) {
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
            }
            else {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("Reset MtpEx Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }
            if( handlesIn[i] == -1) //TODO properly handle open/close dev
            {
                PRINT_ASSERT( Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[i] ));
            }
            error = tfa98xxCalResetMTPEX( handlesIn[i] );
            if ( error!=tfa_srv_api_error_Ok ) {
                PRINT_ERROR("Reset MTPEX failed\n");
                return error;
            }
        }
    }

    // must come after loading param files, it will need the loaded speakerfile
    if ( gCmdLine.calibrate_given ) {
        int maxdev;
        if ( tfa98xx_cnt_max_device() < 0 ) {
            PRINT_ERROR("Please provide/load container file for calibration.\n");
            goto errorexit;
        }
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        for (i=0; i < maxdev; i++ ) {
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
                PRINT("Warning: using parameters for device[0] from cnt file!\n");
            }
            else
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */

            PRINT("Found Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            if( handlesIn[i] == -1) //TODO properly handle open/close dev
            {
                PRINT_ASSERT( Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[i] ));
            }
            //once if o , else always
            error = tfa98xxCalibration(handlesIn, i, gCmdLine.calibrate_arg[0]=='o');

            if ( error!=tfa_srv_api_error_Ok ) {
                PRINT_ERROR("1st-time calibration failed\n");
                goto errorexit;
            }
        }
    }

    // shows the current  impedance
    if ( gCmdLine.calshow_given )
    {
        unsigned char buffer[TFA98XX_SPEAKERPARAMETER_LENGTH];
        int maxdev;
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 1;
        }

        for (i=0; i < maxdev; i++ ) {
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
            }
            else {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("Found Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }
            if( handlesIn[i] == -1) //TODO properly handle open/close dev
            {
                PRINT_ASSERT( Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[i] ));
            }
            error = tfa98xxGetCalibrationImpedance(&imp, handlesIn, i);
            PRINT("Current calibration impedance: %f\n", imp);
            error = Tfa98xx_DspReadSpeakerParameters(handlesIn[i],  TFA98XX_SPEAKERPARAMETER_LENGTH, buffer);
            tCoef = tfa98xxCaltCoefFromSpeaker(buffer);
            PRINT("Current calibration tCoef: %f\n", tCoef);
        }
        if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
        }
    }

    for (i = 0; i < (int)gCmdLine.volume_given; ++i) {
        int steps[TFACONT_MAXDEVS],j;
        for(j=0;j<tfa98xx_cnt_max_device();j++)
            steps[j]=gCmdLine.volume_arg[i];
        if ( gCmdLine.profile_given ) {
            error = tfa98xx_start( gCmdLine.profile_arg, steps, tfa98xx_cnt_max_device());
        }
        else {
            error = tfa98xx_start( nxpTfaCurrentProfile, steps, tfa98xx_cnt_max_device());
        }
    }

    if(gCmdLine.tone_given) {
    if ( tfa98xx_cnt_max_device() < 0 ) {
        PRINT("Please provide container file.\n");
        return 1;
    }

        if( handlesIn[0] == -1) //TODO properly handle open/close dev
        PRINT_ASSERT( nxpTfa98xx_Open(&handlesIn[0]));

        error = tfa98xx_set_tone_detection(handlesIn[0], gCmdLine.tone_arg);
    }

    if ( gCmdLine.versions_given ) {
        error = nxpTfa98xxVersions(handlesIn, (char*)buffer, sizeof(buffer));
        if(error != tfa_srv_api_error_BadParam) {
        length = (int)(strlen((char*)buffer));
                tfa98xxDiagPrintFeatures(handlesIn[0], (char*)buffer+length);
                length = (int)(strlen((char*)buffer));
                length += sprintf((char*)buffer+length, "Climax cli: %s\n",CMDLINE_PARSER_VERSION);
#if (defined(WIN32) || defined(_X64))
                error = tfa98xx_read_versions(buffer + length);
                length = (int)(strlen((char*)buffer));
#else
                // append scribo rev here becaus the nxp interface does not now about scribo
                length += lxScriboGetRev(targetfd, (char*)buffer+length); // overwrite the terminator
#endif
                *(buffer+length) = '\0';        // terminate
                puts((char*)buffer);
        }
    }

    if ( gCmdLine.dsp_given ) {
        int count;
        /* the maximum message length in the communication with the DSP */
#undef MAX_PARAM_SIZE
#define MAX_PARAM_SIZE (144)
        count = gCmdLine.count_given? gCmdLine.count_arg :  MAX_PARAM_SIZE;
        if(count>sizeof(buffer)) count=sizeof(buffer);
        // speakerboost=1
        error = tfa98xxDspGetParam( 1, gCmdLine.dsp_arg,  count, buffer, handlesIn);
        if (error == tfa_srv_api_error_Ok) {
            for(i=0;i<count;i++)
            {
                PRINT("0x%02x ", buffer[i]);
            }
            PRINT("\n");
        }
    }

    /*
     * gCmdLine.diag_arg=0 means all tests
     */
    if ( gCmdLine.diag_given) {
        int maxdev,i,code,nr,slave = gCmdLine.slave_given? gCmdLine.slave_arg : tfa98xxI2cSlave;

        nxpTfaCurrentProfile = gCmdLine.profile_given ? gCmdLine.profile_arg : 0; /* set the profile used in diag */
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
                if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 1;
        }

        for (i=0; i < maxdev; i++ )
        {
            nr = gCmdLine.diag_arg;
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
            }
            else {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("Diagnostics run on Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }

            if (nr>0)
                code = tfa98xxDiag ( tfa98xxI2cSlave,  nr) ;
            else if (nr==-1) {
                code = tfa98xxDiag ( tfa98xxI2cSlave,  0) ; /* only print descriptions */
                break;
            }
            else // all
                code = tfa98xxDiagGroup(tfa98xxI2cSlave,  tfa98xxDiagGroupname(xarg));

            nr = tfa98xxDiagGetLatest();
            PRINT("test %d %s (code=%d) %s\n", nr, code ? "Failed" : "Passed", code,
                    tfa98xxDiagGetLastErrorString());
            error = code ? tfa_srv_api_error_Fail : tfa_srv_api_error_Ok; // non-0 fail
            if(error)
                goto errorexit;
        }
    }
    /*
     * dump
     */
    if ( gCmdLine.dump_given) {
        int maxdev;
        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        if ( maxdev < 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 1;
        }
        for (i=0; i < maxdev; i++ ) {
            if (gCmdLine.slave_given) {
                tfa98xxI2cSlave=gCmdLine.slave_arg;
                tfa98xxI2cSetSlave(tfa98xxI2cSlave);
            }
            else {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("Dump Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }

            tfa98xxDiagRegisterDump(tfa98xxI2cSlave);
            tfa98xxDiagRegisterDumpTdm(tfa98xxI2cSlave);
            tfa98xxDiagRegisterDumpInt(tfa98xxI2cSlave);
        }
    }

    if ( gCmdLine.dumpmodel_given) {
        int model=gCmdLine.dumpmodel_arg[0]=='x';
        char *output_arg_stdout = "stdout";
        int maxdev = gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); // max dev only if no slave option

        if(gCmdLine.dumpmodel_arg[0]!='x' && gCmdLine.dumpmodel_arg[0]!='z') {
            PRINT("Invalid argument given for dumpmodel.\n");
            error = Tfa98xx_Error_Bad_Parameter;
            goto errorexit;
        }

        if (gCmdLine.slave_given) {
            tfa98xxI2cSlave=gCmdLine.slave_arg;
            tfa98xxI2cSetSlave(tfa98xxI2cSlave);

            if( handlesIn[i] == -1) //TODO properly handle open/close dev
                error = Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[0]);
        } else {
            error = nxpTfa98xxOpenLiveDataSlaves(handlesIn, tfa98xxI2cSlave, maxdev);
        }

        if (error == tfa_srv_api_error_Ok) {
            PRINT("dumping %s model\n", model?"excursion":"impedance");

            /* ACS bit test */
            if (tfaRunIsCold(handlesIn[0]) ) {
                PRINT("Unable to continue, (coldboot state)\n");
                error = Tfa98xx_Error_DSP_not_running;
                goto errorexit;
            }

            for (i=0; i < maxdev; i++ ) {
                if ( gCmdLine.output_given) {
                    error = tfa98xxPrintSpeakerModel(handlesIn, gCmdLine.output_arg, model, i, maxdev);
                    if (error != tfa_srv_api_error_Ok)
                        goto errorexit;

                    PRINT("written to %s\n", gCmdLine.output_arg);
                } else {
                    error = tfa98xxPrintSpeakerModel(handlesIn, output_arg_stdout, model, i, maxdev);
                    if (error != tfa_srv_api_error_Ok)
                        goto errorexit;
                }
            }
        } else
            goto errorexit;
    }

errorexit:
    if ( gCmdLine.loop_given) {
        loopcount = ( gCmdLine.loop_arg == 0) ? 1 : loopcount-1 ;
        if (cli_verbose) PRINT("loop count=%d\n", printloop++); // display the count of the executed loops
        if (error)
            loopcount=0; //stop
    }

    if ( (error!=tfa_srv_api_error_Ok)  | cli_verbose ) {
        PRINT("last status :%d (%s)\n", error, nxpTfa98xxGetErrorString(error));
    }
#if !(defined(WIN32) || defined(_X64))
    if (gTfaRun_timingVerbose)
        PRINT("i2c bytes transferred:%d (%d writes, %d reads)\n",
                gNXP_i2c_writes+gNXP_i2c_reads, gNXP_i2c_writes, gNXP_i2c_reads);

    gNXP_i2c_writes = gNXP_i2c_reads =0;
#endif

  } while (loopcount>0) ;

  if ( gCmdLine.save_given)
  {
    error = tfa98xxSaveFileWrapper(handlesIn, gCmdLine.save_arg);
    if ( error == 0 ) {
        PRINT("Save file %s\n", gCmdLine.save_arg);
    } else {
        PRINT("Unknown file type given %s\n", gCmdLine.save_arg);
        PRINT("last status:%s\n", nxpTfa98xxGetErrorString(tfa_srv_api_error_BadParam));
        return tfa_srv_api_error_BadParam;
    }
  }
  return error;
}

/*
 *
 */
int cliTargetDevice(char *devname)
{
    int fd;

    TRACEIN(__FUNCTION__);

    fd = lxScriboRegister(devname);

    if (fd < 0) {
        PRINT("Can't open %s\n", devname);
        exit(1);
    }

    return fd;

    TRACEOUT(__FUNCTION__);

    //return fd or die
}


