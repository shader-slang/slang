# GCP Runner Scaler

Auto-scales Linux and Windows VMs on GCP based on GitHub Actions job queue
depth. Uses the [GitHub Actions Scale Set Client](https://github.com/actions/scaleset)
(no Kubernetes required). Both GPU test pools and CPU-only build/analytics pools
are managed the same way.

## Architecture

```text
┌─────────────────────────────────────────────────────────────────┐
│ e2-small VM (always-on)                                          │
│                                                                  │
│  scaler (Windows GPU)     --labels=Windows,self-hosted,GCP-T4    │
│  scaler (Windows build)   --labels=Windows,self-hosted,build     │
│  scaler (Linux GPU)       --labels=Linux,self-hosted,GPU,GCP     │
│  scaler (Linux SM80Plus)  --labels=Linux,self-hosted,SM80Plus    │
│  scaler (Linux build)     --labels=Linux,self-hosted,build,GCP   │
│  scaler (Linux analytics) --labels=Linux,self-hosted,analytics,GCP│
└──┬────────────┬────────────┬────────────┬────────────┬──────────┘
   ▼            ▼            ▼            ▼            ▼
┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────────┐
│ Win T4   │ │ Linux T4 │ │ Linux L4 │ │ Linux    │ │ Linux      │
│ + build  │ │ VMs      │ │ VMs      │ │ build    │ │ analytics  │
│ VMs      │ │ (GPU)    │ │ (GPU)    │ │ n1-std-8 │ │ e2-small   │
│ ephemeral│ │ ephemeral│ │ ephemeral│ │ no GPU   │ │ no GPU     │
│ scale 0  │ │ scale 0  │ │ scale 0  │ │ scale 0  │ │ scale 0    │
└──────────┘ └──────────┘ └──────────┘ └──────────┘ └────────────┘
```

Several instances of the same binary run on one control VM, each targeting a
different platform with different instance templates and labels. Zones are
selected dynamically based on GPU quota availability across US regions (CPU-only
pools skip the GPU-quota check via `--gcp-gpu-type=none`).

The CPU-only Linux **build** and **analytics** pools exist to keep work off the
GitHub-hosted runner pool, which is capped at 20 concurrent jobs org-wide on the
Free plan. Moving the `ubuntu-22.04` x86_64 build/sanitizer jobs and the
scheduled CI-analytics jobs onto self-hosted runners removes them from that
shared cap entirely. (See `slang-ci-docs` for the saturation analysis that
motivated this.)

Both pools carry the `GCP` label, like the Linux GPU test pool, so jobs pin to
the GCP scaler explicitly and cannot be picked up by another self-hosted host
that happens to share the generic `build` label (e.g. an internal bare-metal
pool). Routing by an unambiguous pool label avoids the silent misrouting that
the `GCP` label was introduced to prevent.

## Build

```bash
cd extras/scaler
go build -o scaler ./cmd/scaler

# Cross-compile for deployment (control VM runs Linux)
GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
```

## Run

```bash
# Windows GPU runners
./scaler \
  --url=https://github.com/shader-slang/slang \
  --name=windows-gpu-runners \
  --token=ghp_... \
  --labels=Windows,self-hosted,GCP-T4 \
  --platform=windows \
  --gcp-zones=us-east1-c,us-east1-d,us-central1-a,us-west1-a \
  --gcp-instance-template=windows-gpu-runner \
  --max-runners=5

# Linux GPU runners
./scaler \
  --url=https://github.com/shader-slang/slang \
  --name=linux-gpu-runners \
  --token=ghp_... \
  --labels=Linux,self-hosted,GPU,GCP \
  --platform=linux \
  --gcp-zones=us-east1-c,us-east1-d,us-central1-a,us-west1-a \
  --gcp-instance-template=linux-gpu-runner \
  --max-runners=10

# Linux SM80Plus GPU runners (L4 initially)
./scaler \
  --url=https://github.com/shader-slang/slang \
  --name=linux-gpu-sm80plus-runners \
  --token=ghp_... \
  --labels=Linux,self-hosted,SM80Plus \
  --platform=linux \
  --gcp-zones=us-east1-d,us-east1-b,us-east1-c,us-central1-a,us-west1-a \
  --gcp-instance-template=linux-gpu-runner-sm80plus-l4 \
  --gcp-gpu-type=nvidia-l4 \
  --vm-prefix=linux-sm80plus \
  --max-runners=1
```

## Configuration

| Flag                      | Default                      | Description                                               |
| ------------------------- | ---------------------------- | --------------------------------------------------------- |
| `--url`                   | (required)                   | GitHub URL (e.g. `https://github.com/shader-slang/slang`) |
| `--name`                  | `windows-gpu-runners`        | Scale set name (must be unique)                           |
| `--labels`                | `Windows,self-hosted,GCP-T4` | Comma-separated runner labels                             |
| `--runner-group`          | `default`                    | Runner group                                              |
| `--max-runners`           | `5`                          | Max concurrent VMs                                        |
| `--min-runners`           | `0`                          | Min warm VMs                                              |
| `--platform`              | `windows`                    | Runner platform: `windows` or `linux`                     |
| `--gcp-project`           | `slang-runners`              | GCP project                                               |
| `--gcp-zones`             | `us-east1-c,...,us-west1-a`  | Comma-separated zones (selected by GPU quota)             |
| `--gcp-instance-template` | `windows-gpu-runner`         | Instance template name                                    |
| `--gcp-gpu-type`          | `nvidia-tesla-t4`            | GPU type (for quota lookup)                               |

**Authentication** (flag or environment variable):

| Flag                    | Env Var                      | Description                  |
| ----------------------- | ---------------------------- | ---------------------------- |
| `--token`               | `SCALER_TOKEN`               | GitHub PAT                   |
| `--app-client-id`       | `SCALER_APP_CLIENT_ID`       | GitHub App client ID         |
| `--app-installation-id` | `SCALER_APP_INSTALLATION_ID` | GitHub App installation ID   |
| `--app-private-key`     | `SCALER_APP_PRIVATE_KEY`     | GitHub App private key (PEM) |

## Dynamic Zone Selection

The scaler checks GPU quota across all configured zones before creating a VM.
Zones are grouped by region (GPU quota is per-region in GCP), and the region
with the most available GPUs is selected. This allows spreading VMs across
regions to avoid quota limits.

```text
Configured zones: us-east1-c, us-east1-d, us-central1-a, us-west1-a
                       ↓
Query quota: us-east1 (5 free), us-central1 (3 free), us-west1 (0 free)
                       ↓
Selected: us-east1-c (most available)
```

If all regions are full, VM creation fails for that job but the scaler keeps
running and retries on the next polling cycle.

## Drain Mode (Seamless Updates)

Send `SIGUSR1` to enter drain mode. The scaler stops accepting new jobs but
continues processing completions for running VMs. When all VMs finish, it
exits cleanly.

```bash
# Via systemctl
sudo systemctl reload scaler-windows   # Enter drain mode
sudo journalctl -u scaler-windows -f   # Watch for "all VMs finished"
# ... wait for drain ...
sudo systemctl stop scaler-windows     # Stop (won't kill running VMs)

# Update binary
./deploy/update-scaler.sh

# Restart
sudo systemctl start scaler-windows
```

Or manually:

```bash
kill -USR1 $(pidof scaler)   # Drain
kill -TERM $(pidof scaler)   # Stop (after drain completes)
```

## Deployment

See `deploy/` directory:

```bash
# First time setup
cd extras/scaler
GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
cp deploy/scaler.env.example deploy/scaler.env   # Add your GitHub token
./deploy/setup-scaler-host.sh

# Update binary
cd extras/scaler
GOOS=linux GOARCH=amd64 go build -o scaler-linux ./cmd/scaler
./deploy/update-scaler.sh
```

**Files:**
| File | Purpose |
|------|---------|
| `deploy/setup-scaler-host.sh` | One-command deploy: creates VM, uploads binary, installs services |
| `deploy/update-scaler.sh` | Update binary on existing host |
| `deploy/scaler-windows.service` | systemd unit for Windows GPU scaler |
| `deploy/scaler-windows-build.service` | systemd unit for Windows build scaler (no GPU) |
| `deploy/scaler-linux.service` | systemd unit for Linux GPU scaler |
| `deploy/scaler-linux-sm80plus.service` | systemd unit for Linux SM80Plus scaler |
| `deploy/scaler-linux-build.service` | systemd unit for Linux build scaler (no GPU) |
| `deploy/scaler-linux-analytics.service` | systemd unit for Linux analytics scaler (no GPU, tiny VM) |
| `deploy/scaler.env.example` | Template for GitHub credentials |

## How It Works

1. **Polls GitHub** via Scale Set API (long-polling, ~50s intervals)
2. **Detects queued jobs** matching the configured labels
3. **Selects best zone** by checking GPU quota across configured regions
4. **Requests JIT config** from GitHub (one-time runner credentials)
5. **Creates GCP VM** from instance template, passes JIT config via metadata
6. **VM boots** (~2-3 minutes), startup script reads JIT config and starts runner
7. **Runner executes job** (ephemeral - one job only)
8. **Job completes** → scaler receives event and deletes VM immediately

## Base Images

Both platforms use base images created by snapshotting existing runners
(with the runner service removed). The startup script handles fresh
runner registration via JIT config on each boot.

**Windows image** (`--image-family=windows-gpu-runner`):

- Windows Server 2022, NVIDIA Driver, Visual Studio 2022, Git, Ninja, CMake, Python
- ccache at `C:/ccache-slang/` (from snapshot, read-only on ephemeral VMs)
- GitHub Actions Runner agent at `C:\actions-runner`

**Linux image** (`--image-family=linux-gpu-runner`):

- Ubuntu 22.04, NVIDIA Driver, Docker, nvidia-container-toolkit
- GitHub Actions Runner agent at `/actions-runner`

**Linux build image** (`--gcp-instance-template=linux-build-runner`):

- CPU-only `n1-standard-8` (no GPU attached, `--gcp-gpu-type=none`).
- The two container build jobs (`build-linux-debug/release-gcc-x86_64`) run the
  build inside the `linux-gpu-ci` Docker image, so the host only needs Docker —
  the existing `linux-gpu-runner` base image (Ubuntu 22.04 + Docker, GPU driver
  unused) is sufficient and is the simplest starting point. The wasm and
  sanitizer jobs build directly on the host toolchain; confirm the base image
  carries the toolchain they expect (gcc/clang, CMake, Ninja, Python, sccache)
  or extend the image before flipping those jobs.
- GitHub Actions Runner agent at `/actions-runner`.

**Linux analytics image** (`--gcp-instance-template=linux-analytics-runner`):

- Tiny CPU-only `e2-small` (no GPU). Runs only lightweight scheduled jobs
  (`ci-health`, `ci-analytics`): checkout + Python + `gh api` + GCP auth.
- Needs Python 3 and the GitHub CLI; a stock Ubuntu 22.04 image plus the runner
  agent is sufficient.

## Cost

- Control VM: e2-small (24/7)
- Windows GPU test runners: n1-standard-8 + T4 (on-demand)
- Linux GPU test runners: n1-standard-8 + T4 (on-demand)
- Linux SM80Plus test runners: g2-standard-8 + L4 (on-demand, narrow capability job only)
- Windows build runners: n1-standard-8, no GPU (on-demand)
- Linux build runners: n1-standard-8, no GPU (on-demand, `--max-runners=16`)
- Linux analytics runners: e2-small, no GPU (on-demand, scheduled jobs only)

All on-demand pools scale to zero when idle, so the build and analytics pools
cost effectively nothing when CI is quiet. `--max-runners=16` for the build pool
was sized from the observed peak of 12 concurrent x86_64 build jobs (see the
rollout checklist below); at full burst it uses ~128 vCPU against a 1500 vCPU
regional quota.

See [GCP pricing](https://cloud.google.com/compute/vm-instance-pricing) for current rates.

## Rollout checklist — Linux build + analytics pools

These pools are inert until their instance templates exist and the corresponding
workflow jobs are pointed at their labels. Recommended staged rollout:

1. **Create instance templates** (outside this repo, via `gcloud`/packer):
   - `linux-build-runner` — `n1-standard-8`, no GPU, ~200 GB disk, snapshot of a
     Linux runner image with Docker + the build toolchain.
   - `linux-analytics-runner` — `e2-small`, no GPU, small disk, Python 3 + `gh`.
2. **Start the scalers** (they sit at `min-runners=0`, zero cost until used):
   ```bash
   sudo systemctl enable --now scaler-linux-build
   sudo systemctl enable --now scaler-linux-analytics
   ```
3. **Validate** a manual `workflow_dispatch` job on each label lands on a GCP VM
   and completes (build green, sccache/GCS cache working, analytics auth via
   Workload Identity working).
4. **Flip the workflow jobs** in a follow-up PR (these are _not_ part of the
   infra PR that adds these files):
   - `.github/workflows/ci.yml`: the four `runs-on: '["ubuntu-22.04"]'` jobs
     (`build-linux-debug-gcc-x86_64`, `build-linux-release-gcc-x86_64`,
     `build-linux-release-gcc-wasm`, `sanitizer-linux-clang-x86_64`) →
     `'["Linux", "self-hosted", "build", "GCP"]'`.
   - `.github/workflows/ci-health.yml` and `.github/workflows/ci-analytics.yml`:
     `runs-on: ubuntu-latest` →
     `runs-on: ["Linux", "self-hosted", "analytics", "GCP"]`.
   - The `analytics` and `GCP` labels must be registered in
     `.github/actionlint.yaml` (this PR adds both; `build` was already there).
   - Confirm fork-PR approval gating covers the new build pool (build jobs run PR
     code), and that the analytics Workload Identity provider condition does not
     assert a hosted-runner claim.
   - aarch64 builds stay on `ubuntu-24.04-arm` (the GCP pool is x86_64).
5. **Update `slang-ci-docs`** runner/scaler inventory with the two new pools
   (labels, machine types, `--max-runners`, cost) once deployed.
