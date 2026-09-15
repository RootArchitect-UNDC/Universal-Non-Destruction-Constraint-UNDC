#!/bin/bash
# ------------------------------------------------------------
# UNDC Enforcement Demo — run_test.sh
# Demonstrates: kernel hook firing, trie lookup, -EPERM refusal
# Requires: root (sudo), a Linux kernel with BPF LSM enabled
# ------------------------------------------------------------
set -e

DENY_PATH="/tmp/undc-deny-test"
ALLOW_PATH="/bin/ls"

echo "===================================================="
echo "🛡️  UNDC Enforcement Demo"
echo "===================================================="

# 0. Ensure the test binary exists
if [ ! -x "$DENY_PATH" ]; then
    echo "🔧 Creating test binary at $DENY_PATH (copied from /bin/echo)"
    cp /bin/echo "$DENY_PATH"
    chmod +x "$DENY_PATH"
fi

# 1. Build (assumes Makefile produces undc-daemon and undc_compliance.bpf.o)
echo "📦 Building daemon and eBPF object..."
make undc-daemon undc_compliance.bpf.o

# 2. Start daemon in the background with the deny path as argument
echo "🚀 Starting daemon (deny: $DENY_PATH)..."
sudo ./undc-daemon "$DENY_PATH" &
DAEMON_PID=$!

sleep 2  # let it load, attach, and seed the trie

# 3. Sanity check: run the ALLOWED path — should succeed
echo "✅ Testing allowed path: $ALLOW_PATH"
if "$ALLOW_PATH" >/dev/null 2>&1; then
    echo "   → allowed (as expected)"
else
    echo "   → UNEXPECTED failure"
fi

# 4. Run the DENIED path — should be refused by the kernel hook
echo "🛑 Testing denied path: $DENY_PATH"
if "$DENY_PATH" "hello" 2>/dev/null; then
    echo "   → UNEXPECTED: execution succeeded"
else
    echo "   → refused (as expected; check daemon output above)"
fi

# 5. Clean up
echo "🧹 Stopping daemon..."
sudo kill $DAEMON_PID 2>/dev/null || true
wait $DAEMON_PID 2>/dev/null || true

echo "===================================================="
echo "Demo complete. Daemon output above shows the kernel"
echo "hook firing for both attempts, with action=ALLOW and"
echo "action=DENY respectively."
echo "===================================================="
