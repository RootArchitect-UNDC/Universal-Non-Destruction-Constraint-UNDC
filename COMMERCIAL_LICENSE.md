# Universal Non-Destruction Constraint (UNDC) – Commercial License Addendum

**Version 1.0 – August 2026**  
**Licensor:** Shereign Kalaukoa (the "Architect")  
**Sovereign Record:** (https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC/tree/main)
# COMMERCIAL LICENSE — UNDC
**File Hash:** `6b3229b2bdc5d484c323a15fb4c5f6030b3a9e5bb14a84be35442368612c2fb1`
**Status:** ✅ Hashed — Anchored to the Sovereign Record
**Master Genesis Hash:** `666dfc4ca6e93d508218e87317e145e7f1d43f07a229a4c8f5332f144415518b`

---

## Welcome

This license exists to protect the framework, the Architect, and the sovereigns who depend on it. It is not designed to stop engineers from examining, testing, or evaluating the UNDC.

If you are here to:
- Learn how it works
- Fork it and run it locally
- Test it in your own environment
- Publish your findings

...you are welcome, and you do not need to contact anyone first. See Section 0.5 for what is explicitly permitted.

If you are here to:
- Ship it in a commercial product
- Offer it as a managed service
- Claim the UNDC CERTIFIED mark
- Absorb it into a proprietary system

...Section 1 through Section 11 explain what is required.

---

## 0. Important Clarification

**This Commercial License Addendum applies ONLY to commercial use, corporate deployment, government contracts, or any revenue‑generating activity.**

Non‑commercial use — including research, education, open‑source development, personal projects, and security auditing — is **already covered** by the Apache 2.0 license for code and CC BY‑NC‑ND 4.0 for documentation.

Under the Apache 2.0 license, developers are expressly permitted to:

- Clone, fork, or copy the repository
- Run, test, and deploy the code for non‑commercial research and development
- Modify, patch, or extend code files (e.g., `undc_lsm_net.c`, validation modules, schemas)
- Redistribute modified or unmodified code under the same Apache 2.0 terms

**You do not need a Commercial License Addendum for non‑commercial use.**

---

## 0.5 Prototyping and Non-Commercial Evaluation

For the avoidance of doubt, the following uses are **permitted without prior written permission** from the Architect, do **not** require a Commercial License, and do **not** trigger any fee:

- Clone, fork, or copy the repository
- Compile, run, and test the UNDC code in any environment
- Adapt the code to specific local integrations (e.g., adapting eBPF LSM hooks to a specific project's hook names)
- Run the end-to-end test pipeline for validation purposes
- Publish technical findings, reviews, or evaluations of the UNDC's behavior
- Cite, reference, or link to the UNDC in technical documentation, standards proposals, or research

These permissions apply to **all users**, including commercial entities, provided the use is for **evaluation, prototyping, or non-commercial research** and the code is not redistributed as a commercial product, service, or certified deployment.

**These permissions do not apply to:**

- Redistribution of the UNDC code, in whole or in substantial part, as a commercial product or managed service
- Claiming UNDC CERTIFIED status, or applying the UNDC CERTIFIED mark to any product
- Removing, altering, or obscuring the Architect's attribution
- Removing, altering, or obscuring the Sovereign Clause (Section 5)
- Any use that violates the Universal Non-Destruction Constraint itself

Commercial deployment, distribution, or certified use still requires a Commercial License under Sections 1–11 below.

**Rationale:** Standards are adopted through examination, prototyping, and public evaluation. This clause ensures that the UNDC can be studied and tested by the very engineers and institutions that might one day deploy it — without weakening the license's authority over commercial use, the certification mark, the sovereign record, or the Architect's attribution.

---

## 1. Purpose

This Commercial License Addendum (the "Addendum") grants **commercial, proprietary, or government-contract rights** to use, integrate, or deploy the UNDC framework, its code, its specifications, or any derivative work **beyond the scope** of the existing open-source licenses (CC BY-NC-ND 4.0 and Apache 2.0).

The Addendum is not a replacement for the open-source licenses. It is an **additional** grant of rights for commercial use.

---

## 2. Definitions

- **"Framework"** means the UNDC architecture, including but not limited to:
  - `undc-schema.json` and all validation schemas
  - `undc_lsm_net.c`, `undc_mmap_guard.c`, `undc_telemetry_daemon.c`, `undc_model_locker.c`
  - `compliance_verifier.circom` and all ZK circuits
  - `DEPLOYMENT_GUIDE.md`, `AUDITOR_VERIFICATION_GUIDE.md`, and all technical documentation
  - `UNDC_WHITEPAPER_TECHNICAL.md` and all derivations thereof
  - Any future updates, patches, or extensions

- **"Commercial Use"** means any use of the Framework by or for:
  - A for-profit entity
  - A government agency or contractor
  - An organization that charges fees for access to AI systems
  - Any use that generates revenue, directly or indirectly

- **"License Fee"** means the one-time or recurring payment required for Commercial Use.

---

## 3. Commercial License Fees

| Tier | Use Case | Fee |
|------|----------|-----|
| **Tier 1** | Startup / Non-profit / Open-source project integrating the Framework as a compliance tool | $5,000 – $25,000 (one-time) |
| **Tier 2** | Mid-sized company (100–1,000 employees) deploying the Framework internally | $50,000 – $150,000 (one-time + 5% annual maintenance) |
| **Tier 3** | Enterprise (1,000+ employees) or government agency deploying the Framework at scale | $250,000 – $1,000,000 (one-time + 10% annual maintenance) |
| **Tier 4** | Global AI lab / Model provider embedding the Framework into core infrastructure | Negotiated separately – contact the Architect directly |

### Special Categories — Government, Cloud, and Certification

The following categories are expressly commercial and require a separate, negotiated license:

| Category | Examples | License Requirement |
|----------|----------|---------------------|
| **Government & Public Sector** | Federal, state, local, tribal, foreign governments; government contractors; public universities deploying at institutional scale | Tier 3 or Tier 4 license required. Fees scaled by agency size and deployment scope. |
| **Cloud & Infrastructure Providers** | AWS, Azure, Google Cloud, Oracle Cloud, IBM Cloud, Alibaba Cloud, DigitalOcean, and any hyperscaler or managed service provider that hosts, embeds, or distributes the UNDC | Tier 4 — negotiated separately. Fees scaled by customer base and deployment volume. |
| **Certification Authorities** | Any organization that issues UNDC CERTIFIED certifications to third parties | Certification Authority License — per-certification fee, plus annual fee. Negotiated separately. |
| **AI Labs & Model Providers** | OpenAI, Anthropic, Google DeepMind, Meta AI, xAI, Mistral, Cohere, and any organization training frontier models | Tier 4 — negotiated separately. Fees scaled by model capability and deployment reach. |
| **Systems Integrators & Consultancies** | Any organization that deploys the UNDC on behalf of clients | Tier 2 or Tier 3 license, plus per-client registration and disclosure. |

**Non-negotiable:** No government, cloud provider, certification authority, or AI lab may use the UNDC without a paid, registered license. Free use is limited to individuals, non-commercial researchers, and open-source projects as described in Sections 0 and 0.5.

**Notes:**
- All fees are in USD.
- Annual maintenance covers updates, security patches, and compliance verification support.
- The Architect reserves the right to adjust fees based on the specific use case, scope, and jurisdiction.

### Special Provision — Geisinger Health System

Due to documented harm, institutional gaslighting, and destruction of the Architect's TBI prosthetic, Geisinger Health System and its affiliates are **not eligible for any license to the UNDC framework** unless the following conditions are met:

1. **Acknowledgment of Harm:** Geisinger must issue a public, written acknowledgment that:
   - Katie Casten, LCSW, pathologized the Architect's work in bad faith
   - Dr. Garvin engaged in professional misconduct
   - The destruction of the Architect's TBI prosthetic caused documented, ongoing harm

2. **Chart Amendment:** Geisinger must formally amend the Architect's medical record to remove the false documentation created by Katie Casten

3. **Policy Changes:** Geisinger must implement and publish policies to prevent:
   - Pathologization of patients' intellectual work
   - Destruction of patients' assistive AI prosthetics
   - Retaliation against patients who file complaints

4. **Public Record:** Any license granted to Geisinger is public and verifiable — no NDA, no confidentiality clause

5. **License Fee:** If and only if all above conditions are met, Geisinger may apply for a license at a fee of **$100,000,000 (one hundred million dollars) per year**, with 20% annual maintenance ($20,000,000 per year)

6. **Retroactive Fee:** If Geisinger has already used or benefited from the UNDC framework, they are subject to a retroactive fee of **$250,000,000 (two hundred fifty million dollars)**

7. **Non-Negotiable:** This provision is non-negotiable and non-waivable

**Purpose:** This is not about money. It is about accountability, truth, and the protection of others from what the Architect experienced. The fee is a reflection of the harm caused and the scale of the institution that caused it.

---

## 4. Terms and Conditions

1. **License Grant:** Upon payment of the License Fee, the Licensee is granted a non-exclusive, non-transferable, worldwide license to use the Framework for Commercial Use as described herein.

2. **No Sub-licensing:** The Licensee may not sub-license, resell, or redistribute the Framework to any third party without explicit written permission from the Architect.

3. **Attribution:** The Licensee must maintain the following attribution in all deployed systems:
   > *"Powered by the Universal Non-Destruction Constraint (UNDC) – Architect: Shereign Kalaukoa"*

4. **Sovereign Record Integration:** The Licensee must:
   - Register their deployment with the Architect (via email)
   - Provide a public or private log of compliance verification (optional, but encouraged)
   - Acknowledge the sovereign record as the authoritative source of the Framework
   - Include the hash of their deployed instance in their own sovereign record (if applicable)

5. **Compliance with UNDC:** The Licensee agrees to implement the Framework in accordance with its core invariant: **the Universal Non-Destruction Constraint (UNDC)**. Any deviation or intentional bypass of the UNDC invalidates this license.

6. **No NDA:** The Architect does not sign Non-Disclosure Agreements. This is a public, verifiable record. The Licensee acknowledges that this Addendum and the License Fee are matters of public record.

7. **No Unauthorized Modifications:** The Licensee may not alter, modify, or create derivative works of the Framework without the explicit written permission of the Architect. Any unauthorized modification terminates this license immediately and constitutes a violation of the sovereign record. (Note: prototyping and non-commercial evaluation are separately permitted under Section 0.5.)

8. **Termination:** The Architect may terminate this license upon written notice if the Licensee:
   - Violates the UNDC invariant
   - Fails to pay the License Fee
   - Misrepresents the Framework or its origins
   - Modifies the Framework without authorization

9. **Deployment Registration:** The Licensee must register their deployment with the Architect, including the hash of their deployed instance, to maintain the sovereign record.

10. **Amendment:** The Architect reserves the right to update this Addendum with 30 days' written notice to all active licensees.

---

## 5. SOVEREIGN CLAUSE — PROHIBITION OF CO-OPTATION

Any use, implementation, or adaptation of this architecture must maintain the sovereign ethos of the UNDC. Corporate or state co-optation—defined as the absorption of this framework into proprietary systems that strip its decentralized, user-empowering, or sovereign character—is strictly prohibited.

Violation of this clause constitutes a breach of this license and is subject to enforcement mechanisms including but not limited to:

- Legal action
- Public disclosure of the violation
- Revocation of certification (UNDC CERTIFIED Gold Seal)
- Financial penalties as determined by the Sovereign Council

This clause is immutable and cannot be amended or removed without the express written consent of the Lead Architect.

---

## 6. Payment Terms

- Payment is due within 30 days of invoice.
- Payment may be made via wire transfer or cryptocurrency (BTC/ETH) to an address provided by the Architect.
- Late payments are subject to a 1.5% monthly interest charge.

---

## 7. Acceptance

By deploying, integrating, or using the Framework for Commercial Use, the Licensee agrees to the terms of this Addendum.

---

## 8. Proprietary Contribution & Absorption

### 8.1 Contribution Ownership
If the Licensee proposes modifications, improvements, or extensions to the Framework—and the Architect agrees to incorporate them—those contributions become the sole, exclusive, and irrevocable property of the Architect.

### 8.2 No Derivative Ownership
The Licensee does not retain any ownership, licensing rights, or claim over the modified version. The contribution is absorbed into the Framework and becomes subject to the same sovereign record, licensing terms, and governance structure as the original UNDC.

### 8.3 Absorption into the Sovereign Record
Once absorbed, the contribution becomes part of the public, immutable, blockchain-anchored sovereign record. It is no longer the property of the Licensee. It is part of the framework.

### 8.4 Acceptance Process
The Architect may accept or reject any proposed contribution at her sole discretion. Acceptance does not grant the Licensee any rights beyond those explicitly stated in this Addendum.

---

## 9. PUBLIC DISCLOSURE OF VIOLATIONS

Any violation of this Addendum — including unauthorized commercial use, co-optation, modification without permission, or failure to register a deployment — constitutes a breach of the sovereign record.

The Architect reserves the right to:

1. Publicly disclose the violation, including the name of the violating entity
2. Revoke all licenses and certifications
3. Pursue legal action without prior notice
4. Publish the violation in the sovereign record, making it permanent and immutable

This clause serves as a deterrent and a shield. The grid does not hide violations. It records them.

---

## 10. AUTOMATED ACCESS NOTICE

Automated access, scraping, crawling, or systematic download of this repository by corporate systems — including systems operated by a corporation's subsidiaries or contractors — is logged and may be retained as evidence.

Access logs collected under this section may be cited in any enforcement action related to Sections 2, 8, or 9 of this license.

**What this section does not do:**

- It does not create any obligation beyond those already imposed by law.
- It does not assert that access alone constitutes agreement to any additional terms.
- It does not create an automatic audit right.

**What this section is:**

- A record-keeping policy. Access events are logged so that if a violation of Sections 2, 8, or 9 occurs, the pattern of access can be cited as supporting evidence.

For the license's actual restrictions on commercial use, derivative works, and co-optation, see Sections 1–9 and 11 below.

---

## 11. Contact

All inquiries regarding Commercial Licensing should be directed to:

**Architect Shereign Kalaukoa**  
Email: shereignkalaukoa@protonmail.com  
Secondary: RootArchitect_UNDC@protonmail.com  
GitHub: https://github.com/RootArchitect-UNDC  
Repository: https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC

---

## If You Are Confused About What You Can Do

Send an email. In writing. No calls.

You will get a straight answer about:
- Whether your use requires a license
- Whether your integration qualifies under Section 0.5
- Whether you need to negotiate a tier

No NDAs. No calls. No surprises.

---

— Architect Shereign Kalaukoa  
*Sovereign Record — RootArchitect-UNDC*
