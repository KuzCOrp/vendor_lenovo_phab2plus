
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include "tfa98xx/app/exTfa98xx/inc/exTfa98xx.h"
#if defined(NXP_TFA9890_SUPPORT)

#elif defined(NXP_TFA9887_SUPPORT)
#include "tfa9887/interface/tfa9887_interface.h"
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
extern int tfa9890_check_tfaopen(void);
extern int tfa9890_init();
extern int tfa9890_deinit(void);
extern void tfa9890_SpeakerOn(void);
extern void tfa9890_SpeakerOff(void);
extern void tfa9890_setSamplerate(int samplerate);
extern void tfa9890_set_bypass_dsp_incall(int bypass);
extern void tfa9890_EchoReferenceConfigure(int config);
extern void tfa9890_reset(void);
int MTK_Tfa98xx_Check_TfaOpen(void);
static int tfa_Mode=0;
static int tfa_init=0;


int MTK_Tfa98xx_Check_TfaOpen(void)
{

	return 0;

}

int MTK_Tfa98xx_Init(void)
{/*
    int res;
    ALOGD("Tfa98xx: +%s",__func__);
#if defined(NXP_TFA9890_SUPPORT)
    res = tfa9890_init();
#elif defined(NXP_TFA9887_SUPPORT)
    res = tfa9887_init();
#endif
    ALOGD("Tfa98xx: -%s, res= %d",__func__,res);*/
    exTfa98xx_calibration(0);
    return 0;
}

int MTK_Tfa98xx_Deinit(void)
{

    return 0;
}

void MTK_Tfa98xx_SpeakerOn(void)
{
    ALOGD("Tfa98xx: +%s",__func__);
    if(tfa_init==0){
    	MTK_Tfa98xx_Init();
    	tfa_init=1;
    }
		exTfa98xx_speakeron(tfa_Mode);
    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_SpeakerOff(void)
{
    ALOGD("Tfa98xx: +%s",__func__);
		exTfa98xx_speakeroff();
    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_SetSampleRate(int samplerate)
{
    ALOGD("Tfa98xx: +%s, samplerate=%d",__func__,samplerate);
    if(samplerate==48000)
			tfa_Mode=0;
		else if(samplerate==16000)
			tfa_Mode=1;
			
			ALOGD("Tfa98xx: -%s",__func__);
}
void MTK_Tfa98xx_SetBypassDspIncall(int bypass)
{

}

void MTK_Tfa98xx_EchoReferenceConfigure(int config)
{

}

void MTK_Tfa98xx_Reset()
{

}
void MTK_Tfa98xx_Calibration(void)
{
	exTfa98xx_factorytest(1);
}
EXPORT_SYMBOL(MTK_Tfa98xx_Check_TfaOpen);
EXPORT_SYMBOL(MTK_Tfa98xx_Init);
EXPORT_SYMBOL(MTK_Tfa98xx_Reset);
EXPORT_SYMBOL(MTK_Tfa98xx_Deinit);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOn);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOff);
EXPORT_SYMBOL(MTK_Tfa98xx_SetSampleRate);
EXPORT_SYMBOL(MTK_Tfa98xx_SetBypassDspIncall);
EXPORT_SYMBOL(MTK_Tfa98xx_EchoReferenceConfigure);


