/*
Copyright 2014 NXP Semiconductors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
/*
 * exTfa98xx.h
 *
 */

#ifndef exTFA98xx_H_
#define exTFA98xx_H_

#include "Tfa98xx.h"

enum exTfa98xx_Audio_Mode
{
    Audio_Mode_Music_Normal = 0,
    Audio_Mode_Voice
};

typedef enum exTfa98xx_Audio_Mode exTfa98xx_audio_mode_t;

int exTfa98xx_speakeron( exTfa98xx_audio_mode_t mode);

void exTfa98xx_speakeroff(void);

int exTfa98xx_calibration(int bManual);

FIXEDPT exTfa98xx_getImped25(int dev);

void exTfa98xx_setvolumestep(int leftvolume, int rightvolume);

void exTfa98xx_LR_Switch( int i32Ori );

void exTfa98xx_factorytest(int bManual);

#endif


