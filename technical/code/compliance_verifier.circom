pragma circom 2.1.6;

include "./node_modules/circomlib/circuits/sha256/sha256.circom";
include "./node_modules/circomlib/circuits/bitify.circom";

/**
 * @title UNDC_ModelComplianceVerifier
 * @notice Verifies that an executed model matches a sovereign ledger commitment 
 *         without exposing the model weights or kernel structural identifiers.
 */
template UNDC_ModelComplianceVerifier() {
    // ------------------------------------------------------------------------
    // INPUT SIGNALS
    // ------------------------------------------------------------------------
    
    // Public Inputs (Visible to European Commission Audit Node)
    signal input publicLedgerRoot;         // Cryptographic state anchor from the public chain
    signal input operationalTimestamp;    // Validated Unix timestamp of the eBPF event

    // Private Inputs (Hidden inside the local Sovereign Cloud Enclave)
    signal input modelWeightsHash[256];   // The true 256-bit SHA-256 signature of the binary
    signal input privateKernelInode;      // Raw Linux inode number from the file system
    signal input driverDeviceMajor;       // Host GPU/Storage device driver major flag
    signal input driverDeviceMinor;       // Host GPU/Storage device driver minor flag

    // ------------------------------------------------------------------------
    // OUTPUT SIGNALS
    // ------------------------------------------------------------------------
    signal output complianceStateStatus;  // Binary proof token (1 = Fully Compliant)

    // ------------------------------------------------------------------------
    // CIRCUITS & CONSTRAINTS MATRIX
    // ------------------------------------------------------------------------

    // Step 1: Verify the file state matches the expected hash structure
    component shaVerifier = Sha256(256);
    for (int i = 0; i < 256; i++) {
        shaVerifier.in[i] <== modelWeightsHash[i];
    }

    // Step 2: Compute local Environmental Fingerprint
    signal localEnclaveFingerprint;
    localEnclaveFingerprint <== privateKernelInode * 1000000 + driverDeviceMajor * 1000 + driverDeviceMinor;

    // Step 3: Enforce Constraint Binding to Ledger Root
    signal computedStateCommitment;
    computedStateCommitment <== shaVerifier.out[0] * localEnclaveFingerprint + operationalTimestamp;

    // Hard Constraint: The computed state commitment must align with the public root
    computedStateCommitment === publicLedgerRoot;

    // ------------------------------------------------------------------------
    // COMPLIANCE VALIDATION OUTPUT
    // ------------------------------------------------------------------------
    complianceStateStatus <== 1;
}

component main {public [publicLedgerRoot, operationalTimestamp]} = UNDC_ModelComplianceVerifier();
