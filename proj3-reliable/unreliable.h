#ifndef __UNRELIABLE_H__
#define __UNRELIABLE_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


ssize_t unreliable_sendto(int sockfd, const void *buf, size_t len,
			  int flags, const struct sockaddr *dest_addr,
			  socklen_t addrlen);


#endif
