#include "dwmc.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>


int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  char buf[100];
  int fd,rc;

  if (argc > 1) snprintf(buf, sizeof(buf), "%s", argv[1]);
  else exit(-1);

  rc = strlen(buf);

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  if (*socket_path == '\0') {
    *addr.sun_path = '\0';
    exit(-1);
  } else {
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path);
  }

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect error");
    exit(-1);
  }

  printf("%s", buf);
  if (send(fd, buf, rc, 0) != rc) {
    if (rc > 0) fprintf(stderr,"partial write");
    else {
      perror("write error");
      exit(-1);
    }
  }

  struct pollfd pfd[] = {
    { fd, POLLIN, 0 }
  };

  while (poll(pfd, 1, -1) > 0) {
    if (pfd[0].revents & POLLIN) {
      if ((rc = recv(fd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[rc] = '\0';
        printf("%s\n", buf);
        break;
      } else {
        break;
      }
    }
  }

  close(fd);

  return 0;
}
