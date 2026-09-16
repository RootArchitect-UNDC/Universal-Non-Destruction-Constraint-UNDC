// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v2.0 (Per-CPU Scratch Key)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Byte-exact hash map keys via per-CPU scratch buffer
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
// No packed attribute. Natural alignment. Both compilers agree
// on layout for { __u32; char[256]; } = 260 bytes.
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

// Per-CPU scratch slot for the lookup key. A single entry,
// key=0. Each CPU gets its own slot, so no locking needed.
// This avoids ever putting a 260-byte key on the 512-byte BPF
// stack, where clang's codegen has been observed to duplicate
// writes at misaligned offsets.
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

    // Get the per-CPU scratch key slot. It is zeroed once at map
    // creation; we clear only the path region each invocation to
    // avoid stale bytes from the previous exec.
    lookup_key = bpf_map_lookup_elem(&undc_scratch_key, &zero);
    if (!lookup_key) {
        return 0;
    }

    // Zero the path buffer. Loop is bounded by MAX_PATH_LEN (256),
    // which is small enough that clang unrolls it into straight
    // stores the verifier accepts.
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        lookup_key->path[i] = 0;
    }
    lookup_key->prefixlen = 0;

    path_len = bpf_d_path(&bprm->file->f_path, lookup_key->path, MAX_PATH_LEN);

    if (path_len < 0) {
        event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
        if (event) {
            event->syscall_type = 1;
            event->pid = bpf_get_current_pid_tgid() >> 32;
            event->action_taken = -1;
            __builtin_memcpy(event->path, lookup_key->path, MAX_PATH_LEN);
            bpf_ringbuf_submit(event, 0);
        }
        return 0;
    }

    action = bpf_map_lookup_elem(&undc_invariant_map, lookup_key);
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

    // ── ENFORCEMENT BRANCH ──
    if (decision == ACTION_DENY) {
        return -1; // -EPERM
    }

    return 0;
}
