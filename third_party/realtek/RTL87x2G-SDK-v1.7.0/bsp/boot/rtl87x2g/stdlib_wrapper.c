/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include <app_section.h>

#if defined ( __ARMCC_VERSION   )
/* Standard IO device handles. */
#define STDIN       0
#define STDOUT      1
#define STDERR      2

typedef int FILEHANDLE;
typedef int FILE;

/*
 * These names are used during library initialization as the
 * file names opened for stdin, stdout, and stderr.
 * As we define _sys_open() to always return the same file handle,
 * these can be left as their default values.
 */
const char __stdin_name[] =  ":tt";
const char __stdout_name[] =  ":tt";
const char __stderr_name[] =  ":tt";

FILEHANDLE _sys_open(const char *name, int openmode)
{
    return 1; /* everything goes to the same output */
}
int _sys_close(FILEHANDLE fh)
{
    return 0;
}
int _sys_write(FILEHANDLE fh, const unsigned char *buf,
               unsigned len, int mode)
{
//  your_device_write(buf, len);
    return 0;
}
int _sys_read(FILEHANDLE fh, unsigned char *buf,
              unsigned len, int mode)
{
    return -1; /* not supported */
}
void _ttywrch(int ch)
{
    char c = ch;
//  your_device_write(&c, 1);
}
int _sys_istty(FILEHANDLE fh)
{
    return 0; /* buffered output */
}
int _sys_seek(FILEHANDLE fh, long pos)
{
    return -1; /* not supported */
}
long _sys_flen(FILEHANDLE fh)
{
    return -1; /* not supported */
}

void _sys_exit(int return_code)
{
    /* TODO: perhaps exit the thread which is invoking this function */
    while (1);
}

/**
 * used by tmpnam() or tmpfile()
 */
int _sys_tmpnam(char *name, int fileno, unsigned maxlength)
{
    return -1;
}

char *_sys_command_string(char *cmd, int len)
{
    /* no support */
    return NULL;
}

int fputc(int ch, FILE *f)
{
    return ch;
}

#elif defined ( __GNUC__   )
extern void *__aeabi_memset(void *s, size_t n, int c);
APP_RAM_TEXT_SECTION
void *memset(void *s, int c, size_t n)
{
    __aeabi_memset(s, n, c);
    return s;
}

#include <sys/stat.h>
#include <errno.h>
#undef errno
extern int errno;

int __attribute__((weak)) _close(int file)
{
    return -1;
}

int __attribute__((weak)) _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int __attribute__((weak)) _getpid(void)
{
    return 1;
}

int __attribute__((weak)) _isatty(int file)
{
    return 1;
}

int __attribute__((weak)) _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int __attribute__((weak)) _lseek(int file, int ptr, int dir)
{
    return 0;
}

int __attribute__((weak)) _read(int file, char *ptr, int len)
{
    return 0;
}

int __attribute__((weak)) _write(int file, char *ptr, int len)
{
    return len;
}
#endif



