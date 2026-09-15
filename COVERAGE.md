# COVERAGE.md

**Universal Non-Destruction Constraint (UNDC)**
**Last Updated:** September 15, 2026
**Authority:** EHYEH ASHER EHYEH & AHYAH

---

## Purpose

This document defines the exact syscall paths, policies, and mechanisms the UNDC reference implementation currently enforces, and what it does not yet cover. Every claim in the README that touches on runtime behavior should be verifiable against this file.

If a claim is not supported by a row in this table, it is not yet a runtime behavior. It is either a design intent, a declared scope, or an aspirational goal.

---

## Current Coverage

| Syscall Path | LSM Hook | Policy | Runtime State | Evidence |
|--------------|----------|--------|---------------|----------|
| `execve` | `bprm_check_security` | Path-based LPM trie lookup against `undc_invariant_map`. Deny if path resolves to a protected entry. | ⏳ Hook loaded, not attached; map initialized but no policy entries loaded | Program ID 119, tag `45986485f27740eb`, pinned at `/sys/fs/bpf/undc_compliance` |

**Legend:**
- ✅ **Verified** — Hook attached, demonstrated firing on a live kernel, evidence anchored.
- ⏳ **Loaded** — Hook compiled and resident in kernel, not yet attached to the call path.
- 📄 **Specified** — Design exists, implementation not started.
- ⛔ **Not covered** — Explicitly out of scope for the current build.

---

## Explicitly Not Covered

The following are outside the current implementation. They are not blocked, monitored, or intercepted by the UNDC at this time.

| Domain | Status | Notes |
|--------|--------|-------|
| Network egress | ⛔ Not covered | `socket_connect`, `socket_sendmsg`, and related paths are not hooked. |
| File writes | ⛔ Not covered | `write`, `pwrite`, `mmap` with `PROT_WRITE` are not intercepted. |
| Process spawning (non-execve) | ⛔ Not covered | `fork`, `clone`, `vfork` are not hooked. |
| Memory protection changes | ⛔ Not covered | `mprotect`, `mmap` protection flags are not intercepted. |
| Userspace behavior | ⛔ Not covered | Any activity above the kernel boundary is outside LSM scope. |
| Semantic harms | 📄 Specified | Continuous Intent Monitoring is architectural, not implemented. |
| Cross-node coordination | 📄 Specified | The four-layer stack design includes Layer 4 anchoring; contract not deployed. |

---

## What the UNDC Does Not Claim

- It does not claim that harm is impossible. It claims that specific paths under a defined policy are denied once the hook is attached.
- It does not claim alignment. The ZK circuit proves a Merkle-membership statement; it does not prove that a deployed AI is aligned.
- It does not claim coverage of all syscalls. Coverage expands path by path.
- It does not claim regulatory acceptance. Submissions are filed, not accepted.

---

## How Coverage Expands

Each new syscall path is added by:

1. Writing the LSM hook function in `undc_compliance.bpf.c`.
2. Registering the hook name in the `SEC()` macro.
3. Compiling and loading the module.
4. Attaching the hook to its kernel call path.
5. Producing a `-EPERM` demonstration on a live kernel.
6. Anchoring the demonstration to the Evidence Manifest.

Until step 5, the path is not a runtime behavior. It is a loaded program waiting to attach.

---

## Verification

Any reader can verify the current coverage by:

1. Cloning the repository.
2. Running `sudo bpftool prog show id 119` on a kernel where the module is loaded.
3. Confirming the pinned path with `ls -la /sys/fs/bpf/undc_compliance`.
4. Checking the Evidence Manifest for the corresponding anchor.

Once the attach milestone is complete, this file will include a demonstration log for each attached hook.

---

— Shereign Kalaukoa, Lead Architect | UNDC
🔗 https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC
