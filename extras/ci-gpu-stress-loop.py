#!/usr/bin/env python3
"""
ci-gpu-stress-loop-v2.py — Enhanced ephemeral GCP VM stress test for GPU intermittency.

Creates ephemeral VMs matching CI scaler config (n1-standard-8 + T4) and runs
the full test suite inside the CI container to reproduce intermittent GPU
failures ("Failed to initialize NVML: Unknown Error").

Usage:
    python3 extras/ci-gpu-stress-loop-v2.py <artifact-tarball> [options]

    artifact-tarball: path to slang-debug.tar.gz with CI artifacts
                      (or a directory — it will be auto-tarballed)

Options:
    --iterations N    Number of VMs to create and test (default: 40)
    --config CONFIG   debug or release (default: debug)
    --parallel N      Run N VMs concurrently (default: 1, max: 4)
    --gfx-only        Run only gfx-unit-tests (faster, targets the failing tests)

Requires: gcloud CLI authenticated to slang-runners project, gh CLI for GHCR token
"""

import argparse
import csv
import os
import re
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime
from pathlib import Path

PROJECT = "slang-runners"
ZONES = ["us-east1-c", "us-east1-d", "us-central1-a", "us-west1-a"]
MACHINE_TYPE = "n1-standard-8"
IMAGE_FAMILY = "linux-gpu-runner"
VM_PREFIX = "gpu-stress-v2"
CONTAINER_IMAGE = "ghcr.io/shader-slang/slang-linux-gpu-ci:v1.5.1"
def run_cmd(cmd, *, timeout=600, check=False, capture=True):
    """Run a command, return (returncode, stdout, stderr)."""
    try:
        r = subprocess.run(
            cmd,
            capture_output=capture,
            text=True,
            timeout=timeout,
        )
        return r.returncode, r.stdout, r.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "timeout"
    except Exception as e:
        return -1, "", str(e)


def gcloud_ssh(vm_name, zone, command, *, timeout=600):
    """Run a command on a VM via gcloud compute ssh."""
    cmd = [
        "gcloud", "compute", "ssh", vm_name,
        f"--zone={zone}", f"--project={PROJECT}",
        f"--command={command}",
    ]
    return run_cmd(cmd, timeout=timeout)


def gcloud_scp(local_path, vm_name, remote_path, zone):
    """SCP a file to a VM."""
    cmd = [
        "gcloud", "compute", "scp", local_path,
        f"{vm_name}:{remote_path}",
        f"--zone={zone}", f"--project={PROJECT}",
    ]
    return run_cmd(cmd, timeout=300)


def create_vm(vm_name, zone):
    """Create an ephemeral GCP VM with T4 GPU."""
    cmd = [
        "gcloud", "compute", "instances", "create", vm_name,
        f"--project={PROJECT}", f"--zone={zone}",
        f"--machine-type={MACHINE_TYPE}",
        "--accelerator=type=nvidia-tesla-t4,count=1",
        "--maintenance-policy=TERMINATE",
        f"--image-family={IMAGE_FAMILY}", f"--image-project={PROJECT}",
        "--boot-disk-size=512", "--boot-disk-type=pd-ssd",
        "--quiet",
    ]
    return run_cmd(cmd, timeout=180)


def delete_vm(vm_name, zone):
    """Delete a VM."""
    cmd = [
        "gcloud", "compute", "instances", "delete", vm_name,
        f"--zone={zone}", f"--project={PROJECT}", "--quiet",
    ]
    run_cmd(cmd, timeout=60)


def wait_for_ssh(vm_name, zone, max_attempts=18):
    """Wait for SSH to become available on the VM."""
    for _ in range(max_attempts):
        rc, out, _ = gcloud_ssh(vm_name, zone, "echo ready", timeout=15)
        if rc == 0 and "ready" in out:
            return True
        time.sleep(10)
    return False


def get_ghcr_token():
    """Get GHCR token from environment or gh CLI."""
    token = os.environ.get("GHCR_TOKEN", "")
    if not token:
        rc, out, _ = run_cmd(["gh", "auth", "token"], timeout=10)
        if rc == 0:
            token = out.strip()
    return token


def generate_test_script(cmake_config, gfx_only, config):
    """Generate the shell script to run on the VM inside the container."""
    if gfx_only:
        test_cmd = (
            f"timeout 600 /workspace/{cmake_config}/bin/slang-test"
            " gfx-unit-test-tool/ -show-adapter-info"
        )
    else:
        test_cmd = (
            f"timeout 2400 /workspace/{cmake_config}/bin/slang-test"
            " -category full"
            " -expected-failure-list tests/expected-failure-github.txt"
            " -expected-failure-list tests/expected-failure-linux.txt"
            " -expected-failure-list tests/expected-failure-linux-gpu.txt"
            " -skip-reference-image-generation"
            " -show-adapter-info"
            " -use-test-server"
            " -server-count 4"
        )
        if config == "debug":
            test_cmd += " -skip-list tests/skip-list-debug.txt"

    # This script runs inside the container
    container_script = f"""#!/bin/bash
set -e
export LD_LIBRARY_PATH=/workspace/{cmake_config}/lib:$LD_LIBRARY_PATH

# Use libEGL_nvidia as Vulkan ICD to avoid TOCTOU race in EGL init (driver 580.x)
mkdir -p /tmp/vulkan-icd
cat > /tmp/vulkan-icd/nvidia_icd.json <<'ICDEOF'
{{
    "file_format_version": "1.0.1",
    "ICD": {{
        "library_path": "libEGL_nvidia.so.0",
        "api_version": "1.4.312"
    }}
}}
ICDEOF
export VK_ICD_FILENAMES=/tmp/vulkan-icd/nvidia_icd.json
rm -f /usr/share/glvnd/egl_vendor.d/10_nvidia.json 2>/dev/null || true

{test_cmd}
"""

    # This script runs on the VM host
    host_script = f"""#!/bin/bash
set -e
cd ~

# Initialize GPU devices for Docker passthrough
sudo nvidia-smi > /dev/null 2>&1
sudo nvidia-modprobe -m 2>/dev/null || true

# Pull container image
if ! docker image inspect {CONTAINER_IMAGE} > /dev/null 2>&1; then
    echo 'Pulling container image...'
    docker pull {CONTAINER_IMAGE} 2>&1
fi

# Prepare workspace
mkdir -p workspace/slang && cd workspace/slang
tar xzf ~/repo.tar.gz --strip-components=1 2>/dev/null || true
tar xzf ~/artifacts.tar.gz --strip-components=1 2>/dev/null || true
chmod +x {cmake_config}/bin/* 2>/dev/null || true
cd ~

# Write container script
cat > ~/run-tests.sh << 'CONTAINER_SCRIPT_EOF'
{container_script}
CONTAINER_SCRIPT_EOF
chmod +x ~/run-tests.sh

echo '=== Starting test run in container ==='
set +e
docker run --rm \
    --gpus all \
    --user root \
    --cap-add SYSLOG \
    --device /dev/nvidia-modeset:/dev/nvidia-modeset \
    --device /dev/dri:/dev/dri \
    -e NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics \
    -e SLANG_RUN_SPIRV_VALIDATION=1 \
    -e SLANG_USE_SPV_SOURCE_LANGUAGE_UNKNOWN=1 \
    -v /etc/vulkan/icd.d/nvidia_icd.json:/etc/vulkan/icd.d/nvidia_icd.json:ro \
    -v /usr/share/nvidia:/usr/share/nvidia:ro \
    -v /usr/share/glvnd/egl_vendor.d/10_nvidia.json:/usr/share/glvnd/egl_vendor.d/10_nvidia.json:ro \
    -v $HOME/workspace/slang:/workspace \
    -v $HOME/run-tests.sh:/run-tests.sh:ro \
    -w /workspace \
    {CONTAINER_IMAGE} \
    bash /run-tests.sh 2>&1
SLANG_EXIT=$?
set -e

echo ""
echo "=== EXIT_CODE: $SLANG_EXIT ==="
echo ""

# Post-test diagnostics (from host)
echo '=== Post-test nvidia-smi ==='
nvidia-smi 2>&1 || echo 'NVIDIA-SMI FAILED'
echo ''
echo '=== Post-test nvidia-smi full query ==='
nvidia-smi -q 2>&1 || echo 'NVIDIA-SMI QUERY FAILED'
echo ''
echo '=== Post-test XID errors ==='
nvidia-smi -q -d XID_ERRORS 2>&1 || echo 'XID QUERY FAILED'
echo ''
echo '=== PCI device info ==='
lspci 2>/dev/null | grep -i nvidia || echo 'lspci not available'
echo ''
echo '=== Full dmesg ==='
sudo dmesg 2>/dev/null || echo '(dmesg not available)'
"""
    return host_script


def parse_results(log_text):
    """Parse test output log to extract structured results."""
    result = {
        "exit_code": "unknown",
        "gpu_healthy_after": "unknown",
        "vk_pass_count": 0,
        "vk_fail_count": 0,
        "last_pass_vk_time": "",
        "first_fail_vk_time": "",
        "xid_codes": "",
        "dmesg_faults": "",
    }

    # Determine exit code
    if "Stopped scheduling new tests after too many consecutive failures" in log_text:
        result["exit_code"] = "aborted"
    elif "NVIDIA-SMI FAILED" in log_text:
        result["exit_code"] = "gpu_crash"
    elif "Failed to create Vulkan instance" in log_text:
        result["exit_code"] = "vulkan_fail"
    elif "EXIT_CODE: 0" in log_text:
        result["exit_code"] = "pass"
    elif re.search(r"EXIT_CODE: \d+", log_text):
        result["exit_code"] = "test_fail"

    # GPU healthy?
    if "NVIDIA-SMI FAILED" in log_text:
        result["gpu_healthy_after"] = "false"
    elif "Post-test nvidia-smi" in log_text:
        result["gpu_healthy_after"] = "true"

    # Count Vulkan pass/fail
    vk_pass_lines = re.findall(r"passed test:.*\(vk\)|passed test:.*Vulkan", log_text)
    vk_fail_lines = re.findall(r"FAILED test:.*\(vk\)|FAILED test:.*Vulkan", log_text)
    result["vk_pass_count"] = len(vk_pass_lines)
    result["vk_fail_count"] = len(vk_fail_lines)

    # Also count gfx-unit-test Vulkan results
    gfx_vk_pass = re.findall(r"passed test:.*Vulkan\.internal", log_text)
    gfx_vk_fail = re.findall(r"FAILED test:.*Vulkan\.internal", log_text)
    result["vk_pass_count"] += len(gfx_vk_pass)
    result["vk_fail_count"] += len(gfx_vk_fail)

    # Timestamps — look for lines with timestamps at start
    pass_vk_times = re.findall(
        r"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}).*passed test:.*(?:\(vk\)|Vulkan)",
        log_text,
    )
    if pass_vk_times:
        result["last_pass_vk_time"] = pass_vk_times[-1]

    fail_vk_times = re.findall(
        r"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}).*FAILED test:.*(?:\(vk\)|Vulkan)",
        log_text,
    )
    if fail_vk_times:
        result["first_fail_vk_time"] = fail_vk_times[0]

    # XID errors
    xids = re.findall(r"Xid.*?: (\d+)", log_text)
    result["xid_codes"] = ";".join(sorted(set(xids)))

    # Fault indicators in dmesg
    faults = re.findall(
        r".*(?:segfault|nvrm.*error|nvidia.*error).*",
        log_text,
        re.IGNORECASE,
    )
    result["dmesg_faults"] = ";".join(faults[:5])[:200]

    return result


def run_iteration(i, total, artifact_tarball, repo_tarball, cmake_config, config,
                  gfx_only, ghcr_token, results_dir):
    """Run a single stress test iteration on an ephemeral VM."""
    zone = ZONES[(i - 1) % len(ZONES)]
    vm_name = f"{VM_PREFIX}-{i}-{int(time.time())}"
    iter_dir = results_dir / f"iter_{i:03d}"
    iter_dir.mkdir(parents=True, exist_ok=True)

    tag = f"[{vm_name}]"
    print(f"\n=== Iteration {i} / {total} — VM: {vm_name} ({zone}) ===")

    row = {
        "iteration": i, "exit_code": "unknown", "duration_s": 0,
        "vm_name": vm_name, "zone": zone, "gpu_healthy_after": "unknown",
        "gpu_serial": "", "pci_device": "", "driver_version": "",
        "last_pass_vk_time": "", "first_fail_vk_time": "",
        "vk_pass_count": 0, "vk_fail_count": 0,
        "xid_codes": "", "dmesg_faults": "",
    }

    # Create VM
    print(f"  {tag} Creating VM in {zone}...")
    rc, out, err = create_vm(vm_name, zone)
    if rc != 0:
        print(f"  {tag} FAILED to create VM: {err}")
        (iter_dir / "vm_create.log").write_text(f"{out}\n{err}")
        row["exit_code"] = "create_failed"
        return row

    try:
        # Wait for SSH
        print(f"  {tag} Waiting for SSH...")
        if not wait_for_ssh(vm_name, zone):
            print(f"  {tag} SSH not ready after 3 minutes. Skipping.")
            row["exit_code"] = "ssh_failed"
            return row

        # Collect pre-test GPU info
        print(f"  {tag} Collecting GPU info...")
        rc, gpu_info, _ = gcloud_ssh(
            vm_name, zone,
            "nvidia-smi --query-gpu=name,driver_version,serial,gpu_bus_id,memory.total"
            " --format=csv,noheader 2>&1;"
            " echo '---FULL---';"
            " nvidia-smi -q 2>&1;"
            " echo '---PCI---';"
            " lspci 2>/dev/null | grep -i nvidia || echo 'lspci not available'",
        )
        (iter_dir / "gpu_pre.txt").write_text(gpu_info)

        # Parse GPU info from first line
        if gpu_info:
            parts = gpu_info.split("\n")[0].split(",")
            if len(parts) >= 5:
                row["driver_version"] = parts[1].strip()
                row["gpu_serial"] = parts[2].strip()
                row["pci_device"] = parts[3].strip()
        print(f"  {tag} GPU: serial={row['gpu_serial']} pci={row['pci_device']} driver={row['driver_version']}")

        # Transfer artifacts and repo
        print(f"  {tag} Transferring artifacts...")
        rc1, _, err1 = gcloud_scp(str(artifact_tarball), vm_name, "~/artifacts.tar.gz", zone)
        if rc1 != 0:
            print(f"  {tag} Artifact transfer failed: {err1}")
            row["exit_code"] = "transfer_failed"
            return row

        rc2, _, err2 = gcloud_scp(str(repo_tarball), vm_name, "~/repo.tar.gz", zone)
        if rc2 != 0:
            print(f"  {tag} Repo transfer failed: {err2}")
            row["exit_code"] = "transfer_failed"
            return row

        # Docker auth
        if ghcr_token:
            gcloud_ssh(
                vm_name, zone,
                f"echo '{ghcr_token}' | docker login ghcr.io -u token --password-stdin",
                timeout=30,
            )

        # Generate and transfer the test script
        host_script = generate_test_script(cmake_config, gfx_only, config)
        script_path = iter_dir / "host_script.sh"
        script_path.write_text(host_script)

        rc, _, err = gcloud_scp(str(script_path), vm_name, "~/host_script.sh", zone)
        if rc != 0:
            print(f"  {tag} Script transfer failed: {err}")
            row["exit_code"] = "transfer_failed"
            return row

        # Run the test
        print(f"  {tag} Running tests ({config}, gfx_only={gfx_only})...")
        start_time = time.time()

        rc, test_output, test_err = gcloud_ssh(
            vm_name, zone,
            "chmod +x ~/host_script.sh && ~/host_script.sh",
            timeout=2700,  # 45 min (test suite can take ~40 min + overhead)
        )

        duration = int(time.time() - start_time)
        row["duration_s"] = duration

        # Save full log
        log_text = test_output + "\n" + test_err
        (iter_dir / "test_output.log").write_text(log_text)

        # Parse results
        parsed = parse_results(log_text)
        row.update(parsed)

        status_symbol = "PASS" if row["exit_code"] == "pass" else f"*** {row['exit_code'].upper()} ***"
        print(
            f"  {tag} {status_symbol} (duration: {duration}s, "
            f"gpu_healthy: {row['gpu_healthy_after']}, "
            f"vk_pass: {row['vk_pass_count']}, vk_fail: {row['vk_fail_count']})"
        )

    finally:
        delete_vm(vm_name, zone)

    return row


def create_repo_tarball(results_dir):
    """Create a tarball of tests and expected failure lists from the repo."""
    repo_tarball = results_dir / "repo.tar.gz"
    print("Creating repo tarball (tests + expected failures)...")
    with tempfile.TemporaryDirectory() as tmpdir:
        clone_dir = Path(tmpdir) / "slang"
        rc, _, err = run_cmd(
            ["git", "clone", "--depth", "1",
             "https://github.com/shader-slang/slang.git", str(clone_dir)],
            timeout=120,
        )
        if rc != 0:
            print(f"Failed to clone repo: {err}")
            sys.exit(1)
        run_cmd(
            ["tar", "czf", str(repo_tarball), "-C", tmpdir,
             "slang/tests", "slang/tools/slangc-test"],
            timeout=60,
        )
    size_mb = repo_tarball.stat().st_size / (1024 * 1024)
    print(f"Repo tarball: {size_mb:.1f}MB")
    return repo_tarball


def ensure_tarball(artifact_path):
    """Ensure artifact_path is a tarball. If it's a directory, tar it."""
    p = Path(artifact_path)
    if p.is_dir():
        tarball = p.parent / f"{p.name}.tar.gz"
        print(f"Artifact is a directory, creating tarball: {tarball}")
        run_cmd(["tar", "czf", str(tarball), "-C", str(p), "."], timeout=120)
        return tarball
    return p


def print_summary(results_dir, results):
    """Print summary statistics."""
    print("\n========================================")
    print("  GPU Stress Test Loop v2 Complete")
    print("========================================\n")

    # Print table
    print("=== Results ===")
    header = ("iter", "exit_code", "dur_s", "zone", "gpu_ok", "serial", "vk_pass", "vk_fail")
    print(f"{'iter':>4}  {'exit_code':<14}  {'dur_s':>5}  {'zone':<16}  {'gpu_ok':<6}  {'serial':<16}  {'vk_p':>4}  {'vk_f':>4}")
    print("-" * 80)
    for r in results:
        print(
            f"{r['iteration']:4d}  {r['exit_code']:<14}  {r['duration_s']:5d}  "
            f"{r['zone']:<16}  {r['gpu_healthy_after']:<6}  {r['gpu_serial']:<16}  "
            f"{r['vk_pass_count']:4d}  {r['vk_fail_count']:4d}"
        )
    print()

    # Summary
    total = len(results)
    by_code = {}
    for r in results:
        code = r["exit_code"]
        by_code[code] = by_code.get(code, 0) + 1

    print("=== Summary ===")
    print(f"Total: {total}")
    for code in sorted(by_code.keys()):
        print(f"  {code}: {by_code[code]}")
    print()

    # Per-zone
    print("=== Per-Zone Breakdown ===")
    for zone in ZONES:
        zone_results = [r for r in results if r["zone"] == zone]
        zone_fail = sum(
            1 for r in zone_results
            if r["exit_code"] in ("gpu_crash", "vulkan_fail", "aborted")
        )
        if zone_results:
            print(f"  {zone}: {zone_fail} failures / {len(zone_results)} runs")
    print()

    print(f"Results directory: {results_dir}")
    print("========================================")


def main():
    parser = argparse.ArgumentParser(description="GPU stress test loop v2")
    parser.add_argument("artifact_tarball", help="Path to artifact tarball or directory")
    parser.add_argument("--iterations", type=int, default=40)
    parser.add_argument("--config", choices=["debug", "release"], default="debug")
    parser.add_argument("--parallel", type=int, default=1)
    parser.add_argument("--gfx-only", action="store_true")
    args = parser.parse_args()

    cmake_config = "Release" if args.config == "release" else "Debug"

    # Prepare artifact tarball
    artifact_tarball = ensure_tarball(args.artifact_tarball)
    if not artifact_tarball.exists():
        print(f"Artifact not found: {artifact_tarball}")
        sys.exit(1)

    # Get GHCR token
    ghcr_token = get_ghcr_token()
    if not ghcr_token:
        print("WARNING: No GHCR token — container pull may fail")

    # Setup results directory
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    results_dir = Path(f"./gpu-stress-results/{timestamp}")
    results_dir.mkdir(parents=True, exist_ok=True)

    print("=== GPU Stress Test Loop v2 ===")
    print(f"Artifact: {artifact_tarball}")
    print(f"Config: {args.config} ({cmake_config})")
    print(f"Iterations: {args.iterations}")
    print(f"Parallel: {args.parallel}")
    print(f"GFX-only: {args.gfx_only}")
    print(f"Container: {CONTAINER_IMAGE}")
    print(f"Zones: {', '.join(ZONES)}")
    print(f"Results: {results_dir}")
    print()

    # Create repo tarball
    repo_tarball = create_repo_tarball(results_dir)

    # Run iterations
    results = []
    if args.parallel <= 1:
        for i in range(1, args.iterations + 1):
            row = run_iteration(
                i, args.iterations, artifact_tarball, repo_tarball,
                cmake_config, args.config, args.gfx_only, ghcr_token, results_dir,
            )
            results.append(row)
    else:
        with ThreadPoolExecutor(max_workers=min(args.parallel, 4)) as pool:
            futures = {}
            for i in range(1, args.iterations + 1):
                f = pool.submit(
                    run_iteration,
                    i, args.iterations, artifact_tarball, repo_tarball,
                    cmake_config, args.config, args.gfx_only, ghcr_token, results_dir,
                )
                futures[f] = i
            for f in as_completed(futures):
                row = f.result()
                results.append(row)
        results.sort(key=lambda r: r["iteration"])

    # Write CSV
    csv_path = results_dir / "results.csv"
    fieldnames = [
        "iteration", "exit_code", "duration_s", "vm_name", "zone",
        "gpu_healthy_after", "gpu_serial", "pci_device", "driver_version",
        "last_pass_vk_time", "first_fail_vk_time",
        "vk_pass_count", "vk_fail_count", "xid_codes", "dmesg_faults",
    ]
    with open(csv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(results)

    print_summary(results_dir, results)


if __name__ == "__main__":
    main()
