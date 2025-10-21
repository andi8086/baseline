#ifndef DEV_H
#define DEV_H


#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
        int (*put)(void *ctx, char c);
        char (*get)(void *ctx);
        void *ctx;
} chardev_t;


typedef struct {
        uint16_t blocksize;
        int (*put)(void *ctx, void far *buff);
        int (*get)(void *ctx, void far *buff);
        void *ctx;
} blockdev_t;


typedef struct {
        chardev_t *stdin;
        chardev_t *stdout;
        chardev_t *stderr;
} console_t;
#pragma pack(pop)


extern console_t defconsole;

int dev_init(void);
void ser_putc(char c);
void ser_init(void);

#endif
