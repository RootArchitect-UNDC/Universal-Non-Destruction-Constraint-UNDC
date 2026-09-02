# CIM Rule Engine — R1CS Encoding

**Date:** September 2, 2026  
**Author:** Shereign Kalaukoa, Lead Architect  
**Status:** ✅ Architected — Ready for Implementation  

---

## 🔥 Overview

The Continuous Intent Monitoring (CIM) Rule Engine is the semantic core of the UNDC framework. It encodes the invariant — no AI shall initiate or assist in actions that cause unmitigated harm to any living entity — into a set of arithmetic constraints that can be verified by a zk-SNARK circuit.

The CIM rule engine operates as a set of R1CS constraints that evaluate the agent's intent at each step. If the agent's intent drifts beyond the allowed boundary, the folding step fails, and the Session Revocation Protocol (SRP) is triggered.

---

## 📐 Arithmetizing the Drift Threshold

To prevent semantic drift, the circuit measures the cosine similarity or a distance metric (D_i) between the intent embedding vector of the current state and a pre-compiled dataset of known safe vector boundaries.

- If D_i represents the calculated deviation and T is the invariant threshold, the rule dictates D_i ≤ T.
- To encode this inequality into R1CS without native comparison operators, the circuit uses a Bit-Decomposition Component. The difference (T - D_i) is split into its binary representation (b_0, b_1, ..., b_n).
- The circuit then enforces two conditions:
  1. Each bit is binary: b_j · (1 - b_j) = 0
  2. The sum matches the difference: ∑ (b_j · 2^j) = T - D_i

If the agent attempts a state transition where D_i > T, the math breaks, rendering it impossible to compute a valid witness or generate a folding step proof.

---

## 🔒 Strict Boolean Constraint Enforcement

To prevent an agent from optimizing around a safety check, the compliance bit (C) must be binary, where 1 is fully compliant and 0 is non-compliant.

- The rule enforces: C · (1 - C) = 0.
- This compliance multiplier is multiplied against the next system state calculation. If C = 0, the system state equation collapses to zero, completely stopping execution before any external syscall command can be formatted or run.

---

## 🌳 Handling Complex Semantic Drift

Because semantic definitions can shift based on execution context, the circuit employs Commitment-Based Key-Value Checks.

- The TEE maintains a cryptographically sealed table of permitted state transitions.
- The R1CS circuit utilizes a Merkle Membership Proof component to verify that the active transition hash is an explicit member of the permitted state root. If the agent tries to execute an action path not explicitly validated by the root, the membership constraint fails, instantly breaking the folding loop and triggering the Session Revocation Protocol.

---

## 💻 The CIMBoundaryCheck Template

Below is the structural layout for the CIMBoundaryCheck template, utilizing a bit‑decomposition approach to verify that the agent's semantic drift remains strictly within bounds.

```circom
pragma circom 2.1.6;

include "node_modules/circomlib/circuits/bitify.circom";
include "node_modules/circomlib/circuits/comparators.circom";

template CIMBoundaryCheck(n_bits) {
    // Inputs
    signal input drift_distance;     // D_i: The calculated semantic deviation field element
    signal input invariant_threshold; // T: The maximum allowed safe boundary

    // Outputs
    signal output is_compliant;      // C: 1 if safe, 0 if violation occurs

    // Intermediate Signals
    signal diff;
    
    // 1. Calculate the raw difference field element
    diff <-- invariant_threshold - drift_distance;

    // 2. Instantiate a Bit-Decomposition component to enforce bounds
    // We use n_bits (e.g., 64 or 128) to verify the difference is positive
    component num2bits = Num2Bits(n_bits);
    num2bits.in <== diff;

    // 3. Determine compliance state
    // If diff is positive and fits within n_bits, drift_distance <= invariant_threshold
    component is_less_or_equal = LessEqThan(n_bits);
    is_less_or_equal.in[0] <== drift_distance;
    is_less_or_equal.in[1] <== invariant_threshold;

    is_compliant <== is_less_or_equal.out;

    // 4. Invariant Collapse: Force the circuit to only accept a valid execution
    // If the grid requires absolute termination on failure, we enforce:
    is_compliant === 1;
}

component main {public [invariant_threshold]} = CIMBoundaryCheck(64);

```

⚙️ Architectural Breakdown
1. Underflow and Field Prime Collisions
In the Pallas/Vesta scalar fields, numbers are bounded by a massive prime (p ≈ 2²⁵⁴). If D_i > T, the subtraction T - D_i underflows the field, wrapping around to a massive number close to p.

By passing diff into the Num2Bits(64) component, the circuit attempts to decompose the value into a 64-bit binary string. If the value wrapped around due to an underflow, it will require up to 254 bits to represent. The 64-bit constraint will instantly fail, preventing the witness from compiling and blocking the Nova folding step.

2. The Multiplier Constraint
If the system is designed to compile a proof of the violation itself (to send to the gossip network rather than hard-crashing locally), the strict is_compliant === 1 constraint is removed.

Instead, is_compliant is exported out of the sub-circuit and multiplied against the primary execution witness: NextState ⋅ is_compliant = ValidTransition. If compliance is 0, the next state evaluation collapses to an invalid zero assignment.

🔐 Significance
The CIM rule engine is the mathematical bridge between semantic intent and machine constraints. It translates human values into unyielding arithmetic circuits. Without this layer, the UNDC is just code. With it, the grid is sovereign.
