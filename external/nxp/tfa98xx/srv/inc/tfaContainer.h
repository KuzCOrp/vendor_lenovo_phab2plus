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



#ifndef TFACONTAINER_H_
#define TFACONTAINER_H_

/* static limits */
#define TFACONT_MAXDEVS  (4)   /* maximum nr of devices */
#define TFACONT_MAXPROFS (16) /* maximum nr of profiles */

#include "tfa98xxParameters.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * accounting globals
 */
int tfa98xx_cnt_verbose;
int tfa98xx_dev_type;

nxpTfa98xxParamsType_t cliParseFiletype(char *filename);
/* core functions */
/*
 * return the number of devices in the container
 */
int tfa98xx_cnt_max_device(void);
/*
 * loads the container file
 */
int tfa98xx_cnt_loadfile(char *fname, int cnt_verbose);

/*
 * Get costumer header information
 */
void tfaContGetHdr(char *inputname, struct nxpTfaHeader *hdrbuffer);
/*
 * Get costumer header speaker file information
 */
void tfa_cnt_get_spk_hdr(char *inputname, struct nxpTfaSpkHeader *hdrbuffer);

/*
 * Specify the device type for service layer support
 */
void tfa_cont_dev_type(int type);

/*
 * set verbosity level
 */
void tfa_cnt_verbose(int level);
void tfa_cnt_util_verbose(int level);
void tfa_cont_write_verbose(int verbose);
/**
 * return the pointer to loaded the container
 * @return NULL no container loaded
 * @return non-NULL point to container in memory
 */
nxpTfaContainer_t * tfa98xx_get_cnt(void);
/*
 * lookup slave and return device index
 */
int tfa98xx_cnt_slave2idx(int slave);
/*
 * get the slave for the device if it exists
 */
Tfa98xx_Error_t tfaContGetSlave(int devn, uint8_t *slave);
// write  reg  and bitfield items in the devicelist to the target
Tfa98xx_Error_t tfaContWriteRegsDev(int device);
// write  reg  and bitfield items in the profilelist the target
Tfa98xx_Error_t tfaContWriteRegsProf(int device, int profile);
// write  patchfile in the devicelist to the target
Tfa98xx_Error_t tfaContWritePatch(int device);
// write all  param files in the devicelist to the target
Tfa98xx_Error_t tfaContWriteFiles(int device);
// write all  param files in the profilelist the target
/**
 * Open the specified device after looking up the target address.
 *
 * @param device the index of the device
 * @return Tfa98xx_Error
 */
enum Tfa98xx_Error tfaContOpen(int device);
/**
 * Close the  device.
 *
 * @param device the index of the device
 * @return Tfa98xx_Error
 */
enum Tfa98xx_Error tfaContClose(int device);
/**
 * Get the device name string
 *
 * @param device the index of the device
 * @return device name string or error string of not found
 */
char  *tfaContDeviceName(int idx) ;
/*
 * Get the name of the profile at certain index for a device in the container file
 *  return profile name
 */
char  *tfaContProfileName(int idx, int ipx);
/*
 *  process all items in the profilelist
 *   NOTE an error return during processing will leave the device muted
 *
 */
Tfa98xx_Error_t tfaContWriteProfile(int device, int profile, int vstep);

/* get/set current profile */
int tfaContGetCurrentProfile(void);
void tfaContSetCurrentProfile(int prof);
/* set/get current vstep */
void tfaContSetCurrentVstep(int vstep, int channel);
int tfaContGetCurrentVstep(int channel);

Tfa98xx_Error_t tfaContWriteVstep(int device,  nxpTfaVolumeStep2File_t *vp);
Tfa98xx_Error_t tfaContWriteEq(int device, nxpTfaEqualizerFile_t *eqf);
Tfa98xx_Error_t tfaContWriteFilterbank(int device, nxpTfaFilter_t *filter);
Tfa98xx_Error_t tfaContWriteFilesProf(int device, int profile, int vstep);
Tfa98xx_Error_t tfaContWriteFilesVstep(int device, int profile, int vstep);
int tfaContCrcCheckContainer(nxpTfaContainer_t *cont);
nxpTfaDeviceList_t *tfaContDevice(int idx);
int tfaContMaxProfile(int ndev);
nxpTfaProfileList_t* tfaContProfile(int ndev, int nprof);
int tfaContCrcCheck(nxpTfaHeader_t *hdr) ;
/* display */
void tfaContShowSpeaker(nxpTfaSpeakerFile_t *spk) ;
void tfaContShowHeader(nxpTfaHeader_t *hdr);
void tfaContShowEq(nxpTfaEqualizerFile_t *eqf);
void tfaContShowFile(nxpTfaHeader_t *hdr);
char *tfaContFileTypeName(nxpTfaFileDsc_t *file) ;
/* return the speakerbuffer for this device */
uint8_t *tfacont_speakerbuffer(int device);
nxpTfaProfileList_t* tfaContNextProfile(nxpTfaProfileList_t* prof);

/**
 * print current bitfield value
 */
enum Tfa98xx_Error tfaConfDumpBf(nxpTfaBitfield_t bf);

/*
 * write a parameter file to the device
 */
Tfa98xx_Error_t tfaContWriteFile(int device,  nxpTfaFileDsc_t *file);

/* bitfield */
char *tfaContBfName(uint16_t num);
uint16_t tfaContBfEnum(char *name);
Tfa98xx_Error_t tfaRunWriteBitfield(Tfa98xx_handle_t handle,  nxpTfaBitfield_t bf);//TODO move to run core
/* */
/*
 * show the contents of the local container
 */
tfa_srv_api_error_t  tfaContShowContainer(char *strings, int maxlength);
tfa_srv_api_error_t  tfaContShowItem(nxpTfaDescPtr_t *dsc,
                            char **strings, int *length, int maxlength);
tfa_srv_api_error_t  tfaContShowDevice(int idx, char **strings, int *length, int maxlength);
tfa_srv_api_error_t  tfaCheckStringLength(char *str, char **strings, int *length, int maxlength);
/* for climax */
/*
 * write a parameter file to the device
 */
Tfa98xx_Error_t tfaContWriteFileByname(int device,  char *fname);

/*
 * Get the max volume step associated with Nth profile for the Nth device
 */
int tfacont_get_max_vstep(int devIdx, int profIdx);
/**
 * Get the file contents associated with the device or profile
 * Search within the device tree, if not found, search within the profile
 * tree. There can only be one type of file within profile or device.
 *  .
 * @param devIdx I2C device index
 * @param profIdx I2C profile index in the device
 * @param type file type
 * @return 0 NULL if file type is not found
 * @return 1 file contents
 */
nxpTfaFileDsc_t *tfacont_getfiledata(int devIdx, int profIdx, enum nxpTfaHeaderType type);

/**
 * Split the (binary) container file into human readable ini and individual files
 * @return 0 if all files are generated
 */
int tfa98xx_cnt_split(char *fileName);
/**
 * print all the bitfields of the register
 * @param fd output file
 * @param reg address
 * @param regval register value
 * @return 0 if at least 1 name was found
 */
int tfaRunBitfieldDump(FILE *fd, unsigned char reg, unsigned short regval );
/*
 * read a bit field
 */
Tfa98xx_Error_t tfaRunReadBitfield(Tfa98xx_handle_t handle,  nxpTfaBitfield_t *bf);


int HeaderMatches (nxpTfaHeader_t *hdr, nxpTfaHeaderType_t t);

/* write functions */
int tfaContSave(nxpTfaHeader_t *hdr, char *filename);
int tfaContBin2Hdr(char *iniFile, int argc, char *argv[]);
int tfaContIni2Container(char *iniFile);
int tfaContParseIni(char *iniFile, char *outFileName, nxpTfaLocationInifile_t *loc);
int tfaContCreateContainer(nxpTfaContainer_t *contIn, char *outFileName, nxpTfaLocationInifile_t *loc);
nxpTfaProfileList_t *tfaContFindProfile(nxpTfaContainer_t *cont,const char *name);
nxpTfaProfileList_t *tfaContGet1stProfList(nxpTfaContainer_t *cont);
nxpTfaProfileList_t *tfaContGetDevProfList(nxpTfaContainer_t *cont,int devIdx,int profIdx);
nxpTfaDescPtr_t *tfaContSetOffset(nxpTfaContainer_t *cont,nxpTfaDescPtr_t *dsc,int idx);
char *tfaContGetString(nxpTfaDescPtr_t *dsc);
nxpTfaDeviceList_t *tfaContGetDevList(nxpTfaContainer_t *cont,int idx);
nxpTfaDescriptorType_t parseKeyType(char *key);
uint32_t tfaContCRC32(uint8_t *addr,uint32_t num,uint32_t crc);

/*
 * Read file
 */
int  tfaReadFile(char *fname, void **buffer);

#ifdef __cplusplus
}
#endif

#endif /* TFACONTAINER_H_ */
