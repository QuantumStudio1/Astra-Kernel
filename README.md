# Astra Kernel

Licensed under [MIT](LICENSE), Astra can be forked, modified, published, and
used commercially. See [CONTRIBUTING.md](CONTRIBUTING.md) and the public ABI in
[`include/astra.h`](include/astra.h).

Astra is a small, bootable, 32-bit x86 kernel foundation designed for building a Lua-driven operating system on top of it. It is deliberately a **kernel**, not a complete OS or Linux replacement.

## What works today

- Multiboot boot via GRUB (bootable ISO target)
- VGA text console and serial debug output
- Interrupt descriptor table, PIC remapping, and IRQ dispatch (initialized;
  IRQ delivery is intentionally staged until an OS layer attaches a task loop)
- Programmable timer and keyboard drivers ready for the later IRQ handoff
- Multiboot memory-map discovery and a simple physical page allocator
- Minimal cooperative task scheduler API
- RAM filesystem with `/etc/motd` and `/boot/init.lua` seeded at boot
- Stable, versioned `astra_*` kernel ABI and attachable Lua runtime bridge

## What is intentionally next

Paging / protection rings, ELF program loading, filesystems, networking, SMP, and drivers beyond timer/keyboard are not implemented. Those are sizeable subsystems; the included interfaces leave room to add them cleanly.

## Requirements

Use an `i686-elf` cross compiler, `grub-mkrescue`, `xorriso`, and QEMU. On Linux or WSL these are commonly packaged as `gcc-i686-linux-gnu` (or a cross compiler), `grub-pc-bin`, `xorriso`, and `qemu-system-x86`.

## Configure, build, run

Edit [`config/astra.conf`](config/astra.conf), then run:

```sh
make config
make iso
make run
```

The `config` target turns the friendly config file into `include/astra_config.h`. Override tools if needed:

```sh
make CROSS=i686-elf- QEMU=qemu-system-i386 iso
```

## Lua OS layer

`lua/os.lua` is an example policy layer. Port a Lua runtime by creating a `struct astra_lua_runtime`, calling `astra_lua_attach()`, and binding its `astra` table to the C ABI in [`include/astra.h`](include/astra.h). Then call `astra_lua_run_init()` to execute the RAMFS `/boot/init.lua`. Keep policy (shells, windows, services, app management) in Lua; keep privileged mechanisms (interrupts, memory, hardware) in Astra.

The bridge is included, but the Lua VM is not yet bundled: stock Lua depends on a hosted C runtime, so it needs a small freestanding allocator/libc port before embedding. The bridge keeps that port isolated from the rest of Astra.

## Layout

- `src/` — freestanding kernel implementation and x86 startup code
- `include/` — public kernel ABI
- `config/` — build-time settings
- `lua/` — example Lua OS layer and binding contract
- `scripts/` — configuration generator

## Safety note

Run it in QEMU. This project directly programs x86 hardware and is not appropriate to boot on a physical machine without much more driver and platform work.
