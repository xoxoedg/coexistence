#include "nvme_io_commands.h"
#include "nvme_log.h"
#include "nvme_queue_manager.h"
#include "nvme_types.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";

void submit_command(struct nvme_submission_queue *sq, struct nvme_sq_entry entry) {
    sq->buffer[sq->tail] = entry;
    sq->tail = (sq->tail + 1) % sq->size;
    sq->doorbell[0] = sq->tail;
}

void poll_completion(struct nvme_completion_queue *cq, struct nvme_cq_entry *cq_entry) {
    while (get_phase_bit(cq) != cq->phase) { }
    *cq_entry = cq->buffer[cq->head];
    log_status_field((cq_entry->dw3) >> 17, "cq entry");
    cq->head += 1;
    if (cq->head == cq->size) {
        cq->phase = (cq->phase + 1) % 2;
        cq->head = 0;
    }
    cq->doorbell[0] = cq->head;
};

int main() {
    int fd = open(bar0, O_RDWR);
    struct nvme_queue_pair queue_pair;
    struct nvme_cq_entry cq_entry;
    if (fd == -1) {
        perror("opening bar0 failed");
        return -1;
    }

    volatile uint32_t *p = mmap(NULL, 16384, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }
    uint32_t vs = p[2];
    uint32_t cap = p[1];
    uint32_t dstrd = cap & 0xF;
    uint32_t stride = 1 << (2 + dstrd);
    printf("VS = 0x%08x\n", vs);
    printf("DSTRD = %u\n", dstrd);

    int nvme_fd = open("/dev/nvme0", O_RDONLY);

    if (nvme_fd == -1) {
        perror("open /dev/nvme0 failed");
        return 1;
    }
    int create_ret = create_queue_pair(&queue_pair, p, stride, nvme_fd);
    printf("Return value is %d \n", create_ret);

    if (create_ret == 0) {
        struct nvme_sq_entry sq_entry = create_sq_entry(0, 4660);
        submit_command(&queue_pair.sq, sq_entry);
        poll_completion(&queue_pair.cq, &cq_entry);

        log_cid(cq_entry.dw3);
        log_sqid(cq_entry.dw2);
    }

    if (create_ret == 0) {
        int delete_ret = delete_queue_pair(nvme_fd, &queue_pair);
        printf("Return value is %d \n", delete_ret);
    }

    return 0;
}
