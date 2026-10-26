#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

uint64_t virt_to_phys(void *virtual_addr); 
void *prepare_prp_buffer(size_t size);
void release_queue_buffer(void* cq_buffer, void* sq_buffer, size_t cq_size, size_t sq_size);

#endif
