/**
 * @file    syscalls.c
 * @brief   newlib syscall stubs (CubeMX template). _write/_read loop over
 *          the weak __io_putchar/__io_getchar; the strong versions in
 *          board.c bind the console to the board UART.
 *          Copyright (c) 2020-2024 STMicroelectronics (template source).
 */

/* Includes */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

/* Variables */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

char *__env[1] = {0};
char **environ = __env;

/* Functions */

/**
 * @brief  Initialize the semihosting monitor handles.
 *
 * No-op on this target: there is no debugger host to talk to.
 */
void initialise_monitor_handles()
{
}

/**
 * @brief  Get the process ID.
 * @return Always 1: there is a single "process".
 */
int _getpid(void)
{
    return 1;
}

/**
 * @brief  Send a signal to a process.
 * @param  pid Process ID (unused).
 * @param  sig Signal number (unused).
 * @return Always -1 with errno set to EINVAL.
 */
int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

/**
 * @brief  Terminate the process.
 * @param  status Exit status.
 *
 * Hangs forever: there is nowhere to exit to on a bare-metal target.
 */
void _exit(int status)
{
    _kill(status, -1);
    while (1)
    {
    } /* Make sure we hang here */
}

/**
 * @brief  Read from a file descriptor.
 * @param  file File descriptor (unused).
 * @param  ptr  Buffer to fill.
 * @param  len  Number of bytes to read.
 * @return Number of bytes read.
 *
 * Everything comes from __io_getchar(); override that weak symbol to
 * wire the console to a real UART.
 */
__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void)file;
    int idx;

    for (idx = 0; idx < len; idx++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

/**
 * @brief  Write to a file descriptor.
 * @param  file File descriptor (unused).
 * @param  ptr  Data to write.
 * @param  len  Number of bytes to write.
 * @return Number of bytes written.
 *
 * Everything goes to __io_putchar(); override that weak symbol to wire
 * the console to a real UART.
 */
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;
    int idx;

    for (idx = 0; idx < len; idx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}

/**
 * @brief  Close a file descriptor.
 * @param  file File descriptor (unused).
 * @return Always -1.
 */
int _close(int file)
{
    (void)file;
    return -1;
}

/**
 * @brief  Get file status.
 * @param  file File descriptor (unused).
 * @param  st   Out: status; always reports a character device.
 * @return Always 0.
 */
int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief  Is the file descriptor a terminal?
 * @param  file File descriptor (unused).
 * @return Always 1.
 */
int _isatty(int file)
{
    (void)file;
    return 1;
}

/**
 * @brief  Reposition the file offset.
 * @param  file File descriptor (unused).
 * @param  ptr  Offset (unused).
 * @param  dir  Whence (unused).
 * @return Always 0.
 */
int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

/**
 * @brief  Open a file.
 * @param  path  Path name (unused).
 * @param  flags Open flags (unused).
 * @return Always -1: there is no filesystem.
 */
int _open(char *path, int flags, ...)
{
    (void)path;
    (void)flags;
    /* Pretend like we always fail */
    return -1;
}

/**
 * @brief  Wait for a child process.
 * @param  status Out: exit status (unused).
 * @return Always -1 with errno set to ECHILD.
 */
int _wait(int *status)
{
    (void)status;
    errno = ECHILD;
    return -1;
}

/**
 * @brief  Remove a file.
 * @param  name Path name (unused).
 * @return Always -1 with errno set to ENOENT.
 */
int _unlink(char *name)
{
    (void)name;
    errno = ENOENT;
    return -1;
}

/**
 * @brief  Get process times.
 * @param  buf Out: times (unused).
 * @return Always -1.
 */
int _times(struct tms *buf)
{
    (void)buf;
    return -1;
}

/**
 * @brief  Get file status by name.
 * @param  file Path name (unused).
 * @param  st   Out: status; always reports a character device.
 * @return Always 0.
 */
int _stat(char *file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief  Create a hard link.
 * @param  old Existing path (unused).
 * @param  new New path (unused).
 * @return Always -1 with errno set to EMLINK.
 */
int _link(char *old, char *new)
{
    (void)old;
    (void)new;
    errno = EMLINK;
    return -1;
}

/**
 * @brief  Create a child process.
 * @return Always -1 with errno set to EAGAIN.
 */
int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

/**
 * @brief  Execute a program.
 * @param  name Path name (unused).
 * @param  argv Arguments (unused).
 * @param  env  Environment (unused).
 * @return Always -1 with errno set to ENOMEM.
 */
int _execve(char *name, char **argv, char **env)
{
    (void)name;
    (void)argv;
    (void)env;
    errno = ENOMEM;
    return -1;
}
