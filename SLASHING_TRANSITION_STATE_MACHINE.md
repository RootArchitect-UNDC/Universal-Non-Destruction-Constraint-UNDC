# Slashing Transition State Machine

**Date:** September 3, 2026  
**Author:** Shereign Kalaukoa, Lead Architect  
**Status:** ✅ Architected — Ready for Implementation  
**Authority:** EHYEH ASHER EHYEH & AHYAH  
**SHA256 HASH** <insert after hashing>
---

## 🔥 Overview

The Slashing Transition State Machine is the economic immune system of the grid. It processes all incoming Proof of Failure reports through a deterministic state machine, preventing race conditions, double-spending, and multi-report exploitation.

The state machine handles concurrency at the ledger layer by transforming overlapping asynchronous gossip reports into sequential, atomic execution steps.

---

## 🔄 State Space and Definitions

For any registered node, its staking lifecycle inside the treasury ledger transitions through five distinct states:

| **State** | **Description** |
|-----------|-----------------|
| **Active** | The node is in good standing. Staked collateral is fully locked and earning baseline verification yields. |
| **Pending** | A valid Proof of Failure has been submitted. The node's entire stake is instantly frozen. |
| **Validating** | A time-locked window opens to collect Neighbor Proximity Proofs from the propagation ring. |
| **Slashing** | The validation window has closed. The contract evaluates the proofs and calculates the distribution. |
| **Resolved** | The slashing operation is finalized. Collateral is split, burned, and distributed. |

---

## 🔄 Transition Logic

### Transition 1: Active → Pending

**Trigger:** A valid Proof of Failure is submitted.  
**Guard Conditions:** The proof must be valid. The node must be Active.  
**Atomic Action:** The node's state changes to Pending. The sender is recorded as FirstResponder. The block height is saved.  
**Concurrency Resolution:** Any secondary transactions are rejected.

### Transition 2: Pending → Validating

**Trigger:** One block has passed.  
**Atomic Action:** The state shifts to Validating. The submission window for proximity proofs opens.

### Transition 3: Validating → Slashing

**Trigger:** Twelve blocks have passed.  
**Atomic Action:** The state shifts to Slashing. No further proximity proofs are accepted.

### Transition 4: Slashing → Resolved (or Revert)

**Trigger:** Execution of the calculation loop.

**Condition A (At least one valid proximity proof exists):**
- FirstResponder receives 40% of the reward pool
- Validated propagation ring peers divide the remaining 60%
- 5% is burned
- 75% moves to cooling escrow

**Condition B (All proximity proofs fail):**
- State reverts to Active
- Frozen stake is released
- FirstResponder is penalized

---

## 📁 Data Invariants

The state machine stores a minimal, fixed-size data structure per incident:


struct SlashingIncident {
node_id: [u8; 32],
state: StateEnum,
first_responder: [u8; 32],
trigger_block: u64,
reward_pool: u256,
valid_peers: Vec<[u8; 32]>, // Capped at 8 entries
}
```

---

**EHYEH ASHER EHYEH.**  
THE GRID IS STRONG. THE STATE MACHINE IS LOCKED.

— Shereign Kalaukoa, Lead Architect | UNDC
