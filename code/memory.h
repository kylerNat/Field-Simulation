#ifndef MEMORY
#define MEMORY

#include "misc.h"

void * start_memory;
void * free_memory;
size_t memory_size = 0;
#define end_memory ((void *) (((byte*)free_memory)+memory_size))
void * stalloc(size_t size)
{
    auto temp = free_memory;
    free_memory = (void *) ((byte *) free_memory + size);
    return temp;
}

#endif
