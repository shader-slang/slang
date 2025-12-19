# ARC Pilot Execution Guide

This guide provides step-by-step instructions to execute the ARC pilot deployment on slang-ubuntu-runner-3.

## Prerequisites Checklist

Before starting, ensure you have:

- [ ] Security review document reviewed: `docs/arc-pilot-security-review.md`
- [ ] Security approvals obtained (if required by your organization)
- [ ] Access to GCP project `slang-runners`
- [ ] `gcloud` CLI installed and authenticated
- [ ] SSH access to `slang-ubuntu-runner-3`
- [ ] GitHub admin access to `shader-slang/slang` repository

## Phase 1: Security & Prerequisites

### Step 1.2: Configure GCP Firewall Rules

**Duration**: 2-3 minutes

Run from your local machine (where gcloud CLI is configured):

```bash
cd ~/nv/src/slang
./scripts/configure-gcp-firewall.sh
```

This script will:

- Add network tag `slang-arc-runner` to slang-ubuntu-runner-3
- Create firewall rule for HTTPS egress (port 443)
- Create firewall rule for DNS egress (port 53)

**Expected output**: `✓ Firewall configuration complete!`

✅ **Mark Phase 1.2 complete** when this succeeds.

---

### Step 1.3: Test Network Connectivity

**Duration**: 1-2 minutes

SSH to the runner VM and test connectivity:

```bash
# SSH to runner-3
gcloud compute ssh --zone "us-west1-a" "slang-ubuntu-runner-3" --project "slang-runners"

# Clone or sync the repository
cd ~/nv/src/slang
git pull origin jkiviluoto/enable-copilot-on-selfhosted

# Run connectivity test
./scripts/test-arc-network-connectivity.sh
```

**Expected output**: `✓ All connectivity tests PASSED`

**If tests fail**:

- Review firewall rules configuration
- Check for any proxy or network restrictions
- Verify DNS resolution works

✅ **Mark Phase 1.3 complete** when all tests pass.

---

### Step 1.4: Create GitHub PAT

**Duration**: 2-3 minutes

Follow the instructions in `docs/arc-pilot-github-pat-setup.md`:

**Quick steps**:

1. Go to: https://github.com/settings/tokens
2. Click "Generate new token (classic)"
3. Select scopes: `repo`, `workflow`
4. Set expiration: 90 days (for pilot)
5. Generate and copy the token

**Save the token securely**:

```bash
# On slang-ubuntu-runner-3
export GITHUB_PAT="ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

# Persist for future sessions (optional)
echo "export GITHUB_PAT='ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'" >> ~/.bashrc
```

**Security note**: Never commit this token to git!

✅ **Mark Phase 1.4 complete** when PAT is created and saved.

---

## Phase 2: ARC Pilot Installation

### Important: Complete Installation Script

**All Phase 2 steps (2.1-2.6) are automated in a single script!**

**Duration**: 10-15 minutes

Run on slang-ubuntu-runner-3:

```bash
# Ensure you're on the runner VM
gcloud compute ssh --zone "us-west1-a" "slang-ubuntu-runner-3" --project "slang-runners"

# Ensure GITHUB_PAT is set
echo $GITHUB_PAT  # Should show your token

# Navigate to repo
cd ~/nv/src/slang

# Run the installation script
./scripts/install-arc-pilot.sh
```

### What This Script Does

The script will automatically:

- ✅ **Phase 2.1**: Stop and uninstall traditional runner service
- ✅ **Phase 2.2**: Install k3s (lightweight Kubernetes)
- ✅ **Phase 2.3**: Install cert-manager
- ✅ **Phase 2.4**: Install ARC controller with your GitHub PAT
- ✅ **Phase 2.5**: Install NVIDIA device plugin
- ✅ **Phase 2.6**: Create and deploy ARC scale set

**Expected output**: `Installation Complete!` with all green checkmarks

**What to watch for**:

- Script will create backup of traditional runner config
- May see warnings about existing resources (normal if re-running)
- GPU detection may take a few moments

✅ **Mark Phases 2.1-2.6 complete** when installation succeeds.

---

### Step 2.8: Verify ARC Deployment

**Duration**: 2-3 minutes

Run the verification script on slang-ubuntu-runner-3:

```bash
./scripts/verify-arc-deployment.sh
```

This checks:

- k3s cluster is running
- cert-manager is healthy
- ARC controller is running
- NVIDIA GPU is detected
- Runner pods are created

**Expected output**: `✓ All automated checks PASSED`

**Then manually verify in GitHub**:

1. Go to: https://github.com/shader-slang/slang/settings/actions/runners
2. Look for a runner with labels: `Linux`, `self-hosted`, `GPU`, `arc`
3. Status should be **Idle** (green circle)

**If runner doesn't appear**:

```bash
# Check runner pod logs
kubectl logs -n slang-runners -l app=slang-gpu-runners --tail=50

# Check ARC controller logs
kubectl logs -n actions-runner-system -l app=arc-actions-runner-controller --tail=50

# Check for events
kubectl get events -n slang-runners
```

✅ **Mark Phase 2.8 complete** when runner appears in GitHub as Idle.

---

### Step 2.9: Test GPU Access

**Duration**: 3-5 minutes

Test the runner with the GPU test workflow:

1. Go to: https://github.com/shader-slang/slang/actions/workflows/test-arc-runner.yml

2. Click **"Run workflow"** button
   - Branch: `jkiviluoto/enable-copilot-on-selfhosted`
   - Click green "Run workflow" button

3. Wait for workflow to complete (2-3 minutes)

4. Verify all steps pass:
   - ✅ Check GPU with nvidia-smi
   - ✅ Check Docker
   - ✅ Test CUDA container with GPU
   - ✅ Check Vulkan ICD configuration
   - ✅ Check system resources

**Expected result**: All steps green ✅

**If workflow fails**:

- Click on failed step to see error details
- Check runner pod logs: `kubectl logs -n slang-runners <pod-name>`
- Verify GPU is accessible: `nvidia-smi` on the host VM

✅ **Mark Phase 2.9 complete** when test workflow passes.

---

## Phase 3: Configure Copilot

### Pilot Completion Status

✅ **Infrastructure Complete** (Dec 17, 2025):

- ARC controller running on k3s (slang-ubuntu-runner-3)
- Runner registered and idle in GitHub
- GPU access working: `--gpus all` with nvidia-smi confirmed (Tesla T4, driver 550.54.15)
- Configuration: Uses host Docker socket (not dind) with NVIDIA runtime

### Critical Configuration Details

**Working Setup**:

- Runner uses host Docker daemon (mounted `/var/run/docker.sock`)
- Docker socket permissions: `chmod 666 /var/run/docker.sock` (pilot only - fix for production)
- Docker daemon.json configured with `nvidia` as default runtime
- Runner deployment: `dockerEnabled: false`, explicit Docker socket mount

**Updated Workflow**:

- `copilot-setup-steps.yml` configured to use `[self-hosted, Linux, GPU, arc]`
- Uses CUDA 12.4.1 container with GPU passthrough matching existing CI setup

### Remaining Steps for Copilot:

1. ✅ `copilot-setup-steps.yml` workflow created and configured
2. ⚠️ **Disable Copilot firewall** in repo settings: https://github.com/shader-slang/slang/settings/copilot
3. ⚠️ **Requires license**: Copilot Pro/Business/Enterprise needed to test coding agent

### Testing Copilot (When License Available)

Once someone with a Copilot license is ready:

1. Ensure firewall is disabled in repo settings
2. Open Copilot chat in GitHub
3. Delegate a task (e.g., "Add a test for feature X")
4. Verify it runs on the ARC runner (check Actions tab)
5. Confirm GPU tests execute successfully

---

## Troubleshooting

### Common Issues

**Issue**: `GITHUB_PAT not set`

- **Solution**: `export GITHUB_PAT="ghp_xxx..."`

**Issue**: `k3s installation failed`

- **Solution**: Check if port 6443 is available: `sudo lsof -i :6443`
- Clean up: `sudo /usr/local/bin/k3s-uninstall.sh` and retry

**Issue**: `ARC controller can't authenticate`

- **Solution**: Verify PAT has correct scopes (repo, workflow)
- Check ARC logs: `kubectl logs -n actions-runner-system -l app=arc-actions-runner-controller`

**Issue**: `GPU not detected in pod`

- **Solution**: Verify NVIDIA device plugin is running: `kubectl get daemonset -n kube-system`
- Check node has GPU: `kubectl describe nodes | grep nvidia.com/gpu`

**Issue**: `Runner not appearing in GitHub`

- **Solution**: Check runner pod logs for authentication errors
- Verify repository name is correct in deployment: `shader-slang/slang`

### Getting Help

If you encounter issues:

1. Collect logs: `kubectl logs -n slang-runners <pod-name> > runner-logs.txt`
2. Check deployment: `kubectl describe runnerdeployment -n slang-runners slang-gpu-runners`
3. Review security review document for network requirements

---

## Rollback Procedure

If you need to rollback:

```bash
# Delete ARC scale set
kubectl delete namespace slang-runners

# Stop k3s
sudo systemctl stop k3s

# Uninstall k3s (if needed)
sudo /usr/local/bin/k3s-uninstall.sh

# Restore traditional runner
cd ~/actions-runner
sudo ./svc.sh install
sudo ./svc.sh start
```

---

## Success Criteria

You've successfully completed the pilot when:

- [x] All Phase 1 tasks complete (firewall, connectivity, PAT)
- [x] All Phase 2 tasks complete (k3s, ARC, scale set deployed)
- [x] Runner appears in GitHub with `arc` label as **Idle**
- [x] Test workflow passes with GPU access confirmed

**Time to complete**: Approximately 30-45 minutes total

---

## Next Steps After Pilot Success

1. **Evaluate pilot** (1-2 weeks):
   - Monitor runner stability
   - Review network logs
   - Track any issues or errors

2. **Phase 3: Configure Copilot**:
   - Create Copilot setup workflow
   - Disable integrated firewall
   - Test Copilot on ARC runner

3. **Decision point**:
   - Proceed with full migration (all 3 runners)?
   - Keep separate infrastructure for CI?
   - Scale to production?

Refer to the main plan: `~/.claude/plans/tingly-wobbling-hopper.md`
