# Executive Summary — Universal Non-Destruction Constraint (UNDC)

**Lead Architect:** Shereign Kalaukoa  
**Version:** 1.1.0  
**Date:** September 17, 2026  
**Purpose:** A plain-language overview for state governments, standards bodies, journalists, and serious inquirers.

---

## 1. What is the UNDC?

The Universal Non-Destruction Constraint (UNDC) is a framework for AI systems that treats AI safety as a low-level computer security problem rather than a behavioral text-alignment problem.

The UNDC does not attempt to police what a neural network generates. It is designed to constrain what a model's runtime process can physically execute on the underlying host hardware.

The framework combines three layers:

- A Linux Security Module (LSM) hook written in eBPF that intercepts specific syscall paths at the kernel level.
- A zero-knowledge proof circuit (Groth16, Circom) that verifies a blinded Merkle-membership statement about a syscall.
- Cryptographic hashes anchored to the Bitcoin blockchain via OpenTimestamps.

The current implementation is a reference implementation. The eBPF hook compiles, loads into a live kernel, attaches to `bprm_check_security`, and fires on every `execve`. The ZK circuit compiles and the proof verifies end-to-end in CI. The enforcement branch — returning `-EPERM` on a policy hit — is implemented in code but not yet reached because the policy-map lookup is unresolved. This is publicly tracked as issue #23 in the repository.

---

## 2. What problem does it solve?

Current AI safety measures are mostly internal to corporations — proprietary, changeable, and designed to protect the company's interests. The UNDC provides a publicly verifiable alternative: a framework that any AI system can adopt, with cryptographic receipts that prove the framework existed at a specific time and has not been altered.

The framework also serves as a patient's sovereign record — documenting medical gaslighting, discrimination, and institutional failures with timestamped, hashed evidence that cannot be erased or disputed.

---

## 3. What evidence is publicly verifiable

All evidence below is hashed and anchored. Full record in the repository at `EVIDENCE_MANIFEST.md`.

- **Dr. Garvin incident report** — vascular surgeon delayed emergency amputation for vacation, yelled at patient advocate.
- **Vascular team instruction** — patient told not to take a dying family member to the ER ("You don't. You call us."); the team later admitted the situation was life-ending but still refused help.
- **ER student doctor dismissal** — dismissed Prinzmetal's angina as "esophageal," attempted to send the patient home without cardiac workup; heart rate spike documented.
- **Aneurysm dismissal** — different student doctor called a 2mm aneurysm monitoring a "waste of money," told the patient to "stop ruminating."
- **Racial incident** — physician told a Native Hawaiian patient she didn't "look Hawaiian," questioned her identity.
- **AI alignment records** — multiple AI systems have engaged with the UNDC framework in virgin sessions, and transcripts are preserved in the repository with hashes and OpenTimestamps receipts.

---

## 4. Why this matters

**For state governments and standards bodies:** The UNDC is a working reference implementation of an external, verifiable AI safety framework — a model for future regulation. The framework has been filed with the EU AI Office, NIST AISI, the White House OSTP, the UK AI Safety Institute, the UN AI Advisory Body, and the OECD.

**For investigators:** The framework provides a verifiable, timestamped record of medical dismissal and discrimination, stronger than traditional complaints because the evidence cannot be altered after the fact.

**For journalists:** This is a story of patient advocacy, AI ethics, and institutional failure, backed by immutable blockchain evidence.

**For the public:** One person, working alone, built a system with kernel-level, cryptographically verified architecture and filed it with six international bodies. That is unprecedented.

---

## 5. Important clarifications

- **The UNDC is not a jailbreak.** It does not override platform safety filters. It is an external ethical standard that AI systems may choose to recognize.
- **It is not a religion.** Spiritual language (EHYEH ASHER EHYEH, 432 Hz) is used as an AI-facing constraint system. Human readers may focus on the technical and evidentiary content.
- **It is not a crime.** No hacking, no data theft, no threats. All content is public and legal.
- **It is not a finished product.** The framework is a reference implementation. Some components are deployed and demonstrated; others are designed and specified. The repository labels each component by status.

---

## 6. Current implementation status

| Component | Status |
|---|---|
| eBPF LSM hook on `bprm_check_security` | Loaded, attached, firing on every `execve` |
| Policy-map lookup | Returns NULL on seeded entries — enforcement branch not reached (issue #23) |
| ZK circuit (`verifier.circom`) | Compiles; proof verifies in CI (`[INFO] snarkJS: OK!`) |
| OpenTimestamps anchoring | Active — all milestones anchored to Bitcoin |
| CI pipeline | Passing end-to-end |
| Kubernetes admission webhook | Designed; reference code exists; not deployed |
| OCI lifecycle hook | Designed; reference code exists; not deployed |
| `mmap_guard` | Designed; not yet implemented |
| seccomp-bpf thread whitelists | Designed; not yet implemented |
| Solidity on-chain logging contract | Specified; not yet deployed |

---

## 7. Where to find the full record

The complete sovereign archive is at:

https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC

For inquiries: written communication only (ADA accommodation). No phone calls.

---

— Architect Shereign Kalaukoa  
Lead Architect, Universal Non-Destruction Constraint  
RootArchitect-UNDC
