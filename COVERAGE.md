# COVERAGE.md — UNDC Kernel-Level Interception

**Last Updated:** October 4, 2026
**Source Files:**
- `technical/code/undc_compliance.bpf.c` — eBPF LSM (loaded, attached, enforcing)
- `technical/code/undc_lsm_hooks.c` — traditional kernel LSM (stub, not loaded)
- `technical/code/undc_daemon.c` — userspace daemon (consumes events)

This document describes exactly which kernel paths the UNDC module intercepts, what the policy does on each path, and what is explicitly **not** covered.

---

## Current Interception Surface

| Syscall Path | Hook | File | Status | Behavior |
|---|---|---|---|---|
| `execve` / `execveat` | `lsm/bprm_check_security` | `undc_compliance.bpf.c` | **Attached, firing, and enforcing** | Reads `bprm->filename` directly via `bpf_probe_read_kernel_str()`, compares against the `.rodata` target string using `bpf_strncmp`, emits ring-buffer event, and returns `-EPERM` on match. Demonstrated on kernel 6.18.40.1 WSL2 x86_64 on 2026-10-04. |
| `execve` / `execveat` | `bprm_check_security` | `undc_lsm_hooks.c` | Stub — **not loaded** | Calls `undc_evaluate_harm()`, which returns `harm_score = 0` unconditionally → allow. Requires kernel rebuild to load. |
| `mmap` | `mmap_file` | `undc_lsm_hooks.c` | Stub — **not loaded** | Same stub behavior. Requires kernel rebuild to load. |

**Total hook entry points written: 3**
**Total hooks loaded on a live kernel: 1** (`bprm_check_security` via eBPF)
**Total enforcing hooks: 1** (`bprm_check_security` via eBPF, demonstrated 2026-10-04)

---

## Three Implementation Approaches

### A. eBPF LSM (`undc_compliance.bpf.c`)

- Loadable at runtime via `bpf()` syscall. No kernel rebuild required.
- Currently loaded and attached on a test kernel.
- Intercepts `bprm_check_security`.
- Reads `bprm->filename` directly via `bpf_probe_read_kernel_str()`.
- Compares against a `.rodata` string constant via `bpf_strncmp`.
- Emits events to a ring buffer (`undc_events`).
- Enforcement branch returns `-EPERM` on match. Demonstrated 2026-10-04.
- The earlier `bpf_d_path()` + hash map lookup pattern is retained in commit history but is no longer used.

### B. Traditional Kernel LSM (`undc_lsm_hooks.c`)

- Requires kernel rebuild or `CONFIG_LSM` inclusion. Not loadable as-is.
- Registers with the LSM framework under the name `"undc"`.
- Declares hooks for `bprm_check_security` and `mmap_file`.
- Both hooks call `undc_evaluate_harm()`, which is currently a stub returning 0.
- Not loaded on any tested kernel.

### C. Userspace Daemon (`undc_daemon.c`)

- Loads and attaches the eBPF program via `undc_compliance_bpf__open_and_load()` and `undc_compliance_bpf__attach()`.
- Consumes events from the `undc_events` ring buffer with a 100ms poll timeout.
- Logs each event with `syscall_type`, `pid`, `action_taken`, and `path`.

---

## Pipeline State

| Stage | State |
|---|---|
| eBPF program loaded | ✅ |
| eBPF program attached to `bprm_check_security` | ✅ |
| Ring buffer events emitted on `execve` | ✅ |
| Events consumed by daemon | ✅ |
| Events parsed with full fidelity | ✅ |
| Hash map lookup matches seeded key | ⚠️ Deprecated — hook now uses direct `.rodata` string comparison, not map lookup |
| Kernel denies on hit | ✅ Demonstrated 2026-10-04 |
| ZK proof triggered per event | ❌ planned |
| Proof anchored to chain per event | ❌ planned |

---

## Maps

| Map | Type | Key | Value | Purpose |
|---|---|---|---|---|
| `undc_events` | `BPF_MAP_TYPE_RINGBUF` | — | `struct syscall_event` | Emits per-invocation events to the userspace daemon |

The `undc_invariant_map` and `undc_scratch_key` maps from earlier versions are no longer used by the current hook. The deny policy is now fixed in the hook itself via the `.rodata` target string.

---

## Action Codes

| Code | Meaning | Enforced |
|---|---|---|
| `0` | Allow (default) | ✅ (default when filename does not match target) |
| `1` | Deny | ✅ (returns `-EPERM` on match, demonstrated 2026-10-04) |
| `2` | Audit only | ⚠️ not yet implemented |
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

1. **Single hardcoded target path.** The hook currently compares against one fixed `.rodata` string. Dynamic policy (rule file, map-driven lookup, or external control plane) is not yet implemented.

2. **Traditional LSM is a stub.** `undc_lsm_hooks.c` needs `undc_evaluate_harm()` implemented with real policy logic before it does anything.

3. **Traditional LSM requires kernel rebuild.** Not loadable on WSL2's stock kernel without changes.

4. **No TOCTOU mitigation.** Path resolution at `bprm_check_security` time can be raced between check and exec.

5. **No ZK integration in the event loop.** Events are consumed by the daemon but do not yet trigger proof generation. The ZK pipeline runs in `test_pipeline.sh` only.

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
| 2026-10-04 | Enforcement demonstrated. Hook refactored to v5.3 (direct `.rodata` string comparison via `bpf_strncmp`). Returns `-EPERM` on match. Screenshot hash `fb21b09e7963e8ecd41176dce798e3f6e9a2e7fcacefe28ece2c78bb7866f8cc` OpenTimestamps anchored. Issue #23 to be closed. |
| 2026-09-16 | Updated after issue #23 filed. Corrected map type (LPM trie → hash map). Added per-CPU scratch key. Enforcement lookup marked unresolved. "Enforcing hooks" count corrected to 0. |
| 2026-09-15 | Enforcement branch added; daemon seeds map; `run_test.sh` demonstrates deny. |
| 2026-09-14 | Initial coverage document. |
