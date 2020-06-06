#include "socket.h"

static struct sockaddr_un socketaddr;
static char socketbuf[100] = {0};
static int socketfd, socketcl, socketrc;
static Cmd *cmds;
static unsigned int cmdslen;

void
socketinit(const char *socketpath, Cmd *commands, unsigned int commandslen)
{
    cmds = commands;
    cmdslen = commandslen;

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    /* int prio = 7; */
    /* setsockopt(socketfd, SOL_SOCKET, SO_PRIORITY, &prio, 1); */

    memset(&socketaddr, 0, sizeof(socketaddr));
    socketaddr.sun_family = AF_UNIX;
    if (*socketpath == '\0') {
        *socketaddr.sun_path = '\0';
        exit(-1);
    } else {
        snprintf(socketaddr.sun_path, sizeof(socketaddr.sun_path), "%s", socketpath);
        /* unlink(socketpath); */
    }

    fcntl(socketfd, F_SETFL, O_NONBLOCK);
}

void
socketclose(void)
{
    close(socketfd);
}

void
socketconnect(void)
{
    if (connect(socketfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
        perror("connect error");
        exit(-1);
    }
}

void
socketlisten(void)
{
    if (bind(socketfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
        perror("bind error");
        exit(-1);
    }

    if (listen(socketfd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }
}

static void
socketvwritesocket(int socket, const char *fmt, va_list ap)
{
    int len = vsnprintf(socketbuf, sizeof(socketbuf)-1, fmt, ap);

    if (write(socket, socketbuf, len) != len) {
        if (len > 0) fprintf(stderr, "partial write");
        else {
            perror("write error");
            exit(-1);
        }
    }
}

static void
socketwritesocket(int socket, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	socketvwritesocket(socket, fmt, ap);
	va_end(ap);
}

void
socketwrite(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    socketvwritesocket(socketfd, fmt, ap);
	va_end(ap);
}

static void
socketreadsocket(int socket)
{
    unsigned int i;
    char cmd[64];
    char type[4];
    char param[16];
    int cmd_len = 0, param_len = 0, type_len = 0;
    Arg arg = {0};
    char noarg = 0, fail = 1;

    if ((socketrc = read(socket, socketbuf, sizeof(socketbuf)-1)) > 0) {
        socketbuf[socketrc] = '\0';
        sscanf(socketbuf, "%s%n%s%n%s%n", cmd, &cmd_len, type, &type_len, param, &param_len);

        if (param_len == 0) noarg = 1;
        else if (strncmp(type, "i", 1) == 0)
            sscanf(param, "%i", &(arg.i));
        else if (strncmp(type, "ui", 2) == 0) {
            sscanf(param, "%u", &(arg.ui));
            arg.ui = 1 << (arg.ui-1);
        }
        else if (strncmp(type, "f", 1) == 0)
            sscanf(param, "%f", &(arg.f));
        else if (strncmp(type, "s", 1) == 0)
            sscanf(param, "%s", arg.s);
        else
            noarg = 1;

        printf("%s\n", socketbuf);

        for (i = 0; i < cmdslen; i++) {
            if (strncmp(cmds[i].name, cmd, cmd_len-1) == 0 && cmds[i].func) {
                if (noarg) arg = cmds[i].darg;
                cmds[i].func(&arg);
                fail = 0;
                break;
            }
        }
        socketwritesocket(socket, "status i %i\n", !fail);
    }
}

void
socketaccept(void)
{
    if ((socketcl = accept(socketfd, NULL, NULL)) != -1) {
        fcntl(socketcl, F_SETFL, O_NONBLOCK);

        socketreadsocket(socketcl);

        if (socketrc == -1) {
            perror("read");
        } else if (socketrc == 0) {
            close(socketcl);
        }
    }
}

void
socketread(void)
{
    socketreadsocket(socketfd);
}
