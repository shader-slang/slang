#!/usr/bin/env python3
"""Wait for a coverage report commit to deploy and appear on the public site."""

import argparse
import json
import re
import time
from urllib.error import URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen


API = "https://api.github.com/repos/shader-slang/slang-coverage-reports"
SITE = "https://shader-slang.org/slang-coverage-reports/reports/publication.json"


def read_json(url):
    # These are public read-only endpoints. No publishing credential is needed.
    request = Request(url, headers={
        "Accept": "application/json",
        "User-Agent": "slang-coverage-publication-check",
        "Cache-Control": "no-cache",
    })
    with urlopen(request, timeout=20) as response:
        return json.load(response)


def wait_for_pages(report_sha, timeout=1200, interval=60, fetch=read_json,
                   now=time.monotonic, sleep=time.sleep):
    """Require the exact pushed commit, not a previous successful deployment."""
    if not re.fullmatch(r"[0-9a-f]{40}", report_sha):
        raise ValueError("A full report commit SHA is required")
    if timeout <= 0 or interval <= 0:
        raise ValueError("Timeout and polling interval must be positive")
    deadline = now() + timeout
    query = urlencode({"head_sha": report_sha, "per_page": 5})
    runs_url = f"{API}/actions/workflows/pages.yml/runs?{query}"
    previous = None
    detail = "No Pages run found for the pushed report commit"
    while now() < deadline:
        try:
            runs = fetch(runs_url)["workflow_runs"]
            runs = [run for run in runs if run["head_sha"] == report_sha]
            if runs:
                run = max(runs, key=lambda item: (item["id"], item.get("run_attempt", 1)))
                detail = f'Pages run {run["html_url"]}: {run["status"]}'
                if run["status"] == "completed":
                    if run["conclusion"] != "success":
                        raise RuntimeError(f'{detail} with {run["conclusion"]}')
                    marker_query = urlencode({"report_commit": report_sha, "check": int(now())})
                    marker = fetch(f"{SITE}?{marker_query}")
                    if marker.get("report_commit") == report_sha:
                        print(f"Coverage report {report_sha} is deployed and served: {SITE}")
                        return run["html_url"]
                    detail += "; the served publication marker still identifies an older report"
            else:
                detail = "No Pages run found for the pushed report commit"
        except (URLError, OSError, ValueError, KeyError) as error:
            detail = f"Cannot verify publication yet: {error}"
        if detail != previous:
            print(detail, flush=True)
            previous = detail
        sleep(max(0, min(interval, deadline - now())))
    raise TimeoutError(f"Coverage publication was not verified within {timeout}s. {detail}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report-sha", required=True)
    parser.add_argument("--timeout", type=int, default=1200)
    parser.add_argument("--interval", type=int, default=60)
    args = parser.parse_args()
    try:
        wait_for_pages(args.report_sha, args.timeout, args.interval)
    except (ValueError, RuntimeError, TimeoutError) as error:
        # Escape annotation control characters if an upstream response has them.
        message = str(error).replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
        parser.exit(1, f"::error::{message}\n")


if __name__ == "__main__":
    main()
