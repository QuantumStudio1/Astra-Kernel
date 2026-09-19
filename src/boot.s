.set MB_MAGIC,    0x1BADB002
.set MB_FLAGS,    0x00000003
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)
.altmacro

/* "a" keeps this header in the loadable early ELF segment. GRUB scans only
   the first 8192 bytes of the file, so an unallocated metadata section fails. */
.section .multiboot, "a"
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.extern astra_kernel_main
_start:
  mov $stack_top, %esp
  push %ebx
  push %eax
  call astra_kernel_main
1: hlt
  jmp 1b

/* Vectors 0–31 are exceptions; 32–47 are remapped PIC IRQs. */
.macro VECTOR n
.global astra_vector_\n
astra_vector_\n:
  pusha
  push $\n
  call astra_interrupt_dispatch
  add $4, %esp
  popa
  iret
.endm

.set vector_number, 0
.rept 48
  VECTOR %vector_number
  .set vector_number, vector_number + 1
.endr

.section .rodata
.global astra_vector_table
astra_vector_table:
.macro VECTOR_ENTRY n
  .long astra_vector_\n
.endm
.set table_number, 0
.rept 48
  VECTOR_ENTRY %table_number
  .set table_number, table_number + 1
.endr
