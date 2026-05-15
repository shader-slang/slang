#!/usr/bin/env python3
"""
Hosted-runner quota sampler.

Samples in-progress and queued GitHub-hosted runner jobs for a repo and
returns a structured snapshot suitable for the health dashboard.

The Slang org runs on the public-repo 20-concurrent-runner cap, shared
across every hosted-runner label (ubuntu-*, macos-*, windows-*, etc.).
When usage approaches the cap, gating jobs starve and the merge queue
stalls. See shader-slang/slang#11142 for background.

CLI usage:
    python3 ci_hosted_runner_usage.py
    python3 ci_hosted_runner_usage.py --repo shader-slang/slang --cap 20
"""

import argparse
import json
import os
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"

# The Slang org runs on the standard public-repo concurrent-runner cap
# of 20 hosted runners shared across all labels. The cap is per-org,
# not per-label.
DEFAULT_HOSTED_RUNNER_CAP = 20

HOSTED_LABEL_PREFIXES = ("ubuntu-", "macos-", "windows-")


def is_hosted_label(label):
    """Return True if `label` is a GitHub-hosted-runner label.

    GitHub-hosted images all start with one of `ubuntu-`, `macos-`,
    `windows-`. Self-hosted runners carry the `self-hosted` label and
    use NVIDIA-specific labels (`SM80Plus`, `GCP-T4`, etc.).
    """
    if not isinstance(label, str):
        return False
    return label.startswith(HOSTED_LABEL_PREFIXES)


def classify_hosted_label(labels):
    """Pick the canonical hosted label from a job's labels list.

    Returns the first hosted label or None if no hosted label is
    present. The convention `runs-on: [foo]` produces a single-element
    list in practice, but matrix jobs and array syntax can produce
    multiple labels.
    """
    if not labels:
        return None
    # `self-hosted` rules out hosted runners regardless of other labels.
    if any(l == "self-hosted" for l in labels if isinstance(l, str)):
        return None
    for label in labels:
        if is_hosted_label(label):
            return label
    return None


def fetch_in_progress_runs(repo):
    """Fetch all in-progress workflow runs. Returns `(runs, error)`."""
    runs, err = gh_api_list(
        f"repos/{repo}/actions/runs?status=in_progress&per_page=100",
        "workflow_runs",
    )
    if err:
        print(f"Warning: failed to fetch in-progress runs: {err}", file=sys.stderr)
        return [], err
    return runs or [], None


def fetch_jobs_for_run(repo, run_id):
    """Fetch all jobs for a run. Returns (jobs, error)."""
    jobs, err = gh_api_list(
        f"repos/{repo}/actions/runs/{run_id}/jobs?per_page=100",
        "jobs",
    )
    if err:
        return [], err
    return (jobs if isinstance(jobs, list) else []), None


def fetch_queued_runs(repo):
    """Fetch workflow runs whose top-level status is `queued`.

    GitHub's REST API has no `?status=queued&jobs` shortcut, so callers
    list queued runs here and then expand each run to its queued jobs.
    Returns `(runs, error)`.
    """
    runs, err = gh_api_list(
        f"repos/{repo}/actions/runs?status=queued&per_page=100",
        "workflow_runs",
    )
    if err:
        print(f"Warning: failed to fetch queued runs: {err}", file=sys.stderr)
        return [], err
    return runs or [], None


def collect_hosted_jobs(repo, runs, status_filter):
    """Expand workflow runs to their hosted-runner jobs.

    `status_filter` is a single status string (e.g. `"in_progress"`) or
    an iterable of statuses (e.g. `("in_progress", "queued")`). The
    *job* status is what's matched — not the workflow run's top-level
    status, since an `in_progress` run can contain `queued` jobs that
    are stuck waiting for a hosted runner (the textbook cap-exhaustion
    shape).

    Returns a tuple `(results, error_count)`. Each result dict has
    keys: workflow_name, job_name, hosted_label, status. `error_count`
    is the number of runs whose jobs API call failed and whose
    hosted-runner usage is therefore *missing* from `results`. A
    non-zero `error_count` means the sample undercounts real usage.
    """
    if isinstance(status_filter, str):
        wanted = {status_filter}
    else:
        wanted = set(status_filter)

    results = []
    error_count = 0
    if not runs:
        return results, error_count

    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(fetch_jobs_for_run, repo, run["id"]): run
            for run in runs
        }
        for future in as_completed(futures):
            run = futures[future]
            jobs, err = future.result()
            if err:
                print(
                    f"Warning: failed to fetch jobs for run {run.get('id')}: {err}",
                    file=sys.stderr,
                )
                error_count += 1
                continue
            for job in jobs:
                status = job.get("status")
                if status not in wanted:
                    continue
                hosted_label = classify_hosted_label(job.get("labels", []))
                if not hosted_label:
                    continue
                results.append({
                    "workflow_name": run.get("name", ""),
                    "job_name": job.get("name", ""),
                    "hosted_label": hosted_label,
                    "status": status,
                })
    return results, error_count


def summarize(jobs):
    """Group hosted-runner jobs by workflow and by label.

    Returns dict with:
        total: int
        by_workflow: sorted list of {name, count}
        by_label: sorted list of {label, count}
    """
    by_workflow = {}
    by_label = {}
    for j in jobs:
        wf = j["workflow_name"] or "(unknown)"
        lbl = j["hosted_label"]
        by_workflow[wf] = by_workflow.get(wf, 0) + 1
        by_label[lbl] = by_label.get(lbl, 0) + 1
    return {
        "total": len(jobs),
        "by_workflow": sorted(
            ({"name": k, "count": v} for k, v in by_workflow.items()),
            key=lambda x: (-x["count"], x["name"]),
        ),
        "by_label": sorted(
            ({"label": k, "count": v} for k, v in by_label.items()),
            key=lambda x: (-x["count"], x["label"]),
        ),
    }


def sample_hosted_runner_usage(repo, cap=DEFAULT_HOSTED_RUNNER_CAP):
    """Sample current hosted-runner usage for `repo`.

    Returns a dict suitable for embedding in the health snapshot:
        {
            "cap": 20,
            "in_progress": { total, by_workflow, by_label },
            "queued":      { total, by_workflow, by_label },
        }
    """
    in_progress_runs, ip_list_err = fetch_in_progress_runs(repo)
    queued_runs, q_list_err = fetch_queued_runs(repo)

    # A workflow run's top-level status doesn't dictate its jobs' statuses.
    # An `in_progress` run can carry jobs that are still `queued`, waiting
    # for a hosted runner — exactly the cap-exhaustion case we need to
    # detect. So scan jobs from both run sets for both job statuses, then
    # dedupe by run id in case a run transitions between the two list
    # endpoints during sampling.
    seen = set()
    deduped_runs = []
    for run in list(in_progress_runs) + list(queued_runs):
        rid = run.get("id")
        if rid is None or rid in seen:
            continue
        seen.add(rid)
        deduped_runs.append(run)

    all_jobs, fetch_errors = collect_hosted_jobs(
        repo, deduped_runs, ("in_progress", "queued")
    )
    in_progress_jobs = [j for j in all_jobs if j["status"] == "in_progress"]
    queued_jobs = [j for j in all_jobs if j["status"] == "queued"]

    result = {
        "cap": cap,
        "in_progress": summarize(in_progress_jobs),
        "queued": summarize(queued_jobs),
    }
    # Mark partial if either the per-run job fetches or the top-level
    # run listings failed. A listing failure is more dangerous: it
    # erases an entire status's contribution rather than one run's,
    # so a transient 5xx on `?status=queued` must not look healthy.
    if fetch_errors or ip_list_err or q_list_err:
        result["partial"] = True
    if fetch_errors:
        result["fetch_errors"] = fetch_errors
    if ip_list_err or q_list_err:
        result["list_errors"] = [
            e for e in (ip_list_err, q_list_err) if e
        ]
    return result


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Sample GitHub-hosted runner usage for a repo, broken down "
            "by workflow and label. Aimed at detecting impending "
            "20-runner-cap exhaustion before it stalls the merge queue."
        )
    )
    parser.add_argument(
        "--repo",
        default=DEFAULT_REPO,
        help=f"Repository (default: {DEFAULT_REPO})",
    )
    parser.add_argument(
        "--cap",
        type=int,
        default=DEFAULT_HOSTED_RUNNER_CAP,
        help=(
            f"Hosted-runner concurrency cap to report against "
            f"(default: {DEFAULT_HOSTED_RUNNER_CAP}, the standard public-repo limit)"
        ),
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output the raw JSON snapshot instead of a human-readable summary.",
    )
    return parser.parse_args()


def format_summary(snapshot):
    """Render a one-screen human-readable summary."""
    cap = snapshot["cap"]
    in_use = snapshot["in_progress"]["total"]
    queued = snapshot["queued"]["total"]
    pct = (in_use / cap * 100) if cap else 0

    lines = []
    lines.append(
        f"Hosted runners in use: {in_use} / {cap} ({pct:.0f}%)   "
        f"queued: {queued}"
    )
    if snapshot.get("partial"):
        lines.append(
            "WARNING: sample is partial and may undercount hosted-runner usage."
        )
        if snapshot.get("fetch_errors"):
            lines.append(f"  job fetch failures: {snapshot['fetch_errors']}")
        for err in snapshot.get("list_errors", []):
            lines.append(f"  list failure: {err}")
    lines.append("")
    if snapshot["in_progress"]["by_label"]:
        lines.append("In use by label:")
        for row in snapshot["in_progress"]["by_label"]:
            lines.append(f"  {row['count']:>3}  {row['label']}")
        lines.append("")
    if snapshot["in_progress"]["by_workflow"]:
        lines.append("In use by workflow (top 5):")
        for row in snapshot["in_progress"]["by_workflow"][:5]:
            lines.append(f"  {row['count']:>3}  {row['name']}")
        lines.append("")
    if snapshot["queued"]["by_workflow"]:
        lines.append("Queued by workflow (top 5):")
        for row in snapshot["queued"]["by_workflow"][:5]:
            lines.append(f"  {row['count']:>3}  {row['name']}")
    return "\n".join(lines)


def main():
    args = parse_args()
    snapshot = sample_hosted_runner_usage(args.repo, cap=args.cap)
    if args.json:
        json.dump(snapshot, sys.stdout, indent=2)
        sys.stdout.write("\n")
    else:
        print(format_summary(snapshot))


if __name__ == "__main__":
    main()
