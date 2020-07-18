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



#ifndef TFA98XXPARAMETERS_H_
#define TFA98XXPARAMETERS_H_

#include <stdint.h>
#include "nxpTfa98xx.h"

// the pack pragma is required to make that the size in memory
// matches the actual variable lenghts
// This is to assure that the binary files can be transported between
// different platforms.
#pragma pack (push, 1)

/*
 * typedef for 24 bit value using 3 bytes
 */
typedef struct uint24 {
  uint8_t b[3];
} uint24_t;
/*
 * the generic header
 *   all char types are in ASCII
 */
typedef struct nxpTfaHeader {
    uint16_t id;
    char version[2];     // "V_" : V=version, vv=subversion
    char subversion[2];  // "vv" : vv=subversion
    uint16_t size;       // data size in bytes following CRC
    uint32_t CRC;        // 32-bits CRC for following data
    char customer[8];    // “name of customer”
    char application[8]; // “application name”
    char type[8];         // “application type name”
} nxpTfaHeader_t;

typedef enum nxpTfaSamplerate {
    fs_8k,       // 8kHz
    fs_11k025,   // 11.025kHz
    fs_12k,      // 12kHz
    fs_16k,      // 16kHz
    fs_22k05,    // 22.05kHz
    fs_24k,      // 24kHz
    fs_32k,      // 32kHz
    fs_44k1,     // 44.1kHz
    fs_48k,      // 48kHz
    fs_96k       // 96kHz
} nxpTfaSamplerate_t;

/*
 * the biquad coefficients for the API together with index in filter
 *  the biquad_index is the actual index in the equalizer +1
 */
typedef struct nxpTfaBiquad {
  uint8_t bytes[6*sizeof(uint24_t)];
} nxpTfaBiquad_t;

typedef struct nxpTfaBiquadFloat {
  float headroom;
  float b0;
  float b1;
  float b2;
  float a1;
  float a2;
} nxpTfaBiquadFloat_t;


typedef enum nxpTfaFilterType {
    fCustom,         //User defined biquad coefficients
    fFlat,           //Vary only gain
    fLowpass,        //2nd order Butterworth low pass
    fHighpass,       //2nd order Butterworth high pass
    fLowshelf,
    fHighshelf,
    fNotch,
    fPeak,
    fBandpass,
    f1stLP,
    f1stHP,
    fCount
} nxpTfaFilterType_t;

/*
 * filter parameters for biquad (re-)calculation
 *  this is used for a filter inside a vstep file which defines the samplerate
 */
typedef struct nxpTfaFilter {
  nxpTfaBiquad_t biquad;
  uint8_t enabled;
  uint8_t type; // (== enum FilterTypes, assure 8bits length)
  float frequency;
  float Q;
  float gain;
} nxpTfaFilter_t ;  //8 * float + int32 + byte == 37

#define TFA98XX_MAX_EQ 10
typedef struct nxpTfaEqualizer {
  nxpTfaFilter_t filter[TFA98XX_MAX_EQ];// note: API index counts from 1..10
} nxpTfaEqualizer_t;

/*
 * files
 */
#define HDR(c1,c2) (c2<<8|c1) // little endian
typedef enum nxpTfaHeaderType {
    paramsHdr       = HDR('P','M'),
    volstepHdr         = HDR('V','P'),
    patchHdr         = HDR('P','A'),
    speakerHdr         = HDR('S','P'),
    presetHdr         = HDR('P','R'),
    configHdr         = HDR('C','O'),
    equalizerHdr    = HDR('E','Q'),
    drcHdr             = HDR('D','R'),
    msgHdr        = HDR('M','G')    /* generic message */
} nxpTfaHeaderType_t;

/*
 * equalizer file
 */
#define NXPTFA_EQ_VERSION    '1'
#define NXPTFA_EQ_SUBVERSION "00"
typedef struct nxpTfaEqualizerFile {
    nxpTfaHeader_t hdr;
    uint8_t samplerate;                  // ==enum samplerates, assure 8 bits
    nxpTfaFilter_t filter[TFA98XX_MAX_EQ];// note: API index counts from 1..10
} nxpTfaEqualizerFile_t;

/*
 * patch file
 */
#define NXPTFA_PA_VERSION    '1'
#define NXPTFA_PA_SUBVERSION "00"
typedef struct nxpTfaPatchFile {
    nxpTfaHeader_t hdr;
    uint8_t data[];
} nxpTfaPatch_t;

/*
 * generic message file
 *   -  the payload of this file includes the opcode and is send straight to the DSP
 */
#define NXPTFA_MG_VERSION    '1'
#define NXPTFA_MG_SUBVERSION "00"
typedef struct nxpTfaMsgFile {
    nxpTfaHeader_t hdr;
    uint8_t data[];
} nxpTfaMsg_t;

/*
 * NOTE the tfa98xx API defines the enum Tfa98xx_config_type that defines
 *          the subtypes as decribes below.
 *          tfa98xx_dsp_config_parameter_type() can be used to get the
 *           supported type for the active device..
 */
/*
 * config file V1 sub 1
 */
#define NXPTFA_CO_VERSION    '1'
#define NXPTFA_CO_SUBVERSION1 "01"
typedef struct nxpTfaConfigS1File {
    nxpTfaHeader_t hdr;
    uint8_t data[55*3];
} nxpTfaConfigS1_t;
/*
 * config file V1 sub 2
 */
#define NXPTFA_CO_VERSION    '1'
#define NXPTFA_CO_SUBVERSION2 "02"
typedef struct nxpTfaConfigS2File {
    nxpTfaHeader_t hdr;
    uint8_t data[67*3];
} nxpTfaConfigS2_t;
/*
 * config file V1 sub 3
 */
#define NXPTFA_CO_VERSION    '1'
#define NXPTFA_CO_SUBVERSION3 "03"
typedef struct nxpTfaConfigS3File {
    nxpTfaHeader_t hdr;
    uint8_t data[67*3];
} nxpTfaConfigS3_t;

/*
 * config file V1.0
 */
#define NXPTFA_CO_VERSION    '1'
#define NXPTFA_CO_SUBVERSION "00"
typedef struct nxpTfaConfigFile {
    nxpTfaHeader_t hdr;
    uint8_t data[];
} nxpTfaConfig_t;

/*
 * preset file
 */
#define NXPTFA_PR_VERSION    '1'
#define NXPTFA_PR_SUBVERSION "00"

typedef struct nxpTfaPresetFile {
    nxpTfaHeader_t hdr;
    uint8_t data[];
} nxpTfaPreset_t;
/*
 * drc file
 *   TODO add DRC filter data, treshold ...
 */
#define NXPTFA_DR_VERSION    '1'
#define NXPTFA_DR_SUBVERSION "00"
typedef struct nxpTfaDrcFile {
    nxpTfaHeader_t hdr;
    uint8_t data[];
} nxpTfaDrc_t;

/*
 * volume step structures
 */
// VP01
#define NXPTFA_VP1_VERSION    '1'
#define NXPTFA_VP1_SUBVERSION "01"
typedef struct nxpTfaVolumeStep1 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
} nxpTfaVolumeStep1_t;

// VP02
#define NXPTFA_VP2_VERSION    '2'
#define NXPTFA_VP2_SUBVERSION "01"
typedef struct nxpTfaVolumeStep2 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
    nxpTfaFilter_t filter[TFA98XX_MAX_EQ];// note: API index counts from 1..10
} nxpTfaVolumeStep2_t;

// VP03 is obsolete

// VP04
/** obsolete -DRC is now a different file
#define NXPTFA_VP4_VERSION    "4"
#define NXPTFA_VP4_SUBVERSION "01"
typedef struct nxpTfaVolumeStep4 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
    nxpTfaEqualizer_t eq;
#if (defined(TFA9887B) || defined(TFA98XX_FULL))
    uint8_t drc[TFA98XX_DRC_LENGTH];
#endif
} nxpTfaVolumeStep4_t;
**/
/*
 * volumestep file
 */
#define NXPTFA_VP_VERSION    '1'
#define NXPTFA_VP_SUBVERSION "00"
typedef struct nxpTfaVolumeStepFile {
    nxpTfaHeader_t hdr;
    uint8_t vsteps;      // can also be calulated from size+type
    uint8_t samplerate; // ==enum samplerates, assure 8 bits
    uint8_t payload;     //start of variable length contents:N times volsteps
}nxpTfaVolumeStepFile_t;
/*
 * volumestep2 file
 */
typedef struct nxpTfaVolumeStep2File {
    nxpTfaHeader_t hdr;
    uint8_t vsteps;      // can also be calulated from size+type
    uint8_t samplerate; // ==enum samplerates, assure 8 bits
    nxpTfaVolumeStep2_t vstep[];     //start of variable length contents:N times volsteps
}nxpTfaVolumeStep2File_t;
/**************************old v2 *************************************************/

/*
 * subv 00 volumestep file
 */
typedef struct nxpTfaOldHeader {
    uint16_t id;
    char version[2];     // "V_" : V=version, vv=subversion
    char subversion[2];  // "vv" : vv=subversion
    uint16_t size;       // data size in bytes following CRC
    uint32_t CRC;        // 32-bits CRC for following data
} nxpTfaOldHeader_t;

typedef struct nxpOldTfaFilter {
  double bq[5];
  int32_t type;
  double frequency;
  double Q;
  double gain;
  uint8_t enabled;
} nxpTfaOldFilter_t ;  //8 * float + int32 + byte == 37
typedef struct nxpTfaOldVolumeStep2 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
    nxpTfaOldFilter_t eq[10];
} nxpTfaOldVolumeStep2_t;
typedef struct nxpTfaOldVolumeStepFile {
    nxpTfaOldHeader_t hdr;
    nxpTfaOldVolumeStep2_t step[];
//    uint8_t payload;     //start of variable length contents:N times volsteps

}nxpTfaOldVolumeStep2File_t;
/**************************end old v2 *************************************************/

/*
 * speaker file header
 */
struct nxpTfaSpkHeader {
    struct nxpTfaHeader hdr;
    char name[8];                // speaker nick name (e.g. “dumbo”)
    char vendor[16];
    char type[8];
    //    dimensions (mm)
    uint8_t height;
    uint8_t width;
    uint8_t depth;
    uint16_t ohm;
};


/*
 * speaker file
 */
#define NXPTFA_SP_VERSION        '1'
#define NXPTFA_SP_SUBVERSION    "00"
typedef struct nxpTfaSpeakerFile {
    nxpTfaHeader_t hdr;
    char name[8];                // speaker nick name (e.g. “dumbo”)
    char vendor[16];
    char type[8];
    //    dimensions (mm)
    uint8_t height;
    uint8_t width;
    uint8_t depth;
    uint16_t ohm;
    uint8_t data[]; //payload TFA98XX_SPEAKERPARAMETER_LENGTH
} nxpTfaSpeakerFile_t;

#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))

#define NXPTFA_VP2_VERSION_M2    '2'
#define NXPTFA_VP2_SUBVERSION_M2 "02"

struct nxpTfaFWVer {
    uint8_t Major;
    uint8_t minor;
    uint8_t minor_update:6;
    uint8_t Update:2;
};

struct nxpTfaCmdID {
    uint8_t a;
    uint8_t b;
    uint8_t c;
};

struct nxpTfaMsg {
    struct nxpTfaCmdID cmdId;
    uint8_t data[];
};

struct nxpTfaFWMsg {
    struct nxpTfaFWVer fwVersion;
    struct nxpTfaMsg payload;
};


#define NXPTFA_SP_VERSION_MAX2  '2'
struct nxpTfaSpeakerFileMax2 {
    nxpTfaHeader_t hdr;
    char name[8];                // speaker nick name (e.g. “dumbo”)
    char vendor[16];
    char type[8];
    //    dimensions (mm)
    uint8_t height;
    uint8_t width;
    uint8_t depth;
    uint16_t ohm;
    struct nxpTfaFWMsg FWmsg; //payload including FW ver and Cmd ID
};
#endif

/*
 * parameter container file
 */
/*
 * descriptors
 */
typedef enum nxpTfaDescriptorType {
    dscDevice,        // device list
    dscProfile,        // profile list
    dscRegister,    // register patch
    dscString,        // ascii, zero terminated string
    dscFile,        // filename + file contents
    dscPatch, // patch file
    dscMarker,        // marker to indicate end of a list
    dscMode,
    dscBitfieldBase=0x80 // start of bitfield enums TODO rename to dscBitfield (base is not used anymore)
} nxpTfaDescriptorType_t;
#define TFA_BITFIELDDSCMSK 0x7fffffff

typedef struct nxpTfaDescPtr {
    uint32_t offset:24;
    uint32_t  type:8; // (== enum nxpTfaDescriptorType, assure 8bits length)
}nxpTfaDescPtr_t;

/*
 * generic file descriptor
 */
typedef struct nxpTfaFileDsc {
    nxpTfaDescPtr_t name;
    uint32_t size;    // file data length in bytes
    uint8_t data[]; //payload
} nxpTfaFileDsc_t;


/*
 * device descriptor list
 */
typedef struct nxpTfaDeviceList {
    uint8_t length;            // nr of items in the list
    uint8_t bus;            // bus
    uint8_t dev;            // device
    uint8_t func;            // subfunction or subdevice
    uint32_t devid;        // device  hw fw id
    nxpTfaDescPtr_t name;    // device name
    nxpTfaDescPtr_t list[];    // items list
} nxpTfaDeviceList_t;

/*
 * profile descriptor list
 */
typedef struct nxpTfaProfileList {
    uint32_t length:8;            // nr of items in the list
    uint32_t ID:24;            // profile ID
    nxpTfaDescPtr_t name;    // profile name
    nxpTfaDescPtr_t list[];    // items list
} nxpTfaProfileList_t;
#define TFA_PROFID 0x1234

/*
 * Bitfield descriptor
 */
typedef struct nxpTfaBitfield {
    uint16_t  value;
    uint16_t  field; // ==datasheet defined, 16 bits
} nxpTfaBitfield_t;
/*
 * Bitfield enumuration bits descriptor
 */
typedef struct nxpTfaBfEnum {
    unsigned int  len:4;        // this is the actual length-1
    unsigned int  pos:4;
    unsigned int  address:8;
} nxpTfaBfEnum_t;

/*
 * Register patch descriptor
 */
typedef struct nxpTfaRegpatch {
    uint8_t   address;    // register address
    uint16_t  value;    // value to write
    uint16_t  mask;        // mask of bits to write
} nxpTfaRegpatch_t;

/*
 * Mode descriptor
 */
typedef struct nxpTfaMode {
    int  value;    // mode value, maps to enum Tfa98xx_Mode
} nxpTfaMode_t;

typedef struct nxpTfaLocationInifile{
    char locationIni[FILENAME_MAX]; //TODO remove if possible, this depends on stdio.h
} nxpTfaLocationInifile_t;


/*
 * the container file
 *   - the size field is 32bits long (generic=16)
 *   - all char types are in ASCII
 */
#define NXPTFA_PM_VERSION '1'
#define NXPTFA_PM_SUBVERSION "00"
typedef struct nxpTfaContainer {
    char id[2];          // "XX" : XX=type
    char version[2];     // "V_" : V=version, vv=subversion
    char subversion[2];  // "vv" : vv=subversion
    uint32_t size;       // data size in bytes following CRC
    uint32_t CRC;        // 32-bits CRC for following data
    uint16_t rev;         // "extra chars for rev nr"
    char customer[8];    // “name of customer”
    char application[8]; // “application name”
    char type[8];         // “application type name”
    uint16_t ndev;          // "nr of device lists"
    uint16_t nprof;          // "nr of profile lists"
    nxpTfaDescPtr_t index[]; // start of item index table
} nxpTfaContainer_t;

#pragma pack (pop)

/*
 * bitfield enums (generated from tfa9890)
 */


#endif /* TFA98XXPARAMETERS_H_ */
