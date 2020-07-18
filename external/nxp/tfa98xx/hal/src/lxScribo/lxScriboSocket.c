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
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>

#if !(defined(WIN32) || defined(_X64))
#include <arpa/inet.h>
#endif

static    int listenSocket=-1;
static int activeSocket=-1;

typedef void (*sighandler_t)(int);

/*
 *
 */
void lxScriboSocketExit(int status)
{
    char buf[256];
    struct linger l;

       l.l_onoff = 1;
       l.l_linger = 0;

       printf("%s closing sockets\n", __FUNCTION__);

    // still bind error after re-open when traffic was active
    if (listenSocket>0) {
        shutdown(listenSocket, SHUT_RDWR);
        //close(listenSocket);
    }
    if (activeSocket>0) {
        setsockopt(activeSocket, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
        shutdown(activeSocket, SHUT_RDWR);
        read(activeSocket, buf, 256);
        close(activeSocket);
    }
    _exit(status);
}

/*
 * ctl-c handler
 */
static void lxScriboCtlc(int sig)
{

        (void) signal(SIGINT, SIG_DFL);

        lxScriboSocketExit(0);

}
/*
 * exit handler
 */
static void lxScriboAtexit(void)
{
        lxScriboSocketExit(0);
}

int lxScriboSocketInit(char *server)//, char *hostname)
{
       char *hostname, *portnr;
       int activeSocket = socket(AF_INET, SOCK_STREAM, 0);  /* init socket descriptor */
       struct sockaddr_in sin;
       struct hostent *host;
       int port;

       if ( server==0 ) {
           fprintf (stderr, "%s:called for recovery, exiting for now...", __FUNCTION__);
           lxScriboSocketExit(1);
       }

       portnr = strchr ( server , ':');
       if ( portnr == NULL )
       {
           fprintf (stderr, "%s: %s is not a valid servername, use host:port\n",__FUNCTION__, server);
           return -1;
       }
       hostname=server;
       *portnr++ ='\0'; //terminate
       port=atoi(portnr);

       host = gethostbyname(hostname);
       if ( !host ) {
           fprintf(stderr, "Error: wrong hostname: %s\n", hostname);
           exit(1);
       }

        if(port==0) // illegal input
            return -1;

       /*** PLACE DATA IN sockaddr_in struct ***/
       memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);
       sin.sin_family = AF_INET;
       sin.sin_port = htons(port);

       /*** CONNECT SOCKET TO THE SERVICE DESCRIBED BY sockaddr_in struct ***/
       if (connect(activeSocket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
         {
         fprintf(stderr,"error connecting to %s:%d\n", hostname , port);
         return -1;;
         }

       atexit(lxScriboAtexit);
       (void) signal(SIGINT, lxScriboCtlc);

       return activeSocket;

}

/*
 * the sockets are created first and then waits until a connection is done
 * the active socket is returned
 */
int lxScriboListenSocketInit(char *socketnr)
{
    int port;
    int  rc;
    char hostname[50];
    char clientIP [INET6_ADDRSTRLEN];

    port = atoi(socketnr);
    if(port==0) // illegal input
        return -1;

    rc = gethostname(hostname,sizeof(hostname));

    if(rc == -1){
        printf("Error gethostname\n");
        return -1;
    }

    struct sockaddr_in serverAdd;
    struct sockaddr_in clientAdd;
    socklen_t clientAddLen;

    atexit(lxScriboAtexit);
    (void) signal(SIGINT, lxScriboCtlc);

    printf("Listening to %s:%d\n", hostname, port);

    memset(&serverAdd, 0, sizeof(serverAdd));
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_port = htons(port);

    //Bind to any local server address using htonl (host to network long):
    serverAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    //Or specify address using inet_pton:
    //inet_pton(AF_INET, "127.0.0.1", &serverAdd.sin_addr.s_addr);

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket == -1){
        printf("Error creating socket\n");
        return -1;
    }

    if(bind(listenSocket, (struct sockaddr*) &serverAdd, sizeof(serverAdd)) == -1){
        printf("Bind error\n");
        return -1;
    }

    if(listen(listenSocket, 5) == -1){
        printf("Listen Error\n");
        return -1;
    }

    clientAddLen = sizeof(clientAdd);
    activeSocket = accept(listenSocket, (struct sockaddr*) &clientAdd, &clientAddLen);

    inet_ntop(AF_INET, &clientAdd.sin_addr.s_addr, clientIP, sizeof(clientAdd));
    printf("Received connection from %s\n", clientIP);

    close(listenSocket);

    return (activeSocket);

}


