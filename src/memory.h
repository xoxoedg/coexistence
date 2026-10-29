#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

int virt_to_phys(void *virtual_addr, uint64_t *physical_address); 
void *prepare_prp_buffer(size_t size);
void release_queue_buffer(void* cq_buffer, void* sq_buffer, size_t cq_size, size_t sq_size);
struct dma_buffer create_dma_buffer(size_t size);

#endif
