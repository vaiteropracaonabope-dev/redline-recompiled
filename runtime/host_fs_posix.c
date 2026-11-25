#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include "host_fs.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int redline_fs_offset(uint64_t value, off_t *out) {
    off_t converted = (off_t)value;
    if (converted < 0 || (uint64_t)converted != value) {
        errno = EOVERFLOW;
        return 0;
    }
    *out = converted;
    return 1;
}

RedlineFsHandle redline_fs_open(const char *path, unsigned flags, unsigned mode) {
    int access = (flags & REDLINE_FS_WRITE)
        ? ((flags & REDLINE_FS_READ) ? O_RDWR : O_WRONLY)
        : O_RDONLY;
    int native = access;
    if (flags & REDLINE_FS_CREATE) native |= O_CREAT;
    if (flags & REDLINE_FS_EXCL) native |= O_EXCL;
    if (flags & REDLINE_FS_TRUNC) native |= O_TRUNC;
    if (flags & REDLINE_FS_APPEND) native |= O_APPEND;
#ifdef O_CLOEXEC
    native |= O_CLOEXEC;
#endif
    return (RedlineFsHandle)open(path, native, (mode_t)mode);
}

int redline_fs_close(RedlineFsHandle handle) {
    return close((int)handle);
}

int64_t redline_fs_read(RedlineFsHandle handle, void *buffer, size_t bytes) {
    ssize_t result = read((int)handle, buffer, bytes);
    return (int64_t)result;
}

int64_t redline_fs_write(RedlineFsHandle handle, const void *buffer, size_t bytes) {
    ssize_t result = write((int)handle, buffer, bytes);
    return (int64_t)result;
}

int64_t redline_fs_pread(RedlineFsHandle handle, void *buffer, size_t bytes, uint64_t offset) {
    off_t at;
    if (!redline_fs_offset(offset, &at)) return -1;
    return (int64_t)pread((int)handle, buffer, bytes, at);
}

int64_t redline_fs_pwrite(RedlineFsHandle handle, const void *buffer, size_t bytes, uint64_t offset) {
    off_t at;
    if (!redline_fs_offset(offset, &at)) return -1;
    return (int64_t)pwrite((int)handle, buffer, bytes, at);
}

int64_t redline_fs_seek(RedlineFsHandle handle, int64_t offset, int whence) {
    off_t result = lseek((int)handle, (off_t)offset, whence);
    return result < 0 ? -1 : (int64_t)result;
}

int redline_fs_truncate(RedlineFsHandle handle, uint64_t size) {
    off_t length;
    if (!redline_fs_offset(size, &length)) return -1;
    return ftruncate((int)handle, length);
}

int redline_fs_sync(RedlineFsHandle handle) {
    return fsync((int)handle);
}

int redline_fs_stat(const char *path, RedlineFsInfo *info) {
    struct stat st;
    if (!info) {
        errno = EINVAL;
        return -1;
    }
    if (stat(path, &st)) return -1;
    info->size = (uint64_t)st.st_size;
    info->created_seconds = 0; /* portable POSIX stat has no creation/birth timestamp */
    info->accessed_seconds = (int64_t)st.st_atime;
    info->modified_seconds = (int64_t)st.st_mtime;
#if defined(__APPLE__)
    info->modified_nanoseconds = (uint32_t)st.st_mtimespec.tv_nsec;
#else
    info->modified_nanoseconds = (uint32_t)st.st_mtim.tv_nsec;
#endif
    info->mode = (uint32_t)st.st_mode;
    info->is_directory = S_ISDIR(st.st_mode);
    return 0;
}

int redline_fs_set_times(RedlineFsHandle handle, const RedlineFsTime *created,
                         const RedlineFsTime *accessed, const RedlineFsTime *modified) {
    struct timespec times[2];
    (void)created; /* POSIX does not provide a portable birth-time setter. */
    if (accessed) {
        times[0].tv_sec = (time_t)accessed->seconds;
        times[0].tv_nsec = accessed->nanoseconds;
    } else {
        times[0].tv_sec = 0;
        times[0].tv_nsec = UTIME_OMIT;
    }
    if (modified) {
        times[1].tv_sec = (time_t)modified->seconds;
        times[1].tv_nsec = modified->nanoseconds;
    } else {
        times[1].tv_sec = 0;
        times[1].tv_nsec = UTIME_OMIT;
    }
    return futimens((int)handle, times);
}

int redline_fs_mkdir(const char *path, unsigned mode) {
    return mkdir(path, (mode_t)mode);
}

int redline_fs_unlink(const char *path) {
    return unlink(path);
}

int redline_fs_rmdir(const char *path) {
    return rmdir(path);
}

RedlineFsDirHandle redline_fs_dir_open(const char *path) {
    return (RedlineFsDirHandle)opendir(path);
}

int redline_fs_dir_read(RedlineFsDirHandle handle, RedlineFsDirEntry *entry) {
    DIR *dir = (DIR *)handle;
    struct dirent *item;
    if (!dir || !entry) {
        errno = EINVAL;
        return -1;
    }
    errno = 0;
    item = readdir(dir);
    if (!item) return errno ? -1 : 0;
    size_t length = strlen(item->d_name);
    if (length >= sizeof entry->name) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(entry->name, item->d_name, length + 1);
#ifdef DT_DIR
    entry->is_directory = item->d_type == DT_DIR;
    if (item->d_type == DT_UNKNOWN) {
        struct stat st;
        if (fstatat(dirfd(dir), item->d_name, &st, 0)) return -1;
        entry->is_directory = S_ISDIR(st.st_mode);
    }
#else
    {
        struct stat st;
        if (fstatat(dirfd(dir), item->d_name, &st, 0)) return -1;
        entry->is_directory = S_ISDIR(st.st_mode);
    }
#endif
    return 1;
}

int redline_fs_dir_close(RedlineFsDirHandle handle) {
    return closedir((DIR *)handle);
}
