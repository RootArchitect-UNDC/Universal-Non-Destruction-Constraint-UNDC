#!/bin/bash
# ------------------------------------------------------------
# UNDC Automated Compile & Test Pipeline — v1.0
# Lead Architect: Shereign Kalaukoa
# Authority: EHYEH ASHER EHYEH & AHYAH
# Purpose: Execute end-to-end compilation, witness generation,
#          and zk-SNARK Groth16 verification testing.
# File Hash: f48f556030fb67860eeb365f8aa2835024eac1c541c87578fb4f292d2e7d67de
# ------------------------------------------------------------
set -e

echo "===================================================="
echo "🛡️  Initializing UNDC zk-SNARK Test Pipeline"
echo "===================================================="

# 1. Prerequisites Check
if ! command -v circom &> /dev/null; then
    echo "❌ Error: circom compiler is not installed."
    exit 1
fi
if ! command -v snarkjs &> /dev/null; then
    echo "❌ Error: snarkjs CLI is not installed (run 'npm install -g snarkjs')."
    exit 1
fi

# 2. Cleanup old artifacts
echo "🧹 Cleaning up previous test artifacts..."
rm -rf build circuit_js input.json proof.json public.json verification_key.json
mkdir -p build

# 3. Compile the Circom Circuit
echo "📦 Step 1: Compiling undc_compliance_circuit.circom to R1CS..."
circom undc_compliance_circuit.circom --r1cs --wasm --sym --output ./build

# 4. Generate Off-Chain Test Inputs
echo "🌳 Step 2: Running generate_tree.js to build Merkle Tree & input.json..."
if [ -f "generate_tree.js" ]; then
    node generate_tree.js
else
    echo "❌ Error: generate_tree.js not found in current directory."
    exit 1
fi
mv input.json build/input.json

# 5. Compute the Witness payload
echo "🧮 Step 3: Computing witness payload via WebAssembly runtime..."
node build/undc_compliance_circuit_js/generate_witness.js \
     build/undc_compliance_circuit_js/undc_compliance_circuit.wasm \
     build/input.json \
     build/witness.wtns

# 6. Local Mock Trusted Setup (Groth16)
echo "🔑 Step 4: Initiating local mock Groth16 Power of Tau setup..."
snarkjs groth16 setup build/undc_compliance_circuit.r1cs bn128_tau_exec.ptau build/undc_pk.zkey 2>/dev/null || {
    echo "ℹ️  No universal ptau file found. Generating an ephemeral local setup for testing..."
    snarkjs powersoftau new bn128 12 build/pot12_0000.ptau -v
    snarkjs powersoftau contribute build/pot12_0000.ptau build/pot12_0001.ptau --name="UNDC Contributor 1" -v -e="some_random_entropy"
    snarkjs powersoftau prepare phase2 build/pot12_0001.ptau build/pot12_final.ptau -v
    snarkjs groth16 setup build/undc_compliance_circuit.r1cs build/pot12_final.ptau build/undc_pk.zkey
}
snarkjs zkey export verificationkey build/undc_pk.zkey build/verification_key.json

# 7. Create the Zero-Knowledge Proof
echo "✍️  Step 5: Minting Zero-Knowledge compliance proof file..."
snarkjs groth16 prove build/undc_pk.zkey build/witness.wtns build/proof.json build/public.json

# 8. Verify the Proof (Local Validation Check)
echo "🔍 Step 6: Running local verification test check..."
if snarkjs groth16 verify build/verification_key.json build/public.json build/proof.json; then
    echo "===================================================="
    echo "✅ LOCAL SELF-TEST SUCCESSFUL"
    echo "⚙️  The Merkle proof pipeline is structurally self-consistent."
    echo "📊 Proof successfully verified against the mock local keys."
    echo "===================================================="
else
    echo "❌ CRITICAL ERROR: Proving system mismatch. Verification Failed."
    exit 1
fi
