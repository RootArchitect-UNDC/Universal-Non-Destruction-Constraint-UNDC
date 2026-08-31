// ------------------------------------------------------------
// UNDC zk-SNARK Compliance Circuit — v1.4 (Blinded Merkle Path)
// Lead Architect: Shereign Kalaukoa
// Authority: EHYEH ASHER EHYEH & AHYAH
// Purpose: Prove a syscall belongs to an authorized UNDC Merkle Root
//          with blinding to prevent brute-force enumeration
// Target: EU AI Office, NIST AISI auditors
// ------------------------------------------------------------

pragma circom 2.1.8;

include "circomlib/poseidon.circom";
include "circomlib/switcher.circom"; // Fixed path from switches.circom

// ------------------------------------------------------------
// 1. BLINDED MERKLE PROOF VERIFICATION TEMPLATE
// ------------------------------------------------------------
template MerkleProofVerifyBlinded(depth) {
    // PUBLIC INPUTS (Auditors see these)
    signal input expected_root;      // The public Merkle Root of the safe UNDC policy

    // PRIVATE INPUTS (Hidden from auditors)
    signal input syscall_type;       // The system call being executed (now private!)
    signal input syscall_salt;       // Blinding factor (random nonce)
    signal input path_elements[depth]; // Sibling hashes along the Merkle path
    signal input path_indices[depth];  // 0 if sibling is on the right, 1 if left

    // Intermediate hashes tracking up the tree
    signal level_hashes[depth + 1];

    // BLIND THE LEAF: hash(syscall_type, syscall_salt) before entering the tree
    component leaf_hasher = Poseidon(2);
    leaf_hasher.inputs[0] <== syscall_type;
    leaf_hasher.inputs[1] <== syscall_salt;
    level_hashes[0] <== leaf_hasher.out;

    // Instantiate hashers and switches dynamically for each level of the tree
    component hashers[depth];
    component selectors[depth];

    for (var i = 0; i < depth; i++) {
        // Enforce that path indices must be binary (0 or 1)
        path_indices[i] * (path_indices[i] - 1) === 0;

        hashers[i] = Poseidon(2);
        selectors[i] = Switcher();

        // Switcher arranges inputs: if path_indices[i] is 0, level_hashes[i] stays left
        selectors[i].sel <== path_indices[i];
        selectors[i].L <== level_hashes[i];
        selectors[i].R <== path_elements[i];

        // Feed correctly ordered left/right inputs into the Poseidon hasher
        hashers[i].inputs[0] <== selectors[i].outL;
        hashers[i].inputs[1] <== selectors[i].outR;

        // Store the computed hash as the input for the next level up
        level_hashes[i + 1] <== hashers[i].out;
    }

    // STRICT ENFORCEMENT: The final computed hash MUST equal the public root
    level_hashes[depth] === expected_root;
}

// ------------------------------------------------------------
// 2. MAIN COMPLIANCE CIRCUIT
// ------------------------------------------------------------
// Depth 4 supports 2^4 = 16 distinct rule paths — scale to depth 32 or 64 for production
// syscall_type is now PRIVATE — auditors cannot brute-force the set
component main {public [expected_root]} = MerkleProofVerifyBlinded(4);
