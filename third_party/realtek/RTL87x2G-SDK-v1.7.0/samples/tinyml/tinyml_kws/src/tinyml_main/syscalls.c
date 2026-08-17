/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * syscalls.c - Minimal newlib syscall stubs for bare-metal / RTOS build.
 *
 * The nano C library (libc_nano.a) pulls in POSIX syscalls (_read/_write/
 * _lseek/_kill/...) whenever stdio, assert(), abort() or the *printf family
 * are referenced. Without a backing OS these are satisfied by libnosys.a,
 * whose weak stubs emit link-time warnings such as:
 *     warning: _write is not implemented and will always fail
 *
 * Providing strong definitions here overrides the libnosys stubs and removes
 * those warnings. Behaviour is unchanged (the libnosys versions also just
 * failed): there is no filesystem or process model on this target.
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#undef errno
extern int errno;

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    /* Report a character device so newlib treats streams as unbuffered. */
    if (st != NULL)
    {
        st->st_mode = S_IFCHR;
    }
    return 0;
}

int _isatty(int fd)
{
    /* stdin/stdout/stderr are treated as terminals. */
    return (fd >= 0 && fd <= 2) ? 1 : 0;
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = ENOSYS;
    return -1;
}

off_t _lseek(int fd, off_t ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int fd, char *ptr, int len)
{
    (void)fd;
    (void)ptr;
    (void)len;
    /* No input source: report end-of-file. */
    return 0;
}

int _write(int fd, const char *ptr, int len)
{
    (void)fd;
    (void)ptr;
    /* No output backend: discard the bytes but report them as written so
     * callers such as the *printf family do not treat this as an error.
     * Route to a UART here if console output is desired. */
    return len;
}
