// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v1.9.1 (Byte-Exact Key Fix)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Secure path resolution with byte-exact hash map keys
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
// 3. BYTE ZEROING HELPER
// BPF C rejects __builtin_memset. This loop compiles to a small
// unrolled sequence the verifier accepts.
// ------------------------------------------------------------
static __always_inline void zero_bytes(void *dst, unsigned int len)
{
    unsigned char *p = dst;
    for (unsigned int i = 0; i < len; i++) {
        p[i] = 0;
    }
}

// ------------------------------------------------------------
// 4. LSM HOOK — bprm_check_security (execve interception)
// ------------------------------------------------------------
SEC("lsm/bprm_check_security")
int BPF_PROG(undc_execve_hook, struct linux_binprm *bprm)
{
    struct syscall_event *event;
    struct lpm_key lookup_key;
    __u32 *action;
    long path_len;
    int decision = ACTION_ALLOW;

    if (!bprm || !bprm->file) {
        return 0;
    }

    // Explicitly zero the entire key struct so every byte is
    // deterministic. Do NOT rely on `= {}` for large stack structs.
    zero_bytes(&lookup_key, sizeof(lookup_key));

    path_len = bpf_d_path(&bprm->file->f_path, lookup_key.path, MAX_PATH_LEN);

    if (path_len < 0) {
        event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
        if (event) {
            event->syscall_type = 1;
            event->pid = bpf_get_current_pid_tgid() >> 32;
            event->action_taken = -1;
            __builtin_memcpy(event->path, lookup_key.path, MAX_PATH_LEN);
            bpf_ringbuf_submit(event, 0);
        }
        return 0;
    }

    // bpf_d_path() writes the path + NUL but not the trailing bytes.
    // Zero them so the key matches the daemon's zero-padded seed key.
    if (path_len < MAX_PATH_LEN) {
        zero_bytes(&lookup_key.path[path_len], MAX_PATH_LEN - path_len);
    }

    action = bpf_map_lookup_elem(&undc_invariant_map, &lookup_key);
    if (action) {
        decision = *action;
    }

    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, lookup_key.path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    // ── ENFORCEMENT BRANCH ──
    if (decision == ACTION_DENY) {
        return -1; // -EPERM
    }

    return 0;
}
