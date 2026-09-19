#include "astra.h"

static const struct astra_lua_runtime *lua_runtime;
static char init_source[ASTRA_FS_FILE_CAPACITY];

int astra_lua_attach(const struct astra_lua_runtime *runtime) {
    if (!runtime || runtime->abi_version != ASTRA_ABI_VERSION || !runtime->run_file) return -1;
    lua_runtime = runtime;
    if (runtime->register_kernel_api) runtime->register_kernel_api();
    return 0;
}
int astra_lua_attached(void) { return lua_runtime != NULL; }
int astra_lua_run_init(void) {
    size_t length = 0;
    if (!lua_runtime) return -1;
    if (astra_fs_read("/boot/init.lua", init_source, sizeof(init_source), &length) != 0) return -2;
    return lua_runtime->run_file("/boot/init.lua", init_source, length);
}
