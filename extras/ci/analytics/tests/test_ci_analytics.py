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
import ci_status
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

    def test_generate_health_html_shows_empty_groups_with_message(self):
        """GCP groups with no online runners should still appear with a message."""
        queue_data = {
            "summary": {"jobs_queued": 0, "jobs_running": 0, "runs_queued": 0, "runs_in_progress": 0},
            "self_hosted_runners": [
                {"name": "linux-runner-1", "group": "Linux GPU (GCP)", "status": "online", "busy": False},
            ],
            "queue_by_group": [],
            "longest_waiting_jobs": [],
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.generate_health_html(queue_data, [], tmp)
            with open(os.path.join(tmp, "health.html"), encoding="utf-8") as f:
                html = f.read()

        # All three groups should appear even when no runners are online
        self.assertIn("Linux GPU (GCP)", html)
        self.assertIn("Windows Build (GCP)", html)
        self.assertIn("Windows GPU (GCP)", html)
        self.assertIn("scales to zero when idle", html)


class TestGpuQuota(unittest.TestCase):
    def test_record_snapshot_stores_gpu_quota_per_region(self):
        queue_data = {
            "summary": {"jobs_queued": 0, "jobs_running": 0, "runs_queued": 0, "runs_in_progress": 0},
            "self_hosted_runners": [],
            "queue_by_group": [],
        }
        gpu_quota = {
            "usage": 18,
            "limit": 24,
            "regions": {
                "us-central1": {"usage": 6, "limit": 8},
                "us-east1": {"usage": 7, "limit": 8},
                "us-west1": {"usage": 5, "limit": 8},
            },
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.record_snapshot(queue_data, tmp, gpu_quota=gpu_quota)
            with open(os.path.join(tmp, ci_health.SNAPSHOTS_FILE), encoding="utf-8") as f:
                snapshot = json.loads(f.readline())

        self.assertEqual(snapshot["gpu_quota"]["us-central1"], {"usage": 6, "limit": 8})
        self.assertEqual(snapshot["gpu_quota"]["us-east1"], {"usage": 7, "limit": 8})
        self.assertEqual(snapshot["gpu_quota"]["us-west1"], {"usage": 5, "limit": 8})

    def test_build_history_chart_includes_gpu_quota_when_present(self):
        snapshots = [
            {
                "timestamp": "2026-03-03T10:00:00Z",
                "jobs_queued": 0,
                "jobs_running": 0,
                "runs_queued": 0,
                "runs_in_progress": 0,
                "runner_groups": {},
                "gpu_quota": {
                    "us-central1": {"usage": 6, "limit": 8},
                    "us-east1": {"usage": 7, "limit": 8},
                },
            },
        ]

        html = ci_health.build_history_chart(snapshots)
        self.assertIn("T4 GPU Usage", html)
        self.assertIn("gpuQuota_canvas", html)
        self.assertIn("us-central1", html)
        self.assertIn("us-east1", html)
        self.assertIn("Quota Limit", html)

    def test_build_history_chart_omits_gpu_quota_section_when_absent(self):
        snapshots = [
            {
                "timestamp": "2026-03-03T10:00:00Z",
                "jobs_queued": 0,
                "jobs_running": 0,
                "runs_queued": 0,
                "runs_in_progress": 0,
                "runner_groups": {"Linux GPU (GCP)": {"total": 1}},
            },
        ]

        html = ci_health.build_history_chart(snapshots)
        # The chart section HTML should not be present (JS guard is fine)
        self.assertNotIn("T4 GPU Usage", html)


class TestStatisticsRunnerNamePrefixes(unittest.TestCase):
    def test_statistics_parallel_chart_includes_runner_name_prefix_groups(self):
        """generate_statistics must pick up self-hosted groups defined only
        in runner_name_prefixes (e.g. Windows Build) so they appear in the
        Average Concurrent Runners chart."""
        config = {
            "label_groups": [
                {"labels": ["Linux", "self-hosted", "GPU"], "name": "Linux GPU (GCP)", "self_hosted": True},
            ],
            "runner_name_prefixes": [
                {"prefix": "win-build-", "name": "Windows Build (GCP)", "self_hosted": True},
            ],
            "non_production_periods": {"runners": {}},
        }

        # Two days ago so process_jobs doesn't skip it as "today"
        yesterday = (datetime.now(timezone.utc).replace(hour=12, minute=0, second=0, microsecond=0)
                     - ci_health.timedelta(days=2))
        ts = yesterday.strftime("%Y-%m-%dT%H:%M:%SZ")
        completed = (yesterday + ci_health.timedelta(minutes=30)).strftime("%Y-%m-%dT%H:%M:%SZ")

        jobs = [
            {
                "name": "build-windows",
                "workflow_name": "CI",
                "run_id": 1,
                "run_created_at": ts,
                "created_at": ts,
                "started_at": ts,
                "completed_at": completed,
                "conclusion": "success",
                "event": "push",
                "head_branch": "main",
                "labels": [],
                "runner_name": "win-build-42",
                "duration_seconds": 1800,
                "queued_seconds": 10,
                "html_url": "",
            },
        ]

        data = ci_visualization.process_jobs(jobs, config)

        with tempfile.TemporaryDirectory() as tmp:
            ci_visualization.generate_statistics(data, config, tmp)
            with open(os.path.join(tmp, "statistics.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertIn("Windows Build (GCP)", html)


class TestPRsMergedChart(unittest.TestCase):
    def test_statistics_includes_prs_merged_chart_when_data_present(self):
        """generate_statistics renders PRs Merged chart when pr_merges data exists."""
        two_days_ago = (
            datetime.now(timezone.utc).replace(hour=12, minute=0, second=0, microsecond=0)
            - ci_health.timedelta(days=2)
        )
        ts = two_days_ago.strftime("%Y-%m-%dT%H:%M:%SZ")
        completed = (two_days_ago + ci_health.timedelta(minutes=30)).strftime(
            "%Y-%m-%dT%H:%M:%SZ"
        )

        jobs = [
            {
                "name": "build-linux-debug",
                "workflow_name": "CI",
                "run_id": 1,
                "run_created_at": ts,
                "created_at": ts,
                "started_at": ts,
                "completed_at": completed,
                "conclusion": "success",
                "event": "push",
                "head_branch": "main",
                "labels": [],
                "runner_name": "",
                "duration_seconds": 1800,
                "queued_seconds": 10,
                "html_url": "",
            },
        ]

        config = {
            "label_groups": [],
            "runner_name_prefixes": [],
            "non_production_periods": {"runners": {}},
        }

        data = ci_visualization.process_jobs(jobs, config)
        data["pr_merges"] = [
            {"number": 100, "merged_at": ts, "user": "dev1"},
            {"number": 101, "merged_at": ts, "user": "dev2"},
        ]

        with tempfile.TemporaryDirectory() as tmp:
            ci_visualization.generate_statistics(data, config, tmp)
            with open(os.path.join(tmp, "statistics.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertIn("PRs Merged per Day", html)
        self.assertIn("prsMerged_canvas", html)
        self.assertIn("allPRsMerged", html)

    def test_statistics_omits_prs_merged_chart_when_no_data(self):
        """generate_statistics omits PRs Merged chart when no pr_merges data."""
        two_days_ago = (
            datetime.now(timezone.utc).replace(hour=12, minute=0, second=0, microsecond=0)
            - ci_health.timedelta(days=2)
        )
        ts = two_days_ago.strftime("%Y-%m-%dT%H:%M:%SZ")
        completed = (two_days_ago + ci_health.timedelta(minutes=30)).strftime(
            "%Y-%m-%dT%H:%M:%SZ"
        )

        jobs = [
            {
                "name": "build-linux-debug",
                "workflow_name": "CI",
                "run_id": 1,
                "run_created_at": ts,
                "created_at": ts,
                "started_at": ts,
                "completed_at": completed,
                "conclusion": "success",
                "event": "push",
                "head_branch": "main",
                "labels": [],
                "runner_name": "",
                "duration_seconds": 1800,
                "queued_seconds": 10,
                "html_url": "",
            },
        ]

        config = {
            "label_groups": [],
            "runner_name_prefixes": [],
            "non_production_periods": {"runners": {}},
        }

        data = ci_visualization.process_jobs(jobs, config)
        data["pr_merges"] = []

        with tempfile.TemporaryDirectory() as tmp:
            ci_visualization.generate_statistics(data, config, tmp)
            with open(os.path.join(tmp, "statistics.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertNotIn("PRs Merged per Day", html)

    def test_index_includes_prs_merged_stat_card(self):
        """generate_index shows PRs Merged / day stat card when data present."""
        two_days_ago = (
            datetime.now(timezone.utc).replace(hour=12, minute=0, second=0, microsecond=0)
            - ci_health.timedelta(days=2)
        )
        ts = two_days_ago.strftime("%Y-%m-%dT%H:%M:%SZ")
        completed = (two_days_ago + ci_health.timedelta(minutes=30)).strftime(
            "%Y-%m-%dT%H:%M:%SZ"
        )

        jobs = [
            {
                "name": "build-linux-debug",
                "workflow_name": "CI",
                "run_id": 1,
                "run_created_at": ts,
                "created_at": ts,
                "started_at": ts,
                "completed_at": completed,
                "conclusion": "success",
                "event": "push",
                "head_branch": "main",
                "labels": [],
                "runner_name": "",
                "duration_seconds": 1800,
                "queued_seconds": 10,
                "html_url": "",
            },
        ]

        config = {
            "label_groups": [],
            "runner_name_prefixes": [],
            "non_production_periods": {"runners": {}},
        }

        data = ci_visualization.process_jobs(jobs, config)
        data["pr_merges"] = [
            {"number": 100, "merged_at": ts, "user": "dev1"},
        ]

        with tempfile.TemporaryDirectory() as tmp:
            ci_visualization.generate_index(data, tmp)
            with open(os.path.join(tmp, "index.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertIn("PRs Merged / day", html)


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


class TestStatusPage(unittest.TestCase):
    def test_generate_with_entries(self):
        with tempfile.TemporaryDirectory() as tmp:
            data = {
                "entries": [
                    {
                        "date": "2026-03-12",
                        "severity": "warning",
                        "title": "Runner flaky",
                        "body": "Investigating.",
                        "author": "testuser",
                    },
                    {
                        "date": "2026-03-10",
                        "severity": "info",
                        "title": "Maintenance",
                        "body": "Scheduled downtime.",
                        "author": "admin",
                    },
                ]
            }
            with open(os.path.join(tmp, "status_updates.json"), "w") as f:
                json.dump(data, f)

            ci_status.generate_status_html(tmp)

            with open(os.path.join(tmp, "status.html")) as f:
                html = f.read()
            self.assertIn("Runner flaky", html)
            self.assertIn("Maintenance", html)
            self.assertIn("testuser", html)
            # warning entry should appear before info (sorted by date desc)
            self.assertGreater(html.index("Maintenance"), html.index("Runner flaky"))

    def test_generate_empty_entries(self):
        with tempfile.TemporaryDirectory() as tmp:
            with open(os.path.join(tmp, "status_updates.json"), "w") as f:
                json.dump({"entries": []}, f)

            ci_status.generate_status_html(tmp)

            with open(os.path.join(tmp, "status.html")) as f:
                html = f.read()
            self.assertIn("All Systems Operational", html)

    def test_generate_no_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            ci_status.generate_status_html(tmp)

            with open(os.path.join(tmp, "status.html")) as f:
                html = f.read()
            self.assertIn("All Systems Operational", html)

    def test_severity_colors(self):
        for sev in ("info", "warning", "critical"):
            entry = {"severity": sev, "title": f"Test {sev}", "body": "x"}
            rendered = ci_status.render_entry(entry)
            fg, bg = ci_status.SEVERITY_COLORS[sev]
            self.assertIn(fg, rendered)
            self.assertIn(bg, rendered)

    def test_html_escaping(self):
        entry = {
            "title": "<script>alert(1)</script>",
            "body": "a < b & c > d",
            "author": "<img>",
        }
        rendered = ci_status.render_entry(entry)
        self.assertNotIn("<script>", rendered)
        self.assertIn("&lt;script&gt;", rendered)
        self.assertIn("a &lt; b &amp; c &gt; d", rendered)

    def test_hidden_entries_not_rendered(self):
        with tempfile.TemporaryDirectory() as tmp:
            data = {
                "entries": [
                    {"date": "2026-03-12", "severity": "info", "title": "Visible", "body": "shown", "visible": True},
                    {"date": "2026-03-11", "severity": "warning", "title": "Hidden", "body": "not shown", "visible": False},
                ]
            }
            with open(os.path.join(tmp, "status_updates.json"), "w") as f:
                json.dump(data, f)

            ci_status.generate_status_html(tmp)

            with open(os.path.join(tmp, "status.html")) as f:
                html = f.read()
            self.assertIn("Visible", html)
            self.assertNotIn("Hidden", html)

    def test_all_hidden_shows_all_clear(self):
        with tempfile.TemporaryDirectory() as tmp:
            data = {
                "entries": [
                    {"date": "2026-03-12", "severity": "info", "title": "Old issue", "body": "resolved", "visible": False},
                ]
            }
            with open(os.path.join(tmp, "status_updates.json"), "w") as f:
                json.dump(data, f)

            ci_status.generate_status_html(tmp)

            with open(os.path.join(tmp, "status.html")) as f:
                html = f.read()
            self.assertIn("All Systems Operational", html)
            self.assertNotIn("Old issue", html)

    def test_nav_includes_status(self):
        nav = ci_visualization.nav_html("Status")
        self.assertIn("status.html", nav)
        self.assertIn("Status", nav)


if __name__ == "__main__":
    unittest.main()
