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


#include "tfaOsal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if !(defined(WIN32) || defined(_X64) || defined(__REDLIB__))
#include <libgen.h>
#endif

#include "dbgprint.h"
#include "nxpTfa98xx.h"

#if defined(WIN32) || defined(_X64)
char *basename(const char *fullpath) {
    static char name[_MAX_FNAME];
   static char ext[_MAX_FNAME];
   static char *fullname;

   _splitpath_s(fullpath, NULL, 0, NULL, 0, name, _MAX_FNAME, ext, _MAX_FNAME) == 0 ? name : NULL;

   fullname = strcat(name, ext);

   return fullname;
}

#endif

int tfaosal_filewrite(const char *fullname, unsigned char *buffer, int filelenght )
{
    FILE *f;
    int c=0;
    f = fopen( fullname, "wb");
        if (!f)
        {
            PRINT("%s, Unable to open %s\n", __FUNCTION__, fullname);
            return tfa_srv_api_error_File_Write;
        }
        c = (int)fwrite( buffer, 1, filelenght, f );
        fclose(f);
        if ( c == filelenght)
            return tfa_srv_api_error_Ok;
        else {
            PRINT("%s: File write error %s %d %d \n", __FUNCTION__, fullname, c, filelenght);
            return tfa_srv_api_error_File_Write;
        }
}
