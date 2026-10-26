#include "nvme_queue.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include "memory.h"
#include "nvme_queue_manager.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";


int main() {
    int fd = open(bar0, O_RDWR);
    struct nvme_queue_pair queue_pair;
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
        int delete_ret = delete_queue_pair(nvme_fd, &queue_pair);
        printf("Return value is %d \n", delete_ret);
    }

    return 0;
}
