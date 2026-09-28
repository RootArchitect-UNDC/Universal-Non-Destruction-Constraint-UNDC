// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v2.4 (Null-Terminator Scanning Fix)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Byte-exact hash map keys via per-CPU scratch buffer
//          + Dynamic null-terminator scan to eradicate trailing junk bytes
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

struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(struct lpm_key));
    __uint(max_entries, 1);
} undc_scratch_key SEC(".maps");

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
    struct lpm_key *lookup_key;
    __u32 zero = 0;
    __u32 *action;
    long path_len;
    int decision = ACTION_ALLOW;
    int found_null = 0;
    int i;

    if (!bprm || !bprm->file) {
        return 0;
    }

    lookup_key = bpf_map_lookup_elem(&undc_scratch_key, &zero);
    if (!lookup_key) {
        return 0;
    }

    /* 1. Explicitly zero the structure to guarantee a clean baseline */
    lookup_key->prefixlen = 0;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        lookup_key->path[i] = 0;
    }

    /* 2. Resolve path directly into left-aligned buffer head */
    path_len = bpf_d_path((struct path *)&bprm->file->f_path, lookup_key->path, MAX_PATH_LEN);
    if (path_len < 0) {
        return 0;
    }

    /* 3. Bulletproof Sanitation: Find the first null terminator 
          and explicitly scrub everything after it to eliminate kernel noise. */
    found_null = 0;
    for (i = 0; i < MAX_PATH_LEN; i++) {
        if (found_null) {
            lookup_key->path[i] = 0;
        } else if (lookup_key->path[i] == 0) {
            found_null = 1;
        }
    }

    /* 4. Look up sanitized key structure against policy database */
    action = bpf_map_lookup_elem(&undc_invariant_map, lookup_key);
    if (action) {
        decision = *action;
    }

    /* 5. Relay audited metrics down to user-space daemon */
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, lookup_key->path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    /* 6. Enforce system layer block if rule evaluates to DENY */
    if (decision == ACTION_DENY) {
        return -1; // Yields -EPERM down to syscall boundary
    }

    return 0;
}
