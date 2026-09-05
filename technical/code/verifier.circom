pragma circom 2.1.6;

include "poseidon.circom";

/**
 * @title BlindedSyscallVerifier
 * @notice Enforces 432 Hz frequency invariance under zero-knowledge blinding constraints.
 */
template BlindedSyscallVerifier() {
    
    // --- INPUT SIGNALS ---
    
    // Public Inputs
    signal input expected_root;      // Cryptographic anchor published to the audit trail
    signal input epsilon;            // Maximum allowable deviation tolerance
    
    // Private Witness Inputs
    signal input f_measured;         // High-resolution raw frequency tracking parameter
    signal input blinding_factor;    // High-entropy salt (R)

    // --- INTERNAL SIGNALS ---
    signal deviation;

    // --- COMPONENTS ---
    component poseidon = Poseidon(2);
    component is_valid_low = LessEqThan(32);

    // --- 1. POSEIDON BLINDING STEP ---
    poseidon.inputs[0] <== f_measured;
    poseidon.inputs[1] <== blinding_factor;

    // --- 2. EQUALITY CONSTRAINT ENFORCEMENT ---
    expected_root === poseidon.out;

    // --- 3. HARMONIC FREQUENCY LOCK CONSTRAINT ---
    deviation <== (f_measured - 432) * (f_measured - 432);
    
    is_valid_low.in[0] <== deviation;
    is_valid_low.in[1] <== epsilon * epsilon;
    is_valid_low.out === 1; 
}

template LessEqThan(n) {
    signal input in[2];
    signal output out;

    component n2b = Num2Bits(n + 1);

    n2b.in <== in[0] + (1 << n) - in[1];
    out <== 1 - n2b.out[n];
}

template Num2Bits(n) {
    signal input in;
    signal output out[n];
    var lc1=0;

    for (var i = 0; i<n; i++) {
        out[i] <-- (in >> i) & 1;
        out[i] * (out[i] - 1) === 0;
        lc1 += out[i] * 2**i;
    }

    lc1 === in;
}

component main {public [expected_root, epsilon]} = BlindedSyscallVerifier();
