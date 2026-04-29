#!/usr/bin/env python3
"""
CI Health Page Generator

Queries live CI runner status and queue depth via ci-queue-status.py,
then generates health.html for the analytics dashboard.

Designed to run on a 15-minute schedule, separately from the nightly
full analytics generation.

Usage:
    python3 ci_health.py --output ci_analytics
    python3 ci_health.py --repo OWNER/REPO --output ./output
"""

import argparse
import html as html_mod
import json
import os
import subprocess
import sys
from datetime import datetime, timezone, timedelta

# Import the page template from ci_visualization
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ci_visualization import page_template, chart_section, DOWNLOAD_JS


DEFAULT_REPO = "shader-slang/slang"

# GCP GPU quota configuration
GPU_QUOTA_PROJECT = "slang-runners"
GPU_QUOTA_METRIC = "NVIDIA_T4_GPUS"
GPU_QUOTA_REGIONS = ["us-central1", "us-east1", "us-west1"]
DEFAULT_GPU_QUOTA_METRICS = [
    {
        "metric": GPU_QUOTA_METRIC,
        "name": "T4",
        "regions": GPU_QUOTA_REGIONS,
    },
    {
        "metric": "NVIDIA_L4_GPUS",
        "name": "L4",
        "regions": ["us-central1", "us-east1", "us-west1", "us-west4", "europe-west1"],
    },
]
GCP_VM_GROUPS = [
    "Linux GPU (GCP)",
    "Linux SM80Plus GPU (GCP)",
    "Windows Build (GCP)",
    "Windows GPU (GCP)",
]
GCP_VM_PALETTE = {
    "Linux GPU (GCP)": "#0d6efd",
    "Linux SM80Plus GPU (GCP)": "#6f42c1",
    "Windows Build (GCP)": "#fd7e14",
    "Windows GPU (GCP)": "#28a745",
}


def _esc(s):
    """HTML-escape a string."""
    return html_mod.escape(str(s), quote=True)


def _link(url, text):
    """Build an escaped HTML link, or just escaped text if no URL."""
    if url:
        return f'<a href="{_esc(url)}" target="_blank" rel="noopener noreferrer">{_esc(text)}</a>'
    return _esc(text)
DISPLAY_INTERVAL_MIN = 15


def _round_time(hhmm, interval=DISPLAY_INTERVAL_MIN):
    """Round HH:MM string down to the nearest interval."""
    h, m = int(hhmm[:2]), int(hhmm[3:5])
    m = (m // interval) * interval
    return f"{h:02d}:{m:02d}"
SNAPSHOTS_FILE = "health_snapshots.jsonl"
CHARTJS_CDN = "https://cdn.jsdelivr.net/npm/chart.js"


def _copy_gpu_quota_metrics(metrics):
    return [
        {
            "metric": m["metric"],
            "name": m.get("name", m["metric"]),
            "regions": list(m.get("regions", [])),
        }
        for m in metrics
    ]


def _default_gpu_quota_metrics(reason=None):
    if reason:
        print(
            f"Warning: invalid gpu_quota_metrics in runner_config.json: {reason}; using defaults",
            file=sys.stderr,
        )
    return _copy_gpu_quota_metrics(DEFAULT_GPU_QUOTA_METRICS)


def load_gpu_quota_metrics():
    """Load GPU quota metrics from runner_config.json."""
    config_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "runner_config.json")
    try:
        with open(config_path, "r", encoding="utf-8") as f:
            config = json.load(f)
    except (OSError, json.JSONDecodeError):
        return _default_gpu_quota_metrics()

    if not isinstance(config, dict):
        return _default_gpu_quota_metrics("root must be an object")

    if "gpu_quota_metrics" not in config:
        return _default_gpu_quota_metrics()

    configured_metrics = config["gpu_quota_metrics"]
    if not isinstance(configured_metrics, list):
        return _default_gpu_quota_metrics("gpu_quota_metrics must be a list")

    metrics = []
    for index, entry in enumerate(configured_metrics):
        if not isinstance(entry, dict):
            return _default_gpu_quota_metrics(f"entry {index} must be an object")
        metric = entry.get("metric")
        regions = entry.get("regions", [])
        name = entry.get("name", metric)
        if not isinstance(metric, str) or not metric:
            return _default_gpu_quota_metrics(f"entry {index} metric must be a non-empty string")
        if not isinstance(name, str) or not name:
            return _default_gpu_quota_metrics(f"entry {index} name must be a non-empty string")
        if (
            not isinstance(regions, list)
            or not regions
            or any(not isinstance(region, str) or not region for region in regions)
        ):
            return _default_gpu_quota_metrics(f"entry {index} regions must be a non-empty string list")
        metrics.append({
            "metric": metric,
            "name": name,
            "regions": list(dict.fromkeys(regions)),
        })
    return metrics


def fetch_gpu_quota(metrics=None):
    """Fetch GPU usage and limits from GCP across configured regions.

    Returns a dict with a 'by_metric' breakdown plus legacy T4 'usage',
    'limit', and 'regions' fields, or None if gcloud is unavailable.
    """
    metric_configs = load_gpu_quota_metrics() if metrics is None else metrics
    if not metric_configs:
        return None

    by_metric = {
        config["metric"]: {
            "name": config.get("name", config["metric"]),
            "usage": 0,
            "limit": 0,
            "regions": {},
        }
        for config in metric_configs
    }
    metrics_by_region = {}
    for config in metric_configs:
        for region in config.get("regions", []):
            metrics_by_region.setdefault(region, set()).add(config["metric"])

    for region in metrics_by_region:
        cmd = [
            "gcloud", "compute", "regions", "describe", region,
            "--project", GPU_QUOTA_PROJECT,
            "--format", "json(quotas)",
        ]
        try:
            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=30
            )
        except FileNotFoundError:
            return None
        except subprocess.TimeoutExpired:
            print(f"Warning: timeout fetching GPU quota for {region}", file=sys.stderr)
            continue
        if result.returncode != 0:
            print(
                f"Warning: gcloud failed for {region}: {result.stderr.strip()}",
                file=sys.stderr,
            )
            continue
        try:
            data = json.loads(result.stdout)
        except json.JSONDecodeError:
            print(f"Warning: invalid GPU quota JSON for {region}", file=sys.stderr)
            continue

        wanted_metrics = metrics_by_region[region]
        for q in data.get("quotas", []):
            metric = q.get("metric")
            if metric not in wanted_metrics:
                continue
            usage = int(q.get("usage", 0))
            limit = int(q.get("limit", 0))
            bucket = by_metric[metric]
            bucket["regions"][region] = {"usage": usage, "limit": limit}
            bucket["usage"] += usage
            bucket["limit"] += limit

    by_metric = {
        metric: quota
        for metric, quota in by_metric.items()
        if quota["regions"]
    }
    if not by_metric:
        return None

    result = {"by_metric": by_metric}
    legacy = by_metric.get(GPU_QUOTA_METRIC)
    if legacy:
        result.update({
            "usage": legacy["usage"],
            "limit": legacy["limit"],
            "regions": legacy["regions"],
        })
    return result


def _sum_region_quota(regions, key):
    return sum(region.get(key, 0) for region in regions.values())


def _normalize_gpu_quota_by_metric(gpu_quota):
    """Normalize current and legacy quota payloads into a metric-keyed map."""
    if not gpu_quota:
        return {}

    by_metric = gpu_quota.get("by_metric")
    if isinstance(by_metric, dict) and by_metric:
        return by_metric

    legacy_regions = gpu_quota.get("regions", {})
    if not legacy_regions:
        return {}
    return {
        gpu_quota.get("metric", GPU_QUOTA_METRIC): {
            "name": gpu_quota.get("name", "T4"),
            "usage": gpu_quota.get("usage", _sum_region_quota(legacy_regions, "usage")),
            "limit": gpu_quota.get("limit", _sum_region_quota(legacy_regions, "limit")),
            "regions": legacy_regions,
        }
    }


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate CI health page from live runner and queue data."
    )
    parser.add_argument(
        "--repo",
        default=DEFAULT_REPO,
        help=f"Repository (default: {DEFAULT_REPO})",
    )
    parser.add_argument(
        "--output",
        default="ci_analytics",
        help="Output directory (default: ci_analytics)",
    )
    return parser.parse_args()


def fetch_queue_status(repo):
    """Run ci-queue-status.py --json and return parsed output."""
    script = os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        "ci-queue-status.py",
    )
    cmd = [sys.executable, script, "--repo", repo, "--json"]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    if result.returncode != 0:
        print(f"Warning: ci-queue-status.py failed: {result.stderr}", file=sys.stderr)
        return None
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as e:
        print(f"Warning: invalid JSON from ci-queue-status.py: {e}", file=sys.stderr)
        return None


def fetch_recent_failures(repo):
    """Fetch recent CI workflow failures (last 3 hours)."""
    # Use gh CLI directly for a quick query
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    from gh_api import gh_api_list

    runs, err = gh_api_list(
        f"repos/{repo}/actions/runs?status=completed&per_page=20",
        "workflow_runs",
    )
    if err:
        return []

    cutoff = (datetime.now(timezone.utc) - timedelta(hours=3)).strftime("%Y-%m-%dT%H:%M:%SZ")
    failures = []
    for run in (runs or []):
        if run.get("name") != "CI":
            continue
        if run.get("conclusion") != "failure":
            continue
        if run.get("event") == "merge_group":
            continue  # Merge queue failures shown separately
        event_time = run.get("updated_at") or run.get("created_at", "")
        if event_time < cutoff:
            continue
        failures.append({
            "branch": run.get("head_branch", ""),
            "url": run.get("html_url", ""),
            "created_at": event_time,
            "actor": (run.get("actor") or {}).get("login", ""),
        })
    return failures[:10]


def fetch_merge_queue_status(repo):
    """Fetch recent merge queue CI runs (last 24 hours).

    Returns a dict with 'recent' (list of runs), 'summary' counts.
    """
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    from gh_api import gh_api_list, parse_merge_queue_pr_number

    runs, err = gh_api_list(
        f"repos/{repo}/actions/runs?event=merge_group&per_page=50",
        "workflow_runs",
    )
    if err:
        return None

    cutoff = (datetime.now(timezone.utc) - timedelta(hours=24)).strftime("%Y-%m-%dT%H:%M:%SZ")
    recent = []
    counts = {"success": 0, "failure": 0, "cancelled": 0, "in_progress": 0}
    for run in (runs or []):
        if run.get("name") != "CI":
            continue
        event_time = run.get("created_at", "")
        if event_time < cutoff:
            continue
        conclusion = run.get("conclusion")
        if conclusion in counts:
            counts[conclusion] += 1
        elif run.get("status") in ("queued", "in_progress"):
            counts["in_progress"] += 1

        pr_number = parse_merge_queue_pr_number(run.get("head_branch", ""))
        recent.append({
            "conclusion": conclusion or run.get("status", "in_progress"),
            "branch": run.get("head_branch", ""),
            "pr_number": pr_number,
            "pr_url": f"https://github.com/{repo}/pull/{pr_number}" if pr_number else "",
            "url": run.get("html_url", ""),
            "created_at": event_time,
            "updated_at": run.get("updated_at", ""),
        })

    return {"recent": recent, "summary": counts}


def record_snapshot(queue_data, output_dir, gpu_quota=None, mq_data=None):
    """Append a runner status snapshot to the JSONL time-series file."""
    if not queue_data and not gpu_quota:
        return

    now = datetime.now(timezone.utc)
    snapshot = {"timestamp": now.strftime("%Y-%m-%dT%H:%M:%SZ")}

    if queue_data:
        # Aggregate runner counts by group
        summary = queue_data.get("summary", {})
        snapshot["jobs_queued"] = summary.get("jobs_queued", 0)
        snapshot["jobs_running"] = summary.get("jobs_running", 0)
        snapshot["runs_queued"] = summary.get("runs_queued", 0)
        snapshot["runs_in_progress"] = summary.get("runs_in_progress", 0)

        # Per-group busy/total from runner data
        groups = {}
        for r in queue_data.get("self_hosted_runners", []):
            g = r.get("group", "Other")
            if g not in groups:
                groups[g] = {"busy": 0, "total": 0}
            if r.get("status") == "online":
                groups[g]["total"] += 1
                if r.get("busy"):
                    groups[g]["busy"] += 1
        snapshot["runner_groups"] = groups

        # Per-group queue depth
        queue_groups = {}
        for g in queue_data.get("queue_by_group", []):
            queue_groups[g["name"]] = {
                "queued": g.get("queued", 0),
                "running": g.get("running", 0),
            }
        snapshot["queue_by_group"] = queue_groups

    # GPU quota per region
    if gpu_quota:
        by_metric = _normalize_gpu_quota_by_metric(gpu_quota)
        if by_metric:
            snapshot["gpu_quota_by_metric"] = by_metric
            legacy = by_metric.get(GPU_QUOTA_METRIC)
            if legacy:
                snapshot["gpu_quota"] = legacy.get("regions", {})

    # Merge queue summary
    if mq_data:
        snapshot["merge_queue"] = mq_data.get("summary", {})

    # Append to JSONL file (kept indefinitely, ~55KB/day)
    os.makedirs(output_dir, exist_ok=True)
    path = os.path.join(output_dir, SNAPSHOTS_FILE)
    with open(path, "a", encoding="utf-8") as f:
        f.write(json.dumps(snapshot) + "\n")


def load_snapshots(output_dir, hours=24):
    """Load snapshots from the last N hours (tail-read for large files)."""
    path = os.path.join(output_dir, SNAPSHOTS_FILE)
    if not os.path.exists(path):
        return []

    now = datetime.now(timezone.utc)
    from datetime import timedelta
    cutoff = now - timedelta(hours=hours)
    cutoff_str = cutoff.strftime("%Y-%m-%dT%H:%M:%SZ")

    def _iter_jsonl_tail(fp, block_size=64 * 1024):
        """Yield JSONL lines from end to start without loading full file."""
        fp.seek(0, os.SEEK_END)
        buf = ""
        pos = fp.tell()
        while pos > 0:
            read_size = block_size if pos >= block_size else pos
            pos -= read_size
            fp.seek(pos)
            buf = fp.read(read_size) + buf
            lines = buf.split("\n")
            buf = lines[0]
            for line in reversed(lines[1:]):
                if line:
                    yield line
        if buf:
            yield buf

    snapshots_rev = []
    with open(path, "r", encoding="utf-8") as f:
        for line in _iter_jsonl_tail(f):
            try:
                snap = json.loads(line)
            except json.JSONDecodeError:
                continue
            ts = snap.get("timestamp", "")
            if ts < cutoff_str:
                break
            snapshots_rev.append(snap)
    return list(reversed(snapshots_rev))


def _deduplicate_snapshots(snapshots):
    """Keep only the latest snapshot per rounded time window."""
    by_window = {}
    for s in snapshots:
        key = _round_time(s["timestamp"][11:16])
        # Include date to handle day boundaries
        date = s["timestamp"][:10]
        by_window[(date, key)] = s
    return [by_window[k] for k in sorted(by_window)]


_REGION_COLORS = ["#0d6efd", "#fd7e14", "#28a745", "#dc3545", "#6f42c1", "#20c997"]


def _region_palette(regions):
    """Map region names to colors."""
    return {r: _REGION_COLORS[i % len(_REGION_COLORS)] for i, r in enumerate(regions)}


def _gpu_quota_chart_id(metric):
    if metric == GPU_QUOTA_METRIC:
        return "gpuQuota"
    suffix = "".join(ch if ch.isalnum() else "_" for ch in metric)
    return f"gpuQuota_{suffix}"


def _snapshot_gpu_quota_by_metric(snapshot):
    by_metric = snapshot.get("gpu_quota_by_metric")
    if isinstance(by_metric, dict) and by_metric:
        return by_metric

    legacy_regions = snapshot.get("gpu_quota", {})
    if legacy_regions:
        return {
            GPU_QUOTA_METRIC: {
                "name": "T4",
                "usage": _sum_region_quota(legacy_regions, "usage"),
                "limit": _sum_region_quota(legacy_regions, "limit"),
                "regions": legacy_regions,
            }
        }
    return {}


def _quota_metric_display_order(snapshots):
    ordered = []
    seen = set()
    for config in load_gpu_quota_metrics():
        metric = config["metric"]
        if metric not in seen:
            ordered.append(metric)
            seen.add(metric)
    for snapshot in snapshots:
        for metric in _snapshot_gpu_quota_by_metric(snapshot):
            if metric not in seen:
                ordered.append(metric)
                seen.add(metric)
    return ordered


def _build_gpu_quota_charts(snapshots):
    charts = []
    for metric in _quota_metric_display_order(snapshots):
        metric_snapshots = [
            _snapshot_gpu_quota_by_metric(snapshot).get(metric, {})
            for snapshot in snapshots
        ]
        regions = sorted({
            region
            for quota in metric_snapshots
            for region in quota.get("regions", {})
        })
        if not regions:
            continue

        region_series = {
            region: [
                quota.get("regions", {}).get(region, {}).get("usage", 0)
                for quota in metric_snapshots
            ]
            for region in regions
        }
        quota_limit = 0
        name = metric
        for quota in reversed(metric_snapshots):
            if quota.get("regions"):
                name = quota.get("name", metric)
                quota_limit = sum(
                    region.get("limit", 0)
                    for region in quota.get("regions", {}).values()
                )
                break

        charts.append({
            "id": _gpu_quota_chart_id(metric),
            "metric": metric,
            "name": name,
            "regionData": region_series,
            "limit": quota_limit,
            "regionColors": _region_palette(regions),
        })
    return charts


def build_history_chart(snapshots):
    """Build Chart.js HTML for 24h runner load history."""
    if not snapshots:
        return "<p>No history data yet. Snapshots accumulate every 15 minutes.</p>"

    snapshots = _deduplicate_snapshots(snapshots)
    timestamps = [_round_time(s["timestamp"][11:16]) for s in snapshots]

    # Only show GCP VM groups, exclude scaler host and test runners
    gcp_vm_groups = GCP_VM_GROUPS
    palette = GCP_VM_PALETTE

    # Queue depth over time
    queued_data = [s.get("jobs_queued", 0) for s in snapshots]
    running_data = [s.get("jobs_running", 0) for s in snapshots]

    # Active CI workflow runs over time
    runs_in_progress = [s.get("runs_in_progress", 0) for s in snapshots]
    runs_queued = [s.get("runs_queued", 0) for s in snapshots]

    # Build per-group data series
    group_series = {}
    for g in gcp_vm_groups:
        group_series[g] = [s.get("runner_groups", {}).get(g, {}).get("total", 0) for s in snapshots]

    gpu_quota_charts = _build_gpu_quota_charts(snapshots)

    # Merge queue cumulative success/failure from snapshots
    # Each snapshot records the running 24h totals; we just plot them over time.
    mq_success_data = [s.get("merge_queue", {}).get("success", 0) for s in snapshots]
    mq_failure_data = [s.get("merge_queue", {}).get("failure", 0) for s in snapshots]
    has_mq_snapshots = any(s.get("merge_queue") for s in snapshots)

    charts_html = (
        chart_section("runnerHistory", "GCP Runner VMs",
            "Number of GCP-provisioned runner VMs online per group, sampled every 15 minutes.")
        + chart_section("workflowHistory", "Active CI Workflows",
            "CI workflow runs currently in progress or queued.")
        + chart_section("queueHistory", "Job Queue Depth",
            "Individual jobs waiting in queue vs. actively running on runners.")
    )
    for quota_chart in gpu_quota_charts:
        charts_html += chart_section(
            quota_chart["id"],
            f"{quota_chart['name']} GPU Usage vs. Quota",
            f"{quota_chart['name']} GPUs in use per GCP region, stacked. "
            "Dashed line shows total quota limit.",
        )
    if has_mq_snapshots:
        charts_html += chart_section("mqHistory", "Merge Queue Checks (24h rolling)",
            "Rolling 24-hour count of merge queue CI check outcomes, sampled every 15 minutes.")

    return f"""
<div class="chart-section">
  <label>Time window: </label>
  <select id="historyRange" onchange="updateHistoryRange()">
    <option value="4">Last 4 hours</option>
    <option value="12">Last 12 hours</option>
    <option value="24" selected>Last 24 hours</option>
  </select>
</div>
{charts_html}
<script src="{CHARTJS_CDN}"></script>
<script>
{DOWNLOAD_JS}
// All snapshot data
const allTimestamps = {json.dumps(timestamps)};
const allRunnerData = {json.dumps({g: group_series[g] for g in gcp_vm_groups})};
const allRunsInProgress = {json.dumps(runs_in_progress)};
const allRunsQueued = {json.dumps(runs_queued)};
const allJobsQueued = {json.dumps(queued_data)};
const allJobsRunning = {json.dumps(running_data)};

const gpuQuotaCharts = {json.dumps(gpu_quota_charts)};

const mqSuccessData = {json.dumps(mq_success_data)};
const mqFailureData = {json.dumps(mq_failure_data)};
const hasMqSnapshots = {json.dumps(has_mq_snapshots)};

const runnerColors = {json.dumps(palette)};
const pointsPerHour = 4; // 15-min intervals

let charts = {{}};

function sliceLast(arr, n) {{ return arr.slice(-n); }}

function thinLabels(labels, step) {{
  return labels.map((l, i) => i % step === 0 ? l : '');
}}

function buildCharts(hours) {{
  const n = hours * pointsPerHour;
  const tickStep = hours >= 24 ? 2 : 1; // every 30min for 24h, 15min for shorter
  const labels = sliceLast(allTimestamps, n);
  const displayLabels = thinLabels(labels, tickStep);

  // Destroy existing charts
  Object.values(charts).forEach(c => c.destroy());
  charts = {{}};

  // Runner VMs
  const runnerDatasets = [];
  for (const [g, color] of Object.entries(runnerColors)) {{
    runnerDatasets.push({{
      label: g,
      data: sliceLast(allRunnerData[g], n),
      borderColor: color,
      backgroundColor: color + '55',
      fill: true,
      tension: 0.3,
    }});
  }}
  charts.runner = new Chart(document.getElementById('runnerHistory_canvas').getContext('2d'), {{
    type: 'line',
    data: {{ labels: displayLabels, datasets: runnerDatasets }},
    options: {{
      responsive: true,
      scales: {{ y: {{ min: 0, stacked: true, title: {{ display: true, text: 'GCP VMs Online' }} }} }}
    }}
  }});

  // Workflow runs
  charts.workflow = new Chart(document.getElementById('workflowHistory_canvas').getContext('2d'), {{
    type: 'line',
    data: {{
      labels: displayLabels,
      datasets: [
        {{ label: 'Runs In Progress', data: sliceLast(allRunsInProgress, n), borderColor: '#0d6efd', fill: true, backgroundColor: 'rgba(13,110,253,0.1)', tension: 0.3 }},
        {{ label: 'Runs Queued', data: sliceLast(allRunsQueued, n), borderColor: '#ffc107', fill: true, backgroundColor: 'rgba(255,193,7,0.1)', tension: 0.3 }}
      ]
    }},
    options: {{
      responsive: true,
      scales: {{ y: {{ min: 0, title: {{ display: true, text: 'Workflow Runs' }} }} }}
    }}
  }});

  // Job queue
  charts.queue = new Chart(document.getElementById('queueHistory_canvas').getContext('2d'), {{
    type: 'line',
    data: {{
      labels: displayLabels,
      datasets: [
        {{ label: 'Jobs Queued', data: sliceLast(allJobsQueued, n), borderColor: '#dc3545', fill: true, backgroundColor: 'rgba(220,53,69,0.1)', tension: 0.3 }},
        {{ label: 'Jobs Running', data: sliceLast(allJobsRunning, n), borderColor: '#0d6efd', fill: false, tension: 0.3 }}
      ]
    }},
    options: {{
      responsive: true,
      scales: {{ y: {{ min: 0, title: {{ display: true, text: 'Jobs' }} }} }}
    }}
  }});

  // GPU quota (stacked by region + limit line)
  for (const quotaChart of gpuQuotaCharts) {{
    const gpuCanvas = document.getElementById(quotaChart.id + '_canvas');
    if (!gpuCanvas || Object.keys(quotaChart.regionData).length === 0) continue;

    const gpuDatasets = [];
    for (const [region, color] of Object.entries(quotaChart.regionColors)) {{
      gpuDatasets.push({{
        label: region,
        data: sliceLast(quotaChart.regionData[region] || [], n),
        borderColor: color,
        backgroundColor: color + '55',
        fill: 'stack',
        stack: 'usage',
        tension: 0.3,
      }});
    }}
    // Quota limit as a dashed line (not stacked)
    gpuDatasets.push({{
      label: 'Quota Limit',
      data: Array(labels.length).fill(quotaChart.limit),
      borderColor: '#dc3545',
      borderDash: [6, 3],
      borderWidth: 2,
      pointRadius: 0,
      fill: false,
    }});
    charts[quotaChart.id] = new Chart(gpuCanvas.getContext('2d'), {{
      type: 'line',
      data: {{ labels: displayLabels, datasets: gpuDatasets }},
      options: {{
        responsive: true,
        scales: {{
          y: {{ min: 0, stacked: true, title: {{ display: true, text: quotaChart.name + ' GPUs' }} }}
        }},
        plugins: {{
          tooltip: {{
            callbacks: {{
              afterBody: function(items) {{
                const idx = items[0].dataIndex;
                let total = 0;
                for (const region of Object.keys(quotaChart.regionColors)) {{
                  const ds = items[0].chart.data.datasets.find(d => d.label === region);
                  if (ds) total += ds.data[idx] || 0;
                }}
                return 'Total: ' + total + ' / ' + quotaChart.limit;
              }}
            }}
          }}
        }}
      }}
    }});
  }}

  // Merge queue checks over time
  const mqCanvas = document.getElementById('mqHistory_canvas');
  if (mqCanvas && hasMqSnapshots) {{
    charts.mq = new Chart(mqCanvas.getContext('2d'), {{
      type: 'line',
      data: {{
        labels: displayLabels,
        datasets: [
          {{ label: 'Success (24h)', data: sliceLast(mqSuccessData, n), borderColor: '#28a745', fill: true, backgroundColor: 'rgba(40,167,69,0.1)', tension: 0.3 }},
          {{ label: 'Failure (24h)', data: sliceLast(mqFailureData, n), borderColor: '#dc3545', fill: true, backgroundColor: 'rgba(220,53,69,0.1)', tension: 0.3 }},
        ]
      }},
      options: {{
        responsive: true,
        scales: {{ y: {{ min: 0, title: {{ display: true, text: 'Merge Queue Checks' }} }} }}
      }}
    }});
  }}
}}

function updateHistoryRange() {{
  const hours = parseInt(document.getElementById('historyRange').value);
  buildCharts(hours);
}}

// Initial render
buildCharts(24);
</script>
"""


def generate_health_html(queue_data, failures, output_dir, mq_data=None):
    """Generate health.html from live data."""
    now = datetime.now(timezone.utc)
    fetched_at = now.strftime("%Y-%m-%d %H:%M UTC")

    # Runner status section — only online GCP VM runners
    GCP_GPU_GROUPS = set(GCP_VM_GROUPS)
    runners_html = ""
    if queue_data and "self_hosted_runners" in queue_data:
        runners = queue_data["self_hosted_runners"]
        from collections import defaultdict
        groups = defaultdict(list)
        for r in runners:
            g = r.get("group", "Other")
            if g in GCP_GPU_GROUPS and r.get("status") == "online":
                groups[g].append(r)

        # Show all GCP groups, including ones with 0 online runners (auto-scaling)
        for group_name in sorted(GCP_GPU_GROUPS):
            group_runners = groups.get(group_name, [])
            busy = sum(1 for r in group_runners if r.get("busy"))
            total = len(group_runners)

            runners_html += f'<h3>{_esc(group_name)} ({busy}/{total} busy)</h3>\n'
            if not group_runners:
                runners_html += '<p style="color:#6c757d">No runners online (auto-scaling group scales to zero when idle).</p>\n'
                continue
            runners_html += '<table><tr><th>Runner</th><th>Status</th><th>Current Job</th></tr>\n'
            for r in sorted(group_runners, key=lambda x: x.get("name", "")):
                name = _esc(r.get("name", ""))
                busy_flag = r.get("busy", False)
                state = '<span style="color:#0d6efd">BUSY</span>' if busy_flag else '<span style="color:#28a745">IDLE</span>'

                job_info = ""
                job = r.get("job")
                if job:
                    job_name = job.get("name", "")
                    job_branch = job.get("branch", "")
                    job_url = job.get("html_url", "")
                    label = f"{job_name} ({job_branch})" if job_branch else job_name
                    job_info = _link(job_url, label)

                runners_html += f"<tr><td>{name}</td><td>{state}</td><td>{job_info}</td></tr>\n"
            runners_html += "</table>\n"

        # Other runners (non-GCP GPU, online only)
        other_runners = [
            r for r in runners
            if r.get("group", "Other") not in GCP_GPU_GROUPS
            and r.get("status") == "online"
        ]
        if other_runners:
            runners_html += '\n<h3>Other Runners</h3>\n'
            runners_html += '<table><tr><th>Runner</th><th>Group</th><th>Status</th><th>Current Job</th></tr>\n'
            for r in sorted(other_runners, key=lambda x: x.get("name", "")):
                name = _esc(r.get("name", ""))
                group = _esc(r.get("group", ""))
                busy_flag = r.get("busy", False)
                state = '<span style="color:#0d6efd">BUSY</span>' if busy_flag else '<span style="color:#28a745">IDLE</span>'

                job_info = ""
                job = r.get("job")
                if job:
                    job_name = job.get("name", "")
                    job_branch = job.get("branch", "")
                    job_url = job.get("html_url", "")
                    label = f"{job_name} ({job_branch})" if job_branch else job_name
                    job_info = _link(job_url, label)

                runners_html += f"<tr><td>{name}</td><td>{group}</td><td>{state}</td><td>{job_info}</td></tr>\n"
            runners_html += "</table>\n"
    elif queue_data:
        runners_html = "<p>Runner data not available (may require admin access).</p>"
    else:
        runners_html = "<p>Could not fetch runner status.</p>"

    # Queue summary section
    queue_html = ""
    if queue_data:
        summary = queue_data.get("summary", {})
        queue_html = f"""
<div>
  <div class="stat-card"><div class="value">{summary.get('jobs_queued', 0)}</div><div class="label">Jobs Queued</div></div>
  <div class="stat-card"><div class="value">{summary.get('jobs_running', 0)}</div><div class="label">Jobs Running</div></div>
  <div class="stat-card"><div class="value">{summary.get('runs_queued', 0)}</div><div class="label">Runs Queued</div></div>
  <div class="stat-card"><div class="value">{summary.get('runs_in_progress', 0)}</div><div class="label">Runs In Progress</div></div>
</div>
"""
        # Queue depth by group
        groups = queue_data.get("queue_by_group", [])
        if groups:
            queue_html += '\n<h3>Queue Depth by Runner Group</h3>\n'
            queue_html += '<table><tr><th>Group</th><th>Queued</th><th>Running</th>'
            if queue_data.get("runners_available"):
                queue_html += '<th>Runners</th>'
            queue_html += '</tr>\n'
            for g in groups:
                name = g.get("name", "")
                queued = g.get("queued", 0)
                running = g.get("running", 0)
                queue_html += f"<tr><td>{_esc(name)}</td><td>{queued}</td><td>{running}</td>"
                if queue_data.get("runners_available"):
                    runners = g.get("runners", {})
                    idle = runners.get("idle", 0)
                    total = runners.get("total", 0)
                    if total > 0:
                        queue_html += f"<td>{idle} idle / {total} total</td>"
                    elif g.get("self_hosted"):
                        queue_html += "<td>(org-level)</td>"
                    else:
                        queue_html += "<td>(cloud)</td>"
                queue_html += "</tr>\n"
            queue_html += "</table>\n"

        # Longest waiting jobs
        waiting = queue_data.get("longest_waiting_jobs", [])[:5]
        if waiting:
            queue_html += '\n<h3>Longest Waiting Jobs</h3>\n'
            queue_html += '<table><tr><th>Wait</th><th>Job</th><th>Branch</th></tr>\n'
            for j in waiting:
                wait_s = j.get("wait_seconds", 0)
                if wait_s >= 3600:
                    wait_str = f"{wait_s // 3600}h {(wait_s % 3600) // 60:02d}m"
                elif wait_s >= 60:
                    wait_str = f"{wait_s // 60}m {wait_s % 60:02d}s"
                else:
                    wait_str = f"{wait_s}s"
                name = j.get("name", "")
                branch = j.get("branch", "")
                url = j.get("html_url", "")
                name_html = _link(url, name)
                queue_html += f"<tr><td>{wait_str}</td><td>{name_html}</td><td>{_esc(branch)}</td></tr>\n"
            queue_html += "</table>\n"
    else:
        queue_html = "<p>Could not fetch queue status.</p>"

    # Recent failures section
    failures_html = ""
    if failures:
        failures_html = '<table><tr><th>Branch</th><th>Actor</th><th>Time</th></tr>\n'
        for f in failures:
            branch = f.get("branch", "")
            actor = f.get("actor", "")
            url = f.get("url", "")
            created = f.get("created_at", "")[:16].replace("T", " ")
            link = _link(url, branch)
            failures_html += f"<tr><td>{link}</td><td>{_esc(actor)}</td><td>{created}</td></tr>\n"
        failures_html += "</table>\n"
    else:
        failures_html = "<p>No recent CI failures.</p>"

    # Merge queue section
    mq_html = ""
    if mq_data:
        summary = mq_data.get("summary", {})
        fail_rate = 0
        sf_total = summary.get("success", 0) + summary.get("failure", 0)
        if sf_total > 0:
            fail_rate = summary.get("failure", 0) / sf_total * 100

        mq_html = f"""
<div>
  <div class="stat-card"><div class="value">{summary.get('success', 0)}</div><div class="label">Passed (24h)</div></div>
  <div class="stat-card"><div class="value" style="color:#dc3545">{summary.get('failure', 0)}</div><div class="label">Failed (24h)</div></div>
  <div class="stat-card"><div class="value">{summary.get('cancelled', 0)}</div><div class="label">Cancelled (24h)</div></div>
  <div class="stat-card"><div class="value">{summary.get('in_progress', 0)}</div><div class="label">In Progress</div></div>
  <div class="stat-card"><div class="value">{fail_rate:.0f}%</div><div class="label">Failure Rate</div></div>
</div>
"""
        # Recent failures table
        recent_failures = [r for r in mq_data.get("recent", []) if r.get("conclusion") == "failure"]
        if recent_failures:
            mq_html += '\n<h3>Recent Merge Queue Failures</h3>\n'
            mq_html += '<table><tr><th>PR</th><th>Run</th><th>Time</th></tr>\n'
            for r in recent_failures[:10]:
                pr_num = r.get("pr_number", "")
                pr_url = r.get("pr_url", "")
                run_url = r.get("url", "")
                created = r.get("created_at", "")[:16].replace("T", " ")
                if pr_num:
                    pr_link = _link(pr_url, f"#{pr_num}")
                else:
                    pr_link = _esc(r.get("branch", "")[:50])
                run_link = _link(run_url, "logs") if run_url else ""
                mq_html += f"<tr><td>{pr_link}</td><td>{run_link}</td><td>{created}</td></tr>\n"
            mq_html += "</table>\n"
        else:
            mq_html += "<p>No merge queue failures in the last 24 hours.</p>"
    else:
        mq_html = "<p>Could not fetch merge queue status.</p>"

    # Load snapshots and build history chart
    snapshots = load_snapshots(output_dir, hours=24)
    history_html = build_history_chart(snapshots)

    body = f"""
<h1>CI System Health</h1>
<p style="color:#6c757d">Last updated: {fetched_at}</p>

<h2>Queue Status</h2>
{queue_html}

<h2>Merge Queue</h2>
{mq_html}

<h2>Load History</h2>
{history_html}

<h2>Self-Hosted Runner Status</h2>
{runners_html}

<h2>Recent CI Failures</h2>
{failures_html}
"""
    os.makedirs(output_dir, exist_ok=True)
    with open(os.path.join(output_dir, "health.html"), "w") as f:
        f.write(page_template("Health", body, "Health"))


def main():
    args = parse_args()

    print(f"Fetching queue status for {args.repo}...")
    queue_data = fetch_queue_status(args.repo)

    print("Fetching GPU quota...")
    gpu_quota = fetch_gpu_quota()
    if gpu_quota:
        for quota in _normalize_gpu_quota_by_metric(gpu_quota).values():
            print(f"  {quota.get('name', 'GPU')} GPUs: {quota['usage']}/{quota['limit']} in use")
    else:
        print("  GPU quota unavailable (gcloud not configured or not accessible)")

    print("Fetching merge queue status...")
    mq_data = fetch_merge_queue_status(args.repo)
    if mq_data:
        s = mq_data["summary"]
        print(f"  24h: {s['success']} passed, {s['failure']} failed, {s['cancelled']} cancelled, {s['in_progress']} in progress")
    else:
        print("  Merge queue data unavailable")

    print("Recording snapshot...")
    record_snapshot(queue_data, args.output, gpu_quota=gpu_quota, mq_data=mq_data)

    print("Fetching recent CI failures...")
    failures = fetch_recent_failures(args.repo)

    print(f"Generating health.html in {args.output}/...")
    generate_health_html(queue_data, failures, args.output, mq_data=mq_data)

    print("Done.")


if __name__ == "__main__":
    main()
