// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v2.6 (Clean-Room Key Isolation)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Byte-exact hash map keys via isolated stack copies
//          + Eliminates bpf_d_path buffer side-effects completely
// Status: ENFORCING — returns -EPERM on map hit with deny action
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
#define ACTION_AUDIT 2

// ------------------------------------------------------------
// 0. KEY STRUCTURE
// ------------------------------------------------------------
struct lpm_key {
    __u32 prefixlen;
    char path[MAX_PATH_LEN];
};

// ------------------------------------------------------------
// 1. MAP DEFINITIONS
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// 2. EVENT STRUCTURE
// ------------------------------------------------------------
struct syscall_event {
    unsigned long syscall_type;
    int pid;
    int action_taken;
    char path[MAX_PATH_LEN];
};

// ------------------------------------------------------------
// 3. LSM HOOK — bprm_check_security (execve interception)
// ------------------------------------------------------------
SEC("lsm/bprm_check_security")
int BPF_PROG(undc_execve_hook, struct linux_binprm *bprm)
{
    struct syscall_event *event;
    struct lpm_key clean_key = {}; // Pristine, 100% zero-initialized target key
    char tmp_path[MAX_PATH_LEN] = {}; // Isolated scratch buffer for d_path
    __u32 *action;
    long path_len;
    int decision = ACTION_ALLOW;
    int copy_done = 0;
    int i;

    if (!bprm || !bprm->file) {
        return 0;
    }

    /* 1. Resolve path into the isolated temporary scratch buffer */
    path_len = bpf_d_path((struct path *)&bprm->file->f_path, tmp_path, MAX_PATH_LEN);
    if (path_len < 0) {
        return 0;
    }

    /* 2. Isolated Extraction Loop: Copy ONLY valid string characters.
          Once the first null terminator is reached, remaining bytes stay pure 0x00. */
    clean_key.prefixlen = 0;
    copy_done = 0;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        if (tmp_path[i] == 0) {
            copy_done = 1;
        }
        
        if (!copy_done) {
            clean_key.path[i] = tmp_path[i];
        } else {
            clean_key.path[i] = 0;
        }
    }

    /* 3. Look up clean isolated key structure against policy database */
    action = bpf_map_lookup_elem(&undc_invariant_map, &clean_key);
    if (action) {
        decision = *action;
    }

    /* 4. Relay audited metrics down to user-space daemon */
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, clean_key.path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    /* 5. Enforce system layer block if rule evaluates to DENY */
    if (decision == ACTION_DENY) {
        return -1; // Yields -EPERM down to syscall boundary
    }

    return 0;
}
