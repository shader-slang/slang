#!/bin/bash
#
# Install ARC (Actions Runner Controller) Pilot on slang-ubuntu-runner-3
#
# This script performs the complete installation of k3s and ARC for the pilot deployment.
#
# Prerequisites:
# - Run this script ON slang-ubuntu-runner-3 (SSH into the VM first)
# - Network connectivity test passed
# - GitHub PAT created and set in GITHUB_PAT environment variable
# - Traditional runner service stopped
#
# Usage:
#   export GITHUB_PAT="ghp_xxxxxxxxxxxxxxxxxxxx"
#   ./install-arc-pilot.sh
#

set -e

# Configuration
GITHUB_ORG="shader-slang"
GITHUB_REPO="slang"
RUNNER_REPLICAS=1
ARC_NAMESPACE="actions-runner-system"
RUNNERS_NAMESPACE="slang-runners"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=========================================="
echo "ARC Pilot Installation"
echo "==========================================${NC}"
echo ""

# Check prerequisites
echo "Checking prerequisites..."

# Check if running as correct user (not root)
if [ "$EUID" -eq 0 ]; then
  echo -e "${RED}ERROR: Do not run this script as root${NC}"
  echo "Run as the regular user that will manage the runner"
  exit 1
fi

# Check if GITHUB_PAT is set
if [ -z "$GITHUB_PAT" ]; then
  echo -e "${RED}ERROR: GITHUB_PAT environment variable not set${NC}"
  echo "Please set it with: export GITHUB_PAT='ghp_xxxx...'"
  exit 1
fi

echo -e "${GREEN}✓ Running as user: $(whoami)${NC}"
echo -e "${GREEN}✓ GitHub PAT is set${NC}"
echo ""

# Phase 2.1: Stop traditional runner service
echo -e "${BLUE}Phase 2.1: Stopping traditional runner service...${NC}"

if [ -d "$HOME/actions-runner" ]; then
  cd "$HOME/actions-runner"

  # Check if service is installed
  if sudo ./svc.sh status &>/dev/null; then
    echo "Stopping runner service..."
    sudo ./svc.sh stop || true

    echo "Uninstalling runner service..."
    sudo ./svc.sh uninstall || true

    echo -e "${GREEN}✓ Traditional runner service stopped and uninstalled${NC}"
  else
    echo -e "${YELLOW}⚠ Runner service not found (may already be uninstalled)${NC}"
  fi

  # Backup the runner directory (excluding work directory with permission issues)
  echo "Backing up runner configuration..."
  cd "$HOME"
  tar -czf "actions-runner-backup-$(date +%Y%m%d-%H%M%S).tar.gz" --exclude='actions-runner/_work' actions-runner/
  echo -e "${GREEN}✓ Runner configuration backed up${NC}"
else
  echo -e "${YELLOW}⚠ Runner directory not found at $HOME/actions-runner${NC}"
fi

echo ""

# Phase 2.2: Install k3s
echo -e "${BLUE}Phase 2.2: Installing k3s...${NC}"

if command -v kubectl &>/dev/null && kubectl cluster-info &>/dev/null; then
  echo -e "${YELLOW}⚠ Kubernetes cluster already detected${NC}"
  echo "Skipping k3s installation..."
else
  echo "Downloading and installing k3s..."
  curl -sfL https://get.k3s.io | sh -s - --write-kubeconfig-mode 644

  # Wait for k3s to be ready
  echo "Waiting for k3s to be ready..."
  sleep 10

  # Verify k3s installation
  if kubectl cluster-info &>/dev/null; then
    echo -e "${GREEN}✓ k3s installed successfully${NC}"
    kubectl get nodes
  else
    echo -e "${RED}ERROR: k3s installation failed${NC}"
    exit 1
  fi
fi

# Set up kubeconfig for current user
export KUBECONFIG=/etc/rancher/k3s/k3s.yaml

echo ""

# Phase 2.3: Install cert-manager
echo -e "${BLUE}Phase 2.3: Installing cert-manager...${NC}"

if kubectl get namespace cert-manager &>/dev/null; then
  echo -e "${YELLOW}⚠ cert-manager namespace already exists${NC}"
  echo "Skipping cert-manager installation..."
else
  echo "Applying cert-manager manifests..."
  kubectl apply -f https://github.com/cert-manager/cert-manager/releases/download/v1.13.0/cert-manager.yaml

  echo "Waiting for cert-manager to be ready..."
  kubectl wait --for=condition=Available --timeout=300s deployment/cert-manager -n cert-manager
  kubectl wait --for=condition=Available --timeout=300s deployment/cert-manager-webhook -n cert-manager
  kubectl wait --for=condition=Available --timeout=300s deployment/cert-manager-cainjector -n cert-manager

  echo -e "${GREEN}✓ cert-manager installed successfully${NC}"
fi

echo ""

# Phase 2.4: Install ARC controller
echo -e "${BLUE}Phase 2.4: Installing ARC controller...${NC}"

# Install Helm if not present
if ! command -v helm &>/dev/null; then
  echo "Installing Helm..."
  curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash
  echo -e "${GREEN}✓ Helm installed${NC}"
fi

# Add ARC Helm repo
echo "Adding ARC Helm repository..."
helm repo add actions-runner-controller https://actions-runner-controller.github.io/actions-runner-controller
helm repo update

# Check if ARC is already installed
if helm list -n "$ARC_NAMESPACE" | grep -q "^arc"; then
  echo -e "${YELLOW}⚠ ARC controller already installed${NC}"
  echo "Upgrading ARC controller..."
  helm upgrade arc \
    --namespace "$ARC_NAMESPACE" \
    actions-runner-controller/actions-runner-controller \
    --set authSecret.github_token="$GITHUB_PAT"
else
  echo "Installing ARC controller..."
  helm install arc \
    --namespace "$ARC_NAMESPACE" \
    --create-namespace \
    actions-runner-controller/actions-runner-controller \
    --set authSecret.github_token="$GITHUB_PAT"
fi

echo "Waiting for ARC controller to be ready..."
kubectl wait --for=condition=Available --timeout=300s deployment/arc-actions-runner-controller -n "$ARC_NAMESPACE" || true

echo -e "${GREEN}✓ ARC controller installed${NC}"
echo ""

# Phase 2.5: Install NVIDIA device plugin
echo -e "${BLUE}Phase 2.5: Installing NVIDIA device plugin...${NC}"

if kubectl get daemonset -n kube-system nvidia-device-plugin-daemonset &>/dev/null; then
  echo -e "${YELLOW}⚠ NVIDIA device plugin already installed${NC}"
else
  echo "Installing NVIDIA device plugin..."
  kubectl create -f https://raw.githubusercontent.com/NVIDIA/k8s-device-plugin/v0.14.0/nvidia-device-plugin.yml

  echo "Waiting for NVIDIA device plugin to be ready..."
  sleep 10

  echo -e "${GREEN}✓ NVIDIA device plugin installed${NC}"
fi

# Verify GPU is visible
echo "Verifying GPU availability in Kubernetes..."
if kubectl describe nodes | grep -q "nvidia.com/gpu"; then
  echo -e "${GREEN}✓ GPU detected by Kubernetes${NC}"
  kubectl describe nodes | grep -A 5 "nvidia.com/gpu"
else
  echo -e "${YELLOW}⚠ GPU not detected by Kubernetes${NC}"
  echo "This may be normal if GPU drivers are not installed"
fi

echo ""

# Phase 2.6: Create ARC scale set
echo -e "${BLUE}Phase 2.6: Creating ARC scale set...${NC}"

# Create namespace for runners
kubectl create namespace "$RUNNERS_NAMESPACE" --dry-run=client -o yaml | kubectl apply -f -

# Create runner deployment manifest
cat >/tmp/slang-arc-runner-set.yaml <<EOF
apiVersion: actions.summerwind.dev/v1alpha1
kind: RunnerDeployment
metadata:
  name: slang-gpu-runners
  namespace: ${RUNNERS_NAMESPACE}
spec:
  replicas: ${RUNNER_REPLICAS}
  template:
    spec:
      repository: ${GITHUB_ORG}/${GITHUB_REPO}
      labels:
        - Linux
        - self-hosted
        - GPU
        - arc
        - copilot
      
      # Use host Docker daemon (not Docker-in-Docker)
      dockerEnabled: false
      
      # Explicitly set workspace directory to avoid path issues
      workDir: /runner/_work
      
      image: summerwind/actions-runner:ubuntu-22.04
      
      env:
        - name: DOCKER_HOST
          value: unix:///var/run/docker.sock
          
      volumeMounts:
        # Mount host Docker socket for GPU access
        - name: dockersock
          mountPath: /var/run/docker.sock
        # Workspace directory for job execution
        - name: work
          mountPath: /runner/_work
        # Externals directory (Node.js and GitHub Actions tools)
        - name: externals
          mountPath: /runner/externals
          
      volumes:
        - name: dockersock
          hostPath:
            path: /var/run/docker.sock
            type: Socket
        - name: work
          hostPath:
            path: /runner/_work # Must match pod path for Docker bind-mounts
            type: DirectoryOrCreate
        - name: externals
          hostPath:
            path: /runner/externals # Contains Node.js for actions
            type: DirectoryOrCreate
EOF

echo "Applying runner deployment..."
kubectl apply -f /tmp/slang-arc-runner-set.yaml

echo "Waiting for runner pods to be created..."
sleep 15

echo -e "${GREEN}✓ ARC scale set created${NC}"
echo ""

# Check runner status
echo -e "${BLUE}Checking runner status...${NC}"
kubectl get runnerdeployment -n "$RUNNERS_NAMESPACE"
kubectl get pods -n "$RUNNERS_NAMESPACE"

echo ""
echo -e "${BLUE}=========================================="
echo "Installation Complete!"
echo "==========================================${NC}"
echo ""
echo -e "${GREEN}✓ k3s installed and running${NC}"
echo -e "${GREEN}✓ cert-manager installed${NC}"
echo -e "${GREEN}✓ ARC controller installed${NC}"
echo -e "${GREEN}✓ NVIDIA device plugin installed${NC}"
echo -e "${GREEN}✓ Runner scale set deployed${NC}"
echo ""
echo "Next steps:"
echo "1. Verify runner registration in GitHub:"
echo "   Visit: https://github.com/${GITHUB_ORG}/${GITHUB_REPO}/settings/actions/runners"
echo ""
echo "2. Check runner pod logs:"
echo "   kubectl logs -n ${RUNNERS_NAMESPACE} -l app=slang-gpu-runners --tail=50"
echo ""
echo "3. Test the runner with the test workflow:"
echo "   Go to: https://github.com/${GITHUB_ORG}/${GITHUB_REPO}/actions/workflows/test-arc-runner.yml"
echo "   Click 'Run workflow'"
echo ""
echo "Useful commands:"
echo "  kubectl get pods -n ${RUNNERS_NAMESPACE}              # Check runner pods"
echo "  kubectl logs -n ${RUNNERS_NAMESPACE} <pod-name>       # View runner logs"
echo "  kubectl describe pod -n ${RUNNERS_NAMESPACE} <pod>    # Debug pod issues"
echo "  kubectl get runnerdeployment -n ${RUNNERS_NAMESPACE}  # Check deployment"
echo ""
