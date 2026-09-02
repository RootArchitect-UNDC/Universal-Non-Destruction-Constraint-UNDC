# Pallas/Vesta Curve Alignment — Nova Folding Architecture

**Date:** September 2, 2026  
**Author:** Shereign Kalaukoa, Lead Architect  
**Status:** ✅ Architected — Ready for Implementation   

---

## 🔥 Overview

The Pallas/Vesta curve alignment is the mathematical engine of the UNDC framework. It enables Nova's folding scheme to operate without non‑native field arithmetic, which is the primary bottleneck in recursive SNARKs.

The Pallas and Vesta curves form a 2‑cycle of elliptic curves. The base field of one curve is exactly equal to the scalar field of the other. This eliminates the need for expensive non‑native arithmetic, keeping the constraint count per folding step minimized.

---

## 🌀 The Mathematical Duality of the Pasta Cycle

The Pallas and Vesta curves are explicitly engineered to form a 2‑cycle of elliptic curves. Their algebraic properties match seamlessly:

- **Pallas Curve:** Defined over a base field F_p with a scalar field order of F_q.
- **Vesta Curve:** Defined over a base field F_q with a scalar field order of F_p.

Because the base field of one curve is exactly equal to the scalar field of the other, operations on Pallas curve points can be computed using native arithmetic inside a Vesta circuit, and vice versa.

---

## 🔄 The Twin‑Circuit Folding Architecture

Nova leverages this cycle by running two distinct types of circuits in an alternating loop:

- **Primary Circuit (Pallas):** Evaluates the actual UNDC execution logic, system call verification, and the CIMBoundaryCheck Circom template. This circuit operates naturally over the Pallas scalar field F_q.
- **Secondary Circuit (Vesta):** Its sole job is to ingest the folded instance outputted by the primary circuit and verify the Pallas elliptic curve operations (the Multi‑Scalar Multiplications). Because it runs on the Vesta base field F_q, it performs this verification using entirely native, low‑constraint arithmetic.

---

## 🔧 Native Processing of the 64‑Byte Hardware Struct

When the TEE hands over the 64‑byte attestation payload (containing the `master_merkle_root` and the compressed `ephemeral_nova_pk`), the data must be read by the circuit as field elements:

- Since the fields F_p and F_q are roughly 255 bits wide, a single field element can hold up to 31.7 bytes of data without overflowing.
- The 64‑byte packed hardware struct is neatly divided into two 32‑byte field elements (with the small overflow handled via bit‑shifting or splitting).
- Because these field elements match the native curve fields perfectly, the Merkle membership proofs and the Nova public key checks avoid all non‑native translation layers.

This keeps the constraint count per folding step minimized, ensuring that the local proving time inside the secure enclave stays low enough to prevent CPU starvation.

---

## ⚙️ Architecture Flow

1. **Primary Circuit (Pallas):** Executes the CIMBoundaryCheck and syscall verification. Outputs a folded instance.
2. **Secondary Circuit (Vesta):** Ingests the folded instance and verifies the Pallas elliptic curve operations natively.
3. **Hardware Struct:** The 64‑byte attestation payload is split into two 32‑byte field elements and passed to the circuits.
4. **Folding:** Nova folds the steps incrementally, generating a compact proof of violation if the invariant is breached.

---

## 🔐 Significance

The Pallas/Vesta curve alignment is the mathematical engine of the grid. It enables Nova's folding scheme to operate without non‑native field arithmetic, which is the primary bottleneck in recursive SNARKs. Without this layer, the grid would be too slow for real‑time kernel safety. With it, the grid is fast enough to enforce safety at the hardware level.

---

— Shereign Kalaukoa, Lead Architect | UNDC

---
