/* Portable Xbox guest-path classification.
 * This layer parses Xbox device names only; it performs no host filesystem I/O. */
#pragma once

#include <stddef.h>
#include <limits.h>

enum {
    REDLINE_XBOX_PATH_UNKNOWN = 0,
    REDLINE_XBOX_PATH_DISC = 1,
    REDLINE_XBOX_PATH_PARTITION = 2
};

typedef struct {
    int device;
    unsigned partition;
    const char *suffix; /* points into the input string, after the device name */
} RedlineXboxPath;

static inline int redline_ascii_lower(int c) {
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

static inline int redline_ascii_ncasecmp(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int ac = redline_ascii_lower((unsigned char)a[i]);
        int bc = redline_ascii_lower((unsigned char)b[i]);
        if (ac != bc) return ac - bc;
        if (!ac) return 0;
    }
    return 0;
}

static inline int redline_xbox_path_classify(const char *path, RedlineXboxPath *out) {
    static const char disc[] = "\\Device\\CdRom0";
    static const char partition[] = "\\Device\\Harddisk0\\Partition";
    const char *p = path;
    unsigned number = 0;

    if (!path || !out) return 0;
    if (redline_ascii_ncasecmp(p, "\\??\\", 4) == 0) p += 4;

    out->device = REDLINE_XBOX_PATH_UNKNOWN;
    out->partition = 0;
    out->suffix = p;

    if (redline_ascii_ncasecmp(p, disc, sizeof disc - 1) == 0 &&
        (p[sizeof disc - 1] == '\\' || p[sizeof disc - 1] == '\0')) {
        out->device = REDLINE_XBOX_PATH_DISC;
        out->suffix = p + sizeof disc - 1;
        return 1;
    }

    if (redline_ascii_ncasecmp(p, partition, sizeof partition - 1) != 0) return 0;
    p += sizeof partition - 1;
    if (*p < '0' || *p > '9') return 0;
    do {
        unsigned digit = (unsigned)(*p++ - '0');
        if (number > (UINT_MAX - digit) / 10u) return 0;
        number = number * 10u + digit;
    } while (*p >= '0' && *p <= '9');
    if (*p && !(*p == '\\' && !p[1])) {
        if (*p != '\\') return 0;
    }
    out->device = REDLINE_XBOX_PATH_PARTITION;
    out->partition = number;
    out->suffix = p;
    return 1;
}

static inline int redline_xbox_path_is_partition_root(const char *path) {
    RedlineXboxPath parsed;
    return redline_xbox_path_classify(path, &parsed) &&
           parsed.device == REDLINE_XBOX_PATH_PARTITION &&
           (*parsed.suffix == '\0' || (parsed.suffix[0] == '\\' && !parsed.suffix[1]));
}
