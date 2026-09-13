// SPDX-License-Identifier: Apache-2.0
/*
 * UNDC Go-eBPF Binding Library
 *
 * This library provides the Go interface for loading, attaching, and
 * managing the UNDC eBPF enforcement program.
 *
 * Lead Architect: Shereign Kalaukoa
 * Authority: EHYEH ASHER EHYEH & AHYAH
 */

package ebpf

import (
    "fmt"

    "github.com/cilium/ebpf"
    "github.com/cilium/ebpf/link"
    "github.com/cilium/ebpf/rlimit"
)

// UNDCProgramManager manages the lifecycle of the UNDC eBPF program.
type UNDCProgramManager struct {
    CollectionSpec *ebpf.CollectionSpec
    Collection     *ebpf.Collection
    LSMLink        link.Link
}

// NewUNDCManager loads the compiled eBPF bytecode and prepares it for attachment.
func NewUNDCManager(bytecodePath string) (*UNDCProgramManager, error) {
    // Remove the memlock limit to allow eBPF map creation.
    if err := rlimit.RemoveMemlock(); err != nil {
        return nil, fmt.Errorf("failed to remove memlock limits: %w", err)
    }

    // Load the compiled eBPF object file.
    spec, err := ebpf.LoadCollectionSpec(bytecodePath)
    if err != nil {
        return nil, fmt.Errorf("failed to load eBPF bytecode asset: %w", err)
    }

    return &UNDCProgramManager{CollectionSpec: spec}, nil
}

// EnforceInvariants attaches the eBPF program to the kernel LSM hooks and
// populates the protected paths and syscall whitelist maps.
func (m *UNDCProgramManager) EnforceInvariants(protectedPaths []string, allowedSyscalls []uint32) error {
    // Instantiate the eBPF collection in the kernel.
    coll, err := ebpf.NewCollection(m.CollectionSpec)
    if err != nil {
        return fmt.Errorf("failed to instantiate eBPF collection in kernel: %w", err)
    }
    m.Collection = coll

    // Populate the protected paths map.
    protectedMap := m.Collection.Maps["undc_protected_paths"]
    for i, path := range protectedPaths {
        pathBytes := append([]byte(path), 0)
        if err := protectedMap.Put(uint32(i), pathBytes); err != nil {
            return fmt.Errorf("failed to load protected path index %d into kernel map: %w", i, err)
        }
    }

    // Populate the syscall whitelist map.
    syscallMap := m.Collection.Maps["undc_syscall_whitelist"]
    for _, syscallID := range allowedSyscalls {
        var active uint8 = 1
        if err := syscallMap.Put(syscallID, active); err != nil {
            return fmt.Errorf("failed to authorize syscall ID %d in kernel space: %w", syscallID, err)
        }
    }

    // Attach the LSM program to the kernel.
    lsmProgram := m.Collection.Programs["undc_lsm_handler"]
    l, err := link.AttachLSM(link.LSMOptions{
        Program: lsmProgram,
    })
    if err != nil {
        return fmt.Errorf("failed to attach atomic eBPF LSM security layer: %w", err)
    }
    m.LSMLink = l

    return nil
}

// Close detaches the LSM hook and releases the eBPF collection.
func (m *UNDCProgramManager) Close() error {
    if m.LSMLink != nil {
        m.LSMLink.Close()
    }
    if m.Collection != nil {
        m.Collection.Close()
    }
    return nil
}
