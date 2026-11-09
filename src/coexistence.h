#ifndef COEXISTENCE_H
#define COEXISTENCE_H
#include <stddef.h>
#include <stdint.h>
#include "nvme_types.h"

struct coex_device {
    uint32_t vs;
    int bar0_fd;
    volatile uint32_t *bar0;
    int nvme_fd;
    uint32_t stride;
    struct nvme_queue_pair queue_pair;
    uint16_t cid;
};
int coex_init(struct coex_device *dev, uint16_t qid);
int coex_close(struct coex_device *dev);
int coex_read(struct coex_device *dev, uint64_t slba, uint16_t nlb, struct dma_buffer *buffer);

int coex_write(struct coex_device *dev, uint64_t slba, uint16_t nlb, struct dma_buffer *buffer);
int coex_alloc_buffer(size_t size, struct dma_buffer *buffer);
void coex_free_buffer(struct dma_buffer *buffer);

#endif
