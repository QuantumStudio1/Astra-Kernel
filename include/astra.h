#pragma once
#include <stdint.h>
#include <stddef.h>
#include "astra_config.h"

#define ASTRA_ABI_VERSION 1u
#define ASTRA_PAGE_SIZE 4096u

typedef void (*astra_task_fn)(void);
typedef void (*astra_irq_fn)(uint32_t irq);

/* Freestanding runtime services. Used by ports such as the embedded Lua VM. */
void astra_heap_init(void);
void *astra_malloc(size_t size);
void *astra_realloc(void *pointer, size_t size);
void astra_free(void *pointer);
void *astra_memcpy(void *destination, const void *source, size_t length);
void *astra_memmove(void *destination, const void *source, size_t length);
void *astra_memset(void *destination, int value, size_t length);
int astra_memcmp(const void *left, const void *right, size_t length);
size_t astra_strlen(const char *text);

#define ASTRA_FS_MAX_FILES 16u
#define ASTRA_FS_FILE_CAPACITY 4096u

/* RAM filesystem: paths are absolute, files are retained until reboot. */
int astra_fs_write(const char *path, const void *data, size_t length);
int astra_fs_read(const char *path, void *out, size_t capacity, size_t *length);
int astra_fs_exists(const char *path);
void astra_fs_init(void);

/*
 * A Lua port supplies this small adapter after it creates lua_State. This keeps
 * Lua as OS policy while Astra retains ownership of privileged mechanisms.
 */
struct astra_lua_runtime {
    uint32_t abi_version;
    int (*run_file)(const char *path, const char *source, size_t length);
    void (*register_kernel_api)(void);
};
int astra_lua_attach(const struct astra_lua_runtime *runtime);
int astra_lua_run_init(void);
int astra_lua_attached(void);

/* Core services intended for a Lua binding. All return 0 on success. */
uint32_t astra_abi_version(void);
void astra_write(const char *text);
uint32_t astra_ticks(void);
int astra_task_spawn(astra_task_fn entry);
void astra_yield(void);
void *astra_page_alloc(void);
void astra_irq_install(uint8_t irq, astra_irq_fn handler);

/* Platform initialization; called by the x86 bootstrap only. */
void astra_kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info);
