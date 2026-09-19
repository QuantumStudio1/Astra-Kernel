#include "astra.h"

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002u
#define MB_FLAG_MMAP (1u << 6)

struct mb_info { uint32_t flags, mem_lower, mem_upper, boot_device, cmdline, mods_count, mods_addr; uint32_t syms[4], mmap_length, mmap_addr; } __attribute__((packed));
struct mb_mmap { uint32_t size; uint64_t addr, len; uint32_t type; } __attribute__((packed));

static volatile uint16_t *const vga = (uint16_t *)0xB8000;
static uint16_t cursor;
static volatile uint32_t tick_count;
static uint32_t next_page = 0x00400000u;
static astra_task_fn tasks[MAX_TASKS];
static uint8_t task_count, current_task;

static inline void outb(uint16_t p, uint8_t v) { __asm__ volatile ("outb %0, %1" : : "a"(v), "Nd"(p)); }
static inline uint8_t inb(uint16_t p) { uint8_t v; __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(p)); return v; }
static void serial_putc(char c) { if (SERIAL_DEBUG) { while (!(inb(0x3FD) & 0x20)) {} outb(0x3F8, (uint8_t)c); } }
static void putc(char c) { if (c == '\n') cursor = (uint16_t)((cursor / SCREEN_COLUMNS + 1) * SCREEN_COLUMNS); else vga[cursor++] = 0x0F00u | (uint8_t)c; serial_putc(c); }
void astra_write(const char *s) { while (*s) putc(*s++); }
uint32_t astra_abi_version(void) { return ASTRA_ABI_VERSION; }
uint32_t astra_ticks(void) { return tick_count; }
void *astra_page_alloc(void) { uint32_t p = next_page; next_page += ASTRA_PAGE_SIZE; return (void *)p; }
int astra_task_spawn(astra_task_fn entry) { if (!entry || task_count == MAX_TASKS) return -1; tasks[task_count++] = entry; return task_count - 1; }
void astra_yield(void) { if (task_count) { current_task = (uint8_t)((current_task + 1) % task_count); tasks[current_task](); } }
static void timer(uint32_t irq) { (void)irq; ++tick_count; }
static void keyboard(uint32_t irq) { (void)irq; (void)inb(0x60); }
extern void astra_pic_init(void);
extern void astra_idt_init(void);
static void timer_init(void) { uint16_t d = (uint16_t)(1193182u / TIMER_HZ); outb(0x43, 0x36); outb(0x40, d & 0xFF); outb(0x40, d >> 8); }
static void serial_init(void) { if (!SERIAL_DEBUG) return; outb(0x3F9, 0); outb(0x3FB, 0x80); outb(0x3F8, 3); outb(0x3F9, 0); outb(0x3FB, 3); outb(0x3FC, 3); }
static void memory_report(uint32_t info_ptr) { struct mb_info *i = (struct mb_info *)info_ptr; if (!(i->flags & MB_FLAG_MMAP)) { astra_write("no memory map\n"); return; } astra_write("multiboot memory map found\n"); }
void astra_kernel_main(uint32_t magic, uint32_t info) {
    serial_init(); astra_write("Astra kernel v1\n");
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) { astra_write("bad boot protocol\n"); for (;;) __asm__ volatile("hlt"); }
    memory_report(info); astra_heap_init(); astra_fs_init();
    static const char motd[] = "Astra RAM filesystem online.\n";
    static const char init_lua[] = "-- /boot/init.lua\nastra.write('Astra Lua OS layer ready\\n')\n";
    (void)astra_fs_write("/etc/motd", motd, sizeof(motd) - 1);
    (void)astra_fs_write("/boot/init.lua", init_lua, sizeof(init_lua) - 1);
    astra_idt_init(); astra_pic_init(); astra_irq_install(0, timer); astra_irq_install(1, keyboard); timer_init();
    /* Keep IF clear during the first boot milestone. The IDT/PIC/timer setup
       above is ready for the later scheduler handoff, but enabling hardware
       IRQs before a Lua runtime or task loop is attached can turn a driver
       fault into a reset with no useful diagnostic. */
    astra_write("RAMFS ready; Lua bridge may attach (IRQs staged)\n");
    for (;;) __asm__ volatile("hlt");
}
