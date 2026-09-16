// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v2.1 (Printk Diagnostics)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Byte-exact hash map keys via per-CPU scratch buffer
//          + bpf_printk diagnostics at lookup time
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
    char path[MAX_PATH_LEN];
    __u32 prefixlen;
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

    if (!bprm || !bprm->file) {
        return 0;
    }

    lookup_key = bpf_map_lookup_elem(&undc_scratch_key, &zero);
    if (!lookup_key) {
        return 0;
    }

    for (int i = 0; i < MAX_PATH_LEN; i++) {
        lookup_key->path[i] = 0;
    }
    lookup_key->prefixlen = 0;

    path_len = bpf_d_path(&bprm->file->f_path, lookup_key->path, MAX_PATH_LEN);

    if (path_len < 0) {
        return 0;
    }

    // ── DIAGNOSTIC: print key state right before lookup ──
    bpf_printk("UNDC: pre-lookup path_len=%ld path=%.32s prefixlen=%u",
               path_len, lookup_key->path, lookup_key->prefixlen);

    action = bpf_map_lookup_elem(&undc_invariant_map, lookup_key);

    // ── DIAGNOSTIC: print lookup result ──
    bpf_printk("UNDC: post-lookup action_ptr=%px", action);

    if (action) {
        decision = *action;
    }

    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = decision;
        __builtin_memcpy(event->path, lookup_key->path, MAX_PATH_LEN);
        bpf_ringbuf_submit(event, 0);
    }

    if (decision == ACTION_DENY) {
        return -1;
    }

    return 0;
}
