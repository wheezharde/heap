#ifndef __heap64kb_h__
#define __heap64kb_h__

struct heap64kb_t * heap64kb_init(void * const, const size_t size);
void* heap64kb_alloc(struct heap64kb_t * const, const size_t size);
void heap64kb_free(struct heap64kb_t * const, void * const ptr);

#endif // __heap64kb_h__
