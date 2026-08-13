#!/usr/bin/env python3
"""Classify one compile-perf nightly into the icon + sentence Slack shows.

The Slack step has three signals to work from — the trend step's outcome, the
job's overall status, and the warning count — and has to turn them into a
single line a human reads at a glance. That mapping lived in the workflow as a
five-branch bash ladder, where nothing could test it: the branch ORDER is
load-bearing (see classify), and getting it wrong produces a plausible message
rather than an error, on a path that only runs on a scheduled nightly.

It lives here instead of in trend.py because the Slack step has to classify
runs where trend.py never executed: the trend step is gated on
`PUBLISH == 'true'`, and "the trend check did not run" is one of the states
being reported.

    python3 slack_status.py            # reads the env vars the workflow sets
    python3 slack_status.py --field icon
"""
import argparse
import os
import sys

# (icon, status) for each outcome. Kept as constants so the self-checks below
# name states rather than repeat message text, and a wording change does not
# silently turn into a classification change.
#
# The icons are ordered by severity and deliberately match trend.py's
# step-summary header for the same data (🔴 regression, ⚠️ warning tier): a
# reader moving from the Slack message to the CI summary should not have to
# re-learn the palette. Note the regression icon is RED, not the yellow
# triangle — a job-failing >=10% regression must not read as milder than the
# 5-10% warning tier, which is what a yellow triangle beside a yellow circle
# conveys.
REGRESSION = (":red_circle:",
              "Regression detected (>=10% over trailing median) — see CI run "
              "for details")
JOB_FAILED = (":x:", "Nightly job failed — see CI run for details")
CLEAN = (":white_check_mark:", "No regressions detected")
NOT_RUN = (":information_source:",
           "Trend check did not run — see CI run for details")


def warnings_status(n):
    """The yellow warning-tier line for `n` warning-level changes."""
    return (":large_yellow_circle:",
            f"{n} warning-level change(s) (>=5%, below the 10% error gate) "
            f"— see CI summary")


def classify(trend_outcome, job_status, warnings):
    """Return the ``(icon, status)`` pair for one nightly.

    `trend_outcome` is the trend step's GitHub Actions outcome ("success",
    "failure", or "skipped"), `job_status` is the job's overall status, and
    `warnings` is the count trend.py wrote to GITHUB_OUTPUT (0 when it wrote
    nothing, via the workflow's `|| '0'` fallback).

    The branch ORDER is the part worth protecting, and it is why this is a
    function rather than a dict lookup:

    1. A regression is classified FIRST. trend.py exits non-zero on one, which
       also drives job.status to failure — so testing job_status first would
       shadow every regression as a generic "job failed". That single
       reordering is the failure this module exists to make testable.
    2. job_status then separates "the trend step never ran because an earlier
       step failed" from "it never ran because this is a publish=false
       measurement-only run".
    3. Warnings only outrank a clean report, never a regression: a night with
       both is a regression night.
    4. Anything left is the trend step not having run while the job stayed
       green — its gate makes the outcome "skipped", which matches nothing
       above. Phrased generically so the message stays true if that gate
       changes.
    """
    if trend_outcome == "failure":
        return REGRESSION
    if job_status != "success":
        return JOB_FAILED
    if trend_outcome == "success" and warnings:
        return warnings_status(warnings)
    if trend_outcome == "success":
        return CLEAN
    return NOT_RUN


def _warnings_from_env(raw):
    """Parse TREND_WARNINGS. Anything unparseable counts as "some warnings"
    rather than zero: the count only decides between the yellow and green
    lines, and reporting green because a number could not be parsed is the
    one wrong answer — it hides the state the key exists to surface."""
    try:
        return int(raw or 0)
    except ValueError:
        print(f"::warning title=Slack status::could not parse "
              f"TREND_WARNINGS={raw!r}; treating as non-zero", file=sys.stderr)
        return 1


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--field", choices=["icon", "status"], default=None,
                    help="print just this field (default: icon then status, "
                         "one per line)")
    args = ap.parse_args()
    icon, status = classify(os.environ.get("TREND_OUTCOME", ""),
                            os.environ.get("JOB_STATUS", ""),
                            _warnings_from_env(os.environ.get("TREND_WARNINGS")))
    if args.field == "icon":
        print(icon)
    elif args.field == "status":
        print(status)
    else:
        print(icon)
        print(status)


# Import-time self-checks (the directory idiom, run by check-python-core.yml).
# Every branch, and above all the ordering: these are the assertions the bash
# ladder could not have.
assert classify("success", "success", 0) == CLEAN
assert classify("success", "success", 3)[0] == ":large_yellow_circle:"
assert "3 warning-level" in classify("success", "success", 3)[1]
assert classify("skipped", "success", 0) == NOT_RUN
assert classify("failure", "failure", 0) == REGRESSION
assert classify("", "failure", 0) == JOB_FAILED

# THE ordering invariant: a regression drives job.status to failure too, so a
# ladder that tested job_status first would report every regression as a
# generic job failure. Both of these must stay REGRESSION.
assert classify("failure", "failure", 0) == REGRESSION, \
    "a regression must outrank the generic job-failure case, not be shadowed by it"
assert classify("failure", "failure", 5) == REGRESSION, \
    "a regression with warnings alongside it is still a regression"

# Warnings outrank clean but never a regression, and zero warnings is green
# rather than a yellow "0 warning-level change(s)".
assert classify("success", "success", 1)[0] == ":large_yellow_circle:"
assert classify("success", "success", 0)[0] == ":white_check_mark:"

# Severity ordering is a contract, not decoration. The job-failing regression
# must be visually stronger than the warning tier and must agree with
# trend.py's step-summary header (🔴 / ⚠️) for the same data. Pinned because
# the earlier yellow-triangle regression icon read as MILDER than the yellow
# circle beside it, and nothing would catch that being reintroduced.
assert REGRESSION[0] == ":red_circle:", \
    ("a job-failing regression must not use a yellow icon: beside the warning "
     "tier's yellow circle it reads as the lesser alert")
assert REGRESSION[0] != warnings_status(1)[0] != CLEAN[0], \
    "regression, warning and clean must be visually distinguishable at a glance"

# A skipped trend step on a failed job is a job failure, not "did not run":
# the failure is the more actionable of the two.
assert classify("skipped", "failure", 0) == JOB_FAILED

# Unparseable counts never resolve to green. The bad-input case is run with
# stderr captured: _warnings_from_env emits a ::warning:: annotation, and
# letting the fixture emit it would put a spurious annotation on every CI run
# that merely imports this module.
assert _warnings_from_env("2") == 2
assert _warnings_from_env(None) == 0
assert _warnings_from_env("") == 0
_buf = __import__("io").StringIO()
with __import__("contextlib").redirect_stderr(_buf):
    _bad = _warnings_from_env("not-a-number")
assert _bad != 0, "an unparseable warning count must not read as zero warnings"
assert "could not parse" in _buf.getvalue(), \
    "an unparseable warning count must still be reported, not silently coerced"
del _buf, _bad


if __name__ == "__main__":
    main()
