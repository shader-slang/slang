#!/usr/bin/env python3
"""
Analyze GitHub Actions CI run parallelization bottlenecks.

Usage:
    # Using gh CLI directly (recommended)
    gh api repos/OWNER/REPO/actions/runs/RUN_ID/jobs | python3 analyze-ci-parallelization.py

    # Or save jobs data to file first
    gh api repos/OWNER/REPO/actions/runs/RUN_ID/jobs > jobs.json
    python3 analyze-ci-parallelization.py jobs.json

    # Example
    gh api repos/shader-slang/slang/actions/runs/21618930965/jobs | python3 analyze-ci-parallelization.py
"""

import json
import sys
from datetime import datetime
from typing import List, Dict, Any
from collections import defaultdict


def parse_jobs(jobs_data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Parse job data and calculate durations."""
    parsed_jobs = []

    for job in jobs_data:
        # Skip jobs that haven't started or completed
        if not job.get("started_at") or not job.get("completed_at"):
            continue

        # Skip skipped jobs
        if job.get("conclusion") == "skipped":
            continue

        start = datetime.fromisoformat(job["started_at"].replace("Z", "+00:00"))
        end = datetime.fromisoformat(job["completed_at"].replace("Z", "+00:00"))
        duration = (end - start).total_seconds() / 60

        parsed_jobs.append({
            "name": job["name"],
            "started_at": job["started_at"],
            "completed_at": job["completed_at"],
            "start_time": start,
            "end_time": end,
            "duration_min": duration,
            "runner_name": job.get("runner_name", "Unknown"),
            "conclusion": job.get("conclusion", "unknown"),
        })

    return parsed_jobs


def analyze_workflow(jobs: List[Dict[str, Any]]) -> None:
    """Perform comprehensive parallelization analysis."""
    if not jobs:
        print("Error: No jobs to analyze")
        return

    # Calculate workflow metrics
    workflow_start = min(j["start_time"] for j in jobs)
    workflow_end = max(j["end_time"] for j in jobs)
    total_duration = (workflow_end - workflow_start).total_seconds() / 60
    total_cpu_time = sum(j["duration_min"] for j in jobs)

    print("=" * 100)
    print("CI RUN PARALLELIZATION ANALYSIS")
    print("=" * 100)
    print(f"\nWorkflow Duration: {total_duration:.1f} minutes")
    print(f"Start: {workflow_start.strftime('%Y-%m-%d %H:%M:%S UTC')}")
    print(f"End: {workflow_end.strftime('%Y-%m-%d %H:%M:%S UTC')}")
    print(f"\nTotal Jobs: {len(jobs)}")
    print(f"Total CPU Time: {total_cpu_time:.1f} minutes")
    print(f"Parallelization Efficiency: {(total_cpu_time / total_duration):.1f}x")

    # Check for failed jobs
    failed_jobs = [j for j in jobs if j["conclusion"] not in ("success", "skipped")]
    if failed_jobs:
        print(f"\n⚠️  Failed Jobs: {len(failed_jobs)}")
        for job in failed_jobs:
            print(f"   - {job['name']} ({job['conclusion']})")

    # Analyze longest running jobs
    print("\n" + "=" * 100)
    print("LONGEST RUNNING JOBS (Critical Path Candidates)")
    print("=" * 100)
    sorted_by_duration = sorted(jobs, key=lambda x: x["duration_min"], reverse=True)
    for i, job in enumerate(sorted_by_duration[:10], 1):
        status = "✓" if job["conclusion"] == "success" else "✗"
        print(f"{status} {i:2d}. {job['name']:60s} {job['duration_min']:6.1f} min")

    # Analyze job dependencies by grouping related jobs
    analyze_job_chains(jobs, workflow_start)

    # Analyze runner utilization
    analyze_runner_utilization(jobs, workflow_start, total_duration)

    # Provide recommendations
    provide_recommendations(jobs, total_duration, total_cpu_time)


def analyze_job_chains(jobs: List[Dict[str, Any]], workflow_start: datetime) -> None:
    """Identify sequential job chains (build → test patterns)."""
    print("\n" + "=" * 100)
    print("SEQUENTIAL JOB CHAINS (Build → Test Dependencies)")
    print("=" * 100)

    # Group jobs by platform/configuration prefix
    chains = defaultdict(list)
    for job in jobs:
        # Extract platform identifier (everything before the last " / ")
        parts = job["name"].split(" / ")
        if len(parts) >= 2:
            platform = parts[0]
            chains[platform].append(job)
        else:
            chains["other"].append(job)

    # Analyze each chain
    for platform, platform_jobs in sorted(chains.items()):
        if len(platform_jobs) <= 1:
            continue

        platform_jobs.sort(key=lambda x: x["start_time"])
        total_work = sum(j["duration_min"] for j in platform_jobs)
        elapsed = (platform_jobs[-1]["end_time"] - platform_jobs[0]["start_time"]).total_seconds() / 60

        # Calculate potential parallelization opportunity
        if elapsed > total_work:
            gap = elapsed - total_work
            print(f"\n{platform}:")
            print(f"  Jobs: {len(platform_jobs)}, Total work: {total_work:.1f} min, Elapsed: {elapsed:.1f} min")
            print(f"  🔍 Gap: {gap:.1f} min (potential wait time)")
        else:
            print(f"\n{platform}:")
            print(f"  Jobs: {len(platform_jobs)}, Total work: {total_work:.1f} min, Elapsed: {elapsed:.1f} min")

        for job in platform_jobs:
            rel_start = (job["start_time"] - workflow_start).total_seconds() / 60
            stage = job["name"].split(" / ")[-1] if " / " in job["name"] else job["name"]
            status = "✓" if job["conclusion"] == "success" else "✗"
            print(f"    {status} +{rel_start:5.1f}m: {stage:40s} ({job['duration_min']:5.1f}m)")


def analyze_runner_utilization(jobs: List[Dict[str, Any]], workflow_start: datetime, total_duration: float) -> None:
    """Analyze how efficiently runners are being used."""
    print("\n" + "=" * 100)
    print("RUNNER UTILIZATION")
    print("=" * 100)

    runners = defaultdict(list)
    for job in jobs:
        runner = job["runner_name"]
        runners[runner].append(job)

    print(f"\nTotal runners used: {len(runners)}")

    # Sort by total work time
    runner_stats = []
    for runner, runner_jobs in runners.items():
        runner_jobs.sort(key=lambda x: x["start_time"])
        total_work = sum(j["duration_min"] for j in runner_jobs)

        # Calculate idle time (gaps between jobs)
        idle_time = 0
        for i in range(len(runner_jobs) - 1):
            gap = (runner_jobs[i+1]["start_time"] - runner_jobs[i]["end_time"]).total_seconds() / 60
            if gap > 0:
                idle_time += gap

        # Calculate utilization if runner was active throughout workflow
        first_start = runner_jobs[0]["start_time"]
        last_end = runner_jobs[-1]["end_time"]
        runner_lifetime = (last_end - first_start).total_seconds() / 60
        utilization = (total_work / runner_lifetime * 100) if runner_lifetime > 0 else 0

        runner_stats.append({
            "name": runner,
            "jobs": runner_jobs,
            "total_work": total_work,
            "idle_time": idle_time,
            "utilization": utilization,
        })

    runner_stats.sort(key=lambda x: x["total_work"], reverse=True)

    for stat in runner_stats[:10]:  # Show top 10 busiest runners
        runner = stat["name"]
        runner_jobs = stat["jobs"]
        total_work = stat["total_work"]
        idle_time = stat["idle_time"]
        utilization = stat["utilization"]

        print(f"\n{runner}:")
        print(f"  Jobs: {len(runner_jobs)}, Total work: {total_work:.1f} min, Idle: {idle_time:.1f} min, Utilization: {utilization:.0f}%")

        if len(runner_jobs) > 3:
            print(f"  ⚠️  Runner handles {len(runner_jobs)} sequential jobs - potential bottleneck")

        for job in runner_jobs:
            rel_start = (job["start_time"] - workflow_start).total_seconds() / 60
            status = "✓" if job["conclusion"] == "success" else "✗"
            print(f"    {status} +{rel_start:5.1f}m: {job['name']:55s} ({job['duration_min']:5.1f}m)")


def provide_recommendations(jobs: List[Dict[str, Any]], total_duration: float, total_cpu_time: float) -> None:
    """Provide optimization recommendations based on analysis."""
    print("\n" + "=" * 100)
    print("OPTIMIZATION RECOMMENDATIONS")
    print("=" * 100)

    parallelization_efficiency = total_cpu_time / total_duration

    # Check overall efficiency
    if parallelization_efficiency < 5:
        print("\n🔴 LOW PARALLELIZATION EFFICIENCY")
        print(f"   Current: {parallelization_efficiency:.1f}x")
        print(f"   Many jobs are running sequentially. Consider:")
        print("   - Reviewing job dependencies to allow more parallel execution")
        print("   - Adding more runners to handle parallel workloads")
    elif parallelization_efficiency < 10:
        print("\n🟡 MODERATE PARALLELIZATION EFFICIENCY")
        print(f"   Current: {parallelization_efficiency:.1f}x")
        print("   Some optimization possible. Consider:")
        print("   - Identifying sequential bottlenecks")
        print("   - Splitting long-running jobs")
    else:
        print("\n🟢 GOOD PARALLELIZATION EFFICIENCY")
        print(f"   Current: {parallelization_efficiency:.1f}x")

    # Identify long-running jobs
    long_jobs = [j for j in jobs if j["duration_min"] > 20]
    if long_jobs:
        print(f"\n⏱️  LONG-RUNNING JOBS ({len(long_jobs)} jobs over 20 minutes)")
        for job in sorted(long_jobs, key=lambda x: x["duration_min"], reverse=True):
            print(f"   - {job['name']}: {job['duration_min']:.1f} min")
        print("   Consider splitting these jobs or optimizing their workload")

    # Identify runner bottlenecks
    runners = defaultdict(list)
    for job in jobs:
        runners[job["runner_name"]].append(job)

    busy_runners = [r for r, jobs in runners.items() if len(jobs) >= 3]
    if busy_runners:
        print(f"\n🔧 RUNNER BOTTLENECKS ({len(busy_runners)} runners with 3+ sequential jobs)")
        for runner in busy_runners:
            job_count = len(runners[runner])
            total_work = sum(j["duration_min"] for j in runners[runner])
            print(f"   - {runner}: {job_count} jobs, {total_work:.1f} min")
        print("   Consider adding more runners or redistributing work")

    print("\n" + "=" * 100)


def main():
    """Main entry point."""
    # Read input from stdin or file
    if len(sys.argv) > 1:
        # Read from file
        with open(sys.argv[1], 'r') as f:
            data = json.load(f)
    else:
        # Read from stdin
        try:
            data = json.load(sys.stdin)
        except json.JSONDecodeError as e:
            print(f"Error: Invalid JSON input: {e}", file=sys.stderr)
            print(__doc__, file=sys.stderr)
            sys.exit(1)

    # Handle different input formats
    if isinstance(data, dict) and "jobs" in data:
        jobs_data = data["jobs"]
    elif isinstance(data, list):
        jobs_data = data
    else:
        print("Error: Expected JSON with 'jobs' array or array of jobs", file=sys.stderr)
        sys.exit(1)

    # Parse and analyze
    jobs = parse_jobs(jobs_data)

    if not jobs:
        print("Error: No valid jobs found in input", file=sys.stderr)
        sys.exit(1)

    analyze_workflow(jobs)


if __name__ == "__main__":
    main()
