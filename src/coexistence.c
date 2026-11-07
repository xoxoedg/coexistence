#include "coexistence.h"
#include "memory.h"
#include "nvme_queue_manager.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define BAR0_SIZE 16384

const char bar0[] = "/sys/bus/pci/devices/0000:01:00.0/resource0";
int open_bar0() {
    int fd = open(bar0, O_RDWR);

    if (fd == -1) {
        perror("Failed to open bar0");
        return -1;
    }
    return fd;
}

void set_stride(struct coex_device *dev) {
    uint32_t cap = dev->bar0[1];
    uint32_t dstrd = cap & 0xF;
    uint32_t stride = 1 << (2 + dstrd);
    dev->stride = stride;
}

void set_version(struct coex_device *dev) {
    dev->vs = dev->bar0[2];
}
void set_fd_bar0(struct coex_device *dev, int fd) {
    dev->bar0_fd = fd;
}

int open_nvme_fd(char nvme_dev[]) {
    int nvme_fd = open(nvme_dev, O_RDONLY);

    if (nvme_fd == -1) {
        fprintf(stderr, "Failed to open dev: %s\n", nvme_dev);
        return -1;
    }
    return nvme_fd;
}

void set_nvme_fd(struct coex_device *dev, int fd) {
    dev->nvme_fd = fd;
}

int map_bar0(size_t size, int fd, struct coex_device *dev) {
    dev->bar0 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (dev->bar0 == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }
    return 0;
}

int setup_queue_pair(struct coex_device *dev, uint16_t qid) {
    return create_queue_pair(&dev->queue_pair, dev->bar0, dev->stride, dev->nvme_fd, qid);
}

int coex_init(struct coex_device *dev, uint16_t qid) {
    int fd_bar0 = open_bar0();
    if (fd_bar0 == -1) {
        return -1;
    }

    set_fd_bar0(dev, fd_bar0);

    int ret_map_bar0 = map_bar0(BAR0_SIZE, fd_bar0, dev);
    if (ret_map_bar0 == -1) {
        close(fd_bar0);
        return -1;
    }

    set_version(dev);
    set_stride(dev);

    int fd_nvme = open_nvme_fd("/dev/nvme0");
    if (fd_nvme == -1) {
        release_buffer((void *)dev->bar0, BAR0_SIZE);
        close(fd_bar0);
        return -1;
    }

    set_nvme_fd(dev, fd_nvme);
    int queue_pair_init_ret = setup_queue_pair(dev, qid);

    if (queue_pair_init_ret == -1) {
        release_buffer((void *)dev->bar0, BAR0_SIZE);
        close(fd_nvme);
        close(fd_bar0);
        fprintf(stderr, "Initializing Queue failed: %d", qid);
        return -1;
    }
    dev->cid = 0;

    return 0;
}

int coex_close(struct coex_device *dev) {
    int ret = delete_queue_pair(dev->nvme_fd, &dev->queue_pair);
    release_buffer((void *)dev->bar0, BAR0_SIZE);
    close(dev->bar0_fd);
    close(dev->nvme_fd);
    return ret;
}
