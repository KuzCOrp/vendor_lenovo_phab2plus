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
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#if !(defined(WIN32) || defined(_X64))
#include <unistd.h>
#endif

#include "lxScribo.h"

#ifndef B460800
#define B460800 460800
#endif
//#define BAUD B500000 // 16M xtal
#define BAUD B460800 // 14.7M xtal    //TODO autodetect baud

extern int lxScribo_verbose;
static   char filename[FILENAME_MAX];

/*
 * init or recover the connection
 *   if dev==NULL recover else just open
 */
int lxScriboSerialInit(char *dev)
{
    struct termios tio;
    static int tty_fd=-1;
//    char tmp[256];

    if ( dev )
        strcpy(filename, dev);

    if ( lxScribo_verbose )
        printf("Opening serial Scribo connection on %s\n", filename);

    if (tty_fd != -1 )
        if (close(tty_fd) )
            _exit(1);

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL|CSTOPB;           // 8n2, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;

//    /* Flush old read data */
//    tty_fd=open(filename, O_RDWR | O_NONBLOCK);
//    if (read(tty_fd, tmp, sizeof(tmp)))
//        if ( lxScribo_verbose )
//            printf ("serial read had stale data\n");
//    close(tty_fd);

    tty_fd=open(filename, O_RDWR);
    cfsetospeed(&tio,BAUD);         // 115200 baud
    cfsetispeed(&tio,BAUD);         // 115200 baud

    tcsetattr(tty_fd,TCSANOW,&tio);


    //    lxScriboSetPin(tty_fd, 4, 0x8000); // Weak pull-up on PA4. Is power-up UDA1355.

    return tty_fd;
}
