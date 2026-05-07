#!/bin/bash
# Update the scaler binary on the host VM
#
# Use this after rebuilding the scaler binary to deploy a new version.
#
# Usage:
#   cd extras/scaler
#   GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
#   ./deploy/update-scaler.sh

set -euo pipefail

# Resolve script directory so paths work regardless of cwd
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SCALER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

PROJECT="slang-runners"
ZONE="us-west1-a"
VM_NAME="gpu-scaler-host"
BINARY="${SCALER_DIR}/scaler-linux"

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

echo "Stopping services, replacing binary, restarting..."
# Keep the service list synchronized between the stop block and the restart loop.
# scaler-linux-sm80plus was added in PR #10967 (2026-04-29) but was missing from
# this script, so deployments silently left the SM80Plus tier on the old binary
# until manually restarted.
SCALER_SERVICES="scaler-windows scaler-linux scaler-linux-sm80plus scaler-windows-build"
gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo systemctl stop ${SCALER_SERVICES} 2>/dev/null || true
    sudo mv /tmp/scaler /opt/scaler/scaler
    sudo chmod 755 /opt/scaler/scaler
    sudo chown scaler:scaler /opt/scaler/scaler
    failed=0
    for svc in ${SCALER_SERVICES}; do
        if sudo systemctl is-enabled \$svc 2>/dev/null | grep -q enabled; then
            sudo systemctl start \$svc
            if sudo systemctl is-active \$svc >/dev/null 2>&1; then
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
