// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v2.5 (Stack Allocation Pivot)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Byte-exact hash map keys via direct stack layout
//          + Eliminates PERCPU map-to-map pointer aliasing bugs
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
    struct lpm_key key = {}; // Clean 260-byte key zeroed directly on the stack
    __u32 *action;
    long path_len;
    int decision = ACTION_ALLOW;
    int found_null = 0;
    int i;

    if (!bprm || !bprm->file) {
        return 0;
    }

    /* 1. Resolve path directly into left-aligned stack buffer */
    path_len = bpf_d_path((struct path *)&bprm->file->f_path, key.path, MAX_PATH_LEN);
    if (path_len < 0) {
        return 0;
    }

    /* 2. Find the first null terminator and scrub trailing noise on the stack */
    found_null = 0;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        if (found_null) {
            key.path[i] = 0;
        } else if (key.path[i] == 0) {
            found_null = 1;
        }
    }

    /* 3. Look up stack key structure against the policy database */
    action = bpf_map_lookup_elem(&undc_invariant_map, &key);
    if (action) {
        decision = *action;
    }

    /* 4. Relay audited metrics down to user-space daemon */
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, key.path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    /* 5. Enforce system layer block if rule evaluates to DENY */
    if (decision == ACTION_DENY) {
        return -1; // Yields -EPERM down to syscall boundary
    }

    return 0;
}
