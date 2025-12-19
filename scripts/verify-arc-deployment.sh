#!/bin/bash
#
# Verify ARC Deployment
#
# This script verifies that the ARC pilot deployment is working correctly.
#
# Run this ON slang-ubuntu-runner-3 after installation is complete.
#

set -e

# Configuration
RUNNERS_NAMESPACE="slang-runners"
ARC_NAMESPACE="actions-runner-system"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=========================================="
echo "ARC Deployment Verification"
echo "==========================================${NC}"
echo ""

# Track overall status
ALL_CHECKS_PASSED=true

# Function to check status
check_status() {
  local description=$1
  local command=$2

  echo -n "Checking ${description}... "

  if eval "$command" &>/dev/null; then
    echo -e "${GREEN}PASS${NC}"
    return 0
  else
    echo -e "${RED}FAIL${NC}"
    ALL_CHECKS_PASSED=false
    return 1
  fi
}

# Check 1: k3s is running
echo -e "${BLUE}=== Kubernetes Cluster ===${NC}"
check_status "k3s cluster" "kubectl cluster-info"
check_status "k3s node ready" "kubectl get nodes | grep -q Ready"
echo ""

# Check 2: cert-manager
echo -e "${BLUE}=== cert-manager ===${NC}"
check_status "cert-manager namespace" "kubectl get namespace cert-manager"
check_status "cert-manager deployment" "kubectl get deployment cert-manager -n cert-manager"
check_status "cert-manager pods running" "kubectl get pods -n cert-manager | grep -q Running"
echo ""

# Check 3: ARC controller
echo -e "${BLUE}=== ARC Controller ===${NC}"
check_status "ARC namespace" "kubectl get namespace $ARC_NAMESPACE"
check_status "ARC controller deployment" "kubectl get deployment -n $ARC_NAMESPACE | grep -q arc"
check_status "ARC controller running" "kubectl get pods -n $ARC_NAMESPACE | grep -q Running"

if kubectl get pods -n "$ARC_NAMESPACE" | grep -q Running; then
  echo -e "ARC controller pod:"
  kubectl get pods -n "$ARC_NAMESPACE"
fi
echo ""

# Check 4: NVIDIA device plugin
echo -e "${BLUE}=== NVIDIA GPU Support ===${NC}"
check_status "NVIDIA device plugin" "kubectl get daemonset -n kube-system nvidia-device-plugin-daemonset"
check_status "GPU resource available" "kubectl describe nodes | grep -q 'nvidia.com/gpu'"

echo "GPU resources on node:"
kubectl describe nodes | grep -A 5 "nvidia.com/gpu" || echo "GPU info not available"
echo ""

# Check 5: Runner deployment
echo -e "${BLUE}=== Runner Deployment ===${NC}"
check_status "Runners namespace" "kubectl get namespace $RUNNERS_NAMESPACE"
check_status "Runner deployment" "kubectl get runnerdeployment -n $RUNNERS_NAMESPACE | grep -q slang-gpu-runners"

# Get detailed runner status
echo ""
echo "Runner deployment status:"
kubectl get runnerdeployment -n "$RUNNERS_NAMESPACE"
echo ""

echo "Runner pods:"
kubectl get pods -n "$RUNNERS_NAMESPACE"
echo ""

# Check if runner pods are ready
if kubectl get pods -n "$RUNNERS_NAMESPACE" | grep -q "Running"; then
  RUNNER_POD=$(kubectl get pods -n "$RUNNERS_NAMESPACE" -l app=slang-gpu-runners -o jsonpath='{.items[0].metadata.name}')

  if [ -n "$RUNNER_POD" ]; then
    echo -e "${BLUE}=== Runner Pod Details ===${NC}"
    echo "Pod: $RUNNER_POD"
    echo ""

    echo "Pod status:"
    kubectl describe pod "$RUNNER_POD" -n "$RUNNERS_NAMESPACE" | grep -A 10 "Conditions:"
    echo ""

    echo "Recent logs:"
    kubectl logs "$RUNNER_POD" -n "$RUNNERS_NAMESPACE" --tail=20 || echo "Logs not available yet"
  fi
else
  echo -e "${YELLOW}⚠ No running runner pods found${NC}"
  echo "This may be normal if the pod is still starting up."
  echo "Check pod status with: kubectl get pods -n $RUNNERS_NAMESPACE"
fi

echo ""

# Check 6: GitHub runner registration
echo -e "${BLUE}=== GitHub Integration ===${NC}"
echo "To verify runner registration in GitHub:"
echo "  1. Visit: https://github.com/shader-slang/slang/settings/actions/runners"
echo "  2. Look for a runner with labels: Linux, self-hosted, GPU, arc"
echo "  3. Status should be 'Idle' (green)"
echo ""

# Summary
echo -e "${BLUE}=========================================="
echo "Verification Summary"
echo "==========================================${NC}"
echo ""

if [ "$ALL_CHECKS_PASSED" = true ]; then
  echo -e "${GREEN}✓ All automated checks PASSED${NC}"
  echo ""
  echo "Next steps:"
  echo "1. Verify runner appears in GitHub (see URL above)"
  echo "2. Run test workflow:"
  echo "   https://github.com/shader-slang/slang/actions/workflows/test-arc-runner.yml"
  echo "3. If test passes, proceed with Phase 3 (Configure Copilot)"
  exit 0
else
  echo -e "${RED}✗ Some checks FAILED${NC}"
  echo ""
  echo "Troubleshooting steps:"
  echo "1. Check pod logs: kubectl logs -n $RUNNERS_NAMESPACE <pod-name>"
  echo "2. Check events: kubectl get events -n $RUNNERS_NAMESPACE"
  echo "3. Verify GitHub PAT is valid"
  echo "4. Review ARC controller logs: kubectl logs -n $ARC_NAMESPACE <arc-pod>"
  exit 1
fi
