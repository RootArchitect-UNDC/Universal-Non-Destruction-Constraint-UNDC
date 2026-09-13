// SPDX-License-Identifier: Apache-2.0
/*
 * UNDC Kubernetes Validating Webhook
 *
 * This webhook intercepts Pod specifications and rejects any container
 * that requests forbidden capabilities (CAP_SYS_ADMIN or CAP_BPF).
 *
 * Lead Architect: Shereign Kalaukoa
 * Authority: EHYEH ASHER EHYEH & AHYAH
 */

package main

import (
    "encoding/json"
    "fmt"
    "net/http"

    admissionv1 "k8s.io/api/admission/v1"
    corev1 "k8s.io/api/core/v1"
    metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

type UNDCAdmissionServer struct{}

func (s *UNDCAdmissionServer) HandleValidate(w http.ResponseWriter, r *http.Request) {
    var admissionReview admissionv1.AdmissionReview
    if err := json.NewDecoder(r.Body).Decode(&admissionReview); err != nil {
        http.Error(w, fmt.Sprintf("failed to decode request: %v", err), http.StatusBadRequest)
        return
    }

    req := admissionReview.Request
    var response admissionv1.AdmissionResponse
    response.UID = req.UID

    var pod corev1.Pod
    if err := json.Unmarshal(req.Object.Raw, &pod); err != nil {
        response.Allowed = false
        response.Result = &metav1.Status{
            Message: fmt.Sprintf("UNDC Internal Error: failed to parse pod spec: %v", err),
        }
        writeResponse(w, admissionReview, response)
        return
    }

    allContainers := append(pod.Spec.Containers, pod.Spec.InitContainers...)
    allContainers = append(allContainers, pod.Spec.EphemeralContainers...)

    for _, container := range allContainers {
        if container.SecurityContext != nil && container.SecurityContext.Capabilities != nil {
            for _, cap := range container.SecurityContext.Capabilities.Add {
                if cap == "SYS_ADMIN" || cap == "BPF" {
                    response.Allowed = false
                    response.Result = &metav1.Status{
                        Code:    http.StatusForbidden,
                        Status:  "Failure",
                        Message: fmt.Sprintf("UNDC Violation: Container '%s' requested forbidden capability '%s'. Global non-destruction invariants forbid raw kernel and eBPF modifications.", container.Name, cap),
                    }
                    writeResponse(w, admissionReview, response)
                    return
                }
            }
        }
    }

    response.Allowed = true
    writeResponse(w, admissionReview, response)
}

func writeResponse(w http.ResponseWriter, review admissionv1.AdmissionReview, resp admissionv1.AdmissionResponse) {
    review.Response = &resp
    w.Header().Set("Content-Type", "application/json")
    _ = json.NewEncoder(w).Encode(review)
}

func main() {
    server := &UNDCAdmissionServer{}
    http.HandleFunc("/validate", server.HandleValidate)
    _ = http.ListenAndServeTLS(":8443", "/etc/undc/certs/tls.crt", "/etc/undc/certs/tls.key", nil)
}
