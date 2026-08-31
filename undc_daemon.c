// ------------------------------------------------------------
// UNDC User-Space Bridge Daemon — v1.1 (eBPF Ring Buffer)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Poll the eBPF ring buffer map to process safe system calls
// ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "undc_compliance.skel.h" // Skeleton generated from compiler toolchain

struct syscall_event {
    unsigned long syscall_type;
    int pid;
};

// Callback triggered whenever the eBPF hook posts a syscall footprint to the map
static int handle_event(void *ctx, void *data, size_t data_sz) {
    if (data_sz < sizeof(struct syscall_event)) {
        fprintf(stderr, "⚠️  Warning: Received truncated event footprint.\n");
        return 0;
    }

    const struct syscall_event *event = data;
    printf("📡 [eBPF Intercept] Syscall Type: %lu executed by PID: %d\n", 
           event->syscall_type, event->pid);

    // TODO: Ingest event->syscall_type, look up its cryptographic salt,
    // refresh the build/input.json file, and invoke the proving toolchain.
    return 0;
}

int main(int argc, char **argv) {
    struct undc_compliance_bpf *skel;
    struct ring_buffer *rb = NULL;
    int err;

    printf("🛡️  Initializing UNDC User-Space eBPF Daemon Gateway...\n");

    // 1. Open and load the eBPF application skeleton
    skel = undc_compliance_bpf__open_and_load();
    if (!skel) {
        fprintf(stderr, "❌ Failed to open and load eBPF architectural program.\n");
        return 1;
    }

    // 2. Attach the LSM / tracepoint hooks to the running kernel state
    err = undc_compliance_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "❌ Failed to attach eBPF structural hooks.\n");
        goto cleanup;
    }

    // 3. Initialize the high-performance user-space Ring Buffer consumer map
    // "undc_events" matches the defined map name inside the BPF kernel code
    rb = ring_buffer__new(bpf_map__fd(skel->maps.undc_events), handle_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "❌ Failed to initialize libbpf ring buffer interface.\n");
        goto cleanup;
    }

    printf("🚀 Loop Active: Continuous structural tracking initialized successfully.\n");

    // 4. Infinite Polling Loop
    while (1) {
        err = ring_buffer__poll(rb, 100 /* timeout in ms */);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "⚠️  Error polling architectural ring buffer: %d\n", err);
            break;
        }
    }

cleanup:
    ring_buffer__free(rb);
    undc_compliance_bpf__destroy(skel);
    return err < 0 ? -err : 0;
}
