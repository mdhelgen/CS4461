#ifndef __COMMON_H__
#define __COMMON_H__

/* You shouldn't need to modify this file! */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>


size_t common_getLine(char **string, FILE *stream);
FILE*  common_getStream(int sock);
int common_createSocket(char *hostname, char *port);
int common_acceptConnect(int sock);

#endif
