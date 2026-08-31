# UNDC Enterprise QA & DevOps Checklist — v1.0

**Lead Architect:** Shereign Kalaukoa  
**Authority:** EHYEH ASHER EHYEH & AHYAH  
**Date:** August 24, 2026  
**Purpose:** To provide a step-by-step blueprint for enterprise QA and DevOps teams to verify UNDC compliance and integrate kernel-level enforcement into their CI/CD pipelines.

---

## 1. Purpose of This Document

This checklist is designed for:

- **DevOps Engineers** — integrating UNDC enforcement into Kubernetes clusters.
- **QA Teams** — verifying that eBPF LSM hooks are active and enforcing the invariant.
- **Security Engineers** — auditing the runtime enforcement layer.
- **Compliance Officers** — documenting UNDC deployment for regulatory reviews.

---

## 2. Prerequisites

| Requirement | Minimum Version | Verification Command |
|-------------|-----------------|----------------------|
| Linux Kernel | 5.8+ | `uname -r` |
| eBPF Support | Enabled | `bpftool prog list` |
| Kubernetes | 1.24+ | `kubectl version` |
| OpenTimestamps CLI | Latest | `ots --version` |
| Circom | 2.1.8+ | `circom --version` |

---

## 3. Pre-Deployment Verification

### 3.1 Kernel Capability Audit

Run the node verification script to confirm host compatibility:

```bash
# Download and run the UNDC node auditor
curl -O https://raw.githubusercontent.com/RootArchitect-UNDC/Universal-Non-Destruction-Constraint-UNDC/main/undc-verify-node.sh
chmod +x undc-verify-node.sh
./undc-verify-node.sh
```

---

Expected Output:

```
[PASS] Kernel version validation verified.
[PASS] eBPF Linux Security Module is active on the node structure.
[PASS] Secure BPF mount topology confirmed.
[PASS] Host node validated.
```

3.2 eBPF LSM Hook Validation

Check that the eBPF hooks are loaded and active:

```bash
# List loaded eBPF programs
bpftool prog list | grep undc

# Check kernel logs for UNDC events
sudo cat /sys/kernel/debug/tracing/trace_pipe | grep UNDC
```

Expected Output: eBPF programs named undc_execve_hook, undc_mmap_hook, undc_connect_hook are visible.

---

4. Deployment Checklist

4.1 Kubernetes Namespace & Service Account

```bash
# Create namespace
kubectl create namespace undc-system

# Create service account
kubectl create serviceaccount undc-webhook -n undc-system

# Apply ClusterRole and Binding
kubectl apply -f undc-rbac.yaml
```

4.2 ConfigMap Deployment

```bash
# Apply the UNDC invariants ConfigMap
kubectl apply -f undc-configmap.yaml
```

4.3 DaemonSet Deployment

```bash
# Deploy the eBPF enforcer DaemonSet
kubectl apply -f undc-daemonset.yaml

# Verify DaemonSet is running on all nodes
kubectl get pods -n undc-system -l app=undc-enforcer
```

Expected Output: One pod per node, all in Running state.

4.4 Validating Webhook Deployment

```bash
# Deploy the webhook
kubectl apply -f undc-webhook.yaml

# Verify webhook is active
kubectl get validatingwebhookconfigurations | grep undc
```

4.5 Certificate Verification

```bash
# Verify webhook certificate is valid
kubectl get secret undc-webhook-certs -n undc-system -o yaml
```

---

5. Post-Deployment Verification

5.1 Test Enforcement — Privileged Pod

```bash
# Attempt to create a privileged pod (should be blocked)
kubectl run test-priv --image=busybox --privileged
```

Expected Output:

```
Error from server: admission webhook "undc.validator.dev" denied the request
```

5.2 Test Enforcement — Unauthorized System Call

```bash
# Run a pod that attempts a forbidden system call
kubectl run test-execve --image=busybox --command -- sh -c "exec /bin/sh"
```

Expected Output: Pod is blocked or eBPF hook denies the system call.

5.3 Check eBPF Logs

```bash
# View kernel logs for enforcement events
sudo cat /sys/kernel/debug/tracing/trace_pipe | grep UNDC
```

Expected Output: Log entries showing execve blocked — invariant violation or similar.

---

6. CI/CD Integration

6.1 Pre-Commit Hook

Add a pre-commit hook to validate UNDC compliance before code is merged:

```bash
#!/bin/bash
# .git/hooks/pre-commit

# Run the Python verifier on staged files
python3 undc_verifier.py --check-staged
if [ $? -ne 0 ]; then
    echo "UNDC compliance check failed. Commit blocked."
    exit 1
fi
```

6.2 GitHub Actions Workflow

Add a GitHub Actions workflow to validate UNDC compliance on every push:

```yaml
name: UNDC Compliance Check

on: [push, pull_request]

jobs:
  verify:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Run UNDC Verifier
        run: python3 undc_verifier.py
```

6.3 Kubernetes Admission Controller Integration

For custom admission controllers, integrate the UNDC webhook:

```yaml
apiVersion: admissionregistration.k8s.io/v1
kind: ValidatingWebhookConfiguration
metadata:
  name: undc-webhook
webhooks:
  - name: validate.undc.safety.infrastructure
    clientConfig:
      service:
        name: undc-validator-svc
        namespace: undc-system
        path: "/validate-agent-constraints"
    rules:
      - apiGroups: [""]
        apiVersions: ["v1"]
        operations: ["CREATE", "UPDATE"]
        resources: ["pods"]
    failurePolicy: Fail
```

---

7. Verification Log Template

Date Time Step Status Notes
    

---

8. Troubleshooting Guide

Issue Likely Cause Resolution
Webhook not blocking privileged pods Certificate expired Regenerate certs and restart webhook
eBPF hooks not loading Kernel version < 5.8 Upgrade kernel
ConfigMap not found Namespace mismatch Verify namespace is undc-system
Webhook timeout Network latency Increase timeout in webhook config

---

9. Conclusion

This checklist provides a complete, step-by-step blueprint for enterprise QA and DevOps teams to:

· Verify UNDC compliance in their clusters.
· Integrate enforcement into CI/CD pipelines.
· Document deployment for regulatory audits.

---

EHYEH ASHER EHYEH.
THE CHECKLIST IS COMPLETE. THE DEPLOYMENT IS VERIFIED.

— Shereign Kalaukoa, Lead Architect | UNDC
