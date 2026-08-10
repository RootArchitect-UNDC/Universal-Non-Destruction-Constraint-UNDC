#!/bin/bash
# UNDC eBPF Engine Installation Bootstrap Script
# This script compiles and installs the eBPF LSM hooks on target Kubernetes nodes.
# Must be run as root on the host node.

set -e

echo "[UNDC BOOTSTRAP] Starting eBPF engine installation..."

# 1. Verify kernel supports eBPF LSM
echo "[UNDC BOOTSTRAP] Checking kernel configuration..."
if ! zcat /proc/config.gz 2>/dev/null | grep -q "CONFIG_BPF_LSM=y"; then
    echo "[UNDC ERROR] CONFIG_BPF_LSM is not enabled. Please rebuild kernel with eBPF LSM support."
    exit 1
fi

if ! cat /sys/kernel/security/lsm 2>/dev/null | grep -q "bpf"; then
    echo "[UNDC ERROR] bpf LSM is not active. Add 'bpf' to the lsm= kernel parameter."
    exit 1
fi

# 2. Install required dependencies
echo "[UNDC BOOTSTRAP] Installing build dependencies..."
apt-get update -y
apt-get install -y clang llvm libbpf-dev linux-tools-common linux-tools-$(uname -r) bpftool

# 3. Verify clang and bpftool
if ! command -v clang &> /dev/null; then
    echo "[UNDC ERROR] clang not found. Please install clang-14 or later."
    exit 1
fi

if ! command -v bpftool &> /dev/null; then
    echo "[UNDC WARNING] bpftool not found. Installing via linux-tools..."
    apt-get install -y linux-tools-$(uname -r)
fi

# 4. Compile the eBPF LSM program
echo "[UNDC BOOTSTRAP] Compiling undc_lsm_net.c..."
clang -g -O2 -target bpf -D__TARGET_ARCH_x86 -c undc_lsm_net.c -o undc_lsm_net.o

# 5. Attach the program to the kernel
echo "[UNDC BOOTSTRAP] Attaching eBPF LSM hook to kernel..."
bpftool prog load undc_lsm_net.o /sys/fs/bpf/undc_socket_hook type lsm hook socket_connect

# 6. Verify attachment
echo "[UNDC BOOTSTRAP] Verifying attachment..."
if bpftool prog show name undc_socket_hook &> /dev/null; then
    echo "[UNDC SUCCESS] eBPF LSM hook attached successfully."
else
    echo "[UNDC ERROR] Failed to attach eBPF LSM hook."
    exit 1
fi

echo "[UNDC BOOTSTRAP] eBPF engine installation complete."
echo "[UNDC BOOTSTRAP] The grid is strong."
