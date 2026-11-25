/* Platform-neutral filesystem contract. POSIX/Android implementation lives in host_fs_posix.c.
 * Paths are host paths; Xbox guest-path translation remains a separate layer. */
#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    REDLINE_FS_READ   = 1u << 0,
    REDLINE_FS_WRITE  = 1u << 1,
    REDLINE_FS_CREATE = 1u << 2,
    REDLINE_FS_EXCL   = 1u << 3,
    REDLINE_FS_TRUNC  = 1u << 4,
    REDLINE_FS_APPEND = 1u << 5
};

typedef intptr_t RedlineFsHandle;
typedef intptr_t RedlineFsDirHandle;

typedef struct {
    uint64_t size;
    int64_t modified_seconds;
    uint32_t modified_nanoseconds;
    uint32_t mode;
    int is_directory;
} RedlineFsInfo;

typedef struct {
    char name[256];
    int is_directory;
} RedlineFsDirEntry;

/* Returns a nonnegative handle on success, -1 on failure (errno is preserved). */
RedlineFsHandle redline_fs_open(const char *path, unsigned flags, unsigned mode);
int redline_fs_close(RedlineFsHandle handle);
int64_t redline_fs_read(RedlineFsHandle handle, void *buffer, size_t bytes);
int64_t redline_fs_write(RedlineFsHandle handle, const void *buffer, size_t bytes);
int64_t redline_fs_pread(RedlineFsHandle handle, void *buffer, size_t bytes, uint64_t offset);
int64_t redline_fs_pwrite(RedlineFsHandle handle, const void *buffer, size_t bytes, uint64_t offset);
int64_t redline_fs_seek(RedlineFsHandle handle, int64_t offset, int whence);
int redline_fs_truncate(RedlineFsHandle handle, uint64_t size);
int redline_fs_sync(RedlineFsHandle handle);
int redline_fs_stat(const char *path, RedlineFsInfo *info);
int redline_fs_mkdir(const char *path, unsigned mode);
int redline_fs_unlink(const char *path);
int redline_fs_rmdir(const char *path);
RedlineFsDirHandle redline_fs_dir_open(const char *path);
int redline_fs_dir_read(RedlineFsDirHandle handle, RedlineFsDirEntry *entry); /* 1 entry, 0 end, -1 error */
int redline_fs_dir_close(RedlineFsDirHandle handle);
