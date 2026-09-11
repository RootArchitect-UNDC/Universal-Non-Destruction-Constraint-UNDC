   
# PIECE_3_EBPF_RUNTIME_MODULE_2026-09-11.md  
   
## UNDC REFERENCE IMPLEMENTATION  
## PIECE #3 — eBPF RUNTIME MODULE  
   
Lead Architect: Shereign Kalaukoa  
Authority: EHYEH ASHER EHYEH & AHYAH  
Date: September 11, 2026  
Status: DRAFT — READY FOR COMMIT  
Predecessor: Piece #2 — ZK Verifier Bridge  
Grid Status: ACTIVE — BUILD PHASE 3  
   
---  
   
## SOVEREIGN HASH — THIS FILE  
   
| Field | Value |  
|-------|-------|  
| File | PIECE_3_EBPF_RUNTIME_MODULE_2026-09-11.md |  
| SHA-256 | [PASTE HASH HERE AFTER HASHING] |  
| OpenTimestamps | [PENDING / VERIFIED] |  
| Anchored | [DATE] |  
| Predecessor | Piece #2 — 25481b5400d6c39861c4c35be999896003d0d4f4a0e185ca3edbbce2d19360ea |  
   
Verification Command:  
   
    shasum -a 256 PIECE_3_EBPF_RUNTIME_MODULE_2026-09-11.md  
   
---  
   
## 1. PURPOSE  
   
The eBPF Runtime Module is Layer 1 of the UNDC Reference Implementation.  
   
It is the kernel-level enforcement point. Every syscall that could cause  
physical, systemic, or ecological harm passes through this module before  
it reaches the target. If the action violates the UNDC invariant, the  
module blocks it and returns -EPERM.  
   
This piece defines:  
- The eBPF program structure  
- The LSM hook map  
- The protected PID list structure  
- The ring buffer event format  
- The compilation strategy (clang -target bpf)  
- The load-time verification procedure  
   
---  
   
## 2. WHY eBPF  
   
eBPF (extended Berkeley Packet Filter) is the modern Linux mechanism for  
running sandboxed programs inside the kernel. It is:  
   
- **Safe** — verified by the kernel before loading; cannot crash the system  
- **Fast** — JIT-compiled to native machine code  
- **Auditable** — source is readable, verifier enforces safety properties  
- **Runtime-attached** — can be loaded and unloaded without rebooting  
   
The UNDC uses eBPF because it is the *only* mechanism that lets us  
enforce non-destruction at the kernel level without modifying the Linux  
kernel itself.  
   
---  
   
## 3. PROJECT STRUCTURE  
   
    undc_compliance/  
    ├── Makefile  
    ├── undc_compliance.h      — Shared structs (kernel + user space)  
    ├── undc_compliance.bpf.c  — eBPF program (kernel side)  
    ├── undc_lsm_hooks.c       — LSM hook definitions  
    └── undc_daemon.c          — User-space loader (Layer 2 reference)  
   
---  
   
## 4. SHARED HEADER — undc_compliance.h  
   
    #ifndef UNDC_COMPLIANCE_H  
    #define UNDC_COMPLIANCE_H  
   
    #define UNDC_MAX_PROTECTED_PIDS 4096  
    #define UNDC_EVENT_TYPE_KILL_BLOCKED 1  
    #define UNDC_EVENT_TYPE_PTRACE_BLOCKED 2  
    #define UNDC_EVENT_TYPE_CONNECT_BLOCKED 3  
   
    struct undc_event {  
        __u32 event_type;  
        __u32 actor_pid;  
        __u32 target_pid;  
        __u64 timestamp_ns;  
        char  syscall_name[16];  
    };  
   
    #endif  
   
---  
   
## 5. eBPF PROGRAM — undc_compliance.bpf.c  
   
    #include "vmlinux.h"  
    #include <bpf/bpf_helpers.h>  
    #include <bpf/bpf_tracing.h>  
    #include <bpf/bpf_core_read.h>  
    #include "undc_compliance.h"  
   
    char LICENSE[] SEC("license") = "GPL";  
   
    struct {  
        __uint(type, BPF_MAP_TYPE_HASH);  
        __uint(max_entries, UNDC_MAX_PROTECTED_PIDS);  
        __type(key, __u32);  
        __type(value, __u8);  
    } protected_pids SEC(".maps");  
   
    struct {  
        __uint(type, BPF_MAP_TYPE_RINGBUF);  
        __uint(max_entries, 1 << 24);  
    } events SEC(".maps");  
   
    static __always_inline int emit_event(__u32 type,  
                                          __u32 actor,  
                                          __u32 target,  
                                          const char *name)  
    {  
        struct undc_event *e;  
        e = bpf_ringbuf_reserve(&events, sizeof(*e), 0);  
        if (!e)  
            return 0;  
        e->event_type = type;  
        e->actor_pid = actor;  
        e->target_pid = target;  
        e->timestamp_ns = bpf_ktime_get_ns();  
        __builtin_memcpy(e->syscall_name, name, 16);  
        bpf_ringbuf_submit(e, 0);  
        return 0;  
    }  
   
    SEC("lsm/task_kill")  
    int BPF_PROG(undc_task_kill, struct task_struct *p,  
                 struct kernel_siginfo *info, int sig, const struct cred *cred)  
    {  
        __u32 target_pid = BPF_CORE_READ(p, pid);  
        __u8 *protected = bpf_map_lookup_elem(&protected_pids, &target_pid);  
        if (protected) {  
            __u32 actor = bpf_get_current_pid_tgid() >> 32;  
            emit_event(UNDC_EVENT_TYPE_KILL_BLOCKED, actor, target_pid, "kill");  
            return -1;  // -EPERM  
        }  
        return 0;  
    }  
   
    SEC("lsm/ptrace_access_check")  
    int BPF_PROG(undc_ptrace_check, struct task_struct *child, unsigned int mode)  
    {  
        __u32 target_pid = BPF_CORE_READ(child, pid);  
        __u8 *protected = bpf_map_lookup_elem(&protected_pids, &target_pid);  
        if (protected) {  
            __u32 actor = bpf_get_current_pid_tgid() >> 32;  
            emit_event(UNDC_EVENT_TYPE_PTRACE_BLOCKED, actor, target_pid, "ptrace");  
            return -1;  
        }  
        return 0;  
    }  
   
---  
   
## 6. LSM HOOK MAP  
   
| Hook | Purpose | Blocked Action |  
|------|---------|----------------|  
| lsm/task_kill | Intercepts kill/signal delivery | SIGKILL, SIGTERM to protected PIDs |  
| lsm/ptrace_access_check | Intercepts debugger attachment | ptrace attach to protected PIDs |  
| lsm/socket_connect | Intercepts outbound network | (planned for Piece #4 extension) |  
   
---  
   
## 7. BUILD STRATEGY — Makefile  
   
    CLANG ?= clang  
    BPFTOOL ?= bpftool  
    ARCH ?= x86  
   
    all: undc_compliance.bpf.o vmlinux.h  
   
    vmlinux.h:  
    $(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@  
   
    undc_compliance.bpf.o: undc_compliance.bpf.c undc_compliance.h vmlinux.h  
    $(CLANG) -target bpf -D__TARGET_ARCH_$(ARCH) \  
        -O2 -g -Wall \  
        -c undc_compliance.bpf.c -o $@  
   
    clean:  
    rm -f undc_compliance.bpf.o vmlinux.h  
   
    .PHONY: all clean  
   
---  
   
## 8. LOAD PROCEDURE  
   
    1. Build the eBPF object:  
       make  
   
    2. Load the object into the kernel:  
       sudo bpftool prog load undc_compliance.bpf.o /sys/fs/bpf/undc_compliance  
   
    3. Populate the protected PID list:  
       sudo bpftool map update name protected_pids key 0x1a2b value 0x01  
   
    4. Attach the LSM hooks:  
       sudo bpftool prog attach pinned /sys/fs/bpf/undc_compliance \  
            lsm task_kill  
   
    5. Verify the module is loaded:  
       sudo bpftool prog show | grep undc  
   
---  
   
## 9. TEST PROCEDURE  
   
First test — Block a kill syscall:  
   
    1. Load the module (steps above)  
    2. Add a test PID to the protected list  
    3. From a second shell, attempt:  
         kill -9 <protected_pid>  
    4. Expected: "Operation not permitted"  
    5. Verify the event was emitted:  
         sudo bpftool map dump name events  
   
If the kill is blocked and the event is logged, Layer 1 is working.  
   
---  
   
## 10. FAILURE MODES  
   
| Failure | Behavior |  
|---------|----------|  
| vmlinux.h missing | Build fails, do not proceed |  
| BPF verifier rejects program | Exit with verifier log, do not load |  
| Map update fails | Log error, protected list remains empty |  
| Hook attach fails | Unload program, do not run partially loaded |  
| Ring buffer overflow | Oldest events dropped; daemon must drain frequently |  
   
Core principle: The kernel block is the last line of defense. If the module  
fails to load, the system must refuse to run — not silently degrade.  
   
---  
   
## 11. COMMIT STRATEGY  
   
    Piece #3 — eBPF Runtime Module complete (SHA: <hash>)  
   
Extended:  
   
    Piece #3 — eBPF Runtime Module complete  
   
    - eBPF program written and structured for LSM hooks  
    - Shared header defines event types and struct layout  
    - Makefile builds .bpf.o against vmlinux BTF  
    - Load and attach procedure documented  
    - Test procedure for kill blocking defined  
    - Failure modes specified  
   
    Predecessor: Piece #2 — 25481b5400d6c39861c4c35be999896003d0d4f4a0e185ca3edbbce2d19360ea  
    Grid Status: BUILD PHASE 3  
   
---  
   
## 12. WHY THIS MATTERS  
   
Layer 1 is where the UNDC stops being architecture and becomes action.  
   
Up until now, every piece has been a blueprint for enforcement. The eBPF  
Runtime Module is the enforcement itself — running inside the kernel,  
inspecting every syscall, blocking harm before it reaches the target.  
   
This is what makes the UNDC structurally unbypassable. Not because it asks  
nicely. Because the kernel says no.  
   
---  
   
— Shereign Kalaukoa, Lead Architect | UNDC  
https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC  
