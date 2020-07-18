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

#ifndef VERSION_H
#define VERSION_H

#include "ProductVersion.h"

#define VER_FILEVERSION             TFA98XX_API_REV_MAJOR,TFA98XX_API_REV_MINOR
#define VER_FILEVERSION_STR         TFA98XX_API_REV_STR "\0"

#ifdef _X64
#define VER_FILEDESCRIPTION_STR     "TFA98xx 64 bit Device API Reference Code\0"
#else
#define VER_FILEDESCRIPTION_STR     "TFA98xx 32 bit Device API Reference Code\0"
#endif /* _X64 */

#define VER_INTERNALNAME_STR        "Tfa98xx.dll\0"
#define VER_ORIGINALFILENAME_STR    "Tfa98xx.dll\0"

#endif /* VERSION_H */
