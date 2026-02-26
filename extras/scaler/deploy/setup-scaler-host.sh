#!/bin/bash
# Deploy GPU Runner Scaler Host
#
# Creates a small always-on VM and deploys the scaler binary as
# systemd services. One service per platform (Linux GPU, Windows GPU).
#
# Prerequisites:
#   - gcloud CLI authenticated
#   - Go 1.25+ installed (for building the binary)
#   - GitHub credentials ready (PAT or App)
#
# Usage:
#   cd extras/scaler
#
#   # 1. Build the scaler binary for Linux amd64
#   GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
#
#   # 2. Create scaler.env with your credentials
#   cp deploy/scaler.env.example deploy/scaler.env
#   # Edit deploy/scaler.env with your GitHub token or App credentials
#
#   # 3. Run this script
#   ./deploy/setup-scaler-host.sh

set -euo pipefail

# Resolve script directory so paths work regardless of cwd
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SCALER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

PROJECT="slang-runners"
ZONE="us-west1-a"
VM_NAME="gpu-scaler-host"
MACHINE_TYPE="e2-small"
BINARY="${SCALER_DIR}/scaler-linux"
ENV_FILE="${SCALER_DIR}/deploy/scaler.env"

# Validate prerequisites
if [ ! -f "${BINARY}" ]; then
  echo "ERROR: Binary not found at ${BINARY}"
  echo "Build it first:"
  echo "  cd extras/scaler && GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler"
  exit 1
fi

if [ ! -f "${ENV_FILE}" ]; then
  echo "ERROR: ${ENV_FILE} not found."
  echo "Create it from the example:"
  echo "  cp extras/scaler/deploy/scaler.env.example extras/scaler/deploy/scaler.env"
  echo "  # Then edit with your credentials"
  exit 1
fi

echo "=== Deploying GPU Scaler Host ==="
echo ""

# Step 1: Create VM if it doesn't exist
echo "--- Step 1: Creating scaler VM ---"
if gcloud compute instances describe "${VM_NAME}" \
  --zone="${ZONE}" --project="${PROJECT}" >/dev/null 2>&1; then
  echo "VM ${VM_NAME} already exists."
else
  gcloud compute instances create "${VM_NAME}" \
    --project="${PROJECT}" \
    --zone="${ZONE}" \
    --machine-type="${MACHINE_TYPE}" \
    --image-family=ubuntu-2204-lts \
    --image-project=ubuntu-os-cloud \
    --boot-disk-size=20 \
    --boot-disk-type=pd-standard \
    --scopes=compute-rw \
    --tags=scaler-host
  echo "VM created. Waiting for SSH to be ready..."
  sleep 30
fi
echo ""

# Step 2: Create scaler user and directories on VM
echo "--- Step 2: Setting up scaler user ---"
gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo useradd -r -s /usr/sbin/nologin scaler 2>/dev/null || true
    sudo mkdir -p /opt/scaler
    sudo chown scaler:scaler /opt/scaler
"
echo ""

# Step 3: Upload binary and config
echo "--- Step 3: Uploading scaler binary ---"
gcloud compute scp "${BINARY}" "${VM_NAME}:/tmp/scaler" \
  --zone="${ZONE}" --project="${PROJECT}"

gcloud compute scp "${ENV_FILE}" "${VM_NAME}:/tmp/scaler.env" \
  --zone="${ZONE}" --project="${PROJECT}"

gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo mv /tmp/scaler /opt/scaler/scaler
    sudo chmod 755 /opt/scaler/scaler
    sudo mv /tmp/scaler.env /opt/scaler/scaler.env
    sudo chmod 600 /opt/scaler/scaler.env
    sudo chown scaler:scaler /opt/scaler/scaler /opt/scaler/scaler.env
"
echo ""

# Step 4: Upload and install systemd services
echo "--- Step 4: Installing systemd services ---"
gcloud compute scp "${SCALER_DIR}/deploy/scaler-windows.service" "${VM_NAME}:/tmp/scaler-windows.service" \
  --zone="${ZONE}" --project="${PROJECT}"
gcloud compute scp "${SCALER_DIR}/deploy/scaler-linux.service" "${VM_NAME}:/tmp/scaler-linux.service" \
  --zone="${ZONE}" --project="${PROJECT}"

gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo mv /tmp/scaler-windows.service /etc/systemd/system/scaler-windows.service
    sudo mv /tmp/scaler-linux.service /etc/systemd/system/scaler-linux.service
    sudo systemctl daemon-reload
"
echo ""

# Step 5: Start Windows scaler (Linux scaler needs its image first)
echo "--- Step 5: Starting Windows scaler ---"
gcloud compute ssh "${VM_NAME}" --zone="${ZONE}" --project="${PROJECT}" --command="
    sudo systemctl enable scaler-windows
    sudo systemctl start scaler-windows
    sudo systemctl status scaler-windows --no-pager
"
echo ""

echo "=== Deployment Complete ==="
echo ""
echo "Windows scaler is running. Linux scaler is installed but not started"
echo "(needs linux-gpu-runner instance template first)."
echo ""
echo "Useful commands:"
echo "  # SSH into scaler host"
echo "  gcloud compute ssh ${VM_NAME} --zone=${ZONE} --project=${PROJECT}"
echo ""
echo "  # View logs"
echo "  gcloud compute ssh ${VM_NAME} --zone=${ZONE} --project=${PROJECT} --command='sudo journalctl -u scaler-windows -f'"
echo "  gcloud compute ssh ${VM_NAME} --zone=${ZONE} --project=${PROJECT} --command='sudo journalctl -u scaler-linux -f'"
echo ""
echo "  # Restart services"
echo "  gcloud compute ssh ${VM_NAME} --zone=${ZONE} --project=${PROJECT} --command='sudo systemctl restart scaler-windows'"
echo ""
echo "  # Start Linux scaler (after creating linux-gpu-runner template)"
echo "  gcloud compute ssh ${VM_NAME} --zone=${ZONE} --project=${PROJECT} --command='sudo systemctl enable --now scaler-linux'"
