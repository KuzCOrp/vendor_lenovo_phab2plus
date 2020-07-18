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



#ifndef CLIMAX_H_
#define CLIMAX_H_

#include "nxpTfa98xx.h"

char *cliInit(int argc, char **argv);
int cliCommands(int targetfd, char *xarg, Tfa98xx_handle_t *handlesIn);

int cliTargetDevice(char *devname);
#ifndef WIN32
void cliSocketServer(char *socket);
void cliClientServer(char *socket);
#endif
//int cliSaveParamsFile( char *filename );
//int cliLoadParamsFile( char *filename );


/*
 *  globals for output control
 */
extern int cli_verbose;    /* verbose flag */
extern int cli_trace;    /* message trace flag from bb_ctrl */
extern int cli_quiet;    /* don't print too much */

#define TRACEIN(F)  if(cli_trace) printf("Enter %s\n", F);
#define TRACEOUT(F) if(cli_trace) printf("Leave %s\n", F);

#endif /* CLIMAX_H_ */
