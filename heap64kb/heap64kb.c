#include <stdint.h>

struct heap64kbHeader_t {
    uint16_t size;
};

struct heap64kbFreeHeader_t {
    uint16_t size;
    uint16_t next;
};

struct heap64kb_t {
    struct heap64kbFreeHeader_t * head; /* smaller as offset, but this is grokkable */
};

#define HEAP64KB_MASK_IN_USE ((uint16_t)1)

struct heap64kb_t * heap64kb_init(void * const ptr, const size_t size) {
    struct heap64kb_t * const obj = (struct heap64kb_t *)ptr;
    struct heap64kbFreeHeader_t * const hdr = (struct heap64kbFreeHeader_t *)((uintptr_t)ptr + sizeof(struct heap64kb_t));
    const uint16_t usable = (uint16_t)(size & ~HEAP64KB_MASK_IN_USE) - sizeof(struct heap64kb_t);

    if (!ptr || size > 64 * 1024 || size < sizeof(struct heap64kb_t) + sizeof(struct heap64kbFreeHeader_t)) {
        return 0;
    }

    obj->head = hdr;
    hdr->size = usable;
    hdr->next = 0;

    return obj;
}

void heap64kb_free(struct heap64kb_t * const obj, void * const ptr) {
    struct heap64kbFreeHeader_t * const hdr = (struct heap64kbFreeHeader_t *)((uintptr_t)ptr - sizeof(struct heap64kbHeader_t));
    struct heap64kbFreeHeader_t * prev;

    if (ptr == 0) {
        return;
    }

    hdr->size &= ~HEAP64KB_MASK_IN_USE;

    if (!obj->head) {
        obj->head = hdr;
        return;
    }

    /*
    * best fit orders by size (small->large) then optionally address for early/best
    * worst fit orders by size (large->small)
    * first fit orders by address
    * fast versions might use a btree
    */

    /* head insertion */
    if (hdr->size < obj->head->size) {
        hdr->next = (uint16_t)((uintptr_t)obj->head - (uintptr_t)obj);
        obj->head = hdr;
        return;
    }

    /* body insertion */
    prev = obj->head;
    while (prev->next) {
        struct heap64kbFreeHeader_t * const next = (struct heap64kbFreeHeader_t *)((uintptr_t)obj + prev->next);
        if (next->size >= hdr->size) {
            hdr->next = (uint16_t)((uintptr_t)next - (uintptr_t)obj);
            prev->next = (uint16_t)((uintptr_t)hdr - (uintptr_t)obj);
            return;
        }
        prev = next;
    }

    /* tail insertion */
    hdr->next = 0;
    prev->next = (uint16_t)((uintptr_t)hdr - (uintptr_t)obj);
}

void* heap64kb_alloc(struct heap64kb_t * const obj, const size_t size) {
    struct heap64kbFreeHeader_t * prev = 0;
    struct heap64kbFreeHeader_t * next;
    const uint16_t requestSize = (uint16_t)(size + 1 & ~1);

    if (obj == 0) {
        return 0;
    }

    if (!obj->head) {
        return 0;
    }

    next = obj->head;
    do {
        const uint16_t usable = (uint16_t)(next->size - sizeof(struct heap64kbFreeHeader_t));
        if (usable >= requestSize) {
            uint8_t * const ptr = (uint8_t *)next + sizeof(struct heap64kbHeader_t);
            const uint16_t left = usable - requestSize;

            /* remove current from free list */
            if (prev) {
                prev->next = next->next;
            } else if (next->next) {
                obj->head = (struct heap64kbFreeHeader_t *)((uintptr_t)obj + next->next);
            } else {
                obj->head = 0;
            }

            /* put remainder of block in free list */
            if (left >= sizeof(struct heap64kbFreeHeader_t)) {
                struct heap64kbHeader_t * const splitHeader = (struct heap64kbHeader_t *)(ptr + requestSize);

                next->size -= left;

                splitHeader->size = left | HEAP64KB_MASK_IN_USE;

                heap64kb_free(obj, splitHeader + 1);
            }
            next->size |= HEAP64KB_MASK_IN_USE;
            return ptr;
        }
        prev = next;
        next = (struct heap64kbFreeHeader_t *)((uintptr_t)obj + next->next);
    } while (prev->next);
    return 0;
}
