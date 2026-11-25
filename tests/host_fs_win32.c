#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "host_fs.h"

#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "host fs win32 test failed at line %d\n", __LINE__); return 1; } } while (0)

int main(void) {
    char root[MAX_PATH], temp[MAX_PATH], file[MAX_PATH], subdir[MAX_PATH];
    char buffer[32] = {0};
    const char payload[] = "windows-fs";
    RedlineFsInfo info;
    RedlineFsDirEntry entry;
    RedlineFsDirHandle dir;
    RedlineFsHandle handle;
    int found = 0;

    CHECK(GetTempPathA(MAX_PATH, root) > 0);
    CHECK(GetTempFileNameA(root, "rfs", 0, temp) != 0);
    CHECK(DeleteFileA(temp));
    CHECK(CreateDirectoryA(temp, NULL));
    CHECK(snprintf(file, sizeof file, "%s\\save.dat", temp) > 0);
    CHECK(snprintf(subdir, sizeof subdir, "%s\\cache", temp) > 0);
    CHECK(redline_fs_mkdir(subdir, 0700) == 0);

    handle = redline_fs_open(file, REDLINE_FS_READ | REDLINE_FS_WRITE |
                             REDLINE_FS_CREATE | REDLINE_FS_EXCL, 0600);
    CHECK(handle >= 0);
    CHECK(redline_fs_write(handle, payload, sizeof payload) == (int64_t)sizeof payload);
    CHECK(redline_fs_seek(handle, 4, SEEK_SET) == 4);
    CHECK(redline_fs_truncate(handle, 5) == 0);
    CHECK(redline_fs_seek(handle, 0, SEEK_CUR) == 4); /* truncate preserves current offset */
    CHECK(redline_fs_pread(handle, buffer, 5, 0) == 5);
    CHECK(memcmp(buffer, "windo", 5) == 0);
    CHECK(redline_fs_pwrite(handle, "SAVE", 4, 0) == 4);
    CHECK(redline_fs_sync(handle) == 0);
    CHECK(redline_fs_close(handle) == 0);

    CHECK(redline_fs_stat(file, &info) == 0);
    CHECK(info.size == 5 && !info.is_directory);
    CHECK(redline_fs_stat(subdir, &info) == 0 && info.is_directory);

    dir = redline_fs_dir_open(temp);
    CHECK(dir != 0);
    while (redline_fs_dir_read(dir, &entry) == 1)
        if (strcmp(entry.name, "save.dat") == 0) found = 1;
    CHECK(found);
    CHECK(redline_fs_dir_close(dir) == 0);
    CHECK(redline_fs_unlink(file) == 0);
    CHECK(redline_fs_rmdir(subdir) == 0);
    CHECK(redline_fs_rmdir(temp) == 0);
    return 0;
}
