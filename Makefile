CROSS ?= i686-elf-
CC := $(CROSS)gcc
LD := $(CROSS)ld
QEMU ?= qemu-system-i386

CFLAGS := -m32 -std=gnu11 -ffreestanding -fno-stack-protector -fno-pie -fno-pic -Wall -Wextra -Werror -Iinclude
# Keep the Multiboot header inside GRUB's mandatory first-8-KiB scan window.
LDFLAGS := -m elf_i386 -z max-page-size=0x1000 -T linker.ld
OBJS := build/boot.o build/interrupts.o build/runtime.o build/fs.o build/lua_bridge.o build/kernel.o

.PHONY: all config lua-check kernel iso run clean
all: kernel

config: include/astra_config.h
lua-check:
	@test -f vendor/lua/src/lua.h && grep -q 'Lua 5.5.1' vendor/lua/README || (echo 'Missing or unexpected vendor/lua source; see docs/LUA_PORT.md' && false)
include/astra_config.h: config/astra.conf scripts/configure.py
	python3 scripts/configure.py $< $@

build:
	mkdir -p build
build/%.o: src/%.c include/astra.h include/astra_config.h | build
	$(CC) $(CFLAGS) -c $< -o $@
build/boot.o: src/boot.s | build
	$(CC) -m32 -c $< -o $@

kernel: include/astra_config.h $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o build/astra.elf $(OBJS)

iso: kernel
	mkdir -p build/isodir/boot/grub
	cp build/astra.elf build/isodir/boot/astra.elf
	printf 'menuentry "Astra" { multiboot /boot/astra.elf; boot }\n' > build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o build/astra.iso build/isodir

run: iso
	$(QEMU) -cdrom build/astra.iso -serial stdio

clean:
	rm -rf build include/astra_config.h
