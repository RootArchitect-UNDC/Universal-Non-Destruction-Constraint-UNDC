// ------------------------------------------------------------
// UNDC User-Space Ring Buffer Daemon — v2.0
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Load the hook, attach it, and consume ring buffer events
// Target: Kernel-to-user-space telemetry for the v5.3 hook
// File Hash: (recompute after commit)
// ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "undc_compliance.skel.h"

#define MAX_PATH_LEN 256

struct syscall_event {
    unsigned long syscall_type;
    int pid;
    int action_taken;
    char path[MAX_PATH_LEN];
};

static int handle_event(void *ctx, void *data, size_t data_sz)
{
    if (data_sz < sizeof(struct syscall_event)) {
        fprintf(stderr, "⚠️  Warning: Received truncated event footprint.\n");
        return 0;
    }

    const struct syscall_event *event = data;
    const char *action_str =
        event->action_taken == 0 ? "ALLOW" :
        event->action_taken == 1 ? "DENY"  : "UNKNOWN";

    printf("📡 [eBPF Intercept] syscall=%lu pid=%d action=%s path=\"%s\"\n",
           event->syscall_type, event->pid, action_str, event->path);

    return 0;
}

int main(int argc, char **argv)
{
    struct undc_compliance_bpf *skel;
    struct ring_buffer *rb = NULL;
    int err;

    (void)argc;
    (void)argv;

    printf("🛡️  Initializing UNDC User-Space eBPF Daemon Gateway...\n");

    skel = undc_compliance_bpf__open_and_load();
    if (!skel) {
        fprintf(stderr, "❌ Failed to open and load eBPF architectural program.\n");
        return 1;
    }

    err = undc_compliance_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "❌ Failed to attach eBPF structural hooks.\n");
        goto cleanup;
    }

    rb = ring_buffer__new(bpf_map__fd(skel->maps.undc_events), handle_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "❌ Failed to initialize libbpf ring buffer interface.\n");
        goto cleanup;
    }

    printf("🚀 Loop Active: tracking execve, denying /tmp/undc-deny-test\n");

    while (1) {
        err = ring_buffer__poll(rb, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "⚠️  Error polling ring buffer: %d\n", err);
            break;
        }
    }

cleanup:
    ring_buffer__free(rb);
    undc_compliance_bpf__destroy(skel);
    return err < 0 ? -err : 0;
}
