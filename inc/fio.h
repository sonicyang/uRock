#ifndef __FIO_H__
#define __FIO_H__

#include <stdio.h>
#include <sys/types.h>

enum open_types_t {
    O_RDONLY = 0,
    O_WRONLY = 1,                                                               
    O_RDWR = 2,
    O_CREAT = 4,
    O_TRUNC = 8,
    O_APPEND = 16,
};

#define MAX_FDS 32

typedef ssize_t (*fdread_t)(void * opaque, void * buf, size_t count);
typedef ssize_t (*fdwrite_t)(void * opaque, const void * buf, size_t count);
typedef off_t (*fdseek_t)(void * opaque, off_t offset, int whence);
typedef int (*fdclose_t)(void * opaque);

struct fddef_t {
    fdread_t fdread;
    fdwrite_t fdwrite;
    fdseek_t fdseek;
    fdclose_t fdclose;
    void * opaque;
};


/* Need to be called before using any other fio functions */
__attribute__((constructor)) void fio_init();

int fio_is_open(int fd);
int fio_open(fdread_t, fdwrite_t, fdseek_t, fdclose_t, void * opaque);
ssize_t fio_read(int fd, void * buf, size_t count);
ssize_t fio_write(int fd, const void * buf, size_t count);
off_t fio_seek(int fd, off_t offset, int whence);
int fio_close(int fd);
void fio_set_opaque(int fd, void * opaque);

void register_devfs();

#endif
