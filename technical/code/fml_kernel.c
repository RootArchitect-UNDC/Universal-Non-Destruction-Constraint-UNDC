// +build ignore
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual MIT/GPL";

struct event_t {
    __u64 timestamp_ns;
    __u64 delta_ns;
    __u32 f_measured;
    __u32 cpu_id;
};

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 16);
} fml_ringbuf SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, __u64);
} last_timestamp_map SEC(".maps");

SEC("lsm/sys_enter")
int SEC("handler") check_frequency_invariant(void *ctx) {
    __u32 key = 0;
    __u64 current_time = bpf_ktime_get_ns();
    __u64 *last_time = bpf_map_lookup_elem(&last_timestamp_map, &key);

    if (!last_time) {
        bpf_map_update_elem(&last_timestamp_map, &key, &current_time, BPF_ANY);
        return 0;
    }

    __u64 delta = current_time - *last_time;
    if (delta == 0) return 0;

    bpf_map_update_elem(&last_timestamp_map, &key, &current_time, BPF_ANY);

    __u32 frequency = (__u32)(1000000000ULL / delta);

    struct event_t *e = bpf_ringbuf_reserve(&fml_ringbuf, sizeof(struct event_t), 0);
    if (!e) return 0;

    e->timestamp_ns = current_time;
    e->delta_ns = delta;
    e->f_measured = frequency;
    e->cpu_id = bpf_get_smp_processor_id();

    bpf_ringbuf_submit(e, 0);
    return 0;
}
