#!/usr/bin/env python3
"""Classify one compile-perf nightly into the icon + sentence Slack shows.

The Slack step has four signals to work from — the trend step's outcome and
exit code, the job's overall status, and the warning count — and has to turn
them into a single line a human reads at a glance. That mapping lived in the
workflow as a five-branch bash ladder, where nothing could test it: the branch
ORDER is load-bearing (see classify), and getting it wrong produces a plausible
message rather than an error, on a path that only runs on a scheduled nightly.

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
TREND_ERROR = (":x:", "Trend check could not evaluate this run — see CI run "
                      "for details")
CLEAN = (":white_check_mark:", "No regressions detected")
NOT_RUN = (":information_source:",
           "Trend check did not run — see CI run for details")

# trend.py's exit codes, imported FROM here by trend.py so the two cannot
# drift. A regression and an operational abort both left main() non-zero and
# both collapse to steps.trend.outcome == 'failure', so before these existed
# the classifier had no way to tell them apart and announced every abort as a
# >=10% regression. That is not hypothetical: the 2026-08-15 nightly died at
# import in the measurement job, and Slack reported a perf regression for a
# night that never measured anything.
EXIT_REGRESSION = 1
EXIT_CANNOT_EVALUATE = 2


def warnings_status(n):
    """The yellow warning-tier line for `n` warning-level changes."""
    return (":large_yellow_circle:",
            f"{n} warning-level change(s) (>=5%, below the 10% error gate) "
            f"— see CI summary")


def classify(trend_outcome, job_status, warnings, trend_exit=None):
    """Return the ``(icon, status)`` pair for one nightly.

    `trend_outcome` is the trend step's GitHub Actions outcome ("success",
    "failure", or "skipped"), `job_status` is the job's overall status, and
    `warnings` is the count trend.py wrote to GITHUB_OUTPUT (0 when it wrote
    nothing, via the workflow's `|| '0'` fallback). `trend_exit` is trend.py's
    exit code when the workflow captured one, and None when it did not.

    The branch ORDER is the part worth protecting, and it is why this is a
    function rather than a dict lookup:

    1. A regression is classified FIRST, so it can never be shadowed by a
       generic failure line. The qualifier is that `outcome == 'failure'` is
       necessary but NOT sufficient: trend.py leaves main() non-zero for
       operational aborts too, and EXIT_CANNOT_EVALUATE is what separates
       "a timer moved" from "this run could not be judged".
    2. job_status then separates "the trend step never ran because an earlier
       step failed" from "it never ran because this is a publish=false
       measurement-only run". Note it is the MEASUREMENT job's status, not
       this one's, so a regression does not reach here at all.
    3. A trend step that failed without a regression, on a run whose sweep
       succeeded, is its own state: the benchmark produced numbers but the
       series could not be read or the point could not be found.
    4. Warnings only outrank a clean report, never a regression: a night with
       both is a regression night.
    5. Anything left is the trend step not having run while the job stayed
       green — its gate makes the outcome "skipped", which matches nothing
       above. Phrased generically so the message stays true if that gate
       changes.

    `trend_exit` defaults to None so an unset or unparseable value keeps the
    pre-existing behaviour of reporting a regression. Of the two ways to be
    wrong, a false alarm is recoverable and a silently swallowed regression is
    not.
    """
    if trend_outcome == "failure" and trend_exit != EXIT_CANNOT_EVALUATE:
        return REGRESSION
    if job_status != "success":
        return JOB_FAILED
    if trend_outcome == "failure":
        return TREND_ERROR
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


def _exit_from_env(raw):
    """Parse TREND_EXIT into an int, or None when there is nothing usable.

    Unset is the normal case for a skipped trend step, and an unparseable
    value means the workflow wiring broke rather than that the run was clean —
    both fall back to None, which classify() reads as "cannot rule out a
    regression"."""
    if raw is None or raw == "":
        return None
    try:
        return int(raw)
    except ValueError:
        print(f"::warning title=Slack status::could not parse "
              f"TREND_EXIT={raw!r}; not using it to classify", file=sys.stderr)
        return None


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--field", choices=["icon", "status"], default=None,
                    help="print just this field (default: icon then status, "
                         "one per line)")
    args = ap.parse_args()
    icon, status = classify(os.environ.get("TREND_OUTCOME", ""),
                            os.environ.get("JOB_STATUS", ""),
                            _warnings_from_env(os.environ.get("TREND_WARNINGS")),
                            _exit_from_env(os.environ.get("TREND_EXIT")))
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

# THE ordering invariant: a regression must never be shadowed by a generic
# failure line, whatever else is true of the run.
assert classify("failure", "failure", 0) == REGRESSION, \
    "a regression must outrank the generic job-failure case, not be shadowed by it"
assert classify("failure", "failure", 5) == REGRESSION, \
    "a regression with warnings alongside it is still a regression"
assert classify("failure", "success", 0, EXIT_REGRESSION) == REGRESSION, \
    "an explicit regression exit code must still classify as a regression"

# The 2026-08-15 nightly, exactly: the measurement job died at import, so no
# point was registered, so trend.py aborted with "no such point in the
# tracking series" — an operational abort, reported to Slack as a >=10%
# regression. The exit code is the whole difference between these two lines.
assert classify("failure", "failure", 0, EXIT_CANNOT_EVALUATE) == JOB_FAILED, \
    ("a trend step that could not evaluate must not be announced as a "
     "regression; with a failed sweep behind it the sweep is the story")
assert classify("failure", "success", 0, EXIT_CANNOT_EVALUATE) == TREND_ERROR, \
    ("a green sweep whose trend check could not evaluate is its own state: "
     "the numbers exist but the series could not be read")

# An unknown exit code is not a licence to assume the run was fine. Only the
# one code that MEANS "could not evaluate" suppresses the regression line.
assert classify("failure", "success", 0, None) == REGRESSION, \
    "an unavailable exit code must not downgrade a failing trend step"
assert classify("failure", "success", 0, 99) == REGRESSION, \
    "an unrecognised exit code must not downgrade a failing trend step"

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

# TREND_EXIT parses to an int or to None; None is the "cannot rule out a
# regression" fallback, so an unparseable value must not become a number that
# happens to equal EXIT_CANNOT_EVALUATE.
assert _exit_from_env("1") == EXIT_REGRESSION
assert _exit_from_env("2") == EXIT_CANNOT_EVALUATE
assert _exit_from_env(None) is None
assert _exit_from_env("") is None
_buf2 = __import__("io").StringIO()
with __import__("contextlib").redirect_stderr(_buf2):
    assert _exit_from_env("not-a-number") is None
assert "could not parse" in _buf2.getvalue(), \
    "an unparseable exit code must still be reported, not silently ignored"
del _buf2

assert EXIT_REGRESSION == 1, \
    ("1 is what a bare `raise SystemExit(1)` and an uncaught exception both "
     "produce, so the regression code must be the one that needs no wiring "
     "to stay correct")
assert EXIT_REGRESSION != EXIT_CANNOT_EVALUATE
_buf = __import__("io").StringIO()
with __import__("contextlib").redirect_stderr(_buf):
    _bad = _warnings_from_env("not-a-number")
assert _bad != 0, "an unparseable warning count must not read as zero warnings"
assert "could not parse" in _buf.getvalue(), \
    "an unparseable warning count must still be reported, not silently coerced"
del _buf, _bad


if __name__ == "__main__":
    main()
