#ifndef MEMORY
#define MEMORY

#include "misc.h"

void * start_memory;
void * free_memory;
size_t memory_size = 0;
#define end_memory ((void *) (((byte*)start_memory)+memory_size))
#define free_memory_size (((byte*)end_memory)-((byte*)free_memory))

#define stalloc(size) ((void*) ((byte*)(free_memory = (void*) ((byte*) free_memory + size))-size))

#endif
