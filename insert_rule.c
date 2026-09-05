// ------------------------------------------------------------
// UNDC Rule Inserter — v1.0
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Insert a path-based prefix rule into the LPM trie map
// File Hash: 20e04cca824e239d6b4f59af9dedaa2a353ffdaa400bf99cac2d3ff0be63b7ec
// ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "undc_compliance.skel.h" // Generated skeleton from BPF object

struct lpm_key {
    struct bpf_lpm_trie_key trie_key;
    char path[256];
};

int main(int argc, char **argv) {
    struct undc_compliance_bpf *skel;
    struct lpm_key key = {};
    __u32 value = 1; // 1 = blocked/destructive
    int map_fd, err;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_prefix>\n", argv[0]);
        fprintf(stderr, "Example: %s /tmp/\n", argv[0]);
        return 1;
    }

    // 1. Open and load the BPF skeleton
    skel = undc_compliance_bpf__open_and_load();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }

    // 2. Get the file descriptor for the LPM trie map
    map_fd = bpf_map__fd(skel->maps.undc_invariant_map);
    if (map_fd < 0) {
        fprintf(stderr, "Failed to get map FD: %d\n", map_fd);
        goto cleanup;
    }

    // 3. Prepare the LPM key
    //    prefixlen = strlen(path) * 8 (bits)
    //    This allows the trie to match any path under this prefix.
    size_t path_len = strlen(argv[1]);
    if (path_len > 255) {
        fprintf(stderr, "Path too long (max 255 chars)\n");
        goto cleanup;
    }
    key.trie_key.prefixlen = path_len * 8;
    strncpy(key.path, argv[1], sizeof(key.path) - 1);
    key.path[sizeof(key.path) - 1] = '\0';

    // 4. Insert the rule into the map
    err = bpf_map_update_elem(map_fd, &key, &value, BPF_ANY);
    if (err < 0) {
        fprintf(stderr, "Failed to insert rule: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("✅ Rule inserted: '%s' (prefixlen = %d bits)\n",
           argv[1], key.trie_key.prefixlen);

cleanup:
    undc_compliance_bpf__destroy(skel);
    return err;
}
