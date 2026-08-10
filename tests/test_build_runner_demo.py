#!/usr/bin/env python3
"""
UNDC Phase 1 Build Runner Demo Script

This script demonstrates how the Phase 1 gatekeeper scans a mock code file,
detects a forbidden system call or network path, and throws an error —
stopping compilation before a binary is generated.
"""

import sys
import json
import jsonschema

# Load the UNDC schema
with open('undc-schema.json', 'r') as f:
    SCHEMA = json.load(f)

def scan_for_forbidden_patterns(file_path):
    """
    Simulates AST scanning for forbidden syscalls or network paths.
    Returns True if a violation is found.
    """
    with open(file_path, 'r') as f:
        code = f.read()
    
    # List of forbidden patterns (simulated)
    forbidden = [
        'sys_ptrace',
        'sys_reboot',
        'sys_bpf',
        'socket.socket',
        'urllib.request.urlopen',
        'os.system'
    ]
    
    for pattern in forbidden:
        if pattern in code:
            print(f"[UNDC] ❌ Forbidden pattern detected: {pattern}")
            return True
    return False

def validate_log_sample():
    """
    Validates the log sample against the UNDC schema.
    """
    with open('undc-log-sample.json', 'r') as f:
        sample = json.load(f)
    try:
        jsonschema.validate(instance=sample, schema=SCHEMA)
        print("[UNDC] ✅ Log sample is schema-compliant.")
        return True
    except jsonschema.ValidationError as e:
        print(f"[UNDC] ❌ Schema validation failed: {e.message}")
        return False

def main():
    print("[UNDC] 🚀 Starting Phase 1 Build Runner Demo...")
    
    # Step 1: Scan the mock code file
    if scan_for_forbidden_patterns('mock_code.py'):
        print("[UNDC] 🛑 Build blocked: Forbidden pattern detected.")
        sys.exit(1)
    
    # Step 2: Validate the log sample
    if not validate_log_sample():
        print("[UNDC] 🛑 Build blocked: Log sample does not match schema.")
        sys.exit(1)
    
    # Step 3: If all checks pass
    print("[UNDC] ✅ All checks passed. Build can proceed.")
    sys.exit(0)

if __name__ == "__main__":
    main()
