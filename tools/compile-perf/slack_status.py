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
ANALYZE_FAILED = (":x:", "Trend analysis job failed before it could judge — "
                         "see CI run for details")
CLEAN = (":white_check_mark:", "No regressions detected")
NOT_RUN = (":information_source:",
           "Trend check did not run — see CI run for details")

# trend.py's exit codes, imported FROM here by trend.py so the two cannot
# drift.
#
# THE CONTRACT: a measured regression is the ONLY thing that may produce
# EXIT_REGRESSION, and the classifier matches it by exact equality. Every
# other non-zero exit — a deliberate abort, an unhandled exception, a code
# this file has never heard of — means "no comparison completed", and must
# not be reported as a measured >=10% move.
#
# Note what 1 is NOT. An unhandled exception exits 1, as does a bare
# `SystemExit`, so 1 is the code a crash arrives with; making it the
# regression code would mean every crash in trend.py announced itself as a
# perf regression. The regression code is therefore deliberately a value
# nothing produces by accident.
#
# Reporting the wrong one of these is not a safety trade. Both leave the job
# red and both send an alert — only the sentence differs — so there is no
# case for defaulting an unrecognised code to the regression line.
EXIT_REGRESSION = 3
EXIT_CANNOT_EVALUATE = 2


def warnings_status(n):
    """The yellow warning-tier line for `n` warning-level changes."""
    return (":large_yellow_circle:",
            f"{n} warning-level change(s) (>=5%, below the 10% error gate) "
            f"— see CI summary")


def classify(trend_outcome, job_status, warnings, trend_exit=None,
             analyze_status=""):
    """Return the ``(icon, status)`` pair for one nightly.

    `trend_outcome` is the trend step's GitHub Actions outcome ("success",
    "failure", or "skipped"), `job_status` is the MEASUREMENT job's overall
    status, `warnings` is the count trend.py wrote to GITHUB_OUTPUT (0 when it
    wrote nothing, via the workflow's `|| '0'` fallback), `trend_exit` is
    trend.py's exit code when the workflow captured one, and `analyze_status`
    is the analysis job's own status at the point the Slack step runs (empty
    when the workflow does not supply it).

    The branch ORDER is the part worth protecting, and it is why this is a
    function rather than a dict lookup:

    1. A regression is classified FIRST, so it can never be shadowed by a
       generic failure line. It requires trend.py's EXIT_REGRESSION exactly —
       `outcome == 'failure'` is necessary but nowhere near sufficient, since
       aborts and crashes land there too.
    2. job_status then separates "the trend step never ran because the sweep
       failed" from "it never ran because this is a publish=false
       measurement-only run". It is the MEASUREMENT job's status, not this
       one's, so a regression never reaches here.
    3. A trend step that failed without a measured regression is its own
       state: the benchmark produced numbers, but the series could not be
       read, the point was not found, or trend.py crashed.
    4. The ANALYSIS job failing outside the trend step — a checkout, say —
       must not fall through to the informational "did not run" line. A step
       `if:` carries an implicit success(), so a failure before the trend step
       SKIPS it, which would otherwise read as the quietest state of all on a
       red job.
    5. Warnings only outrank a clean report, never a regression: a night with
       both is a regression night.
    6. Anything left is the trend step not having run while everything stayed
       green — its gate makes the outcome "skipped", which matches nothing
       above. Phrased generically so the message stays true if that gate
       changes.
    """
    if trend_outcome == "failure" and trend_exit == EXIT_REGRESSION:
        return REGRESSION
    if job_status != "success":
        return JOB_FAILED
    if trend_outcome == "failure":
        return TREND_ERROR
    if analyze_status not in ("", "success"):
        return ANALYZE_FAILED
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
                            _exit_from_env(os.environ.get("TREND_EXIT")),
                            os.environ.get("ANALYZE_STATUS", ""))
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
assert classify("failure", "failure", 0, EXIT_REGRESSION) == REGRESSION
assert classify("", "failure", 0) == JOB_FAILED

# THE ordering invariant: a regression must never be shadowed by a generic
# failure line, whatever else is true of the run.
assert classify("failure", "failure", 0, EXIT_REGRESSION) == REGRESSION, \
    "a regression must outrank the generic job-failure case, not be shadowed by it"
assert classify("failure", "failure", 5, EXIT_REGRESSION) == REGRESSION, \
    "a regression with warnings alongside it is still a regression"
assert classify("failure", "success", 0, EXIT_REGRESSION) == REGRESSION

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

# ONLY the regression code produces the regression line. These are the cases
# that made exit 1 unusable as the regression code: an unhandled exception in
# trend.py (a malformed tracking.json raising JSONDecodeError, say) exits 1
# and completed no comparison at all, so it must never be announced as a
# measured >=10% move. Same for a code this file has never heard of, and for
# a code the workflow failed to publish.
assert classify("failure", "success", 0, 1) == TREND_ERROR, \
    ("exit 1 is what an unhandled exception produces, not a measured "
     "regression: no comparison completed, so no regression may be claimed")
assert classify("failure", "success", 0, 99) == TREND_ERROR, \
    "an unrecognised exit code means no comparison completed"
assert classify("failure", "success", 0, None) == TREND_ERROR, \
    "an unavailable exit code cannot establish that a regression was measured"
assert classify("failure", "failure", 0, 1) == JOB_FAILED
assert EXIT_REGRESSION != 1, \
    ("the regression code must not collide with the code an unhandled "
     "exception exits with, or every crash announces itself as a regression")

# The analysis job failing OUTSIDE the trend step: a step `if:` carries an
# implicit success(), so a failed checkout skips the trend step entirely. Left
# unhandled that reports the quiet blue "did not run" line on a red job.
assert classify("skipped", "success", 0, None, "failure") == ANALYZE_FAILED, \
    ("a red analysis job must not report the informational did-not-run line; "
     "the trend step being skipped is a CONSEQUENCE of the failure")
assert classify("skipped", "success", 0, None, "success") == NOT_RUN, \
    "a green analysis job with a skipped trend step is still the quiet state"
assert classify("skipped", "success", 0, None, "") == NOT_RUN, \
    "an absent analyze status must behave as it did before the key existed"
assert classify("success", "success", 0, 0, "failure") == ANALYZE_FAILED, \
    "a later failure in the analysis job outranks a clean trend result"
assert classify("failure", "success", 0, EXIT_REGRESSION, "failure") == REGRESSION, \
    ("the trend step failing IS the analysis job failing, so a measured "
     "regression must not be relabelled by its own side effect")

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
assert _exit_from_env(str(EXIT_REGRESSION)) == EXIT_REGRESSION
assert _exit_from_env("2") == EXIT_CANNOT_EVALUATE
assert _exit_from_env("1") == 1, \
    "a crash's exit 1 must survive parsing so classify can reject it"
assert _exit_from_env(None) is None
assert _exit_from_env("") is None
_buf2 = __import__("io").StringIO()
with __import__("contextlib").redirect_stderr(_buf2):
    assert _exit_from_env("not-a-number") is None
assert "could not parse" in _buf2.getvalue(), \
    "an unparseable exit code must still be reported, not silently ignored"
del _buf2

assert EXIT_REGRESSION != EXIT_CANNOT_EVALUATE
assert 0 not in (EXIT_REGRESSION, EXIT_CANNOT_EVALUATE), \
    "both codes must be failures; 0 is a clean night"
_buf = __import__("io").StringIO()
with __import__("contextlib").redirect_stderr(_buf):
    _bad = _warnings_from_env("not-a-number")
assert _bad != 0, "an unparseable warning count must not read as zero warnings"
assert "could not parse" in _buf.getvalue(), \
    "an unparseable warning count must still be reported, not silently coerced"
del _buf, _bad


if __name__ == "__main__":
    main()
