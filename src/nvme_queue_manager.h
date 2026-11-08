#ifndef NVME_QUEUE_MANAGER_H
#define NVME_QUEUE_MANAGER_H

#include "nvme_types.h"
#include <stdint.h>
#include <sys/types.h>
#include "util.h"


int create_queue_pair(struct nvme_queue_pair *queue_pair, volatile uint32_t *bar0, uint32_t stride, int nvme_fd, uint16_t qid);
int delete_queue_pair(int nvme_fd, struct nvme_queue_pair *queue_pair);
uint32_t get_phase_bit(struct nvme_completion_queue *cq); 
void submit_command(struct nvme_submission_queue *sq, struct nvme_sq_entry entry);
int poll_completion(struct nvme_completion_queue *cq, struct nvme_cq_entry *cq_entry);
  
#endif

