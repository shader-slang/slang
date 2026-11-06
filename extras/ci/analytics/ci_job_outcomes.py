"""Classify execution outcomes from raw or archived CI job evidence."""

APPROVAL_GATE_JOB_NAME = "falcor-build-approval-gate"
GATE_JOB_NAME = "wait-for-human-priority"
YIELDED_STEP_NAME = "Stop yielded bot CI"
CHECK_CI_JOB_NAME = "check-ci"
RERUNNABLE_CONCLUSIONS = {"failure", "cancelled"}


def yielded_marker_failed(job):
    """Identify intentional yielding by the dedicated marker step.

    The collector preserves this fact as priority_yielded because archived
    jobs do not otherwise retain their steps. A checkout or API failure in
    the priority job is still a real failure, not an intentional yield.
    """
    if job.get("name") != GATE_JOB_NAME:
        return False
    if job.get("conclusion") not in RERUNNABLE_CONCLUSIONS:
        return False
    if job.get("priority_yielded") is True:
        return True
    return any(
        step.get("name") == YIELDED_STEP_NAME
        and step.get("conclusion") in RERUNNABLE_CONCLUSIONS
        for step in job.get("steps") or []
    )


def failed_only_because_priority_gate(jobs):
    """Return true when a verified yield is the only cause of failure.

    Consider a yielded run whose Falcor approval gate was cancelled hours
    later. Cancelling that approval does not indicate a build/test failure;
    the run remains a verified priority yield. Any other failed or cancelled job
    prevents this classification, including an error executing the approval.
    """
    found_yielded_marker = False
    for job in jobs:
        name = job.get("name")
        conclusion = job.get("conclusion")
        if yielded_marker_failed(job):
            found_yielded_marker = True
            continue
        if name == CHECK_CI_JOB_NAME and conclusion in RERUNNABLE_CONCLUSIONS:
            continue
        if name == APPROVAL_GATE_JOB_NAME and conclusion == "cancelled":
            continue
        if conclusion in RERUNNABLE_CONCLUSIONS:
            return False
    return found_yielded_marker


