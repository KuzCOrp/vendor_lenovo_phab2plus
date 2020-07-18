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

typedef enum nxpTfaBfEnumList {
    bfVDDS  = 0x0000,    /*!< Power-on-reset flag                                */
    bfPLLS  = 0x0010,    /*!< PLL lock                                           */
    bfOTDS  = 0x0020,    /*!< Over Temperature Protection alarm                  */
    bfOVDS  = 0x0030,    /*!< Over Voltage Protection alarm                      */
    bfUVDS  = 0x0040,    /*!< Under Voltage Protection alarm                     */
    bfOCDS  = 0x0050,    /*!< Over Current Protection alarm                      */
    bfCLKS  = 0x0060,    /*!< Clocks stable flag                                 */
    bfCLIPS = 0x0070,    /*!< Amplifier clipping                                 */
    bfMTPB  = 0x0080,    /*!< MTP busy                                           */
    bfNOCLK = 0x0090,    /*!< Flag lost clock from clock generation unit         */
    bfSPKS  = 0x00a0,    /*!< Speaker error flag                                 */
    bfACS   = 0x00b0,    /*!< Cold Start flag                                    */
    bfSWS   = 0x00c0,    /*!< Flag Engage                                        */
    bfWDS   = 0x00d0,    /*!< Flag watchdog reset                                */
    bfAMPS  = 0x00e0,    /*!< Amplifier is enabled by manager                    */
    bfAREFS = 0x00f0,    /*!< References are enabled by manager                  */
    bfBATS  = 0x0109,    /*!< Battery voltage readout; 0 .. 5.5 [V]              */
    bfTEMPS = 0x0208,    /*!< Temperature readout from the temperature sensor    */
    bfREV   = 0x030b,    /*!< Device type number is                           */
    bfRCV   = 0x0420,    /*!< Enable Receiver Mode                               */
    bfCHS12 = 0x0431,    /*!< Channel Selection TDM input for Coolflux           */
    bfCHSA  = 0x0461,    /*!< Input selection for amplifier                      */
    bfI2SF  = 0x0402,    /*!< I2SFormat data 1 input:                            */
//    bfCHS12 = 0x0431,    /*!< ChannelSelection data1 input  (In CoolFlux)        */
    bfCHS3  = 0x0450,    /*!< ChannelSelection data 2 input (coolflux input, the DCDC converter gets the other signal) */
//    bfCHSA  = 0x0461,    /*!< Input selection for amplifier                      */
    bfI2SDOC= 0x0481,    /*!< selection data out                                 */
    bfDISP  = 0x04a0,    /*!< idp protection                                     */
    bfI2SDOE= 0x04b0,    /*!< Enable data output                                 */
    bfI2SSR = 0x04c3,    /*!< sample rate setting                                */
    bfBSSCR = 0x0501,    /*!< Protection Attack Time                             */
    bfBSST  = 0x0523,    /*!< ProtectionThreshold                                */
    bfBSSRL = 0x0561,    /*!< Protection Maximum Reduction                       */
    bfBSSRR = 0x0582,    /*!< Battery Protection Release Time                    */
    bfBSSHY = 0x05b1,    /*!< Battery Protection Hysteresis                      */
    bfBSSR  = 0x05e0,    /*!< battery voltage for I2C read out only              */
    bfBSSBY = 0x05f0,    /*!< bypass clipper battery protection                  */
    bfBSSBY87 = 0x0500,    /*!<                                                    */
    bfBSSCR87 = 0x0511,    /*!< 00 = 0.56 dB/Sample                                */
    bfBSST87 = 0x0532,    /*!< 000 = 2.92V                                        */
    bfI2SDOC87 = 0x05f0,    /*!< selection data out                                 */
    bfDPSA  = 0x0600,    /*!< Enable dynamic powerstage activation               */
    bfCFSM  = 0x0650,    /*!< Soft mute in CoolFlux                              */
    bfBSSS  = 0x0670,    /*!< BatSenseSteepness                                  */
    bfVOL   = 0x0687,    /*!< volume control (in CoolFlux)                       */
    bfDCVO  = 0x0702,    /*!< Boost Voltage                                      */
    bfDCMCC = 0x0733,    /*!< Max boost coil current - step of 175 mA            */
    bfDCIE  = 0x07a0,    /*!< Adaptive boost mode                                */
    bfDCSR  = 0x07b0,    /*!< Soft RampUp/Down mode for DCDC controller          */
    bfDCPAVG= 0x07c0,    /*!< ctrl_peak2avg for analog part of DCDC              */
    bfTROS  = 0x0800,    /*!< Select external temperature also the ext_temp will be put on the temp read out  */
    bfEXTTS = 0x0818,    /*!< external temperature setting to be given by host   */
    bfPWDN  = 0x0900,    /*!< Device Mode                                        */
    bfI2CR  = 0x0910,    /*!< I2C Reset                                          */
    bfCFE   = 0x0920,    /*!< Enable CoolFlux                                    */
    bfAMPE  = 0x0930,    /*!< Enable Amplifier                                   */
    bfDCA   = 0x0940,    /*!< EnableBoost                                        */
    bfSBSL  = 0x0950,    /*!< Coolflux configured                                */
    bfAMPC  = 0x0960,    /*!< Selection on how Amplifier is enabled              */
    bfDCDIS = 0x0970,    /*!< DCDC not connected                                 */
    bfPSDR  = 0x0980,    /*!< IDDQ test amplifier                                */
    bfDCCV  = 0x0991,    /*!< Coil Value                                         */
    bfCCFD  = 0x09b0,    /*!< Selection CoolFlux Clock                           */
    bfINTPAD= 0x09c1,    /*!< INT pad configuration control                      */
    bfIPLL  = 0x09e0,    /*!< PLL input reference clock selection                */
    bfISEL90 = 0x09d0,    /*!< selection input 1 or 2                             */
    bfDOLS  = 0x0a02,    /*!< Output selection dataout left channel              */
    bfDORS  = 0x0a32,    /*!< Output selection dataout right channel             */
    bfSPKL  = 0x0a62,    /*!< Selection speaker induction                        */
    bfSPKR  = 0x0a91,    /*!< Selection speaker impedance                        */
    bfDCFG  = 0x0ab3,    /*!< DCDC speaker current compensation gain             */
    bfDOLS90 = 0x0a02,    /*!< Output selection dataout left channel              */
    bfDORS90 = 0x0a32,    /*!< Output selection dataout right channel             */
    bfSPKL90 = 0x0a62,    /*!< Selection speaker induction                        */
    bfSPKR90 = 0x0a91,    /*!< Selection speaker impedance                        */
    bfDCFG90 = 0x0ab3,    /*!< DCDC speaker current compensation gain             */
    bfMTPK  = 0x0b07,    /*!< 5Ah, 90d To access KEY1_Protected registers (Default for engineering) */
    bfCVFDLY= 0x0c25,    /*!< Fractional delay adjustment between current and voltage sense */
    bfVDDD  = 0x0f00,    /*!< mask flag_por for interupt generation              */
    bfOTDD  = 0x0f10,    /*!< mask flag_otpok for interupt generation            */
    bfOVDD  = 0x0f20,    /*!< mask flag_ovpok for interupt generation            */
    bfUVDD  = 0x0f30,    /*!< mask flag_uvpok for interupt generation            */
    bfOCDD  = 0x0f40,    /*!< mask flag_ocp_alarm for interupt generation        */
    bfCLKD  = 0x0f50,    /*!< mask flag_clocks_stable for interupt generation    */
    bfDCCD  = 0x0f60,    /*!< mask flag_pwrokbst for interupt generation         */
    bfSPKD  = 0x0f70,    /*!< mask flag_cf_speakererror for interupt generation  */
    bfWDD   = 0x0f80,    /*!< mask flag_watchdog_reset for interupt generation   */
    bfINT   = 0x0fe0,    /*!< enabling interrupt                                 */
    bfINTP  = 0x0ff0,    /*!< Setting polarity interupt                          */
    bfVDDD90 = 0x0f00,    /*!< mask flag_por for interupt generation              */
    bfOTDD90 = 0x0f10,    /*!< mask flag_otpok for interupt generation            */
    bfOVDD90 = 0x0f20,    /*!< mask flag_ovpok for interupt generation            */
    bfUVDD90 = 0x0f30,    /*!< mask flag_uvpok for interupt generation            */
    bfOCDD90 = 0x0f40,    /*!< mask flag_ocp_alarm for interupt generation        */
    bfCLKD90 = 0x0f50,    /*!< mask flag_clocks_stable for interupt generation    */
    bfDCCD90 = 0x0f60,    /*!< mask flag_pwrokbst for interupt generation         */
    bfSPKD90 = 0x0f70,    /*!< mask flag_cf_speakererror for interupt generation  */
    bfWDD90 = 0x0f80,    /*!< mask flag_watchdog_reset for interupt generation   */
    bfLCLK90 = 0x0f90,    /*!< mask flag_lost_clk for interupt generation         */
    bfINT90 = 0x0fe0,    /*!< enabling interrupt                                 */
    bfINTP90 = 0x0ff0,    /*!< Setting polarity interupt                          */
    bfTDMMODE= 0x1000,    /*!< TDM Mode                                           */
    bfPDMSEL= 0x1000,    /*!< PDM Mode Select                                     */
    bfPDMOUTPUTEN= 0x1010,    /*!< PDM Output Enable                             */
    bfTDMPRF= 0x1011,    /*!< TDM_usecase                                        */
    bfTDMEN = 0x1030,    /*!< TDM interface control                              */
    bfTDMCKINV= 0x1040,    /*!< TDM clock inversion                                */
    bfTDMFSLN= 0x1053,    /*!< TDM FS length                                      */
    bfTDMFSPOL= 0x1090,    /*!< TDM FS polarity                                    */
    bfTDMSAMSZ= 0x10a4,    /*!< TDM Sample Size for all tdm sinks/sources          */
    bfTDMSLOTS= 0x1103,    /*!< Number of slots                                    */
    bfTDMSLLN= 0x1144,    /*!< Slot length                                        */
    bfTDMBRMG= 0x1194,    /*!< Bits remaining                                     */
    bfTDMDDEL= 0x11e0,    /*!< Data delay                                         */
    bfTDMDADJ= 0x11f0,    /*!< Data adjustment                                    */
    bfTDMTXFRM= 0x1201,    /*!< TXDATA format                                      */
    bfTDMUUS0= 0x1221,    /*!< TXDATA format unused slot sd0                      */
    bfTDMUUS1= 0x1241,    /*!< TXDATA format unused slot sd1                      */
    bfTDMSI0EN= 0x1270,    /*!< TDM sink0 enable                                   */
    bfTDMSI1EN= 0x1280,    /*!< TDM sink1 enable                                   */
    bfTDMSI2EN= 0x1290,    /*!< TDM sink2 enable                                   */
    bfTDMSO0EN= 0x12a0,    /*!< TDM source0 enable                                 */
    bfTDMSO1EN= 0x12b0,    /*!< TDM source1 enable                                 */
    bfTDMSO2EN= 0x12c0,    /*!< TDM source2 enable                                 */
    bfTDMSI0IO= 0x12d0,    /*!< tdm_sink0_io                                       */
    bfTDMSI1IO= 0x12e0,    /*!< tdm_sink1_io                                       */
    bfTDMSI2IO= 0x12f0,    /*!< tdm_sink2_io                                       */
    bfTDMSO0IO= 0x1300,    /*!< tdm_source0_io                                     */
    bfTDMSO1IO= 0x1310,    /*!< tdm_source1_io                                     */
    bfTDMSO2IO= 0x1320,    /*!< tdm_source2_io                                     */
    bfTDMSI0SL= 0x1333,    /*!< sink0_slot [GAIN IN]                               */
    bfTDMSI1SL= 0x1373,    /*!< sink1_slot [CH1 IN]                                */
    bfTDMSI2SL= 0x13b3,    /*!< sink2_slot [CH2 IN]                                */
    bfTDMSO0SL= 0x1403,    /*!< source0_slot [GAIN OUT]                            */
    bfTDMSO1SL= 0x1443,    /*!< source1_slot [Voltage Sense]                       */
    bfTDMSO2SL= 0x1483,    /*!< source2_slot [Current Sense]                       */
    bfNBCK  = 0x14c3,    /*!< NBCK                                               */
    bfINTOVDDS= 0x2000,    /*!< flag_por_int_out                                   */
    bfINTOPLLS= 0x2010,    /*!< flag_pll_lock_int_out                              */
    bfINTOOTDS= 0x2020,    /*!< flag_otpok_int_out                                 */
    bfINTOOVDS= 0x2030,    /*!< flag_ovpok_int_out                                 */
    bfINTOUVDS= 0x2040,    /*!< flag_uvpok_int_out                                 */
    bfINTOOCDS= 0x2050,    /*!< flag_ocp_alarm_int_out                             */
    bfINTOCLKS= 0x2060,    /*!< flag_clocks_stable_int_out                         */
    bfINTOCLIPS= 0x2070,    /*!< flag_clip_int_out                                  */
    bfINTOMTPB= 0x2080,    /*!< mtp_busy_int_out                                   */
    bfINTONOCLK= 0x2090,    /*!< flag_lost_clk_int_out                              */
    bfINTOSPKS= 0x20a0,    /*!< flag_cf_speakererror_int_out                       */
    bfINTOACS= 0x20b0,    /*!< flag_cold_started_int_out                          */
    bfINTOSWS= 0x20c0,    /*!< flag_engage_int_out                                */
    bfINTOWDS= 0x20d0,    /*!< flag_por_int_out                                   */
    bfINTOAMPS= 0x20e0,    /*!< flag_por_int_out                                   */
    bfINTOAREFS= 0x20f0,    /*!< flag_por_int_out                                   */
    bfINTOACK= 0x2201,    /*!< Interrupt status register output - Corresponding flag */
    bfINTIVDDS= 0x2300,    /*!< flag_por_int_out                                   */
    bfINTIPLLS= 0x2310,    /*!< flag_pll_lock_int_in                               */
    bfINTIOTDS= 0x2320,    /*!< flag_otpok_int_in                                  */
    bfINTIOVDS= 0x2330,    /*!< flag_ovpok_int_in                                  */
    bfINTIUVDS= 0x2340,    /*!< flag_uvpok_int_in                                  */
    bfINTIOCDS= 0x2350,    /*!< flag_ocp_alarm_int_in                              */
    bfINTICLKS= 0x2360,    /*!< flag_clocks_stable_int_in                          */
    bfINTICLIPS= 0x2370,    /*!< flag_clip_int_in                                   */
    bfINTIMTPB= 0x2380,    /*!< mtp_busy_int_in                                    */
    bfINTINOCLK= 0x2390,    /*!< flag_lost_clk_int_in                               */
    bfINTISPKS= 0x23a0,    /*!< flag_cf_speakererror_int_in                        */
    bfINTIACS= 0x23b0,    /*!< flag_cold_started_int_in                           */
    bfINTISWS= 0x23c0,    /*!< flag_engage_int_in                                 */
    bfINTIWDS= 0x23d0,    /*!< flag_watchdog_reset_int_in                         */
    bfINTIAMPS= 0x23e0,    /*!< flag_enbl_amp_int_in                               */
    bfINTIAREFS= 0x23f0,    /*!< flag_enbl_ref_int_in                               */
    bfINTIACK= 0x2501,    /*!< Interrupt register input                           */
    bfINTENVDDS= 0x2600,    /*!< flag_por_int_enable                                */
    bfINTENPLLS= 0x2610,    /*!< flag_pll_lock_int_enable                           */
    bfINTENOTDS= 0x2620,    /*!< flag_otpok_int_enable                              */
    bfINTENOVDS= 0x2630,    /*!< flag_ovpok_int_enable                              */
    bfINTENUVDS= 0x2640,    /*!< flag_uvpok_int_enable                              */
    bfINTENOCDS= 0x2650,    /*!< flag_ocp_alarm_int_enable                          */
    bfINTENCLKS= 0x2660,    /*!< flag_clocks_stable_int_enable                      */
    bfINTENCLIPS= 0x2670,    /*!< flag_clip_int_enable                               */
    bfINTENMTPB= 0x2680,    /*!< mtp_busy_int_enable                                */
    bfINTENNOCLK= 0x2690,    /*!< flag_lost_clk_int_enable                           */
    bfINTENSPKS= 0x26a0,    /*!< flag_cf_speakererror_int_enable                    */
    bfINTENACS= 0x26b0,    /*!< flag_cold_started_int_enable                       */
    bfINTENSWS= 0x26c0,    /*!< flag_engage_int_enable                             */
    bfINTENWDS= 0x26d0,    /*!< flag_watchdog_reset_int_enable                     */
    bfINTENAMPS= 0x26e0,    /*!< flag_enbl_amp_int_enable                           */
    bfINTENAREFS= 0x26f0,    /*!< flag_enbl_ref_int_enable                           */
    bfINTENACK= 0x2801,    /*!< Interrupt enable register                          */
    bfINTPOLVDDS= 0x2900,    /*!< flag_por_int_pol                                   */
    bfINTPOLPLLS= 0x2910,    /*!< flag_pll_lock_int_pol                              */
    bfINTPOLOTDS= 0x2920,    /*!< flag_otpok_int_pol                                 */
    bfINTPOLOVDS= 0x2930,    /*!< flag_ovpok_int_pol                                 */
    bfINTPOLUVDS= 0x2940,    /*!< flag_uvpok_int_pol                                 */
    bfINTPOLOCDS= 0x2950,    /*!< flag_ocp_alarm_int_pol                             */
    bfINTPOLCLKS= 0x2960,    /*!< flag_clocks_stable_int_pol                         */
    bfINTPOLCLIPS= 0x2970,    /*!< flag_clip_int_pol                                  */
    bfINTPOLMTPB= 0x2980,    /*!< mtp_busy_int_pol                                   */
    bfINTPOLNOCLK= 0x2990,    /*!< flag_lost_clk_int_pol                              */
    bfINTPOLSPKS= 0x29a0,    /*!< flag_cf_speakererror_int_pol                       */
    bfINTPOLACS= 0x29b0,    /*!< flag_cold_started_int_pol                          */
    bfINTPOLSWS= 0x29c0,    /*!< flag_engage_int_pol                                */
    bfINTPOLWDS= 0x29d0,    /*!< flag_watchdog_reset_int_pol                        */
    bfINTPOLAMPS= 0x29e0,    /*!< flag_enbl_amp_int_pol                              */
    bfINTPOLAREFS= 0x29f0,    /*!< flag_enbl_ref_int_pol                              */
    bfINTPOLACK= 0x2b01,    /*!< Interrupt status flags polarity register           */
    bfPWMDEL= 0x4134,    /*!< PWM DelayBits to set the delay                     */
    bfPWMSH = 0x4180,    /*!< PWM Shape                                          */
    bfPWMRE = 0x4190,    /*!< PWM Bitlength in noise shaper                      */
    bfTCC   = 0x48e1,    /*!< sample & hold track time:                          */
    bfCLIP  = 0x4900,    /*!< Bypass clip control                                */
    bfCIMTP = 0x62b0,    /*!< start copying all the data from i2cregs_mtp to mtp [Key 2 protected] */
    bfRST   = 0x7000,    /*!< Reset CoolFlux DSP                                 */
    bfDMEM  = 0x7011,    /*!< Target memory for access                           */
    bfAIF   = 0x7030,    /*!< Autoincrement-flag for memory-address              */
    bfCFINT = 0x7040,    /*!< Interrupt CoolFlux DSP                             */
    bfREQ   = 0x7087,    /*!< request for access (8 channels)                    */
    bfMADD  = 0x710f,    /*!< memory-address to be accessed                      */
    bfMEMA  = 0x720f,    /*!< activate memory access (24- or 32-bits data is written/read to/from memory */
    bfERR   = 0x7307,    /*!< Coolflux error flags                               */
    bfACK   = 0x7387,    /*!< acknowledge of requests (8 channels)               */
    bfMTPOTC= 0x8000,    /*!< Calibration schedule (key2 protected)              */
    bfMTPEX = 0x8010,    /*!< (key2 protected)                                   */
    bfVERSION= 0x8f0f,    /*!< (key1 protected)                                   */
} nxpTfaBfEnumList_t;

typedef struct TfaBfName {
   unsigned short bfEnum;
   char  *bfName;
} tfaBfName_t;

#define TFA_NAMETABLE static tfaBfName_t TfaBfNames[]= {\
   { 0x0, "VDDS"},    /* Power-on-reset flag                               , */\
   { 0x10, "PLLS"},    /* PLL lock                                          , */\
   { 0x20, "OTDS"},    /* Over Temperature Protection alarm                 , */\
   { 0x30, "OVDS"},    /* Over Voltage Protection alarm                     , */\
   { 0x40, "UVDS"},    /* Under Voltage Protection alarm                    , */\
   { 0x50, "OCDS"},    /* Over Current Protection alarm                     , */\
   { 0x60, "CLKS"},    /* Clocks stable flag                                , */\
   { 0x70, "CLIPS"},    /* Amplifier clipping                                , */\
   { 0x80, "MTPB"},    /* MTP busy                                          , */\
   { 0x90, "NOCLK"},    /* Flag lost clock from clock generation unit        , */\
   { 0xa0, "SPKS"},    /* Speaker error flag                                , */\
   { 0xb0, "ACS"},    /* Cold Start flag                                   , */\
   { 0xc0, "SWS"},    /* Flag Engage                                       , */\
   { 0xd0, "WDS"},    /* Flag watchdog reset                               , */\
   { 0xe0, "AMPS"},    /* Amplifier is enabled by manager                   , */\
   { 0xf0, "AREFS"},    /* References are enabled by manager                 , */\
   { 0x109, "BATS"},    /* Battery voltage readout; 0 .. 5.5 [V]             , */\
   { 0x208, "TEMPS"},    /* Temperature readout from the temperature sensor   , */\
   { 0x30b, "REV"},    /* Device type number is B97                         , */\
   { 0x420, "RCV"},    /* Enable Receiver Mode                              , */\
   { 0x431, "CHS12"},    /* Channel Selection TDM input for Coolflux          , */\
   { 0x450, "INPLVL"},    /* Input level selection control                     , */\
   { 0x461, "CHSA"},    /* Input selection for amplifier                     , */\
   { 0x4c3, "AUDFS"},    /* Audio sample rate setting                         , */\
   { 0x402, "I2SF"},    /* I2SFormat data 1 input:                           , */\
   { 0x431, "CHS12"},    /* ChannelSelection data1 input  (In CoolFlux)       , */\
   { 0x450, "CHS3"},    /* ChannelSelection data 2 input (coolflux input, the DCDC converter gets the other signal), */\
   { 0x461, "CHSA"},    /* Input selection for amplifier                     , */\
   { 0x481, "I2SDOC"},    /* selection data out                                , */\
   { 0x4a0, "DISP"},    /* idp protection                                    , */\
   { 0x4b0, "I2SDOE"},    /* Enable data output                                , */\
   { 0x4c3, "I2SSR"},    /* sample rate setting                               , */\
   { 0x501, "BSSCR"},    /* Protection Attack Time                            , */\
   { 0x523, "BSST"},    /* ProtectionThreshold                               , */\
   { 0x561, "BSSRL"},    /* Protection Maximum Reduction                      , */\
   { 0x582, "BSSRR"},    /* Battery Protection Release Time                   , */\
   { 0x5b1, "BSSHY"},    /* Battery Protection Hysteresis                     , */\
   { 0x5e0, "BSSR"},    /* battery voltage for I2C read out only             , */\
   { 0x5f0, "BSSBY"},    /* bypass clipper battery protection                 , */\
   { 0x500, "BSSBY87 "},    /*                                                   , */\
   { 0x511, "BSSCR87 "},    /* 00 = 0.56 dB/Sample                               , */\
   { 0x532, "BSST87 "},    /* 000 = 2.92V                                       , */\
   { 0x5f0, "I2SDOC87 "},    /* selection data out                                , */\
   { 0x600, "DPSA"},    /* Enable dynamic powerstage activation              , */\
   { 0x650, "CFSM"},    /* Soft mute in CoolFlux                             , */\
   { 0x670, "BSSS"},    /* BatSenseSteepness                                 , */\
   { 0x687, "VOL"},    /* volume control (in CoolFlux)                      , */\
   { 0x702, "DCVO"},    /* Boost Voltage                                     , */\
   { 0x733, "DCMCC"},    /* Max boost coil current - step of 175 mA           , */\
   { 0x7a0, "DCIE"},    /* Adaptive boost mode                               , */\
   { 0x7b0, "DCSR"},    /* Soft RampUp/Down mode for DCDC controller         , */\
   { 0x7c0, "DCPAVG"},    /* ctrl_peak2avg for analog part of DCDC             , */\
   { 0x800, "TROS"},    /* Select external temperature also the ext_temp will be put on the temp read out , */\
   { 0x818, "EXTTS"},    /* external temperature setting to be given by host  , */\
   { 0x900, "PWDN"},    /* Device Mode                                       , */\
   { 0x910, "I2CR"},    /* I2C Reset                                         , */\
   { 0x920, "CFE"},    /* Enable CoolFlux                                   , */\
   { 0x930, "AMPE"},    /* Enable Amplifier                                  , */\
   { 0x940, "DCA"},    /* EnableBoost                                       , */\
   { 0x950, "SBSL"},    /* Coolflux configured                               , */\
   { 0x960, "AMPC"},    /* Selection on how Amplifier is enabled             , */\
   { 0x970, "DCDIS"},    /* DCDC not connected                                , */\
   { 0x980, "PSDR"},    /* IDDQ test amplifier                               , */\
   { 0x991, "DCCV"},    /* Coil Value                                        , */\
   { 0x9b0, "CCFD"},    /* Selection CoolFlux Clock                          , */\
   { 0x9c1, "INTPAD"},    /* INT pad configuration control                     , */\
   { 0x9e0, "IPLL"},    /* PLL input reference clock selection               , */\
   { 0x9d0, "ISEL90 "},    /* selection input 1 or 2                            , */\
   { 0xa02, "DOLS"},    /* Output selection dataout left channel             , */\
   { 0xa32, "DORS"},    /* Output selection dataout right channel            , */\
   { 0xa62, "SPKL"},    /* Selection speaker induction                       , */\
   { 0xa91, "SPKR"},    /* Selection speaker impedance                       , */\
   { 0xab3, "DCFG"},    /* DCDC speaker current compensation gain            , */\
   { 0xa02, "DOLS90 "},    /* Output selection dataout left channel             , */\
   { 0xa32, "DORS90 "},    /* Output selection dataout right channel            , */\
   { 0xa62, "SPKL90 "},    /* Selection speaker induction                       , */\
   { 0xa91, "SPKR90 "},    /* Selection speaker impedance                       , */\
   { 0xab3, "DCFG90 "},    /* DCDC speaker current compensation gain            , */\
   { 0xb07, "MTPK"},    /* 5Ah, 90d To access KEY1_Protected registers (Default for engineering), */\
   { 0xc25, "CVFDLY"},    /* Fractional delay adjustment between current and voltage sense, */\
   { 0xf00, "VDDD"},    /* mask flag_por for interupt generation             , */\
   { 0xf10, "OTDD"},    /* mask flag_otpok for interupt generation           , */\
   { 0xf20, "OVDD"},    /* mask flag_ovpok for interupt generation           , */\
   { 0xf30, "UVDD"},    /* mask flag_uvpok for interupt generation           , */\
   { 0xf40, "OCDD"},    /* mask flag_ocp_alarm for interupt generation       , */\
   { 0xf50, "CLKD"},    /* mask flag_clocks_stable for interupt generation   , */\
   { 0xf60, "DCCD"},    /* mask flag_pwrokbst for interupt generation        , */\
   { 0xf70, "SPKD"},    /* mask flag_cf_speakererror for interupt generation , */\
   { 0xf80, "WDD"},    /* mask flag_watchdog_reset for interupt generation  , */\
   { 0xfe0, "INT"},    /* enabling interrupt                                , */\
   { 0xff0, "INTP"},    /* Setting polarity interupt                         , */\
   { 0xf00, "VDDD90 "},    /* mask flag_por for interupt generation             , */\
   { 0xf10, "OTDD90 "},    /* mask flag_otpok for interupt generation           , */\
   { 0xf20, "OVDD90 "},    /* mask flag_ovpok for interupt generation           , */\
   { 0xf30, "UVDD90 "},    /* mask flag_uvpok for interupt generation           , */\
   { 0xf40, "OCDD90 "},    /* mask flag_ocp_alarm for interupt generation       , */\
   { 0xf50, "CLKD90 "},    /* mask flag_clocks_stable for interupt generation   , */\
   { 0xf60, "DCCD90 "},    /* mask flag_pwrokbst for interupt generation        , */\
   { 0xf70, "SPKD90 "},    /* mask flag_cf_speakererror for interupt generation , */\
   { 0xf80, "WDD90 "},    /* mask flag_watchdog_reset for interupt generation  , */\
   { 0xf90, "LCLK90 "},    /* mask flag_lost_clk for interupt generation        , */\
   { 0xfe0, "INT90 "},    /* enabling interrupt                                , */\
   { 0xff0, "INTP90 "},    /* Setting polarity interupt                         , */\
   { 0x1000, "TDMMODE"},    /* TDM Mode                                          , */\
   { 0x1011, "TDMPRF"},    /* TDM_usecase                                       , */\
   { 0x1000, "PDMSEL"},    /* PDM Mode Select                                   , */\
   { 0x1010, "PDMOUTPUTEN"},    /* PDM Output Enable                            , */\
   { 0x1030, "TDMEN"},    /* TDM interface control                             , */\
   { 0x1040, "TDMCKINV"},    /* TDM clock inversion                               , */\
   { 0x1053, "TDMFSLN"},    /* TDM FS length                                     , */\
   { 0x1090, "TDMFSPOL"},    /* TDM FS polarity                                   , */\
   { 0x1103, "TDMSLOTS"},    /* Number of slots                                   , */\
   { 0x1144, "TDMSLLN"},    /* Slot length                                       , */\
   { 0x1194, "TDMBRMG"},    /* Bits remaining                                    , */\
   { 0x11e0, "TDMDDEL"},    /* Data delay                                        , */\
   { 0x11f0, "TDMDADJ"},    /* Data adjustment                                   , */\
   { 0x1201, "TDMCOMP"},    /* sample_compression                                , */\
   { 0x1221, "TDMTXFRM"},    /* TXDATA format                                     , */\
   { 0x1241, "TDMUUS1"},    /* TXDATA format unused slot sd1                     , */\
   { 0x1270, "TDMSI0EN"},    /* TDM sink0 enable                                  , */\
   { 0x1280, "TDMSI1EN"},    /* TDM sink1 enable                                  , */\
   { 0x1290, "TDMSI2EN"},    /* TDM sink2 enable                                  , */\
   { 0x12a0, "TDMSO0EN"},    /* TDM source0 enable                                , */\
   { 0x12b0, "TDMSO1EN"},    /* TDM source1 enable                                , */\
   { 0x12c0, "TDMSO2EN"},    /* TDM source2 enable                                , */\
   { 0x12d0, "TDMSI0IO"},    /* tdm_sink0_io                                      , */\
   { 0x12e0, "TDMSI1IO"},    /* tdm_sink1_io                                      , */\
   { 0x12f0, "TDMSI2IO"},    /* tdm_sink2_io                                      , */\
   { 0x1300, "TDMSO0IO"},    /* tdm_source0_io                                    , */\
   { 0x1310, "TDMSO1IO"},    /* tdm_source1_io                                    , */\
   { 0x1320, "TDMSO2IO"},    /* tdm_source2_io                                    , */\
   { 0x1333, "TDMSI0SL"},    /* sink0_slot [GAIN IN]                              , */\
   { 0x1373, "TDMSI1SL"},    /* sink1_slot [CH1 IN]                               , */\
   { 0x13b3, "TDMSI2SL"},    /* sink2_slot [CH2 IN]                               , */\
   { 0x1403, "TDMSO0SL"},    /* source0_slot [GAIN OUT]                           , */\
   { 0x1443, "TDMSO1SL"},    /* source1_slot [Voltage Sense]                      , */\
   { 0x1483, "TDMSO2SL"},    /* source2_slot [Current Sense]                      , */\
   { 0x14c3, "NBCK"},    /* NBCK                                              , */\
   { 0x2000, "INTOVDDS"},    /* flag_por_int_out                                  , */\
   { 0x2010, "INTOPLLS"},    /* flag_pll_lock_int_out                             , */\
   { 0x2020, "INTOOTDS"},    /* flag_otpok_int_out                                , */\
   { 0x2030, "INTOOVDS"},    /* flag_ovpok_int_out                                , */\
   { 0x2040, "INTOUVDS"},    /* flag_uvpok_int_out                                , */\
   { 0x2050, "INTOOCDS"},    /* flag_ocp_alarm_int_out                            , */\
   { 0x2060, "INTOCLKS"},    /* flag_clocks_stable_int_out                        , */\
   { 0x2070, "INTOCLIPS"},    /* flag_clip_int_out                                 , */\
   { 0x2080, "INTOMTPB"},    /* mtp_busy_int_out                                  , */\
   { 0x2090, "INTONOCLK"},    /* flag_lost_clk_int_out                             , */\
   { 0x20a0, "INTOSPKS"},    /* flag_cf_speakererror_int_out                      , */\
   { 0x20b0, "INTOACS"},    /* flag_cold_started_int_out                         , */\
   { 0x20c0, "INTOSWS"},    /* flag_engage_int_out                               , */\
   { 0x20d0, "INTOWDS"},    /* flag_por_int_out                                  , */\
   { 0x20e0, "INTOAMPS"},    /* flag_por_int_out                                  , */\
   { 0x20f0, "INTOAREFS"},    /* flag_por_int_out                                  , */\
   { 0x2201, "INTOACK"},    /* Interrupt status register output - Corresponding flag, */\
   { 0x2300, "INTIVDDS"},    /* flag_por_int_out                                  , */\
   { 0x2310, "INTIPLLS"},    /* flag_pll_lock_int_in                              , */\
   { 0x2320, "INTIOTDS"},    /* flag_otpok_int_in                                 , */\
   { 0x2330, "INTIOVDS"},    /* flag_ovpok_int_in                                 , */\
   { 0x2340, "INTIUVDS"},    /* flag_uvpok_int_in                                 , */\
   { 0x2350, "INTIOCDS"},    /* flag_ocp_alarm_int_in                             , */\
   { 0x2360, "INTICLKS"},    /* flag_clocks_stable_int_in                         , */\
   { 0x2370, "INTICLIPS"},    /* flag_clip_int_in                                  , */\
   { 0x2380, "INTIMTPB"},    /* mtp_busy_int_in                                   , */\
   { 0x2390, "INTINOCLK"},    /* flag_lost_clk_int_in                              , */\
   { 0x23a0, "INTISPKS"},    /* flag_cf_speakererror_int_in                       , */\
   { 0x23b0, "INTIACS"},    /* flag_cold_started_int_in                          , */\
   { 0x23c0, "INTISWS"},    /* flag_engage_int_in                                , */\
   { 0x23d0, "INTIWDS"},    /* flag_watchdog_reset_int_in                        , */\
   { 0x23e0, "INTIAMPS"},    /* flag_enbl_amp_int_in                              , */\
   { 0x23f0, "INTIAREFS"},    /* flag_enbl_ref_int_in                              , */\
   { 0x2501, "INTIACK"},    /* Interrupt register input                          , */\
   { 0x2600, "INTENVDDS"},    /* flag_por_int_enable                               , */\
   { 0x2610, "INTENPLLS"},    /* flag_pll_lock_int_enable                          , */\
   { 0x2620, "INTENOTDS"},    /* flag_otpok_int_enable                             , */\
   { 0x2630, "INTENOVDS"},    /* flag_ovpok_int_enable                             , */\
   { 0x2640, "INTENUVDS"},    /* flag_uvpok_int_enable                             , */\
   { 0x2650, "INTENOCDS"},    /* flag_ocp_alarm_int_enable                         , */\
   { 0x2660, "INTENCLKS"},    /* flag_clocks_stable_int_enable                     , */\
   { 0x2670, "INTENCLIPS"},    /* flag_clip_int_enable                              , */\
   { 0x2680, "INTENMTPB"},    /* mtp_busy_int_enable                               , */\
   { 0x2690, "INTENNOCLK"},    /* flag_lost_clk_int_enable                          , */\
   { 0x26a0, "INTENSPKS"},    /* flag_cf_speakererror_int_enable                   , */\
   { 0x26b0, "INTENACS"},    /* flag_cold_started_int_enable                      , */\
   { 0x26c0, "INTENSWS"},    /* flag_engage_int_enable                            , */\
   { 0x26d0, "INTENWDS"},    /* flag_watchdog_reset_int_enable                    , */\
   { 0x26e0, "INTENAMPS"},    /* flag_enbl_amp_int_enable                          , */\
   { 0x26f0, "INTENAREFS"},    /* flag_enbl_ref_int_enable                          , */\
   { 0x2801, "INTENACK"},    /* Interrupt enable register                         , */\
   { 0x2900, "INTPOLVDDS"},    /* flag_por_int_pol                                  , */\
   { 0x2910, "INTPOLPLLS"},    /* flag_pll_lock_int_pol                             , */\
   { 0x2920, "INTPOLOTDS"},    /* flag_otpok_int_pol                                , */\
   { 0x2930, "INTPOLOVDS"},    /* flag_ovpok_int_pol                                , */\
   { 0x2940, "INTPOLUVDS"},    /* flag_uvpok_int_pol                                , */\
   { 0x2950, "INTPOLOCDS"},    /* flag_ocp_alarm_int_pol                            , */\
   { 0x2960, "INTPOLCLKS"},    /* flag_clocks_stable_int_pol                        , */\
   { 0x2970, "INTPOLCLIPS"},    /* flag_clip_int_pol                                 , */\
   { 0x2980, "INTPOLMTPB"},    /* mtp_busy_int_pol                                  , */\
   { 0x2990, "INTPOLNOCLK"},    /* flag_lost_clk_int_pol                             , */\
   { 0x29a0, "INTPOLSPKS"},    /* flag_cf_speakererror_int_pol                      , */\
   { 0x29b0, "INTPOLACS"},    /* flag_cold_started_int_pol                         , */\
   { 0x29c0, "INTPOLSWS"},    /* flag_engage_int_pol                               , */\
   { 0x29d0, "INTPOLWDS"},    /* flag_watchdog_reset_int_pol                       , */\
   { 0x29e0, "INTPOLAMPS"},    /* flag_enbl_amp_int_pol                             , */\
   { 0x29f0, "INTPOLAREFS"},    /* flag_enbl_ref_int_pol                             , */\
   { 0x2b01, "INTPOLACK"},    /* Interrupt status flags polarity register          , */\
   { 0x4134, "PWMDEL"},    /* PWM DelayBits to set the delay                    , */\
   { 0x4180, "PWMSH"},    /* PWM Shape                                         , */\
   { 0x4190, "PWMRE"},    /* PWM Bitlength in noise shaper                     , */\
   { 0x48e1, "TCC"},    /* sample & hold track time:                         , */\
   { 0x4900, "CLIP"},    /* Bypass clip control                               , */\
   { 0x62b0, "CIMTP"},    /* start copying all the data from i2cregs_mtp to mtp [Key 2 protected], */\
   { 0x7000, "RST"},    /* Reset CoolFlux DSP                                , */\
   { 0x7011, "DMEM"},    /* Target memory for access                          , */\
   { 0x7030, "AIF"},    /* Autoincrement-flag for memory-address             , */\
   { 0x7040, "CFINT"},    /* Interrupt CoolFlux DSP                            , */\
   { 0x7087, "REQ"},    /* request for access (8 channels)                   , */\
   { 0x710f, "MADD"},    /* memory-address to be accessed                     , */\
   { 0x720f, "MEMA"},    /* activate memory access (24- or 32-bits data is written/read to/from memory, */\
   { 0x7307, "ERR"},    /* Coolflux error flags                              , */\
   { 0x7387, "ACK"},    /* acknowledge of requests (8 channels)              , */\
   { 0x8000, "MTPOTC"},    /* Calibration schedule (key2 protected)             , */\
   { 0x8010, "MTPEX"},    /* (key2 protected)                                  , */\
    0xffff,"Unknown bitfield enum"    /* not found */\
};

#define TFA_BITNAMETABLE static tfaBfName_t TfaBfNames[]= {\
   { 0x0, "lag_por"},    /* Power-on-reset flag                               , */\
   { 0x10, "lag_pll_lock"},    /* PLL lock                                          , */\
   { 0x20, "lag_otpok"},    /* Over Temperature Protection alarm                 , */\
   { 0x30, "lag_ovpok"},    /* Over Voltage Protection alarm                     , */\
   { 0x40, "lag_uvpok"},    /* Under Voltage Protection alarm                    , */\
   { 0x50, "lag_ocp_alarm"},    /* Over Current Protection alarm                     , */\
   { 0x60, "lag_clocks_stable"},    /* Clocks stable flag                                , */\
   { 0x70, "lag_clip"},    /* Amplifier clipping                                , */\
   { 0x80, "mtp_busy"},    /* MTP busy                                          , */\
   { 0x90, "lag_lost_clk"},    /* Flag lost clock from clock generation unit        , */\
   { 0xa0, "lag_cf_speakererror"},    /* Speaker error flag                                , */\
   { 0xb0, "lag_cold_started"},    /* Cold Start flag                                   , */\
   { 0xc0, "lag_engage"},    /* Flag Engage                                       , */\
   { 0xd0, "lag_watchdog_reset"},    /* Flag watchdog reset                               , */\
   { 0xe0, "lag_enbl_amp"},    /* Amplifier is enabled by manager                   , */\
   { 0xf0, "lag_enbl_ref"},    /* References are enabled by manager                 , */\
   { 0x109, "at_adc"},    /* Battery voltage readout; 0 .. 5.5 [V]             , */\
   { 0x208, "temp_adc"},    /* Temperature readout from the temperature sensor   , */\
   { 0x30b, "rev_reg"},    /* Device type number is B97                         , */\
   { 0x420, "ctrl_rcv"},    /* Enable Receiver Mode                              , */\
   { 0x431, "chan_sel"},    /* Channel Selection TDM input for Coolflux          , */\
   { 0x450, "input_level"},    /* Input level selection control                     , */\
   { 0x461, "vamp_sel"},    /* Input selection for amplifier                     , */\
   { 0x4c3, "audio_fs"},    /* Audio sample rate setting                         , */\
   { 0x501, "vbat_prot_attacktime"},    /* Protection Attack Time                            , */\
   { 0x523, "vbat_prot_thlevel"},    /* ProtectionThreshold                               , */\
   { 0x561, "vbat_prot_max_reduct"},    /* Protection Maximum Reduction                      , */\
   { 0x582, "vbat_prot_release_t"},    /* Battery Protection Release Time                   , */\
   { 0x5b1, "vbat_prot_hysterese"},    /* Battery Protection Hysteresis                     , */\
   { 0x5d0, "reset_min_vbat"},    /* reset clipper                                     , */\
   { 0x5e0, "sel_vbat"},    /* battery voltage for I2C read out only             , */\
   { 0x5f0, "ypass_clipper"},    /* bypass clipper battery protection                 , */\
   { 0x600, "dpsa"},    /* Enable dynamic powerstage activation              , */\
   { 0x650, "cf_mute"},    /* Soft mute in CoolFlux                             , */\
   { 0x670, "atsense_steepness"},    /* BatSenseSteepness                                 , */\
   { 0x687, "vol"},    /* volume control (in CoolFlux)                      , */\
   { 0x702, "oost_volt"},    /* Boost Voltage                                     , */\
   { 0x733, "oost_cur"},    /* Max boost coil current - step of 175 mA           , */\
   { 0x7a0, "oost_intel"},    /* Adaptive boost mode                               , */\
   { 0x7b0, "oost_speed"},    /* Soft RampUp/Down mode for DCDC controller         , */\
   { 0x7c0, "oost_peak2avg"},    /* ctrl_peak2avg for analog part of DCDC             , */\
   { 0x800, "ext_temp_sel"},    /* Select external temperature also the ext_temp will be put on the temp read out , */\
   { 0x818, "ext_temp"},    /* external temperature setting to be given by host  , */\
   { 0x8b2, "dcdc_synchronisation"},    /* DCDC synchronisation off + 7 positions            , */\
   { 0x900, "powerdown"},    /* Device Mode                                       , */\
   { 0x910, "reset"},    /* I2C Reset                                         , */\
   { 0x920, "enbl_coolflux"},    /* Enable CoolFlux                                   , */\
   { 0x930, "enbl_amplifier"},    /* Enable Amplifier                                  , */\
   { 0x940, "enbl_boost"},    /* EnableBoost                                       , */\
   { 0x950, "coolflux_configured"},    /* Coolflux configured                               , */\
   { 0x960, "sel_enbl_amplifier"},    /* Selection on how Amplifier is enabled             , */\
   { 0x970, "dcdcoff_mode"},    /* DCDC not connected                                , */\
   { 0x980, "iddqtest"},    /* IDDQ test amplifier                               , */\
   { 0x991, "coil_value"},    /* Coil Value                                        , */\
   { 0x9b0, "sel_cf_clock"},    /* Selection CoolFlux Clock                          , */\
   { 0x9c1, "int_pad_io"},    /* INT pad configuration control                     , */\
   { 0x9e0, "sel_fs_bck"},    /* PLL input reference clock selection               , */\
   { 0x9f0, "sel_scl_cf_clock"},    /* Coolflux sub-system clock                         , */\
   { 0xb07, "mtpkey2"},    /* 5Ah, 90d To access KEY1_Protected registers (Default for engineering), */\
   { 0xc00, "enbl_volt_sense"},    /* Voltage sense enabling control bit                , */\
   { 0xc10, "vsense_pwm_sel"},    /* Voltage sense PWM source selection control        , */\
   { 0xc25, "vi_frac_delay"},    /* Fractional delay adjustment between current and voltage sense, */\
   { 0xc80, "sel_voltsense_out"},    /* TDM output data selection control                 , */\
   { 0xc90, "vsense_bypass_avg"},    /* Voltage Sense Average Block Bypass                , */\
   { 0xd05, "cf_frac_delay"},    /* Fractional delay adjustment between current and voltage sense by firmware, */\
   { 0xe00, "ypass_dcdc_curr_prot"},    /* Control to switch off dcdc current reduction with bat protection, */\
   { 0xe80, "disable_clock_sh_prot"},    /* disable clock_sh protection                       , */\
   { 0xe96, "reserve_reg_1_15_9"},    /*                                                   , */\
   { 0x1011, "tdm_usecase"},    /* TDM_usecase                                       , */\
   { 0x1030, "tdm_enable"},    /* TDM interface control                             , */\
   { 0x1040, "tdm_clk_inversion"},    /* TDM clock inversion                               , */\
   { 0x1053, "tdm_fs_ws_length"},    /* TDM FS length                                     , */\
   { 0x1090, "tdm_fs_ws_polarity"},    /* TDM FS polarity                                   , */\
   { 0x10a4, "tdm_sample_size"},    /* TDM Sample Size for all tdm sinks/sources         , */\
   { 0x1103, "tdm_nb_of_slots"},    /* Number of slots                                   , */\
   { 0x1144, "tdm_slot_length"},    /* Slot length                                       , */\
   { 0x1194, "tdm_bits_remaining"},    /* Bits remaining                                    , */\
   { 0x11e0, "tdm_data_delay"},    /* Data delay                                        , */\
   { 0x11f0, "tdm_data_adjustment"},    /* Data adjustment                                   , */\
   { 0x1201, "tdm_txdata_format"},    /* TXDATA format                                     , */\
   { 0x1221, "tdm_txdata_format_unused_slot_sd0"},    /* TXDATA format unused slot sd0                     , */\
   { 0x1241, "tdm_txdata_format_unused_slot_sd1"},    /* TXDATA format unused slot sd1                     , */\
   { 0x1270, "tdm_sink0_enable"},    /* TDM sink0 enable                                  , */\
   { 0x1280, "tdm_sink1_enable"},    /* TDM sink1 enable                                  , */\
   { 0x1290, "tdm_sink2_enable"},    /* TDM sink2 enable                                  , */\
   { 0x12a0, "tdm_source0_enable"},    /* TDM source0 enable                                , */\
   { 0x12b0, "tdm_source1_enable"},    /* TDM source1 enable                                , */\
   { 0x12c0, "tdm_source2_enable"},    /* TDM source2 enable                                , */\
   { 0x12d0, "tdm_sink0_io"},    /* tdm_sink0_io                                      , */\
   { 0x12e0, "tdm_sink1_io"},    /* tdm_sink1_io                                      , */\
   { 0x12f0, "tdm_sink2_io"},    /* tdm_sink2_io                                      , */\
   { 0x1300, "tdm_source0_io"},    /* tdm_source0_io                                    , */\
   { 0x1310, "tdm_source1_io"},    /* tdm_source1_io                                    , */\
   { 0x1320, "tdm_source2_io"},    /* tdm_source2_io                                    , */\
   { 0x1333, "tdm_sink0_slot"},    /* sink0_slot [GAIN IN]                              , */\
   { 0x1373, "tdm_sink1_slot"},    /* sink1_slot [CH1 IN]                               , */\
   { 0x13b3, "tdm_sink2_slot"},    /* sink2_slot [CH2 IN]                               , */\
   { 0x1403, "tdm_source0_slot"},    /* source0_slot [GAIN OUT]                           , */\
   { 0x1443, "tdm_source1_slot"},    /* source1_slot [Voltage Sense]                      , */\
   { 0x1483, "tdm_source2_slot"},    /* source2_slot [Current Sense]                      , */\
   { 0x14c3, "tdm_nbck"},    /* NBCK                                              , */\
   { 0x1500, "lag_tdm_lut_error"},    /* TDM LUT error flag                                , */\
   { 0x1512, "lag_tdm_status"},    /* TDM interface status bits                         , */\
   { 0x1540, "lag_tdm_error"},    /* TDM interface error indicator                     , */\
   { 0x2000, "lag_por_int_out"},    /* flag_por_int_out                                  , */\
   { 0x2010, "lag_pll_lock_int_out"},    /* flag_pll_lock_int_out                             , */\
   { 0x2020, "lag_otpok_int_out"},    /* flag_otpok_int_out                                , */\
   { 0x2030, "lag_ovpok_int_out"},    /* flag_ovpok_int_out                                , */\
   { 0x2040, "lag_uvpok_int_out"},    /* flag_uvpok_int_out                                , */\
   { 0x2050, "lag_ocp_alarm_int_out"},    /* flag_ocp_alarm_int_out                            , */\
   { 0x2060, "lag_clocks_stable_int_out"},    /* flag_clocks_stable_int_out                        , */\
   { 0x2070, "lag_clip_int_out"},    /* flag_clip_int_out                                 , */\
   { 0x2080, "mtp_busy_int_out"},    /* mtp_busy_int_out                                  , */\
   { 0x2090, "lag_lost_clk_int_out"},    /* flag_lost_clk_int_out                             , */\
   { 0x20a0, "lag_cf_speakererror_int_out"},    /* flag_cf_speakererror_int_out                      , */\
   { 0x20b0, "lag_cold_started_int_out"},    /* flag_cold_started_int_out                         , */\
   { 0x20c0, "lag_engage_int_out"},    /* flag_engage_int_out                               , */\
   { 0x20d0, "lag_watchdog_reset_int_out"},    /* flag_watchdog_reset_int_out                       , */\
   { 0x20e0, "lag_enbl_amp_int_out"},    /* flag_enbl_amp_int_out                             , */\
   { 0x20f0, "lag_enbl_ref_int_out"},    /* flag_enbl_ref_int_out                             , */\
   { 0x2100, "lag_voutcomp_int_out"},    /* flag_voutcomp_int_out                             , */\
   { 0x2110, "lag_voutcomp93_int_out"},    /* flag_voutcomp93_int_out                           , */\
   { 0x2120, "lag_voutcomp86_int_out"},    /* flag_voutcomp86_int_out                           , */\
   { 0x2130, "lag_hiz_int_out"},    /* flag_hiz_int_out                                  , */\
   { 0x2140, "lag_ocpokbst_int_out"},    /* flag_ocpokbst_int_out                             , */\
   { 0x2150, "lag_peakcur_int_out"},    /* flag_peakcur_int_out                              , */\
   { 0x2160, "lag_ocpokap_int_out"},    /* flag_ocpokap_int_out                              , */\
   { 0x2170, "lag_ocpokan_int_out"},    /* flag_ocpokan_int_out                              , */\
   { 0x2180, "lag_ocpokbp_int_out"},    /* flag_ocpokbp_int_out                              , */\
   { 0x2190, "lag_ocpokbn_int_out"},    /* flag_ocpokbn_int_out                              , */\
   { 0x21a0, "lag_adc10_ready_int_out"},    /* flag_adc10_ready_int_out                          , */\
   { 0x21b0, "lag_clipa_high_int_out"},    /* flag_clipa_high_int_out                           , */\
   { 0x21c0, "lag_clipa_low_int_out"},    /* flag_clipa_low_int_out                            , */\
   { 0x21d0, "lag_clipb_high_int_out"},    /* flag_clipb_high_int_out                           , */\
   { 0x21e0, "lag_clipb_low_int_out"},    /* flag_clipb_low_int_out                            , */\
   { 0x21f0, "lag_tdm_error_int_out"},    /* flag_tdm_error_int_out                            , */\
   { 0x2201, "interrupt_out3"},    /* Interrupt status register output - Corresponding flag, */\
   { 0x2300, "lag_por_int_in"},    /* flag_por_int_in                                   , */\
   { 0x2310, "lag_pll_lock_int_in"},    /* flag_pll_lock_int_in                              , */\
   { 0x2320, "lag_otpok_int_in"},    /* flag_otpok_int_in                                 , */\
   { 0x2330, "lag_ovpok_int_in"},    /* flag_ovpok_int_in                                 , */\
   { 0x2340, "lag_uvpok_int_in"},    /* flag_uvpok_int_in                                 , */\
   { 0x2350, "lag_ocp_alarm_int_in"},    /* flag_ocp_alarm_int_in                             , */\
   { 0x2360, "lag_clocks_stable_int_in"},    /* flag_clocks_stable_int_in                         , */\
   { 0x2370, "lag_clip_int_in"},    /* flag_clip_int_in                                  , */\
   { 0x2380, "mtp_busy_int_in"},    /* mtp_busy_int_in                                   , */\
   { 0x2390, "lag_lost_clk_int_in"},    /* flag_lost_clk_int_in                              , */\
   { 0x23a0, "lag_cf_speakererror_int_in"},    /* flag_cf_speakererror_int_in                       , */\
   { 0x23b0, "lag_cold_started_int_in"},    /* flag_cold_started_int_in                          , */\
   { 0x23c0, "lag_engage_int_in"},    /* flag_engage_int_in                                , */\
   { 0x23d0, "lag_watchdog_reset_int_in"},    /* flag_watchdog_reset_int_in                        , */\
   { 0x23e0, "lag_enbl_amp_int_in"},    /* flag_enbl_amp_int_in                              , */\
   { 0x23f0, "lag_enbl_ref_int_in"},    /* flag_enbl_ref_int_in                              , */\
   { 0x2400, "lag_voutcomp_int_in"},    /* flag_voutcomp_int_in                              , */\
   { 0x2410, "lag_voutcomp93_int_in"},    /* flag_voutcomp93_int_in                            , */\
   { 0x2420, "lag_voutcomp86_int_in"},    /* flag_voutcomp86_int_in                            , */\
   { 0x2430, "lag_hiz_int_in"},    /* flag_hiz_int_in                                   , */\
   { 0x2440, "lag_ocpokbst_int_in"},    /* flag_ocpokbst_int_in                              , */\
   { 0x2450, "lag_peakcur_int_in"},    /* flag_peakcur_int_in                               , */\
   { 0x2460, "lag_ocpokap_int_in"},    /* flag_ocpokap_int_in                               , */\
   { 0x2470, "lag_ocpokan_int_in"},    /* flag_ocpokan_int_in                               , */\
   { 0x2480, "lag_ocpokbp_int_in"},    /* flag_ocpokbp_int_in                               , */\
   { 0x2490, "lag_ocpokbn_int_in"},    /* flag_ocpokbn_int_in                               , */\
   { 0x24a0, "lag_adc10_ready_int_in"},    /* flag_adc10_ready_int_in                           , */\
   { 0x24b0, "lag_clipa_high_int_in"},    /* flag_clipa_high_int_in                            , */\
   { 0x24c0, "lag_clipa_low_int_in"},    /* flag_clipa_low_int_in                             , */\
   { 0x24d0, "lag_clipb_high_int_in"},    /* flag_clipb_high_int_in                            , */\
   { 0x24e0, "lag_clipb_low_int_in"},    /* flag_clipb_low_int_in                             , */\
   { 0x24f0, "lag_tdm_error_int_in"},    /* flag_tdm_error_int_in                             , */\
   { 0x2501, "interrupt_in3"},    /* Interrupt register input                          , */\
   { 0x2600, "lag_por_int_enable"},    /* flag_por_int_enable                               , */\
   { 0x2610, "lag_pll_lock_int_enable"},    /* flag_pll_lock_int_enable                          , */\
   { 0x2620, "lag_otpok_int_enable"},    /* flag_otpok_int_enable                             , */\
   { 0x2630, "lag_ovpok_int_enable"},    /* flag_ovpok_int_enable                             , */\
   { 0x2640, "lag_uvpok_int_enable"},    /* flag_uvpok_int_enable                             , */\
   { 0x2650, "lag_ocp_alarm_int_enable"},    /* flag_ocp_alarm_int_enable                         , */\
   { 0x2660, "lag_clocks_stable_int_enable"},    /* flag_clocks_stable_int_enable                     , */\
   { 0x2670, "lag_clip_int_enable"},    /* flag_clip_int_enable                              , */\
   { 0x2680, "mtp_busy_int_enable"},    /* mtp_busy_int_enable                               , */\
   { 0x2690, "lag_lost_clk_int_enable"},    /* flag_lost_clk_int_enable                          , */\
   { 0x26a0, "lag_cf_speakererror_int_enable"},    /* flag_cf_speakererror_int_enable                   , */\
   { 0x26b0, "lag_cold_started_int_enable"},    /* flag_cold_started_int_enable                      , */\
   { 0x26c0, "lag_engage_int_enable"},    /* flag_engage_int_enable                            , */\
   { 0x26d0, "lag_watchdog_reset_int_enable"},    /* flag_watchdog_reset_int_enable                    , */\
   { 0x26e0, "lag_enbl_amp_int_enable"},    /* flag_enbl_amp_int_enable                          , */\
   { 0x26f0, "lag_enbl_ref_int_enable"},    /* flag_enbl_ref_int_enable                          , */\
   { 0x2700, "lag_voutcomp_int_enable"},    /* flag_voutcomp_int_enable                          , */\
   { 0x2710, "lag_voutcomp93_int_enable"},    /* flag_voutcomp93_int_enable                        , */\
   { 0x2720, "lag_voutcomp86_int_enable"},    /* flag_voutcomp86_int_enable                        , */\
   { 0x2730, "lag_hiz_int_enable"},    /* flag_hiz_int_enable                               , */\
   { 0x2740, "lag_ocpokbst_int_enable"},    /* flag_ocpokbst_int_enable                          , */\
   { 0x2750, "lag_peakcur_int_enable"},    /* flag_peakcur_int_enable                           , */\
   { 0x2760, "lag_ocpokap_int_enable"},    /* flag_ocpokap_int_enable                           , */\
   { 0x2770, "lag_ocpokan_int_enable"},    /* flag_ocpokan_int_enable                           , */\
   { 0x2780, "lag_ocpokbp_int_enable"},    /* flag_ocpokbp_int_enable                           , */\
   { 0x2790, "lag_ocpokbn_int_enable"},    /* flag_ocpokbn_int_enable                           , */\
   { 0x27a0, "lag_adc10_ready_int_enable"},    /* flag_adc10_ready_int_enable                       , */\
   { 0x27b0, "lag_clipa_high_int_enable"},    /* flag_clipa_high_int_enable                        , */\
   { 0x27c0, "lag_clipa_low_int_enable"},    /* flag_clipa_low_int_enable                         , */\
   { 0x27d0, "lag_clipb_high_int_enable"},    /* flag_clipb_high_int_enable                        , */\
   { 0x27e0, "lag_clipb_low_int_enable"},    /* flag_clipb_low_int_enable                         , */\
   { 0x27f0, "lag_tdm_error_int_enable"},    /* flag_tdm_error_int_enable                         , */\
   { 0x2801, "interrupt_enable3"},    /* Interrupt enable register                         , */\
   { 0x2900, "lag_por_int_pol"},    /* flag_por_int_pol                                  , */\
   { 0x2910, "lag_pll_lock_int_pol"},    /* flag_pll_lock_int_pol                             , */\
   { 0x2920, "lag_otpok_int_pol"},    /* flag_otpok_int_pol                                , */\
   { 0x2930, "lag_ovpok_int_pol"},    /* flag_ovpok_int_pol                                , */\
   { 0x2940, "lag_uvpok_int_pol"},    /* flag_uvpok_int_pol                                , */\
   { 0x2950, "lag_ocp_alarm_int_pol"},    /* flag_ocp_alarm_int_pol                            , */\
   { 0x2960, "lag_clocks_stable_int_pol"},    /* flag_clocks_stable_int_pol                        , */\
   { 0x2970, "lag_clip_int_pol"},    /* flag_clip_int_pol                                 , */\
   { 0x2980, "mtp_busy_int_pol"},    /* mtp_busy_int_pol                                  , */\
   { 0x2990, "lag_lost_clk_int_pol"},    /* flag_lost_clk_int_pol                             , */\
   { 0x29a0, "lag_cf_speakererror_int_pol"},    /* flag_cf_speakererror_int_pol                      , */\
   { 0x29b0, "lag_cold_started_int_pol"},    /* flag_cold_started_int_pol                         , */\
   { 0x29c0, "lag_engage_int_pol"},    /* flag_engage_int_pol                               , */\
   { 0x29d0, "lag_watchdog_reset_int_pol"},    /* flag_watchdog_reset_int_pol                       , */\
   { 0x29e0, "lag_enbl_amp_int_pol"},    /* flag_enbl_amp_int_pol                             , */\
   { 0x29f0, "lag_enbl_ref_int_pol"},    /* flag_enbl_ref_int_pol                             , */\
   { 0x2a00, "lag_voutcomp_int_pol"},    /* flag_voutcomp_int_pol                             , */\
   { 0x2a10, "lag_voutcomp93_int_pol"},    /* flag_voutcomp93_int_pol                           , */\
   { 0x2a20, "lag_voutcomp86_int_pol"},    /* flag_voutcomp86_int_pol                           , */\
   { 0x2a30, "lag_hiz_int_pol"},    /* flag_hiz_int_pol                                  , */\
   { 0x2a40, "lag_ocpokbst_int_pol"},    /* flag_ocpokbst_int_pol                             , */\
   { 0x2a50, "lag_peakcur_int_pol"},    /* flag_peakcur_int_pol                              , */\
   { 0x2a60, "lag_ocpokap_int_pol"},    /* flag_ocpokap_int_pol                              , */\
   { 0x2a70, "lag_ocpokan_int_pol"},    /* flag_ocpokan_int_pol                              , */\
   { 0x2a80, "lag_ocpokbp_int_pol"},    /* flag_ocpokbp_int_pol                              , */\
   { 0x2a90, "lag_ocpokbn_int_pol"},    /* flag_ocpokbn_int_pol                              , */\
   { 0x2aa0, "lag_adc10_ready_int_pol"},    /* flag_adc10_ready_int_pol                          , */\
   { 0x2ab0, "lag_clipa_high_int_pol"},    /* flag_clipa_high_int_pol                           , */\
   { 0x2ac0, "lag_clipa_low_int_pol"},    /* flag_clipa_low_int_pol                            , */\
   { 0x2ad0, "lag_clipb_high_int_pol"},    /* flag_clipb_high_int_pol                           , */\
   { 0x2ae0, "lag_clipb_low_int_pol"},    /* flag_clipb_low_int_pol                            , */\
   { 0x2af0, "lag_tdm_error_int_pol"},    /* flag_tdm_error_int_pol                            , */\
   { 0x2b01, "status_polarity3"},    /* Interrupt status flags polarity register          , */\
   { 0x3000, "lag_voutcomp"},    /* flag_voutcomp, indication Vset is larger than Vbat, */\
   { 0x3010, "lag_voutcomp93"},    /* flag_voutcomp93, indication Vset is larger than 1.07* Vbat, */\
   { 0x3020, "lag_voutcomp86"},    /* flag_voutcomp86, indication Vset is larger than 1.14* Vbat, */\
   { 0x3030, "lag_hiz"},    /* flag_hiz, indication Vbst is larger than  Vbat    , */\
   { 0x3040, "lag_ocpokbst"},    /* flag_ocpokbst, indication no over current in boost converter pmos switch, */\
   { 0x3050, "lag_peakcur"},    /* flag_peakcur, indication current is max in dcdc converter, */\
   { 0x3060, "lag_ocpokap"},    /* flag_ocpokap, indication no over current in amplifier "a" pmos output stage, */\
   { 0x3070, "lag_ocpokan"},    /* flag_ocpokan, indication no over current in amplifier "a" nmos output stage, */\
   { 0x3080, "lag_ocpokbp"},    /* flag_ocpokbp, indication no over current in amplifier "b" pmos output stage, */\
   { 0x3090, "lag_ocpokbn"},    /* flag_ocpokbn, indication no over current in amplifier"b" nmos output stage, */\
   { 0x30a0, "lag_adc10_ready"},    /* flag_adc10_ready, indication adc10 is ready       , */\
   { 0x30b0, "lag_clipa_high"},    /* flag_clipa_high, indication pmos amplifier "a" is clipping, */\
   { 0x30c0, "lag_clipa_low"},    /* flag_clipa_low, indication nmos amplifier "a" is clipping, */\
   { 0x30d0, "lag_clipb_high"},    /* flag_clipb_high, indication pmos amplifier "b" is clipping, */\
   { 0x30e0, "lag_clipb_low"},    /* flag_clipb_low, indication nmos amplifier "b" is clipping, */\
   { 0x310f, "mtp_man_data_out"},    /* single word read from MTP (manual copy)           , */\
   { 0x3200, "key01_locked"},    /* key01_locked, indication key 1 is locked          , */\
   { 0x3210, "key02_locked"},    /* key02_locked, indication key 2 is locked          , */\
   { 0x3225, "mtp_ecc_tcout"},    /* mtp_ecc_tcout                                     , */\
   { 0x3280, "mtpctrl_valid_test_rd"},    /* mtp test readout for read                         , */\
   { 0x3290, "mtpctrl_valid_test_wr"},    /* mtp test readout for write                        , */\
   { 0x32a0, "lag_in_alarm_state"},    /* Alarm state                                       , */\
   { 0x32b0, "mtp_ecc_err2"},    /* two or more bit errors detected in MTP, can not reconstruct value, */\
   { 0x32c0, "mtp_ecc_err1"},    /* one bit error detected in MTP, reconstructed value, */\
   { 0x32d0, "mtp_mtp_hvf"},    /* high voltage ready flag for MTP                   , */\
   { 0x32f0, "mtp_zero_check_fail"},    /* zero check failed (tbd) for MTP                   , */\
   { 0x3309, "data_adc10_tempbat"},    /* data_adc10_tempbat[9;0], adc 10 data output for testing, */\
   { 0x400f, "hid_code"},    /* 5A6Bh, 23147d to access registers (Default for engineering), */\
   { 0x4100, "ypass_hp"},    /* Bypass_High Pass Filter                           , */\
   { 0x4110, "hard_mute"},    /* Hard Mute                                         , */\
   { 0x4120, "soft_mute"},    /* Soft Mute                                         , */\
   { 0x4134, "pwm_delay"},    /* PWM DelayBits to set the delay                    , */\
   { 0x4180, "pwm_shape"},    /* PWM Shape                                         , */\
   { 0x4190, "pwm_bitlength"},    /* PWM Bitlength in noise shaper                     , */\
   { 0x4203, "drive"},    /* Drive bits to select amount of power stage amplifier, */\
   { 0x4240, "reclock_pwm"},    /*                                                   , */\
   { 0x4250, "reclock_voltsense"},    /*                                                   , */\
   { 0x4281, "dpsalevel"},    /* DPSA Threshold level                              , */\
   { 0x42a1, "dpsa_release"},    /* DPSA Release time                                 , */\
   { 0x42c0, "coincidence"},    /* Prevent simultaneously switching of output stage  , */\
   { 0x42d0, "kickback"},    /* Prevent double pulses of output stage             , */\
   { 0x4306, "drivebst"},    /* Drive bits to select the powertransistor sections boost converter, */\
   { 0x43a0, "ocptestbst"},    /* Boost OCP. For old ocp (ctrl_reversebst is 0);For new ocp (ctrl_reversebst is 1);, */\
   { 0x43d0, "test_abistfft_enbl"},    /* FFT coolflux                                      , */\
   { 0x43f0, "test_bcontrol"},    /* test _bcontrol                                    , */\
   { 0x4400, "reversebst"},    /* OverCurrent Protection selection of power stage boost converter, */\
   { 0x4410, "sensetest"},    /* Test option for the sense NMOS in booster for current mode control., */\
   { 0x4420, "enbl_engagebst"},    /* Enable power stage dcdc controller                , */\
   { 0x4470, "enbl_slopecur"},    /* Enable bit of max-current dac                     , */\
   { 0x4480, "enbl_voutcomp"},    /* Enable vout comparators                           , */\
   { 0x4490, "enbl_voutcomp93"},    /* Enable vout-93 comparators                        , */\
   { 0x44a0, "enbl_voutcomp86"},    /* Enable vout-86 comparators                        , */\
   { 0x44b0, "enbl_hizcom"},    /* Enable hiz comparator                             , */\
   { 0x44c0, "enbl_peakcur"},    /* Enable peak current                               , */\
   { 0x44d0, "ypass_ovpglitch"},    /* Bypass OVP Glitch Filter                          , */\
   { 0x44e0, "enbl_windac"},    /* Enable window dac                                 , */\
   { 0x44f0, "enbl_powerbst"},    /* Enable line of the powerstage                     , */\
   { 0x4507, "ocp_thr"},    /* ocp_thr threshold level for OCP                   , */\
   { 0x4580, "ypass_glitchfilter"},    /* Bypass glitch filter                              , */\
   { 0x4590, "ypass_ovp"},    /* Bypass OVP                                        , */\
   { 0x45a0, "ypass_uvp"},    /* Bypass UVP                                        , */\
   { 0x45b0, "ypass_otp"},    /* Bypass OTP                                        , */\
   { 0x45c0, "ypass_ocp"},    /* Bypass OCP                                        , */\
   { 0x45d0, "ypass_ocpcounter"},    /* BypassOCPCounter                                  , */\
   { 0x45e0, "ypass_lost_clk"},    /* Bypasslost_clk detector                           , */\
   { 0x45f0, "vpalarm"},    /* vpalarm (uvp ovp handling)                        , */\
   { 0x4600, "ypass_gc"},    /* bypass_gc, bypasses the CS gain correction        , */\
   { 0x4610, "cs_gain_control"},    /* gain control by means of MTP or i2c; 0 is MTP     , */\
   { 0x4627, "cs_gain"},    /* + / - 128 steps in steps of 1/4 percent  2's compliment, */\
   { 0x46a0, "ypass_lp"},    /* bypass Low-Pass filter in temperature sensor      , */\
   { 0x46b0, "ypass_pwmcounter"},    /* bypass_pwmcounter                                 , */\
   { 0x46c0, "cs_negfixed"},    /* does not switch to neg                            , */\
   { 0x46d2, "cs_neghyst"},    /* switches to neg depending on level                , */\
   { 0x4700, "switch_fb"},    /* switch_fb                                         , */\
   { 0x4713, "se_hyst"},    /* se_hyst                                           , */\
   { 0x4754, "se_level"},    /* se_level                                          , */\
   { 0x47a5, "ktemp"},    /* temperature compensation trimming                 , */\
   { 0x4800, "cs_negin"},    /* negin                                             , */\
   { 0x4810, "cs_sein"},    /* cs_sein                                           , */\
   { 0x4820, "cs_coincidence"},    /* Coincidence current sense                         , */\
   { 0x4830, "iddqtestbst"},    /* for iddq testing in powerstage of boost convertor , */\
   { 0x4840, "coincidencebst"},    /* Switch protection on to prevent simultaneously switching power stages bst and amp, */\
   { 0x4876, "delay_se_neg"},    /* delay of se and neg                               , */\
   { 0x48e1, "cs_ttrack"},    /* sample & hold track time                          , */\
   { 0x4900, "ypass_clip"},    /* Bypass clip control                               , */\
   { 0x4920, "cf_cgate_off"},    /* to disable clock gating in the coolflux           , */\
   { 0x4940, "clipfast"},    /* clock switch for battery protection clipper, it switches back to old frequency, */\
   { 0x4950, "cs_8ohm"},    /* 8 ohm mode for current sense (gain mode)          , */\
   { 0x4974, "delay_clock_sh"},    /* delay_sh, tunes S7H delay                         , */\
   { 0x49c0, "inv_clksh"},    /* Invert the sample/hold clock for current sense ADC, */\
   { 0x49d0, "inv_neg"},    /* Invert neg signal                                 , */\
   { 0x49e0, "inv_se"},    /* Invert se signal                                  , */\
   { 0x49f0, "setse"},    /* switches between Single Ende and differential mode; 1 is single ended, */\
   { 0x4a12, "adc10_sel"},    /* select the input to convert the 10b ADC           , */\
   { 0x4a60, "adc10_reset"},    /* Global asynchronous reset (active HIGH) 10 bit ADC, */\
   { 0x4a81, "adc10_test"},    /* Test mode selection signal 10 bit ADC             , */\
   { 0x4aa0, "ypass_lp_vbat"},    /* lp filter in batt sensor                          , */\
   { 0x4ae0, "dc_offset"},    /* switch offset control on/off, is decimator offset control, */\
   { 0x4af0, "tsense_hibias"},    /* bit to set the biasing in temp sensor to high     , */\
   { 0x4b00, "adc13_iset"},    /* Micadc Setting of current consumption. Debug use only, */\
   { 0x4b14, "adc13_gain"},    /* Micadc gain setting (2-compl)                     , */\
   { 0x4b61, "adc13_slowdel"},    /* Micadc Delay setting for internal clock. Debug use only, */\
   { 0x4b83, "adc13_offset"},    /* Micadc ADC offset setting                         , */\
   { 0x4bc0, "adc13_bsoinv"},    /* Micadc bit stream output invert mode for test     , */\
   { 0x4bd0, "adc13_resonator_enable"},    /* Micadc Give extra SNR with less stability. Debug use only, */\
   { 0x4be0, "testmicadc"},    /* Mux at input of MICADC for test purpose           , */\
   { 0x4c0f, "abist_offset"},    /* offset control for ABIST testing                  , */\
   { 0x4d05, "windac"},    /* for testing direct control windac                 , */\
   { 0x4dc3, "pwm_dcc_cnt"},    /* control pwm duty cycle when enbl_pwm_dcc is 1     , */\
   { 0x4e04, "slopecur"},    /* for testing direct control slopecur               , */\
   { 0x4e50, "ctrl_dem"},    /* dyn element matching control, rest of codes are optional, */\
   { 0x4ed0, "enbl_pwm_dcc"},    /* to enable direct control of pwm duty cycle        , */\
   { 0x5007, "gain"},    /* Gain setting of the gain multiplier               , */\
   { 0x5081, "sourceb"},    /* Set OUTB to                                       , */\
   { 0x50a1, "sourcea"},    /* Set OUTA to                                       , */\
   { 0x50c1, "sourcebst"},    /* Sets the source of the pwmbst output to boost converter input for testing, */\
   { 0x50e0, "tdm_enable_loopback"},    /* TDM loopback test                                 , */\
   { 0x5104, "pulselengthbst"},    /* pulse length setting test input for boost converter, */\
   { 0x5150, "ypasslatchbst"},    /* bypass_latch in boost converter                   , */\
   { 0x5160, "invertbst"},    /* invert pwmbst test signal                         , */\
   { 0x5174, "pulselength"},    /* pulse length setting test input for amplifier     , */\
   { 0x51c0, "ypasslatch"},    /* bypass_latch in PWM source selection module       , */\
   { 0x51d0, "invertb"},    /* invert pwmb test signal                           , */\
   { 0x51e0, "inverta"},    /* invert pwma test signal                           , */\
   { 0x51f0, "ypass_ctrlloop"},    /* bypass_ctrlloop bypasses the control loop of the amplifier, */\
   { 0x5210, "test_rdsona"},    /* tbd for rdson testing                             , */\
   { 0x5220, "test_rdsonb"},    /* tbd for rdson testing                             , */\
   { 0x5230, "test_rdsonbst"},    /* tbd for rdson testing                             , */\
   { 0x5240, "test_cvia"},    /* tbd for rdson testing                             , */\
   { 0x5250, "test_cvib"},    /* tbd for rdson testing                             , */\
   { 0x5260, "test_cvibst"},    /* tbd for rdson testing                             , */\
   { 0x5306, "digimuxa_sel"},    /* DigimuxA input selection control (see Digimux list for details), */\
   { 0x5376, "digimuxb_sel"},    /* DigimuxB input selection control (see Digimux list for details), */\
   { 0x5400, "hs_mode"},    /* hs_mode, high speed mode I2C bus                  , */\
   { 0x5412, "test_parametric_io"},    /* test_parametric_io for testing pads               , */\
   { 0x5440, "enbl_ringo"},    /* enbl_ringo, for test purpose to check with ringo  , */\
   { 0x5456, "digimuxc_sel"},    /* DigimuxC input selection control (see Digimux list for details), */\
   { 0x54c0, "dio_ehs"},    /* Slew control for DIO in output mode               , */\
   { 0x54d0, "gainio_ehs"},    /* Slew control for GAINIO in output mode            , */\
   { 0x550d, "enbl_amp"},    /* enbl_amp for testing to enable all analoge blocks in amplifier, */\
   { 0x5600, "use_direct_ctrls"},    /* use_direct_ctrls, to overrule several functions direct for testing, */\
   { 0x5610, "rst_datapath"},    /* rst_datapath, datapath reset                      , */\
   { 0x5620, "rst_cgu"},    /* rst_cgu, cgu reset                                , */\
   { 0x5637, "enbl_ref"},    /* for testing to enable all analoge blocks in references, */\
   { 0x56b0, "enbl_engage"},    /* Enable output stage amplifier                     , */\
   { 0x56c0, "use_direct_clk_ctrl"},    /* use_direct_clk_ctrl, to overrule several functions direct for testing, */\
   { 0x56d0, "use_direct_pll_ctrl"},    /* use_direct_pll_ctrl, to overrule several functions direct for testing, */\
   { 0x56e0, "use_direct_ctrls_2"},    /* use_direct_sourseamp_ctrls, to overrule several functions direct for testing, */\
   { 0x5707, "anamux"},    /* Anamux control                                    , */\
   { 0x57c0, "ocptest"},    /* ctrl_ocptest, deactivates the over current protection in the power stages of the amplifier. The ocp flag signals stay active., */\
   { 0x57e0, "otptest"},    /* otptest, test mode otp amplifier                  , */\
   { 0x57f0, "reverse"},    /* 1: Normal mode, slope is controlled               , */\
   { 0x5813, "pll_selr"},    /* pll_selr                                          , */\
   { 0x5854, "pll_selp"},    /* pll_selp                                          , */\
   { 0x58a5, "pll_seli"},    /* pll_seli                                          , */\
   { 0x5950, "pll_mdec_msb"},    /* most significant bits of pll_mdec[16]             , */\
   { 0x5960, "pll_ndec_msb"},    /* most significant bits of pll_ndec[9]              , */\
   { 0x5970, "pll_frm"},    /* pll_frm                                           , */\
   { 0x5980, "pll_directi"},    /* pll_directi                                       , */\
   { 0x5990, "pll_directo"},    /* pll_directo                                       , */\
   { 0x59a0, "enbl_pll"},    /* enbl_pll                                          , */\
   { 0x59f0, "pll_bypass"},    /* pll_bypass                                        , */\
   { 0x5a0f, "tsig_freq"},    /* tsig_freq, internal sinus test generator, frequency control, */\
   { 0x5b02, "tsig_freq_msb"},    /* select internal sinus test generator, frequency control msb bits, */\
   { 0x5b30, "inject_tsig"},    /* inject_tsig, control bit to switch to internal sinus test generator, */\
   { 0x5b44, "adc10_prog_sample"},    /* control ADC10                                     , */\
   { 0x5c0f, "pll_mdec"},    /* bits 15..0 of pll_mdec[16;0]                      , */\
   { 0x5d06, "pll_pdec"},    /* pll_pdec                                          , */\
   { 0x5d78, "pll_ndec"},    /* bits 8..0 of pll_ndec[9;0]                        , */\
   { 0x6007, "mtpkey1"},    /* 5Ah, 90d To access KEY1_Protected registers (Default for engineering), */\
   { 0x6185, "mtp_ecc_tcin"},    /* Mtp_ecc_tcin                                      , */\
   { 0x6203, "mtp_man_address_in"},    /* address from I2C regs for writing one word single mtp, */\
   { 0x6260, "mtp_ecc_eeb"},    /* enable code bit generation (active low!)          , */\
   { 0x6270, "mtp_ecc_ecb"},    /* enable correction signal (active low!)            , */\
   { 0x6280, "man_copy_mtp_to_iic"},    /* start copying single word from mtp to i2cregs_mtp , */\
   { 0x6290, "man_copy_iic_to_mtp"},    /* start copying single word from i2cregs_mtp to mtp [Key 1 protected], */\
   { 0x62a0, "auto_copy_mtp_to_iic"},    /* start copying all the data from mtp to i2cregs_mtp, */\
   { 0x62b0, "auto_copy_iic_to_mtp"},    /* start copying all the data from i2cregs_mtp to mtp [Key 2 protected], */\
   { 0x62d2, "mtp_speed_mode"},    /* Speed mode                                        , */\
   { 0x6340, "mtp_direct_enable"},    /* mtp_direct_enable (key1 protected)                , */\
   { 0x6350, "mtp_direct_wr"},    /* mtp_direct_wr (key1 protected)                    , */\
   { 0x6360, "mtp_direct_rd"},    /* mtp_direct_rd  (key1 protected)                   , */\
   { 0x6370, "mtp_direct_rst"},    /* mtp_direct_rst  (key1 protected)                  , */\
   { 0x6380, "mtp_direct_ers"},    /* mtp_direct_ers  (key1 protected)                  , */\
   { 0x6390, "mtp_direct_prg"},    /* mtp_direct_prg  (key1 protected)                  , */\
   { 0x63a0, "mtp_direct_epp"},    /* mtp_direct_epp  (key1 protected)                  , */\
   { 0x63b4, "mtp_direct_test"},    /* mtp_direct_test  (key1 protected)                 , */\
   { 0x640f, "mtp_man_data_in"},    /* single word to be written to MTP (manual copy)    , */\
   { 0x7000, "cf_rst_dsp"},    /* Reset CoolFlux DSP                                , */\
   { 0x7011, "cf_dmem"},    /* Target memory for access                          , */\
   { 0x7030, "cf_aif"},    /* Autoincrement-flag for memory-address             , */\
   { 0x7040, "cf_int"},    /* Interrupt CoolFlux DSP                            , */\
   { 0x7087, "cf_req"},    /* request for access (8 channels)                   , */\
   { 0x710f, "cf_madd"},    /* memory-address to be accessed                     , */\
   { 0x720f, "cf_mema"},    /* activate memory access (24- or 32-bits data is written/read to/from memory, */\
   { 0x7307, "cf_err"},    /* Coolflux error flags                              , */\
   { 0x7387, "cf_ack"},    /* acknowledge of requests (8 channels)              , */\
   { 0x8000, "calibration_onetime"},    /* Calibration schedule (key2 protected)             , */\
   { 0x8010, "calibr_ron_done"},    /* (key2 protected)                                  , */\
   { 0x8105, "calibr_vout_offset"},    /* calibr_vout_offset (DCDCoffset) 2's compliment (key1 protected), */\
   { 0x8163, "calibr_delta_gain"},    /* delta gain for vamp (alpha) 2's compliment (key1 protected), */\
   { 0x81a5, "calibr_offs_amp"},    /* offset for vamp (Ampoffset) 2's compliment (key1 protected), */\
   { 0x8207, "calibr_gain_cs"},    /* gain current sense (Imeasalpha) 2's compliment (key1 protected), */\
   { 0x8284, "calibr_temp_offset"},    /* temperature offset 2's compliment (key1 protected), */\
   { 0x82d2, "calibr_temp_gain"},    /* temperature gain 2's compliment (key1 protected)  , */\
   { 0x830f, "calibr_ron"},    /* Ron resistance of coil (key1 protected)           , */\
   { 0x8505, "type_bits_HW"},    /* Key1_Protected_MTP5                               , */\
   { 0x8601, "type_bits_1_0_SW"},    /* MTP-control SW                                    , */\
   { 0x8681, "type_bits_8_9_SW"},    /* MTP-control SW                                    , */\
   { 0x870f, "type_bits2_SW"},    /* MTP-control SW2                                   , */\
   { 0x8806, "htol_iic_addr"},    /* 7-bit I2C address to be used during HTOL testing  , */\
   { 0x8870, "htol_iic_addr_en"},    /* HTOL_I2C_Address_Enable                           , */\
   { 0x8881, "ctrl_ovp_response"},    /* OVP response control                              , */\
   { 0x88a0, "disable_ovp_alarm_state"},    /* OVP alarm state control                           , */\
   { 0x88b0, "enbl_stretch_ovp"},    /* OVP alram strech control                          , */\
   { 0x88c0, "cf_debug_mode"},    /* Coolflux debug mode                               , */\
   { 0x8a0f, "production_data1"},    /* (key1 protected)                                  , */\
   { 0x8b0f, "production_data2"},    /* (key1 protected)                                  , */\
   { 0x8c0f, "production_data3"},    /* (key1 protected)                                  , */\
   { 0x8d0f, "production_data4"},    /* (key1 protected)                                  , */\
   { 0x8e0f, "production_data5"},    /* (key1 protected)                                  , */\
   { 0x8f0f, "production_data6"},    /* (key1 protected)                                  , */\
    0xffff,"Unknown bitfield enum"    /* not found */\
};

enum tfa_irq {
    tfa_irq_vdds = 0,
    tfa_irq_plls = 1,
    tfa_irq_ds = 2,
    tfa_irq_vds = 3,
    tfa_irq_uvds = 4,
    tfa_irq_cds = 5,
    tfa_irq_clks = 6,
    tfa_irq_clips = 7,
    tfa_irq_mtpb = 8,
    tfa_irq_clk = 9,
    tfa_irq_spks = 10,
    tfa_irq_acs = 11,
    tfa_irq_sws = 12,
    tfa_irq_wds = 13,
    tfa_irq_amps = 14,
    tfa_irq_arefs = 15,
    tfa_irq_16 = 16,
    tfa_irq_17 = 17,
    tfa_irq_18 = 18,
    tfa_irq_19 = 19,
    tfa_irq_20 = 20,
    tfa_irq_21 = 21,
    tfa_irq_22 = 22,
    tfa_irq_23 = 23,
    tfa_irq_24 = 24,
    tfa_irq_25 = 25,
    tfa_irq_26 = 26,
    tfa_irq_27 = 27,
    tfa_irq_28 = 28,
    tfa_irq_29 = 29,
    tfa_irq_30 = 30,
    tfa_irq_31 = 31,
    tfa_irq_cfma_ack = 32,
    tfa_irq_cfma_err = 33,
    tfa_irq_max = 34,
    tfa_irq_all = -1 /* all irqs */};

typedef struct TfaIrqName {
    unsigned short irqEnum;
    char  *irqName;
} tfaIrqName_t;

#define TFA_IRQ_NAMETABLE static tfaIrqName_t TfaIrqNames[]= {\
    { 0, "VDDS"},\
    { 1, "PLLS"},\
    { 2, "DS"},\
    { 3, "VDS"},\
    { 4, "UVDS"},\
    { 5, "CDS"},\
    { 6, "CLKS"},\
    { 7, "CLIPS"},\
    { 8, "MTPB"},\
    { 9, "CLK"},\
    { 10, "SPKS"},\
    { 11, "ACS"},\
    { 12, "SWS"},\
    { 13, "WDS"},\
    { 14, "AMPS"},\
    { 15, "AREFS"},\
    { 16, "16"},\
    { 17, "17"},\
    { 18, "18"},\
    { 19, "19"},\
    { 20, "20"},\
    { 21, "21"},\
    { 22, "22"},\
    { 23, "23"},\
    { 24, "24"},\
    { 25, "25"},\
    { 26, "26"},\
    { 27, "27"},\
    { 28, "28"},\
    { 29, "29"},\
    { 30, "30"},\
    { 31, "31"},\
    { 32, "ACK"},\
    { 33, "33"},\
};