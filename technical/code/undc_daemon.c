// ------------------------------------------------------------
// UNDC User-Space Ring Buffer Daemon — v1.5 (Key Dump Diagnostic)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Consume eBPF events with key hexdump; seed hash map policy
// Target: Kernel-to-user-space telemetry pipeline with demo policy
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
#define ACTION_DENY  1

struct lpm_key {
    __u32 prefixlen;
    char path[MAX_PATH_LEN];
} __attribute__((packed));

struct syscall_event {
    unsigned long syscall_type;
    int pid;
    int action_taken;
    char path[MAX_PATH_LEN];
    unsigned char key_dump[sizeof(struct lpm_key)];
};

static void hexdump(const char *label, const unsigned char *data, size_t len)
{
    printf("   %s (%zu bytes):\n   ", label, len);
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n   ");
        }
    }
    printf("\n");
}

static int seed_policy(struct undc_compliance_bpf *skel, const char *deny_path)
{
    int map_fd = bpf_map__fd(skel->maps.undc_invariant_map);
    struct lpm_key key = {};
    __u32 action = ACTION_DENY;

    size_t plen = strlen(deny_path);
    if (plen >= MAX_PATH_LEN) {
        fprintf(stderr, "❌ Deny path too long (max %d): %s\n", MAX_PATH_LEN - 1, deny_path);
        return -1;
    }

    memcpy(key.path, deny_path, plen);

    int err = bpf_map_update_elem(map_fd, &key, &action, BPF_ANY);
    if (err) {
        fprintf(stderr, "❌ Failed to insert deny policy for %s: %s\n",
                deny_path, strerror(errno));
        return err;
    }

    printf("🛡️  Policy seeded: %s → DENY\n", deny_path);
    hexdump("SEED key", (const unsigned char *)&key, sizeof(key));
    return 0;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
    if (data_sz < sizeof(struct syscall_event)) {
        fprintf(stderr, "⚠️  Warning: Received truncated event footprint.\n");
        return 0;
    }

    const struct syscall_event *event = data;
    const char *action_str =
        event->action_taken == 0 ? "ALLOW" :
        event->action_taken == 1 ? "DENY"  :
        event->action_taken == 2 ? "AUDIT" : "UNKNOWN";

    printf("📡 [eBPF Intercept] syscall=%lu pid=%d action=%s path=\"%s\"\n",
           event->syscall_type, event->pid, action_str, event->path);

    /* Dump the hook's key for every event where the path looks
     * like a deny candidate. Full dump on deny-looking events. */
    if (strstr(event->path, "undc-deny-test")) {
        hexdump("HOOK key", event->key_dump, sizeof(event->key_dump));
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct undc_compliance_bpf *skel;
    struct ring_buffer *rb = NULL;
    int err;

    const char *deny_path = (argc > 1) ? argv[1] : "/usr/bin/undc-test-deny";

    printf("🛡️  Initializing UNDC User-Space eBPF Daemon Gateway...\n");
    printf("🛡️  sizeof(struct lpm_key) = %zu\n", sizeof(struct lpm_key));

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

    err = seed_policy(skel, deny_path);
    if (err) {
        goto cleanup;
    }

    rb = ring_buffer__new(bpf_map__fd(skel->maps.undc_events), handle_event, NULL, NULL);
    if (!rb) {
        fprintf(stderr, "❌ Failed to initialize libbpf ring buffer interface.\n");
        goto cleanup;
    }

    printf("🚀 Loop Active: tracking execve, denying %s\n", deny_path);

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
