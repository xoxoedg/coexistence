#include "nvme_queue.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include "memory.h"
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#define DOORBELL_BASE 0x1000
#define CQ_QUEUE_BUFFER_SIZE 1024
#define SQ_QUEUE_BUFFER_SIZE 4096 
const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";



volatile uint32_t *calculate_submission_queue_doorbell(volatile uint32_t *p, uint32_t queue_id) {
    uint32_t userspace_doorbell_offset = 2 * queue_id;
    return (p + DOORBELL_BASE / 4) + userspace_doorbell_offset;
}

volatile uint32_t *calculate_completion_queue_doorbell(volatile uint32_t *p, uint32_t queue_id) {
    return calculate_submission_queue_doorbell(p, queue_id) + 1;
}

int delete_cq_queue(int nvme_fd) {

    struct nvme_passthru_cmd cq_delete_cmd = delete_cq_admin_command();
    int cq_delete_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &cq_delete_cmd);

    if (handle_nvme_result(cq_delete_ret, "delete cq") != 0) {
        return -1;
        }
    return 0;
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

    void *physical = prepare_prp_buffer(1024);
    uint64_t physical_address = virt_to_phys(physical);

    printf("virtual:  %p\n", physical);
    printf("physical: 0x%llx\n", (unsigned long long)physical_address);


    int nvme_fd = open("/dev/nvme0", O_RDONLY);

    if (nvme_fd == -1) {
        perror("open /dev/nvme0 failed");
        return 1;
    }
    void* cq_prp_buffer = prepare_prp_buffer(CQ_QUEUE_BUFFER_SIZE );
    struct nvme_passthru_cmd cq_cmd = create_cq_admin_command(cq_prp_buffer, CQ_QUEUE_BUFFER_SIZE);


    void* sq_prp_buffer = prepare_prp_buffer(SQ_QUEUE_BUFFER_SIZE );
    struct nvme_passthru_cmd sq_cmd = create_sq_admin_command(sq_prp_buffer, SQ_QUEUE_BUFFER_SIZE);

    int cq_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &cq_cmd);

    if (handle_nvme_result(cq_ret, "create cq") != 0) {
        return -1;
    }

    int sq_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &sq_cmd);

    if (handle_nvme_result(sq_ret, "create sq") != 0) {
        delete_cq_queue(nvme_fd);
        return -1;
    }

    struct nvme_passthru_cmd sq_delete_cmd = delete_sq_admin_command();

    int sq_delete_ret = ioctl(nvme_fd, NVME_IOCTL_ADMIN_CMD, &sq_delete_cmd);

    if (handle_nvme_result(sq_delete_ret, "delete sq") != 0) {
        return -1;
    }

    if (delete_cq_queue(nvme_fd) != 0) return 1;
    return 0;
}
