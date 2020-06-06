#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "cmd.h"

void socketinit(const char* socketpath, Cmd *commands, unsigned int commandslen);
void socketaccept(void);
void socketread(void);
void socketconnect(void);
void socketlisten(void);
void socketwrite(const char *fmt, ...);
void socketclose(void);

#endif // __SOCKET_H_
