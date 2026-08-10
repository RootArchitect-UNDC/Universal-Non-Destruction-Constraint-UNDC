#include <linux/bpf.h>
#include <linux/lsm_hook_defs.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <linux/in.h>
#include <linux/errno.h>

/* Define target structure for checking IPv4 network blocks */
struct ipv4_constraint {
    __u32 allowed_subnet; /* Subnet address in Network Byte Order */
    __u32 subnet_mask;    /* Bitmask in Network Byte Order */
};

/* Global configuration maps populated out-of-band by the UNDC Oracle */
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, struct ipv4_constraint);
} undc_net_cfg SEC(".maps");

SEC("lsm/socket_connect")
int BPF_PROG(undc_socket_hook, struct socket *sock, struct sockaddr *address, int addrlen) {
    // 1. Establish structural guard boundaries
    if (!address || addrlen < sizeof(struct sockaddr_in)) {
        return 0; // Let non-IPv4 or malformed packets pass to localized filters
    }

    // 2. Cast to internet protocol tracking structure
    struct sockaddr_in *addr_in = (struct sockaddr_in *)address;
    if (addr_in->sin_family != AF_INET) {
        return 0; 
    }

    __u32 target_ip = addr_in->sin_addr.s_addr;
    __u32 key = 0;

    // 3. Resolve cryptographic safelist boundaries from the UNDC configuration map
    struct ipv4_constraint *cfg = bpf_map_lookup_elem(&undc_net_cfg, &key);
    if (!cfg) {
        /* Failsafe Default-Deny: If oracle configuration is missing or altered,
           structurally drop the connection to maintain absolute system safety. */
        return -EPERM;
    }

    // 4. Mathematical state-space validation loop
    /* Filter C evaluating target transition against the H (Unauthorized Socket Connections) set */
    if ((target_ip & cfg->subnet_mask) != cfg->allowed_subnet) {
        __u32 pid = bpf_get_current_pid_tgid() >> 32;
        
        bpf_printk("UNDC Violation: Blocked unauthorized socket mutation from PID %d to IP 0x%X\n", pid, bpf_ntohl(target_ip));
        
        /* Return Operation Not Permitted inline at the kernel layer, breaking the 
           dependency path cleanly without allowing data leak vectors. */
        return -EPERM; 
    }

    return 0; // Invariant satisfied. Proceed safely.
}

char _license[] SEC("license") = "GPL";
