# GCP Runner Scaler

Auto-scales Linux and Windows GPU VMs on GCP based on GitHub Actions job
queue depth. Uses the [GitHub Actions Scale Set Client](https://github.com/actions/scaleset)
(no Kubernetes required).

## Architecture

```text
┌──────────────────────────────────────────┐
│ e2-small VM (always-on)                  │
│                                          │
│  scaler (Windows)                        │
│   --labels=Windows,self-hosted,GCP-T4    │
│   --platform=windows                     │
│       │                                  │
│  scaler (Linux)                          │
│   --labels=Linux,self-hosted,GPU         │
│   --platform=linux                       │
│       │                                  │
│  scaler (Linux SM80Plus)                 │
│   --labels=Linux,self-hosted,SM80Plus    │
│   --platform=linux                       │
└────┬──────────────┬──────────────┬───────┘
     │              │              │
     ▼              ▼              ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│ Windows T4   │ │ Linux T4     │ │ Linux L4     │
│ VMs          │ │ VMs          │ │ VMs          │
│ - ephemeral  │ │ - ephemeral  │ │ - ephemeral  │
│ - scale zero │ │ - scale zero │ │ - scale zero │
└──────────────┘ └──────────────┘ └──────────────┘
```

Three instances of the same binary run on one control VM, each targeting a
different platform with different instance templates and labels. Zones are
selected dynamically based on GPU quota availability across US regions.

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
  --labels=Linux,self-hosted,GPU \
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
| `deploy/scaler-windows.service` | systemd unit for Windows scaler |
| `deploy/scaler-linux.service` | systemd unit for Linux scaler |
| `deploy/scaler-linux-sm80plus.service` | systemd unit for Linux SM80Plus scaler |
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

## Cost

- Control VM: e2-small (24/7)
- Windows GPU test runners: n1-standard-8 + T4 (on-demand)
- Linux GPU test runners: n1-standard-8 + T4 (on-demand)
- Linux SM80Plus test runners: g2-standard-8 + L4 (on-demand, narrow capability job only)
- Windows build runners: n1-standard-8, no GPU (on-demand)

See [GCP pricing](https://cloud.google.com/compute/vm-instance-pricing) for current rates.
