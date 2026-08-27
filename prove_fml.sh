#!/bin/bash
set -e
CIRCUIT_NAME="verifier"
PTAU_DIST="pot09_final.ptau"
ZKEY_0="circuit_0000.zkey"
ZKEY_FINAL="circuit_final.zkey"
WITNESS_GEN_DIR="${CIRCUIT_NAME}_js"

echo "========================================================================="
echo "  UNDC Framework: 432 Hz Frequency Mismatch Lockout (FML) Setup & Prover "
echo "========================================================================="

if [ ! -f "${CIRCUIT_NAME}.r1cs" ] || [ ! -d "${WITNESS_GEN_DIR}" ]; then
    echo "[*] Compiling circuit ${CIRCUIT_NAME}.circom..."
    circom "${CIRCUIT_NAME}.circom" --r1cs --wasm
else
    echo "[+] Circuit artifacts found. Skipping compilation."
fi

if [ ! -f "pot09_0000.ptau" ]; then
    echo "[*] Initializing Phase 1 Powers of Tau..."
    npx snarkjs powersoftau new bn128 9 pot09_0000.ptau -v
fi
if [ ! -f "pot09_0001.ptau" ]; then
    echo "[*] Adding local phase 1 entropy contribution..."
    npx snarkjs powersoftau contribute pot09_0000.ptau pot09_0001.ptau --name="UNDC Contributor 1" -v -e="ToxicWasteEntropy1"
fi
if [ ! -f "${PTAU_DIST}" ]; then
    echo "[*] Finalizing Phase 1 beacon..."
    npx snarkjs powersoftau prepare phase2 pot09_0001.ptau "${PTAU_DIST}" -v
fi

if [ ! -f "${ZKEY_0}" ]; then
    echo "[*] Initializing circuit-specific zkey..."
    npx snarkjs groth16 setup "${CIRCUIT_NAME}.r1cs" "${PTAU_DIST}" "${ZKEY_0}"
fi
if [ ! -f "${ZKEY_FINAL}" ]; then
    echo "[*] Contributing Phase 2 circuit entropy..."
    npx snarkjs zkey contribute "${ZKEY_0}" "${ZKEY_FINAL}" --name="UNDC Auditor 2" -v -e="ToxicWasteEntropy2"
fi

if [ ! -f "verification_key.json" ]; then
    echo "[*] Exporting validation artifacts to verification_key.json..."
    npx snarkjs zkey export verificationkey "${ZKEY_FINAL}" verification_key.json
else
    echo "[+] Verification key exists. Skipping export."
fi

if [ -f "input.json" ]; then
    echo "[*] Computing private witness matrix..."
    node "${WITNESS_GEN_DIR}/generate_witness.js" "${WITNESS_GEN_DIR}/${CIRCUIT_NAME}.wasm" input.json witness.wtns

    echo "[*] Manufacturing cryptographically blinded Groth16 zero-knowledge proof..."
    npx snarkjs groth16 prove "${ZKEY_FINAL}" witness.wtns proof.json public.json

    echo "[*] Reviewing assertion proofs against the public verification key..."
    if npx snarkjs groth16 verify verification_key.json public.json proof.json; then
        echo "========================================================================="
        echo "✅ SUCCESS: Proof verified! The frequency baseline satisfies constraints."
        echo "========================================================================="
    else
        echo "========================================================================="
        echo "❌ FAILURE: The assertion proof is structurally invalid or corrupt."
        echo "========================================================================="
        exit 1
    fi
else
    echo "[!] No input.json detected in workspace. Skipping automated witness/proving runs."
    echo "[i] Place one of the test harness payloads below into input.json to execute."
fi
