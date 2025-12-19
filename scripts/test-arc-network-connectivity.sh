#!/bin/bash
#
# Network Connectivity Test for ARC Runner
#
# This script tests connectivity to all required endpoints for GitHub Copilot
# and GitHub Actions on self-hosted runners.
#
# Usage: ./test-arc-network-connectivity.sh
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "ARC Runner Network Connectivity Test"
echo "=========================================="
echo ""

# Track overall success
ALL_PASSED=true

# Function to test endpoint connectivity
test_endpoint() {
  local endpoint=$1
  local port=${2:-443}
  local description=$3

  echo -n "Testing ${description}... "

  # Test DNS resolution
  if ! host "$endpoint" >/dev/null 2>&1; then
    echo -e "${RED}FAIL${NC} (DNS resolution failed)"
    ALL_PASSED=false
    return 1
  fi

  # Test TCP connection
  if timeout 5 bash -c "cat < /dev/null > /dev/tcp/${endpoint}/${port}" 2>/dev/null; then
    echo -e "${GREEN}PASS${NC}"
    return 0
  else
    echo -e "${RED}FAIL${NC} (TCP connection failed)"
    ALL_PASSED=false
    return 1
  fi
}

# Function to test HTTPS endpoint with curl
test_https_endpoint() {
  local url=$1
  local description=$2
  local allow_4xx=${3:-false} # Optional: allow 4xx responses (API endpoints)

  echo -n "Testing ${description}... "

  local status_code=$(curl -s -o /dev/null -w "%{http_code}" --connect-timeout 5 --max-time 10 "$url" || echo "000")

  # Accept 2xx and 3xx as success
  if echo "$status_code" | grep -q "^[23]"; then
    echo -e "${GREEN}PASS${NC}"
    return 0
  fi

  # For API endpoints, 4xx means reachable but needs auth (still counts as connectivity success)
  if [ "$allow_4xx" = "true" ] && echo "$status_code" | grep -q "^4"; then
    echo -e "${GREEN}PASS${NC} (reachable, status: ${status_code})"
    return 0
  fi

  # Connection failed or 5xx error
  echo -e "${RED}FAIL${NC} (HTTP status: ${status_code})"
  ALL_PASSED=false
  return 1
}

echo "=== Required Endpoints for Copilot ==="
echo ""

# GitHub Copilot endpoints (API endpoints may return 4xx without auth - that's OK)
test_https_endpoint "https://api.githubcopilot.com" "Copilot API" true
test_https_endpoint "https://uploads.github.com" "GitHub Uploads"
test_https_endpoint "https://user-images.githubusercontent.com" "User Images CDN" true

echo ""
echo "=== Required Endpoints for GitHub Actions ==="
echo ""

# GitHub Actions endpoints
test_https_endpoint "https://github.com" "GitHub Platform"
test_https_endpoint "https://api.github.com" "GitHub API" true
test_endpoint "pipelines.actions.githubusercontent.com" 443 "Actions Pipelines"
test_endpoint "results-receiver.actions.githubusercontent.com" 443 "Actions Results Receiver"

echo ""
echo "=== Additional GitHub Infrastructure ==="
echo ""

# Additional GitHub infrastructure
test_https_endpoint "https://ghcr.io" "GitHub Container Registry"
test_https_endpoint "https://objects.githubusercontent.com" "GitHub Objects CDN"
test_https_endpoint "https://codeload.github.com" "GitHub Codeload"

echo ""
echo "=== Package Managers and Tools ==="
echo ""

# Package manager endpoints (commonly used in workflows)
test_https_endpoint "https://registry.npmjs.org" "npm Registry (optional)"
test_https_endpoint "https://pypi.org" "PyPI (optional)"
test_https_endpoint "https://index.docker.io" "Docker Hub (optional)"

echo ""
echo "=== NVIDIA/CUDA Endpoints ==="
echo ""

# NVIDIA endpoints (for GPU workflows)
test_https_endpoint "https://developer.download.nvidia.com" "NVIDIA Developer Downloads"

echo ""
echo "=========================================="
if [ "$ALL_PASSED" = true ]; then
  echo -e "${GREEN}✓ All connectivity tests PASSED${NC}"
  echo ""
  echo "Network configuration is ready for ARC deployment."
  exit 0
else
  echo -e "${RED}✗ Some connectivity tests FAILED${NC}"
  echo ""
  echo "Action Required:"
  echo "1. Review GCP firewall rules for this VM"
  echo "2. Check security groups allow outbound HTTPS (port 443)"
  echo "3. Verify no proxy or network restrictions"
  echo "4. Contact network/infrastructure team if issues persist"
  exit 1
fi
