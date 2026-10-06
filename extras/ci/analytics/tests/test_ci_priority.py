"""Regression coverage for approval-only waits and intentionally yielded CI."""

import contextlib
import importlib.util
import io
import os
import re
import sys
import tempfile
import unittest
from unittest import mock

ANALYTICS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CI_DIR = os.path.dirname(ANALYTICS_DIR)
sys.path.insert(0, ANALYTICS_DIR)
sys.path.insert(0, CI_DIR)

import ci_priority_common as priority


def load_script(filename):
    spec = importlib.util.spec_from_file_location(filename, os.path.join(CI_DIR, filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


wait = load_script("wait-for-priority.py")
retry = load_script("retry-yielded-bot-ci.py")


def job(name, conclusion="success", **fields):
    return dict(
        id=1, run_id=10, run_attempt=1, workflow_name="CI", name=name,
        status="completed", conclusion=conclusion, created_at="2026-10-04T05:23:23Z",
        started_at="2026-10-04T05:23:23Z", completed_at="2026-10-04T05:23:37Z",
        duration_seconds=14, **fields
    )


def yielded_jobs():
    return [
        job(retry.GATE_JOB_NAME, "failure", steps=[
            {"name": retry.YIELDED_STEP_NAME, "conclusion": "failure"}
        ]),
        job(retry.CHECK_CI_JOB_NAME, "failure"),
        job(retry.APPROVAL_GATE_JOB_NAME, "cancelled"),
    ]


class TestFalcorWorkflow(unittest.TestCase):
    def workflow_job(self, name):
        root = os.path.dirname(os.path.dirname(CI_DIR))
        with open(os.path.join(root, ".github/workflows/ci.yml")) as workflow:
            text = workflow.read()
        # These jobs use inline needs lists and block scalar conditions. Read
        # those directly to test the actual workflow without a YAML dependency.
        block = re.search(
            r"^  " + re.escape(name) + r":\n(.*?)(?=^  [\w-]+:|\Z)",
            text, re.MULTILINE | re.DOTALL,
        ).group(1)
        needs = re.search(r"^    needs: \[(.*?)\]", block, re.MULTILINE).group(1)
        condition = re.search(
            r"^    if: (?:\||>-)\n((?:      .*\n)+)", block, re.MULTILINE
        ).group(1)
        return [value.strip() for value in needs.split(",")], " ".join(condition.split())

    def allows(self, condition, priority_result, approval_result, event):
        values = {
            "needs.filter.outputs.should-run": "true",
            "needs.wait-for-human-priority.result": priority_result,
            "needs.falcor-build-approval-gate.result": approval_result,
            "github.event_name": event,
        }
        for name, value in values.items():
            condition = condition.replace(name, repr(value))
        condition = condition.replace("always()", "True")
        condition = condition.replace("&&", " and ").replace("||", " or ")
        return eval(condition, {"__builtins__": {}}, {})

    def test_falcor_build_requires_priority_and_approved_or_merge_queue(self):
        needs, condition = self.workflow_job("build-windows-release-cl-x86_64-gpu-falcor")
        self.assertIn("wait-for-human-priority", needs)
        for priority_result, approval, event, expected in [
            ("success", "success", "pull_request", True),
            ("success", "skipped", "merge_group", True),
            ("failure", "skipped", "workflow_dispatch", False),
            ("failure", "success", "workflow_dispatch", False),
            ("skipped", "skipped", "workflow_dispatch", False),
            ("success", "skipped", "pull_request", False),
            ("success", "failure", "pull_request", False),
            ("success", "cancelled", "pull_request", False),
        ]:
            with self.subTest(priority=priority_result, approval=approval, event=event):
                self.assertEqual(self.allows(condition, priority_result, approval, event), expected)

    def test_yield_does_not_open_approval_request(self):
        needs, condition = self.workflow_job("falcor-build-approval-gate")
        self.assertIn("wait-for-human-priority", needs)
        self.assertFalse(self.allows(condition, "failure", "skipped", "workflow_dispatch"))
        self.assertTrue(self.allows(condition, "success", "success", "pull_request"))
        self.assertFalse(self.allows(condition, "success", "skipped", "merge_group"))


class TestApprovalWaits(unittest.TestCase):
    def fetch(self, jobs, error=None):
        run = {"id": 42, "status": "waiting", "run_number": 1,
               "actor": {"login": "human"}, "event": "pull_request"}

        def api(endpoint, key):
            if key == "jobs":
                return jobs, error
            return ([run] if "status=waiting&" in endpoint else []), None

        with mock.patch.object(priority, "gh_api_list", side_effect=api):
            return priority.fetch_active_runs("owner/repo", "ci.yml")

    def test_approval_only_run_blocks_neither_gate_nor_retry(self):
        runs = self.fetch([{"status": "completed"}, {"status": "waiting"}])
        self.assertEqual(wait.classify_blockers(runs, 100, 2, set()), ([], []))
        self.assertEqual(retry.any_active_ci(runs), [])

    def test_waiting_run_with_running_or_queued_sibling_still_blocks(self):
        for status in ["in_progress", "queued", "pending", "requested"]:
            with self.subTest(status=status):
                runs = self.fetch([{"status": "waiting"}, {"status": status}])
                self.assertEqual(len(wait.classify_blockers(runs, 100, 2, set())[0]), 1)
                self.assertEqual(len(retry.any_active_ci(runs)), 1)

    def test_missing_job_state_is_conservative(self):
        for jobs in [[], [{}]]:
            with self.subTest(jobs=jobs):
                self.assertEqual(len(self.fetch(jobs)), 1)

    def test_job_api_error_does_not_allow_bot_ci(self):
        with self.assertRaisesRegex(RuntimeError, "Failed to list jobs"):
            self.fetch([], "permission denied")

    def test_active_human_merge_and_older_bot_preserve_priority(self):
        runs = [
            {"id": 1, "status": "queued", "actor": {"login": "human"}},
            {"id": 2, "status": "in_progress", "event": "merge_group",
             "actor": {"login": "nv-slang-bot[bot]"}},
            {"id": 3, "status": "pending", "run_number": 3,
             "actor": {"login": "nv-slang-bot[bot]"}},
            {"id": 5, "status": "in_progress", "run_number": 5,
             "actor": {"login": "nv-slang-bot[bot]"}},
        ]
        human, bots = wait.classify_blockers(runs, 4, 4, priority.normalize_bot_logins())
        self.assertEqual([r["id"] for r in human], [1, 2])
        self.assertEqual([r["id"] for r in bots], [3])


class TestYieldClassification(unittest.TestCase):
    def test_cancelled_approval_does_not_prevent_yield_retry(self):
        self.assertTrue(retry.failed_only_because_priority_gate(yielded_jobs()))

    def test_real_failure_or_cancellation_is_not_hidden(self):
        for name in ["build / build", retry.APPROVAL_GATE_JOB_NAME]:
            for conclusion in ["failure", "cancelled"]:
                if name == retry.APPROVAL_GATE_JOB_NAME and conclusion == "cancelled":
                    continue
                with self.subTest(name=name, conclusion=conclusion):
                    self.assertFalse(retry.failed_only_because_priority_gate(
                        yielded_jobs() + [job(name, conclusion)]
                    ))

    def test_gate_api_failure_is_not_a_yield(self):
        jobs = yielded_jobs()
        jobs[0]["steps"] = [{"name": "Check priority gate", "conclusion": "failure"}]
        self.assertFalse(retry.failed_only_because_priority_gate(jobs))


if __name__ == "__main__":
    unittest.main()
