// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v1.4 (Hardened LPM Path Resolution)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Secure, canonical path resolution with correct LPM trie semantics
// Status: OBSERVATION-ONLY — enforcement commented out
// File Hash: 9f955daecd6c9b4131b0e17aa063ed5150e0b2fff711cef679e76d7c27549491
// ------------------------------------------------------------

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <linux/binfmts.h>
#include <linux/errno.h>

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
    int action_taken; // 0 = allowed, 1 = blocked, -1 = resolution failure
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

    // 1. Resolve canonical VFS path — protects against symlinks and relative paths
    //    WARNING: bpf_d_path availability depends on kernel version and LSM hook type.
    //    Test with bpftool on target kernel before relying on it.
    path_len = bpf_d_path(&bprm->file->f_path, lookup_key.path, MAX_PATH_LEN);

    // 2. Handle path resolution failure
    //    CURRENTLY OBSERVATION-ONLY: returns 0 (permit) even on failure.
    //    To enable fail-closed behavior, uncomment the -EPERM line below.
    if (path_len < 0) {
        event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
        if (event) {
            event->syscall_type = 1;
            event->pid = bpf_get_current_pid_tgid() >> 32;
            event->action_taken = -1; // Resolution failure
            bpf_ringbuf_submit(event, 0);
        }
        // return -EPERM; // Uncomment for fail-closed enforcement
        return 0;
    }

    // 3. LPM TRIE LOOKUP — CRITICAL: prefixlen must be set to the maximum size
    //    This allows the kernel's trie algorithm to correctly match any stored prefix
    //    (from short /usr/ prefixes up to exact full paths).
    //    Do NOT set this to the actual path length — that breaks LPM semantics.
    lookup_key.trie_key.prefixlen = MAX_PATH_LEN * 8;

    // 4. Query the Invariant Map
    action = bpf_map_lookup_elem(&undc_invariant_map, &lookup_key);

    // 5. Submit event to user space
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (event) {
        event->syscall_type = 1;
        event->pid = bpf_get_current_pid_tgid() >> 32;
        event->action_taken = action ? *action : 0;
        bpf_ringbuf_submit(event, 0);
    }

    // 6. ENFORCEMENT (CURRENTLY DISABLED)
    //    To activate enforcement, uncomment the line below.
    //    This will block execution if the path is marked as destructive (value == 1).
    // if (action && *action == 1) return -EPERM;

    return 0;
}
