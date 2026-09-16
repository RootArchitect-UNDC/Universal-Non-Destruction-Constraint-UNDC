UNDC Production Deployment Guide

Version: 1.1.0 (Current Implementation Status)
Date: September 16, 2026
Repository: https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC

This guide describes the deployment model for the Universal Non-Destruction Constraint (UNDC). It separates components that are implemented and deployed from components that are designed and specified but not yet operational.

Implemented means the code exists in the public repository, compiles, and (where applicable) runs.
Designed means the architecture is specified but the code is not deployed.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. WHAT RUNS TODAY

1.1 Schema Validation (Implemented)

A CI workflow validates the UNDC log format against undc-schema.json on every push. This runs in GitHub Actions and passes.

Workflow file: .github/workflows/validate-undc-logs.yml

What it does:
- Checks out the source tree
- Installs check-jsonschema
- Validates undc-log-sample-v2.json against undc-schema.json
- Fails the build if the schema does not match

1.2 ZK Proof Pipeline (Implemented)

A separate CI workflow compiles the Circom circuits and verifies the Groth16 proof end-to-end.

Workflow file: .github/workflows/undc-test.yml

What it does:
- Compiles verifier.circom and undc_compliance_circuit.circom
- Generates witness
- Runs trusted setup
- Generates proof
- Verifies proof -> [INFO] snarkJS: OK!

1.3 eBPF LSM Hook on bprm_check_security (Implemented — Partial)

A Linux Security Module (LSM) hook written in eBPF. It attaches to bprm_check_security and fires on every execve.

What runs:
- Compiles to undc_compliance.bpf.o
- Loads into a live Linux kernel
- Attaches to bprm_check_security
- Fires on every execve
- Emits events to a userspace ring buffer

What is not yet demonstrated:
- The policy-map lookup currently returns NULL on seeded entries. The -EPERM enforcement branch is implemented in the code but not reached. Tracked as issue #23.

Files:
- technical/code/undc_compliance.bpf.c
- technical/code/undc_daemon.c
- technical/code/Makefile

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

2. LOCAL BUILD AND RUN

2.1 Prerequisites

- Linux kernel with BPF LSM enabled (cat /sys/kernel/security/lsm must include bpf)
- clang, bpftool, libbpf-dev, gcc, make
- circom, snarkjs, node (for ZK pipeline)

2.2 Build

    cd technical/code
    make

This produces:
- undc_compliance.bpf.o — the eBPF object
- undc_compliance.skel.h — the libbpf skeleton
- undc-daemon — the userspace consumer

2.3 Run

    sudo ./undc-daemon /tmp/undc-deny-test

This:
- Loads and attaches the eBPF program
- Seeds the policy map with /tmp/undc-deny-test → DENY
- Consumes ring buffer events and logs them

Expected output:

    Initializing UNDC User-Space eBPF Daemon Gateway...
    Policy seeded: /tmp/undc-deny-test → DENY
    Loop Active: tracking execve, denying /tmp/undc-deny-test
    [eBPF Intercept] syscall=1 pid=... action=ALLOW path="/usr/bin/ls"

Expected output when /tmp/undc-deny-test is executed:

The hook fires, resolves the path, looks it up in the map. Because the lookup currently returns NULL (issue #23), the action is reported as ALLOW. This is the honest current state.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

3. WHAT IS DESIGNED BUT NOT YET DEPLOYED

The following components are specified in documentation and, where noted, have reference code in the repository. None are deployed or tested.

3.1 Phase 1 Extension: Docker Multi-Stage Build Gates (Designed)

A design for injecting check-jsonschema into Docker build stages, so that non-compliant manifests fail before image construction. The design is described in this document's version history; the Dockerfile integration is not implemented.

3.2 Phase 2: Kubernetes DaemonSet for Node-Wide Deployment (Designed)

A design for a DaemonSet that would deploy the eBPF ELF binary across Kubernetes cluster nodes. A reference manifest exists at undc-agent-pod.yaml. It has not been applied to any cluster.

3.3 Network Hook on socket_connect (Designed)

A design for an additional LSM hook on socket_connect, to block network egress to unauthorized addresses. The file technical/code/undc_lsm_net.c exists as a reference, but the hook is not attached, and no test has been run. Reports of a blocked curl command would not be reproducible against the current code.

3.4 Container Runtime Integration (Designed)

A design for an OCI lifecycle hook that registers each new container's PID into a pinned BPF map. Reference code exists at technical/code/undc_oci_hook/main.go. It has not been wired into any container runtime.

3.5 Admission Webhook (Designed)

A design for a Kubernetes validating admission webhook that rejects pods requesting CAP_SYS_ADMIN or CAP_BPF. Reference code exists at technical/code/undc_validator/main.go. It has not been deployed.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

4. POST-DEPLOYMENT VERIFICATION (WHAT ACTUALLY WORKS TODAY)

To verify the LSM hook fires:

1. Start the daemon as described in section 2.3.
2. In another terminal, run any command that calls execve:

       /bin/ls

3. In the daemon terminal, you will see an event:

       [eBPF Intercept] syscall=1 pid=... action=ALLOW path="/usr/bin/ls"

The hook fires and the event flows. The lookup returns NULL and the decision is ALLOW. Tracked as issue #23.

To verify the schema validation runs:

    cd technical/code
    check-jsonschema --schemafile undc-schema.json undc-log-sample-v2.json

Exit code 0 on success.

To verify the ZK circuit:

    cd technical/code
    bash test_pipeline.sh

Expected: [INFO] snarkJS: OK! at the end.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

5. COVERAGE

The current kernel interception surface is documented in COVERAGE.md. It is one hook, on one syscall path (bprm_check_security), attached to one kernel LSM hook. Network hooks, file hooks, mmap hooks, and container-runtime integration are all designed but not implemented.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

6. VERSION HISTORY

Version 1.0.0 | August 19, 2026
Initial guide. Described Phase 1 and Phase 2 as operational. Presented sample output from tests that had not been run.

Version 1.1.0 | September 16, 2026
Rewritten to separate implemented components (schema validation, ZK pipeline, LSM hook on bprm_check_security) from designed components (Docker build gates, DaemonSet, socket_connect hook, OCI hook, admission webhook). Removed fabricated test output.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

— Shereign Kalaukoa
Lead Architect, Universal Non-Destruction Constraint
RootArchitect-UNDC
