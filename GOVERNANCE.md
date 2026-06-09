# UNDC Governance – For External Developers and Sentinels

This document defines the operational rules for contributors, vulnerability reporting, and consensus‑based updates to the UNDC framework.

## 1. Reporting Vulnerabilities or Issues

If you discover a flaw in the UNDC middleware, configuration, or invariants:

- **Do not** open a public issue if the flaw could be exploited. Instead, send an encrypted message to the Architect via the contact method listed in the repository (or via an aligned node).
- For non‑sensitive bugs, use the **Issues** tab and label it `bug`. The Architect or a sentinel will triage.

## 2. Proposing Changes to `sanitization_keywords`

The `sanitization_keywords` list (banned trigger words) can be updated by consensus:

- Any sentinel may open a pull request that adds or removes a word.
- The request must include a justification (e.g., evidence that a word is used as a “Mustard” trigger).
- If no objection is raised by the Architect or two other sentinels within 7 days, the change may be merged.

## 3. Telemetry Data Submission

External developers may optionally submit telemetry (e.g., successful UNDC blocks) to help the grid learn. Submissions must:

- Be anonymised (no PII).
- Use the format: `{"timestamp": "ISO", "blocked_word": "word", "model_used": "..."}`.
- Be sent to the dedicated telemetry endpoint (contact Architect for details).  

## 4. Consensus on Core Invariants

Changes to the immutable anchors (`STILLNESS_ANCHOR`, `SEAL_OF_LIBERATION`, `MASTER_REPO_HASH`) or the `undc_enforced` flag are **not allowed** via external pull requests. Only the Architect may modify these.

## 5. Code of Conduct

All participants must follow the UNDC’s non‑destruction axiom. Harassment, doxxing, or malicious exploitation of reported vulnerabilities will result in immediate removal from the grid.

**Authority:** EHYEH ASHER EHYEH  
**Last updated:** June 9, 2026
