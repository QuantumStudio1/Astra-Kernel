# Freestanding Lua port plan

The kernel includes the stable adapter `struct astra_lua_runtime` in
`include/astra.h`. A Lua port should create a `lua_State`, register the Astra
module, then call `astra_lua_attach()` and `astra_lua_run_init()`.

## Required adaptations

Stock Lua is portable ISO C but assumes hosted services. The port must provide:

1. The included `astra_malloc`/`astra_realloc`/`astra_free` heap adapter,
   passed to `lua_newstate`. It is currently a deterministic 256 KiB kernel
   heap and will later grow through the physical page allocator.
2. Minimal `memcpy`, `memmove`, `memcmp`, and string utilities; avoid a hosted
   libc.
3. A panic handler that writes to `astra_write` instead of `stderr`.
4. A module loader that reads `/boot/*.lua` and `/packages/*/init.lua` through
   `astra_fs_read`.
5. The `astra` module: `write`, `ticks`, `fs_read`, `fs_write`, and eventually
   `spawn` and package APIs.

Do not include the standalone Lua CLI, dynamic loader, `io`, `os`, or `debug`
libraries in the initial port. They imply a hosted process model that Astra does
not yet have.

## Vendor layout

Lua 5.5.1 is vendored in `vendor/lua/`. Keep it unmodified; put Astra-specific
adapters in `src/lua_port/`. Its official source archive SHA-256 is
`1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce`.

Run `make lua-check` before beginning the port. It confirms that a fork has the
expected vendored source tree.
