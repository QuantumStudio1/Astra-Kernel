#include "astra.h"

#define ASTRA_PATH_CAPACITY 64u

struct ram_file {
    char path[ASTRA_PATH_CAPACITY];
    uint8_t data[ASTRA_FS_FILE_CAPACITY];
    size_t length;
    uint8_t used;
};

static struct ram_file files[ASTRA_FS_MAX_FILES];

static int equal(const char *left, const char *right) {
    while (*left && *right && *left == *right) { ++left; ++right; }
    return *left == '\0' && *right == '\0';
}
static size_t copy(char *destination, const char *source, size_t capacity) {
    size_t at = 0;
    while (source[at] && at + 1 < capacity) { destination[at] = source[at]; ++at; }
    destination[at] = '\0';
    return at;
}
static struct ram_file *find(const char *path) {
    for (size_t at = 0; at < ASTRA_FS_MAX_FILES; ++at)
        if (files[at].used && equal(files[at].path, path)) return &files[at];
    return NULL;
}
static struct ram_file *create(const char *path) {
    for (size_t at = 0; at < ASTRA_FS_MAX_FILES; ++at) {
        if (!files[at].used) {
            if (copy(files[at].path, path, ASTRA_PATH_CAPACITY) == 0 || path[ASTRA_PATH_CAPACITY - 1]) return NULL;
            files[at].used = 1;
            return &files[at];
        }
    }
    return NULL;
}
void astra_fs_init(void) { for (size_t at = 0; at < ASTRA_FS_MAX_FILES; ++at) files[at].used = 0; }
int astra_fs_exists(const char *path) { return path && find(path) != NULL; }
int astra_fs_write(const char *path, const void *data, size_t length) {
    if (!path || !data || path[0] != '/' || length > ASTRA_FS_FILE_CAPACITY) return -1;
    struct ram_file *file = find(path);
    if (!file) file = create(path);
    if (!file) return -2;
    const uint8_t *source = data;
    for (size_t at = 0; at < length; ++at) file->data[at] = source[at];
    file->length = length;
    return 0;
}
int astra_fs_read(const char *path, void *out, size_t capacity, size_t *length) {
    struct ram_file *file = path ? find(path) : NULL;
    if (!file) return -1;
    if (length) *length = file->length;
    if (!out) return 0;
    if (capacity < file->length) return -2;
    uint8_t *destination = out;
    for (size_t at = 0; at < file->length; ++at) destination[at] = file->data[at];
    return 0;
}
