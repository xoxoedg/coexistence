#include "nvme_queue.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#define DOORBELL_BASE 0x1000

const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";

volatile uint32_t *calculate_submission_queue_doorbell(volatile uint32_t *p, uint32_t queue_id) {
    uint32_t userspace_doorbell_offset = 2 * queue_id;
    return (p + DOORBELL_BASE / 4) + userspace_doorbell_offset;
}

volatile uint32_t *calculate_completion_queue_doorbell(volatile uint32_t *p, uint32_t queue_id) {
    return calculate_submission_queue_doorbell(p, queue_id) + 1;
}

int main() {
    int fd = open(bar0, O_RDWR);

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

    void *physical = prepare_prp_buffer();
    uint64_t physical_address = virt_to_phys(physical);

    printf("virtual:  %p\n", physical);
    printf("physical: 0x%llx\n", (unsigned long long)physical_address);

    return 0;
}
