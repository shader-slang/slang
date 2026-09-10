#!/bin/bash
# Install the GitHub artifact-backed scaler auto-updater on gpu-scaler-host.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

PROJECT="${PROJECT:-slang-runners}"
ZONE="${ZONE:-us-west1-a}"
VM_NAME="${VM_NAME:-gpu-scaler-host}"
REMOTE_TMP="/tmp/scaler-auto-update.$$"

UPDATE_SCRIPT="${SCRIPT_DIR}/update-scaler-from-github-artifact.sh"
SERVICE_FILE="${SCRIPT_DIR}/scaler-auto-update.service"
TIMER_FILE="${SCRIPT_DIR}/scaler-auto-update.timer"

for path in "$UPDATE_SCRIPT" "$SERVICE_FILE" "$TIMER_FILE"; do
  if [ ! -f "$path" ]; then
    echo "ERROR: missing $path" >&2
    exit 1
  fi
done

echo "=== Installing scaler auto-update timer on ${VM_NAME} ==="

gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" --command="
    set -e
    rm -rf '${REMOTE_TMP}'
    mkdir -p '${REMOTE_TMP}'
"

gcloud compute scp "$UPDATE_SCRIPT" "${VM_NAME}:${REMOTE_TMP}/update-scaler-from-github-artifact.sh" \
  --zone="$ZONE" --project="$PROJECT"
gcloud compute scp "$SERVICE_FILE" "${VM_NAME}:${REMOTE_TMP}/scaler-auto-update.service" \
  --zone="$ZONE" --project="$PROJECT"
gcloud compute scp "$TIMER_FILE" "${VM_NAME}:${REMOTE_TMP}/scaler-auto-update.timer" \
  --zone="$ZONE" --project="$PROJECT"

gcloud compute ssh "$VM_NAME" --zone="$ZONE" --project="$PROJECT" --command="
    set -e
    sudo install -m 755 '${REMOTE_TMP}/update-scaler-from-github-artifact.sh' /opt/scaler/update-scaler-from-github-artifact.sh
    sudo install -m 644 '${REMOTE_TMP}/scaler-auto-update.service' /etc/systemd/system/scaler-auto-update.service
    sudo install -m 644 '${REMOTE_TMP}/scaler-auto-update.timer' /etc/systemd/system/scaler-auto-update.timer
    sudo systemctl daemon-reload
    sudo systemctl enable --now scaler-auto-update.timer
    systemctl list-timers scaler-auto-update.timer --no-pager
    rm -rf '${REMOTE_TMP}'
"

echo ""
echo "=== Auto-update timer installed ==="
