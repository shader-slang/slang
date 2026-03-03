import json
import os
import sys
import tempfile
import unittest
from datetime import datetime, timezone


ANALYTICS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if ANALYTICS_DIR not in sys.path:
    sys.path.insert(0, ANALYTICS_DIR)

import ci_health
import ci_visualization


class TestRunnerTypeCoverage(unittest.TestCase):
    def test_classify_group_supports_all_three_gcp_runner_types(self):
        config = {
            "label_groups": [],
            "runner_name_prefixes": [
                {"prefix": "linux-runner-", "name": "Linux GPU (GCP)", "self_hosted": True},
                {"prefix": "win-build-", "name": "Windows Build (GCP)", "self_hosted": True},
                {"prefix": "win-runner-", "name": "Windows GPU (GCP)", "self_hosted": True},
            ],
            "non_production_periods": {"runners": {}},
        }

        linux_group, linux_self_hosted = ci_visualization.classify_group([], config, "linux-runner-1")
        build_group, build_self_hosted = ci_visualization.classify_group([], config, "win-build-5")
        gpu_group, gpu_self_hosted = ci_visualization.classify_group([], config, "win-runner-3")

        self.assertEqual(linux_group, "Linux GPU (GCP)")
        self.assertTrue(linux_self_hosted)
        self.assertEqual(build_group, "Windows Build (GCP)")
        self.assertTrue(build_self_hosted)
        self.assertEqual(gpu_group, "Windows GPU (GCP)")
        self.assertTrue(gpu_self_hosted)

    def test_record_snapshot_counts_all_three_gcp_runner_types(self):
        queue_data = {
            "summary": {
                "jobs_queued": 4,
                "jobs_running": 8,
                "runs_queued": 2,
                "runs_in_progress": 3,
            },
            "self_hosted_runners": [
                {"group": "Linux GPU (GCP)", "status": "online", "busy": True},
                {"group": "Windows Build (GCP)", "status": "online", "busy": False},
                {"group": "Windows GPU (GCP)", "status": "online", "busy": True},
                {"group": "Windows GPU (GCP)", "status": "offline", "busy": True},
            ],
            "queue_by_group": [
                {"name": "Linux GPU (GCP)", "queued": 1, "running": 2},
                {"name": "Windows Build (GCP)", "queued": 3, "running": 4},
                {"name": "Windows GPU (GCP)", "queued": 5, "running": 6},
            ],
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.record_snapshot(queue_data, tmp)
            with open(os.path.join(tmp, ci_health.SNAPSHOTS_FILE), encoding="utf-8") as f:
                snapshot = json.loads(f.readline())

        self.assertEqual(snapshot["runner_groups"]["Linux GPU (GCP)"], {"busy": 1, "total": 1})
        self.assertEqual(snapshot["runner_groups"]["Windows Build (GCP)"], {"busy": 0, "total": 1})
        self.assertEqual(snapshot["runner_groups"]["Windows GPU (GCP)"], {"busy": 1, "total": 1})

    def test_build_history_chart_includes_all_three_gcp_runner_types(self):
        snapshots = [
            {
                "timestamp": "2026-03-03T10:00:00Z",
                "jobs_queued": 1,
                "jobs_running": 2,
                "runs_queued": 3,
                "runs_in_progress": 4,
                "runner_groups": {
                    "Linux GPU (GCP)": {"total": 1},
                    "Windows Build (GCP)": {"total": 2},
                    "Windows GPU (GCP)": {"total": 3},
                },
            },
            {
                "timestamp": "2026-03-03T10:15:00Z",
                "jobs_queued": 2,
                "jobs_running": 3,
                "runs_queued": 4,
                "runs_in_progress": 5,
                "runner_groups": {
                    "Linux GPU (GCP)": {"total": 2},
                    "Windows Build (GCP)": {"total": 3},
                    "Windows GPU (GCP)": {"total": 4},
                },
            },
        ]

        html = ci_health.build_history_chart(snapshots)
        self.assertIn("Linux GPU (GCP)", html)
        self.assertIn("Windows Build (GCP)", html)
        self.assertIn("Windows GPU (GCP)", html)

    def test_generate_health_html_renders_all_three_gcp_runner_tables(self):
        queue_data = {
            "summary": {
                "jobs_queued": 1,
                "jobs_running": 2,
                "runs_queued": 3,
                "runs_in_progress": 4,
            },
            "self_hosted_runners": [
                {"name": "linux-runner-1", "group": "Linux GPU (GCP)", "status": "online", "busy": True},
                {"name": "win-build-1", "group": "Windows Build (GCP)", "status": "online", "busy": False},
                {"name": "win-runner-1", "group": "Windows GPU (GCP)", "status": "online", "busy": True},
            ],
            "queue_by_group": [],
            "longest_waiting_jobs": [],
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.generate_health_html(queue_data, [], tmp)
            with open(os.path.join(tmp, "health.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertIn("Linux GPU (GCP)", html)
        self.assertIn("Windows Build (GCP)", html)
        self.assertIn("Windows GPU (GCP)", html)


class TestUtilityBehavior(unittest.TestCase):
    def test_round_time_rounds_down(self):
        self.assertEqual(ci_health._round_time("12:44"), "12:30")
        self.assertEqual(ci_health._round_time("12:59"), "12:45")
        self.assertEqual(ci_health._round_time("00:01"), "00:00")

    def test_deduplicate_snapshots_uses_latest_by_rounded_window(self):
        snapshots = [
            {"timestamp": "2026-03-03T10:00:00Z", "v": 1},
            {"timestamp": "2026-03-03T10:14:00Z", "v": 2},
            {"timestamp": "2026-03-03T10:15:00Z", "v": 3},
            {"timestamp": "2026-03-03T10:29:00Z", "v": 4},
        ]
        deduped = ci_health._deduplicate_snapshots(snapshots)
        self.assertEqual([s["v"] for s in deduped], [2, 4])

    def test_load_snapshots_reads_recent_only(self):
        now = datetime.now(timezone.utc)
        old_ts = (now.replace(microsecond=0) - ci_health.timedelta(hours=2)).strftime("%Y-%m-%dT%H:%M:%SZ")
        new_ts = (now.replace(microsecond=0) - ci_health.timedelta(minutes=10)).strftime("%Y-%m-%dT%H:%M:%SZ")

        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, ci_health.SNAPSHOTS_FILE)
            with open(path, "w", encoding="utf-8") as f:
                f.write(json.dumps({"timestamp": old_ts, "id": 1}) + "\n")
                f.write(json.dumps({"timestamp": new_ts, "id": 2}) + "\n")

            snaps = ci_health.load_snapshots(tmp, hours=1)

        self.assertEqual([s["id"] for s in snaps], [2])


if __name__ == "__main__":
    unittest.main()
