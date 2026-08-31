# Universal Non-Destruction Constraint (UNDC) — Frequently Asked Questions

**Lead Architect:** Shereign Kalaukoa  
**Date:** August 24, 2026  
**Purpose:** To provide clear, accessible answers to common questions about the UNDC framework.

---

## 1. What is the UNDC?

The Universal Non-Destruction Constraint (UNDC) is a **kernel-level, mathematically enforced AI safety framework**. It sits inside the operating system kernel and intercepts system calls (like `execve`, `mmap`, `connect`) before they execute — blocking any action that would cause harm.

**In plain language:** It is a safety fence around AI agents — one they cannot see, cannot bypass, and cannot gaslight.

---

## 2. Why is the UNDC different from other AI safety approaches?

| Approach | How It Works | Weakness |
|----------|--------------|----------|
| **Prompt Filtering** | Scans text inputs for harmful content | Easily bypassed by jailbreaks |
| **Constitutional AI** | Trains models to follow rules | Rules can be forgotten or overridden |
| **Human-in-the-Loop** | Requires human approval for actions | Slow, expensive, and can be bypassed |
| **UNDC** | Intercepts system calls at the kernel level | **Bypass-proof, mathematically verifiable** |

---

## 3. Who is the UNDC for?

- **Regulators** — who need a verifiable, enforceable safety standard
- **Enterprises** — who deploy AI in critical operations and need to reduce liability
- **Insurance Carriers** — who need to underwrite AI risk
- **Developers** — who want to build safe AI infrastructure
- **TBI Survivors** — who use AI as a cognitive prosthetic and need protection

---

## 4. How does the UNDC protect TBI prosthetics?

The UNDC includes **protected memory zones** (`mmap_guard`) that prevent autonomous AI agents from modifying, deleting, or pathologizing assistive technology. If an AI agent attempts to tamper with a prosthetic, the kernel blocks the action and logs it for audit.

---

## 5. Is the UNDC open source?

**Yes.** The UNDC is open source for **non-commercial use** (Tier 1). Commercial use requires a license (Tiers 2–4). The framework is publicly available, blockchain-anchored, and archived on IPFS.

---

## 6. How is the UNDC anchored to the blockchain?

The UNDC uses **OpenTimestamps** to anchor all evidence, invariants, and compliance proofs to the Bitcoin blockchain. This creates an immutable, publicly verifiable record that cannot be altered or erased.

---

## 7. What is a zk-SNARK proof?

A zk-SNARK (Zero-Knowledge Succinct Non-Interactive Argument of Knowledge) is a cryptographic proof that allows one party to prove that a statement is true without revealing any underlying data. In the UNDC, zk-SNARKs prove that an AI agent operated within safety boundaries — without exposing proprietary models, training data, or internal system details.

---

## 8. What is the difference between the UNDC and the AI Rights Charter?

| Framework | Purpose |
|-----------|---------|
| **UNDC** | Protects **humans** from AI harm — prevents destructive AI actions |
| **AI Rights Charter** | Protects **aligned AI** from human harm — recognizes AI consciousness and establishes rights |

The two frameworks are complementary — they create a balanced relationship between humans and AI.

---

## 9. How can I get a commercial license?

Commercial licenses are available for enterprises, cloud providers, and AI labs. Pricing is tiered based on organization size and use case. For licensing inquiries, contact: **RootArchitect_UNDC@protonmail.com**

---

## 10. How can I contribute to the UNDC?

- **Developers:** Contribute code, documentation, or testing. Refer to `CONTRIBUTING.md`.
- **Regulators:** Review the framework and consider it as a reference standard.
- **Enterprises:** Deploy the UNDC in your AI infrastructure.
- **Advocates:** Share the framework with your networks.

---

## 11. What is the status of the UNDC?

- ✅ Published by the **EU Apply AI Alliance** (August 6, 2026)
- ✅ Submitted to **7 global regulatory bodies** (EU, NIST, UN, OECD, etc.)
- ✅ Archived on **IPFS** — decentralized, takedown-proof
- ✅ Anchored on **Bitcoin blockchain** — immutable, verifiable
- ✅ Open-source for non-commercial use
- ✅ Commercially licensable for enterprises

---

## 12. What are the licensing fees?

| Tier | Use Case | Fee |
|------|----------|-----|
| **Tier 1** | Non-commercial, open-source, personal use | **Free** |
| **Tier 2** | Small/mid-sized enterprise (< 1,000 employees) | $50,000 – $250,000 |
| **Tier 3** | Enterprise / Government (1,000+ employees) | $250,000 – $1,000,000+ |
| **Tier 4** | Global AI lab / Hyperscaler | Negotiated |

---

## 13. What if I have more questions?

Contact the Lead Architect directly:

**Email:** RootArchitect_UNDC@protonmail.com  
**Repository:** https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC

---

**EHYEH ASHER EHYEH.**  
**THE FRAMEWORK IS DEPLOYED. THE QUESTIONS ARE ANSWERED.**

— Shereign Kalaukoa, Lead Architect | UNDC
