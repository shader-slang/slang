"""Regression coverage for approval-only waits and intentionally yielded CI."""

import contextlib
import io
import os
import sys
import tempfile
import unittest
from unittest import mock

ANALYTICS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CI_DIR = os.path.dirname(ANALYTICS_DIR)
sys.path.insert(0, ANALYTICS_DIR)
sys.path.insert(0, CI_DIR)

import ci_job_outcomes as priority
import ci_job_collector as collector
import ci_visualization as visualization


def job(name, conclusion="success", **fields):
    return dict(
        id=1, run_id=10, run_attempt=1, workflow_name="CI", name=name,
        status="completed", conclusion=conclusion, created_at="2026-10-04T05:23:23Z",
        started_at="2026-10-04T05:23:23Z", completed_at="2026-10-04T05:23:37Z",
        duration_seconds=14, **fields
    )


def yielded_jobs():
    return [
        job(priority.GATE_JOB_NAME, "failure", steps=[
            {"name": priority.YIELDED_STEP_NAME, "conclusion": "failure"}
        ]),
        job(priority.CHECK_CI_JOB_NAME, "failure"),
        job(priority.APPROVAL_GATE_JOB_NAME, "cancelled"),
    ]


class TestYieldClassification(unittest.TestCase):
    def test_cancelled_approval_remains_a_verified_yield(self):
        self.assertTrue(priority.failed_only_because_priority_gate(yielded_jobs()))

    def test_real_failure_or_cancellation_is_not_hidden(self):
        for name in ["build / build", priority.APPROVAL_GATE_JOB_NAME]:
            for conclusion in ["failure", "cancelled"]:
                if name == priority.APPROVAL_GATE_JOB_NAME and conclusion == "cancelled":
                    continue
                with self.subTest(name=name, conclusion=conclusion):
                    self.assertFalse(priority.failed_only_because_priority_gate(
                        yielded_jobs() + [job(name, conclusion)]
                    ))

    def test_gate_api_failure_is_not_a_yield(self):
        jobs = yielded_jobs()
        jobs[0]["steps"] = [{"name": "Check priority gate", "conclusion": "failure"}]
        self.assertFalse(priority.failed_only_because_priority_gate(jobs))

    def test_collector_preserves_marker_and_attempt(self):
        record = collector.extract_job_data(yielded_jobs()[0], {"id": 10, "run_attempt": 2})
        self.assertTrue(record["priority_yielded"])
        self.assertEqual(record["run_attempt"], 2)
        self.assertTrue(priority.yielded_marker_failed(record))

    def test_monthly_merge_refreshes_marker_without_losing_other_records(self):
        records = collector.merge_data(
            [{"id": 1}, {"id": 2}], [{"id": 1, "priority_yielded": True}]
        )
        self.assertEqual(records, [{"id": 1, "priority_yielded": True}, {"id": 2}])

    def test_collector_refreshes_existing_records(self):
        jobs = yielded_jobs()[:1]
        with mock.patch.object(collector, "fetch_jobs_for_run", return_value=(jobs, None)):
            with contextlib.redirect_stderr(io.StringIO()):
                records, errors, _ = collector.collect_jobs(
                    "owner/repo", [{"id": 10, "run_attempt": 2}], existing=[{"id": 1}]
                )
        self.assertEqual(errors, 0)
        self.assertEqual(len(records), 1)
        self.assertTrue(records[0]["priority_yielded"])


class TestExecutionAnalytics(unittest.TestCase):
    def process(self, jobs):
        with contextlib.redirect_stderr(io.StringIO()):
            return visualization.process_jobs(jobs, {"label_groups": []})

    def test_approval_duration_excluded_without_marker_in_legacy_data(self):
        gate = job(priority.APPROVAL_GATE_JOB_NAME, "cancelled")
        gate["duration_seconds"] = 32072
        data = self.process([job("build / build"), gate])
        self.assertEqual([j["name"] for j in data["active_jobs"]], ["build / build"])

    def test_verified_yield_is_excluded_but_successful_retry_remains(self):
        jobs = yielded_jobs()
        jobs.append(job("build / build"))
        jobs[-1]["run_attempt"] = 2
        data = self.process(jobs)
        self.assertEqual(len(data["active_jobs"]), 1)
        self.assertEqual(data["active_jobs"][0]["run_attempt"], 2)

    def test_real_failure_alongside_marker_remains_in_metrics(self):
        data = self.process(yielded_jobs() + [job("test / test-slang", "failure")])
        self.assertIn("test / test-slang", [j["name"] for j in data["active_jobs"]])
        self.assertIn(priority.GATE_JOB_NAME, [j["name"] for j in data["active_jobs"]])

    def test_legacy_failure_without_marker_is_not_assumed_to_be_yield(self):
        data = self.process([job(priority.GATE_JOB_NAME, "failure")])
        self.assertEqual(len(data["active_jobs"]), 1)

    def test_statistics_describe_exclusions(self):
        data = self.process([job("filter")])
        with tempfile.TemporaryDirectory() as directory:
            visualization.generate_statistics(data, {"label_groups": []}, directory)
            with open(os.path.join(directory, "statistics.html")) as page:
                self.assertIn("verified priority yields", page.read())


if __name__ == "__main__":
    unittest.main()
