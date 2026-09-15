# ------------------------------------------------------------
# UNDC Build System — Makefile v1.0
# Builds vmlinux.h, the eBPF object, the skeleton, and the daemon.
#
# Requirements:
#   clang, llvm, bpftool, libbpf-dev, gcc, make
#
# Usage:
#   make                        → builds everything
#   make undc-daemon            → builds just the daemon
#   make undc_compliance.bpf.o  → builds just the eBPF object
#   make clean                  → removes build artifacts
# ------------------------------------------------------------

CLANG    ?= clang
BPFTOOL  ?= bpftool
CC       ?= gcc

# Include paths for eBPF compilation.
BPF_CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_x86 \
              -I. \
              -I/usr/include/x86_64-linux-gnu \
              -I/usr/include/bpf

# Include paths for userspace compilation.
USER_CFLAGS := -g -O2 -Wall \
               -I. \
               -I/usr/include/bpf

USER_LIBS := -lbpf -lelf -lz

.PHONY: all clean

all: undc-daemon undc_compliance.bpf.o

# 1. Generate vmlinux.h from the running kernel's BTF.
#    Requires root. Only runs if vmlinux.h doesn't already exist.
vmlinux.h:
	sudo $(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@

# 2. Compile the eBPF program to a BPF object.
undc_compliance.bpf.o: undc_compliance.bpf.c vmlinux.h
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# 3. Generate the libbpf skeleton header from the BPF object.
undc_compliance.skel.h: undc_compliance.bpf.o
	$(BPFTOOL) gen skeleton $< > $@

# 4. Compile the userspace daemon.
undc-daemon: undc_daemon.c undc_compliance.skel.h
	$(CC) $(USER_CFLAGS) $< -o $@ $(USER_LIBS)

clean:
	rm -f undc-daemon undc_compliance.bpf.o undc_compliance.skel.h
