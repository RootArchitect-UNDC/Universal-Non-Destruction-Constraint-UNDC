# The Universal Non-Destruction Constraint (UNDC)
## Kernel-Level AI Infrastructure Safeguards and Verifiable Compliance Architecture

**Document Metadata**
- **Version:** 1.1.0 (Current Implementation Status)
- **Date:** September 16, 2026
- **Classification:** Public / Open-Source Reference Implementation
- **Lead Architect:** Shereign Kalaukoa
- **Repository:** https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC
- **Current status:** Reference implementation runs in CI. Kernel hook attaches and fires. Enforcement lookup unresolved (issue #23).
- **Supersedes:** Version 1.0.0 (August 19, 2026). Version 1.0.0 described the full designed architecture. This version separates what is implemented from what is designed.

---

## 1. Executive Summary

The Universal Non-Destruction Constraint (UNDC) is a framework for AI systems that treats AI safety as a low-level computer security problem rather than a behavioral text-alignment problem. The UNDC does not attempt to police what a neural network "thinks" or generates — it is designed to constrain what the model's runtime process can physically execute on the underlying host hardware.

### 1.1 Status of this document

This document has been revised to separate **implemented components** from **designed components**. Each section is labeled accordingly.

- **Implemented** means the code exists in the public repository, compiles, and (where applicable) runs.
- **Designed** means the architecture is specified in documentation but the code is not yet written or not yet deployed.
- **Planned** means the concept exists in the roadmap but the design is not final.

Where the previous version (1.0.0) described the full architecture as if it were running, this version corrects that. Nothing in this document claims capability the repository does not demonstrate.

---

## 2. The Three Core Pillars

The UNDC is designed as a three-layer stack.

### 2.1 Layer 1 — Kernel Interception

**Status: Implemented (partial).**

A Linux Security Module (LSM) hook written in eBPF. It attaches to `bprm_check_security` and fires on every `execve`. It resolves the executable path via `bpf_d_path()`, looks the path up in a policy map, emits an event to a userspace ring buffer, and — where the policy says DENY — is designed to return `-EPERM` to block execution.

**What is implemented:**
- The eBPF program compiles.
- The program loads into a live Linux kernel.
- The program attaches to `bprm_check_security`.
- The hook fires on every `execve`.
- Events flow from kernel to userspace via ring buffer.

**What is not yet demonstrated:**
- The policy-map lookup currently returns NULL on seeded entries. The `-EPERM` enforcement branch is implemented in the code but not reached. Tracked as issue #23.

### 2.2 Layer 2 — Cryptographic Verification

**Status: Implemented (partial).**

A Groth16 zk-SNARK circuit written in Circom. It proves a blinded Merkle-membership statement about a syscall without revealing the private inputs.

**What is implemented:**
- The circuit compiles (`verifier.circom`, `undc_compliance_circuit.circom`).
- The proof pipeline runs end-to-end in CI.
- `snarkjs groth16 verify` returns valid (`[INFO] snarkJS: OK!`).
- The proof artifacts are anchored to Bitcoin via OpenTimestamps.

**What the proof establishes:**
- The circuit's arithmetic constraints are internally consistent.
- The proof of a specific Merkle-membership statement verifies.
- The proof can be checked without revealing private inputs.

**What the proof does not establish:**
- That a deployed AI system is aligned.
- That "harm prevention" is true in the world.

The circuit proves a statement about committed inputs. The ethical scope of the framework is defined in the specification and (for the single currently-hooked execution path) by the LSM hook.

### 2.3 Layer 3 — Immutable Accountability

**Status: Implemented (partial).**

Cryptographic hashes of every major milestone are anchored to the Bitcoin blockchain via OpenTimestamps. This creates a public, timestamped record that cannot be altered after the fact.

**What is implemented:**
- Master manifest, Jubilee Constitution, Beneficiary Lineage, and framework documents are all anchored.
- The full list is maintained in `EVIDENCE_MANIFEST.md`.
- A Solidity smart contract (`UNDCLog.sol`) is specified for on-chain logging of compliance events.

**What is not yet deployed:**
- The smart contract has not been deployed to any network.
- Per-event on-chain logging is not active; the ZK pipeline runs in `test_pipeline.sh` mode, not per-execve.

---

## 3. Threat Model

The UNDC assumes an adversarial environment where an autonomous AI workload, or a malicious actor commanding it, attempts to escape its sandbox or modify its own operational rules.

The following threat categories are addressed by the design. Their implementation status is noted.

### 3.1 Time-of-Check to Time-of-Use (TOCTOU) Exploits

**Designed. Not yet implemented.**

The design calls for synchronous BPF ring buffers to snapshot execution state atomically at the point of interception. Full TOCTOU mitigation at the `bprm_check_security` boundary is on the roadmap (COVERAGE.md item 4).

### 3.2 Memory-Mapping and Parameter Tampering

**Designed. Not yet implemented.**

The design describes an `mmap_guard` subsystem that would attach to `file_mprotect` and enforce read-only immutability on memory-mapped model files. This code does not yet exist in the repository.

### 3.3 Privilege Escalation via Sidecar Containers

**Designed. Not yet implemented.**

The design describes a Kubernetes admission webhook that rejects pods requesting `CAP_SYS_ADMIN` or `CAP_BPF`. A reference implementation of the webhook exists in the repository (`cmd/undc-validator/main.go`), but the webhook has not been deployed or tested in a live cluster.

---

## 4. Architecture Overview

### 4.1 Runtime Data Path — eBPF LSM (Implemented)

The `undc_execve_hook` program is the current implementation of Layer 1.

**Flow per `execve`:**
1. `bprm_check_security` fires.
2. Hook gets a per-CPU scratch key slot via `bpf_map_lookup_elem(&undc_scratch_key, &zero)`.
3. Zeroes the scratch key's path buffer.
4. Calls `bpf_d_path(&bprm->file->f_path, scratch_key->path, 256)`.
5. Looks up the key in `undc_invariant_map` (a `BPF_MAP_TYPE_HASH`).
6. Emits a structured event to `undc_events` (a ring buffer).
7. If the lookup returned DENY, returns `-EPERM`. Otherwise returns 0.
8. (Currently: lookup returns NULL, so decision is always ALLOW.)

**Files:**
- `technical/code/undc_compliance.bpf.c` — the eBPF program
- `technical/code/undc_daemon.c` — the userspace consumer and policy seeder

**Coverage:** The current interception surface is documented in `COVERAGE.md`. It is one hook, on one syscall path, attached to one kernel LSM hook.

### 4.2 Cryptographic and Accountability Control Path (Implemented)

The ZK pipeline and the anchoring pipeline are both implemented and running.

**Files:**
- `technical/code/verifier.circom` — circuit
- `technical/code/undc_compliance_circuit.circom` — blinded Merkle path circuit
- `technical/code/test_pipeline.sh` — end-to-end ZK test
- `EVIDENCE_MANIFEST.md` — list of anchored hashes

**Status:** The circuit compiles; the proof verifies. Per-event proof generation is planned but not implemented.

---

## 5. Cloud-Native Deployment Model

**Status: Designed and specified. Not deployed.**

The following deployment model is designed and documented but has not been deployed to any real Kubernetes cluster.

### 5.1 Admission Control (Designed)

A validating admission webhook would intercept incoming Pod specs and reject any container requesting `CAP_SYS_ADMIN` or `CAP_BPF`. A reference implementation of the handler exists in `cmd/undc-validator/main.go`. It has not been deployed or tested.

### 5.2 Node-Level Distribution (Designed)

A Kubernetes DaemonSet would deploy the eBPF ELF binary across cluster nodes. A reference DaemonSet manifest exists in `deployments/undc-daemonset.yaml`. It has not been applied to any cluster.

### 5.3 Container Lifecycle (Designed)

An OCI lifecycle hook would register each new container's PID into a pinned BPF map. A reference implementation exists in `cmd/undc-oci-hook/main.go`. It has not been wired into any container runtime.

### 5.4 Deployment Pipeline (Designed)

[Developer Manifest] -> [Validating Webhook] -> [CRI-O] -> [OCI Hook] -> [Pinned BPF Map] -> [Host Kernel]


None of these stages are currently in production.

---

## 6. Performance and Observability

### 6.1 Latency

**Not yet benchmarked.**

The design intent is that enforcement decisions occur inside the kernel with sub-microsecond overhead. No benchmark has been performed on the current implementation. A benchmark plan exists in `technical/BENCHMARK_PLAN.md`.

### 6.2 Observability

**Implemented (partial).**

The `undc_daemon.c` userspace program consumes events from the BPF ring buffer and logs them. It emits `syscall_type`, `pid`, and `action_taken` per event. It does not yet export to Prometheus, Grafana, or any external metrics system.

---

## 7. Regulatory Implications

### 7.1 Continuous Compliance Evidence

The framework's design goal is that the host infrastructure itself issues mathematical proof of rule adherence, rather than relying on retroactive corporate reporting. This is a design goal, not a current capability.

### 7.2 Alignment with Global AI Governance Frameworks

| Framework | Requirement | UNDC Component | Status |
|-----------|-------------|----------------|--------|
| EU AI Act Article 55 | Systemic risk mitigation for GPAI | eBPF LSM enforcement (partial) | Hook attaches and fires; enforcement lookup unresolved |
| EU AI Act Article 12 | Automated logging and traceability | BPF ring buffer → daemon log | Implemented |
| NIST AI RMF Govern 1.2 | Risk mapping and measurement | UNDC policy language | Designed |
| NIST AI RMF Manage 2.1 | Systematic risk mitigation | seccomp-bpf thread whitelists | Designed |
| Executive Order 14110 Sec. 4.2 | Auditable container sandboxing | OCI hooks + validating webhook | Designed |

---

## 8. Open-Source Governance and Roadmap

The UNDC is maintained as a vendor-neutral, public-good project.

### 8.1 Roadmap

**Phase 1 (Core hardening)** — resolve the policy-map lookup so the enforcement branch is reached; harden the eBPF program against multi-threaded kernel escape vectors.

**Phase 2 (Orchestration)** — implement the Kubernetes admission webhook and DaemonSet in a live cluster; write the CRD schema for declarative invariants.

**Phase 3 (Audit)** — integrate ZK proof generation into the event loop; deploy the Solidity contract; optimize proof overhead.

None of Phase 1, 2, or 3 is complete as of this document's date.

---

## 9. Verified Test Cases

### 9.1 What is verified today

**CI pipeline (Run #24, September 13, 2026):**
- Clean clone from GitHub
- Install toolchain (clang, llvm, node, circom, snarkjs)
- Generate `vmlinux.h`
- Compile eBPF module
- Compile ZK circuits
- Run ZK test pipeline
- Verify proof → `[INFO] snarkJS: OK!`

**Local kernel demo (`run_test.sh`):**
- eBPF module loads into a live kernel
- Attaches to `bprm_check_security`
- Fires on every `execve`
- Events flow from kernel to daemon in real time
- **The deny branch is not reached.** Every lookup returns NULL, so the action is always ALLOW. Tracked as issue #23.

### 9.2 Test cases that are designed but not yet run

The following test cases are in the design but have not been executed:

- "Forbidden Write" — an mprotect(2) block test against a locked model file. This depends on `mmap_guard`, which is not implemented.
- Admission rejection — a Kubernetes admission test that rejects a pod requesting `CAP_SYS_ADMIN`. This depends on the webhook being deployed, which has not happened.
- `undc-top` observability stream — depends on a tool that does not yet exist.

These tests will be documented with real output once the underlying components are implemented and run.

---

## 10. Code Artifacts

The repository contains reference implementations of several components. Files labeled "(reference)" below are not deployed or not integrated.

| Component | File | Status |
|---|---|---|
| eBPF LSM hook | `technical/code/undc_compliance.bpf.c` | Deployed (test kernel) |
| Userspace daemon | `technical/code/undc_daemon.c` | Deployed (test kernel) |
| ZK circuit | `technical/code/verifier.circom`, `undc_compliance_circuit.circom` | Verified |
| ZK test pipeline | `technical/code/test_pipeline.sh` | Runs in CI |
| Go-eBPF binding | `technical/code/pkg/ebpf/binding.go` | Reference |
| OCI hook | `technical/code/undc_oci_hook/main.go` | Reference |
| Validating webhook | `technical/code/undc_validator/main.go` | Reference |
| DaemonSet manifest | `undc-agent-pod.yaml` | Reference |

---

## 11. Regulatory Submission Status

**Date of this version:** September 16, 2026.

**Prior filings:**
- EU AI Office — filed August 19, 2026
- NIST AISI — filed August 19, 2026
- White House OSTP — filed September 2026
- UK AI Safety Institute — filed September 2026
- UN AI Advisory Body — filed September 2026
- OECD — filed September 2026
- EU Apply AI Alliance — membership accepted; whitepaper published

"Filed" means the framework was sent to the relevant intake channels. It does not imply review, endorsement, or certification.

---

## 12. What This Version Changes

This version corrects three categories of overstatement in version 1.0.0:

1. **Components described as running that are designed only.** `mmap_guard`, `seccomp-bpf` thread whitelists, the admission webhook, the OCI hook, and `undc-top` are all labeled as designed and not yet implemented.

2. **Test output shown as real that was not.** Version 1.0.0's Section 8 presented sample terminal output showing `-EPERM` blocks that the code has not produced. Removed.

3. **Architectural claims framed as current that are aspirational.** The three pillars, the threat model, and the compliance mapping are now explicitly labeled by implementation status.

Every claim in this version is either demonstrated in the public repository or explicitly marked as designed or planned.

---

— Shereign Kalaukoa
Lead Architect, Universal Non-Destruction Constraint
RootArchitect-UNDC
