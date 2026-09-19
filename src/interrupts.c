#include "astra.h"

static astra_irq_fn irq_handlers[16];
static inline void outb(uint16_t p, uint8_t v) { __asm__ volatile ("outb %0, %1" : : "a"(v), "Nd"(p)); }

struct idt_gate { uint16_t offset_low, selector; uint8_t zero, flags; uint16_t offset_high; } __attribute__((packed));
struct idt_pointer { uint16_t limit; uint32_t base; } __attribute__((packed));
static struct idt_gate idt[256];
extern uint32_t astra_vector_table[];

static void idt_set(uint8_t vector, uint32_t target) {
    idt[vector] = (struct idt_gate){ (uint16_t)target, 0x08, 0, 0x8E, (uint16_t)(target >> 16) };
}
void astra_idt_init(void) {
    for (uint8_t vector = 0; vector < 48; ++vector) idt_set(vector, astra_vector_table[vector]);
    struct idt_pointer pointer = { sizeof(idt) - 1, (uint32_t)idt };
    __asm__ volatile ("lidt %0" : : "m"(pointer));
}

void astra_irq_install(uint8_t irq, astra_irq_fn handler) { if (irq < 16) irq_handlers[irq] = handler; }
void astra_irq_dispatch(uint32_t irq) {
    if (irq < 16 && irq_handlers[irq]) irq_handlers[irq](irq);
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
void astra_interrupt_dispatch(uint32_t vector) {
    if (vector >= 32 && vector < 48) astra_irq_dispatch(vector - 32);
    /* Exceptions arrive here too. Keeping interrupts disabled makes this a safe stop. */
    else { __asm__ volatile ("cli"); for (;;) __asm__ volatile ("hlt"); }
}
void astra_pic_init(void) {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 4); outb(0xA1, 2);
    outb(0x21, 1); outb(0xA1, 1);
    outb(0x21, 0xFC); outb(0xA1, 0xFF); /* timer + keyboard */
}
