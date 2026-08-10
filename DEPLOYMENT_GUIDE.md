# UNDC Production Deployment Guide

This guide details the installation of the Universal Non-Destruction Constraint (UNDC) engine across modern containerized nodes. It defines a dual-stage deployment architecture matching Phase 1 (Compile-Time AST Gates) and Phase 2 (Kernel-Level eBPF Interceptors) specifications.

---

## 1. Phase 1: Compile-Time Pipeline Integration (CI/CD Gates)

To enforce the invariant C before execution, validation blocks must be embedded directly into automated build runtimes. This prevents non-compliant dependency graphs from generating deployable container artifacts.

### 1.1 GitHub Actions Workflow Implementation

Create or update `.github/workflows/validate-undc-logs.yml` to run strict pre-build checks over workspace manifests:

```yaml
name: "UNDC Schema Compliance Validation"
on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]
jobs:
  validate:
    runs-on: ubuntu-22.04
    steps:
      - name: Checkout Source Tree
        uses: actions/checkout@v4

      - name: Initialize Python Environment
        uses: actions/setup-python@v5
        with:
          python-version: '3.10'

      - name: Install Invariant Validation CLI
        run: pip install check-jsonschema

      - name: Validate Structural Log Format
        run: |
          check-jsonschema --schemafile undc-schema.json undc-log-sample.json
```

### 1.2 Docker Multi-Stage Build Invariant Protection

Inject validation scripts directly into the base compilation layers of project Dockerfiles to prevent bypasses during manual local image builds:

```dockerfile
# syntax=docker/dockerfile:1
FROM python:3.10-slim AS undc-validator
WORKDIR /anchor
RUN pip install --no-cache-dir check-jsonschema

# Copy invariant blueprints and target manifests
COPY undc-schema.json ./schema.json
COPY undc-log-sample.json ./target-manifest.json

# Perform validation. If exit code != 0, build fails deterministically here
RUN check-jsonschema --schemafile schema.json target-manifest.json

# Secondary operational build phase only reached if structural safety passes
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY --from=undc-validator /anchor /safety_receipt
COPY . .
RUN go build -o secure-binary main.go
```

---

## 2. Phase 2: Kernel-Level Node Deployment (eBPF LSM Hooking)

This phase installs your engine directly into the server kernel. It stops unauthorized actions across all running containers instantly.

### 2.1 Kubernetes DaemonSet Setup

Create a file named `undc-daemonset.yaml`. Paste this text inside it to run the engine automatically on your server nodes:

```yaml
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: undc-kernel-enforcer
  namespace: kube-system
spec:
  selector:
    matchLabels:
      name: undc-enforcer
  template:
    metadata:
      labels:
        name: undc-enforcer
    spec:
      hostNetwork: true
      hostPID: true
      containers:
      - name: ebpf-lsm-engine
        image: rootarchitect/undc-lsm-engine:v1.0.0
        securityContext:
          privileged: true
          capabilities:
            add: ["SYS_ADMIN", "SYS_BPF"]
        volumeMounts:
        - name: sys-kernel-security
          mountPath: /sys/kernel/security
        - name: bpf-fs
          mountPath: /sys/fs/bpf
      volumes:
      - name: sys-kernel-security
        hostPath:
          path: /sys/kernel/security
      - name: bpf-fs
        hostPath:
          path: /sys/fs/bpf
```

### 2.2 Server Verification Checks

Run these commands on your host server terminal. They confirm your system supports the eBPF LSM safety engine:

```bash
# 1. Check if the kernel configuration has LSM enabled
zcat /proc/config.gz | grep CONFIG_BPF_LSM

# 2. Confirm 'bpf' is active in the safety engine stack
cat /sys/kernel/security/lsm
```

### 2.3 Compiling and Activating the Engine

Run these final commands to compile your C file and attach it directly to your operating system:

```bash
# 1. Compile the C code into a safe kernel object
clang -g -O2 -target bpf -c undc_lsm_net.c -o undc_lsm_net.o

# 2. Attach the code directly to the network hook point
bpftool prog load undc_lsm_net.o /sys/fs/bpf/undc_socket_hook type lsm hook socket_connect
```

---

## 3. Post-Deployment Verification

Use these simple steps to test that your safety engine is working.

### 3.1 Run a Test Block

Open a terminal inside any running container on your server. Try to connect to an unapproved network address using this command:

```bash
curl --connect-timeout 2 https://198.51.100.1
```

### 3.2 Confirm the Output Block

If Phase 2 is working correctly, the server kernel will stop the command instantly. You will see this exact message:

```bash
curl: (7) Operation not permitted
```

### 3.3 Check the Safety System Logs

Run this command on your main host server to view the internal safety logs and verify the intervention:

```bash
cat /sys/kernel/debug/tracing/trace_pipe
```

You should see a log entry confirming the exact threat block:  
`UNDC Violation: Blocked unauthorized socket mutation`
