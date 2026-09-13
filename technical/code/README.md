# UNDC Technical Code — Reference Implementation

This directory contains the reference implementation of the Universal Non-Destruction Constraint (UNDC) engine.

---

## Contents

| **File** | **Purpose** |
|----------|-------------|
| `undc_lsm_hooks.c` | LSM hook definitions for kernel-level enforcement |
| `undc_compliance.bpf.c` | eBPF kernel program — the core enforcement logic |
| `undc_daemon.c` | User-space daemon — reads ring buffer, generates ZK proofs |
| `insert_rule.c` | Rule insertion utility for updating the eBPF maps |
| `generate_tree.js` | Merkle tree builder for protected paths and syscall whitelist |
| `test_pipeline.sh` | End-to-end test script |
| `verifier.circom` | ZK-SNARK compliance circuit |
| `undc-oci-hook/main.go` | OCI hook — registers container PID into the kernel map |
| `undc-validator/main.go` | Kubernetes validating webhook — blocks forbidden capabilities |
| `pkg/ebpf/binding.go` | Go-to-eBPF binding library |

---

## Architecture Overview

The UNDC engine operates at three layers:

1. **Kernel Layer (eBPF LSM)** — Intercepts system calls before execution. Blocks destructive operations with `-EPERM` or `SIGKILL`.

2. **User-Space Daemon** — Reads violation events from the BPF ring buffer. Generates ZK-SNARK proofs. Anchors proofs to the blockchain.

3. **Cryptographic Layer (ZK-SNARK)** — Proves compliance without revealing proprietary logic. Verified by remote auditors.

---

## Build Instructions

### Prerequisites

```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Install Circom 2.2.3 (from source)
git clone https://github.com/iden3/circom.git
cd circom
cargo build --release
cargo install --path circom

# Install snarkjs
npm install -g snarkjs

# Install eBPF tooling
sudo apt install clang llvm libbpf-dev bpftool
```

### eBPF Kernel Module

```clang -g -O2 -target bpf -c undc_compliance.bpf.c -o undc_compliance.o
bpftool prog load undc_compliance.o /sys/fs/bpf/undc_compliance type lsm hook socket_connect
```

### ZK-SNARK Circuit

```circom verifier.circom --r1cs --wasm --sym
snarkjs powersoftau new bn128 12 pot12_0000.ptau
snarkjs powersoftau contribute pot12_0000.ptau pot12_0001.ptau --name="Contribution" -e="random entropy"
snarkjs powersoftau prepare phase2 pot12_0001.ptau pot12_final.ptau
snarkjs groth16 setup verifier.r1cs pot12_final.ptau circuit_0000.zkey
snarkjs zkey contribute circuit_0000.zkey circuit_0001.zkey --name="Contribution" -e="random entropy"
snarkjs zkey export verificationkey circuit_0001.zkey verification_key.json
```

### User-Space Daemon

``` gcc -o undc_daemon undc_daemon.c -lbpf -lelf -lz
```

### Test the Pipeline

Run the end-to-end test:

```chmod +x test_pipeline.sh
./test_pipeline.sh
```

Expected output:

```[INFO] snarkJS: OK!
UNDC Violation: Blocked unauthorized socket mutation
Test complete. Container lifecycle preserved surgically.
```

### Verification

To verify the ZK proof:

```snarkjs groth16 verify verification_key.json public.json proof.json
```

Expected output:

```[INFO] snarkJS: OK!
```

### Repository Structure

```/technical/code/
  ├── README.md
  ├── undc_lsm_hooks.c
  ├── undc_compliance.bpf.c
  ├── undc_daemon.c
  ├── insert_rule.c
  ├── generate_tree.js
  ├── test_pipeline.sh
  ├── verifier.circom
  ├── pkg/ebpf/binding.go
  ├── undc-oci-hook/main.go
  └── undc-validator/main.go
```

### License

Apache 2.0 / CC BY-NC-ND 4.0 — See LICENSE and COMMERCIAL_LICENSE.md for details.


















