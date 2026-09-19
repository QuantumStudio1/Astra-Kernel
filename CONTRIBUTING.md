# Contributing to Astra

Astra is a small, forkable kernel foundation for Lua-based operating systems.
Keep contributions freestanding: no hosted C library, POSIX assumptions, or
Linux ABI dependencies.

## Rules for kernel changes

- Preserve the public ABI in `include/astra.h`; increment `ASTRA_ABI_VERSION`
  for breaking changes.
- Add a short serial-console diagnostic for a new boot-critical subsystem.
- Keep hardware drivers separate from policy. Lua and the eventual shell define
  OS policy; the kernel owns memory, interrupts, devices, and filesystem I/O.
- Test every boot change in QEMU before opening a pull request.

## Licensing

Contributions are submitted under the MIT License in `LICENSE`. Third-party
code remains under its original license and must be documented in
`THIRD_PARTY_NOTICES.md`.
