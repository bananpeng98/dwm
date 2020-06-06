#ifndef __CMD_H_
#define __CMD_H_

typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
    char s[16];
} Arg;

typedef struct {
    const char* name;
    void (*func)(const Arg *);
    const Arg darg;
} Cmd;

#endif // __CMD_H_
