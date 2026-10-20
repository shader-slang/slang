#!/usr/bin/env python3
"""Fail the nightly on TSan reports or incomplete test execution."""

import os
from pathlib import Path
import re
import sys


TSAN_DIAGNOSTICS = (
    "FATAL: ThreadSanitizer",
    "WARNING: ThreadSanitizer",
    "ThreadSanitizer: CHECK failed",
)


def summarize(log_dir, env):
    """Check reports independently of exits: test servers can absorb child exits."""
    failures = []
    if env.get("BUILD_OUTCOME") != "success":
        failures.append("The instrumented build did not complete.")

    for variable, label in (
        ("TSAN_CONCURRENT_EXIT", "Concurrent compilation"),
        ("SLANG_TEST_EXIT", "slang-test"),
        ("SLANGC_TEST_EXIT", "slangc tests"),
    ):
        status = env.get(variable)
        if status != "0":
            failures.append(f"{label}: exit status {status or 'missing (not completed)'}.")

    for name in ("concurrent-compile.txt", "slang-test.txt"):
        path = log_dir / name
        output = path.read_text(errors="replace") if path.exists() else ""
        totals = re.findall(r"\d+% of tests passed \((\d+)/(\d+)\)", output)
        if not totals or int(totals[-1][1]) == 0:
            failures.append(f"{name}: no completed, nonempty test run.")
        elif name == "concurrent-compile.txt" and int(totals[-1][0]) == 0:
            failures.append("Concurrent compilation did not pass any tests.")
        # Some fatal runtime diagnostics go to stderr rather than log_path.
        if any(marker in output for marker in TSAN_DIAGNOSTICS):
            failures.append(f"{name}: TSan diagnostic on stderr.")

    path = log_dir / "slangc-test.txt"
    output = path.read_text(errors="replace") if path.exists() else ""
    totals = re.findall(r"(\d+) failed, out of (\d+) tests", output)
    if not totals or totals[-1][0] != "0" or int(totals[-1][1]) == 0:
        failures.append("slangc-test.txt: no completed, successful test run.")
    if any(marker in output for marker in TSAN_DIAGNOSTICS):
        failures.append("slangc-test.txt: TSan diagnostic on stderr.")

    reports = sorted(p for p in log_dir.glob("tsan.*") if p.is_file() and p.stat().st_size)
    if reports:
        failures.append(f"{len(reports)} nonempty TSan report file(s).")
    lines = ["## ThreadSanitizer report", ""]
    lines += [f"- {failure}" for failure in failures]
    if not failures:
        lines.append("Concurrent compilation and both test suites completed without TSan reports.")
    lines += ["", "Download `tsan-logs` for reports and test output."]
    return failures, reports, "\n".join(lines) + "\n"


def main():
    failures, reports, summary = summarize(Path(sys.argv[1]), os.environ)
    print(summary)
    for report in reports:
        print(f"::group::{report.name}")
        print(report.read_text(errors="replace"))
        print("::endgroup::")
    for failure in failures:
        print(f"::error::{failure}")
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as stream:
            stream.write(summary)
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())
