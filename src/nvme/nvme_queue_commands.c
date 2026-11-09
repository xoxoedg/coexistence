#include "nvme_queue_commands.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>

#define NVME_ADMIN_CREATE_CQ 0x05
#define NVME_ADMIN_DELETE_CQ 0x04
#define NVME_ADMIN_CREATE_SQ 0x01
#define NVME_ADMIN_DELETE_SQ 0x00

struct nvme_passthru_cmd create_cq_admin_command(volatile void *prp_buffer, size_t size, uint16_t qid) {
    struct nvme_passthru_cmd cmd = { 0 };
    // 5.3 Create I/O Completion Queue command (NVME SPEC)
    uint32_t qsize = (size / CQ_ENTRY_SIZE) - 1;
    uint32_t cdw10 = (qsize << 16) | (uint32_t) qid;

    uint32_t iv = 0;
    uint32_t ien = 0;
    uint32_t pc = 1;

    uint32_t cdw11 = (iv << 16) | (ien << 1) | (pc << 0);

    cmd.opcode = NVME_ADMIN_CREATE_CQ;
    cmd.cdw10 = cdw10;
    cmd.cdw11 = cdw11;
    cmd.data_len = size;
    cmd.addr = (uint64_t)prp_buffer;

    return cmd;
}

struct nvme_passthru_cmd create_sq_admin_command(volatile void *prp_buffer, size_t size, uint16_t qid) {
    struct nvme_passthru_cmd cmd = { 0 };
    // 5.4 Create I/O Submission Queue command
    uint32_t qsize = (size / SQ_ENTRY_SIZE) - 1;
    uint32_t cdw10 = (qsize << 16) | (uint32_t) qid;

    uint32_t cqid = (uint32_t) qid;
    uint32_t qprio = 0;
    uint32_t pc = 1;

    uint32_t cdw11 = (cqid << 16) | (qprio << 1) | (pc << 0);

    cmd.opcode = NVME_ADMIN_CREATE_SQ;
    cmd.cdw10 = cdw10;
    cmd.cdw11 = cdw11;
    cmd.data_len = size;
    cmd.addr = (uint64_t)prp_buffer;

    return cmd;
}

struct nvme_passthru_cmd delete_cq_admin_command(uint16_t qid) {
    struct nvme_passthru_cmd cmd = { 0 };

    cmd.opcode = NVME_ADMIN_DELETE_CQ;
    cmd.cdw10 = (uint32_t) qid;

    return cmd;
}

struct nvme_passthru_cmd delete_sq_admin_command(uint16_t qid) {
    struct nvme_passthru_cmd cmd = { 0 };

    cmd.opcode = NVME_ADMIN_DELETE_SQ;
    cmd.cdw10 = (uint32_t) qid;

    return cmd;
}
