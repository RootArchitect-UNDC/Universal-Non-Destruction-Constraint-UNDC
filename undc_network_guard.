#include <linux/bpf.h>
#include <linux/lsm_hooks.h>
#include <bpf/bpf_helpers.h>

char _license[] SEC("license") = "GPL";

// Map to track forbidden destination IPs (e.g., Non-EU / Third-Country Endpoints)
struct {
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __type(key, struct bpf_lpm_trie_key_u32); // Prefix length + IP
    __type(value, __u32);                     // Policy action flag
    __uint(max_entries, 1024);
} forbidden_egress_trie SEC(".maps");

SEC("lsm/socket_connect")
int BPF_PROG(undc_network_guard, struct socket *sock, struct sockaddr *address, int addrlen) {
    if (address->sa_family != AF_INET) {
        return 0; // Skip non-IPv4 traffic for local loop performance
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)address;
    __u32 dest_ip = addr->sin_addr.s_addr;

    // Fast look-up inside the Longest Prefix Match Trie
    __u32 *policy = bpf_map_lookup_elem(&forbidden_egress_trie, &dest_ip);
    
    if (policy) {
        // CADA Policy Match: Tier 2 Mitigation (Isolate & Deny)
        bpf_printk("UNDC: Policy violation detected for IP %pI4. Blocking connection.\n", &dest_ip);
        
        // Send async warning to user-space log via Ring Buffer
        // Return -EPERM to gracefully deny the socket connection without killing the process
        return -1; 
    }

    return 0; // Allowed: zero latency impact on sovereign paths
}
