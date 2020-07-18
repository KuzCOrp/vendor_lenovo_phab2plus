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



/*
 * dummy maximus i2c sandbox
 */

/*
 * include files
 */
#include <stdio.h>
#if !(defined(WIN32) || defined(_X64))
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
/* TFA98xx API */
#include "dbgprint.h"
#include "Tfa98API.h"
#include "Tfa98xx.h"
#include "Tfa98xx_internals.h"
#include "Tfa98xx_Registers.h"
#include "lxScribo.h"
#include "NXP_I2C.h" /* for the error codes */

/* printf higher level function activity like patch version, reset dsp params */
#if defined (WIN32) || defined(_X64)
#define FUNC_TRACE printf
#else
#define FUNC_TRACE(va...) PRINT("dummy: " va)
#endif
static const char *fsname[]={"48k","44.1k","32k","24k","22.05k","16k","12k","11.0025k","8k"};

#define TFA_XMEM_PATCHVERSION 0x12bf
#define TFA9897_XMEM_PATCHVERSION 0x0d7f

/******************************************************************************
 * globals
 */
typedef enum dummyType {
    tfa9887,
    tfa9890,
    tfa9891,
    tfa9895,
    tfa9887b,
    tfa9897,
} dummyType_t;
static const char *typeNames[] = {
    "tfa9887",
    "tfa9890",
    "tfa9891",
    "tfa9895",
    "tfa9887b",
    "tfa9897"
};

void convertBytes2Data24(int num_bytes, const unsigned char bytes[],
                   int data[]);
/* globals */
static int dummy_warm = 0; // can be set via "warm" argument
static int lxDummy_verbose = 0;
int lxDummyFailTest;        /* nr of the test to fail */

/* the following replaces the former "extern regdef_t regdefs[];"
 *  TODO replace this by a something generated from the regmap
 */
/* *INDENT-OFF* */
static regdef_t regdefs[] = {
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
        { 0x73, 0x0000, 0xffff, "cf_status"},
        { 0x80, 0x0, 0, "mtp"},
        { 0x83, 0x0, 0, "mtp_re0"},
        { 0xff, 0,   0, NULL},
};
static regdef_t tdm_regdefs[] = {
        { 0x10, 0x0220, 0, "tdm_config_reg0"},
        { 0x11, 0xc1f1, 0, "tdm_config_reg1"},
        { 0x12, 0x0020, 0, "tdm_config_reg2"},
        { 0x13, 0x0000, 0, "tdm_config_reg3"},
        { 0x14, 0x2000, 0, "tdm_config_reg4"},
        { 0x15, 0x0000, 0, "tdm_status_reg"},
        { 0xff, 0,0, NULL}
};

static regdef_t int_regdefs[] = {
        { 0x20, 0x0000, 0, "int_out0"},
        { 0x21, 0x0000, 0, "int_out1"},
        { 0x22, 0x0000, 0, "int_out2"},
        { 0x23, 0x0000, 0, "int_in0"},
        { 0x24, 0x0000, 0, "int_in1"},
        { 0x25, 0x0000, 0, "int_in2"},
        { 0x26, 0x0001, 0, "int_ena0"},
        { 0x27, 0x0000, 0, "int_ena1"},
        { 0x28, 0x0000, 0, "int_ena2"},
        { 0x29, 0xf5e2, 0, "int_pol0"},
        { 0x2a, 0xfc2f,  0, "int_pol1"},
        { 0x2b, 0x0003, 0, "int_pol2"},
        { 0xff, 0,0, NULL}
};

/* *INDENT-ON* */
/*
 * for debugging
 */
int dummy_trace = 0;
/******************************************************************************
 * macros
 */

#define DUMMYVERBOSE if (lxDummy_verbose)

#if (defined(WIN32) || defined(_X64))
void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

float roundf(float x)
{
    return (float)(int)x;
}
#endif

/* endian swap */
#define BE2LEW(x)   (( ( (x) << 8 ) | ( (x) & 0xFF00 ) >> 8 )&0xFFFF)
#define BE2LEDW( x)  (\
           ((x) << 24) \
         | (( (x) & 0x0000FF00) << 8 ) \
         | (( (x) & 0x00FF0000) >> 8 ) \
         | ((x) >> 24) \
         )

/******************************************************************************
 * module globals
 */
#define MAX_DUMMIES 4
/*
 * device structure that keeps the state for each individual target
 */
struct dummy_device{
    dummyType_t type;// = tfa9887;
    int config_length;
    uint8_t slave; //dev[thisdev].slave
    uint16_t Reg[256];
    uint8_t currentreg;        /* active register */
    int xmem_patch_version    ; /* highest xmem address */
#define CF_PATCHMEM_START                (1024 * 16)
#define CF_PATCHMEM_LENGTH            512
#define CF_XMEM_START                        0
#define CF_XMEM_LENGTH                    4800    /* 7008 */
#define CF_XMEMROM_START                8192
#define CF_XMEMROM_LENGTH            2048
#define CF_YMEM_START                        0
#define CF_YMEM_LENGTH                    512
#define CF_YMEMROM_START                2048
#define CF_YMEMROM_LENGTH            1536
#define CF_IO_PATCH_START                    768
#define CF_IO_PATCH_LENGTH                40
#define CF_IO_CF_CONTROL_REG            0x8100
    uint8_t pmem[CF_PATCHMEM_LENGTH * 4];
    uint8_t ymem[CF_YMEM_LENGTH * 3];
    uint8_t iomem[CF_IO_PATCH_LENGTH * 3];
    uint8_t xmem[(CF_XMEMROM_START + CF_XMEMROM_LENGTH) * 3];    /* TODO treat xmemrom differently */
    int memIdx;        /* set via cf mad */
    uint16_t intack_sum; // bits for err and ack bits from TFA98XX_CF_STATUS
//    struct _intreg {
//        uint16_t out[3];
//        uint16_t in[3];
//        uint16_t ena[3];
//        uint16_t pol[3];
//    } intreg;
    uint8_t fracdelaytable[3*9];
    int irqpin;
} ;
static struct dummy_device dev[MAX_DUMMIES]; // max
static int thisdev=0;
/*  */
 /* - I2C bus/slave handling code */
/*  */

static int i2cWrite(int length, const uint8_t *data);
static int i2cRead(int length, uint8_t *data);
/*  */
/* - TFA registers read/write */
/*  */
static void resetRegs(dummyType_t type);
static void setRomid(dummyType_t type);
static int tfaRead(uint8_t *data);
static int tfaWrite(const uint8_t *data);
/*  */
/* - CoolFlux subsystem, mainly dev[thisdev].xmem, read and write */
/*  */

static int lxDummyMemr(int type, uint8_t *data);
static int lxDummyMemw(int type, const uint8_t *data);
static int isClockOn(int thisdev); /* True if CF is ok and running */
/*  */
/* - DSP RPC interaction response */
/*  */
static int setDspParamsSpeakerboost(int param);    /* speakerboost */
static int setDspParamsFrameWork(int param);
static int setDspParamsEq(int param);
static int setDspParamsRe0(int param);
static int getStateInfo(void);
static void makeStateInfo(float agcGain, float limGain, float sMax,
              int T, int statusFlag, float X1, float X2, float Re, int shortOnMips);
/* - function emulations */
static int updateInterrupt(int thisdev);

 /* - utility and helper functions */
static int setInputFile(char *file);
static void hexdump(int num_write_bytes, const unsigned char *data);
static void convert24Data2Bytes(int num_data, unsigned char bytes[],
                int data[]);
/*  */
/******************************************************************************
 *  - static default parameter array defs
 */
#define STATE_SIZE             9  // in words
#define STATE_SIZE_DRC            (2* 9)
static unsigned char stateInfo[STATE_SIZE*3];
static unsigned char stateInfoDrc[STATE_SIZE_DRC*3];
/* life data models */
/* ls model (0x86) */
static unsigned char lsmodel[423];
/* ls model (0xc1) */
static unsigned char lsmodelw[423];

//This can be used to always get a "real" dumpmodel=x for the dummy. It is created from a real dumpmodel=x
static unsigned char static_x_model[423] =
{
  0xff, 0xff, 0x53, 0x00, 0x00, 0x42, 0xff, 0xff, 0x93, 0x00, 0x00, 0x1b,
  0xff, 0xff, 0x1d, 0xff, 0xff, 0xef, 0xff, 0xff, 0x55, 0xff, 0xff, 0xca,
  0xff, 0xff, 0xee, 0x00, 0x00, 0x39, 0xff, 0xff, 0x49, 0x00, 0x00, 0x51,
  0xff, 0xff, 0x13, 0xff, 0xff, 0x8a, 0xff, 0xfe, 0xd5, 0xff, 0xff, 0x93,
  0xff, 0xff, 0x09, 0xff, 0xff, 0x00, 0xff, 0xff, 0x7e, 0xff, 0xfe, 0xf3,
  0xff, 0xff, 0x01, 0xff, 0xfe, 0xaa, 0xff, 0xff, 0x0c, 0xff, 0xfe, 0x55,
  0xff, 0xfe, 0x80, 0xff, 0xfe, 0x24, 0xff, 0xfe, 0xad, 0xff, 0xfe, 0x60,
  0xff, 0xfe, 0x36, 0xff, 0xfe, 0xd4, 0xff, 0xfd, 0xee, 0xff, 0xfe, 0x5d,
  0xff, 0xfd, 0x75, 0xff, 0xfe, 0x45, 0xff, 0xfd, 0x2e, 0xff, 0xfd, 0xd3,
  0xff, 0xfd, 0x4c, 0xff, 0xfe, 0x26, 0xff, 0xfc, 0xc9, 0xff, 0xfd, 0x79,
  0xff, 0xfd, 0x07, 0xff, 0xfd, 0x06, 0xff, 0xfd, 0x91, 0xff, 0xfc, 0x6f,
  0xff, 0xfd, 0x26, 0xff, 0xfc, 0x96, 0xff, 0xfd, 0x02, 0xff, 0xfd, 0x18,
  0xff, 0xfd, 0x23, 0xff, 0xfd, 0x1a, 0xff, 0xfc, 0x86, 0xff, 0xfc, 0x9b,
  0xff, 0xfc, 0xb1, 0xff, 0xfc, 0x30, 0xff, 0xfd, 0x35, 0xff, 0xfc, 0x02,
  0xff, 0xfd, 0x40, 0xff, 0xfc, 0xca, 0xff, 0xfd, 0x47, 0xff, 0xfc, 0xe9,
  0xff, 0xfd, 0x23, 0xff, 0xfc, 0x64, 0xff, 0xfc, 0x6a, 0xff, 0xfc, 0xda,
  0xff, 0xfc, 0x78, 0xff, 0xfd, 0xda, 0xff, 0xfd, 0x7c, 0xff, 0xfe, 0x3a,
  0xff, 0xfd, 0x87, 0xff, 0xfd, 0xd7, 0xff, 0xfb, 0x7c, 0xff, 0xfd, 0xd1,
  0xff, 0xfc, 0x1b, 0xff, 0xfe, 0xa8, 0xff, 0xfe, 0x6c, 0xff, 0xfe, 0x7c,
  0xff, 0xff, 0x3a, 0xff, 0xfd, 0x21, 0xff, 0xfd, 0xcc, 0xff, 0xfb, 0x1e,
  0xff, 0xfd, 0x0e, 0xff, 0xfa, 0xd4, 0xff, 0xff, 0xc3, 0xff, 0xfe, 0x64,
  0x00, 0x02, 0x25, 0xff, 0xff, 0x1a, 0xff, 0xfe, 0x54, 0xff, 0xfb, 0x0f,
  0xff, 0xf6, 0xa3, 0xff, 0xf9, 0xa7, 0xff, 0xf7, 0x3f, 0x00, 0x03, 0x9b,
  0xff, 0xfd, 0xce, 0x00, 0x0f, 0x23, 0xff, 0xfa, 0x16, 0x00, 0x08, 0x02,
  0xff, 0xe4, 0x4c, 0xff, 0xf7, 0x42, 0xff, 0xd6, 0xdb, 0x00, 0x04, 0x90,
  0xff, 0xf4, 0xa6, 0x00, 0x31, 0x89, 0x00, 0x13, 0x72, 0x00, 0x2f, 0xe2,
  0xff, 0xe4, 0x6b, 0xff, 0xd8, 0x43, 0xff, 0x93, 0x85, 0xff, 0xad, 0xef,
  0xff, 0xd4, 0xeb, 0x00, 0x39, 0x16, 0x00, 0xa0, 0x8c, 0x00, 0xb0, 0xdf,
  0x00, 0x9f, 0xe0, 0xff, 0xaa, 0x10, 0xff, 0xb9, 0x67, 0x06, 0xfa, 0x73,
  0xff, 0xb4, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static Tfa98xx_Config_t lastConfig;
static Tfa98xx_Preset_t lastPreset;
static Tfa98xx_SpeakerParameters_t lastSpeaker;

/******************************************************************************
 * HAL interface called via Scribo registered functions
 */
int lxDummyWriteRead(int fd, int NrOfWriteBytes, const uint8_t *WriteData,
             int NrOfReadBytes, uint8_t *ReadData, uint32_t *pError)
{
    int length;
    fd = 0; /* Remove unreferenced formal parameter warning */

    *pError = NXP_I2C_Ok;
    /* there's always a write */
    length = i2cWrite(NrOfWriteBytes, WriteData);
    /* and maybe a read */
    if ((NrOfReadBytes != 0) && (length != 0)) {
        length = i2cRead(NrOfReadBytes, ReadData);
    }
    if (length == 0) {
        PRINT_ERROR("lxDummy slave error\n");
        *pError = NXP_I2C_NoAck;
    }

    return length;

/* if (WriteData[0] !=  (tfa98xxI2cSlave<<1)) { */
/* PRINT("wrong slave 0x%02x iso 0x%02x\n", WriteData[0]>>1, tfa98xxI2cSlave); */
/* //      return 0; */
/* } */
}

int lxDummyWrite(int fd, int size, uint8_t *buffer, uint32_t *pError)
{
    return lxDummyWriteRead(fd, size, buffer, 0, NULL, pError);
}

int lxDummyVersion(char *buffer, int fd)
{
        return 1;
}

/*
 * set calibration done
 */
static void set_caldone()
{
    if(dev[thisdev].Reg[TFA98XX_MTP & TFA98XX_MTP_MTPOTC_MSK])
        dev[thisdev].Reg[TFA98XX_MTP] |= TFA98XX_MTP_MTPEX_MSK;

    dev[thisdev].xmem[231 * 3] = 1;    /* calibration done */
    dev[thisdev].Reg[TFA98XX_STATUSREG] &= ~(TFA98XX_STATUSREG_ACS);    /* clear coldstart */
    dev[thisdev].Reg[0x83] = 0x0710;    /* dummy re0 value */
}

void  lxDummyVerbose(int level) {
    lxDummy_verbose = level;
    DUMMYVERBOSE PRINT("dummy verbose on\n");
}

/*
 * the input lxDummyArg is from the -d<lxDummyArg> global
 * the file is the last argument  option
 */
int lxDummyInit(char *file)
{
    int type;
    char *lxDummyArg;        /* extra command line arg for test settings */
    dev[0].slave = 0x34;
    dev[1].slave = 0x35;
    dev[2].slave = 0x36; //default
    dev[3].slave = 0x37;

    if (file) {
    lxDummyArg = strchr(file, ','); /* the extra arg is after the comma */

    if (lxDummyArg) {
        lxDummyArg++; /* skip the comma */
        if (strcmp(lxDummyArg, "warm")==0)
            dummy_warm=1;
        else if (!setInputFile(lxDummyArg))    /* if filename use it */
        {
            lxDummyFailTest = atoi(lxDummyArg);
        }
    }
    DUMMYVERBOSE PRINT("arg: %s\n", lxDummyArg);
    } else {
        PRINT("%s: called with NULL arg\n", __FUNCTION__);
    }

    for(thisdev=0;thisdev<MAX_DUMMIES;thisdev++ ){
        dev[thisdev].xmem_patch_version = TFA_XMEM_PATCHVERSION; /* all except 97 */
        if (sscanf(file, "dummy%x", &type)) {
            switch (type) {
            case 0x90:
                dev[thisdev].type = tfa9890;
                dev[thisdev].config_length = 165;
                break;
            case 0x91:
                dev[thisdev].type = tfa9891;
                dev[thisdev].config_length = 165;
                break;
            case 0x87:
                dev[thisdev].type = tfa9887;
                dev[thisdev].config_length = 165;
                break;
            case 0x95:
                dev[thisdev].type = tfa9895;
                dev[thisdev].config_length = 201;
                break;
            case 0x87b:
                dev[thisdev].type = tfa9887b;
                dev[thisdev].config_length = 165;
                break;
            case 0x97:
                dev[thisdev].type = tfa9897;
                dev[thisdev].config_length = 201;
                dev[thisdev].xmem_patch_version = TFA9897_XMEM_PATCHVERSION; /* all except 97 */
                break;
            default:
                dev[thisdev].type = tfa9887;
                dev[thisdev].config_length = 165;
                break;
            }
        }
        resetRegs(dev[thisdev].type);
        setRomid(dev[thisdev].type);
    }
    thisdev = 2; //default
    PRINT("%s: running DUMMY i2c, type=%s\n", __FUNCTION__,
           typeNames[dev[thisdev].type]);

    ///* dev[thisdev].Reg[0x00] =  0x091d ;    /* statusreg mtp bit is only on after real pwron */ */
    /* lsmodel default */

    memcpy(lastSpeaker, lsmodel, sizeof(Tfa98xx_SpeakerParameters_t));

    /* fail tests */

    switch (lxDummyFailTest) {
    case 1:
        dev[thisdev].slave = 0;    /* no devices */
        break;
    case 2:
        /*  */
        break;
    case 3:
        /* set error bit */
        dev[thisdev].Reg[0] |= TFA98XX_STATUSREG_OCDS;
        break;
    case 4:
        /* set wrong default */
        dev[thisdev].Reg[TFA98XX_SYS_CTRL] = 0xdead;
        break;
    case 5:
        /* clocks */
        break;
    case 10:
        dev[thisdev].Reg[TFA98XX_BATTERYVOLTAGE] = (uint16_t)(1 / (5.5 / 1024));    /* 1V */
        break;
    case 11:
        /* dev[thisdev].xmem[231*3] = 0; //calibration not done */
        break;
    }
/* dev[thisdev].Reg[0]=0x805f; */
/* dev[thisdev].Reg[0x73]=0x1ff; */

    makeStateInfo(1, 2, 3, 4, 5, 6, 7, 8, 9);

    return (int)dev[thisdev].type;
}

/******************************************************************************
 * I2C bus/slave handling code
 */
int lxDummySetSlaveIdx(uint8_t slave) {
    int i;
    for(i=0;i<MAX_DUMMIES;i++){
        if (dev[i].slave == slave) {
            thisdev=i;
            return i;
        }
    }
    return -1;
}
/*
 * read I2C
 */
static int i2cRead(int length, uint8_t *data)
{
    int idx;

    if (lxDummySetSlaveIdx(data[0] / 2) <0) {
        PRINT_ERROR("dummy: slave read NoAck\n");
        return 0;
    }
    DUMMYVERBOSE PRINT("dummy: slave[%d]=0x%02x\n", thisdev, dev[thisdev].slave);

/* ln =length - 1;  // without slaveaddress */
    idx = 1;
    while (idx < length) {
        idx += tfaRead(&data[idx]);    /* write current and return bytes consumed */
    };

    return length;
}

/*
 * write i2c
 */
static int i2cWrite(int length, const uint8_t *data)
{
    //uint8_t slave;        /*  */
    int idx;

    if (lxDummySetSlaveIdx(data[0] / 2) <0) {
        PRINT_ERROR("dummy: slave read NoAck\n");
        return 0;
    }
    DUMMYVERBOSE PRINT("slave[%d]=0x%02x\n", thisdev, dev[thisdev].slave);

    dev[thisdev].currentreg = data[1];    /* start subaddress */

    /* without slaveaddress and regaddr */
    idx = 2;
    while (idx < length) {
        idx += tfaWrite(&data[idx]);    /* write current and return bytes consumed */
    };

    return length;

}

/******************************************************************************
 * TFA I2C registers read/write
 */
/* reg73 */
/* cf_err[7:0]     8   [7 ..0] 0           cf error flags */
/* reg73 cf_ack[7:0]     8   [15..8] 0           acknowledge of requests (8 channels")" */

#define     CTL_CF_RST_DSP    (0)
#define     CTL_CF_DMEM    (1)
#define     CTL_CF_AIF        (3)
#define    CTL_CF_INT        (4)
#define    CTL_CF_REQ        (5)
#define    STAT_CF_ERR        (0)
#define    STAT_CF_ACK        (8)
#define    CF_PMEM            (0)
#define    CF_XMEM            (1)
#define    CF_YMEM            (2)
#define    CF_IOMEM            (3)

/*
 * in the local register cache the values are stored as little endian,
 *  all processing is done in natural little endianess
 * The i2c data is big endian
 */
/*
 * i2c regs reset to default 9887
 */
static void resetRegs9887(void)
{
    dev[thisdev].Reg[0x00] = 0x081d;    /* statusreg */
    dev[thisdev].Reg[0x03] = 0x0012;    /* revisionnumber */
    dev[thisdev].Reg[0x04] = 0x888b;    /* i2sreg */
    dev[thisdev].Reg[0x05] = 0x13aa;    /* bat_prot */
    dev[thisdev].Reg[0x06] = 0x001f;    /* audio_ctr */
    dev[thisdev].Reg[0x07] = 0x0fe6;    /* dcdcboost */
    dev[thisdev].Reg[0x08] = 0x0800;    /* spkr_calibration */
    dev[thisdev].Reg[0x09] = 0x041d;    /* sys_ctrl */
    dev[thisdev].Reg[0x0a] = 0x3ec3;    /* i2s_sel_reg */
    dev[thisdev].Reg[0x40] = 0x0000;    /* hide_unhide_key */
    dev[thisdev].Reg[0x41] = 0x0000;    /* pwm_control */
    dev[thisdev].Reg[0x4c] = 0x0000;    /* abisttest */
    dev[thisdev].Reg[0x62] = 0x0000;
    dev[thisdev].Reg[0x70] = 0x0000;    /* cf_controls */
    dev[thisdev].Reg[0x71] = 0x0000;    /* cf_mad */
    dev[thisdev].Reg[0x72] = 0x0000;    /* cf_mem */
    dev[thisdev].Reg[0x73] = 0x0000;    /* cf_status */
    dev[thisdev].Reg[0x80] = 0x0000;    /* mpt */
    dev[thisdev].Reg[0x83] = 0x0000;    /* mpt_re0 */
}
/*
 * TODO i2c regs reset to default 9897
 */
static void resetRegs9897(void)
{
    int i;

    dev[thisdev].Reg[0x00] = 0x081d;    /* statusreg */
    dev[thisdev].Reg[0x03] = 0x0b97;    /* revisionnumber */
    dev[thisdev].Reg[0x04] = 0x888b;    /* i2sreg */
    dev[thisdev].Reg[0x05] = 0x13aa;    /* bat_prot */
    dev[thisdev].Reg[0x06] = 0x001f;    /* audio_ctr */
    dev[thisdev].Reg[0x07] = 0x0fe6;    /* dcdcboost */
    dev[thisdev].Reg[0x08] = 0x0800;    /* spkr_calibration */
    dev[thisdev].Reg[0x09] = 0x041d;    /* sys_ctrl */
    dev[thisdev].Reg[0x0a] = 0x3ec3;    /* i2s_sel_reg */
    dev[thisdev].Reg[0x40] = 0x0000;    /* hide_unhide_key */
    dev[thisdev].Reg[0x41] = 0x0000;    /* pwm_control */
    dev[thisdev].Reg[0x4c] = 0x0000;    /* abisttest */
    dev[thisdev].Reg[0x62] = 0x0000;
    dev[thisdev].Reg[0x70] = 0x0000;    /* cf_controls */
    dev[thisdev].Reg[0x71] = 0x0000;    /* cf_mad */
    dev[thisdev].Reg[0x72] = 0x0000;    /* cf_mem */
    dev[thisdev].Reg[0x73] = 0x0000;    /* cf_status */
    dev[thisdev].Reg[0x80] = 0x0000;
    dev[thisdev].Reg[0x83] = 0x0000;    /* mtp_re0 */
    for(i=0;i<(sizeof(tdm_regdefs)/sizeof(regdef_t))-1;i++) {//TFA98XX_TDM_CONFIG_REG0=0x10
        dev[thisdev].Reg[i+TFA98XX_TDM_CONFIG_REG0] = tdm_regdefs[i].pwronDefault;
    }

    for(i=0;i<(sizeof(int_regdefs)/sizeof(regdef_t))-1;i++) {
        dev[thisdev].Reg[int_regdefs[i].offset] = int_regdefs[i].pwronDefault;
    }

}
/*
 *  i2c regs reset to default 9890
 */
static void resetRegs9890(void)
{
    dev[thisdev].Reg[0x00] = 0x0a5d;    /* statusreg */
    dev[thisdev].Reg[0x03] = 0x0080;    /* revisionnumber */
    dev[thisdev].Reg[0x04] = 0x888b;    /* i2sreg */
    dev[thisdev].Reg[0x05] = 0x93a2;    /* bat_prot */
    dev[thisdev].Reg[0x06] = 0x001f;    /* audio_ctr */
    dev[thisdev].Reg[0x07] = 0x8fe6;    /* dcdcboost */
    dev[thisdev].Reg[0x08] = 0x3800;    /* spkr_calibration */
    dev[thisdev].Reg[0x09] = 0x825d;    /* sys_ctrl */
    dev[thisdev].Reg[0x0a] = 0x3ec3;    /* i2s_sel_reg */
    dev[thisdev].Reg[0x40] = 0x0000;    /* hide_unhide_key */
    dev[thisdev].Reg[0x41] = 0x0308;    /* pwm_control */
    dev[thisdev].Reg[0x4c] = 0x0000;    /* abisttest */
    dev[thisdev].Reg[0x62] = 0x0000;    /* mtp_copy */
    dev[thisdev].Reg[0x70] = 0x0000;    /* cf_controls */
    dev[thisdev].Reg[0x71] = 0x0000;    /* cf_mad */
    dev[thisdev].Reg[0x72] = 0x0000;    /* cf_mem */
    dev[thisdev].Reg[0x73] = 0x0000;    /* cf_status */
    dev[thisdev].Reg[0x80] = 0x0000;    /* mtp */
    dev[thisdev].Reg[0x83] = 0x0000;    /* mtp_re0 */
    dev[thisdev].Reg[0x84] = 0x1234;    /* MTP for '90 startup system stable detection */
}

/*
 *  i2c regs reset to default 9890B/9891
 */
static void resetRegs9891(void)
{
    dev[thisdev].Reg[0x00] = 0x0a5d;    /* statusreg */
    dev[thisdev].Reg[0x03] = 0x0091;    /* revisionnumber */
    dev[thisdev].Reg[0x04] = 0x888b;    /* i2sreg */
    dev[thisdev].Reg[0x05] = 0x93a2;    /* bat_prot */
    dev[thisdev].Reg[0x06] = 0x001f;    /* audio_ctr */
    dev[thisdev].Reg[0x07] = 0x8fe6;    /* dcdcboost */
    dev[thisdev].Reg[0x08] = 0x3800;    /* spkr_calibration */
    dev[thisdev].Reg[0x09] = 0x825d;    /* sys_ctrl */
    dev[thisdev].Reg[0x0a] = 0x0000;    /* i2s_pdm_sel_reg */
    dev[thisdev].Reg[0x40] = 0x0000;    /* hide_unhide_key */
    dev[thisdev].Reg[0x41] = 0x0308;    /* pwm_control */
    dev[thisdev].Reg[0x4c] = 0x0000;    /* abisttest */
    dev[thisdev].Reg[0x62] = 0x0000;    /* mtp_copy */
    dev[thisdev].Reg[0x70] = 0x0000;    /* cf_controls */
    dev[thisdev].Reg[0x71] = 0x0000;    /* cf_mad */
    dev[thisdev].Reg[0x72] = 0x0000;    /* cf_mem */
    dev[thisdev].Reg[0x73] = 0x0000;    /* cf_status */
    dev[thisdev].Reg[0x80] = 0x0000;    /* mtp */
    dev[thisdev].Reg[0x83] = 0x0000;    /* mtp_re0 */
    dev[thisdev].Reg[0x84] = 0x1234;    /* MTP for '90 startup system stable detection */
}

/*
 * i2c regs reset to default 9895
 */
static void resetRegs9895(void)
{
    dev[thisdev].Reg[0x00] = 0x081d;    /* statusreg */
    dev[thisdev].Reg[0x01] = 0x3ff;    /* battV clock off */
    dev[thisdev].Reg[0x02] = 0x100;    /* ictemp clock off */
    dev[thisdev].Reg[0x03] = 0x0012;    /* revisionnumber */
    dev[thisdev].Reg[0x04] = 0x888b;    /* i2sreg */
    dev[thisdev].Reg[0x05] = 0x13aa;    /* bat_prot */
    dev[thisdev].Reg[0x06] = 0x001f;    /* audio_ctr */
    dev[thisdev].Reg[0x07] = 0x0fe6;    /* dcdcboost */
    dev[thisdev].Reg[0x08] = 0x0c00;    /* spkr_calibration */
    dev[thisdev].Reg[0x09] = 0x041d;    /* sys_ctrl */
    dev[thisdev].Reg[0x0a] = 0x3ec3;    /* i2s_sel_reg */
    dev[thisdev].Reg[0x40] = 0x0000;    /* hide_unhide_key */
    dev[thisdev].Reg[0x41] = 0x0300;    /* pwm_control */
    dev[thisdev].Reg[0x4c] = 0x0000;    /* abisttest */
    dev[thisdev].Reg[0x62] = 0x5be1;
    dev[thisdev].Reg[0x70] = 0;        /* cf_controls */
    dev[thisdev].Reg[0x71] = 0;        /* cf_mad */
    dev[thisdev].Reg[0x72] = 0x0000;    /* cf_mem */
    dev[thisdev].Reg[0x73] = 0x0000;    /* cf_status */
    dev[thisdev].Reg[0x80] = 0x0000;
    dev[thisdev].Reg[0x83] = 0x0000;    /* mtp_re0 */
}

/*
 * i2c regs reset to default
 */
static void resetRegs(dummyType_t type)
{
    /* preserve acs state */
    uint16_t acs = dev[thisdev].Reg[TFA98XX_STATUSREG] & TFA98XX_STATUSREG_ACS_MSK;

    dev[thisdev].Reg[TFA98XX_BATTERYVOLTAGE] = 0x3ff; //when clock is off
    switch (type) {
    case tfa9887:
        resetRegs9887();
        break;
    case tfa9890:
        resetRegs9890();
        break;
    case tfa9891:
        resetRegs9891();
        break;
    case tfa9895:
    case tfa9887b:
        resetRegs9895();
        break;
    case tfa9897:
        resetRegs9897();
        break;
    default:
        PRINT("dummy: %s, unknown type %d\n", __FUNCTION__, type);
        break;
    }
    dev[thisdev].Reg[TFA98XX_STATUSREG] |= acs;
    if (dummy_warm) {
        dev[thisdev].Reg[TFA98XX_STATUSREG] = 0x805f;
        //dev[thisdev].Reg[TFA98XX_STATUS_POLARITY_REG1] = 0x27a0;
    }
}

/*
 * return the regname
 */
static const char *getRegname(int reg)
{
    int i;

    for (i = 0; i < 256; i++) {
        if (regdefs[i].name == NULL)
            break;
        if (reg == regdefs[i].offset)
            return regdefs[i].name;
    }
    return "unknown";
}

/*
 * emulation of tfa9887 register read
 */
static int tfaRead(uint8_t *data)
{
    int reglen = 2;        /* default */
    uint8_t reg = dev[thisdev].currentreg;
    uint16_t regval = 0xdead;
    static short temperature = 0;

    switch (reg) {
    case TFA98XX_STATUSREG /*0x00 */:
    case TFA98XX_REVISIONNUMBER /*0x03 */:
    case 4 /*I2SREG 0x04 */:
    case TFA98XX_BAT_PROT /*0x05 */:
    case TFA98XX_AUDIO_CTR /*0x06 */:
    case TFA98XX_DCDCBOOST /*0x07 */:
    case TFA98XX_SPKR_CALIBRATION /*0x08 */:
    case TFA98XX_SYS_CTRL /*0x09 */:
    case TFA98XX_I2S_SEL_REG /*0x0a */:
    case /*TFA98XX_RESERVED_1 */0x0c :
    case 0x40:
    case /*TFA98XX_CURRENTSENSE1 */0x46:
    case /*TFA98XX_CURRENTSENSE2 */0x47:
    case 0x48 :
    case 0x49 :
    case 0x4c :
    case TFA98XX_CF_CONTROLS /*0x70 */:
    case TFA98XX_MTP_CTRL /*(0x62) */:
    case TFA98XX_MTP /*(0x80) */:
    case 0x52:        /* TODO */
    case 0xb:        /* TODO */
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case 0x84:        /* MTP for '90 startup system stable detection */
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case 0x86:        /* MTP */
#define FEATURE1_TCOEF              0x100    /* bit8 set means tCoefA expected */
#define FEATURE1_DRC                0x200    /* bit9 NOT set means DRC expected */
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case TFA98XX_TEMPERATURE /*0x02 */:
        if (temperature++ > 170)
            temperature = -40;
        dev[thisdev].Reg[TFA98XX_TEMPERATURE] = temperature;
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case TFA98XX_BATTERYVOLTAGE /*0x01 */:
        if (lxDummyFailTest == 10)
            dev[thisdev].Reg[TFA98XX_BATTERYVOLTAGE] = (uint16_t)(1 / (5.5 / 1024));    /* 1V */
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case TFA98XX_CF_STATUS /*0x73 */:
        regval = dev[thisdev].Reg[reg];    /* just return */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    case TFA98XX_CF_MEM /*0x72 */:
        if(isClockOn(thisdev))
            reglen =
                    lxDummyMemr((dev[thisdev].Reg[TFA98XX_CF_CONTROLS] >> CTL_CF_DMEM) &
                            0x03, data);
        break;
    case TFA98XX_CF_MAD /*0x71 */:
        regval = dev[thisdev].Reg[reg];    /* just return */
        if (lxDummyFailTest == 2)
            regval = 0xdead;    /* fail test */
        reglen = 2;
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    default:
        DUMMYVERBOSE PRINT("dummy: undefined rd register: 0x%02x\n", reg);
        regval = dev[thisdev].Reg[reg];    /* just return anyway */
        dev[thisdev].currentreg++;    /* autoinc */
        break;
    }

    if (reg != TFA98XX_CF_MEM) {
        DUMMYVERBOSE
            PRINT("0x%02x:0x%04x (%s)\n", reg, regval, getRegname(reg));

        *(uint16_t *) (data) = BE2LEW(regval);    /* return in proper endian */
    }

    return reglen;
}
/*
 * set dsp firmware ack
 */
static void cfAck(int thisdev, enum tfa_fw_event evt) {
    uint16_t oldack=dev[thisdev].Reg[TFA98XX_CF_STATUS];

    dev[thisdev].Reg[TFA98XX_CF_STATUS] |= (1 << (evt + STAT_CF_ACK));

    if ( oldack != dev[thisdev].Reg[TFA98XX_CF_STATUS])
        updateInterrupt(thisdev);
}

static void cfCountboot(int thisdev) {
    if (isClockOn(thisdev)) {
        dev[thisdev].xmem[0xa1 * 3+2]++;        // count_boot (lsb)
        cfAck(thisdev, tfa_fw_reset_start);
    }
}
/*
 * cf control reg (0x70)
 */
static int i2cCfControlReg(uint16_t val)
{
    unsigned char code1, code2;
    uint16_t xor, clearack, negedge, newval, oldval = dev[thisdev].Reg[TFA98XX_CF_CONTROLS];
    int ack=1;
    newval = val;

    /* REQ/ACK bits:
     *  if REQ was high and goes low then clear ACK
     * */
    xor = oldval^newval;
    if ( xor & TFA98XX_CF_CONTROLS_REQ_MSK ) {
        negedge = (oldval & ~newval);
        clearack = negedge;
        dev[thisdev].Reg[TFA98XX_CF_STATUS] &= ~(clearack & TFA98XX_CF_CONTROLS_REQ_MSK);
    }
    // reset transition 1->0 increment count_boot
    if ( dev[thisdev].Reg[TFA98XX_CF_CONTROLS] & TFA98XX_CF_CONTROLS_RST_MSK ) {// reset is on
            if ( (val  & TFA98XX_CF_CONTROLS_RST_MSK) ==0) { //clear it now
                cfCountboot(thisdev);
                FUNC_TRACE("DSP reset release\n");
            }
            else return val; // stay in reset
}
    if ( (val & TFA98XX_CF_CONTROLS_CFINT) &&
         (val & (1<<TFA98XX_CF_CONTROLS_REQ_POS)) )    /* if cfirq and msg req*/
    {
        code1 = dev[thisdev].xmem[4];
        code2 = dev[thisdev].xmem[5];
        if ((code1 == 0x81) ) {    /* MODULE_SPEAKERBOOST */
            setDspParamsSpeakerboost(code2);

        } else if (code1 == 0x80) {    /* MODULE_FRAMEWORK */
            setDspParamsFrameWork(code2);
        } else if (code1 == 0x82) { // EQ
            setDspParamsEq(code2);
        } else if (code1 == 0x89) { // set Re0
            setDspParamsRe0(code2);
        } else
            ack=0;
        if(ack)
            cfAck(thisdev, tfa_fw_i2c_cmd_ack);
    }

    return val;
}
static int updateInterrupt(int thisdev) {
    uint16_t status[3],latch;
    int x,pin=dev[thisdev].irqpin,edge;
    //uint16_t *r;
    uint16_t *out, *in, *ena, *pol;

    if (!isClockOn(thisdev))
        return 0;

    /* get all the input flags */
    status[0] = dev[thisdev].Reg[TFA98XX_STATUSREG];
    status[1] = dev[thisdev].Reg[0x30]; //TODO statusflags need datasheet?
    /* the Ack flags are all or-ed into a seperate reg */
    dev[thisdev].intack_sum = ((dev[thisdev].Reg[TFA98XX_CF_STATUS] & TFA98XX_CF_STATUS_ACK_MSK) !=0) << 1  |
                                              ((dev[thisdev].Reg[TFA98XX_CF_STATUS] & TFA98XX_CF_STATUS_ERR_MSK) !=0 );
    status[2] = dev[thisdev].intack_sum ;

    for(x=0;x<3;x++) {
        out = &dev[thisdev].Reg[TFA98XX_INTERRUPT_OUT_REG1+x];
        in = &dev[thisdev].Reg[TFA98XX_INTERRUPT_IN_REG1+x];
        pol = &dev[thisdev].Reg[TFA98XX_STATUS_POLARITY_REG1+x];
        ena = &dev[thisdev].Reg[TFA98XX_INTERRUPT_ENABLE_REG1+x];
        latch = *out;
        *out= ~(status[x] ^ *pol); // new bits
        //if (x==1) latch |= 8; // FORCE irq test FAIL
        *out |= latch ; //keep old bits
        *out &= ~*in; //clear bits
        *in=0; //can't read in
        if (*out & *ena) {
            dev[thisdev].irqpin = 1;
        }
    }
    dev[thisdev].Reg[TFA98XX_INTERRUPT_OUT_REG3] &= TFA98XX_INTERRUPT_OUT_REG3_INTOACK_MSK;

    edge = dev[thisdev].irqpin != pin; //true if pin changed
    if (edge)
        FUNC_TRACE("IRQ pin %d\n", dev[thisdev].irqpin);
    return edge;
}

/*
 * i2c interrupt registers
 *   abs addr: TFA98XX_INTERRUPT_OUT_REG1 + idx
 */
static uint16_t i2cInterruptReg(int idx, uint16_t wordvalue)
{
    //int x; //sub index
    //uint16_t *out, *in, *ena, *pol;

    if (!isClockOn(thisdev)) {// need clock for the latch
        FUNC_TRACE("interrupt ip needs  amp running in N1A!\n");
        return 0;
    }
        DUMMYVERBOSE PRINT("0x%02x,intreg[%d] < 0x%04x\n", TFA98XX_INTERRUPT_OUT_REG1+idx, idx, wordvalue);
        dev[thisdev].Reg[TFA98XX_INTERRUPT_OUT_REG1+idx] = wordvalue;
//    switch(idx) {
//    case 0:
//    case 1:
//    case 2:
//        x = idx;
//        // out
//        break;
//    case 3:
//    case 4:
//    case 5:
//        x = idx-3;
//        // in
//        break;
//    case 6:
//    case 7:
//    case 8:  //ena
//        x = idx-6;
//        break;
//    case  9:
//    case 10:
//    case 11:        // pol
//        x = idx-9;
//        break;
//    default:
//            PRINT_ERROR("%s: register index:%d out of range\n", __FUNCTION__, idx); //TODO add debug.h printing
//            exit(1);
//        break;
//    }
//    out = &dev[thisdev].Reg[TFA98XX_INTERRUPT_OUT_REG1+x];
//    in = &dev[thisdev].Reg[TFA98XX_INTERRUPT_IN_REG1+x];
//    pol = &dev[thisdev].Reg[TFA98XX_STATUS_POLARITY_REG1+x];
//    ena = &dev[thisdev].Reg[TFA98XX_INTERRUPT_ENABLE_REG1+x];
//
//    switch(idx) {
//    case 0:// out
//    case 1:
//    case 2:/    //can't directly write out *out = ~(status[x] ^ *pol);
//        break;
//    case 3:// in
//    case 4:
//    case 5:
//        *in = wordvalue; //can't read , fix in irqupdate?
//        //in update *out = ~*in;
//        break;
//    case 6:
//    case 7:
//    case 8:  //ena
//        *ena = wordvalue;
//        break;
//    case  9:
//    case 10:
//    case 11:        // pol
//        x = idx-9;
//        *pol = wordvalue;
//        break;
//    default:
//            PRINT_ERROR("%s: register index:%d out of range\n", __FUNCTION__, idx); //TODO add debug.h printing
//            exit(1);
//        break;
//    }
//done from main level    updateInterrupt(thisdev);
    return wordvalue;
}
/*
 * i2c  control reg r9
 */
static uint16_t i2cControlReg(uint16_t wordvalue)
{
    if ( (wordvalue & TFA98XX_SYS_CTRL_I2CR) ) {
        FUNC_TRACE("I2CR reset\n");
        resetRegs(dev[thisdev].type);
        wordvalue = dev[thisdev].Reg[TFA98XX_SYS_CTRL]; //reset value
    }
    /* if PLL input changed */
    if ( (wordvalue ^ dev[thisdev].Reg[TFA98XX_SYS_CTRL]) & TFA98XX_SYS_CTRL_IPLL_MSK) {
        int ipll=(wordvalue & TFA98XX_SYS_CTRL_IPLL_MSK)>>TFA98XX_SYS_CTRL_IPLL_POS;
        FUNC_TRACE("PLL in=%d (%s)\n", ipll, ipll? "fs":"bck");
    }

    if ((wordvalue & (1 << 0)) && !(wordvalue & (1 << 13))) {    /* powerdown=1, i2s input 1 */
        FUNC_TRACE("power off\n");
        dev[thisdev].Reg[TFA98XX_STATUSREG] &=
            ~(TFA98XX_STATUSREG_PLLS | TFA98XX_STATUSREG_CLKS);
        dev[thisdev].Reg[TFA98XX_BATTERYVOLTAGE] = 0;
    } else {
        FUNC_TRACE("power on\n");
        dev[thisdev].Reg[TFA98XX_STATUSREG] |=
            (TFA98XX_STATUSREG_PLLS | TFA98XX_STATUSREG_CLKS);
        dev[thisdev].Reg[TFA98XX_STATUSREG] |= (TFA98XX_STATUSREG_AREFS);
        dev[thisdev].Reg[TFA98XX_BATTERYVOLTAGE] = 0x3b2; //=5.08V R*5.5/1024
    }
    if (wordvalue & TFA98XX_SYS_CTRL_SBSL) {    /* configured */
        FUNC_TRACE("configured\n");
        set_caldone();
    }
    if ( (wordvalue & TFA98XX_SYS_CTRL_AMPE) && !(wordvalue & TFA98XX_SYS_CTRL_CFE)) {    /* assume bypass if AMPE and not CFE */
        FUNC_TRACE("CF by-pass\n");
        dev[thisdev].Reg[0x73] = 0x00ff;// else irq test wil fail
    }

    /* reset with ACS active */
    if(dev[thisdev].Reg[TFA98XX_STATUSREG] & (TFA98XX_STATUSREG_ACS)){
        PRINT("reset with ACS active\n");
                dev[thisdev].xmem[231 * 3] = 0;    /* clear calibration done */
                dev[thisdev].xmem[231 * 3 + 1] = 0;    /* clear calibration done */
                dev[thisdev].xmem[231 * 3 + 2 ] = 0;    /* clear calibration done */
    }

    return wordvalue;
}

/*
 * emulation of tfa9887 register write
 *
 *  write current register , autoincrement  and return bytes consumed
 *
 */
static int tfaWrite(const uint8_t *data)
{
    int reglen = 2;        /* default */
    uint8_t reg = dev[thisdev].currentreg;
    uint16_t wordvalue;

    wordvalue = data[0] << 8 | data[1];
    switch (reg) {
    case TFA98XX_STATUSREG /*0x00 */:
    case TFA98XX_BATTERYVOLTAGE /*0x01 */:
    case TFA98XX_TEMPERATURE /*0x02 */:
    case TFA98XX_REVISIONNUMBER /*0x03 */:
    case TFA98XX_BAT_PROT /*0x05 */:
    case TFA98XX_DCDCBOOST /*0x07 */:
    case TFA98XX_I2S_SEL_REG /*0x0a */:
    case /*TFA98XX_RESERVED_1 */0x0c :
    case 0x40:
    case /*TFA98XX_CURRENTSENSE1 */0x46:
    case /*TFA98XX_CURRENTSENSE2 */0x47:
    case 0x48 :
    case 0x49 :
    case 0x4c :
    case TFA98XX_CF_STATUS /*0x73 */:
    case TFA98XX_MTP_CTRL /*(0x62) */:
    case TFA98XX_MTP /*(0x80) */:
    case 0x52:        /* TODO */
    case 0xb:        /* TODO */
        dev[thisdev].Reg[reg] = wordvalue;
        break;
    case TFA98XX_AUDIO_CTR /*0x06 */:
        /* if volume rate changed */
        if ( (wordvalue ^ dev[thisdev].Reg[reg]) & TFA98XX_AUDIO_CTR_VOL_MSK) {
            int vol=(wordvalue & TFA98XX_AUDIO_CTR_VOL_MSK)>>TFA98XX_AUDIO_CTR_VOL_POS;
            FUNC_TRACE("volume=%d %s\n", vol, vol==0xff?"(softmute)":"");
            if (vol==0xff)
                cfAck(thisdev, tfa_fw_soft_mute_ready);
        }
        dev[thisdev].Reg[reg] = wordvalue;
        break;
    case 4 /*I2SREG 0x04 or Audio control reg for 97 */:
        /* if sample rate changed */
        if ( (wordvalue ^ dev[thisdev].Reg[TFA98XX_I2SREG]) & TFA98XX_I2SREG_I2SSR_MSK) {
            int fs=(wordvalue & TFA98XX_I2SREG_I2SSR_MSK)>>TFA98XX_I2SREG_I2SSR_POS;
            FUNC_TRACE("sample rate=%d (%s)\n", fs, fsname[8-fs]);
        }
        dev[thisdev].Reg[reg] = wordvalue;
        break;
    case TFA98XX_SPKR_CALIBRATION /*0x08 PVP bit */:
        if (dev[thisdev].type == tfa9887)
            dev[thisdev].Reg[reg] = wordvalue;    /* PVP bit is RW */
        else
            dev[thisdev].Reg[reg] = wordvalue | 0x0400;    /* PVP bit is always 1 */
        break;
    case TFA98XX_CF_MAD /*0x71 */:
        dev[thisdev].Reg[reg] = wordvalue;
        dev[thisdev].memIdx = wordvalue * 3;    /* set glbl mem idx */
        break;
    case TFA98XX_CF_CONTROLS /*0x70 */:
        dev[thisdev].Reg[reg] = (uint16_t)i2cCfControlReg(wordvalue);
        break;
    case TFA98XX_CF_MEM /*0x72 */:
        reglen =
            lxDummyMemw((dev[thisdev].Reg[TFA98XX_CF_CONTROLS] >> CTL_CF_DMEM) &
                0x03, data);
        break;
    case TFA98XX_SYS_CTRL /*0x09 */:
        if (lxDummyFailTest != 5) {    /* normal if not fail */
            wordvalue = i2cControlReg(wordvalue);
        }
        dev[thisdev].Reg[reg] = wordvalue;
        break;
    default:
        if ( reg>>4 == 2 ) {
            wordvalue = i2cInterruptReg(reg & 0x0f, wordvalue);
        } else {
            DUMMYVERBOSE PRINT("dummy: undefined wr register: 0x%02x\n",
                    reg);
        }
        dev[thisdev].Reg[reg] = wordvalue;
        break;
    }

    /* all but cf_mem autoinc and 2 bytes */
    if (reg != TFA98XX_CF_MEM) {
        dev[thisdev].currentreg++;    /* autoinc */
        reglen = 2;

        DUMMYVERBOSE
            PRINT("0x%02x<0x%04x (%s)\n", reg, wordvalue,
               getRegname(reg));
    }
    updateInterrupt(thisdev);
    return reglen;
}

/******************************************************************************
 * CoolFlux subsystem, mainly dev[thisdev].xmem, read and write
 */

/*
 * set value returned for the patch load romid check
 */
static void setRomid(dummyType_t type)
{
    if (dummy_warm) {
        set_caldone();
    }
    switch (type) {
    case tfa9887:
        dev[thisdev].xmem[0x2210 * 3] = 0x73;    /* N1D2 */
        dev[thisdev].xmem[0x2210 * 3 + 1] = 0x33;
        dev[thisdev].xmem[0x2210 * 3 + 2] = 0x33;
        break;
    case tfa9890:
    case tfa9891:
        dev[thisdev].xmem[0x20c6 * 3] = 0x00;    /* 90 N1C3 */
        dev[thisdev].xmem[0x20c6 * 3 + 1] = 0x00;
        dev[thisdev].xmem[0x20c6 * 3 + 2] = 0x31; // C2=0x31;
        break;
    case tfa9895:
    case tfa9887b:
        dev[thisdev].xmem[0x21b4 * 3] = 0x00;    /* 95 */
        dev[thisdev].xmem[0x21b4 * 3 + 1] = 0x77;
        dev[thisdev].xmem[0x21b4 * 3 + 2] = 0x9a;
        break;
    case tfa9897:
        /* N1B: 0x22b0=0x000032*/
        dev[thisdev].xmem[0x22B0 * 3] = 0x00;    /* 97N1B */
        dev[thisdev].xmem[0x22B0 * 3 + 1] = 0x00;
        dev[thisdev].xmem[0x22B0 * 3 + 2] = 0x32;
        //n1a
//        dev[thisdev].xmem[0x2286 * 3] = 0x00;    /* 97N1A */
//        dev[thisdev].xmem[0x2286 * 3 + 1] = 0x00;
//        dev[thisdev].xmem[0x2286 * 3 + 2] = 0x33;

        break;
    default:
        PRINT("dummy: %s, unknown type %d\n", __FUNCTION__, type);
        break;
    }
}

/* modules */
#define MODULE_SPEAKERBOOST  1

/* RPC commands */
#define PARAM_SET_LSMODEL        0x06    /* Load a full model into SpeakerBoost. */
#define PARAM_SET_LSMODEL_SEL    0x07    /* Select one of the default models present in Tfa9887 ROM. */
#define PARAM_SET_EQ             0x0A    /* 2 Equaliser Filters. */
#define PARAM_SET_PRESET         0x0D    /* Load a preset */
#define PARAM_SET_CONFIG         0x0E    /* Load a config */

#define PARAM_GET_RE0            0x85    /* gets the speaker calibration impedance (@25 degrees celsius) */
#define PARAM_GET_LSMODEL        0x86    /* Gets current LoudSpeaker Model. */
#define PARAM_GET_LSMODELW       0xC1    /* Gets current LoudSpeaker excursion Model. */
#define PARAM_GET_ALL            0x80    /* read current config and preset. */
#define PARAM_GET_STATE          0xC0
#define PARAM_GET_TAG            0xFF

/* RPC Status results */
#define STATUS_OK                  0
#define STATUS_INVALID_MODULE_ID   2
#define STATUS_INVALID_PARAM_ID    3
#define STATUS_INVALID_INFO_ID     4

static char *cfmemName[] = { "dev[thisdev].pmem", "dev[thisdev].xmem", "dev[thisdev].ymem", "dev[thisdev].iomem" };

/* True if CF is ok and running */
static int isClockOn(int thisdev) {
    return dev[thisdev].Reg[TFA98XX_STATUSREG] & TFA98XX_STATUSREG_CLKS_MSK; /* clks should be enough */
}

/*
 * write to CF memory space
 */
static int lxDummyMemw(int type, const uint8_t *data)
{
    uint8_t *memptr=(uint8_t *)data;
    int idx = dev[thisdev].memIdx;

    switch (type) {
    case CF_PMEM:
        /* dev[thisdev].pmem is 4 bytes */
        idx = (dev[thisdev].memIdx - (dev[thisdev].Reg[0x71] * 3)) / 4;    /* this is the offset */
        idx += CF_PATCHMEM_START;
        DUMMYVERBOSE
            PRINT("W %s[%02d]: 0x%02x 0x%02x 0x%02x 0x%02x\n",
               cfmemName[type], idx, data[0], data[1], data[2],
               data[3]);
        if ((CF_PATCHMEM_START <= idx)
&&(idx < (CF_PATCHMEM_START + CF_PATCHMEM_LENGTH))) {
            memptr = &dev[thisdev].pmem[(idx - CF_PATCHMEM_START) * 4];
            dev[thisdev].memIdx += 4;
            *memptr++ = *data++;
            *memptr++ = *data++;
            *memptr++ = *data++;
            *memptr++ = *data++;
            return 4;

        } else {
            PRINT("dummy: dev[thisdev].pmem[%d] write is illegal!\n", idx);
            return 0;
        }

        break;
    case CF_XMEM:
        memptr = &dev[thisdev].xmem[idx];
        if(idx/3 ==dev[thisdev].xmem_patch_version)
            FUNC_TRACE("Patch version=%d.%d.%d\n", data[0], data[1], data[2]);
        break;
    case CF_YMEM:
        memptr = &dev[thisdev].ymem[idx];
        break;
    case CF_IOMEM:
        /* address is in TFA98XX_CF_MAD */
        if (dev[thisdev].Reg[TFA98XX_CF_MAD] == 0x8100) {    /* CF_CONTROL reg */
            if (data[2] & 1) {    /* set ACS */
                dev[thisdev].Reg[TFA98XX_STATUSREG] |=
                    (TFA98XX_STATUSREG_ACS);
                dev[thisdev].memIdx += 3;
                return 3;    /* go back, writing is done */
            } else {         /* clear ACS */
                dev[thisdev].Reg[TFA98XX_STATUSREG] &=  ~(TFA98XX_STATUSREG_ACS);
                dev[thisdev].memIdx += 3;
                return 3;    /* go back, writing is done */            }
        } else if ((CF_IO_PATCH_START <= dev[thisdev].Reg[TFA98XX_CF_MAD]) &&
               (dev[thisdev].Reg[TFA98XX_CF_MAD] <
                (CF_IO_PATCH_START + CF_IO_PATCH_LENGTH))) {
            memptr = &dev[thisdev].iomem[idx];
        } else {
            /* skip other io's */
            return 3;
        }
        memptr = &dev[thisdev].iomem[idx];
        break;
    }
    DUMMYVERBOSE
        PRINT("W %s[%02d]: 0x%02x 0x%02x 0x%02x\n", cfmemName[type],
           idx / 3, data[0], data[1], data[2]);
    *memptr++ = *data++;
    *memptr++ = *data++;
    *memptr++ = *data++;
    dev[thisdev].memIdx += 3;
    return 3;        /* TODO 3 */
}

/*
 * read from CF memory space
 */
static int lxDummyMemr(int type, uint8_t *data)
{
    uint8_t *memptr = data;
    int idx = dev[thisdev].memIdx;

    switch (type) {
    case CF_PMEM:
        memptr = &dev[thisdev].pmem[idx];
        break;
    case CF_XMEM:
        memptr = &dev[thisdev].xmem[idx];
        break;
    case CF_YMEM:
        memptr = &dev[thisdev].ymem[idx];
        break;
    case CF_IOMEM:
        memptr = &dev[thisdev].iomem[idx];
        break;
    }
    DUMMYVERBOSE
        PRINT("R %s[%02d]: 0x%02x 0x%02x 0x%02x\n", cfmemName[type],
           idx / 3, memptr[0], memptr[1], memptr[2]);

    *data++ = *memptr++;
    *data++ = *memptr++;
    *data++ = *memptr++;
/* *data++ =0; */
/* *data++ =0; */
/* *data++ =0; */

    dev[thisdev].memIdx += 3;

    return 3;        /* TODO 3 */
}

/******************************************************************************
 * DSP RPC interaction response
 */
static void setStateInfo(Tfa98xx_StateInfo_t *pInfo, unsigned char *bytes);
static void makeStateInfo(float agcGain, float limGain, float sMax,
              int T, int statusFlag, float X1, float X2, float Re, int shortOnMips);

/* from Tfa9887_internals.h */
#define SPKRBST_HEADROOM             7    /* Headroom applied to the main input signal */
#define SPKRBST_AGCGAIN_EXP            SPKRBST_HEADROOM    /* Exponent used for AGC Gain related variables */
#define SPKRBST_TEMPERATURE_EXP     9
#define SPKRBST_LIMGAIN_EXP                4    /* Exponent used for Gain Corection related variables */
#define SPKRBST_TIMECTE_EXP         1

static void setStateInfo(Tfa98xx_StateInfo_t *pInfo, unsigned char *bytes)
{
    int data[STATE_SIZE];

    data[0] =
        (int)roundf(pInfo->agcGain * (1 << (23 - SPKRBST_AGCGAIN_EXP)));
    data[1] =
        (int)roundf(pInfo->limGain * (1 << (23 - SPKRBST_LIMGAIN_EXP)));
    data[2] = (int)roundf(pInfo->sMax * (1 << (23 - SPKRBST_HEADROOM)));
    data[3] = pInfo->T * (1 << (23 - SPKRBST_TEMPERATURE_EXP));
    data[4] = pInfo->statusFlag;
    data[5] = (int)roundf(pInfo->X1 * (1 << (23 - SPKRBST_HEADROOM)));
    // 97 has shorter stateinfo
    if ( dev[0].type != tfa9897) { // TODO allow mixed types ?
        data[6] = (int)roundf(pInfo->X2 * (1 << (23 - SPKRBST_HEADROOM)));
        data[7] = (int)roundf(pInfo->Re * (1 << (23 - SPKRBST_TEMPERATURE_EXP)));
        data[8]= pInfo->shortOnMips;
    } else {
        data[6] =  (int)roundf(pInfo->Re * (1 << (23 - SPKRBST_TEMPERATURE_EXP)));
        data[7]=pInfo->shortOnMips;
        data[8]=0 ; // not used
    }

    convert24Data2Bytes(STATE_SIZE, bytes, data);

}
/*
 * fill the StateInfo structure with the input data
 */
static void makeStateInfo(float agcGain, float limGain, float sMax,
              int T, int statusFlag, float X1, float X2, float Re, int shortOnMips)
{
    Tfa98xx_StateInfo_t Info;

    Info.agcGain = agcGain;
    Info.limGain = limGain;
    Info.sMax = sMax;
    Info.T = T;
    Info.statusFlag = statusFlag;
    Info.X1 = X1;
    Info.X2 = X2; // skipped for 97
    Info.Re = Re;
    Info.shortOnMips = shortOnMips;
    setStateInfo(&Info, stateInfo);

}
#define TFA9887_MAXTAG              (138)
/* TFA9887 revstring */
#define DSP_revstring        "< Dec 21 2011 - 12:33:16 -  SpeakerBoostOnCF >"
/* the number of elements in Tfa98xx_SpeakerBoost_StateInfo */
#define FW_STATE_SIZE             9
#define FW_STATE_MAX_SIZE       FW_STATE_SIZE

/*
 * fill dev[thisdev].xmem Speakerboost module RPC buffer with the return values
 */
static int setDspParamsSpeakerboost(int param)
{
    int i = 0, j;
    uint8_t *ptr;
    char *tag = DSP_revstring;
    /* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters) */

    switch (param) {
    case 0xff:        /* tag */
        ptr = &dev[thisdev].xmem[6 + 2];
        for (i = 0, j = 0; i < TFA9887_MAXTAG; i++, j += 3) {
            ptr[j] = tag[i];    /* 24 bits, byte[2] */
        }
        if (lxDummyFailTest == 6)
            ptr[0] = '!';    /* fail */
        /* *pRpcStatus = (mem[0]<<16) | (mem[1]<<8) | mem[2]; */
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        break;
    case PARAM_SET_CONFIG:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        memcpy(lastConfig, &dev[thisdev].xmem[6], sizeof(Tfa98xx_Config_t));
        if (lxDummyFailTest == 7)
            lastConfig[0] = ~lastConfig[0];
        FUNC_TRACE("loaded config\n");
        break;
    case PARAM_SET_PRESET:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        memcpy(lastPreset, &dev[thisdev].xmem[6], sizeof(Tfa98xx_Preset_t));
        if (lxDummyFailTest == 8)
            lastPreset[1] = ~lastPreset[1];
        FUNC_TRACE("loaded preset\n");
        break;
    case PARAM_SET_LSMODEL:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        memcpy(lastSpeaker, &dev[thisdev].xmem[6],
               sizeof(Tfa98xx_SpeakerParameters_t));
        FUNC_TRACE("loaded speaker\n");
        break;
    case PARAM_GET_LSMODEL:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        if (lxDummyFailTest != 9)
            memcpy(&dev[thisdev].xmem[6], lastSpeaker, sizeof(Tfa98xx_SpeakerParameters_t));
        else
            //bzero(&dev[thisdev].xmem[6], sizeof(Tfa98xx_SpeakerParameters_t));
            memset(&dev[thisdev].xmem[6], 0, sizeof(Tfa98xx_SpeakerParameters_t));
        break;
    case PARAM_GET_LSMODELW:    /* for now just return the speakermodel */
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        //memcpy(&dev[thisdev].xmem[6], lsmodelw, sizeof(Tfa98xx_SpeakerParameters_t)); //Original
        memcpy(&dev[thisdev].xmem[6], static_x_model, sizeof(Tfa98xx_SpeakerParameters_t)); //static_x_model is a static kopie of a real X-model
        break;
    case PARAM_GET_ALL:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;

        memcpy(&dev[thisdev].xmem[6], lastConfig, dev[thisdev].config_length);
        memcpy(&dev[thisdev].xmem[6 + dev[thisdev].config_length], lastPreset, sizeof(Tfa98xx_Preset_t));

        //memcpy(&dev[thisdev].xmem[6], lastConfig, sizeof(Tfa98xx_Config_t));
        //memcpy(&dev[thisdev].xmem[6 + sizeof(Tfa98xx_Config_t)], lastPreset, sizeof(Tfa98xx_Preset_t));
        break;
    case PARAM_GET_STATE:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        getStateInfo();
        if (lxDummyFailTest != 12)
            //memcpy(&dev[thisdev].xmem[6], stateInfo, FW_STATE_MAX_SIZE);
            memcpy(&dev[thisdev].xmem[6], stateInfo, 24 /*sizeof(stateInfo) FW_STATE_MAX_SIZE*/);

        else
            //bzero(&dev[thisdev].xmem[6], sizeof(Tfa98xx_SpeakerParameters_t));
            memset(&dev[thisdev].xmem[6], 0, sizeof(Tfa98xx_SpeakerParameters_t));
        break;

    case PARAM_GET_RE0:
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        /* i2cExecuteRS W   2: 0x6c 0x72 */
        /* i2cExecuteRS R   4: 0x6d  0x01 0xc6 0x90 */
        dev[thisdev].xmem[6] = 0x01;
        dev[thisdev].xmem[7] = 0xc6;
        dev[thisdev].xmem[8] = 0x90;

        if (lxDummyFailTest == 11) {
            dev[thisdev].xmem[6] = 0;
            dev[thisdev].xmem[7] = 0;
            dev[thisdev].xmem[8] = 0;
        }
        break;
    case SB_PARAM_SET_DRC:
        FUNC_TRACE("set DRC (dummy tbd)\n");
        break;
    case SB_PARAM_SET_AGCINS:
        FUNC_TRACE("set agc gain insert (dummy tbd)\n");
        break;
    default:
        PRINT("%s: unknown RPC PARAM:0x%0x\n", __FUNCTION__, param);
        break;
    }

    return i - 1;
}
/*
 *
 */
#define BIQUAD_COEFF_SIZE 6
static int setDspParamsEq(int param)
{
    int coeff_buffer[BIQUAD_COEFF_SIZE];
    float bq[5];
    int headroom;


        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;

        DUMMYVERBOSE PRINT("\nEQ[%d] ", param);
        DUMMYVERBOSE hexdump(18, &dev[thisdev].xmem[6]);

        convertBytes2Data24(3*BIQUAD_COEFF_SIZE, &dev[thisdev].xmem[6], coeff_buffer);
        headroom = coeff_buffer[0];
        //coeff_buffer[1] = -a2 * (1 << (23 - headroom));
        bq[0] = (float)(coeff_buffer[3]/(1 << (23 - headroom)));
        //PRINT("bq %f\n", bq[0]);
//        coeff_buffer[2] = (int) (-a1 * (1 << (23 - headroom)));
//        coeff_buffer[3] = (int) (b2 * (1 << (23 - headroom)));
//        coeff_buffer[4] = (int) (b1 * (1 << (23 - headroom)));
//        coeff_buffer[5] = (int) (b0 * (1 << (23 - headroom)));


//        memcpy(lastSpeaker, &dev[thisdev].xmem[6],
//               sizeof(Tfa98xx_SpeakerParameters_t));
        return 0;
}

static int setDspParamsRe0(int param)
{
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        PRINT("\nRe[%d] ", param);

        return 0;
}
/*
 * fill dev[thisdev].xmem Framework module RPC buffer with the return values
 */
static int setDspParamsFrameWork(int param)
{
 int i;
    switch (param) {
    case FW_PAR_ID_GET_FEATURE_BITS:        /* FW_PARAM_GET_FEATURE_BITS: */
        FUNC_TRACE("FW_PAR_ID_GET_FEATURE_BITS\n");
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = (dev[thisdev].type == tfa9887) || (dev[thisdev].type == tfa9890) ? 3 : 0;    /* no feature bits */
        /* i2cExecuteRS W   2: 0x6c 0x72 */
        /* i2cExecuteRS R   4: 0x6d 0x00 0x00 0x03 */
        dev[thisdev].xmem[6] = 0x00;
        dev[thisdev].xmem[7] = dev[thisdev].type == tfa9895 ? 0x00 : 0x02;    /* No DRC */
        dev[thisdev].xmem[8] = 0x00;
        break;
    case FW_PAR_ID_SET_CURFRAC_DELAY:        /* : */
        FUNC_TRACE("set fractional delay table:");
        for(i=0;i<9;i++)
            PRINT(" %d", dev[thisdev].xmem[8+i*3]);
        PRINT("\n");
        //TODO if N1B memcpy(dev[thisdev].fracdelaytable, &dev[thisdev].xmem[6], sizeof(dev[thisdev].fracdelaytable));
        break;
    case FW_PAR_ID_SET_CURRENT_DELAY:        /* : */
        FUNC_TRACE("set firmware   delay table:");
        for(i=0;i<9;i++)
            PRINT(" %d", dev[thisdev].xmem[8+i*3]);
        PRINT("\n");
        //TODO if N1B memcpy(dev[thisdev].fracdelaytable, &dev[thisdev].xmem[6], sizeof(dev[thisdev].fracdelaytable));
        break;
    case FW_PAR_ID_GLOBAL_GET_INFO:        /* FW_PARAM_GET_STATE: */
        dev[thisdev].xmem[0] = 0;
        dev[thisdev].xmem[1] = 0;
        dev[thisdev].xmem[2] = 0;
        /* i2cExecuteRS W   2: 0x6c 0x72 */
        /* i2cExecuteRS R   4: 0x6d 0x00 0x00 0x03 */
        dev[thisdev].xmem[6] = 0x00;
        dev[thisdev].xmem[7] = 0x00;
        dev[thisdev].xmem[8] = 0x03;
        getStateInfo();
//        memcpy(&dev[thisdev].xmem[6], stateInfo, FW_STATE_MAX_SIZE);
        memcpy(&dev[thisdev].xmem[6], stateInfo, (STATE_SIZE)*3);
        memcpy(&dev[thisdev].xmem[6+STATE_SIZE*3], stateInfoDrc, (STATE_SIZE_DRC)*3);
        break;
    }

    return 0;
}

/******************************************************************************
 * utility and helper functions
 */
static FILE *infile;
/*
 * get state info from file and wrap around
 */
static int getStateInfo(void)
{
    int n, linenr, ShortOnMips;
    float agcGain, limitGain, limitClip, batteryVoltage,
        boostExcursion, manualExcursion, speakerResistance;
    unsigned int icTemp, speakerTemp;
    unsigned int shortOnMips = 0;
    unsigned short statusFlags, statusRegister;
    char line[256];

    if (infile == 0)
        return -1;

    if (feof(infile)) {
        rewind(infile);
        fgets(line, sizeof(line), infile);    /* skip 1st line */
    }

    fgets(line, sizeof(line), infile);
    n = sscanf(line, "%d,%hx,0x%4hx,%f,%f,%f,%f,%d,%d,%f,%f,%f,%d",    /* 1 2 *///TODO update csv!!!!
           &linenr,    /* 3 */
           &statusRegister,    /* 4 */
           &statusFlags,    /* 5 */
           &agcGain,    /* 6 */
           &limitGain,    /* 7 */
           &limitClip,    /* 8 */
           &batteryVoltage,    /* 9 */
           &speakerTemp,    /* 10 */
           &icTemp,    /* 11 */
           &boostExcursion,    /* 12 */
           &manualExcursion,    /* 13 */
           &speakerResistance,    /* 14 */
           &ShortOnMips);    /* 15 */
    /* PRINT("%x >%s\n",statusRegister,line); */

    if (13 == n) {

        makeStateInfo(agcGain, limitGain, limitClip, speakerTemp,
                  statusFlags, boostExcursion, manualExcursion,
                  speakerResistance,shortOnMips);
        dev[thisdev].Reg[0] = statusRegister;
        dev[thisdev].Reg[1] = (uint16_t)batteryVoltage;
        dev[thisdev].Reg[2] = (uint16_t)icTemp;
        return 0;
    }
    return 1;

}

/*
 * set the input file for state info input
 */
static int setInputFile(char *file)
{
    char line[256];

    infile = fopen(file, "r");

    if (infile == 0)
        return 0;

    fgets(line, sizeof(line), infile);    /* skip 1st line */

    return 1;
}

static void hexdump(int num_write_bytes, const unsigned char *data)
{
    int i;

    for (i = 0; i < num_write_bytes; i++) {
        PRINT("0x%02x ", data[i]);
    }

}

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
static void convert24Data2Bytes(int num_data, unsigned char bytes[], int data[])
{
    int i;            /* index for data */
    int k;            /* index for bytes */
/* int num_bytes = num_data * 3; */

    for (i = 0, k = 0; i < num_data; ++i, k += 3) {
        *bytes = 0xff & (data[i] >> 16);
        if (data[i] < 0)
            *bytes++ |= 0x80;    /* sign */
        else
            bytes++;
        *bytes++ = 0xff & (data[i] >> 8);
        *bytes++ = 0xff & (data[i]);
    }
}

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
void convertBytes2Data24(int num_bytes, const unsigned char bytes[],
                   int data[])
{
    int i;            /* index for data */
    int k;            /* index for bytes */
    int d;
    int num_data = num_bytes / 3;
    //_ASSERT((num_bytes % 3) == 0);
    for (i = 0, k = 0; i < num_data; ++i, k += 3) {
        d = (bytes[k] << 16) | (bytes[k + 1] << 8) | (bytes[k + 2]);
        //_ASSERT(d >= 0);
        //_ASSERT(d < (1 << 24));    /* max 24 bits in use */
        if (bytes[k] & 0x80)    /* sign bit was set */
            d = -((1 << 24) - d);

        data[i] = d;
    }
}

