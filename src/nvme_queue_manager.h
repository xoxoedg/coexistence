#ifndef NVME_QUEUE_MANAGER_H
#define NVME_QUEUE_MANAGER_H

#include "nvme_types.h"
#include <stdint.h>

int create_queue_pair(struct nvme_queue_pair *queue_pair, volatile uint32_t *bar0, uint32_t stride, int nvme_fd);
int delete_queue_pair(int nvme_fd, struct nvme_queue_pair *queue_pair);

#endif

