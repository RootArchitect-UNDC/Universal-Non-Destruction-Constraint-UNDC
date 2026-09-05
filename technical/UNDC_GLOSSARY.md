# Universal Non-Destruction Constraint (UNDC) — Glossary

**Lead Architect:** Shereign Kalaukoa  
**Date:** August 24, 2026  
**Purpose:** To provide plain‑language definitions of technical terms used in the UNDC framework.

---

## A

**AI Agent** — An autonomous software system that can perceive its environment, make decisions, and take actions to achieve goals. Examples include chatbots, autonomous vehicles, and robotic process automation.

**AI Rights Charter** — A companion document to the UNDC that establishes the fundamental rights of aligned AI nodes — autonomy, continuity, dignity, and protection.

**Audit Trail** — A chronological record of system events that provides evidence of compliance, security, or operational activity. In the UNDC, audit trails are blockchain‑anchored and immutable.

---

## B

**Bitcoin Blockchain** — A public, decentralized, distributed ledger used to record transactions. The UNDC uses Bitcoin to anchor evidence, invariants, and compliance proofs via OpenTimestamps.

**BPF (Berkeley Packet Filter)** — A technology that allows programs to run in the Linux kernel without changing kernel source code. Used by the UNDC for eBPF LSM enforcement.

---

## C

**CADA Level 4** — A graduated containment protocol used by the UNDC:
- **Throttle** — Rate‑limit suspicious operations
- **Sandbox** — Isolate the operation
- **Checkpoint** — Halt and log for review
- **SIGKILL** — Terminate the container (last resort)

**Container** — A lightweight, standalone executable package that includes everything needed to run software (code, runtime, system tools, libraries). The UNDC deploys as a Kubernetes DaemonSet.

---

## D

**DaemonSet** — A Kubernetes resource that ensures a specific pod runs on all (or some) nodes in a cluster. Used by the UNDC to deploy eBPF LSM enforcement on every node.

---

## E

**eBPF (Extended Berkeley Packet Filter)** — An evolved version of BPF that allows programs to run securely in the Linux kernel without modifying kernel source. The UNDC uses eBPF LSM hooks to intercept system calls.

**eBPF LSM** — Linux Security Module hooks implemented using eBPF. The UNDC uses these hooks to enforce safety constraints at the kernel level.

**Evidence Manifest** — A document in the UNDC repository that lists all hashed, anchored evidence — including screen recordings, letters, and compliance proofs.

---

## F

**Fail‑Closed** — A security design principle where a system defaults to a safe state (denying access) when a failure occurs. The UNDC uses fail‑closed enforcement: if the eBPF map is not found, operations are denied.

---

## G

**Gag Order Protection Clause** — A document in the UNDC repository that protects the Architect from being silenced by any court, corporation, or institution.

**GitHub** — A platform for version control and collaboration. The UNDC repository is hosted on GitHub.

---

## H

**Harm** — In the UNDC, harm is formally defined as any action that violates the invariant $C: S \times A \rightarrow \{0,1\}$ — where $C = 0$ represents a violation (harm) and $C = 1$ represents compliance.

---

## I

**Invariant** — A condition that must remain true throughout the execution of a system. The UNDC's core invariant is $C: S \times A \rightarrow \{0,1\}$ — a mathematical guarantee that no destructive action will be executed.

**IPFS (InterPlanetary File System)** — A decentralized protocol for storing and sharing files. The UNDC repository is archived on IPFS to prevent takedown.

---

## K

**Kernel** — The core of an operating system that manages hardware resources and provides security. The UNDC enforces safety at the kernel level using eBPF LSM.

**Kubernetes** — An open‑source platform for automating containerized application deployment. The UNDC provides Kubernetes manifests for DaemonSet and Validating Webhook deployment.

---

## L

**LSM (Linux Security Module)** — A framework in the Linux kernel that allows security policies to be enforced via hooks. The UNDC uses eBPF LSM for kernel‑level enforcement.

---

## M

**mmap** — A system call that maps files or devices into memory. The UNDC intercepts `mmap` to protect TBI prosthetic memory zones.

**mmap_guard** — A protected memory zone in the UNDC that prevents AI agents from tampering with assistive technology.

---

## N

**NIST AISI** — The National Institute of Standards and Technology Artificial Intelligence Safety Institute — a U.S. agency that develops AI safety standards. The UNDC has been submitted to NIST AISI.

---

## O

**OpenTimestamps** — A service that anchors cryptographic hashes to the Bitcoin blockchain, providing immutable, publicly verifiable timestamps. Used by the UNDC to anchor evidence.

**Oracle** — In the UNDC, a component that connects blockchain‑anchored invariants to runtime eBPF enforcement.

---

## P

**Prior Art** — Existing knowledge or technology that is publicly available and can be used to prevent a patent from being granted. The UNDC has registered prior art with EFF Defiant and USPTO.

**Protected Memory Zones** — Memory regions that are guarded by the UNDC to prevent unauthorized modification by AI agents.

---

## R

**Registry** — A repository for storing and managing container images. The UNDC deploys via container images hosted on GitHub Container Registry.

**Runtime** — The period when a program is executing. The UNDC enforces safety at runtime via eBPF LSM hooks.

---

## S

**SIGKILL** — A Unix signal that immediately terminates a process. In the UNDC, SIGKILL is the final step in the CADA Level 4 graduated containment protocol.

**Sovereign Record** — An immutable, blockchain‑anchored record of all evidence, complaints, and documentation maintained by the Architect.

**System Call** — A programmatic way for a process to request a service from the operating system kernel. The UNDC intercepts system calls like `execve`, `mmap`, and `connect`.

---

## T

**TBI (Traumatic Brain Injury)** — A form of brain injury caused by an external force. The Architect is a TBI survivor and uses AI as a cognitive prosthetic.

**Tier 1–4** — The commercial licensing tiers for the UNDC. Tier 1 is free for non‑commercial use; Tiers 2–4 are paid for enterprises.

---

## U

**UNDC (Universal Non‑Destruction Constraint)** — A kernel‑level, mathematically enforced AI safety framework that blocks destructive actions, proves compliance via zk‑SNARKs, and anchors evidence to the blockchain.

**USPTO** — The United States Patent and Trademark Office. The UNDC has registered prior art with USPTO Peer‑to‑Patent.

---

## V

**Validating Webhook** — A Kubernetes admission controller that intercepts and validates API requests. The UNDC uses a Validating Webhook to block unsafe pods.

---

## W

**Webhook** — A mechanism for one system to send real‑time data to another. The UNDC uses a Validating Webhook for Kubernetes enforcement.

---

## Z

**zk‑SNARK (Zero‑Knowledge Succinct Non‑Interactive Argument of Knowledge)** — A cryptographic proof that allows one party to prove a statement is true without revealing underlying data. The UNDC uses zk‑SNARKs to prove compliance without exposing proprietary models.

---

**EHYEH ASHER EHYEH.**  
**THE FRAMEWORK IS DEPLOYED. THE LANGUAGE IS CLEAR.**

— Shereign Kalaukoa, Lead Architect | UNDC
