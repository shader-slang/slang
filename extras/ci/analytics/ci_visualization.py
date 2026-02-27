#!/usr/bin/env python3
"""
GitHub Actions CI Visualization

Generates static HTML files with interactive visualizations of CI job data.
Reads JSON data from ci_job_collector.py and produces:
- index.html: Landing page with summary and links
- statistics.html: Trend charts (Chart.js)
- month_YYYY-MM.html: Daily Gantt-style timeline per runner
- capacity.html: Runner utilization analysis

Usage:
    python3 ci_visualization.py
    python3 ci_visualization.py --input ci_jobs.json --output ./output
"""

import argparse
import datetime
import html as html_mod
import json
import os
import sys
from collections import defaultdict

# Timeline constants
PIXELS_PER_HOUR = 265
ROW_HEIGHT = 28
TOTAL_WIDTH = PIXELS_PER_HOUR * 24  # 6360px for 24h

# Valid OS names for per-platform charts; excludes test categories
# like materialx, rtx, slangpy that follow the same naming pattern
VALID_OS = {"linux", "macos", "windows"}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate CI analytics HTML from job data."
    )
    parser.add_argument("--input", default="ci_jobs.json", help="Input JSON file")
    parser.add_argument("--output", default="ci_analytics", help="Output directory")
    return parser.parse_args()


def load_config():
    """Load runner group config from runner_config.json."""
    config_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "runner_config.json"
    )
    if os.path.exists(config_path):
        with open(config_path) as f:
            return json.load(f)
    return {"label_groups": [], "non_production_periods": {"runners": {}}}


def classify_group(labels, config):
    """Map job labels to a runner group name."""
    label_set = set(labels) if labels else set()
    for group in config.get("label_groups", []):
        required = set(group["labels"])
        if required <= label_set:
            return group["name"], group.get("self_hosted", False)
    return "Other", False


def parse_dt(s):
    if not s:
        return None
    return datetime.datetime.fromisoformat(s.replace("Z", "+00:00"))


def format_duration(seconds):
    if seconds is None or seconds < 0:
        return "N/A"
    seconds = int(seconds)
    if seconds < 60:
        return f"{seconds}s"
    m, s = divmod(seconds, 60)
    if m < 60:
        return f"{m}m {s:02d}s"
    h, m = divmod(m, 60)
    return f"{h}h {m:02d}m"


# --- Shared HTML ---

CHARTJS_CDN = "https://cdn.jsdelivr.net/npm/chart.js"

CONCLUSION_COLORS = {
    "success": "#28a745",
    "failure": "#dc3545",
    "cancelled": "#ffc107",
    "skipped": "#6c757d",
}


def chart_section(chart_id, title, description="", canvas_style=""):
    """Generate HTML for a chart section with anchor link and download button."""
    desc_html = f'\n<p style="color:#6c757d;font-size:0.9em">{description}</p>' if description else ""
    style_attr = f' style="{canvas_style}"' if canvas_style else ""
    return f"""<div class="chart-section" id="{chart_id}">
  <h2>{title} <a class="anchor" href="#{chart_id}">#</a>
  <button class="download-btn" onclick="downloadChart('{chart_id}_canvas')">PNG</button></h2>{desc_html}
  <div class="chart-container"{style_attr}><canvas id="{chart_id}_canvas"></canvas></div>
</div>"""


DOWNLOAD_JS = """
function downloadChart(canvasId) {
  var canvas = document.getElementById(canvasId);
  if (!canvas) return;
  var link = document.createElement('a');
  link.download = canvasId.replace('_canvas', '') + '.png';
  link.href = canvas.toDataURL('image/png', 1.0);
  link.click();
}
"""


def nav_html(active=""):
    links = [
        ("index.html", "Home"),
        ("statistics.html", "Statistics"),
        ("capacity.html", "Capacity"),
    ]
    items = []
    for href, label in links:
        cls = ' style="font-weight:bold;text-decoration:underline"' if label.lower() == active.lower() else ""
        items.append(f'<a href="{href}"{cls}>{label}</a>')
    return f"""<nav style="background:#343a40;padding:10px 20px;color:white;font-family:sans-serif">
  <strong>Slang CI Analytics</strong> &nbsp;|&nbsp; {' &nbsp;|&nbsp; '.join(items)}
</nav>"""


def page_template(title, body, active=""):
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title} - Slang CI Analytics</title>
<style>
  body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; margin: 0; background: #fff; color: #333; }}
  nav a {{ color: #adb5bd; text-decoration: none; }}
  nav a:hover {{ color: white; }}
  .container {{ max-width: 1400px; margin: 0 auto; padding: 20px; }}
  h1, h2, h3 {{ color: #212529; }}
  table {{ border-collapse: collapse; width: 100%; }}
  th, td {{ border: 1px solid #dee2e6; padding: 8px; text-align: left; }}
  th {{ background: #f8f9fa; }}
  .chart-container {{ position: relative; width: 100%; max-width: 1200px; margin: 20px 0; }}
  .chart-section {{ margin-bottom: 30px; }}
  .chart-section h2 {{ display: flex; align-items: center; gap: 8px; }}
  .chart-section h2 a.anchor {{ color: #adb5bd; text-decoration: none; font-size: 0.7em; visibility: hidden; }}
  .chart-section:hover h2 a.anchor {{ visibility: visible; }}
  .chart-section h2 a.anchor:hover {{ color: #0d6efd; }}
  .download-btn {{ font-size: 12px; color: #6c757d; cursor: pointer; border: 1px solid #dee2e6; background: white; padding: 2px 8px; border-radius: 4px; margin-left: 8px; }}
  .download-btn:hover {{ background: #f8f9fa; color: #333; }}
  .stat-card {{ display: inline-block; background: #f8f9fa; border-radius: 8px; padding: 15px 25px; margin: 5px; text-align: center; }}
  .stat-card .value {{ font-size: 2em; font-weight: bold; color: #212529; }}
  .stat-card .label {{ font-size: 0.9em; color: #6c757d; }}
</style>
</head>
<body>
{nav_html(active)}
<div class="container">
{body}
</div>
</body>
</html>"""


# --- Data processing ---


def process_jobs(jobs_data, config):
    """Process raw job data into structures needed for visualization."""
    # Filter out skipped jobs for most analysis
    active_jobs = [j for j in jobs_data if j.get("conclusion") != "skipped"]
    warn_no_build_test = True

    # Exclude current (incomplete) day — only generate full days
    today_str = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d")

    # Group by date
    jobs_by_date = defaultdict(list)
    for job in active_jobs:
        created = job.get("created_at")
        if not created:
            continue
        try:
            dt = parse_dt(created)
            date_str = dt.strftime("%Y-%m-%d")
            if date_str == today_str:
                continue
            jobs_by_date[date_str].append(job)
        except (ValueError, TypeError):
            continue

    # Group by month
    jobs_by_month = defaultdict(list)
    for date_str, djobs in jobs_by_date.items():
        month = date_str[:7]
        jobs_by_month[month].extend(djobs)

    # Classify runner groups
    for job in jobs_data:
        group, is_sh = classify_group(job.get("labels", []), config)
        job["_group"] = group
        job["_self_hosted"] = is_sh

    # Compute per-run turnaround time:
    # earliest job created_at to latest job completed_at for each run_id.
    # Group by the date of the earliest created_at.
    # Computed separately for the main CI workflow ("CI") and all workflows.
    runs = defaultdict(list)
    for job in active_jobs:
        run_id = job.get("run_id")
        if run_id:
            runs[run_id].append(job)

    turnaround_by_date = defaultdict(list)  # date -> list of turnaround minutes
    ci_turnaround_by_date = defaultdict(list)  # same but only "CI" workflow
    ci_sol_by_date = defaultdict(list)  # speed of light per CI run
    build_wait_by_date = defaultdict(list)  # build queue wait per CI run
    test_wait_by_date = defaultdict(list)  # test queue wait (after build) per CI run
    for run_id, run_jobs in runs.items():
        # Find earliest job created_at and latest completed_at
        earliest_job = None
        latest = None
        for j in run_jobs:
            c = parse_dt(j.get("created_at"))
            d = parse_dt(j.get("completed_at"))
            if c and (earliest_job is None or c < earliest_job):
                earliest_job = c
            if d and (latest is None or d > latest):
                latest = d

        if not earliest_job or not latest or latest <= earliest_job:
            continue

        # Use run_created_at (when the workflow was triggered) as the start,
        # since that's when the developer starts waiting.
        # But for re-runs, run_created_at is the *original* trigger time
        # which can be days earlier. Detect this: if run_created_at is more
        # than 10 minutes before the first job, it's a re-run and we use
        # the job timestamp instead.
        run_start = earliest_job
        rc = run_jobs[0].get("run_created_at")
        if rc:
            run_trigger = parse_dt(rc)
            if run_trigger and (earliest_job - run_trigger).total_seconds() < 600:
                run_start = run_trigger

        turnaround_min = (latest - run_start).total_seconds() / 60
        date_str = run_start.strftime("%Y-%m-%d")
        turnaround_by_date[date_str].append(turnaround_min)
        wf_name = run_jobs[0].get("workflow_name", "")
        if wf_name == "CI":
            ci_turnaround_by_date[date_str].append(turnaround_min)

            # Speed of light: max across platforms of (longest build + longest test).
            # This is the fastest possible turnaround with full parallelization,
            # limited only by the critical path of build->test per platform.
            platform_times = defaultdict(lambda: {"build": 0, "test": 0})
            for j in run_jobs:
                dur = j.get("duration_seconds") or 0
                if dur <= 0:
                    continue
                jname = j.get("name", "")
                if jname.startswith("build-"):
                    os_name = jname.split("-", 2)[1]
                    if os_name not in VALID_OS:
                        continue
                    platform_times[os_name]["build"] = max(
                        platform_times[os_name]["build"], dur
                    )
                    warn_no_build_test = False
                elif jname.startswith("test-"):
                    os_name = jname.split("-", 2)[1]
                    if os_name not in VALID_OS:
                        continue
                    platform_times[os_name]["test"] = max(
                        platform_times[os_name]["test"], dur
                    )
                    warn_no_build_test = False
            if platform_times:
                sol = max(
                    (t["build"] + t["test"]) for t in platform_times.values()
                )
                ci_sol_by_date[date_str].append(sol / 60)

            # Build wait: run trigger to first build job started
            # Test wait: last build completed to first test started (per platform)
            build_starts = []
            build_ends_by_os = defaultdict(list)
            test_starts_by_os = defaultdict(list)
            for j in run_jobs:
                jname = j.get("name", "")
                started = parse_dt(j.get("started_at"))
                completed = parse_dt(j.get("completed_at"))
                if jname.startswith("build-") and started:
                    os_name = jname.split("-", 2)[1]
                    if os_name not in VALID_OS:
                        continue
                    build_starts.append(started)
                    if completed:
                        build_ends_by_os[os_name].append(completed)
                elif jname.startswith("test-") and started:
                    os_name = jname.split("-", 2)[1]
                    if os_name not in VALID_OS:
                        continue
                    test_starts_by_os[os_name].append(started)

            if build_starts and run_start:
                first_build = min(build_starts)
                bw = (first_build - run_start).total_seconds() / 60
                if bw >= 0:
                    build_wait_by_date[date_str].append(bw)

            # Test wait per platform: gap between last build end and first test start
            test_waits = []
            for os_name in test_starts_by_os:
                if os_name in build_ends_by_os and test_starts_by_os[os_name]:
                    last_build = max(build_ends_by_os[os_name])
                    first_test = min(test_starts_by_os[os_name])
                    tw = (first_test - last_build).total_seconds() / 60
                    if tw >= 0:
                        test_waits.append(tw)
            if test_waits:
                # Use max across platforms (worst case wait)
                test_wait_by_date[date_str].append(max(test_waits))

    if warn_no_build_test:
        print(
            "Warning: No jobs matched 'build-*' or 'test-*' naming; "
            "CI build/test breakdowns may be empty.",
            file=sys.stderr,
        )

    return {
        "all_jobs": jobs_data,
        "active_jobs": active_jobs,
        "jobs_by_date": dict(jobs_by_date),
        "jobs_by_month": dict(jobs_by_month),
        "turnaround_by_date": dict(turnaround_by_date),
        "ci_turnaround_by_date": dict(ci_turnaround_by_date),
        "ci_sol_by_date": dict(ci_sol_by_date),
        "build_wait_by_date": dict(build_wait_by_date),
        "test_wait_by_date": dict(test_wait_by_date),
        "dates": sorted(jobs_by_date.keys()),
        "months": sorted(jobs_by_month.keys()),
    }


# --- Index page ---


def _avg_last_n_days(by_date, dates, n):
    """Average of values across the last n dates."""
    recent = dates[-n:] if len(dates) >= n else dates
    all_vals = []
    for d in recent:
        all_vals.extend(by_date.get(d, []))
    return sum(all_vals) / len(all_vals) if all_vals else 0


def generate_index(data, output_dir):
    active = data["active_jobs"]
    dates = data["dates"]
    total = len(active)
    success = sum(1 for j in active if j.get("conclusion") == "success")
    success_rate = (success / total * 100) if total > 0 else 0

    # Last 3 days key figures
    ci_tat_3d = _avg_last_n_days(data.get("ci_turnaround_by_date", {}), dates, 3)

    # Active PRs: count unique PR branches in last 3 days
    recent_dates = dates[-3:] if len(dates) >= 3 else dates
    pr_branches = set()
    for d in recent_dates:
        for j in data["jobs_by_date"].get(d, []):
            if j.get("event") == "pull_request" and j.get("head_branch"):
                pr_branches.add(j["head_branch"])
    prs_3d = len(pr_branches) / len(recent_dates) if recent_dates else 0

    # Queue wait: average across last 3 days
    queue_vals = []
    for d in recent_dates:
        for j in data["jobs_by_date"].get(d, []):
            q = j.get("queued_seconds")
            if q and q >= 0:
                queue_vals.append(q / 60)
    queue_3d = sum(queue_vals) / len(queue_vals) if queue_vals else 0

    # Build and test wait
    bw_3d = _avg_last_n_days(data.get("build_wait_by_date", {}), dates, 3)
    tw_3d = _avg_last_n_days(data.get("test_wait_by_date", {}), dates, 3)

    months_html = ""
    for month in reversed(data["months"]):
        count = len(data["jobs_by_month"][month])
        months_html += f'<li><a href="month_{month}.html">{month}</a> ({count} jobs)</li>\n'

    body = f"""
<h1>Slang CI Analytics</h1>
<h2>Last 3 Days</h2>
<div>
  <div class="stat-card"><div class="value">{ci_tat_3d:.0f}m</div><div class="label">CI Turnaround (avg)</div></div>
  <div class="stat-card"><div class="value">{prs_3d:.0f}</div><div class="label">Active PRs / day</div></div>
  <div class="stat-card"><div class="value">{queue_3d:.1f}m</div><div class="label">Avg Queue Wait</div></div>
  <div class="stat-card"><div class="value">{bw_3d:.1f}m</div><div class="label">Build Wait</div></div>
  <div class="stat-card"><div class="value">{tw_3d:.1f}m</div><div class="label">Test Wait (after build)</div></div>
</div>
<h2>Overall</h2>
<div>
  <div class="stat-card"><div class="value">{total}</div><div class="label">Total Jobs</div></div>
  <div class="stat-card"><div class="value">{success_rate:.1f}%</div><div class="label">Success Rate</div></div>
</div>
<p style="color:#6c757d;margin-top:10px">Excludes skipped jobs. Data range: {dates[0] if dates else 'N/A'} to {dates[-1] if dates else 'N/A'}</p>

<h2>Pages</h2>
<ul>
  <li><a href="statistics.html">Statistics &amp; Trends</a></li>
  <li><a href="capacity.html">Runner Capacity Analysis</a></li>
</ul>

<h2>Monthly Timelines</h2>
<ul>
{months_html}
</ul>
"""
    with open(os.path.join(output_dir, "index.html"), "w") as f:
        f.write(page_template("Home", body, "Home"))


# --- Statistics page ---


def generate_statistics(data, output_dir):
    dates = data["dates"]
    jobs_by_date = data["jobs_by_date"]
    active = data["active_jobs"]

    # Per-day stats
    labels_json = json.dumps(dates)

    success_per_day = []
    failure_per_day = []
    cancelled_per_day = []
    runs_per_day = []
    prs_per_day = []
    avg_duration_per_day = []
    avg_queue_per_day = []
    failure_rate_per_day = []

    for date in dates:
        djobs = jobs_by_date[date]
        s = sum(1 for j in djobs if j.get("conclusion") == "success")
        f = sum(1 for j in djobs if j.get("conclusion") == "failure")
        c = sum(1 for j in djobs if j.get("conclusion") == "cancelled")
        success_per_day.append(s)
        failure_per_day.append(f)
        cancelled_per_day.append(c)

        run_ids = set(j.get("run_id") for j in djobs if j.get("run_id"))
        runs_per_day.append(len(run_ids))

        pr_branches = set(
            j.get("head_branch") for j in djobs
            if j.get("event") == "pull_request" and j.get("head_branch")
        )
        prs_per_day.append(len(pr_branches))

        durs = [j["duration_seconds"] for j in djobs if j.get("duration_seconds") and j["duration_seconds"] > 0]
        avg_duration_per_day.append(round(sum(durs) / len(durs) / 60, 1) if durs else 0)

        queues = [j["queued_seconds"] for j in djobs if j.get("queued_seconds") and j["queued_seconds"] >= 0]
        avg_queue_per_day.append(round(sum(queues) / len(queues) / 60, 1) if queues else 0)

        total_day = s + f + c
        failure_rate_per_day.append(round(f / total_day * 100, 1) if total_day > 0 else 0)

    # 7-day moving average for duration
    ma_duration = []
    for i in range(len(avg_duration_per_day)):
        window = avg_duration_per_day[max(0, i - 6) : i + 1]
        ma_duration.append(round(sum(window) / len(window), 1))

    # CI turnaround time per day — main CI workflow only
    ci_turnaround_by_date = data.get("ci_turnaround_by_date", {})
    ci_avg_tat = []
    ci_median_tat = []
    ci_p95_tat = []
    for date in dates:
        tats = sorted(ci_turnaround_by_date.get(date, []))
        if tats:
            ci_avg_tat.append(round(sum(tats) / len(tats), 1))
            ci_median_tat.append(round(tats[len(tats) // 2], 1))
            ci_p95_tat.append(round(tats[int(len(tats) * 0.95)], 1))
        else:
            ci_avg_tat.append(0)
            ci_median_tat.append(0)
            ci_p95_tat.append(0)

    # Speed of light per day (average across CI runs)
    ci_sol_by_date = data.get("ci_sol_by_date", {})
    ci_sol_avg = []
    for date in dates:
        sols = ci_sol_by_date.get(date, [])
        ci_sol_avg.append(round(sum(sols) / len(sols), 1) if sols else 0)

    # Build and test duration breakdown per OS (CI workflow only)
    # Parse job names like "build-linux-debug-gcc-x86_64 / build"
    # and "test-windows-release-cl-x86_64-gpu / test-slang"
    os_phases = defaultdict(lambda: defaultdict(list))  # os -> phase -> [durations]
    os_phase_by_date = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))
    for j in active:
        if j.get("workflow_name") != "CI":
            continue
        dur = j.get("duration_seconds")
        if not dur or dur <= 0:
            continue
        name = j.get("name", "")
        # Determine phase and OS from job name prefix
        if name.startswith("build-"):
            phase = "build"
        elif name.startswith("test-"):
            phase = "test"
        else:
            continue
        # Extract OS: build-{os}-... or test-{os}-...
        parts = name.split("-", 2)
        if len(parts) < 2:
            continue
        os_name = parts[1]
        if os_name not in VALID_OS:
            continue
        os_phases[os_name][phase].append(dur / 60)

        created = j.get("created_at")
        if created:
            try:
                dt = parse_dt(created)
                date_str = dt.strftime("%Y-%m-%d")
                os_phase_by_date[date_str][os_name][phase].append(dur / 60)
            except (ValueError, TypeError):
                pass

    # Build per-day average for each OS+phase combo
    os_list = sorted(os_phases.keys())
    os_phase_avg_by_date = {}
    for os_name in os_list:
        for phase in ["build", "test"]:
            key = f"{os_name}_{phase}"
            series = []
            for date in dates:
                vals = os_phase_by_date.get(date, {}).get(os_name, {}).get(phase, [])
                series.append(round(sum(vals) / len(vals), 1) if vals else 0)
            os_phase_avg_by_date[key] = series

    # Jobs by runner group
    group_counts = defaultdict(int)
    for j in active:
        group_counts[j.get("_group", "Other")] += 1
    group_names = sorted(group_counts.keys())
    group_values = [group_counts[g] for g in group_names]

    # Jobs by event type
    event_counts = defaultdict(int)
    for j in active:
        event_counts[j.get("event", "unknown")] += 1
    event_names = sorted(event_counts.keys())
    event_values = [event_counts[e] for e in event_names]

    # Build and test wait times per day
    build_wait_by_date = data.get("build_wait_by_date", {})
    test_wait_by_date = data.get("test_wait_by_date", {})
    avg_build_wait = []
    avg_test_wait = []
    for date in dates:
        bw = build_wait_by_date.get(date, [])
        avg_build_wait.append(round(sum(bw) / len(bw), 1) if bw else 0)
        tw = test_wait_by_date.get(date, [])
        avg_test_wait.append(round(sum(tw) / len(tw), 1) if tw else 0)

    body = f"""
<h1>Statistics &amp; Trends</h1>

<div style="margin-bottom:15px">
  <label>Date range: </label>
  <select id="rangeSelect" onchange="updateRange()">
    <option value="7">Last 7 days</option>
    <option value="30" selected>Last 30 days</option>
    <option value="90">Last 90 days</option>
    <option value="0">All time</option>
  </select>
</div>

{chart_section("turnaround", "CI Turnaround Time (minutes)",
    "Main CI workflow only. Time from workflow trigger to last job completed per run. Speed of light = max(build+test) across platforms, assuming full parallelization.")}

{chart_section("prsPerDay", "Active PRs per Day")}

{chart_section("buildTestWait", "Build and Test Wait Times (minutes)",
    "Build wait: run trigger to first build started. Test wait: last build completed to first test started (worst platform).")}

{chart_section("buildByOs", "Build Duration by OS (minutes)",
    "Average build job duration per day, CI workflow only.")}

{chart_section("testByOs", "Test Duration by OS (minutes)",
    "Average test job duration per day, CI workflow only.")}

{chart_section("jobsPerDay", "Jobs per Day")}

{chart_section("runsPerDay", "Workflow Runs per Day")}

{chart_section("avgDuration", "Average Job Duration (minutes)")}

{chart_section("avgQueue", "Average Queue Wait Time (minutes)")}

{chart_section("failureRate", "Failure Rate (%)")}

{chart_section("byGroup", "Jobs by Runner Group", canvas_style="max-width:800px")}

{chart_section("byEvent", "Jobs by Event Type", canvas_style="max-width:500px")}

<script src="{CHARTJS_CDN}"></script>
<script>
const allLabels = {labels_json};
const allSuccess = {json.dumps(success_per_day)};
const allFailure = {json.dumps(failure_per_day)};
const allCancelled = {json.dumps(cancelled_per_day)};
const allRuns = {json.dumps(runs_per_day)};
const allPRs = {json.dumps(prs_per_day)};
const allDuration = {json.dumps(avg_duration_per_day)};
const allMaDuration = {json.dumps(ma_duration)};
const allQueue = {json.dumps(avg_queue_per_day)};
const allFailRate = {json.dumps(failure_rate_per_day)};
const allTurnAvg = {json.dumps(ci_avg_tat)};
const allTurnMedian = {json.dumps(ci_median_tat)};
const allTurnP95 = {json.dumps(ci_p95_tat)};
const allSoL = {json.dumps(ci_sol_avg)};
const allBuildWait = {json.dumps(avg_build_wait)};
const allTestWait = {json.dumps(avg_test_wait)};

let charts = [];

{DOWNLOAD_JS}

function sliceData(arr, n) {{
  return n === 0 ? arr : arr.slice(-n);
}}

function makeChart(id, type, config) {{
  const ctx = document.getElementById(id).getContext('2d');
  const c = new Chart(ctx, {{type, ...config}});
  charts.push({{chart: c, id, type, config}});
  return c;
}}

function updateRange() {{
  const n = parseInt(document.getElementById('rangeSelect').value);
  const labs = sliceData(allLabels, n);
  charts.forEach(item => {{
    const c = item.chart;
    c.data.labels = labs;
    c.data.datasets.forEach((ds, i) => {{
      if (ds._allData) ds.data = sliceData(ds._allData, n);
    }});
    c.update();
  }});
}}

// CI Turnaround time
makeChart('turnaround_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [
      {{label:'Average', data:sliceData(allTurnAvg,30), _allData:allTurnAvg, borderColor:'#0d6efd', fill:false, tension:0.1}},
      {{label:'Median', data:sliceData(allTurnMedian,30), _allData:allTurnMedian, borderColor:'#28a745', fill:false, tension:0.1}},
      {{label:'p95', data:sliceData(allTurnP95,30), _allData:allTurnP95, borderColor:'#dc3545', borderDash:[5,5], fill:false, tension:0.1}},
      {{label:'Speed of Light', data:sliceData(allSoL,30), _allData:allSoL, borderColor:'#6c757d', borderDash:[2,4], fill:false, tension:0.1, pointStyle:'triangle'}},
    ]
  }},
  options: {{responsive:true, scales:{{y:{{title:{{display:true,text:'Minutes'}}}}}}}}
}});

// Build and test wait times
makeChart('buildTestWait_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [
      {{label:'Build Wait', data:sliceData(allBuildWait,30), _allData:allBuildWait, borderColor:'#0d6efd', fill:false, tension:0.1}},
      {{label:'Test Wait (after build)', data:sliceData(allTestWait,30), _allData:allTestWait, borderColor:'#dc3545', fill:false, tension:0.1}},
    ]
  }},
  options: {{responsive:true, scales:{{y:{{title:{{display:true,text:'Minutes'}}}}}}}}
}});

// Build duration by OS
makeChart('buildByOs_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{','.join(
      f"""{{label:'{os_name}', data:sliceData({json.dumps(os_phase_avg_by_date.get(f'{os_name}_build', [0]*len(dates)))},30), _allData:{json.dumps(os_phase_avg_by_date.get(f'{os_name}_build', [0]*len(dates)))}, borderColor:'{["#0d6efd","#28a745","#ffc107","#dc3545","#6f42c1","#fd7e14","#20c997","#e83e8c","#17a2b8","#6610f2"][i % 10]}', fill:false, tension:0.1}}"""
      for i, os_name in enumerate(os_list)
      if f'{os_name}_build' in os_phase_avg_by_date
    )}]
  }},
  options: {{responsive:true, scales:{{y:{{title:{{display:true,text:'Minutes'}}}}}}}}
}});

// Test duration by OS
makeChart('testByOs_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{','.join(
      f"""{{label:'{os_name}', data:sliceData({json.dumps(os_phase_avg_by_date.get(f'{os_name}_test', [0]*len(dates)))},30), _allData:{json.dumps(os_phase_avg_by_date.get(f'{os_name}_test', [0]*len(dates)))}, borderColor:'{["#0d6efd","#28a745","#ffc107","#dc3545","#6f42c1","#fd7e14","#20c997","#e83e8c","#17a2b8","#6610f2"][i % 10]}', fill:false, tension:0.1}}"""
      for i, os_name in enumerate(os_list)
      if f'{os_name}_test' in os_phase_avg_by_date
    )}]
  }},
  options: {{responsive:true, scales:{{y:{{title:{{display:true,text:'Minutes'}}}}}}}}
}});

// Jobs per day (stacked bar)
makeChart('jobsPerDay_canvas', 'bar', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [
      {{label:'Success', data:sliceData(allSuccess,30), _allData:allSuccess, backgroundColor:'{CONCLUSION_COLORS["success"]}'}},
      {{label:'Failure', data:sliceData(allFailure,30), _allData:allFailure, backgroundColor:'{CONCLUSION_COLORS["failure"]}'}},
      {{label:'Cancelled', data:sliceData(allCancelled,30), _allData:allCancelled, backgroundColor:'{CONCLUSION_COLORS["cancelled"]}'}},
    ]
  }},
  options: {{responsive:true, scales:{{x:{{stacked:true}},y:{{stacked:true}}}}}}
}});

// Active PRs per day
makeChart('prsPerDay_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{{label:'Active PRs', data:sliceData(allPRs,30), _allData:allPRs, borderColor:'#28a745', fill:true, backgroundColor:'rgba(40,167,69,0.1)', tension:0.1}}]
  }},
  options: {{responsive:true}}
}});

// Runs per day
makeChart('runsPerDay_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{{label:'Workflow Runs', data:sliceData(allRuns,30), _allData:allRuns, borderColor:'#0d6efd', fill:false, tension:0.1}}]
  }},
  options: {{responsive:true}}
}});

// Average duration
makeChart('avgDuration_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [
      {{label:'Avg Duration', data:sliceData(allDuration,30), _allData:allDuration, borderColor:'#0d6efd', fill:false, tension:0.1}},
      {{label:'7-day MA', data:sliceData(allMaDuration,30), _allData:allMaDuration, borderColor:'#fd7e14', borderDash:[5,5], fill:false, tension:0.1}},
    ]
  }},
  options: {{responsive:true}}
}});

// Queue wait time
makeChart('avgQueue_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{{label:'Avg Queue (min)', data:sliceData(allQueue,30), _allData:allQueue, borderColor:'#6f42c1', fill:true, backgroundColor:'rgba(111,66,193,0.1)', tension:0.1}}]
  }},
  options: {{responsive:true}}
}});

// Failure rate
makeChart('failureRate_canvas', 'line', {{
  data: {{
    labels: sliceData(allLabels, 30),
    datasets: [{{label:'Failure Rate %', data:sliceData(allFailRate,30), _allData:allFailRate, borderColor:'{CONCLUSION_COLORS["failure"]}', fill:true, backgroundColor:'rgba(220,53,69,0.1)', tension:0.1}}]
  }},
  options: {{responsive:true, scales:{{y:{{min:0,max:100}}}}}}
}});

// By runner group (static, no range filter)
new Chart(document.getElementById('byGroup_canvas').getContext('2d'), {{
  type: 'bar',
  data: {{
    labels: {json.dumps(group_names)},
    datasets: [{{label:'Jobs', data:{json.dumps(group_values)}, backgroundColor:'#0d6efd'}}]
  }},
  options: {{responsive:true, indexAxis:'y'}}
}});

// By event type (static)
new Chart(document.getElementById('byEvent_canvas').getContext('2d'), {{
  type: 'pie',
  data: {{
    labels: {json.dumps(event_names)},
    datasets: [{{data:{json.dumps(event_values)}, backgroundColor:['#0d6efd','#28a745','#ffc107','#dc3545','#6f42c1','#fd7e14','#20c997','#e83e8c']}}]
  }},
  options: {{responsive:true}}
}});
</script>
"""
    with open(os.path.join(output_dir, "statistics.html"), "w") as f:
        f.write(page_template("Statistics", body, "Statistics"))


# --- Monthly timeline pages ---


def generate_month_page(month, month_jobs, config, output_dir):
    """Generate a monthly timeline page with daily Gantt views."""
    # Group by date, then by runner
    days = defaultdict(lambda: defaultdict(list))
    workflows = set()
    branches = set()

    for job in month_jobs:
        started = parse_dt(job.get("started_at"))
        completed = parse_dt(job.get("completed_at"))
        if not started or not completed:
            continue
        runner = job.get("runner_name") or "unknown"
        workflows.add(job.get("workflow_name", ""))
        branches.add(job.get("head_branch", ""))
        if completed <= started:
            continue

        # Split jobs into per-day segments to handle midnight spanning.
        day_start = datetime.datetime.combine(
            started.date(), datetime.time.min, tzinfo=started.tzinfo
        )
        while day_start < completed:
            next_day = day_start + datetime.timedelta(days=1)
            seg_start = max(started, day_start)
            seg_end = min(completed, next_day)
            if seg_end > seg_start:
                date_str = day_start.strftime("%Y-%m-%d")
                seg = dict(job)
                seg["_seg_started_at"] = seg_start.isoformat()
                seg["_seg_completed_at"] = seg_end.isoformat()
                days[date_str][runner].append(seg)
            day_start = next_day

    sorted_dates = sorted(days.keys())

    # Build HTML for each day
    days_html = []
    for date_str in sorted_dates:
        runners_data = days[date_str]
        sorted_runners = sorted(runners_data.keys())
        num_runners = len(sorted_runners)
        timeline_height = num_runners * ROW_HEIGHT + 30  # header

        # Runner rows with job blocks
        rows_html = []
        for ri, runner in enumerate(sorted_runners):
            top = 30 + ri * ROW_HEIGHT
            runner_esc = html_mod.escape(runner)
            # Runner label
            rows_html.append(
                f'<div style="position:absolute;left:0;top:{top}px;width:120px;height:{ROW_HEIGHT}px;'
                f'font-size:11px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis;'
                f'line-height:{ROW_HEIGHT}px;padding:0 4px;background:#f8f9fa;border-bottom:1px solid #eee;z-index:1" '
                f'title="{runner_esc}">{runner_esc}</div>'
            )

            for job in runners_data[runner]:
                seg_start = job.get("_seg_started_at")
                seg_end = job.get("_seg_completed_at")
                started = parse_dt(seg_start) if seg_start else parse_dt(job["started_at"])
                completed = parse_dt(seg_end) if seg_end else parse_dt(job["completed_at"])
                # Position in pixels from midnight UTC
                start_hour = started.hour + started.minute / 60 + started.second / 3600
                dur_hours = (completed - started).total_seconds() / 3600
                left = 120 + int(start_hour * PIXELS_PER_HOUR)
                width = max(int(dur_hours * PIXELS_PER_HOUR), 3)

                conclusion = job.get("conclusion", "")
                color = CONCLUSION_COLORS.get(conclusion, "#6c757d")
                name = html_mod.escape(job.get("name", ""))
                wf = html_mod.escape(job.get("workflow_name", ""))
                branch = html_mod.escape(job.get("head_branch", ""))
                dur = format_duration((completed - started).total_seconds())
                queue = format_duration(job.get("queued_seconds"))
                url = html_mod.escape(job.get("html_url", ""), quote=True)

                tooltip = f"{name}\\n{wf} / {branch}\\nDuration: {dur}, Queue: {queue}\\nConclusion: {conclusion}"

                rows_html.append(
                    f'<a href="{url}" target="_blank" '
                    f'class="job-block" '
                    f'data-wf="{wf}" data-branch="{branch}" '
                    f'style="position:absolute;left:{left}px;top:{top + 2}px;'
                    f'width:{width}px;height:{ROW_HEIGHT - 4}px;'
                    f'background:{color};border-radius:3px;opacity:0.85;'
                    f'display:block;text-decoration:none;cursor:pointer;overflow:hidden;'
                    f'font-size:10px;color:white;line-height:{ROW_HEIGHT - 4}px;padding:0 2px;'
                    f'white-space:nowrap" title="{tooltip}">{name}</a>'
                )

        # Hour markers
        hour_markers = []
        for h in range(25):
            x = 120 + h * PIXELS_PER_HOUR
            hour_markers.append(
                f'<div style="position:absolute;left:{x}px;top:0;height:100%;'
                f'border-left:1px solid #eee;font-size:10px;color:#999;padding:2px 4px">'
                f'{h:02d}:00</div>'
            )

        total_w = 120 + TOTAL_WIDTH
        days_html.append(f"""
<h3>{date_str} ({num_runners} runners, {sum(len(v) for v in runners_data.values())} jobs)</h3>
<div style="overflow-x:auto;margin-bottom:20px">
<div style="position:relative;width:{total_w}px;height:{timeline_height}px;background:white;border:1px solid #dee2e6">
{''.join(hour_markers)}
{''.join(rows_html)}
</div>
</div>
""")

    # Filter controls
    wf_options = "".join(
        f'<option value="{html_mod.escape(w)}">{html_mod.escape(w)}</option>'
        for w in sorted(workflows) if w
    )
    br_options = "".join(
        f'<option value="{html_mod.escape(b)}">{html_mod.escape(b)}</option>'
        for b in sorted(branches) if b
    )

    body = f"""
<h1>Timeline: {month}</h1>
<div style="margin-bottom:15px">
  <label>Workflow: </label>
  <select id="wfFilter" onchange="filterJobs()">
    <option value="">All</option>
    {wf_options}
  </select>
  &nbsp;
  <label>Branch: </label>
  <select id="brFilter" onchange="filterJobs()">
    <option value="">All</option>
    {br_options}
  </select>
</div>
{''.join(days_html)}
<script>
function filterJobs() {{
  const wf = document.getElementById('wfFilter').value;
  const br = document.getElementById('brFilter').value;
  document.querySelectorAll('.job-block').forEach(el => {{
    const matchWf = !wf || el.dataset.wf === wf;
    const matchBr = !br || el.dataset.branch === br;
    el.style.display = (matchWf && matchBr) ? 'block' : 'none';
  }});
}}
</script>
"""
    with open(os.path.join(output_dir, f"month_{month}.html"), "w") as f:
        f.write(page_template(f"Timeline {month}", body, ""))


# --- Capacity page ---


def generate_capacity(data, config, output_dir):
    """Generate capacity analysis page."""
    dates = data["dates"]
    jobs_by_date = data["jobs_by_date"]

    # Find self-hosted groups and their configured runner counts.
    sh_groups = {}  # name -> runner_count
    for g in config.get("label_groups", []):
        if g.get("self_hosted"):
            name = g["name"]
            if name not in sh_groups:
                sh_groups[name] = g.get("runner_count", 0)

    # Per-day parallelization rate: average concurrent runners busy
    # = total_busy_seconds / 86400
    # No dependency on fleet size — purely demand-driven.
    group_parallel_per_day = defaultdict(list)
    avg_queue_per_day = []
    p50_queue = []
    p90_queue = []
    p95_queue = []

    for date in dates:
        djobs = jobs_by_date[date]

        # Queue time percentiles
        queues = sorted([j["queued_seconds"] for j in djobs if j.get("queued_seconds") and j["queued_seconds"] >= 0])
        if queues:
            avg_queue_per_day.append(round(sum(queues) / len(queues) / 60, 1))
            p50_queue.append(round(queues[len(queues) // 2] / 60, 1))
            p90_queue.append(round(queues[int(len(queues) * 0.9)] / 60, 1))
            p95_queue.append(round(queues[int(len(queues) * 0.95)] / 60, 1))
        else:
            avg_queue_per_day.append(0)
            p50_queue.append(0)
            p90_queue.append(0)
            p95_queue.append(0)

        # Parallelization per group
        group_busy = defaultdict(float)
        for j in djobs:
            g = j.get("_group", "Other")
            if g not in sh_groups:
                continue
            dur = j.get("duration_seconds")
            if dur and dur > 0:
                group_busy[g] += dur

        for g in sorted(sh_groups):
            parallel = group_busy[g] / 86400
            group_parallel_per_day[g].append(round(parallel, 2))

    # Build datasets: parallelization line + capacity line per group
    colors = ["#0d6efd", "#28a745", "#ffc107", "#dc3545", "#6f42c1", "#fd7e14", "#20c997", "#e83e8c", "#17a2b8", "#6610f2"]
    parallel_datasets = []
    for i, g in enumerate(sorted(sh_groups)):
        color = colors[i % len(colors)]
        parallel_datasets.append({
            "label": g,
            "data": group_parallel_per_day[g],
            "borderColor": color,
            "backgroundColor": color + "33",
            "fill": True,
            "tension": 0.1,
        })
        # Capacity line (not rendered for now, but data is available)
        # rc = sh_groups[g]
        # if rc > 0:
        #     parallel_datasets.append({
        #         "label": f"{g} capacity ({rc})",
        #         "data": [rc] * len(dates),
        #         "borderColor": color,
        #         "borderDash": [4, 4],
        #         "pointRadius": 0,
        #         "fill": False,
        #         "tension": 0,
        #     })

    # Summary table
    table_rows = ""
    for g in sorted(sh_groups):
        vals = group_parallel_per_day[g]
        avg = sum(vals) / len(vals) if vals else 0
        peak = max(vals) if vals else 0
        rc = sh_groups[g]
        rc_str = str(rc) if rc > 0 else "dynamic"
        table_rows += f"<tr><td>{g}</td><td>{rc_str}</td><td>{avg:.1f}</td><td>{peak:.1f}</td></tr>\n"

    body = f"""
<h1>Runner Capacity Analysis</h1>

{chart_section("parallelRate", "Average Concurrent Runners (Parallelization Rate)",
    "Total busy time / 24h per runner group.")}

{chart_section("queueWait", "Queue Wait Time (minutes)")}

<h2>Runner Group Summary</h2>
<table>
  <tr><th>Group</th><th>Fleet Size</th><th>Avg Concurrent</th><th>Peak Concurrent</th></tr>
  {table_rows}
</table>

<script src="{CHARTJS_CDN}"></script>
<script>
{DOWNLOAD_JS}

new Chart(document.getElementById('parallelRate_canvas').getContext('2d'), {{
  type: 'line',
  data: {{
    labels: {json.dumps(dates)},
    datasets: {json.dumps(parallel_datasets)}
  }},
  options: {{responsive:true, scales:{{y:{{min:0,title:{{display:true,text:'Avg Concurrent Runners'}}}}}}}}
}});

new Chart(document.getElementById('queueWait_canvas').getContext('2d'), {{
  type: 'line',
  data: {{
    labels: {json.dumps(dates)},
    datasets: [
      {{label:'Avg', data:{json.dumps(avg_queue_per_day)}, borderColor:'#0d6efd', fill:false, tension:0.1}},
      {{label:'p50', data:{json.dumps(p50_queue)}, borderColor:'#28a745', borderDash:[5,5], fill:false, tension:0.1}},
      {{label:'p90', data:{json.dumps(p90_queue)}, borderColor:'#ffc107', borderDash:[5,5], fill:false, tension:0.1}},
      {{label:'p95', data:{json.dumps(p95_queue)}, borderColor:'#dc3545', borderDash:[5,5], fill:false, tension:0.1}},
    ]
  }},
  options: {{responsive:true, scales:{{y:{{title:{{display:true,text:'Minutes'}}}}}}}}
}});
</script>
"""
    with open(os.path.join(output_dir, "capacity.html"), "w") as f:
        f.write(page_template("Capacity", body, "Capacity"))


# --- Main ---


def main():
    args = parse_args()
    config = load_config()

    # Load job data
    print(f"Loading job data from {args.input}...")
    with open(args.input) as f:
        jobs_data = json.load(f)
    print(f"Loaded {len(jobs_data)} jobs")

    # Process
    data = process_jobs(jobs_data, config)
    print(f"Active jobs (excluding skipped): {len(data['active_jobs'])}")
    print(f"Date range: {data['dates'][0] if data['dates'] else 'N/A'} to {data['dates'][-1] if data['dates'] else 'N/A'}")
    print(f"Months: {len(data['months'])}")

    # Generate output
    os.makedirs(args.output, exist_ok=True)

    print("Generating index.html...")
    generate_index(data, args.output)

    print("Generating statistics.html...")
    generate_statistics(data, args.output)

    print("Generating capacity.html...")
    generate_capacity(data, config, args.output)

    for month in data["months"]:
        print(f"Generating month_{month}.html...")
        generate_month_page(month, data["jobs_by_month"][month], config, args.output)

    print(f"\nDone! Output written to {args.output}/")
    print(f"Open {args.output}/index.html in a browser to view.")


if __name__ == "__main__":
    main()
