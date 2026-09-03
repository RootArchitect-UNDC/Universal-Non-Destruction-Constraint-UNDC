# Merkle Membership Proof — Poseidon-Based Circuit

**Date:** September 2, 2026  
**Author:** Shereign Kalaukoa, Lead Architect  
**Status:** ✅ Architected — Ready for Implementation  

---

## 🔥 Overview

The Merkle Membership Proof circuit is the authentication layer of the UNDC framework. It allows an agent to prove that a specific state transition or syscall is authorized without revealing the entire set of authorized transitions.

The circuit uses the Poseidon hash function, which is optimized for R1CS constraints. Each level of the tree requires fewer than 300 constraints per layer, making it computationally viable for Nova to incrementally fold membership updates inside the TEE.

---

## 🌳 The Merkle Membership Proof Circuit

Below is the implementation structure for checking an allowed transition hash against the master root using the Poseidon hash function.

```circom
pragma circom 2.1.6;

include "node_modules/circomlib/circuits/poseidon.circom";

// n_levels represents the depth of the Merkle tree (e.g., 10 levels allows 1,024 valid states)
template MerkleMembershipProof(n_levels) {
    // Inputs
    signal input leaf;                          // Hash of the current intent/syscall action
    signal input root;                          // The master valid-state root anchored in the TEE
    signal input path_elements[n_levels];       // Neighboring node hashes along the Merkle path
    signal input path_indices[n_levels];        // Binary indicators (0 = left, 1 = right node)

    // Intermediate hashes at each level of the tree
    signal level_hashes[n_levels + 1];
    level_hashes[0] <== leaf;

    // Components for computing hashes at each step up the tree
    component hashers[n_levels];
    component selectors[n_levels][2];

    for (var i = 0; i < n_levels; i++) {
        hashers[i] = Poseidon(2);
        
        // path_indices[i] must be strictly boolean (0 or 1)
        path_indices[i] * (1 - path_indices[i]) === 0;

        // Conditional multiplexing without dynamic if/else branching:
        // If path_indices[i] == 0: Left = current hash, Right = path sibling
        // If path_indices[i] == 1: Left = path sibling, Right = current hash
        hashers[i].inputs[0] <== level_hashes[i] + path_indices[i] * (path_elements[i] - level_hashes[i]);
        hashers[i].inputs[1] <== path_elements[i] + path_indices[i] * (level_hashes[i] - path_elements[i]);

        // Pass the calculated parent hash to the next level
        level_hashes[i + 1] <== hashers[i].out;
    }

    // Invariant Enforcement: The calculated root must exactly match the trusted root
    level_hashes[n_levels] === root;
}

```

⚙️ Architectural Breakdown
1. Zero-Branching Multiplexing
In standard programming, you would check if (index == 0) { hash(left, right) } else { hash(right, left) }. In Circom, this is flattened into an algebraic equation using the path_indices[i] signal as a switch.

The arithmetic statement level_hashes[i] + path_indices[i] * (path_elements[i] - level_hashes[i]) resolves perfectly: if the index is 0, the term cancels out leaving the current hash; if the index is 1, it cleanly simplifies to the sibling hash.

2. Scaling the Tree Constraint Costs
Using a standard SHA-256 hash function inside a circuit requires tens of thousands of constraints per hash operation because SHA-256 relies on bitwise operations that do not map cleanly to prime fields.

By using the Poseidon hash, each level of the tree requires only a fraction of the constraints (typically fewer than 300 R1CS constraints per layer). This makes it computationally viable for Nova to incrementally fold membership updates inside a low-latency TEE environment without bogging down the CPU.

3. Tree Depth and Scalability
The tree depth is set to 10 levels (1,024 valid states). This can be increased or decreased based on the number of allowed transitions. The circuit is designed to scale.

🔐 Significance
The Merkle Membership Proof circuit is the authentication layer of the grid. Without it, the TEE would need to store and verify every allowed state individually. With it, the grid scales while maintaining cryptographic integrity.

---
