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



#ifndef INITTFA9890_H_
#define INITTFA9890_H_

#ifdef __cplusplus
extern "C" {
#endif                /* __cplusplus */

/*
 * Tfa9890 specific functions
 */
enum Tfa98xx_Error tfa9890_specific(Tfa98xx_handle_t handle);

/*
 * Tfa9890_DspSystemStable will compensate for the wrong behavior of CLKS
 */
enum Tfa98xx_Error tfa9890_dsp_system_stable(Tfa98xx_handle_t handle,
                        int *ready);

/*
 * Disable clock gating
 */
enum Tfa98xx_Error tfa9890_clockgating(Tfa98xx_handle_t handle, int on);
/*
 * Tfa9890_DspReset will deal with clock gating control in order
 * to reset the DSP for warm state restart
 */
enum Tfa98xx_Error tfa9890_dsp_reset(Tfa98xx_handle_t handle, int state);

#ifdef __cplusplus
}
#endif                /* */
#endif                /* INITTFA9890_H_ */
