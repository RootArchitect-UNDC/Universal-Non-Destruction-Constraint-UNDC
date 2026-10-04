// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v5.3 (.rodata string evaluation)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Intercept direct string from bprm->filename
// Status: ENFORCING — returns -EPERM on target match
// File Hash: (recompute after commit)
// ------------------------------------------------------------

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char LICENSE[] SEC("license") = "GPL";

#define MAX_PATH_LEN 256
#define ACTION_ALLOW 0
#define ACTION_DENY  1

/* The compiler places this in .rodata, which libbpf exposes as
 * a read-only map. This satisfies bpf_strncmp's arg3 requirement
 * (ARG_PTR_TO_CONST_STR). */
const char TARGET_PATH[] = "/tmp/undc-deny-test";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} undc_events SEC(".maps");

struct syscall_event {
    unsigned long syscall_type;
    int pid;
    int action_taken;
    char path[MAX_PATH_LEN];
};

SEC("lsm/bprm_check_security")
int BPF_PROG(undc_execve_hook, struct linux_binprm *bprm)
{
    struct syscall_event *event;
    char k_filename[MAX_PATH_LEN];
    int decision = ACTION_ALLOW;

    if (!bprm) {
        return 0;
    }

    __builtin_memset(k_filename, 0, sizeof(k_filename));

    long ret = bpf_probe_read_kernel_str(k_filename, sizeof(k_filename), bprm->filename);
    if (ret < 0) {
        return 0;
    }

    if (bpf_strncmp(k_filename, sizeof(TARGET_PATH), TARGET_PATH) == 0) {
        decision = ACTION_DENY;
    }

    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, k_filename, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    if (decision == ACTION_DENY) {
        return -1;
    }

    return 0;
}
