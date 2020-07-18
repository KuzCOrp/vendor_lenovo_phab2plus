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
#if defined(WIN32) || defined(_X64)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <ctype.h>
#include <stdint.h>
#include "cmdline.h"
#include "climax.h"
#include "dbgprint.h"
#include "nxpTfa98xx.h"
#include "lxScribo.h"
#include "tfa98xxLiveData.h"
#include "tfa98xxRuntime.h"
#include "tfaContainer.h"
#include "NXP_I2C.h"  /* for trace */
/*
 * globals
 */
struct gengetopt_args_info gCmdLine; /* Globally visible command line args */

/*
 * module globals for output control
 */
int cli_verbose=0;    /* verbose flag */
int cli_trace=0 ;    /* message trace flag from bb_ctrl */
int cli_quiet=0;    /* don't print too much */

int socket_verbose = 0;
void i2cserver_set_verbose(int val);/* used in i2cserver.c */

#if defined(WIN32) || defined(_X64)
extern int optind;   // processed option count
#endif

int lxScriboListenSocketInit(char *);
void CmdProcess(void* , int );

/*
 * translate string to uppercase
 *  note:  no toupper() in android
 */
static void to_upper(char *str) {
    while (*str!='\0') {
        if (*str  >= 'a' && *str <= 'z') {
            *str = ('A' + *str - 'a');
            str++;
        }
        else
            str++;
    }
}
/**
 * command line arguments to control the main state of the Speakerboost
 * state of the system.
 *
 *  usage: climax start [profile] [vstep]
 *             climax stop
 *
 * @param argc argument count
 * @param *argv[] arguments
 * @return 0 if ok
 */
Tfa98xx_Error_t tfaRunWriteBitfield(Tfa98xx_handle_t handle,  nxpTfaBitfield_t bf) ;

int climax_args(int argc, char *argv[]) {
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;
    nxpTfaBitfield_t bf;
    char *p;
    int i;

    if (argc == 0)
        return 0;

    for(i=0; i<argc; i++) {
        p = strchr(argv[i],'=');

        if (p) {
            *p++='\0';
            bf.value=atoi(p);
        }

        to_upper(argv[i]);
        bf.field = tfaContBfEnum(argv[i]);
        if ( bf.field !=0xffff) {
            if (p) { /* write */
                if (cli_verbose)
                    PRINT("0x%04x=%d\n", bf.field, bf.value);
                err = tfa98xx_writebf(bf);
            } else { /* read */
                err = tfaConfDumpBf(bf);
            }
        }
        PRINT("\n");
    }
    return  !( err == Tfa98xx_Error_Ok);
}
/*
 * functions called from scribo telnet
 */
int climain(int argc, char *argv[]);
int cliload(char *name) {
    return tfa98xx_cnt_loadfile(name, 0);
}
int main(int argc, char *argv[]) {
    Tfa98xx_handle_t handlesIn[] ={-1, -1}; //default one device
    return climain(argc, argv);
}
int climain(int argc, char *argv[])
{
    char *devicename, *xarg, *cntname=0;
    int profile=0, volume=0,vsteps[4]={0,0,0,0};
    int status, fd, i=0;
    Tfa98xx_handle_t handlesIn[] ={-1, -1}; //default one device
    int maxdev, error=0;
    int cnt_verbose = 0;
    char *output_arg_stdout = "stdout";

    tfa98xxI2cSlave = TFA_I2CSLAVEBASE; /* use a default just in case */
    devicename = cliInit(argc, argv);

    if ( gCmdLine.maximus_given ) {
            tfa_cont_dev_type(gCmdLine.maximus_arg);
    } else
            tfa_cont_dev_type(1);

    if ( gCmdLine.ini2cnt_given ) {
        tfaContIni2Container(gCmdLine.ini2cnt_arg);
        return 0;
    }

    if ( gCmdLine.bin2hdr_given ) {
        xarg = argv[optind]; // this is the remaining argv
        if(xarg) {
            tfaContBin2Hdr(gCmdLine.bin2hdr_arg, argc-optind, &argv[optind]);
            return 0;
        } else {
            PRINT_ERROR("Please specify the header arguments.\n");
            return 1;
        }
    }

    /*
     * load container
     */
    if ( gCmdLine.load_given) {
        /* container file contents should ONLY be printed
         * when loading container with verbose is given. Not all the time
         * when verbose_given in combination with other commands!
         */
        if ( (argc == 4) && gCmdLine.verbose_given )
            cnt_verbose = 1;
        if (!tfa98xx_cnt_loadfile(gCmdLine.load_arg, cnt_verbose) )  { // read params
            PRINT_ERROR("Load container failed\n");
            //error = tfa_srv_api_error_BadParam;
            return 1;
        }

        if(cli_verbose)
            PRINT("container:%s\n", gCmdLine.load_arg);

        tfaContGetSlave(0, &tfa98xxI2cSlave); // set 1st slave
    }

    if ( gCmdLine.profile_given ) {
        if ( tfa98xx_cnt_max_device() == -1) {
            PRINT("Please supply a container file with profile argument.\n");
            return 0;
        } else {
            profile = gCmdLine.profile_arg;
            for (i=0; i < tfa98xx_cnt_max_device(); i++ ) {
                if ( profile < 0 || profile >= tfaContMaxProfile(i) ) {
                    PRINT("Incorrect profile given for device %s. The value must be [0 - %d].\n", tfaContDeviceName(i), (tfaContMaxProfile(i)-1));
                    error = tfa_srv_api_error_BadParam;
                }
            }
            if (error != tfa_srv_api_error_Ok) {    // error with profile number so exit
                PRINT("last status: %d (%s)\n", error, nxpTfa98xxGetErrorString(error));
                return error;
            }
        }
    }

    if ( gCmdLine.slave_given ) {
        tfa98xxI2cSlave = gCmdLine.slave_arg;
    }
    tfa98xxI2cSetSlave(tfa98xxI2cSlave);
    if ( cli_verbose ) {
        if ( tfa98xx_cnt_max_device() == -1) {
            PRINT("devicename=%s, i2c=0x%02x\n" ,devicename, tfa98xxI2cSlave); //TODO use lxScriboGetName() after register
        } else {
            PRINT("devicename=%s\n", devicename);
            for (i=0; i < tfa98xx_cnt_max_device(); i++ ) {
                tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                PRINT("\tFound Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
            }
        }
    }

    // split the container file into individual files
    if ( gCmdLine.splitparms_given) {
        if ( tfa98xx_cnt_max_device() == -1) {
            PRINT("Please supply a container file with profile argument.\n");
            return error;
        } else {
            error = tfa98xx_cnt_split(gCmdLine.load_arg);
            if (error != tfa_srv_api_error_Ok) {    // error with profile number so exit
                PRINT("last status: %d (%s)\n", error, nxpTfa98xxGetErrorString(error));
                return error;
            }
        }
    }

    fd = cliTargetDevice(devicename);

    tfaLiveDataVerbose(cli_verbose);

    if ( gCmdLine.start_given ) {
        if ( tfa98xx_cnt_max_device() == -1) {
            PRINT("Please provide container file for this option.\n");
            return error;
        } else {
            error = tfa98xx_start(profile, vsteps, tfa98xx_cnt_max_device());
        }
        if (error != tfa_srv_api_error_Ok) {    // error with profile number so exit
            PRINT("last status: %d (%s)\n", error, nxpTfa98xxGetErrorString(error));
            return error;
        }
    }

    if (gCmdLine.stop_given) {
        if (cli_verbose)
            PRINT("Stop given\n");
        return tfa98xx_stop();
    }

    status = cliCommands(fd, argv[optind], handlesIn);    // execute the command options first

    /************************** main start/stop ***************************/
    status = climax_args(argc-optind, &argv[optind]);

    if ( status ){
        PRINT("error from args : %d\n", status);
    }
    /************************** main start/stop done *********************/

    if ( gCmdLine.record_given ) {  // TODO run in thread
        int loopcount=gCmdLine.count_arg;

        maxdev=gCmdLine.slave_given ? 1 : tfa98xx_cnt_max_device(); /* max dev only if no slave option */
        if ( maxdev <= 0 ) {
            PRINT("Please provide slave address or container file.\n");
            return 0;
        } else {
            for (i=0; i < maxdev; i++ ) {
                if (gCmdLine.slave_given) {
                    tfa98xxI2cSlave=gCmdLine.slave_arg;
                    tfa98xxI2cSetSlave(tfa98xxI2cSlave);
                } else {
                    tfaContGetSlave(i, &tfa98xxI2cSlave); /* get device I2C address */
                    PRINT("Found Device[%d]: %s at 0x%02x.\n", i, tfaContDeviceName(i), tfa98xxI2cSlave);
                }
                if( handlesIn[i] == -1) //TODO properly handle open/close dev
                    error = Tfa98xx_Open(tfa98xxI2cSlave*2, &handlesIn[i]);

                if (error != tfa_srv_api_error_Ok) {    // error with profile number so exit
                    PRINT("last status: %d (%s)\n", error, nxpTfa98xxGetErrorString(error));
                    return error;
                }
            }

            if (gCmdLine.output_given)
                tfa98xxPrintRecordHeader(handlesIn, gCmdLine.output_arg, gCmdLine.record_arg);
            else
                tfa98xxPrintRecordHeader(handlesIn, output_arg_stdout, gCmdLine.record_arg);
             do {
                for (i=0; i < maxdev; i++ ) {
                    if (!gCmdLine.slave_given)
                        tfaContGetSlave(i, &tfa98xxI2cSlave);

                    if ( gCmdLine.output_given)
                        tfa98xxPrintRecord(handlesIn, i, gCmdLine.output_arg, tfa98xxI2cSlave);
                    else
                        tfa98xxPrintRecord(handlesIn, i, output_arg_stdout, tfa98xxI2cSlave);
                }
                loopcount = ( gCmdLine.count_arg == 0) ? 1 : loopcount-1 ;
                tfaRun_Sleepus(1000*gCmdLine.record_arg); // is msinterval
            } while (loopcount>0) ;
        }
        if ( gCmdLine.output_given) {
            PRINT("written to %s\n", gCmdLine.output_arg);
        }
    }

    if (gCmdLine.logger_given) {
        // call with interval and count
        tfa98xxLogger( gCmdLine.logger_arg, gCmdLine.count_arg);
    }

#if !(defined(WIN32) || defined(_X64))
    if ( gCmdLine.server_given ) {
       // PRINT("statusreg:0x%02x\n", tfa98xxReadRegister(0,handlesIn)); // read to ensure device is opened
        cliSocketServer(gCmdLine.server_arg); // note socket is ascii string
    }
    if ( gCmdLine.client_given ) {
        //PRINT("statusreg:0x%02x\n", tfa98xxReadRegister(0,handlesIn)); // read to ensure device is opened
        cliClientServer(gCmdLine.client_arg); // note socket is ascii string
    }
#endif
    if ( strcmp(argv[0], "server"))
        exit (status); // normal
    else
        return status;
}

#if !(defined(WIN32) || defined(_X64))
/*
 *
 */
int activeSocket; // global
void cliSocketServer(char *socket)
{
     int length, i;
     uint8_t cmd[2], buf[256], *ptr, *devname;

    activeSocket=lxScriboListenSocketInit(socket);

    if(activeSocket<0) {
        PRINT_ERROR("something wrong with socket %s\n", socket);
        lxScriboSocketExit(1);
    }

    while(1){
        length = read(activeSocket, buf, 256);
        if (socket_verbose & (length>0)) {
            PRINT("recv: ");
            for(i=0;i<length;i++)
                 PRINT("0x%02x ", buf[i]);
            PRINT("\n");
        }
        if (length>0)
          CmdProcess(buf,  length);
        else {
            close(activeSocket);
            tfaRun_Sleepus(10000);
            activeSocket=lxScriboListenSocketInit(socket);
        }
    }

}

/*
 *
 */

void cliClientServer(char *server)
{
     int length, i;
     uint8_t cmd[2], buf[256], *ptr, *devname;

    activeSocket=lxScriboSocketInit(server);

    if(activeSocket<0) {
        PRINT_ERROR("something wrong with client %s\n", server);
        exit(1);
    }

    while(1){
        length = read(activeSocket, buf, 256);
        if (socket_verbose & (length>0)) {
            PRINT("recv: ");
            for(i=0;i<length;i++)
                 PRINT("0x%02x ", buf[i]);
            PRINT("\n");
        }
        if (length>0)
          CmdProcess(buf,  length);
        else {
            close(activeSocket);
            tfaRun_Sleepus(10000);
            activeSocket=lxScriboSocketInit(server);
        }
    }
}
#endif
/*
 * init the gengetopt stuff
 */
char *cliInit(int argc, char **argv)
{
    char *devicename;
    int lxScribo_verbose=0;

    cmdline_parser (argc, argv, &gCmdLine);
    if(argc==1) // nothing on cmdline
    {
            cmdline_parser_print_help();
            exit(1);
    }
    // extra command line arg for test settings
    // argv[optind]; // this is the remaining argv
    //  lxDummyArg is now passed via the devname: e.g. -ddummy97,warm

    // generic flags
    if (gCmdLine.verbose_given) {
        cli_verbose= 1;
        lxScribo_verbose      =  (1 & gCmdLine.verbose_arg)!=0;
        socket_verbose           =  (2 & gCmdLine.verbose_arg)!=0;
#if !(defined(WIN32) || defined(_X64)) // no --server in windows
        i2cserver_set_verbose ( (4 & gCmdLine.verbose_arg)!=0);
#endif
        gTfaRun_timingVerbose =  (8 & gCmdLine.verbose_arg)!=0;
        cli_trace =                  (0x10 & gCmdLine.verbose_arg)!=0;
        tfaRunVerbose(cli_verbose);
        lxScriboVerbose(lxScribo_verbose);
        tfa_cnt_verbose(cli_verbose);
        lxDummyVerbose((0x10 & gCmdLine.verbose_arg)!=0);
    }
    tfa98xx_verbose = cli_verbose;
    tfa98xx_quiet = gCmdLine.quiet_given;
    tfa_cnt_verbose(tfa98xx_verbose);
    tfa_cont_write_verbose(tfa98xx_verbose);

       NXP_I2C_Trace_file(gCmdLine.trace_arg); /* if 0 stdout will be used */
       NXP_I2C_Trace(gCmdLine.trace_given );   /* if file is open it will be used */

    cli_quiet=gCmdLine.quiet_given;

    if (gCmdLine.device_given)
        devicename = gCmdLine.device_arg;
    else
        devicename = TFA_I2CDEVICE;

    return devicename;

}
