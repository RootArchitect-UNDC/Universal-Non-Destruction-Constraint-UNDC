# COVERAGE.md — UNDC Kernel-Level Interception

**Last Updated:** September 16, 2026
**Source Files:**
- `technical/code/undc_compliance.bpf.c` — eBPF LSM (loaded and attached)
- `technical/code/undc_lsm_hooks.c` — traditional kernel LSM (stub, not loaded)
- `technical/code/undc_daemon.c` — userspace daemon (seeds hash map, consumes events)

This document describes exactly which kernel paths the UNDC module intercepts, what the policy does on each path, and what is explicitly **not** covered.

---

## Current Interception Surface

| Syscall Path | Hook | File | Status | Behavior |
|---|---|---|---|---|
| `execve` / `execveat` | `lsm/bprm_check_security` | `undc_compliance.bpf.c` | Built — **attached, firing; enforcement lookup unresolved** | Resolves binary path via `bpf_d_path()`, looks up path in a `BPF_MAP_TYPE_HASH` via per-CPU scratch key, emits ring-buffer event. The map lookup currently returns NULL on seeded entries; the `-EPERM` enforcement branch is implemented but not yet demonstrated. Tracked as issue [#23](https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC/issues/23). |
| `execve` / `execveat` | `bprm_check_security` | `undc_lsm_hooks.c` | Stub — **not loaded** | Calls `undc_evaluate_harm()`, which returns `harm_score = 0` unconditionally → allow. Requires kernel rebuild to load. |
| `mmap` | `mmap_file` | `undc_lsm_hooks.c` | Stub — **not loaded** | Same stub behavior. Requires kernel rebuild to load. |

**Total hook entry points written: 3**
**Total hooks loaded on a live kernel: 1** (`bprm_check_security` via eBPF)
**Total enforcing hooks: 0** (enforcement lookup unresolved)

---

## Three Implementation Approaches

### A. eBPF LSM (`undc_compliance.bpf.c`)

- Loadable at runtime via `bpf()` syscall. No kernel rebuild required.
- Currently loaded and attached on a test kernel (pinned at `/sys/fs/bpf/undc_compliance`).
- Intercepts `bprm_check_security`.
- Uses a `BPF_MAP_TYPE_HASH` (`undc_invariant_map`) for path→action lookups.
- Uses a `BPF_MAP_TYPE_PERCPU_ARRAY` scratch map (`undc_scratch_key`) to build the 260-byte key off the 512-byte BPF stack.
- Emits events to a ring buffer (`undc_events`).
- Enforcement branch returns `-EPERM` on a DENY hit; currently not reached because the lookup returns NULL. See issue #23.

### B. Traditional Kernel LSM (`undc_lsm_hooks.c`)

- Requires kernel rebuild or `CONFIG_LSM` inclusion. Not loadable as-is.
- Registers with the LSM framework under the name `"undc"`.
- Declares hooks for `bprm_check_security` and `mmap_file`.
- Both hooks call `undc_evaluate_harm()`, which is currently a stub returning 0.
- Not loaded on any tested kernel.

### C. Userspace Daemon (`undc_daemon.c`)

- Loads and attaches the eBPF program via `undc_compliance_bpf__open_and_load()` and `undc_compliance_bpf__attach()`.
- Seeds the hash map at startup with one deny path (default `/usr/bin/undc-test-deny`, overridable via CLI argument).
- Consumes events from the `undc_events` ring buffer with a 100ms poll timeout.
- Logs each event with `syscall_type`, `pid`, and `action_taken`.

---

## Pipeline State

| Stage | State |
|---|---|
| eBPF program loaded | ✅ |
| eBPF program attached to `bprm_check_security` | ✅ |
| Ring buffer events emitted on `execve` | ✅ |
| Events consumed by daemon | ✅ |
| Events parsed with full fidelity | ✅ |
| Hash map populated by daemon | ✅ (verified via `bpftool map dump`) |
| Hash map lookup matches seeded key | ❌ (see issue #23) |
| Kernel denies on hit | ❌ (branch implemented, not reached) |
| ZK proof triggered per event | ❌ planned |
| Proof anchored to chain per event | ❌ planned |

---

## Maps

| Map | Type | Key | Value | Purpose |
|---|---|---|---|---|
| `undc_events` | `BPF_MAP_TYPE_RINGBUF` | — | `struct syscall_event` | Emits per-invocation events to the userspace daemon |
| `undc_invariant_map` | `BPF_MAP_TYPE_HASH` | `struct lpm_key` (260 bytes) | `__u32` action code | Stores paths with an associated action |
| `undc_scratch_key` | `BPF_MAP_TYPE_PERCPU_ARRAY` | `__u32` (index 0) | `struct lpm_key` | Per-CPU buffer for building the lookup key off the BPF stack |

---

## Action Codes

| Code | Meaning | Enforced |
|---|---|---|
| `0` | Allow (default) | ✅ (default when lookup misses) |
| `1` | Deny | ❌ (not reached; lookup misses) |
| `2` | Audit only | ✅ (event emitted, execution allowed) |
| Other | Reserved | ❌ |

---

## What Is Explicitly **Not** Covered

- **Network egress** — no `socket_*` or `sk_*` hooks.
- **File writes / opens** — no `file_open`, `file_permission`, or `inode_*` hooks.
- **Process memory writes** — no `mmap_file` enforcement (declared as a stub only), no `mprotect`, no `ptrace_access_check`.
- **Kernel module loading** — no `kernel_module_request` or `kernel_load_data` hooks.
- **Mount / unmount** — no `sb_mount`, `sb_umount` hooks.
- **Userspace behavior** — anything not routed through a hooked kernel path is outside the surface.
- **Cross-process IPC** — no `task_kill`, `ptrace_*`, or signal hooks.
- **Existing processes** — hooks fire on new calls only; already-running processes are not evaluated retroactively.
- **Container boundaries** — no cgroup or namespace hooks.

---

## Known Issues / Pending Work

1. **Enforcement lookup returns NULL on seeded entries.** The hash-map lookup misses for every path, including paths confirmed byte-correct in the map via `bpftool map dump`. Seven hypotheses have been falsified. Full diagnostic: issue [#23](https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC/issues/23).

2. **Traditional LSM is a stub.** `undc_lsm_hooks.c` needs `undc_evaluate_harm()` implemented with real policy logic before it does anything.

3. **Traditional LSM requires kernel rebuild.** Not loadable on WSL2's stock kernel without changes.

4. **No TOCTOU mitigation.** Path resolution at `bprm_check_security` time can be raced between check and exec.

5. **Only one demo path in map.** `run_test.sh` seeds one deny path. Production use requires a trie/map population mechanism (rule file loader, daemon API, or external control plane).

6. **No ZK integration in the event loop.** Events are consumed by the daemon but do not yet trigger proof generation. The ZK pipeline runs in `test_pipeline.sh` only.

---

## How to Expand Coverage

Every new path added to the interception surface should:

1. Be implemented as a new `SEC("lsm/<hook_name>")` BPF program in `undc_compliance.bpf.c`, or as an additional `LSM_HOOK_INIT` entry in `undc_lsm_hooks.c`.
2. Emit events with a distinct `syscall_type` value.
3. Be added to this document with hook name, function name, status, and a one-line behavior description.
4. Be added to daemon routing logic in `undc_daemon.c`.
5. Be tested via `run_test.sh` or `test_pipeline.sh` with a case that exercises the new path.

---

## Changelog

| Date | Change |
|---|---|
| 2026-09-14 | Initial coverage document. |
| 2026-09-15 | Enforcement branch added; daemon seeds map; `run_test.sh` demonstrates deny. |
| 2026-09-16 | Updated after issue #23 filed. Corrected map type (LPM trie → hash map). Added per-CPU scratch key. Enforcement lookup marked unresolved. "Enforcing hooks" count corrected to 0. |
