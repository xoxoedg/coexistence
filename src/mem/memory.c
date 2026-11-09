#include "memory.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stddef.h>


#define PAGE_SIZE 4096
#define PAGEMAP_ENTRY_SIZE 8
#define PFN_MASK ((1ULL << 55) - 1)

const char pagemap[] = "/proc/self/pagemap";

int virt_to_phys(void *virtual_address, uint64_t *physical_address) {
    int fd = open(pagemap, O_RDONLY);
    if (fd == -1) {
        perror("open pagemap failed ");
        return -1;
    }
    uintptr_t page_number = (uintptr_t)virtual_address / PAGE_SIZE;
    off_t pagemap_position = page_number * PAGEMAP_ENTRY_SIZE;
    uintptr_t page_offset = (uintptr_t)virtual_address % PAGE_SIZE;
    off_t file_offset = lseek(fd, pagemap_position, SEEK_SET);

    if (file_offset == -1) {
        perror("lseek failed");
        close(fd);
        return -1;
    }
    uint64_t entry_buffer;
    ssize_t bytes_read = read(fd, &entry_buffer, PAGEMAP_ENTRY_SIZE);

    if (bytes_read != PAGEMAP_ENTRY_SIZE) {
        perror("reading page map entry failed");
        close(fd);
        return -1;
    }
    uint64_t is_page_present = (entry_buffer >> 63) & 1;

    if (is_page_present == 0) {
        fprintf(stderr, "virt_to_phys: page not present (entry=0x%llx)\n",
            (unsigned long long)entry_buffer);
        close(fd);
        return -1;
    }
    uint64_t pfn = entry_buffer & PFN_MASK;
    *physical_address = pfn * PAGE_SIZE + page_offset;
    close(fd);
    return 0;
}


void *create_prp_buffer(size_t size) {
    void *prp_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (prp_buffer == MAP_FAILED) {
        perror("mmap  buffer");
        return NULL;
    }
    return prp_buffer;
}

void set_mem_buffer(void *prp_buffer, size_t size) {
    memset(prp_buffer, 0, size);
}

int lock_page(void *prp_buffer, size_t size) {
    int status = mlock(prp_buffer, size);
    if (status == -1) {
        perror("mlock queue buffer");
        return -1;
    }

    return status;
}

void release_buffer(void* buffer, size_t size) {
    munmap(buffer, size);
}

void *prepare_prp_buffer(size_t size) {
    void *prp_buffer = create_prp_buffer(size);

    if (prp_buffer == NULL) {
        return NULL;
    }

    set_mem_buffer(prp_buffer, size);
    if (lock_page(prp_buffer, size) == -1) {
        munmap(prp_buffer, size);
        return NULL;
    }
    return prp_buffer;
}

int create_dma_buffer(size_t size, struct dma_buffer *dma_buffer) {
    void* virtual_address = prepare_prp_buffer(size);
    uint64_t physical_address; 
    if (virtual_address == NULL) {
        return -1;
    }
    
    int ret = virt_to_phys(virtual_address, &physical_address); 

    if (ret == -1) {
        release_buffer(virtual_address, size);
        return -1;
    }

    dma_buffer->virtual_address = virtual_address;
    dma_buffer->physical_address = physical_address;
    dma_buffer->size = size;
    return 0;
}


void release_queue_buffer(void* cq_buffer, void* sq_buffer, size_t cq_size, size_t sq_size) {
    munmap(cq_buffer, cq_size);
    munmap(sq_buffer, sq_size);
}
