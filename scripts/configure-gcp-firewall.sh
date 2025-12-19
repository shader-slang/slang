#!/bin/bash
#
# Configure GCP Firewall Rules for ARC Runner
#
# This script configures GCP firewall rules to allow the required
# outbound HTTPS connections for GitHub Copilot and GitHub Actions.
#
# Usage: ./configure-gcp-firewall.sh
#
# Prerequisites:
# - gcloud CLI installed and authenticated
# - Permissions to create firewall rules in the project
#

set -e

PROJECT="slang-runners"
ZONE="us-west1-a"
VM_NAME="slang-ubuntu-runner-3"
NETWORK="default"

echo "=========================================="
echo "GCP Firewall Configuration for ARC Runner"
echo "=========================================="
echo ""
echo "Project: $PROJECT"
echo "Zone: $ZONE"
echo "VM: $VM_NAME"
echo ""

# Check if gcloud is installed
if ! command -v gcloud &>/dev/null; then
  echo "ERROR: gcloud CLI not found. Please install it first."
  echo "Visit: https://cloud.google.com/sdk/docs/install"
  exit 1
fi

# Verify authentication
echo "Verifying gcloud authentication..."
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q "@"; then
  echo "ERROR: Not authenticated with gcloud. Run: gcloud auth login"
  exit 1
fi

echo "✓ Authenticated as: $(gcloud auth list --filter=status:ACTIVE --format='value(account)')"
echo ""

# Set project
echo "Setting project to $PROJECT..."
gcloud config set project "$PROJECT"
echo ""

# Create network tag for the runner
RUNNER_TAG="slang-arc-runner"

echo "Step 1: Adding network tag to VM..."
gcloud compute instances add-tags "$VM_NAME" \
  --zone="$ZONE" \
  --tags="$RUNNER_TAG" \
  2>&1 || echo "Note: Tag may already exist"

echo "✓ Network tag '$RUNNER_TAG' added to $VM_NAME"
echo ""

# Create egress firewall rule for HTTPS
RULE_NAME="allow-arc-runner-https-egress"

echo "Step 2: Creating firewall rule for HTTPS egress..."

# Check if rule already exists
if gcloud compute firewall-rules describe "$RULE_NAME" --project="$PROJECT" &>/dev/null; then
  echo "⚠ Firewall rule '$RULE_NAME' already exists. Deleting and recreating..."
  gcloud compute firewall-rules delete "$RULE_NAME" --project="$PROJECT" --quiet
fi

# Create the firewall rule
gcloud compute firewall-rules create "$RULE_NAME" \
  --project="$PROJECT" \
  --direction=EGRESS \
  --priority=1000 \
  --network="$NETWORK" \
  --action=ALLOW \
  --rules=tcp:443 \
  --destination-ranges=0.0.0.0/0 \
  --target-tags="$RUNNER_TAG" \
  --description="Allow HTTPS egress for ARC runner to GitHub Copilot and Actions endpoints"

echo "✓ Firewall rule '$RULE_NAME' created"
echo ""

# Create egress firewall rule for DNS
DNS_RULE_NAME="allow-arc-runner-dns-egress"

echo "Step 3: Creating firewall rule for DNS egress..."

if gcloud compute firewall-rules describe "$DNS_RULE_NAME" --project="$PROJECT" &>/dev/null; then
  echo "⚠ Firewall rule '$DNS_RULE_NAME' already exists. Deleting and recreating..."
  gcloud compute firewall-rules delete "$DNS_RULE_NAME" --project="$PROJECT" --quiet
fi

gcloud compute firewall-rules create "$DNS_RULE_NAME" \
  --project="$PROJECT" \
  --direction=EGRESS \
  --priority=1000 \
  --network="$NETWORK" \
  --action=ALLOW \
  --rules=udp:53,tcp:53 \
  --destination-ranges=0.0.0.0/0 \
  --target-tags="$RUNNER_TAG" \
  --description="Allow DNS queries for ARC runner"

echo "✓ Firewall rule '$DNS_RULE_NAME' created"
echo ""

# List created rules
echo "=========================================="
echo "Firewall Rules Created:"
echo "=========================================="
gcloud compute firewall-rules list \
  --filter="name:($RULE_NAME OR $DNS_RULE_NAME)" \
  --format="table(name,direction,priority,targetTags,allowed)" \
  --project="$PROJECT"

echo ""
echo "=========================================="
echo "✓ Firewall configuration complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Run the network connectivity test on $VM_NAME"
echo "   ./scripts/test-arc-network-connectivity.sh"
echo ""
echo "2. If test passes, proceed with ARC installation"
echo "   ./scripts/install-arc-pilot.sh"
