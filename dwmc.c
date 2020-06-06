#include "dwmc.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include "socket.h"

int running = 1;

void
status(const Arg *arg)
{
    running = 0;
}

void
echo(const Arg *arg)
{
    if (arg && *(arg->s))
        printf("%s\n", arg->s);
}

Cmd cmds[] = {
    { "status", status, {0} },
    { "echo",   echo, {0} },
};

int main(int argc, char *argv[]) {
    socketinit(socketpath, cmds, 1);
    socketconnect();

    if (argc == 2) socketwrite("%s\n", argv[1]);
    else if (argc == 4) socketwrite("%s %s %s\n", argv[1], argv[2], argv[3]);

    while (running) {
        socketread();
    }

    printf("end\n");

    socketclose();

    return 0;
}
