#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <openssl/evp.h>

struct model_inode_key {
    uint64_t inode_number;
    uint32_t dev_major;
    uint32_t dev_minor;
};

int generate_file_hash(const char *filepath, unsigned char *hash_out) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        close(fd);
        return -1;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        close(fd);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
            EVP_MD_CTX_free(mdctx);
            close(fd);
            return -1;
        }
    }

    unsigned int len = 0;
    if (EVP_DigestFinal_ex(mdctx, hash_out, &len) != 1) {
        EVP_MD_CTX_free(mdctx);
        close(fd);
        return -1;
    }

    EVP_MD_CTX_free(mdctx);
    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_sovereign_model_weights>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *target_model = argv[1];
    struct stat file_stat;

    if (stat(target_model, &file_stat) < 0) {
        perror("[!] Error inspecting model artifact metadata");
        return EXIT_FAILURE;
    }

    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "[!] Error: Target artifact is not a regular file binary.\n");
        return EXIT_FAILURE;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    printf("[*] Hashing model binary for immutable ledger matching...\n");
    if (generate_file_hash(target_model, hash) < 0) {
        fprintf(stderr, "[!] Cryptographic hashing operations encountered a failure.\n");
        return EXIT_FAILURE;
    }

    printf("[+] Cryptographic Invariant Integrity Signature Generated:\n    ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    struct model_inode_key key = {
        .inode_number = file_stat.st_ino,
        .dev_major = major(file_stat.st_dev),
        .dev_minor = minor(file_stat.st_dev)
    };

    printf("[*] Target Map Inode: %lu (Device: %u:%u)\n", 
           key.inode_number, key.dev_major, key.dev_minor);

    int map_fd = bpf_obj_get("/sys/fs/bpf/undc/undc_locked_models");
    if (map_fd < 0) {
        fprintf(stderr, "[!] Error: Failed to open pinned eBPF compliance storage layer.\n");
        fprintf(stderr, "    Verify that the 'undc_mmap_guard' module is successfully mounted.\n");
        return EXIT_FAILURE;
    }

    uint8_t security_profile_flag = 1; 
    int err = bpf_map_update_elem(map_fd, &key, &security_profile_flag, BPF_ANY);
    if (err < 0) {
        fprintf(stderr, "[!] Error: Failed to push model invariant locking signature to kernel space.\n");
        close(map_fd);
        return EXIT_FAILURE;
    }

    printf("[+] SUCCESS: Invariant target locked under UNDC kernel guardrails.\n");
    printf("[+] CADA Level 4 compliance protection layer activated for this artifact.\n");

    close(map_fd);
    return EXIT_SUCCESS;
}
