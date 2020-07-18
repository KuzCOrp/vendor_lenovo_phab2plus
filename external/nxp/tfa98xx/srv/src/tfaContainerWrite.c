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
#if !(defined(WIN32) || defined(_X64))
#include <libgen.h>
#endif
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>//TODO move to tfa api
#include "tfa98xxParameters.h"
#include "nxpTfa98xx.h"
#include "dbgprint.h"
#include "tfaContainer.h"
#include "tfaOsal.h"
#include "tfaFieldnames.h"
#include "minIni.h"

#define VERBOSE if ( tfa98xx_cnt_verbose )
#define TFA98XX_FILENAME_MAX    (4096)

/* Tfa98xx_Mode_t to string mapping */
static const char *tfa98xx_mode_str[] = {
    "normal",
    "rcv",
    NULL
};

static char sg_cntFile[TFA98XX_FILENAME_MAX];
static char sg_full[TFA98XX_FILENAME_MAX];

void tfa_cont_write_verbose(int verbose) {
    tfa98xx_cnt_verbose = verbose;
}

void tfa_cont_dev_type(int type) {
    tfa98xx_dev_type = type;
}

//#define DUMP
#ifdef DUMP
static void dumpDsc1st(nxpTfaDescPtr_t * dsc);
static void dumpStrings(void);
static void dumpDevs(nxpTfaContainer_t * cont);
static void dump(nxpTfaContainer_t * cont, int len);
#else
//static void dumpDsc1st(nxpTfaDescPtr_t * dsc){};
//static void dumpStrings(void){};
//static void dumpDevs(nxpTfaContainer_t * cont){};
static void dump(nxpTfaContainer_t * cont, int len){};
#endif

const char *tfaContGetStringPass1(nxpTfaDescPtr_t * dsc);

static int fsize(const char *name)
{
    struct stat st;
    stat(name, &st);
    return st.st_size;
}
static void fillSpeaker(nxpTfaSpeakerFile_t *spk, int argc, char *argv[]) {
    int i=3;

    if (argc > 3) strncpy( spk->name, argv[i++], sizeof(spk->name)  );
    if (argc > 4) strncpy( spk->vendor, argv[i++], sizeof(spk->vendor)  );
    if (argc > 5) strncpy( spk->type, argv[i++], sizeof(spk->type)  );
    if (argc > 6) spk->height =atoi(argv[i++]);
    if (argc > 7) spk->width =atoi(argv[i++]);
    if (argc > 8) spk->depth =atoi(argv[i++]);
    if (argc > 9) spk->ohm =atoi(argv[i++]);
}

#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
static void fillSpeakerMax2(struct nxpTfaSpeakerFileMax2 *spk, int argc, char *argv[]) {
    int i=3;

    if (argc > 3) strncpy( spk->name, argv[i++], sizeof(spk->name)  );
    if (argc > 4) strncpy( spk->vendor, argv[i++], sizeof(spk->vendor)  );
    if (argc > 5) strncpy( spk->type, argv[i++], sizeof(spk->type)  );
    if (argc > 6) spk->height =atoi(argv[i++]);
    if (argc > 7) spk->width =atoi(argv[i++]);
    if (argc > 8) spk->depth =atoi(argv[i++]);
    if (argc > 9) spk->ohm =atoi(argv[i++]);
}
#endif

static int tfaRename(char *fname, const char *ext) {
    char oldfname[FILENAME_MAX];
    /* rename the old file */
    strcpy( oldfname, fname);
    strcat(oldfname, ext);
    if(rename(fname, oldfname) == 0)
    {
        PRINT("Renamed %s to %s.\n", fname, oldfname);
    }
    else
    {
        ERRORMSG("Error renaming %s to %s (%s).\n", fname, oldfname, strerror(errno));
        return 1;
    }

    return 0;
}
//TODO fix trunk
#define FIXEDPT float
#define BIQUAD_COEFF_SIZE 6
/*
 * This function will be deprecated.
 * Floating point calculations must be done in user-space
 *
 * NOTE: that this function is used by a Windows tool
 */
tfa_srv_api_error_t tfa98xxCalcBiquadCoeff(FIXEDPT b0, FIXEDPT b1, FIXEDPT b2,
                FIXEDPT a1, FIXEDPT a2, unsigned char *bytes)
{
        FIXEDPT max_coeff;
        //FIXEDPT mask;
        int headroom;
        int coeff_buffer[BIQUAD_COEFF_SIZE];
        /* find max value in coeff to define a scaler */
#ifdef __KERNEL__
        max_coeff = abs(b0);
        if (abs(b1) > max_coeff)
                max_coeff = abs(b1);
        if (abs(b2) > max_coeff)
                max_coeff = abs(b2);
        if (abs(a1) > max_coeff)
                max_coeff = abs(a1);
        if (abs(a2) > max_coeff)
                max_coeff = abs(a2);
        /* round up to power of 2 */
        mask = 0x0040000000000000;
        for(headroom=23;headroom>0;headroom--)
        {
            if(max_coeff & mask)
            {
                break;
            }
            mask >>= 1;
        }
#else
        max_coeff = (float)fabs(b0);
        if (fabs(b1) > max_coeff)
                max_coeff = (float)fabs(b1);
        if (fabs(b2) > max_coeff)
                max_coeff = (float)fabs(b2);
        if (fabs(a1) > max_coeff)
                max_coeff = (float)fabs(a1);
        if (fabs(a2) > max_coeff)
                max_coeff = (float)fabs(a2);
        /* round up to power of 2 */
        headroom = (int)ceil(log(max_coeff + pow(2.0, -23)) / log(2.0));
#endif
        /* some sanity checks on headroom */
        if (headroom > 8)
                return Tfa98xx_Error_Bad_Parameter;
        if (headroom < 0)
                headroom = 0;
        /* set in correct order and format for the DSP */
        coeff_buffer[0] = headroom;
        coeff_buffer[1] = (int) (-a2 * (1 << (23 - headroom)));
        coeff_buffer[2] = (int) (-a1 * (1 << (23 - headroom)));
        coeff_buffer[3] = (int) (b2 * (1 << (23 - headroom)));
        coeff_buffer[4] = (int) (b1 * (1 << (23 - headroom)));
        coeff_buffer[5] = (int) (b0 * (1 << (23 - headroom)));
/*
#ifdef __KERNEL__
        coeff_buffer[1] = (int) TO_INT(-a2 * (1 << (23 - headroom)));
        coeff_buffer[2] = (int) TO_INT(-a1 * (1 << (23 - headroom)));
        coeff_buffer[3] = (int) TO_INT(b2 * (1 << (23 - headroom)));
        coeff_buffer[4] = (int) TO_INT(b1 * (1 << (23 - headroom)));
        coeff_buffer[5] = (int) TO_INT(b0 * (1 << (23 - headroom)));
#else
        coeff_buffer[1] = (int) (-a2 * pow(2.0, 23 - headroom));
        coeff_buffer[2] = (int) (-a1 * pow(2.0, 23 - headroom));
        coeff_buffer[3] = (int) (b2 * pow(2.0, 23 - headroom));
        coeff_buffer[4] = (int) (b1 * pow(2.0, 23 - headroom));
        coeff_buffer[5] = (int) (b0 * pow(2.0, 23 - headroom));
#endif
*/

        /* convert to fixed point and then bytes suitable for transmission over I2C */
        Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
        return Tfa98xx_Error_Ok;
}

static void convert_step(nxpTfaOldVolumeStep2_t *old, nxpTfaVolumeStep2_t *newVS) {
    int i;

    newVS->attenuation = old->attenuation;
    memcpy(newVS->preset, old->preset, sizeof(newVS->preset));
    for(i=0;i<TFA98XX_MAX_EQ;i++) {
        newVS->filter[i].enabled = old->eq[i].enabled;
        newVS->filter[i].type = old->eq[i].type;
        newVS->filter[i].Q = (float)old->eq[i].Q;
        newVS->filter[i].frequency = (float)old->eq[i].frequency;
        newVS->filter[i].gain = (float)old->eq[i].gain;
        tfa98xxCalcBiquadCoeff((float)old->eq[i].bq[0],
                (float)old->eq[i].bq[1],
                (float)old->eq[i].bq[2],
                (float)old->eq[i].bq[3],
                (float)old->eq[i].bq[4],
                (uint8_t*)&newVS->filter[i].biquad);

    }
}
enum TFA98XX_SAMPLERATE{
    TFA98XX_SAMPLERATE_08000 = 0,
    TFA98XX_SAMPLERATE_11025 = 1,
    TFA98XX_SAMPLERATE_12000 = 2,
    TFA98XX_SAMPLERATE_16000 = 3,
    TFA98XX_SAMPLERATE_22050 = 4,
    TFA98XX_SAMPLERATE_24000 = 5,
    TFA98XX_SAMPLERATE_32000 = 6,
    TFA98XX_SAMPLERATE_44100 = 7,
    TFA98XX_SAMPLERATE_48000 = 8
};
uint8_t  tfa98xx_fs2enum(int rate) {
    uint8_t value;

    switch (rate) {
    case 48000:
            value =  TFA98XX_SAMPLERATE_48000;
            break;
    case 44100:
            value =  TFA98XX_SAMPLERATE_44100;
            break;
    case 32000:
            value =  TFA98XX_SAMPLERATE_32000;
            break;
    case 24000:
            value =  TFA98XX_SAMPLERATE_24000;
            break;
    case 22050:
            value =  TFA98XX_SAMPLERATE_22050;
            break;
    case 16000:
            value =  TFA98XX_SAMPLERATE_16000;
            break;
    case 12000:
            value =  TFA98XX_SAMPLERATE_12000;
            break;
    case 11025:
            value =  TFA98XX_SAMPLERATE_11025;
            break;
    case 8000:
            value =  TFA98XX_SAMPLERATE_08000;
            break;
    default:
            ERRORMSG("unsupported samplerate : %d\n", rate);
            value = -1;
    }
    return value;
}
/*
 * convert vstep V to current
 */
static int ConvertOldVstep(char *fname, int argc, char *argv[]) {
    nxpTfaHeader_t *hdr;
    nxpTfaOldVolumeStep2File_t *vp2_in;
    nxpTfaOldVolumeStep2_t *oldVstep;
    nxpTfaVolumeStep2File_t *vp2;
    int samplerate;
    uint8_t *inbuf, *base;
    uint32_t crc;
    int i, size=fsize(fname), nsteps;
    FILE *f, *bf;
    char vp2File[FILENAME_MAX], *dot;


    strcpy(vp2File,  basename(fname));
    dot  = strrchr(vp2File, '.');
    if(dot) {
        *dot='\0';
    }
    strcat(vp2File,  ".vstep");

    if (!size) {
        ERRORMSG("Something wrong with %s.\n", fname);
        return 0;
    }
    inbuf=(uint8_t *)malloc(size);
    if ( !inbuf ) {
        ERRORMSG("Can't can't allocate %d bytes for %s.\n", size, fname);
        return 0;
    }
    f=fopen(fname, "r+b");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", fname, strerror(errno));
        free(inbuf);
        return 0;
    }

    // read old file
    if (!fread(inbuf , size, 1, f)) {
        ERRORMSG("Can't read from %s (%s).\n", fname, strerror(errno));
        free(inbuf);
        return 0;
    }

    fclose(f);

    hdr=(nxpTfaHeader_t *)inbuf;

    crc =tfaContCRC32(inbuf+sizeof(nxpTfaOldHeader_t), hdr->size, 0xFFFFFFFF); // nr of bytes following the CRC field, 0);
    if(crc!= hdr->CRC) {
        ERRORMSG("CRC error in %s.\n", fname);
        free(inbuf);
        return 0;
    }

    if  ( strncmp("VP2_00" , (char*)inbuf , 6 )) {
        ERRORMSG("Header version of %s does not match VP2_00\n", fname);
        free(inbuf);
        return 0;
    }

    vp2_in = (nxpTfaOldVolumeStep2File_t *)inbuf;

    nsteps = vp2_in->hdr.size / sizeof(nxpTfaOldVolumeStep2_t);
    vp2 = (nxpTfaVolumeStep2File_t *)malloc(sizeof(nxpTfaVolumeStep2File_t)+nsteps*(sizeof(nxpTfaVolumeStep2_t)));

    if ( !vp2 ) {
        ERRORMSG("Can't allocate %d bytes.\n", (int)(sizeof(nxpTfaVolumeStep2File_t)+nsteps*(sizeof(nxpTfaVolumeStep2_t))));
        free(inbuf);
        return 0;
    }
    /*
     * header
     */
    strcpy( (char*)&vp2->hdr.id, "VP");
    strcpy( vp2->hdr.version , "2_");
    strcpy( vp2->hdr.subversion , "01");

    if (argc > 1) strncpy( vp2->hdr.customer , argv[0], sizeof(vp2->hdr.customer)  );
    if (argc > 2) strncpy( vp2->hdr.application , argv[1], sizeof(vp2->hdr.application)  );
    if (argc > 3) strncpy( vp2->hdr.type , argv[2], sizeof(vp2->hdr.type)  );
    samplerate = (argc > 3) ?  atoi(argv[3] ) :  -1;

    vp2->hdr.size = (uint16_t)(sizeof(nxpTfaVolumeStep2File_t)+nsteps*sizeof(nxpTfaVolumeStep2_t));
    vp2->vsteps = nsteps;

    if ( samplerate > 0 ) {
        if (samplerate < 100) // it's already an enum
            vp2->samplerate = samplerate; // just copy
        else // translate to enum
            vp2->samplerate = tfa98xx_fs2enum(samplerate);
    } else
        vp2->samplerate = -1; // not defined

    for(i=0;i<nsteps;i++){
        oldVstep = &vp2_in->step[i];
        convert_step(oldVstep, &vp2->vstep[i]);
        PRINT("v=%f f=%f, ena=%d\n", oldVstep->attenuation, oldVstep->eq[0].frequency,oldVstep->eq[0].enabled);
    }

    free(inbuf);
    /*
     * calc CRC over bytes following the CRC field
     */
    base = (uint8_t *)&vp2->hdr.CRC + 4; // ptr to bytes following the CRC field
    size = (int)(vp2->hdr.size - (base-(uint8_t *)&vp2->hdr)); // nr of bytes following the CRC field
    vp2->hdr.CRC = tfaContCRC32(base, size, 0); // nr of bytes following the CRC field, 0);

    /* write file */
    bf=fopen(vp2File, "w+b");
    if ( !bf ) {
        ERRORMSG("Can't open %s (%s) for writing.\n", vp2File, strerror(errno));
        size=0;
        goto errorVp1;
    }
    if (vp2->hdr.size != fwrite(vp2, 1, vp2->hdr.size , bf )) {
        ERRORMSG("Writing %s (%s).\n", vp2File, strerror(errno));
        size=0;
        goto errorVp1;

    }
    fclose(bf);
    //must rename here. tfaContBin2Hdr() returns after calling this function.
    tfaRename(fname, ".old");
errorVp1:
    free(vp2);
    return size;
}
/*
 * convert assci biquads to binary
 *
 *  the input file must contain 10 bq's, one on each line
 */
static int eq2bin(char *eqFile) {
    char binFile[FILENAME_MAX];
    FILE *f, *bf;
    int size=fsize(eqFile);
    int ind,index, ret=0;
    nxpTfaBiquadFloat_t fbiquad;
    nxpTfaFilter_t filter[TFA98XX_MAX_EQ];

    //bzero((void*)&filter, sizeof(nxpTfaEqualizer_t));
    memset((void*)&filter, 0, sizeof(nxpTfaEqualizer_t));
    strncpy(binFile, eqFile, sizeof(binFile));
    strcat(binFile, ".bin");

    f=fopen(eqFile, "r");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", eqFile, strerror(errno));
        return 0;
    }
    bf=fopen(binFile, "w+b");
    if ( !bf ) {
        ERRORMSG("Can't open %s (%s).\n", binFile, strerror(errno));
        ret=1;
        goto errorEq1;
    }

    for(ind=0;ind<TFA98XX_MAX_EQ;ind++) {
        /*
         *  get the ascii line from the file (1 filter/line)
         *  and convert to float
         *  convert the 32 bit float to 24 bit/3 byte values
         *  append the 5*3 bytes/filter to the filebuffer
         */
        if ( 6!=fscanf(f, "%d %f %f %f %f %f", &index, &fbiquad.b0, &fbiquad.b1,
                &fbiquad.b2, &fbiquad.a1, &fbiquad.a2 ) ) {
            ERRORMSG("Eq format error in file %s, indexnr=%d\n", eqFile, ind);
            ret=1;
            goto errorEq;
        }
        if ( ind+1 != index ) {
            ERRORMSG("Eq format error in file %s, expected nr: %d, read indexnr=%d\n", eqFile, ind+1, index);
            ret=1;
            goto errorEq;
        }
        if (fbiquad.b0==1 && fbiquad.b1==0 && fbiquad.b2==0 && fbiquad.a1==0 && fbiquad.a2==0)
            filter[ind].enabled = 0;
        else {
            /* Calculate Biquad Coeff for binbuf */
            tfa98xxCalcBiquadCoeff( fbiquad.b0,  fbiquad.b1,  fbiquad.b2,
                    fbiquad.a1,  fbiquad.a2, filter[ind].biquad.bytes );
            filter[ind].enabled = 1;
        }
        //        PRINT("%d %f %f %f %f %f\n", ind, filter[ind].biquad.b0, filter[ind].biquad.b1,
        //filter[ind].biquad.b2, filter[ind].biquad.a1, filter[ind].biquad.a2 );
    }
    ret += (int) fwrite((void*)&filter, 1, sizeof(filter), bf );

    errorEq:
    fclose(f);
    errorEq1:
    fclose(bf);

    return ret;

}

int HeaderMatches (nxpTfaHeader_t *hdr, nxpTfaHeaderType_t t) {
    return hdr->id == t && hdr->version[1] == '_';
}

/*
 * Generate a file with header from input binary <bin>.<type> [customer] [application] [type]
 */

int tfaContBin2Hdr(char *inputname, int argc, char *argv[]) {
    nxpTfaHeader_t *hdr, existingHdr;
    nxpTfaContainer_t *gCont;
    nxpTfaSpeakerFile_t head; // note that this is the largest header
    nxpTfaEqualizerFile_t *eqf;
    char fname[FILENAME_MAX];
    nxpTfa98xxParamsType_t paramsType;
    uint8_t *base;
    int i, size, hdrsize, cntFile = 0;
    FILE *f;

#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
    struct nxpTfaSpeakerFileMax2 spkMax2;
#endif


    strncpy(fname, inputname, FILENAME_MAX);

    PRINT("Creating header for %s with args [", fname);
    for (i=0; i<argc; i++)
        PRINT("%s, ", argv[i]);
    PRINT("]\n");

    size = fsize(fname);
    if ( !size ) {
        ERRORMSG("Can't open %s.\n", fname);
        return 0;
    }

    /*  Read the first x bytes to see if the file already has a header
     *  If a header exists, don't add a new one, just replace some fields and recalculate the CRC
     */
    f=fopen(fname, "rb");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", fname, strerror(errno));
        return 0;
    }

    // read the potential header
    fread(&existingHdr , sizeof(existingHdr), 1, f);
    fclose(f);
    paramsType = cliParseFiletype(fname);

    switch ( paramsType ) {
    case tfa_speaker_params:
        head.hdr.id = (uint16_t)speakerHdr;
        head.hdr.version[0] = NXPTFA_SP_VERSION;
        hdrsize = HeaderMatches(&existingHdr, speakerHdr) ? 0 : sizeof(nxpTfaSpeakerFile_t);
#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
    if( tfa98xx_dev_type == 2 ) {        /* max2 */
        spkMax2.hdr.id = (uint16_t)speakerHdr;
        spkMax2.hdr.version[0] = NXPTFA_SP_VERSION_MAX2;
        spkMax2.hdr.version[1] = '_';
        spkMax2.hdr.subversion[0] = '0';
        spkMax2.hdr.subversion[1] = '0';
        spkMax2.hdr.size = hdrsize+size; // add filesize
    }
#endif
        break;
    case tfa_patch_params:
        head.hdr.id = (uint16_t)patchHdr;
        head.hdr.version[0] = NXPTFA_PA_VERSION;
        hdrsize = HeaderMatches(&existingHdr, patchHdr) ? 0 : sizeof(nxpTfaPatch_t);
        break;
    case tfa_config_params:
        head.hdr.id = (uint16_t)configHdr;
        head.hdr.version[0] = NXPTFA_CO_VERSION;
        hdrsize = HeaderMatches(&existingHdr, configHdr) ? 0 : sizeof(nxpTfaConfig_t);
        break;
    case tfa_msg_params:
        head.hdr.id = (uint16_t)msgHdr;
        head.hdr.version[0] = NXPTFA_MG_VERSION;
        hdrsize = HeaderMatches(&existingHdr, msgHdr) ? 0 : sizeof(nxpTfaMsg_t);
        break;

    case tfa_preset_params:
            if ( HeaderMatches(&existingHdr, volstepHdr) ) {
            return ConvertOldVstep( fname, argc, argv);
            // Don't return 0. Zero means error.
            //return 0;
            //return ConvertOldVstep( fname, argc, argv, (argc > 3) ?  atoi(argv[3] ) :  -1);
        }

        head.hdr.id = (uint16_t)presetHdr;
        head.hdr.version[0] = NXPTFA_PR_VERSION;
        hdrsize = HeaderMatches(&existingHdr, presetHdr) ? 0 : sizeof(nxpTfaHeader_t);
        break;
    case tfa_equalizer_params:// store eq in profile
        //ERRORMSG("old eq file can't be converted.\n"); //TODO eq file conversion
        /*size = eq2bin(fname);
        if(!size) return 0; // eq2bin reported error*/
        hdrsize = sizeof(nxpTfaHeader_t)+1; //samplerate byte
        head.hdr.id = (uint16_t)equalizerHdr;
        head.hdr.version[0] = NXPTFA_EQ_VERSION;
        eqf = (nxpTfaEqualizerFile_t*)&head; // NOTE only if head length is > eqf !!
        /* samplerate in eqf does nothing */
        /**
        if (argc > 3)
            eqf->samplerate = (uint8_t)atoi(argv[3]);
        if ( eqf->samplerate > 0 ) {
            if (eqf->samplerate < 100) // it's already an enum
                ; // just keep
            else // translate to enum
                eqf->samplerate = tfa98xx_fs2enum(eqf->samplerate);
        } else
            eqf->samplerate = -1; // not defined
        //nxpTfaEqualizer_t
        **/
        //strcat(fname, ".bin"); // use new file
        //tfa98xxSetEqualizer(-1,)
        break;
    case tfa_vstep_params:
        head.hdr.id = (uint16_t)volstepHdr;
        head.hdr.version[0] = NXPTFA_VP_VERSION;
        hdrsize = HeaderMatches(&existingHdr, volstepHdr) ? 0 : sizeof(nxpTfaHeader_t);
#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
            if ( tfa98xx_dev_type == 2 ) {
                head.hdr.version[0] = NXPTFA_VP2_VERSION_M2;
                head.hdr.subversion[0] = '0';
                head.hdr.subversion[1] = '2';
                head.hdr.version[1] = '_';
                head.hdr.size = hdrsize+size; // add filesize
            }
#endif
        break;
    case tfa_cnt_params:
        cntFile = 1;
        break;
    default:
        ERRORMSG("Unknown file type:%s\n", fname) ;
        return tfa_srv_api_error_BadParam;
        break;
    }

    if(cntFile) {
        gCont = malloc(size);
    } else {
        /* max 1 */
        if(tfa98xx_dev_type == 1) {
            head.hdr.version[1] = '_';
            head.hdr.subversion[0] = '0';
            head.hdr.subversion[1] = '0';
            head.hdr.size = hdrsize+size; // add filesize
        }

        hdr = malloc(hdrsize + size);
        if ( !hdr ) {
            ERRORMSG("Can't allocate %d bytes.\n", hdrsize + size);
            return 0;
        }
    }

    // cpy formatted header to buffer
    if (hdrsize != 0) {
#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
        if(tfa98xx_dev_type == 2 && paramsType == tfa_speaker_params)
            memcpy(hdr, &spkMax2, hdrsize);
        else
#endif
        if(!cntFile) {
            memcpy(hdr, &head, hdrsize);
        }
    }

    if(cntFile)
        gCont->size = size;
    else
        hdr->size = (hdrsize + size);

    f=fopen(fname, "r+b");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", fname, strerror(errno));
        goto stop;
    }

    if(cntFile)
        fread((uint8_t *)gCont, gCont->size, 1, f);
    else
        fread((uint8_t *)hdr+hdrsize, hdr->size, 1, f);

    fclose(f);

    if(paramsType == tfa_vstep_params) {
        if(hdr->size != (hdrsize + size)) {
            PRINT("\nWarning: The size: %d is not correct. Size is overwritten with new size: %d \n\n",
                        hdr->size, (hdrsize + size));
            hdr->size = (hdrsize + size);
        }
    }

    i=0;
    if(cntFile) {
        if (argc > 0) strncpy( gCont->customer, argv[i++], sizeof(gCont->customer)  );
        if (argc > 1) strncpy( gCont->application, argv[i++], sizeof(gCont->application)  );
        if (argc > 2) strncpy( gCont->type, argv[i++], sizeof(gCont->type)  );
    } else {
        if (argc > 0) strncpy( hdr->customer, argv[i++], sizeof(hdr->customer)  );
        if (argc > 1) strncpy( hdr->application, argv[i++], sizeof(hdr->application)  );
        if (argc > 2) strncpy( hdr->type, argv[i++], sizeof(hdr->type)  );
    }
    if (paramsType == tfa_speaker_params) {
#if ( defined( TFA9888 ) || defined( TFA98XX_FULL ))
        if(tfa98xx_dev_type == 2)    /* max2 */
            fillSpeakerMax2((struct nxpTfaSpeakerFileMax2 *) hdr, argc, argv);
#endif
        /* max 1 */
        if(tfa98xx_dev_type == 1 && !cntFile)
            fillSpeaker((nxpTfaSpeakerFile_t *) hdr, argc, argv);
    }

    /*
     * calc CRC over bytes following the CRC field
     */

    if(cntFile) {
        base = (uint8_t *)&gCont->CRC + 4; // ptr to bytes following the CRC field
        size = gCont->size - (int)(base-(uint8_t *)gCont); // nr of bytes following the CRC field
    } else {
        base = (uint8_t *)&hdr->CRC + 4; // ptr to bytes following the CRC field
        size = hdr->size - (int)(base-(uint8_t *)hdr); // nr of bytes following the CRC field
    }

    if(size > 0)
    {
        if(cntFile)
            gCont->CRC = tfaContCRC32(base, size, 0); // nr of bytes following the CRC field, 0);
        else
            hdr->CRC = tfaContCRC32(base, size, 0); // nr of bytes following the CRC field, 0);

        // EL using different mask
        // hdr->CRC = tfaContCRC32(base, size, 0xFFFFFFFF); // nr of bytes following the CRC field, 0);

        if(hdrsize == 0) {
            PRINT("The existing header is modified for %s.\n", fname);
        } else
            PRINT("A new Header is created for %s.\n", fname);

        tfaRename(fname, ".old");
        /* tfaContShowEq( (nxpTfaEqualizerFile_t*) hdr);  dump eq */

        if(cntFile)
            i = tfaContSave((nxpTfaHeader_t *) gCont, fname);
        else
            i = tfaContSave(hdr, fname);
    }
    else
        PRINT("Error: The size is not correct -> %d\n", size);

        stop:
    if(cntFile)
        free(gCont);
    else
        free(hdr);

    return i;
}

/*
 * Get the absolute path of the file
 */
void tfaGetAbsolutePath(char *fileName, nxpTfaLocationInifile_t *loc)
{
#if 0
    char cntFile[FILENAME_MAX];
    char full[FILENAME_MAX];
#endif
    int strLength;

    //Get the file basename
    strcpy(sg_cntFile,  basename(fileName));

#if defined(WIN32) || defined(_X64)
    //Get the absolute path of the ini file
    _fullpath(sg_full, fileName, FILENAME_MAX);
    //Get the path and basename length
    strLength = (int)strlen(sg_full);
    strLength -= (int)strlen(sg_cntFile);
    //Only copy the path (without filename)
    strncpy(loc->locationIni, sg_full, strLength);
#else
    char *abs_path = realpath(fileName, sg_full);
    if (abs_path) {
        //Get the path and basename length
        strLength = (int)strlen(abs_path);
        if (strLength > FILENAME_MAX) {
            ERRORMSG("%s:%u:strLength(%u) is larger than FILENAME_MAX(%u)",
                __func__,__LINE__,strLength,FILENAME_MAX);
        }
        strLength -= (int)strlen(sg_cntFile);
        //Only copy the path (without filename)
        strncpy(loc->locationIni, abs_path, strLength);
    }
#endif
}


/*
 * Generate the container file from an ini file <this>.ini to <this>.cnt
 *
 */
int tfaContIni2Container(char *iniFile) {
    nxpTfaLocationInifile_t *loc = malloc(sizeof(nxpTfaLocationInifile_t));
    char cntFile[FILENAME_MAX], *dot;
    char completeName[FILENAME_MAX];
    int size = 0;

    memset(loc->locationIni, '\0', sizeof(loc->locationIni));

    //Get the file basename
    strcpy(cntFile,  basename(iniFile));

    tfaGetAbsolutePath(iniFile, loc);

    strcpy(completeName, loc->locationIni);
    strcat(completeName, cntFile);

    PRINT("Generating container using %s \n", completeName);

    dot  = strrchr(cntFile, '.');
    if(dot) {
        *dot='\0';
    }
    strcat(cntFile,  ".cnt");

    size = tfaContParseIni(iniFile, cntFile, loc);
    if(size > 0) {
        PRINT("Created container %s of %d bytes.\n", cntFile, size);
    } else {
        PRINT("No container is created!\n");
    }
    free(loc);
    return size;
}
/* Calculating ZIP CRC-32 in 'C'
   =============================
   Reference model for the translated code */




#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

nxpTfaDescriptorType_t parseKeyType(char *key)
{
    if (strcmp("mode", key) == 0)
        return dscMode;
    if (strcmp("file", key) == 0)
        return dscFile;
    if (strcmp("patch", key) == 0)
        return dscPatch;
    if (strcmp("config", key) == 0)
        return dscFile;
    if (strcmp("preset", key) == 0)
        return dscFile;
    if (strcmp("speaker", key) == 0)
        return dscFile;
    if (strcmp("drc", key) == 0)
        return dscFile;
    if (strcmp("eq", key) == 0)
        return dscFile;
    if (strcmp("volstep", key) == 0)
        return dscFile;
    if (strcmp("vstep", key) == 0)
        return dscFile;
    if (strcmp("device", key) == 0)
        return dscDevice;
//      if (strcmp("profile", key)==0) return dscProfile;
    if (key[0] == '&')
        return dscProfile;
    if (key[0] == '$')
        return dscRegister;
    if (key[0] == '_')
        return dscBitfieldBase;
    if (strcmp("bus", key) == 0)
        return -1;    // skip
    if (strcmp("dev", key) == 0)
        return -1;    // skip
    if (strcmp("devid", key) == 0)
        return -1;    // skip
    if (strcmp("func", key) == 0)
        return -1;    // skip

    return dscString;    //unknown, assume dscString
}

//typedef enum nxpTfaBitEnum {
//       bfDCCV  = 0x0991,    /* Coil Value                                         */
//       bfNiks=0xffff
//} nxpTfaBitEnum_t;
static uint16_t getBitfield(char *name)
{
    if (strcmp(name, "DCCV") == 0)
        return bfDCCV;
    else
        return -1;
}

/*
 * lookup and fill the bit descriptor
 */
typedef union nxpTfaDscBit {
    nxpTfaDescPtr_t dsc;
    nxpTfaBitfield_t bf;
    uint32_t u32;
    uint8_t b[4];
} nxpTfaDscBit_t;
static int setBitfieldDsc(nxpTfaBitfield_t * bf, char *str, uint16_t value)
{
    nxpTfaDscBit_t *uni = (nxpTfaDscBit_t *) bf;
    // pre-formatted bitfield looks like _nnnCCCC : _001DCCV
    uint16_t bfEnum = tfaContBfEnum(&str[4]);

    if (bfEnum < 0xffff) {
        //      bf->field = bfEnum + 0x8000;
        uni->b[3] = (bfEnum >> 8) | 0x80;
        uni->b[2] = (uint8_t) bfEnum;
    } else {
        VERBOSE PRINT("skipped unknown bitfield:%s\n", str);
        return 1;
    }
    //bf->value = value;
    uni->b[1] = value >> 8;
    uni->b[0] = (uint8_t) value;

    return 0;
}

/*
 * file processing key tables
 */


#define MAXKEYLEN 64
#define MAXLINELEN 256
#define MAXCONTAINER (256*1024)    // TODO make a smart size  estimator

static char *inifile; // global used in browse functions
typedef struct namelist {
    int n;
    int len;
    char names[64][MAXKEYLEN];
} namelist_t;

static int findDevices(const char *section, const char *key, const char *value,
               const void *userdata)
{
    namelist_t *keys;

    keys = (namelist_t *) userdata;
    keys->len++;
    // PRINT("    [%s]\t%s=%s\n", section, key, value);
    if (strcmp(section, "system") == 0 && strcmp(key, "device") == 0) {
        strncpy(keys->names[keys->n], value, MAXKEYLEN);
        keys->n++;
        keys->len--;    //don't count system items
    }

    return 1;
}

static char *currentSection;
static int findProfiles(const char *section, const char *key, const char *value,
            const void *userdata)
{
    namelist_t *keys;
#define INI_MAXKEY 64
    char tmp[INI_MAXKEY];
    int i;

    keys = (namelist_t *) userdata;
    keys->len++;
    //PRINT("    [%s]\t%s=%s\n", section, key, value);
    if (strcmp(section, currentSection) == 0 && strcmp(&key[4], "profile") == 0) {    // skip preformat
        // check if it exists
        if (ini_getkey(value, 0, tmp, 10, inifile) == 0) {
            PRINT("no profile section:%s\n", value);
            return 0;
        }

        // avoid duplicates
        for(i=0; i<keys->n;i++) {
            if (strcmp(value, keys->names[i])==0)
                return 1;
        }
        strncpy(keys->names[keys->n], value, MAXKEYLEN);
        keys->n++;
        keys->len--;    //don't count system items
        return 1;
    } else
        return 1;
}

typedef struct browser {
    int idx;
    int cnt;
    int value;
    char section[MAXKEYLEN];
    char key[MAXKEYLEN];
    char str[MAXKEYLEN];
} browser_t;


static char **stringList;
static int *offsetList;        // for storing offsets of items to avoid duplicates
static uint16_t listLength = 0;
static char noname[] = "NONAME";
//static char nostring[] = "NOSTRING";

/*
 *  add string to list
 *   search for duplicate, return index if found
 *   malloc mem and copy string
 *   return index
 */
static int addString(char *strIn)
{
    int len;
    uint16_t idx;
    char *str;

    // skip pre format
    if (strIn[0] == '$' || strIn[0] == '&')
        str = &strIn[4];
    else
        str = strIn;

    len = (int)strlen(str);
    for (idx = 0; idx < listLength; idx++) {
        if (strncmp(stringList[idx], str, len) == 0)
            return idx;
    }
    // new string
    idx = listLength++;
    stringList[idx] = (char *) malloc(len + 1);    //include \0
    assert(stringList[idx] != 0);
    strcpy(stringList[idx], str);

    return idx;
}
#ifdef DUMP
/*
 * dump a descriptor depending on it's type
 *  after 1st pass processing
 */
static void dumpDsc1st(nxpTfaDescPtr_t * dsc)
{
    uint32_t *ptr = (uint32_t *) dsc;
    uint32_t num = *ptr;

    switch (dsc->type) {
    case dscDevice:    // device list
        PRINT("device\n");
        break;
    case dscProfile:    // profile list
        PRINT("profile: %s\n", tfaContGetStringPass1(dsc));
        break;
    case dscMode:    // use case
        PRINT("mode: %s\n", tfaContGetStringPass1(dsc));
        break;
    case dscRegister:    // register patch
        PRINT("register patch: %s\n", tfaContGetStringPass1(dsc));
        break;
    case dscString:    // ascii: zero terminated string
        PRINT("string: %s\n", tfaContGetStringPass1(dsc));
        break;
    case dscFile:        // filename + file contents
    case dscPatch:        // filename + file contents
        PRINT("file: %s\n", tfaContGetStringPass1(dsc));
        break;
    default:
        if (dsc->type & dscBitfieldBase) {
            PRINT("dscBitfieldBase: 0x%08x\n", num);
            //dscBitfieldBase=0x80 // start of bitfield enums
        }
        break;

    }

}
static void dumpStrings(void)
{
    uint16_t idx;
    for (idx = 0; idx < listLength; idx++) {
        PRINT("[0x%0x] = %s\n", idx, stringList[idx]);
    }

}
static void dumpDevs(nxpTfaContainer_t * cont)
{
    nxpTfaDeviceList_t *dev;
    nxpTfaDescPtr_t *dsc;
    int i, d = 0;

    while ((dev = tfaContGetDevList(cont, d++)) != NULL) {
        dsc = &dev->name;
        if (dsc->type != dscString)
            PRINT("expected string: %s:%d\n", __FUNCTION__,
                   __LINE__);
        else
            PRINT("[%s] ", tfaContGetStringPass1(dsc));
        PRINT("devnr:%d length:%d bus=%d dev=0x%02x func=%d\n",    //TODO strings
               dsc->offset, dev->length, dev->bus, dev->dev, dev->func);
        for (i = 0; i < dev->length; i++) {
            dsc = &dev->list[i];
            dumpDsc1st(dsc);
        }
    }
}

/*
 * dump container file
 */
static void dump(nxpTfaContainer_t * cont, int len)
{
    nxpTfaDescPtr_t *dsc = (nxpTfaDescPtr_t *) cont->index;
    uint32_t *ptr;
    uint8_t *bptr;
    int i;

    PRINT("id:%.2s version:%.2s subversion:%.2s\n", cont->id,
           cont->version, cont->subversion);
    PRINT("size:%d CRC:0x%08x rev:%d\n", cont->size, cont->CRC, cont->rev);
    PRINT("customer:%.8s application:%.8s type:%.8s\n", cont->customer,
           cont->application, cont->type);
    PRINT("base=%p\n", cont);

    if ( len<0) {
    len =  cont->size - sizeof(nxpTfaContainer_t);
    len /= sizeof(nxpTfaDescPtr_t);
    len++; // roundup
    }

    for (i = 0; i < len; i++) {
        ptr = (uint32_t *) dsc;
        PRINT("%p=dsc[%d]:type:0x%02x 0x%08x\n", ptr, i, dsc->type,
               *ptr);
//              PRINT("dsc[%d]:type=0x%02x offset:0x%06x\n",i, dsc->type,  dsc->offset[0] );
        dsc++;
    }
}
#endif //DUMP
/*
 * process system section from ini file
 */
static int systemSection(nxpTfaContainer_t * head)
{
    int error = 1;
    char buf[sizeof(head->type)+1] = { 0 };
    head->id[0] = (char)paramsHdr;
    head->id[1] = (char)(paramsHdr >> 8);
    head->version[0] = NXPTFA_PM_VERSION;
    head->version[1] = '_';
    head->subversion[0] = '0';
    head->subversion[1] = '0';

    head->customer[0] = 0;
    head->application[0] = 0;
    head->type[0] = 0;

    head->rev = (uint16_t) ini_getl("system", "rev", 0, inifile);
    ini_gets("system", "customer", "",  buf, sizeof buf, inifile);
    strncpy(head->customer, buf, sizeof head->customer);

    ini_gets("system", "application", "", buf, sizeof buf, inifile);
    strncpy(head->application, buf,     sizeof head->application);

    ini_gets("system", "type", "", buf, sizeof buf, inifile);
    strncpy(head->type, buf, sizeof(head->type));

    if(head->customer[0] == 0 || head->application[0] == 0 || head->type[0] == 0) {
        PRINT("Error: There is a empty key in the system section. \n");
        error = 0;
    }

    return error;
}

/*
 * fill the offset(
 */
nxpTfaDescPtr_t *tfaContSetOffset(nxpTfaContainer_t * cont,
                  nxpTfaDescPtr_t * dsc, int idx)
{
    int offset = sizeof(nxpTfaContainer_t) + idx * sizeof(nxpTfaDescPtr_t);

    dsc->offset = offset;
#ifdef DUMP
    printf("%s: idx:0x%0x offset:0x%0x\n", __FUNCTION__,idx,offset);
#endif
    return dsc;
}



/*
 * lookup the string in the stringlist and return a ptr
 *   this if for pass1 handling only
 *
 */
const char *tfaContGetStringPass1(nxpTfaDescPtr_t * dsc)
{
//      if ( dsc->type != dscString)
//              return nostring;
    if (dsc->offset > listLength)
        return noname;

    return stringList[dsc->offset];
}

nxpTfaProfileList_t *tfaContFindProfile(nxpTfaContainer_t * cont,
                    const char *name)
{
    int idx;
    nxpTfaProfileList_t *prof;
    const char *profName;

    prof = tfaContGet1stProfList(cont);

    for(idx=0;idx<cont->nprof;idx++) {
        profName = tfaContGetStringPass1(&prof->name);
        if (strcmp(profName, name) == 0) {
            return prof;
        }
        // next
        prof = tfaContNextProfile(prof);
        if (prof==NULL) {
            ERRORMSG("Illegal profile nr:%d\n", idx);
            return NULL;
        }
    }

    return NULL;
}

/*
 * all lists are parsed now so the offsets to profile lists can be filled in
 *  the device list has a dsc with the profile name, this need to become the byte
 *  offset of the profile list
 */
static void tfaContFixProfOffsets(nxpTfaContainer_t * cont)
{
    int i = 0, profIdx;
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *prof;

    // walk through all device lists
    while ((dev = tfaContGetDevList(cont, i++)) != NULL) {
        // find all profiles in this dev
        for (profIdx = 0; profIdx < dev->length; profIdx++) {
            if (dev->list[profIdx].type == dscProfile) {
                //dumpDsc1st(&dev->list[profIdx]);
                prof = tfaContFindProfile(cont, tfaContGetStringPass1(&dev->list[profIdx]));    // find by name
                if (prof) {
                    dev->list[profIdx].offset = (uint32_t)((uint8_t *)prof - (uint8_t *)cont);    // fix the offset into the container
                } else {
                    PRINT("Can't find profile:%s \n",
                           tfaContGetStringPass1(&dev->
                                list[profIdx]));
                }
            }
        }
    }
}

/*
 * check if this item is already in the list
 */
static int tfaContHaveItem(nxpTfaContainer_t * cont, nxpTfaDescPtr_t * dsc){
    int i, stringOffset = dsc->offset;

    if ( !(dsc->type & dscBitfieldBase) )
        for(i=0;i<listLength;i++)
            if (offsetList[i]==dsc->offset)
                return 1;
    return 0;
}

/*
 *   append and fix the offset for this item
 *   the item will be appended to the container at 'size' offset
 *   returns the byte size of the item added
 */
static int tfaContAddItem(nxpTfaContainer_t *cont, nxpTfaDescPtr_t *dsc, nxpTfaLocationInifile_t *loc)
{
    nxpTfaHeader_t existingHdr, *buf = NULL;
    nxpTfa98xxParamsType_t paramsType;
    int headerValue = 0;
    char fname[FILENAME_MAX];
    char strLocation[FILENAME_MAX];
    char full[FILENAME_MAX];
    int size = 0, stringOffset = dsc->offset;
    const char *str;
    FILE *f;
    uint8_t *dest = (uint8_t *) cont + cont->size;
    nxpTfaRegpatch_t pat;
    nxpTfaMode_t cas;
    nxpTfaDescPtr_t *pDsc;

    str = tfaContGetStringPass1(dsc);
    strncpy(fname, str, FILENAME_MAX);

    memset(strLocation, '\0', sizeof(strLocation));

#if defined(WIN32) || defined(_X64)
    if(strcmp(_fullpath(full, str, FILENAME_MAX),str) != 0)
        strcpy(strLocation, loc->locationIni);
    strcat(strLocation, str);
#else
    char *abs_path = realpath(str, NULL);

    if(abs_path != NULL) {
        if(strcmp(abs_path,str) != 0)
            strcpy(strLocation, loc->locationIni);
        strcat(strLocation, str);
    } else {
        strcpy(strLocation, loc->locationIni);
        strcat(strLocation, str);
        abs_path = realpath(strLocation, NULL);
    }
    free(abs_path);
#endif

    /*
     * check if this item is already in the offset list
     *  of so then the contents has already been appended
     *  and we only need to update the offset
     */
    if ( !(dsc->type & dscBitfieldBase) ) {
        if (stringOffset < listLength) {    //TODO check if this is good enough for check
            if (offsetList[stringOffset]) {
                dsc->offset = offsetList[stringOffset];    // only fix the offset
                return 0;
            }
        }
        }

    switch (dsc->type) {
    case dscRegister:    // register patch : "$53=0x070,0x050"
        if (sscanf(str, "$%hhx=%hx,%hx", &(pat.address),
                        &(pat.value), &(pat.mask)) == 3) {
            size = sizeof(nxpTfaRegpatch_t);
            memcpy(dest, &pat, size);
        } else if (sscanf(str, "$%hhx=%hx", &(pat.address),
                        &(pat.value)) == 2) {
                        pat.mask = 0xffff;
            size = sizeof(nxpTfaRegpatch_t);
            memcpy(dest, &pat, size);
        } else
            return 1;
        break;
    case dscMode:
        if (sscanf(str, "mode=%d\n", &(cas.value)) == 1) {
            size = sizeof(nxpTfaMode_t);
            memcpy(dest, &cas, size);
        } else {
            int i=0;
            while (tfa98xx_mode_str[i] != NULL) {
                if (strcmp(&str[5],tfa98xx_mode_str[i]) == 0) {
                    cas.value = i;
                    size = sizeof(nxpTfaMode_t);
                    memcpy(dest, &cas, size);
                    break;
                }
                i++;
            }
            if (tfa98xx_mode_str[i] == NULL)
                return 1;
        }
        break;
    case dscString:    // ascii: zero terminated string
        strcpy((char*)dest, str);
        size = (int)strlen(str) + 1;    // include \n
        //PRINT("string: %s\n", tfaContGetStringPass1(dsc));
        break;
    case dscFile:        // filename.dsc + size + file contents + filename.string
    case dscPatch:
        pDsc = (nxpTfaDescPtr_t * )dest; // for the filename
        dest += 4;
        size = fsize(strLocation);    // get the filesize
        *((uint32_t *)dest) = size; // store the filesize
        dest += 4;
        f=fopen(strLocation, "rb");
        if (!f) {
            PRINT("Unable to open %s\n", strLocation);
            return 1;
        }
        size = (int)fread(dest, 1, MAXCONTAINER - cont->size, f);
        rewind(f);
        fread(&existingHdr , sizeof(existingHdr), 1, f);
        fclose(f);

                /* We want to check CRC of every file! */
                tfaReadFile(strLocation, (void**) &buf);
                if (tfaContCrcCheck(buf)) {
                ERRORMSG("CRC error in %s\n", fname);
                        return 1;
            }

        paramsType = cliParseFiletype(fname);
        switch ( paramsType ) {
        case tfa_speaker_params:
            if(!(HeaderMatches(&existingHdr, speakerHdr))) {
                PRINT("Error: %s has no header! \n", fname);
                return 1;
            }
            break;
        case tfa_patch_params:
            if(!(HeaderMatches(&existingHdr, patchHdr))) {
                PRINT("Error: %s has no header! \n", fname);
                return 1;
            }
            break;
        case tfa_config_params:
            if(!(HeaderMatches(&existingHdr, configHdr))) {
                PRINT("Error: %s has no header! \n", fname);
                return 1;
            }
            break;
        case tfa_preset_params:
        case tfa_equalizer_params:
        case tfa_drc_params:
        case tfa_no_params:
                /* Only to remove warning in Linux */
            break;
        }
        pDsc->type = dscString;
        pDsc->offset = cont->size + size + sizeof(nxpTfaDescPtr_t)+4; // the name string is after file
        dest += size;
        str = tfaContGetStringPass1(dsc); // filename
        strcpy((char*)dest, str);
        size += (int)(strlen(str) + 1 +sizeof(nxpTfaDescPtr_t)+4);    // include \n
        break;
    default:
        return 0;
        break;
    }
    offsetList[stringOffset] = dsc->offset = cont->size;
    cont->size += size;

    return 0;
}

/*
 * walk through device lists and profile list
 *  if to-be fixed
 */
static int tfaContFixItemOffsets(nxpTfaContainer_t *cont, nxpTfaLocationInifile_t *loc)
{
    int i, j = 0, maxdev = 0;
    unsigned int idx;
    int error = 0;
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *prof;

    offsetList = malloc(sizeof(int) * listLength);
    //bzero(offsetList, sizeof(int) * listLength);    // make all entries 0 first
    memset(offsetList, 0, sizeof(int) * listLength);
    // walk through all device lists

    while ((dev = tfaContGetDevList(cont, maxdev++)) != NULL)
    {
        // fix name first
        error = tfaContAddItem(cont, &dev->name, loc);
        for (idx = 0; idx < dev->length; idx++) {
            error = tfaContAddItem(cont, &dev->list[idx], loc);
                        if(error != 0) {
                free(offsetList);
                return 1;
            }
        }
    }
    maxdev--;
    // walk through all profile lists
    for (i = 0; i < maxdev; i++) {
        j=0;
        while ((prof = tfaContGetDevProfList(cont, i, j++)) != NULL) {
            // fix name first
            if (!tfaContHaveItem(cont, &prof->name)){
                error = tfaContAddItem(cont, &prof->name, loc);
                for (idx = 0; idx < prof->length; idx++) {
                    error = tfaContAddItem(cont, &prof->list[idx], loc);
                                        if(error != 0) {
                                free(offsetList);
                                return 1;
                            }
                }

            }
        }
    }
    free(offsetList);
    return error;
}



/*
 * pre-format ini file
 *  This is needed for creating unique key names for bitfield
 *  and register keys.
 *  It simplifies the processing of multiple entries.
 */
static int preFormatInitFile(const char *infile, const char *outfile)
{
    FILE *in, *out;
    char buf[MAXLINELEN], *ptr;
    int cnt = 0;

    in = fopen(infile, "r");
    if (in == 0) {
        PRINT("error: can't open %s for reading\n", infile);
        exit(1);
    }

    out = fopen(outfile, "w");
    if (in == 0) {
        PRINT("error: can't open %s for writing\n", outfile);
        exit(1);
    }

    ptr = buf;
    while (!feof(in)) {
        if ( NULL==fgets(ptr, sizeof(buf), in))
            break;
        // skip space
        while (isblank((unsigned)*ptr))
            ptr++;
        // ch pre format
        if (ptr[0] == '$')    // register
            PRINT_FILE(out, "$%03d%s", cnt++, ptr);
        else if (isupper((int)ptr[0]) /*&& isupper(ptr[1])*/)    // bitfield when  starting with uppercase
            PRINT_FILE(out, "_%03d%s", cnt++, ptr);
        else if (strncmp(ptr, "profile", 6 ) == 0)
            PRINT_FILE(out, "&%03d%s", cnt++, ptr);
        else
            fputs(ptr, out);
    }
    fclose(out);
    fclose(in);

    return 0;
}

/*
 *
 */
int tfaContSave(nxpTfaHeader_t *hdr, char *filename)
{
    FILE *f;
    int c, size;

    f = fopen(filename, "wb");
    if (!f) {
        PRINT("Unable to open %s\n", filename);
        return 0;
    }

    // check type
    if ( hdr->id == paramsHdr ) { // if container use other header
        size = ((nxpTfaContainer_t*)hdr)->size;
    } else
        size = hdr->size;

    c = (int)fwrite(hdr, size, 1, f);
    fclose(f);

    return c;
}

/*
 * create a big buffer to hold the entire container file
 *  return final file size
 */
int tfaContCreateContainer(nxpTfaContainer_t * contIn, char *outFileName, nxpTfaLocationInifile_t *loc)
{
    nxpTfaContainer_t *contOut;
    int size, error;
    char strLocation[FILENAME_MAX];
    uint8_t *base;

    contOut = (nxpTfaContainer_t *) malloc(MAXCONTAINER);
    if (contOut == 0) {
        PRINT("Can't allocate %d bytes.\n", MAXCONTAINER);
    }

    size = sizeof(nxpTfaContainer_t) + contIn->size * sizeof(nxpTfaDescPtr_t);    // still has the list count
    memcpy(contOut, contIn, size);
    //dump(contOut, contOut->size);
    contOut->size = size;    // now it's the actual size
    /*
     * walk through device lists and profile list
     *  if to-be fixed
     */

    error = tfaContFixItemOffsets(contOut, loc);
    /*
     * calc CRC over bytes following the CRC field
     */
    if(size > 0 && error == 0) {
        base = (uint8_t *)&contOut->CRC + 4; // ptr to bytes following the CRC field
        size = (int)(contOut->size - (base-(uint8_t *)contOut)); // nr of bytes following the CRC field
        contOut->CRC = tfaContCRC32(base, size, 0); // nr of bytes following the CRC field, 0);

        strcpy(strLocation, loc->locationIni);
        strcat(strLocation, outFileName);
        tfaContSave((nxpTfaHeader_t *)contOut, strLocation);
        dump(contOut, -1);
        size = contOut->size;
    } else {
                size = 0;
        }
    free(contOut);

    return size;
}

/*
 * create the containerfile as descibred by the input 'ini' file
 *
 * return the size of the new file
 */
int tfaContParseIni( char *iniFile, char *outFileName, nxpTfaLocationInifile_t *loc)
{
    int k, i, j, error, idx = 0;
    int keyCount[10] = {0};
    char value[100], key[100];
    nxpTfaContainer_t *headAndDescs;
    namelist_t keys, profiles;
    nxpTfaDescPtr_t *dsc;
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *profhead;
    char preformatIni[FILENAME_MAX];
    char tmp[INI_MAXKEY];
    /*
     *
     */
    strcpy(preformatIni, loc->locationIni);
    strcat(preformatIni, outFileName);
    strcat(preformatIni,  "_preformat.ini");
    preFormatInitFile(iniFile, preformatIni);
    inifile = preformatIni; // point to the current inifile (yes, this is dirty ..)
    VERBOSE PRINT("preformatted ini=%s\n", inifile);
    /*
     * process system section
     */
    // find devices in system section
    keys.n = 0;        // total nr of devices
    profiles.n = 0;
    keys.len = 0;        // maximum length of all desc lists
    ini_browse(findDevices, &keys, preformatIni);    //also counts entries
    // get storage for container header and dsc lists
    headAndDescs = (nxpTfaContainer_t *)malloc(sizeof(nxpTfaContainer_t) + keys.n*2 * keys.len * 4+4*keys.n*4);    //this should be enough
    //bzero(headAndDescs, sizeof(nxpTfaContainer_t) + keys.n*2 * keys.len * 4+4*keys.n*4);
    memset(headAndDescs, 0, sizeof(nxpTfaContainer_t) + keys.n*2 * keys.len * 4+4*keys.n*4);
    stringList = malloc(fsize(preformatIni));    //strings can't be longer the the ini file
    assert(headAndDescs != 0);
    /*
     * system settings
     */
    error = systemSection(headAndDescs);
    if(error == 0) {
        if (!tfa98xx_cnt_verbose)
            remove(preformatIni);

        free(headAndDescs);
        for (j = 0; j < listLength; j++)
            free(stringList[j]);
        free(stringList);
        listLength = 0;
        return 0;
    }

    /*
     * next is the creation of device and profile initlists
     *  these lists consists of index into a stringtable (NOT the final value!)
     *  processing to actual values will be the last step
     */
    // devices
    headAndDescs->ndev = keys.n; // record nr of devs
    dsc = (nxpTfaDescPtr_t *) headAndDescs->index;

    // create idx initlist with names
    for (i = 0; i < keys.n; i++) {
        dsc->type = dscDevice;
        dsc->offset = addString(keys.names[i]);
        dsc++;
    }
    dsc->type = dscMarker;    // mark end of devlist
    dsc->offset = 0xa5a5a5;    // easy to read
    dsc++;
    // create device initlist
    for (i = 0; i < keys.n; i++) {
        dev = (nxpTfaDeviceList_t *) dsc;    //1st device starts after idx list
        currentSection = keys.names[i];
        // PRINT("dev:%s\n", currentSection);
        dev->bus = (uint8_t) ini_getl(keys.names[i], "bus", 0, preformatIni);
        dev->dev = (uint8_t) ini_getl(keys.names[i], "dev", 0, preformatIni);
        dev->func = (uint8_t) ini_getl(keys.names[i], "func", 0, preformatIni);
        dsc++;
        dev->devid = (uint32_t) ini_getl(keys.names[i], "devid", 0, preformatIni);
        dsc++;
        // add the name
        dev->name.type = dscString;

        dev->name.offset = addString(currentSection);
        dsc++;
        // get the dev keys
        for (k = 0; ini_getkey(keys.names[i], k, key, sizearray(key), preformatIni) > 0; k++) {
            VERBOSE PRINT("\tkey: %d %s \n", k, key);
            keyCount[i]++;
            dsc->type = parseKeyType(key);
            switch ((int)dsc->type) {
                        case dscString:    // ascii: zero terminated string
                                PRINT("Warning: In section: [%s] the key: %s is used as a string! \n", currentSection, key);
            case dscFile:    // filename + file contents
            case dscPatch:
            case dscProfile:
                if (ini_gets(keys.names[i], key, "", value,sizeof(value), preformatIni)) {
                    dsc->offset = addString(value);
                    dsc++;
                }
                break;
            case dscMode:
            case dscRegister:    // register patch e.g. $53=0x070,0x050
                strcpy(value, key);    //store value with key
                strcat(value, "=");
                if (ini_gets(keys.names[i], key, "", &value[strlen(key) + 1], sizeof(value) - (int)(strlen(key)) - 1, preformatIni)) {
                    dsc->offset = addString(value);
                    dsc++;
                }
                break;
            case dscBitfieldBase:    // start of bitfield enums
                if (setBitfieldDsc((nxpTfaBitfield_t *) dsc, key,(uint16_t) ini_getl(keys.names[i], key, 0, preformatIni)) == 0)
                    dsc++;
                break;
            case dscDevice:    // device list
                PRINT("error: skipping illegal key:%s\n", key);
                break;
            case -1:
                continue;
                break;
            default:
                VERBOSE PRINT(" skipping key:%s\n", key);
                break;
            }
        }
        idx = (int)((uint32_t *) dsc - (uint32_t *) (dev) - sizeof(nxpTfaDeviceList_t) / 4);
        dev->length = idx;    //store the total list length (inc name dsc)

        // get the profiles names
        error = ini_browse(findProfiles, &profiles, preformatIni);    //also counts entries
    }

    for(i=0; i<keys.n; i++) {
        if(keyCount[i] == 0) {
            PRINT("Error: There are not enough device sections\n");
            error = 0;
        }
    }

    if((keys.n < 2) && (ini_getkey("left", 0, tmp, 10, inifile) > 0) && (ini_getkey("right", 0, tmp, 10, inifile) > 0)) {
            PRINT("Error: There are not enough device keys in the system section\n");
            error = 0;
    }

    if(error == 0) {
        if (!tfa98xx_cnt_verbose)
            remove(preformatIni);
        free(headAndDescs);
        for (j = 0; j < listLength; j++)
            free(stringList[j]);
        free(stringList);
        listLength = 0;
        return 0;
    }

    //If a error already occurred finding profiles, this is useless
    dsc = (nxpTfaDescPtr_t *) headAndDescs->index;
    tfaContSetOffset(headAndDescs, dsc, keys.n + 1);    //+ marker first is after index

    // start idx skips keys and marker
    for (i = 0,idx=keys.n+1 ; i < keys.n - 1; i++) {    // loop one less from total, 1st is done
        dev = tfaContGetDevList(headAndDescs, i);
        idx += dev->length ;    // after this one
        idx += sizeof(nxpTfaDeviceList_t) / 4; // also skip header
        dsc++;        // the next dev list
        tfaContSetOffset(headAndDescs, dsc, idx ); // the device list offset is updated
    }

    /*
    * create profile initlists
    */
    headAndDescs->nprof = profiles.n; // record nr of profiles
    dsc = (nxpTfaDescPtr_t *) tfaContGet1stProfList(headAndDescs);    // after the last dev list
    for (i = 0; i < profiles.n; i++) {
        profhead = (nxpTfaProfileList_t *) dsc;    //
        profhead->ID = 0x1234;
        dsc++;
        currentSection = profiles.names[i];
        VERBOSE PRINT("prof:%s\n", currentSection);
        // start with name
        profhead->name.type = dscString;
        profhead->name.offset = addString(currentSection);
        dsc++;
        // get the profile keys and process them
        for (k = 0; ini_getkey(profiles.names[i], k, key, sizearray(key), preformatIni) > 0; k++) {
            VERBOSE PRINT("\tkey: %d %s \n", k, key);
            dsc->type = parseKeyType(key);
            switch (dsc->type) {
            case dscString:    // ascii: zero terminated string
                                PRINT("Warning: In section: [%s] the key: %s is used as a string! \n", currentSection, key);
                        case dscFile:    // filename + file contents
            case dscPatch: // allow patch in profile?
                if (ini_gets(profiles.names[i], key, "", value, sizeof(value), preformatIni)) {
                    dsc->offset = addString(value);
                    dsc++;
                }
                break;
            case dscMode:
            case dscRegister:    // register patch e.g. $53=0x070,0x050
                strcpy(value, key);    //store value with key
                strcat(value, "=");
                if (ini_gets(profiles.names[i], key, "", &value[strlen(key) + 1], sizeof(value) - (int)(strlen(key)) - 1, preformatIni)) {
                    dsc->offset = addString(value);
                    dsc++;
                }
                break;
            case dscBitfieldBase:    // start of bitfield enums
                if (setBitfieldDsc((nxpTfaBitfield_t *) dsc, key, (uint16_t) ini_getl(profiles.names[i], key, 0, inifile)) == 0)
                    dsc++;
                break;

            case dscProfile:    // profile
            case dscDevice:    // device list
                PRINT("error: skipping illegal key:%s\n", key);
                break;
            default:
                VERBOSE PRINT(" skipping key:%s\n", key);
                break;
            }
        }

        idx = (int)((uint32_t *) dsc - (uint32_t *) (profhead));
        profhead->length = idx - 1;    //store the total list length (exc header, inc name dsc)

    }            /* device lists for loop end */

    dsc->type = dscMarker;    // mark end of proflists
    dsc->offset = 0x555555;    // easy to read
    dsc++;

/*
    * all lists are parsed now so the offsets to profile lists can be filled in
    *  the device list has a dsc with the profile name, this need to become the byte
    *  offset of the profile list
    */
    tfaContFixProfOffsets(headAndDescs);

    headAndDescs->size = (uint32_t)(((uint32_t *)dsc - (uint32_t *)(headAndDescs->index))+1);    //total

    /*
        * remaining for offset fixing:
        *              2 dscRegister,  // register patch
        *              3 dscString,            // ascii, zero terminated string
        *              4 dscFile,              // filename + file contents
        *              5 dscPatch
        */
    /*
        * create a big buffer to hold the entire container file
        */

    i = tfaContCreateContainer(headAndDescs, outFileName, loc);

    /* if no verbose is given the no ini file should be created */
    if (!tfa98xx_cnt_verbose)
        remove(preformatIni);

    free(headAndDescs);
    for (j = 0; j < listLength; j++)
        free(stringList[j]);
    free(stringList);
    listLength = 0;
    return i; // return size
}

/*
 * file type names for enum type nxpTfa98xxParamsType for user-friendly messages
 */
static const char *filetypeName[] = {
        "patch file" ,
        "speaker file" ,
        "preset file" ,
        "config file" ,
        "equalizer file" ,
        "drc file" ,
        "vstep file" ,
//        "mdrc file" ,
        "unknown file"
};
/*
 *
 */
nxpTfa98xxParamsType_t cliParseFiletype(char *filename)
{
    char *ext;
    nxpTfa98xxParamsType_t ftype;

    // get filename extension
    ext = strrchr(filename, '.'); // point to filename extension

    if ( ext == NULL ) {
        ftype = tfa_no_params;    // no '.'
    }
    // now look for supported type
    else if ( strcmp(ext, ".patch")==0 )
        ftype = tfa_patch_params;
    else if ( strcmp(ext, ".speaker")==0 )
        ftype = tfa_speaker_params;
    else if ( strcmp(ext, ".preset")==0 )
        ftype = tfa_preset_params;
    else if ( strcmp(ext, ".config")==0 )
        ftype = tfa_config_params;
    else if ( strcmp(ext, ".eq")==0 )
        ftype = tfa_equalizer_params;
    else if ( strcmp(ext, ".drc")==0 )
        ftype = tfa_drc_params;
    else if ( strcmp(ext, ".vstep")==0 )
        ftype = tfa_vstep_params;
    else if ( strcmp(ext, ".cnt")==0 )
        ftype = tfa_cnt_params;
    else if ( strcmp(ext, ".msg")==0 )
        ftype = tfa_msg_params;
    else
                ftype = tfa_no_params;

    if (ftype != tfa_no_params && tfa98xx_cnt_verbose )
        PRINT("file %s is a %s.\n" , filename, filetypeName[ftype]);

    return ftype;

}

/*
 * Funtion to split the container file into individual files and ini file
 */
int tfa98xx_cnt_split(char *fileName) {
    nxpTfaLocationInifile_t *loc = malloc(sizeof(nxpTfaLocationInifile_t));
    nxpTfaDeviceList_t *dev;
    nxpTfaProfileList_t *prof;
    nxpTfaFileDsc_t *file;
    nxpTfaHeader_t *hdr;
    nxpTfaContainer_t * cont;
    int error = tfa_srv_api_error_Ok;
    int i=0, j=0, k=0;
    unsigned int m=0;
    int devFile=0, profFile=0;
    char *filename;
    char buffer[4*1024];    //,byte TODO check size or use malloc
    char completeName[FILENAME_MAX];
    int length;

    memset(loc->locationIni, '\0', sizeof(loc->locationIni));

    /* Get the absolutepath of the .cnt file */
    tfaGetAbsolutePath(fileName, loc);

    if( tfa98xx_get_cnt() == NULL ) {
                goto stop;
        error = tfa_srv_api_error_Fail;
        }

    error = tfaContShowContainer(buffer, sizeof(buffer));
    length = (int)(strlen(buffer));
    *(buffer+length) = '\0';        // terminate
    puts(buffer);

    if ( tfa98xx_cnt_verbose )
        PRINT("Warning: File with same name will be overwritten.\n");

    cont = tfa98xx_get_cnt();
    for(i=0 ; i < cont->ndev ; i++) {
        dev = tfaContGetDevList(cont, i);
        if( dev==0 ) {
                        goto stop;
                error = tfa_srv_api_error_Fail;
                }

        /* process the device list until a file type is encountered */
        for(j=0;j<dev->length;j++) {
            if ( dev->list[j].type == dscFile ||
                    dev->list[j].type == dscPatch) {
                file = (nxpTfaFileDsc_t *)(dev->list[j].offset+(uint8_t *)cont);
                hdr= (nxpTfaHeader_t *)file->data;
                /* write this file out */
                if ( tfa98xx_cnt_verbose )
                    PRINT("%s: device %s file %s found of type %s\n", __FUNCTION__,
                        tfaContDeviceName(i), tfaContGetString(&file->name),
                        tfaContFileTypeName(file));

                strcpy(completeName, loc->locationIni);
                filename = tfaContGetString(&file->name);    // to remove any relative path
                strcat(completeName, filename); // Add filename to the absolute path
                error = tfaosal_filewrite(completeName, (unsigned char *)hdr, file->size );
            }
        }
        /* Next, look in the profile list until the file type is encountered
         */
        for (k=0; k < tfaContMaxProfile(i); k++ ) {
            prof=tfaContGetDevProfList(cont, i, k);
            for(m=0;m<prof->length;m++) {
                if (prof->list[m].type == dscFile) {
                    file = (nxpTfaFileDsc_t *)(prof->list[m].offset+(uint8_t *)cont);
                    hdr= (nxpTfaHeader_t *)file->data;
                    /* write file type */
                    if ( tfa98xx_cnt_verbose )
                        PRINT("%s: profile %s file %s found of type %s\n", __FUNCTION__,
                            tfaContProfileName(i,k), tfaContGetString(&file->name),
                            tfaContFileTypeName(file));

                    strcpy(completeName, loc->locationIni);
                    filename = tfaContGetString(&file->name);    // to remove any relative path
                    strcat(completeName, filename); // Add filename to the absolute path
                    error = tfaosal_filewrite(completeName, (unsigned char *)hdr, file->size );
                }
            }
        }

    }
    if (error == tfa_srv_api_error_Ok)
        PRINT("Container file split into separated the device files.\n");

stop:
    free (loc);
    return error;
}
