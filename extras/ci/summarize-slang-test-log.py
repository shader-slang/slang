#!/usr/bin/env python3
"""Summarize slang-test output for stress and CI diagnostics."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


def summarize_log(text: str) -> dict:
    failed_tests = re.findall(r"FAILED test: '([^']+)'", text)
    json_rpc_lines = re.findall(r"JSON RPC failure: ([^\r\n]+)", text)
    summary_matches = list(
        re.finditer(
            r"(?P<percent>\d+)% of tests passed "
            r"\((?P<passed>\d+)/(?P<total>\d+)\), "
            r"(?P<ignored>\d+) tests ignored"
            r"(?:, (?P<expected_failed>\d+) tests failed expectedly)?",
            text,
        )
    )
    summary_match = summary_matches[-1] if summary_matches else None

    summary = {
        "failed_test_count": len(failed_tests),
        "failed_tests": failed_tests,
        "json_rpc_failure_count": len(json_rpc_lines),
        "json_rpc_wait_for_result_count": sum(
            1 for line in json_rpc_lines if "waitForResult()" in line
        ),
        "json_rpc_has_message_count": sum(
            1 for line in json_rpc_lines if "hasMessage()" in line
        ),
        "too_many_failed_tests": "Too many failed tests" in text,
        "stopped_after_consecutive_failures": (
            "Stopped scheduling new tests after too many consecutive failures" in text
        ),
        "pass_percent": None,
        "passed_tests": None,
        "total_tests": None,
        "ignored_tests": None,
        "expected_failed_tests": None,
    }

    if summary_match:
        summary.update(
            {
                "pass_percent": int(summary_match.group("percent")),
                "passed_tests": int(summary_match.group("passed")),
                "total_tests": int(summary_match.group("total")),
                "ignored_tests": int(summary_match.group("ignored")),
                "expected_failed_tests": int(
                    summary_match.group("expected_failed") or 0
                ),
            }
        )

    return summary


def write_markdown(path: Path, summary: dict) -> None:
    failed_tests = summary["failed_tests"]
    lines = [
        "## slang-test summary",
        "",
        f"- Failed tests: {summary['failed_test_count']}",
        f"- JSON RPC failures: {summary['json_rpc_failure_count']}",
        f"- JSON RPC waitForResult: {summary['json_rpc_wait_for_result_count']}",
        f"- JSON RPC hasMessage: {summary['json_rpc_has_message_count']}",
        f"- Too many failed tests: {summary['too_many_failed_tests']}",
        f"- Stopped after consecutive failures: {summary['stopped_after_consecutive_failures']}",
    ]

    if summary["total_tests"] is not None:
        lines.extend(
            [
                f"- Passed tests: {summary['passed_tests']} / {summary['total_tests']}",
                f"- Ignored tests: {summary['ignored_tests']}",
                f"- Expected failed tests: {summary['expected_failed_tests']}",
            ]
        )

    if failed_tests:
        lines.extend(["", "### Failed Tests", ""])
        lines.extend(f"- `{test}`" for test in failed_tests[:100])
        if len(failed_tests) > 100:
            lines.append(f"- ... {len(failed_tests) - 100} more")

    with path.open("a", encoding="utf-8") as f:
        f.write("\n".join(lines))
        f.write("\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--json", type=Path, required=True)
    parser.add_argument("--markdown", type=Path)
    args = parser.parse_args()

    if args.log.exists():
        text = args.log.read_text(encoding="utf-8", errors="replace")
    else:
        text = ""

    summary = summarize_log(text)
    args.json.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    if args.markdown:
        write_markdown(args.markdown, summary)

    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
