#!/usr/bin/env python3
"""Observe an L0 candidate without changing slang-test or required CI checks."""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time


def metadata():
    repository = os.environ.get("GITHUB_REPOSITORY", "")
    run_id = os.environ.get("GITHUB_RUN_ID", "")
    return {
        "schema_version": 1,
        "sha": os.environ.get("GITHUB_SHA"),
        "event": os.environ.get("GITHUB_EVENT_NAME"),
        "run_id": run_id,
        "run_attempt": os.environ.get("GITHUB_RUN_ATTEMPT"),
        "runner": os.environ.get("RUNNER_NAME"),
        "run_url": f"{os.environ.get('GITHUB_SERVER_URL', 'https://github.com')}/{repository}/actions/runs/{run_id}",
    }


def execute(command, log_path, timeout):
    started = time.monotonic()
    result = {"command": command, "started_at": datetime.datetime.now(datetime.timezone.utc).isoformat()}
    with log_path.open("w", encoding="utf-8") as log:
        try:
            process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            try:
                result["exit_code"] = process.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                # Kill the test servers as well as the parent before returning.
                os.killpg(process.pid, signal.SIGKILL)
                result["exit_code"] = process.wait()
                result["error"] = "timeout"
        except OSError as error:
            result.update(exit_code=None, error=str(error))
            log.write(f"Could not start command: {error}\n")
    result["elapsed_seconds"] = round(time.monotonic() - started, 3)
    print(log_path.read_text(encoding="utf-8", errors="replace"), end="", flush=True)
    return result


def parse_summary(log):
    matches = list(re.finditer(r"^\d+% of tests passed \((\d+)/(\d+)\)(.*)$", log, re.MULTILINE))
    if not matches:
        return None
    match = matches[-1]
    passed, total = int(match[1]), int(match[2])
    suffix = match[3]

    def count(pattern):
        found = re.search(pattern, suffix)
        return int(found[1]) if found else 0

    expected = count(r"(\d+) tests failed expectedly")
    if total < passed + expected:
        return None
    return {
        "passed": passed,
        "executed": total,
        "expected_failed": expected,
        "unexpected_failed": total - passed - expected,
        "ignored": count(r"(\d+) tests ignored"),
        "dispatch_failures": count(r"(\d+) test-server dispatch failure"),
    }


def save(directory, result):
    (directory / "result.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")


def run(args):
    directory = args.output_dir
    directory.mkdir(parents=True, exist_ok=True)
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        raise SystemExit("A slang-test command is required after --")
    result = metadata()
    result["dry_run"] = execute(command + ["-dry-run"], directory / "dry-run.log", args.discovery_timeout)
    candidates = [line.strip() for line in (directory / "dry-run.log").read_text().splitlines()
                  if line.startswith(("tests/", "slang-unit-test-tool/", "gfx-unit-test-tool/"))]
    (directory / "candidates.txt").write_text("\n".join(candidates) + ("\n" if candidates else ""), encoding="utf-8")
    # Dry-run enumeration precedes runtime API filtering in slang-test.
    result["candidate_count_before_runtime_filtering"] = len(candidates)
    result["test_run"] = execute(command, directory / "test.log", args.test_timeout)
    result["counts"] = parse_summary((directory / "test.log").read_text(encoding="utf-8", errors="replace"))
    if any(part.get("error") for part in (result["dry_run"], result["test_run"])):
        result["status"] = "infrastructure_failure"
    elif result["test_run"]["exit_code"] != 0:
        result["status"] = "failed"
    elif result["dry_run"]["exit_code"] != 0 or not candidates or not result["counts"] or not result["counts"]["executed"]:
        result["status"] = "incomplete"
    elif result["counts"]["unexpected_failed"] or result["counts"]["dispatch_failures"]:
        result["status"] = "failed"
    else:
        result["status"] = "passed"
    save(directory, result)
    return 0 if result["status"] == "passed" else 1


def report(args):
    directory = args.output_dir
    directory.mkdir(parents=True, exist_ok=True)
    path = directory / "result.json"
    result = json.loads(path.read_text()) if path.exists() else metadata()
    result["step_outcomes"] = {name: os.environ.get(f"{name.upper()}_OUTCOME", "unknown")
                               for name in ("setup", "test", "cleanliness")}
    if "status" not in result:
        result["status"] = "infrastructure_failure"
        result["reason"] = "Test observation was not produced; inspect setup and test step outcomes."
    if result["step_outcomes"]["cleanliness"] == "failure":
        result["status"] = "failed"
        result["reason"] = "The test left generated or modified files in the checkout."
    save(directory, result)
    lines = ["## L0 CPU shadow observation", "", "Advisory: full CI remains required.", "",
             f"- Observation: **{result['status']}**", f"- Revision: `{result.get('sha')}`",
             f"- Step outcomes: `{result['step_outcomes']}`"]
    if "reason" in result:
        lines.append(f"- Reason: {result['reason']}")
    if "candidate_count_before_runtime_filtering" in result:
        lines.append(f"- Candidates before runtime API filtering: {result['candidate_count_before_runtime_filtering']}")
    if result.get("counts"):
        lines.append(f"- Actual test counts: `{result['counts']}`")
    for name in ("dry_run", "test_run"):
        if name in result:
            lines.append(f"- {name}: {result[name]['elapsed_seconds']} seconds; exit `{result[name]['exit_code']}`")
    lines.extend(["", "Download the l0-shadow artifact for commands, candidate names, raw logs, and result.json.",
                  "Compare against the full CPU job in this workflow run at the same revision.", ""])
    summary = "\n".join(lines)
    (directory / "summary.md").write_text(summary, encoding="utf-8")
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as stream:
            stream.write(summary)
    print(summary)
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="mode", required=True)
    runner = commands.add_parser("run")
    runner.add_argument("--output-dir", type=Path, required=True)
    runner.add_argument("--discovery-timeout", type=float, default=120)
    runner.add_argument("--test-timeout", type=float, default=600)
    runner.add_argument("command", nargs=argparse.REMAINDER)
    reporter = commands.add_parser("report")
    reporter.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()
    return run(args) if args.mode == "run" else report(args)


if __name__ == "__main__":
    sys.exit(main())
