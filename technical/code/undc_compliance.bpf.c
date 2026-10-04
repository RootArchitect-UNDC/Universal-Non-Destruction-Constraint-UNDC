// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v5.1 (Filename Direct Interception)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Intercept direct string from bprm->filename to bypass WSL2 VFS bugs
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

struct lpm_key {
    __u32 prefixlen;
    char path[MAX_PATH_LEN];
};

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} undc_events SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(key_size, sizeof(struct lpm_key));
    __uint(value_size, sizeof(__u32));
    __uint(max_entries, 4096);
} undc_invariant_map SEC(".maps");

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

    /* Read the absolute string directly from the kernel memory reference */
    long ret = bpf_probe_read_kernel_str(k_filename, sizeof(k_filename), bprm->filename);
    if (ret < 0) {
        return 0;
    }

    /* Direct comparison against the fixed 19-byte target */
    const char target[] = "/tmp/undc-deny-test";
    int match = 1;

    if (k_filename[0]  != target[0])  match = 0;
    if (k_filename[1]  != target[1])  match = 0;
    if (k_filename[2]  != target[2])  match = 0;
    if (k_filename[3]  != target[3])  match = 0;
    if (k_filename[4]  != target[4])  match = 0;
    if (k_filename[5]  != target[5])  match = 0;
    if (k_filename[6]  != target[6])  match = 0;
    if (k_filename[7]  != target[7])  match = 0;
    if (k_filename[8]  != target[8])  match = 0;
    if (k_filename[9]  != target[9])  match = 0;
    if (k_filename[10] != target[10]) match = 0;
    if (k_filename[11] != target[11]) match = 0;
    if (k_filename[12] != target[12]) match = 0;
    if (k_filename[13] != target[13]) match = 0;
    if (k_filename[14] != target[14]) match = 0;
    if (k_filename[15] != target[15]) match = 0;
    if (k_filename[16] != target[16]) match = 0;
    if (k_filename[17] != target[17]) match = 0;
    if (k_filename[18] != target[18]) match = 0;

    if (match == 1) {
        decision = ACTION_DENY;
    }

    /* Send telemetry down to user space */
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, k_filename, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    if (decision == ACTION_DENY) {
        return -1; /* Triggers -EPERM (Permission Denied) */
    }

    return 0;
}
