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
from ci_visualization import page_template


DEFAULT_REPO = "shader-slang/slang"


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


def record_snapshot(queue_data, output_dir):
    """Append a runner status snapshot to the JSONL time-series file."""
    if not queue_data:
        return

    now = datetime.now(timezone.utc)
    snapshot = {"timestamp": now.strftime("%Y-%m-%dT%H:%M:%SZ")}

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


def build_history_chart(snapshots):
    """Build Chart.js HTML for 24h runner load history."""
    if not snapshots:
        return "<p>No history data yet. Snapshots accumulate every 15 minutes.</p>"

    snapshots = _deduplicate_snapshots(snapshots)
    timestamps = [_round_time(s["timestamp"][11:16]) for s in snapshots]

    # Only show GCP VM groups (Linux GPU and Windows GPU), exclude scaler host and test runners
    gcp_vm_groups = ["Linux GPU (GCP)", "Windows GPU (GCP)"]
    palette = {"Linux GPU (GCP)": "#0d6efd", "Windows GPU (GCP)": "#28a745"}

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

    return f"""
<div style="margin-bottom:15px">
  <label>Time window: </label>
  <select id="historyRange" onchange="updateHistoryRange()">
    <option value="4">Last 4 hours</option>
    <option value="12">Last 12 hours</option>
    <option value="24" selected>Last 24 hours</option>
  </select>
</div>
<div style="position:relative;width:100%;max-width:1200px;margin:20px 0">
  <canvas id="runnerHistory"></canvas>
</div>
<div style="position:relative;width:100%;max-width:1200px;margin:20px 0">
  <canvas id="workflowHistory"></canvas>
</div>
<div style="position:relative;width:100%;max-width:1200px;margin:20px 0">
  <canvas id="queueHistory"></canvas>
</div>
<script src="{CHARTJS_CDN}"></script>
<script>
// All snapshot data
const allTimestamps = {json.dumps(timestamps)};
const allRunnerData = {json.dumps({g: group_series[g] for g in gcp_vm_groups})};
const allRunsInProgress = {json.dumps(runs_in_progress)};
const allRunsQueued = {json.dumps(runs_queued)};
const allJobsQueued = {json.dumps(queued_data)};
const allJobsRunning = {json.dumps(running_data)};

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
  charts.runner = new Chart(document.getElementById('runnerHistory').getContext('2d'), {{
    type: 'line',
    data: {{ labels: displayLabels, datasets: runnerDatasets }},
    options: {{
      responsive: true,
      scales: {{ y: {{ min: 0, stacked: true, title: {{ display: true, text: 'GCP VMs Online' }} }} }},
      plugins: {{ title: {{ display: true, text: 'GCP Runner VMs' }} }}
    }}
  }});

  // Workflow runs
  charts.workflow = new Chart(document.getElementById('workflowHistory').getContext('2d'), {{
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
      scales: {{ y: {{ min: 0, title: {{ display: true, text: 'Workflow Runs' }} }} }},
      plugins: {{ title: {{ display: true, text: 'Active CI Workflows' }} }}
    }}
  }});

  // Job queue
  charts.queue = new Chart(document.getElementById('queueHistory').getContext('2d'), {{
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
      scales: {{ y: {{ min: 0, title: {{ display: true, text: 'Jobs' }} }} }},
      plugins: {{ title: {{ display: true, text: 'Job Queue Depth' }} }}
    }}
  }});
}}

function updateHistoryRange() {{
  const hours = parseInt(document.getElementById('historyRange').value);
  buildCharts(hours);
}}

// Initial render
buildCharts(24);
</script>
"""


def generate_health_html(queue_data, failures, output_dir):
    """Generate health.html from live data."""
    now = datetime.now(timezone.utc)
    rounded_time = _round_time(now.strftime("%H:%M"))
    fetched_at = now.strftime(f"%Y-%m-%d {rounded_time} UTC")

    # Runner status section — only online GCP GPU runners
    GCP_GPU_GROUPS = {"Linux GPU (GCP)", "Windows GPU (GCP)"}
    runners_html = ""
    if queue_data and queue_data.get("self_hosted_runners"):
        runners = queue_data["self_hosted_runners"]
        from collections import defaultdict
        groups = defaultdict(list)
        for r in runners:
            g = r.get("group", "Other")
            if g in GCP_GPU_GROUPS and r.get("status") == "online":
                groups[g].append(r)

        if groups:
            for group_name in sorted(groups):
                group_runners = groups[group_name]
                busy = sum(1 for r in group_runners if r.get("busy"))
                total = len(group_runners)

                runners_html += f'<h3>{_esc(group_name)} ({busy}/{total} busy)</h3>\n'
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
        else:
            runners_html = "<p>No GCP GPU runners online (other online runners may be listed below).</p>"

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

    # Load snapshots and build history chart
    snapshots = load_snapshots(output_dir, hours=24)
    history_html = build_history_chart(snapshots)

    body = f"""
<h1>CI System Health</h1>
<p style="color:#6c757d">Last updated: {fetched_at}</p>

<h2>Queue Status</h2>
{queue_html}

<h2>24h Load History</h2>
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

    print("Recording snapshot...")
    record_snapshot(queue_data, args.output)

    print("Fetching recent CI failures...")
    failures = fetch_recent_failures(args.repo)

    print(f"Generating health.html in {args.output}/...")
    generate_health_html(queue_data, failures, args.output)

    print("Done.")


if __name__ == "__main__":
    main()
