#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";

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
    printf("VS = 0x%08x\n", vs);
    printf("DSTRD = %u\n", dstrd);
    return 0;
}
