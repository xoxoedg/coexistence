#include "nvme_queue_manager.h"
#include "memory.h"
#include "nvme_queue_commands.h"
#include "nvme_types.h"
#include <linux/nvme_ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <sys/mman.h>

#define DOORBELL_BASE 0x1000
#define CQ_QUEUE_BUFFER_SIZE 1024
#define SQ_QUEUE_BUFFER_SIZE 4096 
#define QUEUE_ID 5

int delete_cq_queue(int nvme_fd, uint32_t qid);

volatile uint32_t *calculate_doorbell(volatile uint32_t *bar0, uint32_t doorbell_index, uint32_t stride) {
    size_t byte_offset = DOORBELL_BASE + doorbell_index * stride; 
    return bar0 + byte_offset / 4;
    
}

volatile uint32_t *calculate_submission_queue_doorbell(volatile uint32_t *p, uint32_t queue_id, uint32_t stride) {
    return calculate_doorbell(p, 2 * queue_id, stride);
}

volatile uint32_t *calculate_completion_queue_doorbell(volatile uint32_t *p, uint32_t queue_id, uint32_t stride) {
    return calculate_doorbell(p, 2 * queue_id + 1, stride);
}
struct nvme_completion_queue initialize_cq(volatile uint32_t *bar0, uint32_t stride) {
    struct nvme_completion_queue cq;
    void* cq_prp_buffer = prepare_prp_buffer(CQ_QUEUE_BUFFER_SIZE );
    volatile uint32_t *doorbell = calculate_completion_queue_doorbell(bar0, QUEUE_ID, stride);

    cq.buffer = cq_prp_buffer;
    cq.doorbell = doorbell;
    cq.size = CQ_QUEUE_BUFFER_SIZE / CQ_ENTRY_SIZE;
    cq.head = 0;
    cq.phase = 1;

    return cq;
}
// Todo handle prepare_prp_buffer=null
struct nvme_submission_queue initialize_sq(volatile uint32_t *bar0, uint32_t stride) {
    struct nvme_submission_queue sq;
    struct nvme_sq_entry *sq_prp_buffer = prepare_prp_buffer(SQ_QUEUE_BUFFER_SIZE );
    volatile uint32_t *doorbell = calculate_submission_queue_doorbell(bar0, QUEUE_ID, stride);

    sq.buffer = sq_prp_buffer;
    sq.doorbell = doorbell;
    sq.size = SQ_QUEUE_BUFFER_SIZE / SQ_ENTRY_SIZE;
    sq.tail = 0;

    return sq;
}

struct nvme_queue_pair initialize_queue_pair(volatile uint32_t *bar0, uint32_t stride) {
    struct nvme_queue_pair queue_pair;
    queue_pair.qid = QUEUE_ID;
    queue_pair.sq = initialize_sq(bar0, stride);
    queue_pair.cq = initialize_cq(bar0, stride);

    return queue_pair;
}

int create_cq(struct nvme_queue_pair queue_pair, int nvme_fd) {
    volatile void *cq_prp_buffer = queue_pair.cq.buffer;
    struct nvme_passthru_cmd cq_cmd = create_cq_admin_command(cq_prp_buffer, CQ_QUEUE_BUFFER_SIZE, queue_pair.qid);
    int cq_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &cq_cmd);

    if (handle_nvme_result(cq_ret, "create cq") != 0) {
        munmap((void*)cq_prp_buffer, CQ_QUEUE_BUFFER_SIZE);
        return -1;
    }
    return 0;
}

int create_sq(struct nvme_queue_pair queue_pair, int nvme_fd) {
    volatile void* sq_prp_buffer = queue_pair.sq.buffer;
    struct nvme_passthru_cmd sq_cmd = create_sq_admin_command(sq_prp_buffer, SQ_QUEUE_BUFFER_SIZE, queue_pair.qid);
    int sq_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &sq_cmd);

    if (handle_nvme_result(sq_ret, "create sq") != 0) {
        delete_cq_queue(nvme_fd, queue_pair.qid);
        return -1;
    }
    return 0;
}

int create_queue_pair(struct nvme_queue_pair *queue_pair, volatile uint32_t *bar0, uint32_t stride, int nvme_fd) {
    *queue_pair = initialize_queue_pair(bar0, stride);
    int cq_ret = create_cq(*queue_pair, nvme_fd);
    
    if (cq_ret == -1) {
        release_queue_buffer((void*) queue_pair->cq.buffer, (void*) queue_pair->sq.buffer, CQ_QUEUE_BUFFER_SIZE, SQ_QUEUE_BUFFER_SIZE);
        return -1;
    }

    int sq_ret = create_sq(*queue_pair, nvme_fd);

    if (sq_ret == -1) {
        release_queue_buffer((void*) queue_pair->cq.buffer, (void*) queue_pair->sq.buffer, CQ_QUEUE_BUFFER_SIZE, SQ_QUEUE_BUFFER_SIZE);
        return -1 ;
    }
    return 0;
}


int delete_cq_queue(int nvme_fd, uint32_t qid) {

    struct nvme_passthru_cmd cq_delete_cmd = delete_cq_admin_command(qid);
    int cq_delete_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &cq_delete_cmd);

    if (handle_nvme_result(cq_delete_ret, "delete cq") != 0) {
        return -1;
        }
    return 0;
}

int delete_sq_queue(int nvme_fd, uint32_t qid) {
    struct nvme_passthru_cmd sq_delete_cmd = delete_sq_admin_command(qid);
    int sq_delete_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &sq_delete_cmd);

    if (handle_nvme_result(sq_delete_ret, "delete sq") != 0) {
        return -1;
    }
    return 0;
}

int delete_queue_pair(int nvme_fd, struct nvme_queue_pair *queue_pair) {
    // SQ deleten bevor CQ (4.1 Submission Queue & Completion Queue Definition)
    int sq_ret  = delete_sq_queue(nvme_fd, queue_pair->qid);

    int cq_ret = delete_cq_queue(nvme_fd, queue_pair->qid);
    // Todo separat freigeben: delete kann failen, trotzdem geben wir den Puffer frei
    release_queue_buffer((void*) queue_pair->cq.buffer, (void*) queue_pair->sq.buffer, CQ_QUEUE_BUFFER_SIZE, SQ_QUEUE_BUFFER_SIZE);

    if (sq_ret != 0 || cq_ret != 0) {
        return -1;
    }

    return 0;
}


