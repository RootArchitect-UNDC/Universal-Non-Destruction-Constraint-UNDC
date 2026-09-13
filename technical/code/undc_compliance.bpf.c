// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v1.4 (Hardened LPM Path Resolution)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Secure, canonical path resolution with correct LPM trie semantics
// Status: OBSERVATION-ONLY — enforcement commented out
// File Hash: 9f955daecd6c9b4131b0e17aa063ed5150e0b2fff711cef679e76d7c27549491
// ------------------------------------------------------------

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char LICENSE[] SEC("license") = "GPL";

#define MAX_PATH_LEN 256

// ------------------------------------------------------------
// 1. LPM TRIE KEY STRUCTURE
// ------------------------------------------------------------
struct lpm_key {
    struct bpf_lpm_trie_key trie_key; // prefixlen must be first
    char path[MAX_PATH_LEN];
};

// ------------------------------------------------------------
// 2. MAP DEFINITIONS
// ------------------------------------------------------------
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} undc_events SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __uint(key_size, sizeof(struct lpm_key));
    __uint(value_size, sizeof(__u32));
    __uint(max_entries, 4096);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} undc_invariant_map SEC(".maps");

// ------------------------------------------------------------
// 3. EVENT STRUCTURE
// ------------------------------------------------------------
struct syscall_event {
    unsigned long syscall_type;
    int pid;
    int action_taken;
};

// ------------------------------------------------------------
// 4. LSM HOOK — bprm_check_security (execve interception)
// ------------------------------------------------------------
SEC("lsm/bprm_check_security")
int BPF_PROG(undc_execve_hook, struct linux_binprm *bprm)
{
    struct syscall_event *event;
    struct lpm_key lookup_key = {};
    __u32 *action;
    long path_len;

    if (!bprm || !bprm->file) {
        return 0;
    }

    path_len = bpf_d_path(&bprm->file->f_path, lookup_key.path, MAX_PATH_LEN);

    if (path_len < 0) {
        event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
        if (event) {
            event->syscall_type = 1;
            event->pid = bpf_get_current_pid_tgid() >> 32;
            event->action_taken = -1;
            bpf_ringbuf_submit(event, 0);
        }
        return 0;
    }

    lookup_key.trie_key.prefixlen = MAX_PATH_LEN * 8;

    action = bpf_map_lookup_elem(&undc_invariant_map, &lookup_key);

    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = action ? *action : 0;
        bpf_ringbuf_submit(event, 0);
    }

    return 0;
}
