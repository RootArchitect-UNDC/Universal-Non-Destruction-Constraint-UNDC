#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

struct violation_event {
    uint64_t inode_number;
    uint32_t pid;
    uint32_t uid;
    uint32_t prot_flags;
    char comm[16];
};

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

static int handle_violation_event(void *ctx, void *data, size_t data_sz) {
    const struct violation_event *e = data;
    
    if (data_sz < sizeof(*e)) {
        fprintf(stderr, "[!] Malformed telemetry packet dropped. Size discrepancy.\n");
        return 0;
    }

    printf("\n================ [UNDC COMPLIANCE INTRUSION] ================\n");
    printf("[!] ALERT: Unauthorized mutation attempt intercepted.\n");
    printf("[>] Process Comm:  %s (PID: %u)\n", e->comm, e->pid);
    printf("[>] Operator UID:  %u\n", e->uid);
    printf("[>] Target Inode:  %lu\n", e->inode_number);
    printf("[>] Memory Flags:  0x%x (PROT_WRITE detected)\n", e->prot_flags);
    
    printf("[*] Status: Enforced Inline Isolation. Hard Return -EPERM.\n");
    printf("============================================================\n");
    
    return 0;
}

int main(int argc, char **argv) {
    struct ring_buffer *ring_buf = NULL;
    int telemetry_map_fd;
    int err;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    telemetry_map_fd = bpf_obj_get("/sys/fs/bpf/undc/undc_telemetry");
    if (telemetry_map_fd < 0) {
        fprintf(stderr, "[!] Error: Failed to acquire pinned telemetry ring buffer channel.\n");
        fprintf(stderr, "    Ensure 'undc_mmap_guard' eBPF container layer is mounted and pinned.\n");
        return EXIT_FAILURE;
    }

    ring_buf = ring_buffer__new(telemetry_map_fd, handle_violation_event, NULL, NULL);
    if (!ring_buf) {
        fprintf(stderr, "[!] Error: Initialization of user-space Ring Buffer consumer failed.\n");
        close(telemetry_map_fd);
        return EXIT_FAILURE;
    }

    printf("[*] UNDC User-space Telemetry Daemon active.\n");
    printf("[*] Listening on pinned eBPF channel for real-time model protection logs...\n");

    while (!exiting) {
        err = ring_buffer__poll(ring_buf, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "[!] Error occurred during ring buffer polling processing: %d\n", err);
            break;
        }
    }

    printf("\n[*] Stopping telemetry loop. Releasing resources.\n");
    ring_buffer__free(ring_buf);
    close(telemetry_map_fd);

    return EXIT_SUCCESS;
}
