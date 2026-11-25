#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "host_fs.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    HANDLE find;
    WIN32_FIND_DATAA data;
    int first;
} RedlineFsWinDir;

static int redline_fs_errno(DWORD error) {
    switch (error) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND: return ENOENT;
    case ERROR_ACCESS_DENIED: return EACCES;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS: return EEXIST;
    case ERROR_INVALID_HANDLE: return EBADF;
    case ERROR_DISK_FULL: return ENOSPC;
    case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY: return ENOMEM;
    default: return EIO;
    }
}

static void redline_fs_set_errno(void) {
    errno = redline_fs_errno(GetLastError());
}

static int64_t redline_fs_filetime_seconds(FILETIME time) {
    ULARGE_INTEGER ticks;
    ticks.LowPart = time.dwLowDateTime;
    ticks.HighPart = time.dwHighDateTime;
    return (int64_t)((ticks.QuadPart - 116444736000000000ull) / 10000000ull);
}

RedlineFsHandle redline_fs_open(const char *path, unsigned flags, unsigned mode) {
    DWORD access = 0, creation = OPEN_EXISTING;
    HANDLE handle;
    (void)mode;
    if (flags & REDLINE_FS_READ) access |= GENERIC_READ;
    if (flags & REDLINE_FS_WRITE) access |= GENERIC_WRITE;
    if (flags & REDLINE_FS_CREATE) creation = (flags & REDLINE_FS_EXCL) ? CREATE_NEW : OPEN_ALWAYS;
    if (flags & REDLINE_FS_TRUNC) creation = (flags & REDLINE_FS_CREATE) ? CREATE_ALWAYS : TRUNCATE_EXISTING;
    if (flags & REDLINE_FS_APPEND) access |= FILE_APPEND_DATA;
    handle = CreateFileA(path, access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        redline_fs_set_errno();
        return (RedlineFsHandle)-1;
    }
    return (RedlineFsHandle)(intptr_t)handle;
}

int redline_fs_close(RedlineFsHandle handle) {
    if (CloseHandle((HANDLE)(intptr_t)handle)) return 0;
    redline_fs_set_errno();
    return -1;
}

int64_t redline_fs_read(RedlineFsHandle handle, void *buffer, size_t bytes) {
    DWORD done = 0;
    DWORD request = bytes > MAXDWORD ? MAXDWORD : (DWORD)bytes;
    if (ReadFile((HANDLE)(intptr_t)handle, buffer, request, &done, NULL)) return done;
    redline_fs_set_errno();
    return -1;
}

int64_t redline_fs_write(RedlineFsHandle handle, const void *buffer, size_t bytes) {
    DWORD done = 0;
    DWORD request = bytes > MAXDWORD ? MAXDWORD : (DWORD)bytes;
    if (WriteFile((HANDLE)(intptr_t)handle, buffer, request, &done, NULL)) return done;
    redline_fs_set_errno();
    return -1;
}

static int redline_fs_overlapped(HANDLE handle, void *buffer, size_t bytes, uint64_t offset,
                                 int writing, int64_t *result) {
    OVERLAPPED operation;
    DWORD done = 0, request = bytes > MAXDWORD ? MAXDWORD : (DWORD)bytes;
    memset(&operation, 0, sizeof operation);
    operation.Offset = (DWORD)offset;
    operation.OffsetHigh = (DWORD)(offset >> 32);
    BOOL ok = writing ? WriteFile(handle, buffer, request, &done, &operation)
                      : ReadFile(handle, buffer, request, &done, &operation);
    if (!ok && GetLastError() == ERROR_IO_PENDING)
        ok = GetOverlappedResult(handle, &operation, &done, TRUE);
    if (!ok) {
        DWORD error = GetLastError();
        if (!writing && error == ERROR_HANDLE_EOF) {
            *result = 0;
            return 0;
        }
        SetLastError(error);
        redline_fs_set_errno();
        return -1;
    }
    *result = done;
    return 0;
}

int64_t redline_fs_pread(RedlineFsHandle handle, void *buffer, size_t bytes, uint64_t offset) {
    int64_t result;
    return redline_fs_overlapped((HANDLE)(intptr_t)handle, buffer, bytes, offset, 0, &result) ? -1 : result;
}

int64_t redline_fs_pwrite(RedlineFsHandle handle, const void *buffer, size_t bytes, uint64_t offset) {
    int64_t result;
    return redline_fs_overlapped((HANDLE)(intptr_t)handle, (void *)buffer, bytes, offset, 1, &result) ? -1 : result;
}

int64_t redline_fs_seek(RedlineFsHandle handle, int64_t offset, int whence) {
    LARGE_INTEGER distance, result;
    DWORD method = whence == SEEK_SET ? FILE_BEGIN : whence == SEEK_CUR ? FILE_CURRENT :
                   whence == SEEK_END ? FILE_END : (DWORD)-1;
    if (method == (DWORD)-1) { errno = EINVAL; return -1; }
    distance.QuadPart = offset;
    if (!SetFilePointerEx((HANDLE)(intptr_t)handle, distance, &result, method)) {
        redline_fs_set_errno();
        return -1;
    }
    return result.QuadPart;
}

int redline_fs_truncate(RedlineFsHandle handle, uint64_t size) {
    LARGE_INTEGER current, end, ignored, zero;
    HANDLE file = (HANDLE)(intptr_t)handle;
    end.QuadPart = size;
    zero.QuadPart = 0;
    if (!SetFilePointerEx(file, zero, &current, FILE_CURRENT) ||
        !SetFilePointerEx(file, end, &ignored, FILE_BEGIN) || !SetEndOfFile(file)) {
        redline_fs_set_errno();
        return -1;
    }
    if (!SetFilePointerEx(file, current, NULL, FILE_BEGIN)) {
        redline_fs_set_errno();
        return -1;
    }
    return 0;
}

int redline_fs_sync(RedlineFsHandle handle) {
    if (FlushFileBuffers((HANDLE)(intptr_t)handle)) return 0;
    redline_fs_set_errno();
    return -1;
}

uint64_t redline_fs_monotonic_ns(void) {
    LARGE_INTEGER counter, frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) return 0;
    return (uint64_t)((counter.QuadPart * 1000000000ull) / (uint64_t)frequency.QuadPart);
}

int redline_fs_stat(const char *path, RedlineFsInfo *info) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!info) { errno = EINVAL; return -1; }
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data)) {
        redline_fs_set_errno();
        return -1;
    }
    info->size = ((uint64_t)data.nFileSizeHigh << 32) | data.nFileSizeLow;
    info->created_seconds = redline_fs_filetime_seconds(data.ftCreationTime);
    info->accessed_seconds = redline_fs_filetime_seconds(data.ftLastAccessTime);
    info->modified_seconds = redline_fs_filetime_seconds(data.ftLastWriteTime);
    info->modified_nanoseconds = 0;
    info->mode = data.dwFileAttributes;
    info->is_directory = !!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    return 0;
}

static FILETIME redline_fs_to_filetime(const RedlineFsTime *time) {
    ULARGE_INTEGER ticks;
    FILETIME result;
    ticks.QuadPart = ((uint64_t)(time->seconds + 11644473600ll)) * 10000000ull +
                     time->nanoseconds / 100u;
    result.dwLowDateTime = ticks.LowPart;
    result.dwHighDateTime = ticks.HighPart;
    return result;
}

int redline_fs_set_times(RedlineFsHandle handle, const RedlineFsTime *created,
                         const RedlineFsTime *accessed, const RedlineFsTime *modified) {
    FILETIME c, a, m;
    if (created) c = redline_fs_to_filetime(created);
    if (accessed) a = redline_fs_to_filetime(accessed);
    if (modified) m = redline_fs_to_filetime(modified);
    if (SetFileTime((HANDLE)(intptr_t)handle, created ? &c : NULL,
                    accessed ? &a : NULL, modified ? &m : NULL)) return 0;
    redline_fs_set_errno();
    return -1;
}

int redline_fs_mkdir(const char *path, unsigned mode) {
    (void)mode;
    if (CreateDirectoryA(path, NULL)) return 0;
    redline_fs_set_errno();
    return -1;
}

int redline_fs_unlink(const char *path) {
    if (DeleteFileA(path)) return 0;
    redline_fs_set_errno();
    return -1;
}

int redline_fs_rmdir(const char *path) {
    if (RemoveDirectoryA(path)) return 0;
    redline_fs_set_errno();
    return -1;
}

RedlineFsDirHandle redline_fs_dir_open(const char *path) {
    RedlineFsWinDir *dir = (RedlineFsWinDir *)calloc(1, sizeof *dir);
    char pattern[MAX_PATH];
    if (!dir) { errno = ENOMEM; return (RedlineFsDirHandle)0; }
    int length = snprintf(pattern, sizeof pattern, "%s\\*", path);
    if (length < 0 || (size_t)length >= sizeof pattern) {
        free(dir);
        errno = ENAMETOOLONG;
        return (RedlineFsDirHandle)0;
    }
    dir->find = FindFirstFileA(pattern, &dir->data);
    if (dir->find == INVALID_HANDLE_VALUE) {
        redline_fs_set_errno();
        free(dir);
        return (RedlineFsDirHandle)0;
    }
    dir->first = 1;
    return (RedlineFsDirHandle)(intptr_t)dir;
}

int redline_fs_dir_read(RedlineFsDirHandle handle, RedlineFsDirEntry *entry) {
    RedlineFsWinDir *dir = (RedlineFsWinDir *)(intptr_t)handle;
    if (!dir || !entry) { errno = EINVAL; return -1; }
    if (dir->first) dir->first = 0;
    else if (!FindNextFileA(dir->find, &dir->data)) {
        if (GetLastError() == ERROR_NO_MORE_FILES) return 0;
        redline_fs_set_errno();
        return -1;
    }
    if (strlen(dir->data.cFileName) >= sizeof entry->name) { errno = ENAMETOOLONG; return -1; }
    memcpy(entry->name, dir->data.cFileName, strlen(dir->data.cFileName) + 1);
    entry->is_directory = !!(dir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    return 1;
}

int redline_fs_dir_close(RedlineFsDirHandle handle) {
    RedlineFsWinDir *dir = (RedlineFsWinDir *)(intptr_t)handle;
    if (!dir) { errno = EINVAL; return -1; }
    int ok = FindClose(dir->find);
    if (!ok) redline_fs_set_errno();
    free(dir);
    return ok ? 0 : -1;
}
