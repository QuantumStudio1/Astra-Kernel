#include "astra.h"

#define ASTRA_HEAP_BYTES (256u * 1024u)
#define ALIGNMENT 8u

struct heap_block {
    size_t size;
    struct heap_block *next;
    uint8_t free;
};

static uint8_t heap_storage[ASTRA_HEAP_BYTES] __attribute__((aligned(ALIGNMENT)));
static struct heap_block *heap_head;

static size_t align_size(size_t size) { return (size + (ALIGNMENT - 1u)) & ~(ALIGNMENT - 1u); }
void astra_heap_init(void) {
    heap_head = (struct heap_block *)heap_storage;
    heap_head->size = ASTRA_HEAP_BYTES - sizeof(*heap_head);
    heap_head->next = NULL;
    heap_head->free = 1;
}
void *astra_memset(void *destination, int value, size_t length) {
    uint8_t *out = destination;
    for (size_t at = 0; at < length; ++at) out[at] = (uint8_t)value;
    return destination;
}
void *astra_memcpy(void *destination, const void *source, size_t length) {
    uint8_t *out = destination; const uint8_t *in = source;
    for (size_t at = 0; at < length; ++at) out[at] = in[at];
    return destination;
}
void *astra_memmove(void *destination, const void *source, size_t length) {
    uint8_t *out = destination; const uint8_t *in = source;
    if (out <= in) return astra_memcpy(destination, source, length);
    while (length) { --length; out[length] = in[length]; }
    return destination;
}
int astra_memcmp(const void *left, const void *right, size_t length) {
    const uint8_t *a = left; const uint8_t *b = right;
    for (size_t at = 0; at < length; ++at) if (a[at] != b[at]) return a[at] < b[at] ? -1 : 1;
    return 0;
}
size_t astra_strlen(const char *text) { size_t length = 0; while (text && text[length]) ++length; return length; }
static void split_block(struct heap_block *block, size_t size) {
    if (block->size < size + sizeof(*block) + ALIGNMENT) return;
    struct heap_block *tail = (struct heap_block *)((uint8_t *)(block + 1) + size);
    tail->size = block->size - size - sizeof(*tail);
    tail->next = block->next;
    tail->free = 1;
    block->size = size;
    block->next = tail;
}
void *astra_malloc(size_t size) {
    if (!heap_head) astra_heap_init();
    if (!size) return NULL;
    size = align_size(size);
    for (struct heap_block *block = heap_head; block; block = block->next) {
        if (block->free && block->size >= size) {
            split_block(block, size); block->free = 0; return block + 1;
        }
    }
    return NULL;
}
static void coalesce(void) {
    for (struct heap_block *block = heap_head; block && block->next; ) {
        uint8_t *end = (uint8_t *)(block + 1) + block->size;
        if (block->free && block->next->free && end == (uint8_t *)block->next) {
            block->size += sizeof(*block) + block->next->size; block->next = block->next->next;
        } else block = block->next;
    }
}
void astra_free(void *pointer) {
    if (!pointer) return;
    struct heap_block *block = ((struct heap_block *)pointer) - 1;
    block->free = 1; coalesce();
}
void *astra_realloc(void *pointer, size_t size) {
    if (!pointer) return astra_malloc(size);
    if (!size) { astra_free(pointer); return NULL; }
    struct heap_block *old = ((struct heap_block *)pointer) - 1;
    if (old->size >= size) return pointer;
    void *replacement = astra_malloc(size);
    if (!replacement) return NULL;
    astra_memcpy(replacement, pointer, old->size); astra_free(pointer);
    return replacement;
}
