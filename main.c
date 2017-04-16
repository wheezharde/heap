#include <stdint.h>
#include <stdio.h>

#include "heap64kb.h"

static uint8_t heapBlock[ 64 * 1024 ];

int main( int argc, char ** argv ) {
    void * ptr;

    struct heap64kb_t * const heap = heap64kb_init( heapBlock, sizeof( heapBlock ) );

    (void)argc;
    (void)argv;

    ptr = heap64kb_alloc( heap, 64 );
    heap64kb_free( heap, ptr );
    ptr = heap64kb_alloc( heap, 128 );
    heap64kb_free( heap, ptr );
    ptr = heap64kb_alloc( heap, 256 );
    heap64kb_free( heap, ptr );
}
