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
#include <string.h>
#include <stdlib.h>
#if !(defined(WIN32) || defined(_X64) || defined(__REDLIB__))
#include <libgen.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#if !defined(__REDLIB__)
#include <sys/stat.h>
#endif
#include <math.h> //TODO move to tfa api
#include "dbgprint.h"
#include "tfaFieldnames.h"
#include "tfaContainer.h"
#include "tfa98xxRuntime.h"
#include "nxpTfa98xx.h" /* error codes */
#include "tfaOsal.h"

/* module globals */
static nxpTfaContainer_t *gCont=NULL; /* container file */
static int gDevs=-1; // nr of devices
static nxpTfaDeviceList_t *gDev[TFACONT_MAXDEVS];
static int gProfs[TFACONT_MAXDEVS];
static nxpTfaProfileList_t  *gProf[TFACONT_MAXDEVS][TFACONT_MAXPROFS];
static char errorname[] = "!ERROR!";

/*  nxpTfaFilterType_t */
static const char *filterName[] = {
    "Custom",
    "Flat",
    "Lowpass",
    "Highpass",
    "Lowshelf",
    "Highshelf",
    "Notch",
    "Peak",
    "Bandpass",
    "1stLP",
    "1stHP",
    "Count"
};

/*
 * Set the debug option
 */
void tfa_cnt_verbose(int level) {
    tfa98xx_cnt_verbose = level;
}

nxpTfaContainer_t * tfa98xx_get_cnt(void) {
    return gCont;
}
/*
 * bitfield name table
 */
TFA_NAMETABLE
char *tfaContBfName(uint16_t num) {
    int n=0;
    do {
        if (TfaBfNames[n].bfEnum == num)
            return TfaBfNames[n].bfName;
    } while( TfaBfNames[n++].bfEnum != 0xffff);

    return TfaBfNames[n-1].bfName;
}

char *tfaContBfNameNext(uint16_t num, int index) {
    int n=0, i=0;
    do {
        if (TfaBfNames[n].bfEnum == num) {
            if (i == index)
                return TfaBfNames[n].bfName;
            i++;
        }
    } while( TfaBfNames[n++].bfEnum != 0xffff);

    return NULL;
}

uint16_t tfaContBfEnum(char *name)
{
    int n=0;
    do {
        if ( strcmp(name, TfaBfNames[n].bfName) == 0)
            return TfaBfNames[n].bfEnum;
    }
    while( TfaBfNames[n++].bfEnum != 0xffff);

    return 0xffff;

}

/*
 * return device list dsc from index
 */
nxpTfaDeviceList_t *tfaContGetDevList(nxpTfaContainer_t * cont, int idx)
{
    uint8_t *base = (uint8_t *) cont;

    if ( (idx < 0) & (idx >= cont->ndev))
        return NULL;

    if (cont->index[idx].type != dscDevice)
        return NULL;

    base += cont->index[idx].offset;
    return (nxpTfaDeviceList_t *) base;
}


/*
 * get the Nth profile for the Nth device
 */
nxpTfaProfileList_t *tfaContGetDevProfList(nxpTfaContainer_t * cont, int devIdx,
                       int profIdx)
{
    nxpTfaDeviceList_t *dev;
    int idx, hit;
    uint8_t *base = (uint8_t *) cont;

    dev = tfaContGetDevList(cont, devIdx);
    if (dev) {
        for (idx = 0, hit = 0; idx < dev->length; idx++) {
            if (dev->list[idx].type == dscProfile) {
                if (profIdx == hit++)
                    return (nxpTfaProfileList_t *) (dev->
                                    list
                                    [idx].
                                    offset +
                                    base);
            }
        }
    }

    return NULL;
}

/*
 * Get the max volume step associated with Nth profile for the Nth device
 */
int tfacont_get_max_vstep(int devIdx, int profIdx) {
    nxpTfaVolumeStep2File_t *vp;
    vp = (nxpTfaVolumeStep2File_t *)tfacont_getfiledata(devIdx, profIdx, volstepHdr);
    //PRINT("%s: max number of steps in  %d\n", __FUNCTION__, vp->vsteps);
    return vp->vsteps;
}

/**
 * Get the file contents associated with the device or profile
 * Search within the device tree, if not found, search within the profile
 * tree. There can only be one type of file within profile or device.
  */
nxpTfaFileDsc_t *tfacont_getfiledata(int devIdx, int profIdx, enum nxpTfaHeaderType type)
{
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *prof;
    nxpTfaFileDsc_t *file;
    nxpTfaHeader_t *hdr;
    unsigned int i;

    if( gCont==0 )
        return NULL;

    dev = tfaContGetDevList(gCont, devIdx);

    if( dev==0 )
        return NULL;

    /* process the device list until a file type is encountered */
    for(i=0;i<dev->length;i++) {
        if ( dev->list[i].type == dscFile ) {
            file = (nxpTfaFileDsc_t *)(dev->list[i].offset+(uint8_t *)gCont);
            hdr= (nxpTfaHeader_t *)file->data;
            /* check for file type */
            if ( hdr->id == type) {
                //PRINT("%s: file found of type %d in device %s \n", __FUNCTION__, type, tfaContDeviceName(devIdx));
                return (nxpTfaFileDsc_t *)&file->data;
            }
        }
    }

    /* File not found in device tree.
     * So, look in the profile list until the file type is encountered
     */
    prof=tfaContGetDevProfList(gCont, devIdx, profIdx);
    for(i=0;i<prof->length;i++) {
        if (prof->list[i].type == dscFile) {
            file = (nxpTfaFileDsc_t *)(prof->list[i].offset+(uint8_t *)gCont);
            hdr= (nxpTfaHeader_t *)file->data;
            /* check for file type */
            if ( hdr->id == type) {
                //PRINT("%s: file found of type %d in profile %s\n", __FUNCTION__, type, tfaContProfileName(devIdx, profIdx));
                return (nxpTfaFileDsc_t *)&file->data;
            }
        }
    }

    return NULL;
}

/*
 * static functions
 */
static int tfaContLoadContainer(char *fname);
/*
 * fill globals
 */
static void contGetDevs(nxpTfaContainer_t *cont) {
    nxpTfaProfileList_t *prof;
    int i,j;
    int count;

    // get nr of devlists+1
    for(i=0 ; i < cont->ndev ; i++) {
        gDev[i] = tfaContGetDevList(cont, i); // cache it
    }

    gDevs=cont->ndev;
    // walk through devices and get the profile lists
    for (i = 0; i < gDevs; i++) {
        j=0;
        count=0;
        while ((prof = tfaContGetDevProfList(cont, i, j)) != NULL) {
            count++;
            gProf[i][j++] = prof;
        }
        gProfs[i] = count;    // count the nr of profiles per device
    }
}

static int fsize(const char *name) //TODO no file IO allowed in here , this needs to go to osal
{
#if !defined(__REDLIB__)
    struct stat st;
    stat(name, &st);
    return st.st_size;
#else
    return 0;
#endif
}

static char nostring[]="Undefined string";

Tfa98xx_Error_t tfaContWriteFilterbank(int device, nxpTfaFilter_t *filter) {
    unsigned char biquad_index;
    Tfa98xx_Error_t error = Tfa98xx_Error_Ok;

    for(biquad_index=0;biquad_index<10;biquad_index++) {
        if (filter[biquad_index].enabled ) {
            error = Tfa98xx_DspSetParam(device, MODULE_BIQUADFILTERBANK,
                    biquad_index+1, //start @1
                    sizeof(filter[biquad_index].biquad.bytes),
                        filter[biquad_index].biquad.bytes);
        } else {
            error = Tfa98xx_DspBiquad_Disable(device, biquad_index+1);
        }
        if (error) return error;

    }
    return error;
}
//TODO add to API
#define MODULE_BIQUADFILTERBANK 2
#define BIQUAD_COEFF_SIZE       6
Tfa98xx_Error_t tfaContWriteEq(int device, nxpTfaEqualizerFile_t *eqf) {
    return tfaContWriteFilterbank(device, eqf->filter);
}
/*
 * write a parameter file to the device
 *   a filedescriptor needs to be created first to call tfaContWriteFile()
 */
Tfa98xx_Error_t tfaContWriteFileByname(int device,  char *fname) {
    nxpTfaFileDsc_t *file;
    void *fbuf = NULL;
    int size;
    Tfa98xx_Error_t err;

    size = tfaReadFile(fname, &fbuf);
    if (!size) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    file = malloc(size+(sizeof(nxpTfaFileDsc_t)));

    if ( !file ) {
        ERRORMSG("Can't allocate %d bytes.\n", size);
        free(fbuf);
        return Tfa98xx_Error_Other;
    }
    memcpy(file->data, fbuf, size);
    file->size = size;
    err = tfaContWriteFile(device, file);
    free(fbuf);
    free(file);

    return err;
}
/*
 * write a parameter file to de device
 */
Tfa98xx_Error_t tfaContWriteVstep(int device,  nxpTfaVolumeStep2File_t *vp) {
    int vstep;
    Tfa98xx_Error_t err;
    float voldB = 0.0;
    unsigned short vol;
    vstep = tfa98xx_get_vstep();
    if ( vstep < vp->vsteps ){

        voldB = vp->vstep[vstep].attenuation;
        vol = (unsigned short)(voldB / (-0.5f));
        if (vol > 255)    /* restricted to 8 bits */
            vol = 255;

        err = Tfa98xx_SetVolumeLevel(device, vol);

        err = Tfa98xx_DspWritePreset( device, sizeof(vp->vstep[0].preset), vp->vstep[vstep].preset);
        if (err != Tfa98xx_Error_Ok)
            return err;
        err = tfaContWriteFilterbank(device, vp->vstep[vstep].filter);
        if (err != Tfa98xx_Error_Ok)
            return err;
    } else {
        ERRORMSG("Incorrect volume given. The value vstep[%d] >= %d\n", vstep , vp->vsteps);
        err = Tfa98xx_Error_Bad_Parameter;
    }

    if ( tfa98xx_cnt_verbose ) PRINT("vstep[%d][%d]\n", device, vstep);

    return err;
}
/*
 * write a parameter file to the device
 */
Tfa98xx_Error_t tfaContWriteFile(int device,  nxpTfaFileDsc_t *file)
{
    int size, msg_status;
    nxpTfaHeader_t *hdr = (nxpTfaHeader_t *)file->data;
    nxpTfaHeaderType_t type;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if ( tfa98xx_cnt_verbose ) {
        tfaContShowFile(hdr);
    }

    type = (nxpTfaHeaderType_t) hdr->id;

    switch (type) {
    case msgHdr: /* generic DSP message */
        size = hdr->size - sizeof(nxpTfaMsg_t);
        err = tfa98xx_dsp_msg(device, size,
                (const char *)((nxpTfaMsg_t *)hdr)->data, &msg_status);
        if (  msg_status ) {
            PRINT("DSP msg stat: %d\n", msg_status);
        }
        break;
    case volstepHdr:
        err = tfaContWriteVstep(device, (nxpTfaVolumeStep2File_t *)hdr);
        break;
//  case patchHdr:
//      break;
    case speakerHdr:
        // tfaContShowSpeaker((nxpTfaSpeakerFile_t *) hdr);
        size = hdr->size - sizeof(nxpTfaSpeakerFile_t);
        err = Tfa98xx_DspWriteSpeakerParameters( device, size,
                            (const unsigned char *)((nxpTfaSpeakerFile_t *)hdr)->data);
        break;
    case presetHdr:
        size = hdr->size - sizeof(nxpTfaPreset_t);
        err = Tfa98xx_DspWritePreset( device, size,
                            (const unsigned char *)((nxpTfaPreset_t *)hdr)->data);
        break;
    case configHdr:
        size = hdr->size - sizeof(nxpTfaConfig_t);
        err = Tfa98xx_DspWriteConfig( device, size,
                            (const unsigned char *)((nxpTfaConfig_t *)hdr)->data);
        break;
    case equalizerHdr:
         tfaContWriteEq(device, (nxpTfaEqualizerFile_t *) hdr);
        break;
    case patchHdr:
        size = hdr->size - sizeof(nxpTfaPatch_t ); // size is total length
        err = Tfa98xx_DspPatch(device,  size, (const unsigned char *) ((nxpTfaPatch_t *)hdr)->data);
        break;
    default:
        ERRORMSG("Header is of unknown type: 0x%x\n", type);
        return Tfa98xx_Error_Bad_Parameter;
    }

    return err;
}
/*
 * get the slave for the device if it exists
 */
Tfa98xx_Error_t tfaContGetSlave(int devn, uint8_t *slave) {
    nxpTfaDeviceList_t *dev = tfaContDevice (devn);

    if (dev==0 ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    *slave = dev->dev;
    return Tfa98xx_Error_Ok;
}
/**
 * print all the bitfields of the register
 * @param fd output file
 * @param reg address
 * @param regval register value
 * @return 0 if at least 1 name was found
 */
int tfaRunBitfieldDump(FILE *fd, unsigned char reg, unsigned short regval ) {
    union {
        uint16_t field;
        nxpTfaBfEnum_t Enum;
    } bfUni;
    uint16_t     mask;
    int n=sizeof(TfaBfNames)/sizeof(tfaBfName_t)-1; // end of list
    int havename=0;
    do {
        bfUni.field = TfaBfNames[n].bfEnum;
        if (reg==0x73){
            mask=1;
        }
        if ( bfUni.Enum.address == reg ){
            mask = (1<<(bfUni.Enum.len+1))-1;
            PRINT("%s:%d ", TfaBfNames[n].bfName, (regval>>bfUni.Enum.pos) & mask);
            havename=1;
        }
    }
    while( n--);
    PRINT("\n");
    return !havename==1; // name
    //PRINT_FILE(fd, "invalid bitfield; reg=0x%0x value=0x%0x\n", reg, regval);
}

/*
 * write a bit field
 */
Tfa98xx_Error_t tfaRunWriteBitfield(Tfa98xx_handle_t handle,  nxpTfaBitfield_t bf) {
    Tfa98xx_Error_t error;
    union {
        uint16_t field;
        nxpTfaBfEnum_t Enum;
    } bfUni;
    uint16_t value, oldvalue, msk;

    value=bf.value;
    bfUni.field = bf.field;
    //bfUni.field  &= 0x7fff; //mask of high bit, done before

    if ( tfa98xx_cnt_verbose )
        PRINT("bitfield: %s=%d (0x%x[%d..%d]=0x%x)\n", tfaContBfName(bfUni.field), value,
                                                                            bfUni.Enum.address, bfUni.Enum.pos, bfUni.Enum.pos+bfUni.Enum.len, value);
    if (    ((nxpTfaBfEnum_t*)&bf.field)->address &0x80 ) {
        PRINT("WARNING:not a persistant write of MTP\n");
    }


    error = Tfa98xx_ReadRegister16(handle, (unsigned char)(bfUni.Enum.address), &oldvalue);
    if (error) return error;

    msk = ((1<<(bfUni.Enum.len+1))-1)<<bfUni.Enum.pos;

    oldvalue &= ~msk;
    oldvalue |=(bfUni.Enum.len, value<<bfUni.Enum.pos);
    error = Tfa98xx_WriteRegister16(handle, (unsigned char)(bfUni.Enum.address), oldvalue);

    return error;

}
/*
 * read a bit field
 */
Tfa98xx_Error_t tfaRunReadBitfield(Tfa98xx_handle_t handle,  nxpTfaBitfield_t *bf) {
    Tfa98xx_Error_t error;
    union {
        uint16_t field;
        nxpTfaBfEnum_t Enum;
    } bfUni;
    uint16_t regvalue, msk;

    bfUni.field = bf->field;

    error = Tfa98xx_ReadRegister16(handle, (unsigned char)(bfUni.Enum.address), &regvalue);
    if (error) return error;

    msk = ((1<<(bfUni.Enum.len+1))-1)<<bfUni.Enum.pos;

    regvalue &= msk;
    bf->value = regvalue>>bfUni.Enum.pos;

    return error;

}

/*
 * write the register based on the input address, value and mask
 *  only the part that is masked will be updated
 */
Tfa98xx_Error_t tfaRunWriteRegister(Tfa98xx_handle_t handle, nxpTfaRegpatch_t *reg)
{
    Tfa98xx_Error_t error;
    uint16_t value,newvalue;

    if ( tfa98xx_cnt_verbose )
        PRINT("register: 0x%02x=0x%04x (msk=0x%04x)\n", reg->address, reg->value, reg->mask);

    error = Tfa98xx_ReadRegister16(handle, reg->address, &value);
    if (error) return error;

    value &= ~reg->mask;
    newvalue = reg->value & reg->mask;

    value |= newvalue;
    error = Tfa98xx_WriteRegister16(handle,  reg->address, value);

    return error;

}
/*
 * return the bitfield
 */
nxpTfaBitfield_t tfaContDsc2Bf(nxpTfaDescPtr_t dsc) {
    uint32_t *ptr = (uint32_t *) (&dsc);
    union {
    nxpTfaBitfield_t bf;
    uint32_t num;
    } num_bf;

    num_bf.num = *ptr & TFA_BITFIELDDSCMSK;

    return num_bf.bf;
}
// write  reg  and bitfield items in the devicelist to the target
Tfa98xx_Error_t tfaContWriteRegsDev(int device) {
    nxpTfaDeviceList_t *dev = tfaContDevice (device);
    int i;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if ( !dev ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    /* process the list until a patch, file of profile is encountered */
    for(i=0;i<dev->length;i++) {
        if ( dev->list[i].type == dscPatch ||
              dev->list[i].type ==dscFile ||
              dev->list[i].type ==dscProfile ) break;

        if  ( dev->list[i].type & dscBitfieldBase) {
            err = tfaRunWriteBitfield( device, tfaContDsc2Bf(dev->list[i])); /* */
        }
        if  ( dev->list[i].type == dscRegister ) {
            err = tfaRunWriteRegister( device, (nxpTfaRegpatch_t *)( dev->list[i].offset+(char*)gCont));
        }
        if ( err ) break;
    }
    return err;
}
// write  reg  and bitfield items in the profilelist the target
Tfa98xx_Error_t tfaContWriteRegsProf(int device, int profile) {
    nxpTfaProfileList_t *prof = tfaContProfile( device, profile);
    unsigned int i;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if ( !prof ) {
        return Tfa98xx_Error_Bad_Parameter;
    }
    /* process the list until a patch, file of profile is encountered */
    for(i=0;i<prof->length;i++) {
        if ( prof->list[i].type == dscPatch ||
              prof->list[i].type ==dscFile ||
              prof->list[i].type ==dscProfile ) break;

        if  ( prof->list[i].type & dscBitfieldBase) {
            err = tfaRunWriteBitfield( device , tfaContDsc2Bf(prof->list[i])); /*  */
        }
        if  ( !prof->list[i].type == dscRegister ) {
            err = tfaRunWriteRegister( device, (nxpTfaRegpatch_t *)( !prof->list[i].offset+gCont));
        }
        if ( err ) break;
    }
    return err;
}
// write  patchfile in the devicelist to the target
Tfa98xx_Error_t tfaContWritePatch(int device) {
    nxpTfaDeviceList_t *dev = tfaContDevice ( device);
    nxpTfaFileDsc_t *file;
    nxpTfaPatch_t *patchfile;
    int size;

    int i;

    if ( !dev ) {
        return Tfa98xx_Error_Bad_Parameter;
    }
    /* process the list until a patch  is encountered */
    for(i=0;i<dev->length;i++) {
        if ( dev->list[i].type == dscPatch ) {
            file = (nxpTfaFileDsc_t *)(dev->list[i].offset+(uint8_t *)gCont);
            patchfile =(nxpTfaPatch_t *)&file->data;
            if ( tfa98xx_cnt_verbose ) tfaContShowFile(&patchfile->hdr);
            size = patchfile->hdr.size - sizeof(nxpTfaPatch_t ); // size is total length
            return Tfa98xx_DspPatch(device,  size, (const unsigned char *) patchfile->data);
        }

    }

    return Tfa98xx_Error_Bad_Parameter; // patch not in the list
}
/*
 * write all files and items in the device list to the target
 *  This assumes that the DSP framework is running.
 */
Tfa98xx_Error_t tfaContWriteFiles(int device) {
    nxpTfaDeviceList_t *dev = tfaContDevice ( device);

    Tfa98xx_Error_t err;
    nxpTfaFileDsc_t *file;
    int i;

    if ( !dev ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    /* process the list and write all files  */
    for(i=0;i<dev->length;i++) {
        void *here = dev->list[i].offset+(uint8_t *)gCont; /* location of descriptor in container */

        switch (dev->list[i].type) {
        case dscFile:
            file = (nxpTfaFileDsc_t *)here ;
            err = tfaContWriteFile(device,  file);
            break;
        default:
            /* ignore any other type */
            break;
        }
    }

    return err;
}
/*
 *  write all  param files in the profilelist to the target
 *   this is used during startup when maybe ACS is set
 */
Tfa98xx_Error_t tfaContWriteFilesProf(int device, int profile, int vstep) {
    nxpTfaProfileList_t *prof = tfaContProfile(device, profile);
    unsigned int i;
    nxpTfaFileDsc_t *file;

    if ( !prof ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    /* process the list and write all files  */
    for(i=0;i<prof->length;i++) {
        if ( prof->list[i].type == dscFile ) {
            file = (nxpTfaFileDsc_t *)(prof->list[i].offset+(uint8_t *)gCont);
            if ( tfaContWriteFile(device,  file) ){
                return Tfa98xx_Error_Bad_Parameter;
            }
        }
    }

    return Tfa98xx_Error_Ok;
}

Tfa98xx_Error_t  tfaContWriteItem(int device, nxpTfaDescPtr_t * dsc) {
    nxpTfaFileDsc_t *file;
    nxpTfaRegpatch_t *reg;
    nxpTfaMode_t *cas;

    switch (dsc->type) {
    case dscDevice: // ignore
    case dscProfile:    // profile list
        break;
    case dscRegister:   // register patch
        reg = (nxpTfaRegpatch_t *)(dsc->offset+(uint8_t *)gCont);
        return tfaRunWriteRegister( device, reg);
        //PRINT("$0x%2x=0x%02x,0x%02x\n", reg->address, reg->mask, reg->value);
        break;
    case dscString: // ascii: zero terminated string
        PRINT(";string: %s\n", tfaContGetString(dsc));
        break;
    case dscFile:       // filename + file contents
    case dscPatch:
        file = (nxpTfaFileDsc_t *)(dsc->offset+(uint8_t *)gCont);
        if ( tfa98xx_cnt_verbose ) PRINT("%s=%s\n", tfaContFileTypeName(file), tfaContGetString(&file->name));
        break;
    case dscMode:
        cas = (nxpTfaMode_t *)(dsc->offset+(uint8_t *)gCont);
        if(cas->value == Tfa98xx_Mode_RCV)
            tfa98xx_select_mode(device, Tfa98xx_Mode_RCV);
        else
            tfa98xx_select_mode(device, Tfa98xx_Mode_Normal);
        break;
    default:
        if (dsc->type & dscBitfieldBase) {
            return tfaRunWriteBitfield(device , tfaContDsc2Bf(*dsc));
            //PRINT("%s=%d\n", tfaContBfName(num>>16), (uint16_t)num);
        }
        break;

    }
    return tfa_srv_api_error_Ok;
}
/*
 *  process all items in the profilelist
 *   NOTE an error return during processing will leave the device muted
 *
 */
Tfa98xx_Error_t tfaContWriteProfile(int device, int profile, int vstep)
{
    nxpTfaProfileList_t *prof = tfaContProfile(device, profile);
    unsigned int i;
    nxpTfaFileDsc_t *file;

    if ( !prof ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    tfaRunMute(device); // this will wait for SWS
    Tfa98xx_Powerdown(device, 1);

    /*  process and write all non-file items (e.g. registers) first */
    for(i=0;i<prof->length;i++) {
        if ( ( prof->list[i].type == dscRegister ) ||
            ( prof->list[i].type >= dscBitfieldBase ) )
        {
            if ( tfaContWriteItem(device,  &prof->list[i]) != Tfa98xx_Error_Ok )
                return Tfa98xx_Error_Bad_Parameter;
        }
    }

    /* power up CF (needed for writing file items) */
    tfaRunCfPowerup(device);

    /*  process and write all file items after power up of CF */
    for(i=0;i<prof->length;i++) {
        if ( prof->list[i].type == dscFile ) {
            file = (nxpTfaFileDsc_t *)(prof->list[i].offset+(uint8_t *)gCont);
            if ( tfaContWriteFile(device,  file) )
                return Tfa98xx_Error_Bad_Parameter;
        }
        else
        if ( ( prof->list[i].type != dscRegister ) &&
            ( prof->list[i].type < dscBitfieldBase ) )
        {
            if ( tfaContWriteItem(device,  &prof->list[i]) != Tfa98xx_Error_Ok )
                return Tfa98xx_Error_Bad_Parameter;
        }
    }

    return Tfa98xx_Error_Ok;
}

/*
 *  process only vstep in the profilelist
 *
 */
Tfa98xx_Error_t tfaContWriteFilesVstep(int device, int profile, int vstep) {
    nxpTfaProfileList_t *prof = tfaContProfile(device, profile);
    unsigned int i;
    int pwdn=-1;
    nxpTfaFileDsc_t *file;
    nxpTfaHeader_t *hdr;
    nxpTfaHeaderType_t type;
    Tfa98xx_Error_t err = Tfa98xx_Error_Ok;

    if ( !prof ) {
        return Tfa98xx_Error_Bad_Parameter;
    }

    /* process the list and write all registers,
     *  if  a file is encountered then see if we need to poweron first */
    for(i=0;i<prof->length;i++) {
        if ( prof->list[i].type == dscFile ) {
            if ( pwdn<0)
                pwdn = tfaRunIsPwdn(device) ; // cache the value
            if (pwdn) {
                tfaRunCfPowerup(device);
                pwdn=0;
            }
            file = (nxpTfaFileDsc_t *)(prof->list[i].offset+(uint8_t *)gCont);
            hdr = (nxpTfaHeader_t *)file->data;

            type = (nxpTfaHeaderType_t) hdr->id;
            switch (type) {
            case volstepHdr:
                /* set current volume step to TFA */
                err = tfaContWriteVstep(device, (nxpTfaVolumeStep2File_t *)hdr);
                if (err)
                {
                    return err;
                }
                break;
            default:
                break;
            }
        }
    }

    return Tfa98xx_Error_Ok;
}

char *tfaContGetString(nxpTfaDescPtr_t * dsc)
{
      if ( dsc->type != dscString)
              return nostring;

    return dsc->offset+(char*)gCont;
}

/*
 * show the contents of the header
 */
void tfaContShowHeader(nxpTfaHeader_t *hdr) {
    char _id[2];

    PRINT("File header\n");

    _id[1] = hdr->id >> 8;
    _id[0] = hdr->id & 0xff;
    PRINT("\tid:%.2s version:%.2s subversion:%.2s\n", _id,
           hdr->version, hdr->subversion);
    PRINT("\tsize:%d CRC:0x%08x \n", hdr->size, hdr->CRC);
    PRINT( "\tcustomer:%.8s application:%.8s type:%.8s\n", hdr->customer,
               hdr->application, hdr->type);
}

static void hexdump(char *str, const unsigned char * data, int num_write_bytes) //TODO cleanup/consolidate all hexdumps
{
    int i;

    PRINT("%s", str);
    for(i=0;i<num_write_bytes;i++)
    {
        PRINT("0x%02x ", data[i]);
    }

}
/*
 * display the contents of a DSP message
 *  - input 3 bytes buffer
 */
int tfaContShowDspMsg(uint8_t *msg) {
    struct msg {
        uint8_t mod;
        uint8_t m_id;
        uint8_t p_id;
    } *pmsg;

    pmsg = (struct msg*)msg;

    PRINT("Module: 0x%02x ,M_ID: 0x%02x, P_ID:: 0x%02x\n ", pmsg->mod, pmsg->m_id, pmsg->p_id);

    return 0;
}
void tfaContShowMsg(nxpTfaMsg_t *file) {
    tfaContShowHeader( &file->hdr);

    hexdump("msg code: (lsb first)", file->data, 3);
    PRINT("\n");
    tfaContShowDspMsg(file->data);
}

void tfaContShowEq(nxpTfaEqualizerFile_t *eqf) {
    int ind, i;
    tfaContShowHeader( &eqf->hdr);

    //PRINT("samplerate: %d\n", eqf->samplerate);

//  hexdump("eqf ",(const unsigned char * )&eqf->filter[0],sizeof(eqf->filter));
    for(ind=0;ind<TFA98XX_MAX_EQ;ind++) {   //TODO add rest of filter params
        PRINT("%d: ", ind);
        if (eqf->filter[ind].enabled )
            for(i=0;i<sizeof(nxpTfaBiquad_t);i++)
            {
                PRINT("0x%02x ", eqf->filter[ind].biquad.bytes[i]);
            }
        else PRINT("disabled");
                PRINT("\n");

    }

}
/*
 * volume step2 file (VP2)
 */
void tfaContShowVstep2( nxpTfaVolumeStep2File_t *vp){
    int step, i;
    tfaContShowHeader( &vp->hdr);

    PRINT("samplerate: %d\n", vp->samplerate);

    //  hexdump("eqf ",(const unsigned char * )&eqf->filter[0],sizeof(eqf->filter));
    for(step=0;step<vp->vsteps;step++) {   //TODO add rest of filter params
        PRINT("vstep[%d]: ", step);
        PRINT(" att:%.2f ", vp->vstep[step].attenuation);
        for(i=0;i<TFA98XX_MAX_EQ;i++)
            if (vp->vstep[step].filter[i].enabled) {
                break;
            }
        PRINT(", %s filters enabled", i==TFA98XX_MAX_EQ ? "no":"has");
        PRINT("\n");
    }
}

/*
 *
 */
static char *keyName[] = {
        "params",     /* paramsHdr */
        "vstep",     /* volstepHdr     */
        "patch",     /* patchHdr     */
        "speaker",     /* speakerHdr     */
        "preset",     /* presetHdr   */
        "config",     /* configHdr   */
        "eq",       /* equalizerHdr */
};
char *tfaContFileTypeName(nxpTfaFileDsc_t *file) {
    uint16_t type;
    nxpTfaHeader_t *hdr= (nxpTfaHeader_t *)file->data;


    type = hdr->id;

    switch (type) {
    case paramsHdr:
        return keyName[0];
        break;
    case volstepHdr:
        return keyName[1];
        break;
    case patchHdr:
        return keyName[2];
        break;
    case speakerHdr:
        return keyName[3];
        break;
    case presetHdr:
        return keyName[4];
        break;
    case configHdr:
        return keyName[5];
        break;
    case equalizerHdr:
        return keyName[6];
        break;
    default:
        ERRORMSG("File header has of unknown type: %x\n",type);
        return nostring;
    }
}

/*
 * Get the name of the device at certain index in the container file
 *  return device name
 */
char  *tfaContDeviceName(int idx) {
    nxpTfaDeviceList_t *dev;

    if ( idx >= tfa98xx_cnt_max_device() )
        return errorname;

    if ( (dev = tfaContDevice(idx)) == NULL )
        return errorname;

    return tfaContGetString(&dev->name);
}

/*
 * Get the name of the profile at certain index for a device in the container file
 *  return profile name
 */
char  *tfaContProfileName(int idx, int ipx) {
    nxpTfaProfileList_t *prof;

    if ( idx >= tfa98xx_cnt_max_device() )
        return errorname;
    if ( ipx >= tfaContMaxProfile(idx) )
        return errorname;

    // the Nth profiles for this device
    prof=tfaContGetDevProfList(gCont, idx, ipx);
    return tfaContGetString(&prof->name);
}

/**
 * print current bitfield value
 */
enum Tfa98xx_Error tfaConfDumpBf(nxpTfaBitfield_t bf) {
    Tfa98xx_Error_t err = tfa_srv_api_error_Ok;
    int dev, devcount = tfa98xx_cnt_max_device();

    if ( devcount <= 0 ) {
        PRINT_ERROR("No or wrong container file loaded\n");
        return  Tfa98xx_Error_Bad_Parameter;
    }

    for( dev=0; dev < devcount; dev++) {
        err = tfaContOpen(dev);
        if ( err != Tfa98xx_Error_Ok)
            goto error_exit;
        if ( tfa98xx_cnt_verbose )
            PRINT("bf device [%s]\n", tfaContDeviceName(dev));
        err = tfaRunReadBitfield(dev, &bf);
        if ( err != Tfa98xx_Error_Ok)
            goto error_exit;
        PRINT("[%s] %s:%d\n", tfaContDeviceName(dev), tfaContBfName(bf.field), bf.value);
    }

error_exit:
    tfaContClose(dev); /* close it */
    return err;
}

tfa_srv_api_error_t tfaCheckStringLength(char *str, char **strings, int *length, int maxlength)
{
    *length = (int)(strlen(str));

    if ( *length > maxlength )
        return tfa_srv_api_error_BadParam; // max length too short

    strcpy(*strings, str);
    *strings += *length;

    return tfa_srv_api_error_Ok;
}


tfa_srv_api_error_t  tfaContShowItem(nxpTfaDescPtr_t *dsc, char **strings, int *length, int maxlength)
{
    tfa_srv_api_error_t err;
    uint32_t *ptr = (uint32_t *) dsc;
    uint32_t num = *ptr;
    nxpTfaFileDsc_t *file;
    nxpTfaProfileList_t *prof;
    nxpTfaRegpatch_t *reg;
    nxpTfaMode_t *cas;
    char str[NXPTFA_MAXLINE];

    switch (dsc->type) {
        case dscDevice: // device list
            sprintf(str, "device\n");
            break;
        case dscProfile:    // profile list
            prof = (nxpTfaProfileList_t *)(dsc->offset+(uint8_t *)gCont);
            sprintf(str, "profile=%s\n",  tfaContGetString(&prof->name));
            break;
        case dscMode:
            cas = (nxpTfaMode_t *)(dsc->offset+(uint8_t *)gCont);
            sprintf(str, "mode=%d\n", cas->value);
            break;
        case dscRegister:   // register patch
            reg = (nxpTfaRegpatch_t *)(dsc->offset+(uint8_t *)gCont);
                        if(reg->mask == 0xffff)
                                sprintf(str, "$0x%x=0x%02x\n", reg->address, reg->value);
                        else
                    sprintf(str, "$0x%x=0x%02x,0x%02x\n", reg->address, reg->value, reg->mask);
            break;
        case dscString: // ascii: zero terminated string
            sprintf(str, ";string: %s\n", tfaContGetString(dsc));
            break;
        case dscFile:       // filename + file contents
        case dscPatch:
            file = (nxpTfaFileDsc_t *)(dsc->offset+(uint8_t *)gCont);
            sprintf(str, "%s=%s\n", tfaContFileTypeName(file), tfaContGetString(&file->name));
            break;
        default:
            if (dsc->type & dscBitfieldBase) {
                num &= 0x7fffffff; //mask of high bit
                sprintf(str, "%s=%d\n", tfaContBfName(num>>16),  (uint16_t)(0xffff & num));
            }
            break;
    }

    err = tfaCheckStringLength(str, strings, length, maxlength);
    return err;
}

/*
 * show the contents of the local container
 * Takes length and char pointer to store the ini file contents
 * Return error code
 */
tfa_srv_api_error_t tfaContShowContainer( char *strings, int maxlength )
{
    tfa_srv_api_error_t err = tfa_srv_api_error_Ok;
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *prof;
    char str[NXPTFA_MAXLINE];
    int length = 0;
    int i;
    unsigned int j;

    sprintf(str, "[system]\n" "customer=%.8s\n" "application=%.8s\n" "type=%.8s\n", gCont->customer, gCont->application, gCont->type);
    err = tfaCheckStringLength(str, &strings, &length, maxlength);

    for(i=0; i<gCont->ndev; i++) {
        if ( (dev=tfaContDevice(i)) !=NULL) {
            sprintf(str, "device=%s\n", tfaContGetString(&dev->name));
            err = tfaCheckStringLength(str, &strings, &length, maxlength);
        }
    }

    sprintf(str, "\n");
    err = tfaCheckStringLength(str, &strings, &length, maxlength);

    // show devices
    for(i=0; i<gCont->ndev; i++) {
     if ( (dev=tfaContDevice(i)) !=NULL) {

        tfaContShowDevice(i, &strings, &length, maxlength);

        for (j=0; j < dev->length; j++ )
            tfaContShowItem(&dev->list[j], &strings, &length, maxlength);

        sprintf(str, "\n");
        err = tfaCheckStringLength(str, &strings, &length, maxlength);
     }
    }

//  show all profiles
    prof=tfaContGet1stProfList(gCont);

    for (i=0;i < gCont->nprof;i++) {
        sprintf(str, "[%s]\n", tfaContGetString(&prof->name));
        err = tfaCheckStringLength(str, &strings, &length, maxlength);

        // next profile
        for(j=0; j< prof->length-1; j++) //note that the proflist length includes the name item
            tfaContShowItem(&prof->list[j], &strings, &length, maxlength);

        sprintf(str, "\n");
        err = tfaCheckStringLength(str, &strings, &length, maxlength);
        prof = tfaContNextProfile(prof);
    }

    *strings = '\0';

    return err;
}

/*
 * return 1st profile list
 */
nxpTfaProfileList_t *tfaContGet1stProfList(nxpTfaContainer_t * cont)
{
    nxpTfaProfileList_t *prof;
    uint8_t *b = (uint8_t *) cont;

    int maxdev = 0;
    nxpTfaDeviceList_t *dev;

    // get nr of devlists+1
    maxdev = cont->ndev;
    // get last devlist
    dev = tfaContGetDevList(cont, maxdev - 1);
    // the 1st profile starts after the last device list
    b = (uint8_t *) dev + 3*sizeof(nxpTfaDescPtr_t) +
        dev->length * (sizeof(nxpTfaDescPtr_t));
    prof = (nxpTfaProfileList_t *) b;
    return prof;
}

/*
 * Note that the current handle implementation in the API will open the next
 * free handle, it may be out of order if not properly looked after.
 * In future this will change.
 *
 * For now this open() will assure that the handle nr matches the device index
 */
enum Tfa98xx_Error tfaContOpen(int device) {
    Tfa98xx_handle_t handle;
    Tfa98xx_Error_t err;
    uint8_t slave;
    int i;

    err = tfaContGetSlave(device , &slave);
    if ( err != Tfa98xx_Error_Ok )
        return err;

    for(i=0;i<Tfa98xx_MaxDevices();i++) {
        err = Tfa98xx_Open( slave<<1, &handle );
        if ( err != Tfa98xx_Error_Ok )
            return err;
        if ( handle==device)
            return Tfa98xx_Error_Ok;
    }
    ERRORMSG("handle!=devnr");

    return Tfa98xx_Error_OutOfHandles;
}

enum Tfa98xx_Error tfaContClose(int device) {
    return Tfa98xx_Close(device);
}
/*
 * return the device count in the container file
 */
int tfa98xx_cnt_max_device(void) {
    return gDevs;
}
/*
 * lookup slave and return device index
 */
int tfa98xx_cnt_slave2idx(int slave) {
    int idx;

    for(idx=0;idx<gDevs;idx++) {
        if (gDev[idx]->dev == slave )
            return idx;
    }

    return -1;
}
/*
 * return the device list pointer
 */
nxpTfaDeviceList_t *tfaContDevice(int idx) {
    if ( idx < gDevs)
        return gDev[idx];
    //ERRORMSG("Devlist index too high:%d!", idx);
    return NULL;
}
/*
 * return the per device profile count
 */
int tfaContMaxProfile(int ndev) {

    if ( ndev >= gDevs) {
        //ERRORMSG("Devlist index too high:%d!", ndev);
        return 0;
    }

    return gProfs[ndev];
}
/*
 * return the next profile
 */
nxpTfaProfileList_t* tfaContNextProfile(nxpTfaProfileList_t* prof) {
    nxpTfaProfileList_t* nextprof = (nxpTfaProfileList_t *)( (char*)prof + (prof->length*4) + sizeof(nxpTfaProfileList_t) -4);

    if (nextprof->ID == TFA_PROFID)
        return nextprof;

    return NULL;

}
/*
 * return the device list pointer
 */
nxpTfaProfileList_t* tfaContProfile(int ndev, int nprof) {
    if ( ndev >= gDevs) {
        //ERRORMSG("Devlist index too high:%d!", ndev);
        return NULL;
    }
    if ( nprof >= gProfs[ndev]) {
        //ERRORMSG("Proflist index too high:%d!", nprof);
        return NULL;
    }

        return gProf[ndev][nprof];


}
/* return the speakerbuffer for this device */
uint8_t *tfacont_speakerbuffer(int device) {
    nxpTfaDeviceList_t *dev;
    nxpTfaFileDsc_t *file;
    nxpTfaHeader_t *hdr;
    nxpTfaSpeakerFile_t *speakerfile;
    int i;

    if( gCont==0 )
        return NULL;

    dev = tfaContGetDevList(gCont, device);
    if( dev==0 )
        return NULL;

    /* find the 1swt
     *
     */
    /* process the list until a file  is encountered */
    for(i=0;i<dev->length;i++) {
        if ( dev->list[i].type == dscFile ) {
            file = (nxpTfaFileDsc_t *)(dev->list[i].offset+(uint8_t *)gCont);
            hdr= (nxpTfaHeader_t *)file->data;
            /* check if speakerfile */
            if ( hdr->id == speakerHdr) {
                speakerfile =(nxpTfaSpeakerFile_t *)&file->data;
                return speakerfile->data;
            }
        }
    }

    if ( tfa98xx_cnt_verbose )
        PRINT("%s: no speakfile found\n", __FUNCTION__);

    return NULL;
}

enum Tfa98xx_Error tfa98xx_header_check(char *fname, nxpTfaHeader_t *buf)
{
        Tfa98xx_Error_t error = Tfa98xx_Error_Ok;
    nxpTfaHeader_t existingHdr;
    int noHeader = 0;
        int size = 0;
    nxpTfa98xxParamsType_t paramsType;
        FILE *f;

        /*  Read the first x bytes to see if the file has a header */
    f=fopen(fname, "rb");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", fname, strerror(errno));
                fclose(f);
        return Tfa98xx_Error_Bad_Parameter;
    }

    // read the potential header
    fread(&existingHdr , sizeof(existingHdr), 1, f);
    fclose(f);
    paramsType = cliParseFiletype(fname);

    switch ( paramsType ) {
    case tfa_speaker_params:
        if(!HeaderMatches(&existingHdr, speakerHdr))
            noHeader = 1;
        break;
    case tfa_patch_params:
        if(!HeaderMatches(&existingHdr, patchHdr))
            noHeader = 1;
        break;
    case tfa_config_params:
        if(!HeaderMatches(&existingHdr, configHdr))
            noHeader = 1;
        break;

    case tfa_preset_params:
        if(!HeaderMatches(&existingHdr, presetHdr))
            noHeader = 1;
        break;
    case tfa_equalizer_params:// store eq in profile
        if(!HeaderMatches(&existingHdr, equalizerHdr))
            noHeader = 1;
        break;
    case tfa_vstep_params:
        if(!HeaderMatches(&existingHdr, volstepHdr))
            noHeader = 1;
        break;
    default:
        ERRORMSG("Unknown file type:%s \n", fname);
                PRINT("Please check the file type \n");
                error = Tfa98xx_Error_Bad_Parameter;
        break;
    }

        if(error != Tfa98xx_Error_Ok)
                return error;

    if(noHeader) {
        PRINT("File size: %d, does the file contain a header? \n", size);
        return Tfa98xx_Error_Bad_Parameter;
    }

    if(buf->size <= 0) {
        PRINT("\nWarning: %s: Wrong header size: %d. Temporarily changed size to %d! \n\n", fname, buf->size, size);
        buf->size = size;
    }

    if (tfaContCrcCheck(buf)) {
        ERRORMSG("CRC error in %s\n", fname);
        tfaContShowFile(buf);
        error = Tfa98xx_Error_Bad_Parameter;
    }

        return error;
}

void tfa98xx_show_headers(char *fname, nxpTfaHeader_t *buf, nxpTfaHeaderType_t type)
{
    switch (type) {
                case volstepHdr:
                    if ( tfa98xx_cnt_verbose )
                        tfaContShowVstep2((nxpTfaVolumeStep2File_t *) buf);
                    break;
                case speakerHdr:
                    if ( tfa98xx_cnt_verbose )
                        tfaContShowSpeaker((nxpTfaSpeakerFile_t *) buf);
                    break;
                case msgHdr:
                    if ( tfa98xx_cnt_verbose )
                        tfaContShowMsg((nxpTfaMsg_t *) buf);
                    break;
                case equalizerHdr:
                    if ( tfa98xx_cnt_verbose )
                        tfaContShowEq((nxpTfaEqualizerFile_t *) buf);
                    break;
                default:
                        if ( tfa98xx_cnt_verbose )
                                tfaContShowFile(buf);
                        break;
    }
}

/*
 * check file name
 * allocate size
 * read in file
 * return size or 0 on failure
 * NOTE: the caller must free the buffer if size is non-0
 */
int  tfaReadFile(char *fname, void **buffer) {
    int size;
    FILE *f = NULL;

    size = fsize(fname);
    if ( !size ) {
        ERRORMSG("Can't open %s.\n", fname);
        size = 0;
        goto stop;
    }

        /* Because there is never a free() for the buffer.
         * This needs to be done when the cnt file is loaded multiple times
         * Else this will cause memory leaks!
         */
    if(gCont != NULL) {
        if(*buffer != NULL) free(*buffer);
    }

    *buffer = malloc(size);
    if ( !*buffer ) {
        ERRORMSG("Can't allocate %d bytes.\n", size);
        return 0;
    }

    f=fopen(fname, "rb");
    if ( !f ) {
        ERRORMSG("Error when reading %s.\n", fname);
        free(*buffer);
        size = 0;
        goto stop;
    }

    fread(*buffer, size, 1, f);

stop:
    if(NULL != f) fclose(f);
    return size;
}
/*
 *   load the file depending on its type
 *   normally the container will be loaded
 *   any other valid type will by send to the target device(s)
 */
int tfa98xx_cnt_loadfile(char *fname, int cnt_verbose) {
    int length, size = 0;
    nxpTfaHeader_t *buf = 0;
    nxpTfaHeaderType_t type;
    tfa_srv_api_error_t error = tfa_srv_api_error_Ok;
    char buffer[4*1024];    //,byte TODO check size or use malloc

    size = tfaReadFile(fname, (void**) &buf); //mallocs file buffer

    if (size == 0)
                return 0; // tfaReadFile reported error already

    type = (nxpTfaHeaderType_t) buf->id;
    if (type == paramsHdr) { /* Load container file */
        gCont = (nxpTfaContainer_t*) buf;
        size = tfaContLoadContainer(fname);
        if (cnt_verbose) {
            error = tfaContShowContainer(buffer, sizeof(buffer));
            length = (int)(strlen(buffer));
            *(buffer+length) = '\0';        // terminate
            puts(buffer);
        }
        return size; // return here  gCont should not be freed
    } else { /* Header is checked and displayed for other files then .cnt */
                error = tfa98xx_header_check(fname, buf);
                if(error == Tfa98xx_Error_Ok) {
                        tfa98xx_show_headers(fname, buf, type);
                }
        }

    free(buf);
    return size;
}

static int tfaContLoadContainer(char *fname)
{
    // check type
    if ( (HDR(gCont->id[0],gCont->id[1])) != paramsHdr ) {
            ERRORMSG(" %s wrong file type.\n", fname);
            return 0;
    }
    if ( gCont->version[0] !=  NXPTFA_PM_VERSION) {
            ERRORMSG(" %s wrong file type version.\n", fname);
            return 0;
    }
    // check CRC

    if ( tfaContCrcCheckContainer(gCont)) {
        ERRORMSG(" %s CRC error.\n", fname);
        return 0;
    }

    contGetDevs(gCont); /* set globals */

    return gCont->size;
}

/*
 * check CRC for container
 *   CRC is calculated over the bytes following the CRC field
 *
 *   return 0 on error
 */
int tfaContCrcCheckContainer(nxpTfaContainer_t *cont) {
    uint8_t *base;
    int size;
    uint32_t crc;

    base = (uint8_t *)&cont->CRC + 4; // ptr to bytes following the CRC field
    size = (int)(cont->size - (base - (uint8_t *)cont)); // nr of bytes following the CRC field
    crc = tfaContCRC32(base, size, 0);

    return crc != cont->CRC;

}
/*
 * check CRC for file
 *   CRC is calculated over the bytes following the CRC field
 *
 *   return 0 on error
 *
 *   NOTE the container has another check function because the header differs
 */
int tfaContCrcCheck(nxpTfaHeader_t *hdr) {
    uint8_t *base;
    int size;
    uint32_t crc;

    base = (uint8_t *)&hdr->CRC + 4; // ptr to bytes following the CRC field
    size = (int)(hdr->size - (base - (uint8_t *)hdr)); // nr of bytes following the CRC field

    if(size > 0) {
        crc = tfaContCRC32(base, size, 0);
    }
    else {
        PRINT("Error: invalid CRC size: %d, please check the header\n", size);
        return 0;
    }

    return crc != hdr->CRC;

}
#if 1
/*-
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 *
 *  First, the polynomial itself and its table of feedback terms.  The
 *  polynomial is
 *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 *  Note that we take it "backwards" and put the highest-order term in
 *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 *  the MSB being 1
 *
 *  Note that the usual hardware shift register implementation, which
 *  is what we're using (we're merely optimizing it by doing eight-bit
 *  chunks at a time) shifts bits into the lowest-order term.  In our
 *  implementation, that means shifting towards the right.  Why do we
 *  do it this way?  Because the calculated CRC must be transmitted in
 *  order from highest-order term to lowest-order term.  UARTs transmit
 *  characters in order from LSB to MSB.  By storing the CRC this way
 *  we hand it to the UART in the order low-byte to high-byte; the UART
 *  sends each low-bit to hight-bit; and the result is transmission bit
 *  by bit from highest- to lowest-order term without requiring any bit
 *  shuffling on our part.  Reception works similarly
 *
 *  The feedback terms table consists of 256, 32-bit entries.  Notes
 *
 *      The table can be generated at runtime if desired; code to do so
 *      is shown later.  It might not be obvious, but the feedback
 *      terms simply represent the results of eight shift/xor opera
 *      tions for all combinations of data and CRC register values
 *
 *      The values must be right-shifted by eight bits by the "updcrc
 *      logic; the shift must be unsigned (bring in zeroes).  On some
 *      hardware you could probably optimize the shift in assembler by
 *      using byte-swap instructions
 *      polynomial $edb88320
 *
 *
 * CRC32 code derived from work by Gary S. Brown.
 */

//#include <sys/param.h>
//#include <sys/systm.h>

static uint32_t crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t
tfaContCRC32 (uint8_t *buf, uint32_t size, uint32_t crc)
{
    const uint8_t *p;

    p = buf;
    crc = crc ^ ~0U;

    while (size--)
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;
}
#endif
/**************************************************************************************
*/
#define CRCpoly 0xEDB88320uL
/* Some compilers need
   #define poly 0xEDB88320uL
 */

/* On entry, addr=>start of data
             num = length of data
             crc = incoming CRC     */
uint32_t tfaContCRC32_wrong(uint8_t * addr, uint32_t num, uint32_t crc)
{
    int i;

    for (; num > 0; num--) {    /* Step through bytes in memory */
        crc = crc ^ *addr++;    /* Fetch byte from memory, XOR into CRC */
        for (i = 0; i < 8; i++) {   /* Prepare to rotate 8 bits */
            if (crc & 1)    /* b0 is set... */
                crc = (crc >> 1) ^ CRCpoly; /* rotate and XOR with ZIP polynomic */
            else    /* b0 is clear... */
                crc >>= 1;  /* just rotate */
            /* Some compilers need:
               crc &= 0xFFFFFFFF;
             */
        }       /* Loop for 8 bits */
    }           /* Loop until num=0 */
    crc = crc ^ ~0U; /* Pico inverted the crc */
    return (crc);       /* Return updated CRC */
}
