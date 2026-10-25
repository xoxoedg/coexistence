#include "nvme_queue.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>


#define SCT_MASK 0x7 /// 111
#define SCT_SHIFT 8
#define DNR_SHIFT 14
#define M_SHIFT 13
#define SC_MASK ((1UL << 8) - 1)
#define CQ_ENTRY_SIZE 16
#define SQ_ENTRY_SIZE 64
#define NVME_ADMIN_CREATE_CQ 0x05
#define NVME_ADMIN_DELETE_CQ 0x04
#define NVME_ADMIN_CREATE_SQ 0x01
#define NVME_ADMIN_DELETE_SQ 0x00


/* Bit  | Feld | Bedeutung
 * ------+------+---------------------------------------------------------
 *  0-7  | SC   | Status Code: welcher Fehler (Zeile in der Tabelle)
 *  8-10 | SCT  | Status Code Type: welche Tabelle
 *       |      |   0 = Generic Command Status (SC 0 = Erfolg)
 *       |      |   1 = Command Specific Status (z. B. Figure 55, 59)
 *       |      |   2 = Media and Data Integrity Errors
 *       |      |   7 = Vendor Specific
 * 11-12 | -    | reserviert (in 1.3)
 *  13   | M    | More: Zusatzinfos im Error Log
 *  14   | DNR  | Do Not Retry: erneuter Versuch sinnlos
 */

struct nvme_status parse_nvme_status(uint32_t state) {
    struct nvme_status nvme_error_state;

    nvme_error_state.sc = state & SC_MASK;
    nvme_error_state.sct = (state >> SCT_SHIFT) & SCT_MASK;
    nvme_error_state.dnr = (state >> DNR_SHIFT) & 0x1;
    nvme_error_state.m = (state >> M_SHIFT) & 0x1;

    return nvme_error_state;
}

int handle_nvme_result(int ret, const char *what) {
    if (ret < 0) {
        perror(what);
        return -1;
    } else if (ret > 0) {
        struct nvme_status status = parse_nvme_status((uint32_t)ret);
        fprintf(stderr, "%s failed: SCT=%u SC=0x%02x, M=%u, DNR=%u\n", what, status.sct, status.sc,
            status.m, status.dnr);
        return -1;
    } else {
        printf("%s ok\n", what);
        return 0;
    }
}

struct nvme_passthru_cmd create_cq_admin_command(void *prp_buffer, size_t size) {
    struct nvme_passthru_cmd cmd = { 0 };
    // 5.3 Create I/O Completion Queue command (NVME SPEC)
    uint32_t qid = 5;
    uint32_t qsize = (size / CQ_ENTRY_SIZE) - 1;
    uint32_t cdw10 = (qsize << 16) | qid;

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

struct nvme_passthru_cmd create_sq_admin_command(void *prp_buffer, size_t size) {
    struct nvme_passthru_cmd cmd = { 0 };
    // 5.4 Create I/O Submission Queue command
    uint32_t qid = 5;
    uint32_t qsize = (size / SQ_ENTRY_SIZE) - 1;
    uint32_t cdw10 = (qsize << 16) | qid;

    uint32_t cqid = 5;
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

struct nvme_passthru_cmd delete_cq_admin_command(void) {
    struct nvme_passthru_cmd cmd = { 0 };
    uint32_t qid = 5;

    cmd.opcode = NVME_ADMIN_DELETE_CQ;
    cmd.cdw10 = qid;

    return cmd;
}

struct nvme_passthru_cmd delete_sq_admin_command(void) {
    struct nvme_passthru_cmd cmd = { 0 };
    uint32_t qid = 5;

    cmd.opcode = NVME_ADMIN_DELETE_SQ;
    cmd.cdw10 = qid;

    return cmd;
}
