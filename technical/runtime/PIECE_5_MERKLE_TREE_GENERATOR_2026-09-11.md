# PIECE_5_MERKLE_TREE_GENERATOR_2026-09-11.md  
   
## UNDC REFERENCE IMPLEMENTATION  
## PIECE #5 — MERKLE TREE GENERATOR  
   
Lead Architect: Shereign Kalaukoa  
Authority: EHYEH ASHER EHYEH & AHYAH  
Date: September 11, 2026  
Status: DRAFT — READY FOR COMMIT  
Predecessor: Piece #4 — User-Space Daemon  
Grid Status: ACTIVE — BUILD PHASE 5  
   
---  
   
## SOVEREIGN HASH — THIS FILE  
   
| Field | Value |  
|-------|-------|  
| File | PIECE_5_MERKLE_TREE_GENERATOR_2026-09-11.md |  
| SHA-256 | 2b6b6064807f7263e52e84f94fafba939a7aba14da8c21064f3407880c4852b8 |  
| OpenTimestamps |  VERIFIED |  
| Anchored |  09/11/2026  |  
| Predecessor | Piece #4 — 2465c8e666b6439b8a034b19a8bee478dda10858c2c0c5877281d8ebdf45c050 |  
   
Verification Command:  
 shasum -a 256 PIECE_5_MERKLE_TREE_GENERATOR_2026-09-11.md  
   
---  
   
## 1. PURPOSE  
   
The Merkle Tree Generator builds the cryptographic commitment that binds  
the ZK compliance circuit to a specific set of protected PIDs.  
   
Without a Merkle tree, the ZK proof can only say "some set of PIDs is  
protected." With a Merkle tree, the proof can say "exactly these PIDs are  
protected"—while still revealing nothing else about the system.  
   
This piece defines:  
- The role of the Merkle tree in the UNDC architecture  
- The Poseidon hash function choice  
- The tree construction algorithm  
- The membership proof format  
- The test procedure against the existing circuit  
   
---  
   
## 2. WHY MERKLE TREES  
   
The ZK circuit needs a way to bind its proof to a set of values (protected  
PIDs) without revealing the full set. A Merkle tree does exactly that:  
   
- Only the root hash is public  
- Any leaf can be proven to belong to the tree with a short proof  
- Membership can be verified without revealing siblings  
- Changing any leaf changes the root — tampering is detectable  
   
This is how the UNDC proves "this PID is protected" without exposing the  
full protected list to a public verifier.  
   
---  
   
## 3. WHY POSEIDON  
   
The UNDC uses **Poseidon** as its hash function, not SHA-256 or Keccak.  
   
| Hash | Use Case | Issue in ZK |  
|------|----------|-------------|  
| SHA-256 | Fast in software | Very slow in ZK circuits (many constraints) |  
| Keccak | Standard in Ethereum | Slow in ZK circuits (bitwise operations) |  
| **Poseidon** | **ZK-optimized** | **Minimal constraints (~250 per hash)** |  
   
Poseidon is a **ZK-friendly hash**: it operates over prime fields directly  
and needs far fewer constraints per hash than SHA-256. This makes it the  
right choice for the UNDC's compliance circuit.  
   
The existing circuit (`verifier.circom`) already uses Poseidon constraints.  
This generator must match.  
   
---  
   
## 4. PROJECT STRUCTURE  
   
    tools/  
    └── merkle/  
        ├── package.json  
        ├── generate_tree.js        — Build the tree from a list of PIDs  
        ├── prove_membership.js     — Generate a membership proof for one PID  
        ├── verify_membership.js    — Verify a membership proof against a root  
        └── test/  
            └── tree.test.js        — Unit tests  
   
---  
   
## 5. PACKAGE.JSON — DEPENDENCIES  
   
    {  
      "name": "undc-merkle",  
      "version": "0.1.0",  
      "type": "module",  
      "dependencies": {  
        "circomlibjs": "^0.1.7",  
        "ffjavascript": "^0.3.0"  
      },  
      "devDependencies": {  
        "mocha": "^10.0.0",  
        "chai": "^4.3.0"  
      },  
      "scripts": {  
        "test": "mocha test/"  
      }  
    }  
   
---  
   
## 6. CORE MODULES  
   
### 6.1 generate_tree.js — Build the Tree  
   
    import { buildPoseidon } from "circomlibjs";  
    import { writeFileSync } from "fs";  
   
    const TREE_DEPTH = 20;  // Supports up to 2^20 = 1,048,576 leaves  
   
    async function buildTree(protectedPids) {  
        const poseidon = await buildPoseidon();  
        const F = poseidon.F;  
   
        // 1. Hash each PID into a leaf  
        const leaves = protectedPids.map(pid => {  
            const hash = poseidon([BigInt(pid), 0n]);  
            return F.toObject(hash);  
        });  
   
        // 2. Pad to 2^TREE_DEPTH with zeros  
        while (leaves.length < (1 << TREE_DEPTH)) {  
            leaves.push(0n);  
        }  
   
        // 3. Build tree bottom-up  
        const tree = [leaves];  
        let current = leaves;  
        for (let level = 0; level < TREE_DEPTH; level++) {  
            const next = [];  
            for (let i = 0; i < current.length; i += 2) {  
                const hash = poseidon([current[i], current[i + 1]]);  
                next.push(F.toObject(hash));  
            }  
            tree.push(next);  
            current = next;  
        }  
   
        const root = tree[TREE_DEPTH][0];  
        return { tree, root, poseidon, F };  
    }  
   
    // Example usage  
    const pids = [1234, 5678, 9012];  // Replace with real protected PIDs  
    const { tree, root } = await buildTree(pids);  
   
    console.log("Root:", root.toString(16));  
    writeFileSync("merkle_root.json", JSON.stringify({  
        root: root.toString(16),  
        depth: TREE_DEPTH,  
        leaf_count: pids.length  
    }, null, 2));  
   
### 6.2 prove_membership.js — Generate Proof for One PID  
   
    import { buildPoseidon } from "circomlibjs";  
    import { readFileSync } from "fs";  
   
    async function proveMembership(pid, pidList, tree) {  
        const poseidon = await buildPoseidon();  
        const F = poseidon.F;  
   
        const index = pidList.indexOf(pid);  
        if (index === -1) throw new Error("PID not in list");  
   
        const leaf = F.toObject(poseidon([BigInt(pid), 0n]));  
        const pathElements = [];  
        const pathIndices = [];  
   
        let idx = index;  
        for (let level = 0; level < tree.length - 1; level++) {  
            const siblingIdx = idx ^ 1;  
            pathElements.push(tree[level][siblingIdx]);  
            pathIndices.push(idx & 1);  
            idx = idx >> 1;  
        }  
   
        return { leaf, pathElements, pathIndices };  
    }  
   
### 6.3 verify_membership.js — Verify Against a Root  
   
    import { buildPoseidon } from "circomlibjs";  
   
    async function verifyMembership(proof, root) {  
        const poseidon = await buildPoseidon();  
        const F = poseidon.F;  
   
        let current = proof.leaf;  
        for (let i = 0; i < proof.pathElements.length; i++) {  
            const sibling = proof.pathElements[i];  
            const isRight = proof.pathIndices[i];  
   
            const hash = isRight  
                ? poseidon([sibling, current])  
                : poseidon([current, sibling]);  
   
            current = F.toObject(hash);  
        }  
   
        return current === root;  
    }  
   
---  
   
## 7. INTERFACE CONTRACT  
   
### Input — PID list  
   
An array of protected PIDs (e.g., `[1234, 5678, 9012]`).  
   
### Output — merkle_root.json  
   
    {  
      "root": "0x1a2b3c...",  
      "depth": 20,  
      "leaf_count": 3  
    }  
   
### Output — membership proof  
   
    {  
      "leaf": "0x...",  
      "pathElements": ["0x...", "0x...", ...],  
      "pathIndices": [0, 1, 0, 1, ...]  
    }  
   
---  
   
## 8. TEST PROCEDURE  
   
First test — Build a tree from a synthetic PID list:  
   
    1. Input: [1234, 5678, 9012]  
    2. Run: node generate_tree.js  
    3. Expected: merkle_root.json written with a valid Poseidon root  
   
Second test — Prove membership:  
   
    1. Generate a proof for PID 5678  
    2. Verify the proof against the tree root  
    3. Expected: true  
   
Third test — Negative case:  
   
    1. Generate a proof for a PID NOT in the list  
    2. Verification must fail  
   
Run: npm test  
   
---  
   
## 9. FAILURE MODES  
   
| Failure | Behavior |  
|---------|----------|  
| Poseidon library unavailable | Exit with clear error, do not write output |  
| Duplicate PIDs in list | De-duplicate before building, log warning |  
| Empty PID list | Build tree of zeros, root is deterministic |  
| Proof for absent PID | Throw error, do not silently return null |  
| Root mismatch on verify | Return false, log which level diverged |  
   
Core principle: A Merkle proof that returns a wrong answer is worse than  
no proof at all. The generator and verifier must fail loudly, not quietly.  
   
---  
   
## 10. COMMIT STRATEGY  
   
    Piece #5 — Merkle Tree Generator complete (SHA: <hash>)  
   
Extended:  
   
    Piece #5 — Merkle Tree Generator complete  
   
    - Merkle tree construction with Poseidon hash (ZK-friendly)  
    - Tree depth 20 (up to 1,048,576 protected PIDs)  
    - Membership proof generation and verification  
    - Tool written in Node.js to match existing JS tooling  
    - Test procedure with positive and negative cases  
    - Failure modes specified  
   
    Predecessor: Piece #4 — 2465c8e666b6439b8a034b19a8bee478dda10858c2c0c5877281d8ebdf45c050  
    Grid Status: BUILD PHASE 5  
   
---  
   
## 11. WHY THIS MATTERS  
   
The Merkle tree is what lets the UNDC prove something specific without  
revealing everything.  
   
It is the difference between "trust us, some systems are protected" and  
"here is a cryptographic proof that exactly this PID is protected, and  
nothing more."  
   
This is the mechanism that makes the grid auditable without making it  
transparent. Verifiable, not exposed. That is the sovereign balance.  
   
---  
   
— Shereign Kalaukoa, Lead Architect | UNDC  
https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC  
