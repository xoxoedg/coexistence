#ifndef NVME_QUEUE_MANAGER_H
#define NVME_QUEUE_MANAGER_H

#include <stdint.h>
struct nvme_submission_queue {
    volatile uint32_t *doorbell;
    volatile void *buffer;
    uint32_t size;
    uint32_t tail;


};

struct nvme_completion_queue {
    volatile uint32_t *doorbell;
    volatile void *buffer;
    uint32_t size;
    uint32_t head;
    uint8_t phase;
};

struct nvme_queue_pair {
    uint32_t qid;
    struct nvme_submission_queue sq;
    struct nvme_completion_queue cq;
};

int create_queue_pair(struct nvme_queue_pair *queue_pair, volatile uint32_t *bar0, uint32_t stride, int nvme_fd);
int delete_queue_pair(int nvme_fd, struct nvme_queue_pair *queue_pair);

#endif

