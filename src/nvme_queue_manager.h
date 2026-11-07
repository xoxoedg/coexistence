#ifndef NVME_QUEUE_MANAGER_H
#define NVME_QUEUE_MANAGER_H

#include "nvme_types.h"
#include <stdint.h>
#include <sys/types.h>

int create_queue_pair(struct nvme_queue_pair *queue_pair, volatile uint32_t *bar0, uint32_t stride, int nvme_fd, uint16_t qid);
int delete_queue_pair(int nvme_fd, struct nvme_queue_pair *queue_pair);
uint32_t get_phase_bit(struct nvme_completion_queue *cq); 

#endif

