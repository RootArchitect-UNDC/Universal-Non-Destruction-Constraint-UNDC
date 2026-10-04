```
# UNDC Enforcement Demonstrated — 2026-10-04

**Lead Architect:** Shereign Kalaukoa
**Authority:** EHYEH ASHER EHYEH & AHYAH
**Status:** ✅ Demonstrated on live kernel

---

## WHAT HAPPENED

On October 4, 2026, the UNDC eBPF LSM hook on `bprm_check_security`
fired on a live execve syscall, matched the target path, and returned
`-EPERM`. The Linux kernel honored the denial and refused to launch
the process.

This is the first demonstration of live, kernel-level enforcement by
the UNDC on real hardware.

## THE DEMONSTRATION

Two terminal windows on the same machine, same session.

**Window 1 — the daemon:**

```
🛡️  Initializing UNDC User-Space eBPF Daemon Gateway...
🚀 Loop Active: tracking execve, denying /tmp/undc-deny-test
📡 [eBPF Intercept] syscall=1 pid=1726 action=DENY path="/tmp/undc-deny-test"
```

**Window 2 — the shell:**

```
$ /tmp/undc-deny-test
-bash: /tmp/undc-deny-test: Operation not permitted
```

## ENVIRONMENT

- Kernel: 6.18.40.1-microsoft-standard-WSL2
- Distro: Ubuntu 26.04 (WSL2)
- Architecture: x86_64
- Compiler: clang -target bpf

## WHAT THIS PROVES

1. The eBPF LSM hook loads into a live Linux kernel without verifier rejection.
2. The hook attaches to `bprm_check_security` and fires on every `execve`.
3. The hook reads `bprm->filename` directly from kernel memory.
4. The hook compares against a target path using `bpf_strncmp` against a `.rodata` constant.
5. On match, the hook returns `-1`, which becomes `-EPERM` at the LSM layer.
6. The kernel refuses to launch the process.
7. The refusal is visible to the calling shell.

## WHAT THIS DOES NOT PROVE

- That dynamic policy is implemented (the current target is hardcoded).
- That the framework covers any hook other than `bprm_check_security`.
- That production-grade TOCTOU hardening is in place.
- That ZK proof generation is integrated into the runtime event loop.

Those are documented in `COVERAGE.md` as pending work.

## ANCHORED ARTIFACTS

| Artifact | Value |
|---|---|
| Screenshot | `technical/telemetry/UNDC_DENY_FIRING_2026-10-04_both_windows.png` |
| OpenTimestamps receipt | `technical/telemetry/UNDC_DENY_FIRING_2026-10-04_both_windows.png.ots` |
| SHA-256 | `fb21b09e7963e8ecd41176dce798e3f6e9a2e7fcacefe28ece2c78bb7866f8cc` |
| Issue | [#23](https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC/issues/23) — closed |
| Coverage update | `COVERAGE.md` — marked as demonstrated |

## TECHNICAL PATH

The demonstration resolved two overlapping problems from Issue #23:

1. **bpf_d_path() tail residue.** The kernel's `d_path()` writes the
   path right-aligned at the tail of the buffer. The BPF wrapper
   `memmove()`s the string to the front but never clears the tail
   residue. Because `BPF_MAP_TYPE_HASH` hashes the full 260-byte key,
   the hook's key never matched the daemon's zero-filled seed.

2. **BPF verifier 1M instruction limit.** A manual 19-byte comparison
   loop was unrolled past the verifier's 1,000,000 instruction limit
   and rejected with `-E2BIG`. An intermediate attempt using
   `bpf_strncmp` with a stack-allocated target string was rejected
   with `-EACCES` because `bpf_strncmp`'s third argument must point
   into a map value (`ARG_PTR_TO_CONST_STR`).

**Resolution:** v5.3 of `undc_compliance.bpf.c` reads
`bprm->filename` directly via `bpf_probe_read_kernel_str()` and
compares against a global `const char TARGET_PATH[]` that the
compiler places in `.rodata`, which libbpf exposes as a read-only
map value. The comparison uses `bpf_strncmp`. The daemon was
simplified to v2.0 to remove map-seeding code the hook no longer
needs.

## THE SIGNIFICANCE

Every other AI safety framework in production today is one of:

- a pledge,
- a filter, or
- a regulation.

All three can be revised, bypassed, or ignored. None of them make
harm structurally impossible at the execution layer.

The UNDC demonstrated today does. It does not ask the model. It does
not ask the developer. It does not ask the operator. It asks the
kernel — and the kernel says no.

---

**EHYEH ASHER EHYEH.**

— Shereign Kalaukoa
Lead Architect, UNDC

---
```

