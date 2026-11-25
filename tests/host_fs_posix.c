#define _POSIX_C_SOURCE 200809L
#include "host_fs.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "host fs test failed at line %d: %s (errno=%d)\n", __LINE__, #x, errno); return 1; } } while (0)

int main(void) {
    char root[] = "/tmp/redline-fs-XXXXXX";
    char file[512], subdir[512];
    char readback[16] = {0};
    const char payload[] = "android-fs";
    RedlineFsInfo info;
    RedlineFsDirEntry entry;
    RedlineFsDirHandle dir;
    RedlineFsHandle handle;
    int found = 0;

    CHECK(mkdtemp(root) != NULL);
    CHECK(snprintf(file, sizeof file, "%s/save.dat", root) > 0);
    CHECK(snprintf(subdir, sizeof subdir, "%s/cache", root) > 0);
    CHECK(redline_fs_mkdir(subdir, 0700) == 0);

    handle = redline_fs_open(file, REDLINE_FS_READ | REDLINE_FS_WRITE |
                            REDLINE_FS_CREATE | REDLINE_FS_EXCL, 0600);
    CHECK(handle >= 0);
    CHECK(redline_fs_write(handle, payload, sizeof payload) == (int64_t)sizeof payload);
    CHECK(redline_fs_pread(handle, readback, sizeof payload, 0) == (int64_t)sizeof payload);
    CHECK(memcmp(readback, payload, sizeof payload) == 0);
    CHECK(redline_fs_pwrite(handle, "SAVE", 4, 0) == 4);
    CHECK(redline_fs_seek(handle, 0, SEEK_SET) == 0);
    memset(readback, 0, sizeof readback);
    CHECK(redline_fs_read(handle, readback, 4) == 4);
    CHECK(memcmp(readback, "SAVE", 4) == 0);
    CHECK(redline_fs_truncate(handle, 4) == 0);
    CHECK(redline_fs_sync(handle) == 0);
    CHECK(redline_fs_close(handle) == 0);

    CHECK(redline_fs_stat(file, &info) == 0);
    CHECK(info.size == 4 && !info.is_directory);
    CHECK(redline_fs_stat(subdir, &info) == 0 && info.is_directory);

    dir = redline_fs_dir_open(root);
    CHECK(dir != (RedlineFsDirHandle)0);
    while (redline_fs_dir_read(dir, &entry) == 1)
        if (strcmp(entry.name, "save.dat") == 0) found = 1;
    CHECK(found);
    CHECK(redline_fs_dir_close(dir) == 0);
    CHECK(redline_fs_unlink(file) == 0);
    CHECK(redline_fs_rmdir(subdir) == 0);
    CHECK(rmdir(root) == 0);
    return 0;
}
