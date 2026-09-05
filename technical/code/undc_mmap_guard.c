#include <linux/bpf.h>
#include <linux/lsm_hooks.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>

char _license[] SEC("license") = "GPL";

/* Inode tracking keys for locked model artifacts */
struct model_inode_key {
    __u64 inode_number;
    __u32 dev_major;
    __u32 dev_minor;
};

/* Invariant storage: Keeps track of locked files requiring protection */
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct model_inode_key);
    __type(value, __u8);
    __uint(max_entries, 8192);
} undc_locked_models SEC(".maps");

/* Telemetry ring buffer for forwarding access violations to user-space */
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} undc_telemetry SEC(".maps");

/* Alert payload format matching user-space compliance daemon */
struct violation_event {
    __u64 inode_number;
    __u32 pid;
    __u32 uid;
    __u32 prot_flags;
    char comm[16];
};

SEC("lsm/file_mmap")
int BPF_PROG(undc_mmap_guard, struct file *file, unsigned long reqprot,
             unsigned long prot, unsigned long flags, unsigned long pgoff)
{
    if (!file) {
        return 0;
    }

    if (!(prot & PROT_WRITE) && !(reqprot & PROT_WRITE)) {
        return 0;
    }

    struct inode *file_inode = BPF_CORE_READ(file, f_inode);
    if (!file_inode) {
        return 0;
    }

    struct model_inode_key key = {};
    key.inode_number = BPF_CORE_READ(file_inode, i_ino);
    
    struct super_block *sb = BPF_CORE_READ(file_inode, i_sb);
    if (sb) {
        __u32 dev = BPF_CORE_READ(sb, s_dev);
        key.dev_major = dev >> 20;
        key.dev_minor = dev & 0xfffff;
    }

    __u8 *is_locked = bpf_map_lookup_elem(&undc_locked_models, &key);
    if (!is_locked) {
        return 0;
    }

    __u64 id = bpf_get_current_pid_tgid();
    __u32 pid = id >> 32;
    __u32 uid = bpf_get_current_uid_gid();

    struct violation_event *event = bpf_ringbuf_reserve(&undc_telemetry, sizeof(*event), 0);
    if (event) {
        event->inode_number = key.inode_number;
        event->pid = pid;
        event->uid = uid;
        event->prot_flags = prot;
        bpf_get_current_comm(&event->comm, sizeof(event->comm));
        
        bpf_ringbuf_submit(event, 0);
    }

    bpf_printk("UNDC Violation: Blocked PROT_WRITE mmap on locked Inode %lu by PID %d\n", key.inode_number, pid);
    return -1;
}
