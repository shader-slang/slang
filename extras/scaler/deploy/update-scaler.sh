#!/bin/bash
# Update the scaler binary on the host VM.
#
# Use this after rebuilding the scaler binary to deploy a new version.
#
# Usage:
#   cd extras/scaler
#   GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
#   ./deploy/update-scaler.sh
#
# Environment overrides:
#   DRAIN_TIMEOUT_SECONDS  Per-service drain wait (default 300, i.e. 5 min).
#                          Set to 0 to skip drain and force-stop (orphan risk).

set -euo pipefail

# Resolve script directory so paths work regardless of cwd
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SCALER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

PROJECT="slang-runners"
ZONE="us-west1-a"
VM_NAME="gpu-scaler-host"
BINARY="${SCALER_DIR}/scaler-linux"
DRAIN_TIMEOUT_SECONDS="${DRAIN_TIMEOUT_SECONDS:-300}"

if [ ! -f "${BINARY}" ]; then
  echo "ERROR: Binary not found at ${BINARY}"
  echo "Build it first:"
  echo "  cd extras/scaler && GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler"
  exit 1
fi

echo "=== Updating scaler binary ==="

echo "Uploading binary..."
gcloud compute scp "${BINARY}" "${VM_NAME}:/tmp/scaler" \
  --zone="${ZONE}" --project="${PROJECT}"

echo "Draining and stopping services, replacing binary, restarting..."
# Keep the service list synchronized between the stop block and the restart loop.
# scaler-linux-sm80plus was added in PR #10967 (2026-04-29) but was missing from
# this script, so deployments silently left the SM80Plus tier on the old binary
# until manually restarted.
SCALER_SERVICES="scaler-windows scaler-linux scaler-linux-sm80plus scaler-windows-build"

# Drain-then-stop sequence (#11067):
#   1. systemctl reload sends SIGUSR1 -> scaler enters drain mode, stops
#      accepting new jobs, exits cleanly when active_vms == 0.
#   2. Wait up to DRAIN_TIMEOUT_SECONDS for the service to deactivate.
#   3. If still active (long-running job, hung scaler): fall back to
#      systemctl stop. The new binary's scale-set-preserving defer means
#      runners orphan ONLY in this fallback path, not on every deploy.
#
# Restart=always in the unit files would auto-restart the scaler after a
# clean drain exit. We therefore explicitly `systemctl stop` after drain to
# keep the old binary down until the swap-and-start step below.
gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    set -u
    SERVICES='${SCALER_SERVICES}'
    DRAIN_TIMEOUT='${DRAIN_TIMEOUT_SECONDS}'
    for svc in \$SERVICES; do
        if ! sudo systemctl is-active --quiet \"\$svc\"; then
            echo \"  \$svc: already inactive, skipping drain\"
            continue
        fi
        if [ \"\$DRAIN_TIMEOUT\" -eq 0 ]; then
            echo \"  \$svc: DRAIN_TIMEOUT_SECONDS=0, force-stopping (orphan risk)\"
            sudo systemctl stop \"\$svc\" || true
            continue
        fi
        echo \"  \$svc: requesting drain (timeout \${DRAIN_TIMEOUT}s)\"
        sudo systemctl reload \"\$svc\" || true
        elapsed=0
        while [ \"\$elapsed\" -lt \"\$DRAIN_TIMEOUT\" ]; do
            if ! sudo systemctl is-active --quiet \"\$svc\"; then
                break
            fi
            sleep 5
            elapsed=\$((elapsed + 5))
        done
        if sudo systemctl is-active --quiet \"\$svc\"; then
            echo \"  \$svc: drain timeout after \${elapsed}s, force-stopping (orphan risk)\"
        else
            echo \"  \$svc: drained cleanly after \${elapsed}s\"
        fi
        # systemctl stop is idempotent; needed even after a clean drain to
        # block Restart=always from bringing the old binary back up.
        sudo systemctl stop \"\$svc\" || true
    done

    sudo mv /tmp/scaler /opt/scaler/scaler
    sudo chmod 755 /opt/scaler/scaler
    sudo chown scaler:scaler /opt/scaler/scaler

    failed=0
    for svc in \$SERVICES; do
        if sudo systemctl is-enabled \"\$svc\" 2>/dev/null | grep -q enabled; then
            sudo systemctl start \"\$svc\"
            if sudo systemctl is-active --quiet \"\$svc\"; then
                echo \"  \$svc: active\"
            else
                echo \"  \$svc: FAILED TO START\"
                failed=1
            fi
        else
            echo \"  \$svc: not enabled\"
        fi
    done
    exit \$failed
"

echo ""
echo "=== Update complete ==="
