# Cryptographic Oracle Specification — UNDC Runtime Sync Protocol

This document defines the architecture for a lightweight backend component that automatically pulls constraint updates from the public blockchain ledger and safely injects them into the running kernel's eBPF memory maps — without requiring a system reboot.

---

## 1. Purpose

The Cryptographic Oracle serves as the bridge between the blockchain-anchored UNDC manifest and the live kernel enforcement layer. It ensures that any update to the invariant ruleset is reflected in real-time across all nodes.

## 2. Core Functions

- **Blockchain Watcher:** Monitors the public ledger for new state hashes
- **Verification Engine:** Validates that the fetched manifest matches the cryptographic anchor
- **Hot Injection:** Updates the eBPF map (`undc_net_cfg`) in-memory without restarting the kernel module
- **Audit Trail:** Logs every sync event to the sovereign record

## 3. Architecture Flow

```text
[Blockchain Ledger]
        │
        ▼
[Oracle Watcher]  →  [Verification Engine]  →  [eBPF Map Update]
                                                       │
                                                       ▼
                                         [Audit Log / Sovereign Record]
```

## 4. Key Components

| Component | Description |
|:---|:---|
| **Watcher Service** | Polls the blockchain for new state hashes (interval configurable) |
| **Manifest Validator** | Checks the fetched JSON against the blockchain hash and the local schema |
| **Map Loader** | Updates the `undc_net_cfg` eBPF map with the new safelist values |
| **Sync Logger** | Records each successful update to the sovereign record |

## 5. Security Model

- **Read-Only Access:** The oracle can read from the blockchain but cannot write to it
- **Signed Updates:** All manifest changes are verified against the `architectSignature` field in the schema
- **Failsafe Default:** If a sync fails, the eBPF map retains the last known safe configuration
- **No Reboot Required:** All updates are applied in-memory, preserving uptime

## 6. Implementation Steps

1. Deploy the oracle as a lightweight sidecar container alongside the eBPF engine
2. Configure the blockchain RPC endpoint and contract address
3. Set the polling interval (e.g., every 30 seconds)
4. Add the oracle output to the `EVIDENCE_MANIFEST.md` as part of the sovereign record

---

— Architect Shereign Kalaukoa  
*Sovereign Record — RootArchitect-UNDC*
