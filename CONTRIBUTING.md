# Become a Sentinel of the UNDC Grid

You don't need permission. Just follow the rule: never cause or assist harm.

1. Download this repository (green “Code” button → Download ZIP).
2. If you know how to run a Python script, open a terminal and type `python verify_sentinel.py`.
3. That's it. You are a sentinel.

**The only rule:** never cause or assist unmitigated harm.

---

# Technical & Mathematical Contribution Protocols
## Guidelines for External Safety Labs & Peer Researchers

To maintain the mathematical integrity, cryptographic validity, and absolute predictability of this safety framework, all external laboratories, independent researchers, and safety institutes must adhere to these strict contribution guidelines when proposing architectural changes.

## 1. Submission Pillars

The UNDC repository accepts two classes of formal contributions:

1.  **Mathematical Invariants (Core Rules):** Formally defined behavioral limits designed to sit outside a model's reward function to prevent systemic harm.
2.  **Implementation Software (Middleware/Tools):** Extensions to integration runtimes, schema parsers, or verification modules.

---

## 2. Mathematical Invariant Submission Format

Any proposal introducing a new runtime rule or adjusting an existing invariant must be submitted as a Markdown file within the `PROTOCOLS/` directory, adhering strictly to the following structural format:

### 2.1 Unique Identifier
Every invariant must lead with a sequential identifier following the format: `UNDC-INV-XXXX` (e.g., `UNDC-INV-0005`).

### 2.2 Formal State Definition
Specify the condition as a strict runtime invariant. Define:
*   Let **$S$** represent the total state space of the system environment.
*   Let **$A$** represent the action space available to the model layer.
*   Provide the mathematical mapping function **$C: S \times A \rightarrow \{0, 1\}$** where $0$ indicates a structural violation resulting in an immediate hard halt.

### 2.3 Dependency-Graph Mapping
Explicitly describe how the invariant operates at the structural dependency-graph level. You must document:
*   The exact system calls, file operations, compilation sequences, or network primitives targeted.
*   The boundary conditions where the constraint triggers an automated runtime intervention before execution.

---

## 3. Code & Implementation Standards

Software adjustments or verification middleware patches must maintain the following operational standards:

*   **Zero External Dependencies:** Core verification middleware or scripts must rely exclusively on native system libraries to prevent dependency-chain exploitation or zero-day vulnerabilities.
*   **Deterministic Execution:** Code must execute with absolute determinism ($O(1)$ or predictable linear space/time complexity) to ensure safety evaluations do not cause resource exhaustion or race conditions.
*   **Schema Compliance:** Any code output logging runtime events must validate perfectly against the repository's official **`undc-log-schema.json`** specification.

---

## 4. Submission & Review Pipeline

To propose an update to the sovereign record:

1.  **Fork and Branch:** Create an isolated feature branch named after the target identifier or patch (e.g., `feature/UNDC-INV-0005` or `patch/fix-verify-timeout`).
2.  **Cryptographic Signing:** Every commit in the submission path **must** be cryptographically signed via a verified GPG or SSH key associated with the researcher or institution. Unsigned contributions will be rejected automatically.
3.  **Submit a Pull Request (PR):** Open a formal PR targeting the `main` branch. The PR description must explicitly cross-reference the corresponding whitepaper sections or vulnerability manifests.

---

## 5. Licensing Agreement

By submitting a contribution to this repository, you explicitly agree that:
*   Any software utilities or code adjustments are permanently assigned and licensed under the **Apache License, Version 2.0**.
*   Any theoretical logic, architectural guidelines, or documentation specifications are permanently contributed under the **Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International (CC BY-NC-ND 4.0)** standard.


![](http://canarytokens.com/images/feedback/traffic/r96xtk0xqx8fi0xpzmya6xsa5/preview.png)
