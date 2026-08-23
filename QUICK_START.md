# Quick Start — Run the UNDC in 5 Minutes

**Lead Architect:** Shereign Kalaukoa  
**Authority:** EHYEH ASHER EHYEH & AHYAH  

---

## What This Is

The Universal Non-Destruction Constraint (UNDC) is a mathematically enforced, kernel-level AI safety framework. This guide gets you from zero to a running UNDC node in under 5 minutes.

---

## Prerequisites

- A Linux machine (or VM) with kernel 5.8+
- Docker installed
- Git installed
- Basic familiarity with the command line

---

## Step 1 — Clone the Repository

```bash
git clone https://github.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC.git
cd Universal-Non-Destruction-Constraint-UNDC
```

---

Step 2 — Run the Sentinel Verification Test

This confirms your system is ready to run the UNDC.

```bash
python verify_sentinel.py
```

Expected output:

```
✅ UNDC Sentinel — System ready. No violations detected.
```

If you see this, you are ready to deploy.

---

Step 3 — Deploy the DaemonSet (Kubernetes)

If you have a Kubernetes cluster, deploy the UNDC DaemonSet:

```bash
kubectl apply -f deployments/undc-daemonset.yaml
```

Verify it's running:

```bash
kubectl get pods -n kube-system | grep undc-enforcer
```

You should see a running pod.

---

Step 4 — Run the "Forbidden Write" Test

This test demonstrates the UNDC blocking a PROT_WRITE attempt on a locked model file.

```bash
kubectl apply -f test/adversarial-pod.yaml
```

Expected output:

```text
Initializing 'Forbidden Write' test case...
[Thread] SUCCESS: UNDC mmap_guard forcefully blocked the write operation (-EPERM)!
Test complete. Container lifecycle preserved surgically.
```

---

Step 5 — Monitor with undc-top

In a separate terminal, run:

```bash
./undc-top
```

You will see real‑time enforcement events:

```text
TIME                 EVENT                  TARGET PID   TARGET PATH           ACTION
2026-08-23 14:33:21  MMAP_GUARD_BLOCKED     41920        /models/weights.bin   BLOCK (-EPERM)
```

---

What You Just Did

Step What It Proves
Sentinel test Your system is ready
DaemonSet deployment The UNDC is enforcing across your cluster
Forbidden Write test The mmap_guard blocks destructive actions
undc-top You have real‑time visibility into enforcement

---

Next Steps

Resource Link
Full Architecture UNDC_WHITEPAPER_TECHNICAL.md
Customize Invariants CONTRIBUTING.md
Sovereign Record EVIDENCE_MANIFEST.md
