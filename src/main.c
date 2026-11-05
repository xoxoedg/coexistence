#include "memory.h"
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
#include "util.h"

#define TIMEOUT 1000000000
const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";


#define BLOCK_SIZE 512

void submit_command(struct nvme_submission_queue *sq, struct nvme_sq_entry entry) {
    sq->buffer[sq->tail] = entry;
    sq->tail = (sq->tail + 1) % sq->size;
    sq->doorbell[0] = sq->tail;
}

int poll_completion(struct nvme_completion_queue *cq, struct nvme_cq_entry *cq_entry) {
    uint64_t now = now_nano();
    while (get_phase_bit(cq) != cq->phase) {
        if(now_nano() - now > TIMEOUT) {
            return -1;        }
    }
    *cq_entry = cq->buffer[cq->head];
    cq->head += 1;
    if (cq->head == cq->size) {
        cq->phase = (cq->phase + 1) % 2;
        cq->head = 0;
    }
    cq->doorbell[0] = cq->head;
    return 0;
};
// Todo rewrite to api
int main() {
    int fd = open(bar0, O_RDWR);
    struct nvme_queue_pair queue_pair;
    struct nvme_cq_entry cq_entry;
    struct dma_buffer prp_buffer;
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

    uint16_t nlb = 0;
    if (create_dma_buffer((nlb + 1) * BLOCK_SIZE, &prp_buffer) == -1) {
        return 1;
    };
    int create_ret = create_queue_pair(&queue_pair, p, stride, nvme_fd);
    printf("Return value is %d \n", create_ret);

    if (create_ret == 0) {
        for (int i = 0; i < 1; i++) {

            struct nvme_sq_entry sq_entry
                = create_read_entry(0, 1, nlb, prp_buffer.physical_address);
            submit_command(&queue_pair.sq, sq_entry);
            if(poll_completion(&queue_pair.cq, &cq_entry) == -1) {
                fprintf(stderr, "timeout at i=%d\n", i);
                break;
            }

            printf("phys: 0x%lx\n", prp_buffer.physical_address);
            printf("data: %.21s\n", (char *)prp_buffer.virtual_address);
            struct nvme_status_field status_field = parse_status_field(cq_entry.dw3 >> 17);
            uint32_t cid = get_cid(cq_entry.dw3);
            if (cid != i) {
                fprintf(stderr, "Wrong cid (actual: %u | expected: %u)\n", cid, i);
                break;
            }
            uint32_t sqid = get_sqid(cq_entry.dw2);
            if (sqid != queue_pair.qid) {
                fprintf(stderr, "Wrong sqid (actual: %u | expected %u\n", sqid, queue_pair.qid);
                break;
            }
            if (status_field.sct != 0 || status_field.sc != 0) {
                fprintf(stderr, "I/O failed with sct: %u / sc: 0x%02x\n", status_field.sct,
                    status_field.sc);
                break;
            }

            log_cid(cq_entry.dw3);
            log_sqid(cq_entry.dw2);
        }

        release_buffer(prp_buffer.virtual_address, prp_buffer.size);
        int delete_ret = delete_queue_pair(nvme_fd, &queue_pair);
        printf("Return value is %d \n", delete_ret);

        return 0;
    }
}
