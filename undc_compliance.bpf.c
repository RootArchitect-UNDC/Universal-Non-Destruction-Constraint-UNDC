// ------------------------------------------------------------
// UNDC eBPF Kernel Program — v1.1 (Observation-Only)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Intercept system calls and populate ring buffer
//          for user-space daemon consumption.
//
// NOTE: This version is OBSERVATION-ONLY. No enforcement is applied.
// All hooks return 0 (permit). Enforcement logic (returning -EPERM)
// will be added once the invariant map is verified against BTF.
// ------------------------------------------------------------

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "GPL";

// ------------------------------------------------------------
// 1. RING BUFFER — For sending syscall events to user space
// ------------------------------------------------------------
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024); // 256 KB ring buffer
} undc_events SEC(".maps");

// ------------------------------------------------------------
// 2. EVENT STRUCTURE
// ------------------------------------------------------------
struct syscall_event {
    unsigned long syscall_type;
    int pid;
};

// ------------------------------------------------------------
// 3. LSM HOOK — Intercept bprm_check_security (execve)
// ------------------------------------------------------------
SEC("lsm/bprm_check_security")
int BPF_PROG(undc_execve_hook, struct linux_binprm *bprm)
{
    struct syscall_event *event;
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (!event) {
        return 0; // Ring buffer full — drop event
    }

    event->syscall_type = 1; // execve
    event->pid = bpf_get_current_pid_tgid() >> 32; // Userspace PID

    bpf_ringbuf_submit(event, 0);

    // TODO: Add enforcement logic here once invariant map is verified.
    // Return -EPERM to block the action.
    return 0; // Currently observation-only (permit)
}

// ------------------------------------------------------------
// 4. LSM HOOK — Intercept security_mmap_file (mmap)
// ------------------------------------------------------------
SEC("lsm/file_mmap")
int BPF_PROG(undc_mmap_hook, struct file *file, unsigned long prot,
             unsigned long flags)
{
    struct syscall_event *event;
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (!event) {
        return 0; // Ring buffer full — drop event
    }

    event->syscall_type = 2; // mmap
    event->pid = bpf_get_current_pid_tgid() >> 32; // Userspace PID

    bpf_ringbuf_submit(event, 0);

    // TODO: Add enforcement logic here once invariant map is verified.
    // Return -EPERM to block the action.
    return 0; // Currently observation-only (permit)
}

// ------------------------------------------------------------
// 5. LSM HOOK — Intercept socket_connect (network egress)
// ------------------------------------------------------------
SEC("lsm/socket_connect")
int BPF_PROG(undc_connect_hook, struct socket *sock, struct sockaddr *address,
             int addrlen)
{
    struct syscall_event *event;
    event = bpf_ringbuf_reserve(&undc_events, sizeof(struct syscall_event), 0);
    if (!event) {
        return 0; // Ring buffer full — drop event
    }

    event->syscall_type = 3; // connect
    event->pid = bpf_get_current_pid_tgid() >> 32; // Userspace PID

    bpf_ringbuf_submit(event, 0);

    // TODO: Add enforcement logic here once invariant map is verified.
    // Return -EPERM to block the action.
    return 0; // Currently observation-only (permit)
}
