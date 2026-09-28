// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v4.0 (Robust Suffix Matching)
// Lead Architect: Shereign Kalaukoa
// Purpose: Suffix matching to bypass WSL2 mount namespace prefixes
// Status: ENFORCING — returns -EPERM on target suffix match
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
    char tmp_path[MAX_PATH_LEN];
    int decision = ACTION_ALLOW;
    int i;

    if (!bprm || !bprm->file) {
        return 0;
    }

    __builtin_memset(tmp_path, 0, sizeof(tmp_path));

    // Resolve the internal kernel path
    long path_len = bpf_d_path((struct path *)&bprm->file->f_path, tmp_path, MAX_PATH_LEN);
    if (path_len < 0) {
        return 0;
    }

    // Target Suffix: "undc-deny-test" (14 characters)
    const char suffix[] = "undc-deny-test";
    
    // Find where the null terminator is to track the end of the string
    int str_len = 0;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        if (tmp_path[i] == 0) {
            str_len = i;
            break;
        }
    }

    // If the path is long enough, verify if it ends with our target suffix
    if (str_len >= 14) {
        int start_idx = str_len - 14;
        int match = 1;
        
        for (i = 0; i < 14; i++) {
            if (tmp_path[start_idx + i] != suffix[i]) {
                match = 0;
                break;
            }
        }
        
        if (match == 1) {
            decision = ACTION_DENY;
        }
    }

    // Send telemetry down to your daemon window
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, tmp_path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    if (decision == ACTION_DENY) {
        return -1; // Force-block execution with Permission Denied
    }

    return 0;
}
