// SPDX-License-Identifier: Apache-2.0
/*
 * UNDC OCI Hook — Container Lifecycle Registration
 *
 * This OCI hook registers the container PID into the kernel-pinned
 * BPF map before the container's user-space instructions execute.
 *
 * Lead Architect: Shereign Kalaukoa
 * Authority: EHYEH ASHER EHYEH & AHYAH
 */

package main

import (
    "encoding/json"
    "fmt"
    "os"
    "path/filepath"

    "github.com/cilium/ebpf"
)

// OCIState represents the OCI runtime state passed via stdin.
type OCIState struct {
    Version string `json:"ociVersion"`
    ID      string `json:"id"`
    Status  string `json:"status"`
    Pid     uint32 `json:"pid"`
}

const pinnedMapPath = "/sys/fs/bpf/undc/pid_map"

func main() {
    // 1. Decode the OCI state payload from stdin.
    var state OCIState
    if err := json.NewDecoder(os.Stdin).Decode(&state); err != nil {
        fmt.Fprintf(os.Stderr, "UNDC OCI Hook Error: failed to decode OCI state payload: %v\n", err)
        os.Exit(1)
    }

    // 2. Validate the container PID.
    if state.Pid == 0 {
        fmt.Fprintf(os.Stderr, "UNDC OCI Hook Error: container PID is invalid (0)\n")
        os.Exit(1)
    }

    // 3. Open the pinned BPF map created by the DaemonSet.
    absoluteMapPath := filepath.Clean(pinnedMapPath)
    pidMap, err := ebpf.LoadPinnedMap(absoluteMapPath, nil)
    if err != nil {
        fmt.Fprintf(os.Stderr, "UNDC OCI Hook Error: failed to open pinned BPF map at %s: %v\n", absoluteMapPath, err)
        os.Exit(1)
    }
    defer pidMap.Close()

    // 4. Register the container PID in the protected-PID map.
    var activeFlag uint8 = 1
    if err := pidMap.Put(state.Pid, activeFlag); err != nil {
        fmt.Fprintf(os.Stderr, "UNDC OCI Hook Error: failed to register PID %d into kernel map: %v\n", state.Pid, err)
        os.Exit(1)
    }

    os.Exit(0)
}
