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
gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo systemctl stop scaler-windows scaler-linux 2>/dev/null || true
    sudo mv /tmp/scaler /opt/scaler/scaler
    sudo chmod 755 /opt/scaler/scaler
    sudo chown scaler:scaler /opt/scaler/scaler
    sudo systemctl start scaler-windows 2>/dev/null || true
    sudo systemctl start scaler-linux 2>/dev/null || true
    echo ''
    echo 'Service status:'
    sudo systemctl is-active scaler-windows 2>/dev/null && echo '  scaler-windows: active' || echo '  scaler-windows: inactive'
    sudo systemctl is-active scaler-linux 2>/dev/null && echo '  scaler-linux: active' || echo '  scaler-linux: inactive'
"

echo ""
echo "=== Update complete ==="
