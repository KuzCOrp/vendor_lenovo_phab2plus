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
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h> //TODO move to tfa api
#include "dbgprint.h"
#include "tfaContainer.h"
#include "tfaOsal.h"

/*
 * Set the debug option
 */
void tfa_cnt_util_verbose(int level) {
    tfa98xx_cnt_verbose = level;
}


void tfaContShowSpeaker(nxpTfaSpeakerFile_t *spk) {
    tfaContShowHeader( &spk->hdr);

    PRINT("Speaker Info: name=%.8s vendor=%.16s type=%.8s\n", spk->name, spk->vendor, spk->type);
    PRINT("  dimensions: height=%d width=%d depth=%d R=%d\n", spk->height, spk->width ,spk->depth, spk->ohm);

}

void tfaContGetHdr(char *inputname, nxpTfaHeader_t *hdrbuffer)
{
    nxpTfaHeader_t *localbuffer = 0;

    tfaReadFile(inputname, (void**) &localbuffer);

    memcpy(hdrbuffer, localbuffer, sizeof(nxpTfaHeader_t));

    if(localbuffer)
        free(localbuffer);
}

void tfa_cnt_get_spk_hdr(char *inputname, struct nxpTfaSpkHeader *hdrbuffer)
{
    struct nxpTfaSpeakerHdr *localbuffer = 0;

    tfaReadFile(inputname, (void**) &localbuffer);

    memcpy(hdrbuffer, localbuffer, sizeof(struct nxpTfaSpkHeader));

    if(localbuffer)
        free(localbuffer);
}

/*
 * show file info
 */
void tfaContShowFile(nxpTfaHeader_t *hdr) {
    tfaContShowHeader( hdr);
}
/*
 * show the file from the loaded container
 */
void tfaContShowFileDsc(nxpTfaFileDsc_t *f) {
    PRINT("name=%s, size=%d\n", tfaContGetString(&f->name), f->size);
    tfaContShowHeader( (nxpTfaHeader_t*)&f->data);
}

/*
 * display the contents of the devicelist from the container global
 *  return tfa_srv_api_error_Ok if ok
 */
tfa_srv_api_error_t  tfaContShowDevice(int idx, char **strings, int *length, int maxlength)
{
    tfa_srv_api_error_t err = tfa_srv_api_error_Ok;
    nxpTfaDeviceList_t *dev;
    char str[NXPTFA_MAXLINE];

    if ( idx >= tfa98xx_cnt_max_device() )
        return tfa_srv_api_error_BadParam;

    if ( (dev = tfaContDevice(idx)) == NULL )
        return tfa_srv_api_error_BadParam;

    sprintf(str, "[%s]\n",  tfaContGetString(&dev->name));
    err = tfaCheckStringLength(str, strings, length, maxlength);

    sprintf(str, "bus=%d\n" "dev=0x%02x\n",    dev->bus, dev->dev);
    err = tfaCheckStringLength(str, strings, length, maxlength);

    if ( dev->devid) {
        sprintf(str, "devid=0x%08x\n", dev->devid);
        err = tfaCheckStringLength(str, strings, length, maxlength);
    }

    return err;
}

tfa_srv_api_error_t tfa98xx_strip_hdr(char *file, char *buffer, int *size) {
        tfa_srv_api_error_t err = tfa_srv_api_error_Ok;
        char fname[FILENAME_MAX];
        FILE *f;
        nxpTfaHeader_t *buf = 0;
    nxpTfaHeader_t existingHdr;
        nxpTfa98xxParamsType_t paramsType;

        strncpy(fname, file, FILENAME_MAX);
    *size = tfaReadFile(fname, (void**) &buf);

        /* Check the CRC */
        if (tfaContCrcCheck((nxpTfaHeader_t *)buf)) {
        ERRORMSG("CRC error in %s\n", fname);
        return tfa_srv_api_error_BadParam;
    }

        /*  Read the first x bytes to see if the file has a header */
    f=fopen(fname, "rb");
    if ( !f ) {
        ERRORMSG("Can't open %s (%s).\n", fname, strerror(errno));
        return tfa_srv_api_error_BadParam;
    }

    // read the potential header
    fread(&existingHdr , sizeof(existingHdr), 1, f);
    fclose(f);
    paramsType = cliParseFiletype(fname);

    switch ( paramsType ) {
    case tfa_speaker_params:
        if(!HeaderMatches(&existingHdr, speakerHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(nxpTfaSpeakerFile_t);
                        memcpy(buffer, ((nxpTfaSpeakerFile_t *)buf)->data, *size);
                }
        break;
    case tfa_patch_params:
        return tfa_srv_api_error_BadParam;
        break;
    case tfa_config_params:
        if(!HeaderMatches(&existingHdr, configHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;

#if ( defined( TFA9887B ) || defined( TFA98XX_FULL ))
    case tfa_drc_params:
        if(!HeaderMatches(&existingHdr, drcHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;
#endif
    case tfa_preset_params:
        if(!HeaderMatches(&existingHdr, presetHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;
    case tfa_equalizer_params:// store eq in profile
        if(!HeaderMatches(&existingHdr, equalizerHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;
    case tfa_vstep_params:
        if(!HeaderMatches(&existingHdr, volstepHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;
        case tfa_msg_params:
                if(!HeaderMatches(&existingHdr, msgHdr))
            return tfa_srv_api_error_BadParam;
                else {
                        *size -= sizeof(((nxpTfaMsg_t *)buf)->hdr);
                        memcpy(buffer, ((nxpTfaMsg_t *)buf)->data, *size);
                }
        break;
        case tfa_cnt_params:
        return tfa_srv_api_error_BadParam;
        break;
    default:
        ERRORMSG("Unknown file type:%s\n", fname) ;
        return tfa_srv_api_error_BadParam;
        break;
    }

        return err;
}