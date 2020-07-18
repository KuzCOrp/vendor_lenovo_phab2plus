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

//
// lxScribo main entry
//
//      This is the initiator that sets up the connection to either a serial/RS232 device or a socket.
//
//      input args:
//           none:         connect to the default device, /dev/Scribo
//           string:     assume this is a special device name
//           number:    use this to connect to a socket with this number
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if !(defined(WIN32) || defined(_X64))
#include <unistd.h>
#endif
#include <stdint.h>
#include <ctype.h>
#include "dbgprint.h"
#include "NXP_I2C.h"
#include "lxScribo.h"
#include <assert.h>


int lxScribo_verbose = 0;
#define VERBOSE if (lxScribo_verbose)
static char dev[FILENAME_MAX]; /* device name */
static int lxScriboFd=-1;
static int isDirect=0;        // global that tells if scribo is used for the target

// Command headers for UART comms, let op: commando's worden omgekeerd op de seriele bus gezet
// dus 'wr' wordt 'rw' in de buffer.
const uint16_t cmdVersion   = 'v' ;  //Version
const uint16_t cmdRead      = 'r' ;  //Read I2C
const uint16_t cmdWrite     = 'w' ;  //Write I2C
//const uint16_t cmdWriteRead = 'w'<<8|'r';  // 'wr' Write and read I2C
const uint16_t cmdWriteRead = ('w' << 8 | 'r');  // 'wr' Write and read I2C
const uint16_t cmdPinSet        = ('p' << 8 | 's') ;  //pin set
const uint16_t cmdPinRead     = ('p' << 8 | 'r') ;  //pin get

const uint8_t terminator    = 0x02;  //All commands and answers are terminated with 0x02

struct cmdHeader {
    uint16_t cmd;
    uint16_t length;
};

void  lxScriboVerbose(int level) {
    lxScribo_verbose = level;
}

static void hexdump(char *str, const unsigned char * data, int num_write_bytes) //TODO cleanup/consolidate all hexdumps
{
    int i;

    PRINT("%s", str);
    for(i=0;i<num_write_bytes;i++)
    {
        PRINT("0x%02x ", data[i]);
    }
    PRINT("\n");
    fflush(stdout);
}
static void dump_buffer(int length, char *buf)
{
    uint8_t *ptr = (uint8_t*)buf;
    while (length--)
        if (isprint(*ptr))
            putchar(*ptr++);
        else {
            PRINT("<0x%02x>", *ptr++);
            fflush(stdout);
        }
    PRINT("\n");
    fflush(stdout);
}

static int lxScriboGetResponseHeader(int fd, const uint16_t cmd, int* prlength)
{
    uint8_t response[6];
    uint16_t rcmd, rstatus; //, rlength;
    int length;
#ifdef CYGWIN
    uint8_t *pRcv;
    int actual;

        pRcv=response;
    do {
        actual = read(fd, pRcv, sizeof(response)-length); //
        pRcv += actual;
        length += actual;
        if(actual<0)
                break;
    } while (length<sizeof(response) );

#elif !(defined(WIN32) || defined(_X64))
    length = read(fd, response, sizeof(response)); //
#endif //CYGWIN
    VERBOSE hexdump("rsp:", response, sizeof(response));

    fd = 0; /* Remove unreferenced formal parameter warning */

    // response (lsb/msb)= echo cmd[0] cmd[1] , status [0] [1], length [0] [1] ...data[...]....terminator[0]
    if ( length==sizeof(response) )
    {
        rcmd    = response[0] | response[1]<<8;
        rstatus = response[2] | response[3]<<8;
        *prlength = response[4] | response[5]<<8;  /* extra bytes that need to be read */

        /* must get response to expected cmd */
        if  ( cmd != rcmd) {
            ERRORMSG("scribo protocol error: expected %d bytes , got %d bytes\n", cmd, rcmd);
        }
        if  (rstatus != 0) {
            ERRORMSG("scribo status error: 0x%02x\n", rstatus);
            //exit(1);
        }

        return (int)rstatus; /* iso length */
    }
    else {
        ERRORMSG("bad response length=%d\n", length);
        return -1; //exit(1);
    }

    return -1;
}
/*
 * set pin
 */
int lxScriboSetPin(int fd, int pin, int value)
{
#if (defined(WIN32) || defined(_X64))
    fd,    pin, value = 0; /* Remove unreferenced formal parameter warning */
        PRINT("Only works for linux! \n");
    return 0;
#else
    uint8_t cmd[6];
    int stat;
    int ret;
    uint8_t term;
    int rlength;

    VERBOSE PRINT("SetPin[%d]<%d\n", pin, value);

    cmd[0] = cmdPinSet;
    cmd[1] = cmdPinSet>>8;
    cmd[2] = (uint8_t)pin;
    cmd[3] = (uint8_t)value;
    cmd[4] = (uint8_t)(value>>8);
    cmd[5]= terminator;

    // write header
    VERBOSE PRINT("cmd: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
            cmd[0], cmd[1],cmd[2],cmd[3],cmd[4], cmd[5]);
    ret = write(fd, cmd, sizeof(cmd));
    assert(ret == sizeof(cmd));

    stat = lxScriboGetResponseHeader( fd, cmdPinSet, &rlength);
    assert(stat == 0);
    assert(rlength == 0); // expect no return data
    ret = read(fd, &term, 1);
    assert(ret == 1);

    VERBOSE PRINT("term: 0x%02x\n", term);
//    usleep(1000000);
    assert(term == terminator);

    //PRINT("SetPin done\n");
    return stat>=0;
#endif
}
/*
 * get pin state
 */
int lxScriboGetPin(int fd, int pin)
{
#if (defined(WIN32) || defined(_X64))
    fd, pin = 0; /* Remove unreferenced formal parameter warning */
    return 0;
#else
    uint8_t cmd[4];
    int value, length;
    int ret;
    int rlength;

    cmd[0] = cmdPinRead;
    cmd[1] = cmdPinRead>>8;
    cmd[2] = pin;
    cmd[3] = terminator;

    // write header
    VERBOSE PRINT("cmd:0x%02x 0x%02x 0x%02x 0x%02x\n",
            cmd[0], cmd[1],cmd[2],cmd[3]);
    ret = write(fd, cmd, sizeof(cmd));
    assert(ret == sizeof(cmd));

    ret = lxScriboGetResponseHeader( fd, cmdPinRead, &rlength);
    assert(ret == 0);

    length = read(fd, &value, 1);
    assert(length == 1);

    VERBOSE PRINT("GetPin[%d]:%d\n", pin, value);

    return value;
#endif
}

/*
 * retrieve the version string from the device
 */
int lxScriboVersion(char *buffer, int fd)
{
#if (defined(WIN32) || defined(_X64))
    fd, buffer = 0; /* Remove unreferenced formal parameter warning */
    return 0;
#else
    char cmd[3];
    int length;
    int ret;
    int rlength;

    cmd[0] = cmdVersion;
    cmd[1] = cmdVersion>>8;
    cmd[2] = terminator;

    ret = write(fd, cmd, sizeof(cmd));
    assert(ret == sizeof(cmd));

    ret = lxScriboGetResponseHeader( fd, cmdVersion, &rlength);
    assert(ret == 0);
    assert(rlength > 0);

    length = read(fd, buffer, 256);
    /* no new line is added */

    return length;
#endif
}
#if !(defined(WIN32) || defined(_X64))
int lxScriboWrite(int fd, int size, uint8_t *buffer, uint32_t *pError)
{
    uint8_t cmd[5], slave,term;
    int status, total=0;
    int rlength;

    *pError = NXP_I2C_Ok;
    // slave is the 1st byte in wbuffer
    slave = buffer[0] >> 1;

    size -= 1;
    buffer++;

    cmd[0] = cmdWrite;// lsb
    cmd[1] = cmdWrite>>8;// msb
    cmd[2] = slave; // 1st byte is the i2c slave address
    cmd[3] = size & 0xff; // lsb
    cmd[4] = (size >> 8) & 0xff; // msb

    // write header
    VERBOSE PRINT("cmd: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
            cmd[0], cmd[1],cmd[2],cmd[3],cmd[4]);
    status = write(fd, cmd, sizeof(cmd));
    if(status>0)
        total+=status;
    else
    {
        *pError = NXP_I2C_NoAck;
        return status;
    }
    // write payload
    VERBOSE hexdump("\t\twdata:", buffer, size);
    status = write(fd, buffer, size);
    if(status>0)
        total+=status;
    else
    {
        *pError = NXP_I2C_NoAck;
        return status;
    }
    // write terminator
    cmd[0] = terminator;
    VERBOSE PRINT("term: 0x%02x\n", cmd[0]);
    status = write(fd, cmd, 1);
    if(status>0)
        total+=status;
    else
    {
        *pError = NXP_I2C_NoAck;
        return status;
    }

    status = lxScriboGetResponseHeader(fd, cmdWrite, &rlength);
    if (status != 0)
    {
        *pError = status;
    }
    assert(rlength == 0); // expect no return data
    status = read(fd, &term, 1);
    if (status < 0)
    {
        if (*pError == NXP_I2C_Ok) *pError = NXP_I2C_NoAck;
        return status;
    }
    assert(term == terminator);
    VERBOSE PRINT("term: 0x%02x\n", term);
    return total;
}

int lxScriboWriteRead(int fd, int wsize, const uint8_t *wbuffer, int rsize,
        uint8_t *rbuffer, uint32_t *pError) {
    uint8_t cmd[5], rcnt[2], slave, *rptr, term;
    int length = 0;
    int status, total = 0;
    int rlength;

    *pError = NXP_I2C_Ok;
    // slave is the 1st byte in wbuffer
    slave = wbuffer[0] >> 1;

    wsize -= 1;
    wbuffer++;

    if ((slave<<1) + 1 == rbuffer[0]) // write & read to same target
            {
        //Format = 'wr'(16) + sla(8) + w_cnt(16) + data(8 * w_cnt) + r_cnt(16) + 0x02
        cmd[0] = cmdWriteRead & 0xFF ;
        cmd[1] = cmdWriteRead >> 8;
        cmd[2] = slave;
        cmd[3] = wsize & 0xff; // lsb
        cmd[4] = (wsize >> 8) & 0xff; // msb

        // write header
        VERBOSE hexdump("cmd:", cmd, sizeof(cmd));
        status = write(fd, cmd, sizeof(cmd));
        if (status > 0)
            total += status;
        else
        {
            *pError = NXP_I2C_NoAck;
            return status;
        }
        // write payload
        VERBOSE hexdump("\t\twdata:", wbuffer, wsize);
        status = write(fd, wbuffer, wsize);
        if (status > 0)
            total += status;
        else
        {
            *pError = NXP_I2C_NoAck;
            return status;
        }

        // write readcount
        rsize -= 1;
        rcnt[0] = rsize & 0xff; // lsb
        rcnt[1] = (rsize >> 8) & 0xff; // msb
        VERBOSE hexdump("rdcount:",rcnt, 2);
        status = write(fd, rcnt, 2);
        if (status > 0)
            total += status;
        else
        {
            *pError = NXP_I2C_NoAck;
            return status;
        }

        // write terminator
        cmd[0] = terminator;
        VERBOSE PRINT("term: 0x%02x\n", cmd[0]);
        status = write(fd, cmd, 1);
        if (status > 0)
            total += status;
        else
        {
            *pError = NXP_I2C_NoAck;
            return status;
        }

        //if( rcnt[1] | rcnt[0] >100)    // TODO check timing
        if (rsize > 100) // ?????????????????????????????????????
            usleep(20000);
        // slave is the 1st byte in rbuffer, remove here
        //rsize -= 1;
        rptr = rbuffer+1;
        // read back, blocking
        status = lxScriboGetResponseHeader(fd, cmdWriteRead, &rlength);
        if (status != 0)
        {
            *pError = status;
        }
        //assert(rlength == rsize);
        if  ( rlength != rsize) {
            ERRORMSG("scribo protocol error: expected %d bytes , got %d bytes\n", rsize, rlength);
        }
        /*** FOR CYGWING - remove or fix this!
        //    VERBOSE PRINT("Reading %d bytes\n", rsize);
        #ifdef CYGWIN
            length=0;
            do {
                int actual;
                actual = read(fd, rptr, rsize-length);  //
                rptr += actual;
                length += actual;
                if(actual<0)
                        break;
            } while (length<rsize );
        #else
                length = read(fd, rptr, rsize); //
        #endif //CYGWIN
        ****/


        //    VERBOSE PRINT("Reading %d bytes\n", rsize);
        length = read(fd, rptr, rsize); //
        if (length < 0)
        {
            if (*pError == NXP_I2C_Ok) *pError = NXP_I2C_NoAck;
            return -1;
        }
        VERBOSE hexdump("\trdata:",rptr, rsize);
        // else something wrong, still read the terminator
        //    if(status>0) TODO handle error
        status = read(fd, &term, 1);
        if (length < 0)
        {
            if (*pError == NXP_I2C_Ok) *pError = NXP_I2C_NoAck;
            return -1;
        }
        assert(term == terminator);
        VERBOSE PRINT("rterm: 0x%02x\n", term);
    }
    else {
        PRINT("!!!! write slave != read slave !!! %s:%d\n", __FILE__, __LINE__);
        *pError = NXP_I2C_UnsupportedValue;
        status = -1;
        return status;
    }
    return length>0 ? (length + 1) : 0; // we need 1 more for the length because of the slave address

}
#endif // windows

/**
 * register the low level I2C HAL interface
 *  @param target device name; if NULL the default will be registered
 *  @return file descriptor of target device if successful
 */
int lxScriboRegister(char *target)
{
    if (target ){
        strcpy(dev, target);
    } else {
        strcpy(dev, TFA_I2CDEVICE);
    }

    if ( lxScribo_verbose ) {
            PRINT("%s:target device=%s\n", __FUNCTION__, dev);
    }

    /* tell the HAL which interface functions : */

    /////////////// dummy //////////////////////////////
    if ( strncmp (dev, "dummy",  5 ) == 0 ) {// if dummy act dummy
        lxScriboFd = NXP_I2C_Interface(dev, lxDummyInit, lxDummyWrite, lxDummyWriteRead, lxDummyVersion);
        isDirect=1; // don't use unix filedescriptor for read/write
    }
#if !( defined(WIN32) || defined(_X64) ) // posix/linux
#ifdef    HAL_HID
    /////////////// hid //////////////////////////////
    else if ( strncmp( dev , "hid", 3 ) == 0)    { // hid
        lxScriboFd = NXP_I2C_Interface(dev,lxHidInit,lxHidWrite,lxHidWriteRead, lxHidVersion);
    }
#endif
    /////////////// network //////////////////////////////
    else if ( strchr( dev , ':' ) != 0)    { // if : in name > it's a socket
        lxScriboFd = NXP_I2C_Interface(dev, lxScriboSocketInit, lxScriboWrite, lxScriboWriteRead, lxScriboVersion);
    }
    /////////////// i2c //////////////////////////////
    else if ( strncmp (dev, "/dev/sma",  8 ) == 0 ) { // if /dev/i2c... direct i2c device
        lxScriboFd = NXP_I2C_Interface(dev, lxI2cInit, lxI2cWrite, lxI2cWriteRead, lxI2cVersion);
        isDirect=1;    // don't use unix filedescriptorfor read/write
        VERBOSE PRINT("%s: i2c\n", __FUNCTION__);
    }
    /////////////// serial/USB //////////////////////////////
    else if ( strncmp (dev, "/dev/tty",  8 ) == 0 ) { // if /dev/ it must be a serial device
        lxScriboFd = NXP_I2C_Interface(dev, lxScriboSerialInit, lxScriboWrite, lxScriboWriteRead, lxScriboVersion);
    }

#else /////////////// Scribo server //////////////////////////////
    else if ( strncmp (dev, "scribo",  5 ) == 0 ) {// Scribo server dll interface
        lxScriboFd = NXP_I2C_Interface(dev, lxWindowsInit, lxWindowsWrite, lxWindowsWriteRead, lxWindowsVersion);
    }
#endif
    else {
        ERRORMSG("%s: devicename %s is not a valid target\n", __FUNCTION__, dev); // anything else is a file
        _exit(1);
    }

    return lxScriboFd;
}

int lxScriboGetFd(void)
{
    if ( lxScriboFd < 0 )
        ERRORMSG("Warning: the target is not open\n");

    return lxScriboFd;
}

char * lxScriboGetName(void)
{
    return dev;
}

int lxScriboPrintTargetRev(int fd)
{
     int  length, stat;
     char  buf[256];

     if (isDirect) { // no scribo involved
             PRINT("(no scribo used)\n");
             return -1;
     }

    memset(buf, 0, sizeof(buf));

    stat = length = lxScriboVersion(buf, fd);

    if (length) {
        dump_buffer(length, buf);
    }
    return stat;
}
/*
 * return version string if applicable
 */
int lxScriboGetRev(int fd, char *str)
{
    int length;

    if (isDirect) // no scribo involved
        length = sprintf(str, "(no scribo used)\n");
    else
        length = lxScriboVersion(str, fd);

    return length;
}


