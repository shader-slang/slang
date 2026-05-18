import contextlib
import io
import json
import os
import sys
import tempfile
import unittest
from datetime import datetime, timezone
from unittest import mock


ANALYTICS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if ANALYTICS_DIR not in sys.path:
    sys.path.insert(0, ANALYTICS_DIR)

import ci_health
import ci_hosted_runner_usage
import ci_job_collector
import ci_status
import ci_visualization


class TestRunnerTypeCoverage(unittest.TestCase):
    def test_classify_group_supports_all_gcp_runner_types(self):
        config = {
            "label_groups": [],
            "runner_name_prefixes": [
                {"prefix": "linux-sm80plus-", "name": "Linux SM80Plus GPU (GCP)", "self_hosted": True},
                {"prefix": "linux-runner-", "name": "Linux GPU (GCP)", "self_hosted": True},
                {"prefix": "win-build-", "name": "Windows Build (GCP)", "self_hosted": True},
                {"prefix": "win-runner-", "name": "Windows GPU (GCP)", "self_hosted": True},
            ],
            "non_production_periods": {"runners": {}},
        }

        sm80_group, sm80_self_hosted = ci_visualization.classify_group([], config, "linux-sm80plus-1")
        linux_group, linux_self_hosted = ci_visualization.classify_group([], config, "linux-runner-1")
        build_group, build_self_hosted = ci_visualization.classify_group([], config, "win-build-5")
        gpu_group, gpu_self_hosted = ci_visualization.classify_group([], config, "win-runner-3")

        self.assertEqual(sm80_group, "Linux SM80Plus GPU (GCP)")
        self.assertTrue(sm80_self_hosted)
        self.assertEqual(linux_group, "Linux GPU (GCP)")
        self.assertTrue(linux_self_hosted)
        self.assertEqual(build_group, "Windows Build (GCP)")
        self.assertTrue(build_self_hosted)
        self.assertEqual(gpu_group, "Windows GPU (GCP)")
        self.assertTrue(gpu_self_hosted)

    def test_classify_group_supports_sm80plus_without_generic_gpu_label(self):
        config = {
            "label_groups": [
                {
                    "labels": ["Linux", "self-hosted", "SM80Plus"],
                    "name": "Linux SM80Plus GPU (GCP)",
                    "self_hosted": True,
                },
                {"labels": ["Linux", "self-hosted", "GPU"], "name": "Linux GPU (GCP)", "self_hosted": True},
            ],
            "runner_name_prefixes": [],
            "non_production_periods": {"runners": {}},
        }

        group, self_hosted = ci_visualization.classify_group(
            ["Linux", "self-hosted", "SM80Plus"],
            config,
        )

        self.assertEqual(group, "Linux SM80Plus GPU (GCP)")
        self.assertTrue(self_hosted)

    def test_record_snapshot_counts_all_gcp_runner_types(self):
        queue_data = {
            "summary": {
                "jobs_queued": 4,
                "jobs_running": 8,
                "runs_queued": 2,
                "runs_in_progress": 3,
            },
            "self_hosted_runners": [
                {"group": "Linux GPU (GCP)", "status": "online", "busy": True},
                {"group": "Linux SM80Plus GPU (GCP)", "status": "online", "busy": True},
                {"group": "Windows Build (GCP)", "status": "online", "busy": False},
                {"group": "Windows GPU (GCP)", "status": "online", "busy": True},
                {"group": "Windows GPU (GCP)", "status": "offline", "busy": True},
            ],
            "queue_by_group": [
                {"name": "Linux GPU (GCP)", "queued": 1, "running": 2},
                {"name": "Linux SM80Plus GPU (GCP)", "queued": 1, "running": 1},
                {"name": "Windows Build (GCP)", "queued": 3, "running": 4},
                {"name": "Windows GPU (GCP)", "queued": 5, "running": 6},
            ],
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.record_snapshot(queue_data, tmp)
            with open(os.path.join(tmp, ci_health.SNAPSHOTS_FILE), encoding="utf-8") as f:
                snapshot = json.loads(f.readline())

        self.assertEqual(snapshot["runner_groups"]["Linux GPU (GCP)"], {"busy": 1, "total": 1})
        self.assertEqual(snapshot["runner_groups"]["Linux SM80Plus GPU (GCP)"], {"busy": 1, "total": 1})
        self.assertEqual(snapshot["runner_groups"]["Windows Build (GCP)"], {"busy": 0, "total": 1})
        self.assertEqual(snapshot["runner_groups"]["Windows GPU (GCP)"], {"busy": 1, "total": 1})

    def test_build_history_chart_includes_all_gcp_runner_types(self):
        snapshots = [
            {
                "timestamp": "2026-03-03T10:00:00Z",
                "jobs_queued": 1,
                "jobs_running": 2,
                "runs_queued": 3,
                "runs_in_progress": 4,
                "runner_groups": {
                    "Linux GPU (GCP)": {"total": 1},
                    "Linux SM80Plus GPU (GCP)": {"total": 1},
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
                    "Linux SM80Plus GPU (GCP)": {"total": 1},
                    "Windows Build (GCP)": {"total": 3},
                    "Windows GPU (GCP)": {"total": 4},
                },
            },
        ]

        html = ci_health.build_history_chart(snapshots)
        self.assertIn("Linux GPU (GCP)", html)
        self.assertIn("Linux SM80Plus GPU (GCP)", html)
        self.assertIn("Windows Build (GCP)", html)
        self.assertIn("Windows GPU (GCP)", html)

    def test_generate_health_html_renders_all_gcp_runner_tables(self):
        queue_data = {
            "summary": {
                "jobs_queued": 1,
                "jobs_running": 2,
                "runs_queued": 3,
                "runs_in_progress": 4,
            },
            "self_hosted_runners": [
                {"name": "linux-runner-1", "group": "Linux GPU (GCP)", "status": "online", "busy": True},
                {"name": "linux-sm80plus-1", "group": "Linux SM80Plus GPU (GCP)", "status": "online", "busy": True},
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
        self.assertIn("Linux SM80Plus GPU (GCP)", html)
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

        # All GCP groups should appear even when no runners are online
        self.assertIn("Linux GPU (GCP)", html)
        self.assertIn("Linux SM80Plus GPU (GCP)", html)
        self.assertIn("Windows Build (GCP)", html)
        self.assertIn("Windows GPU (GCP)", html)
        self.assertIn("scales to zero when idle", html)


class TestGpuQuota(unittest.TestCase):
    def test_load_gpu_quota_metrics_falls_back_for_malformed_config(self):
        cases = [
            (["not", "an", "object"], "root must be an object"),
            ({"gpu_quota_metrics": {"metric": "NVIDIA_L4_GPUS"}}, "gpu_quota_metrics must be a list"),
            (
                {"gpu_quota_metrics": [{"metric": "NVIDIA_L4_GPUS", "regions": "us-central1"}]},
                "regions must be a non-empty string list",
            ),
        ]

        for payload, warning in cases:
            with self.subTest(payload=payload):
                stderr = io.StringIO()
                with mock.patch("builtins.open", mock.mock_open(read_data=json.dumps(payload))):
                    with contextlib.redirect_stderr(stderr):
                        metrics = ci_health.load_gpu_quota_metrics()

                self.assertEqual(metrics, ci_health.DEFAULT_GPU_QUOTA_METRICS)
                self.assertIn(warning, stderr.getvalue())
                self.assertIn("using defaults", stderr.getvalue())

    def test_load_gpu_quota_metrics_distinguishes_missing_from_empty_config(self):
        with mock.patch("builtins.open", mock.mock_open(read_data=json.dumps({}))):
            missing_metrics = ci_health.load_gpu_quota_metrics()

        stderr = io.StringIO()
        with mock.patch("builtins.open", mock.mock_open(read_data=json.dumps({"gpu_quota_metrics": []}))):
            with contextlib.redirect_stderr(stderr):
                empty_metrics = ci_health.load_gpu_quota_metrics()

        self.assertEqual(missing_metrics, ci_health.DEFAULT_GPU_QUOTA_METRICS)
        self.assertEqual(empty_metrics, [])
        self.assertEqual(stderr.getvalue(), "")

    def test_fetch_gpu_quota_honors_explicit_empty_metric_list(self):
        with mock.patch("ci_health.subprocess.run") as run:
            quota = ci_health.fetch_gpu_quota([])

        self.assertIsNone(quota)
        run.assert_not_called()

    def test_fetch_gpu_quota_fetches_configured_gpu_metrics(self):
        responses = {
            "us-central1": {
                "quotas": [
                    {"metric": "NVIDIA_T4_GPUS", "usage": 1, "limit": 8},
                    {"metric": "NVIDIA_L4_GPUS", "usage": 2, "limit": 16},
                ],
            },
            "us-east1": {
                "quotas": [
                    {"metric": "NVIDIA_L4_GPUS", "usage": 3, "limit": 16},
                ],
            },
        }

        def fake_run(cmd, **kwargs):
            region = cmd[cmd.index("describe") + 1]
            return mock.Mock(
                returncode=0,
                stdout=json.dumps(responses[region]),
                stderr="",
            )

        with mock.patch("ci_health.subprocess.run", side_effect=fake_run):
            quota = ci_health.fetch_gpu_quota([
                {"metric": "NVIDIA_T4_GPUS", "name": "T4", "regions": ["us-central1"]},
                {"metric": "NVIDIA_L4_GPUS", "name": "L4", "regions": ["us-central1", "us-east1"]},
            ])

        self.assertEqual(quota["usage"], 1)
        self.assertEqual(quota["limit"], 8)
        self.assertEqual(quota["by_metric"]["NVIDIA_T4_GPUS"]["regions"]["us-central1"], {"usage": 1, "limit": 8})
        self.assertEqual(quota["by_metric"]["NVIDIA_L4_GPUS"]["usage"], 5)
        self.assertEqual(quota["by_metric"]["NVIDIA_L4_GPUS"]["limit"], 32)

    def test_fetch_gpu_quota_skips_invalid_numeric_values(self):
        response = {
            "quotas": [
                {"metric": "NVIDIA_T4_GPUS", "usage": "bad", "limit": 8},
                {"metric": "NVIDIA_L4_GPUS", "usage": 2, "limit": None},
                {"metric": "NVIDIA_L4_GPUS", "usage": "3", "limit": "16"},
            ],
        }

        def fake_run(cmd, **kwargs):
            return mock.Mock(
                returncode=0,
                stdout=json.dumps(response),
                stderr="",
            )

        stderr = io.StringIO()
        with mock.patch("ci_health.subprocess.run", side_effect=fake_run):
            with contextlib.redirect_stderr(stderr):
                quota = ci_health.fetch_gpu_quota([
                    {"metric": "NVIDIA_T4_GPUS", "name": "T4", "regions": ["us-central1"]},
                    {"metric": "NVIDIA_L4_GPUS", "name": "L4", "regions": ["us-central1"]},
                ])

        self.assertNotIn("NVIDIA_T4_GPUS", quota["by_metric"])
        self.assertEqual(quota["by_metric"]["NVIDIA_L4_GPUS"]["usage"], 3)
        self.assertEqual(quota["by_metric"]["NVIDIA_L4_GPUS"]["limit"], 16)
        self.assertIn("invalid quota values for NVIDIA_T4_GPUS in us-central1", stderr.getvalue())
        self.assertIn("invalid quota values for NVIDIA_L4_GPUS in us-central1", stderr.getvalue())

    def test_record_snapshot_stores_gpu_quota_per_region(self):
        queue_data = {
            "summary": {"jobs_queued": 0, "jobs_running": 0, "runs_queued": 0, "runs_in_progress": 0},
            "self_hosted_runners": [],
            "queue_by_group": [],
        }
        gpu_quota = {
            "by_metric": {
                "NVIDIA_T4_GPUS": {
                    "name": "T4",
                    "usage": 18,
                    "limit": 24,
                    "regions": {
                        "us-central1": {"usage": 6, "limit": 8},
                        "us-east1": {"usage": 7, "limit": 8},
                        "us-west1": {"usage": 5, "limit": 8},
                    },
                },
                "NVIDIA_L4_GPUS": {
                    "name": "L4",
                    "usage": 2,
                    "limit": 32,
                    "regions": {
                        "us-central1": {"usage": 1, "limit": 16},
                        "us-east1": {"usage": 1, "limit": 16},
                    },
                },
            },
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_health.record_snapshot(queue_data, tmp, gpu_quota=gpu_quota)
            with open(os.path.join(tmp, ci_health.SNAPSHOTS_FILE), encoding="utf-8") as f:
                snapshot = json.loads(f.readline())

        self.assertEqual(snapshot["gpu_quota"]["us-central1"], {"usage": 6, "limit": 8})
        self.assertEqual(snapshot["gpu_quota"]["us-east1"], {"usage": 7, "limit": 8})
        self.assertEqual(snapshot["gpu_quota"]["us-west1"], {"usage": 5, "limit": 8})
        self.assertEqual(
            snapshot["gpu_quota_by_metric"]["NVIDIA_L4_GPUS"]["regions"]["us-east1"],
            {"usage": 1, "limit": 16},
        )

    def test_build_history_chart_includes_gpu_quota_when_present(self):
        snapshots = [
            {
                "timestamp": "2026-03-03T10:00:00Z",
                "jobs_queued": 0,
                "jobs_running": 0,
                "runs_queued": 0,
                "runs_in_progress": 0,
                "runner_groups": {},
                "gpu_quota_by_metric": {
                    "NVIDIA_T4_GPUS": {
                        "name": "T4",
                        "usage": 13,
                        "limit": 16,
                        "regions": {
                            "us-central1": {"usage": 6, "limit": 8},
                            "us-east1": {"usage": 7, "limit": 8},
                        },
                    },
                    "NVIDIA_L4_GPUS": {
                        "name": "L4",
                        "usage": 2,
                        "limit": 32,
                        "regions": {
                            "us-central1": {"usage": 1, "limit": 16},
                            "us-east1": {"usage": 1, "limit": 16},
                        },
                    },
                },
            },
        ]

        html = ci_health.build_history_chart(snapshots)
        self.assertIn("T4 GPU Usage", html)
        self.assertIn("L4 GPU Usage", html)
        self.assertIn("gpuQuota_canvas", html)
        self.assertIn("gpuQuota_NVIDIA_L4_GPUS_canvas", html)
        self.assertIn("us-central1", html)
        self.assertIn("us-east1", html)
        self.assertIn("Quota Limit", html)

    def test_build_history_chart_supports_legacy_t4_gpu_quota_snapshots(self):
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

    def test_statistics_range_filter_wires_parallel_and_queue_percentile_charts(self):
        """Range selector must update Average Concurrent Runners and queue percentiles."""
        config = {
            "label_groups": [],
            "runner_name_prefixes": [
                {"prefix": "linux-runner-", "name": "Linux GPU (GCP)", "self_hosted": True},
            ],
            "non_production_periods": {"runners": {}},
        }

        base = datetime.now(timezone.utc).replace(hour=12, minute=0, second=0, microsecond=0)
        jobs = []
        for offset in (3, 2):
            created = (base - ci_health.timedelta(days=offset)).strftime("%Y-%m-%dT%H:%M:%SZ")
            completed = (
                base - ci_health.timedelta(days=offset) + ci_health.timedelta(minutes=30)
            ).strftime("%Y-%m-%dT%H:%M:%SZ")
            jobs.append(
                {
                    "name": "build-linux-debug",
                    "workflow_name": "CI",
                    "run_id": offset,
                    "run_created_at": created,
                    "created_at": created,
                    "started_at": created,
                    "completed_at": completed,
                    "conclusion": "success",
                    "event": "push",
                    "head_branch": "main",
                    "labels": [],
                    "runner_name": "linux-runner-1",
                    "duration_seconds": 1800,
                    "queued_seconds": offset * 60,
                    "html_url": "",
                }
            )

        data = ci_visualization.process_jobs(jobs, config)

        with tempfile.TemporaryDirectory() as tmp:
            ci_visualization.generate_statistics(data, config, tmp)
            with open(os.path.join(tmp, "statistics.html"), encoding="utf-8") as f:
                html = f.read()

        self.assertIn("makeChart('parallelRate_canvas', 'line'", html)
        self.assertIn("_allData", html)
        self.assertIn("const allQueueWaitAvg =", html)
        self.assertIn("const allQueueWaitP95 =", html)
        self.assertIn("makeChart('queueWait_canvas', 'line'", html)


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


class TestMonthlySplit(unittest.TestCase):
    def _make_job(self, job_id, created_at):
        return {
            "id": job_id,
            "run_id": 1,
            "name": "test-job",
            "workflow_name": "CI",
            "workflow_path": ".github/workflows/ci.yml",
            "status": "completed",
            "conclusion": "success",
            "created_at": created_at,
            "started_at": created_at,
            "completed_at": created_at,
            "duration_seconds": 60,
            "queued_seconds": 5,
            "runner_name": "",
            "runner_id": 0,
            "runner_group_name": "",
            "labels": [],
            "head_branch": "main",
            "event": "push",
            "actor": "dev",
            "html_url": "",
            "run_created_at": created_at,
        }

    def test_save_monthly_data_splits_by_month(self):
        jobs = [
            self._make_job(1, "2026-02-15T10:00:00Z"),
            self._make_job(2, "2026-02-20T10:00:00Z"),
            self._make_job(3, "2026-03-01T10:00:00Z"),
            self._make_job(4, "2026-03-15T10:00:00Z"),
        ]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_monthly_data(jobs, tmp)
            feb_path = os.path.join(tmp, "ci_jobs_2026-02.json")
            mar_path = os.path.join(tmp, "ci_jobs_2026-03.json")
            self.assertTrue(os.path.exists(feb_path))
            self.assertTrue(os.path.exists(mar_path))
            with open(feb_path) as f:
                feb_data = json.load(f)
            with open(mar_path) as f:
                mar_data = json.load(f)
            self.assertEqual(len(feb_data), 2)
            self.assertEqual(len(mar_data), 2)
            self.assertEqual({j["id"] for j in feb_data}, {1, 2})
            self.assertEqual({j["id"] for j in mar_data}, {3, 4})

    def test_save_monthly_data_changed_months_filter(self):
        jobs = [
            self._make_job(1, "2026-02-15T10:00:00Z"),
            self._make_job(2, "2026-03-01T10:00:00Z"),
        ]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_monthly_data(jobs, tmp, changed_months={"2026-03"})
            self.assertFalse(os.path.exists(os.path.join(tmp, "ci_jobs_2026-02.json")))
            self.assertTrue(os.path.exists(os.path.join(tmp, "ci_jobs_2026-03.json")))

    def test_load_all_monthly_data_concatenates(self):
        jobs_feb = [self._make_job(1, "2026-02-15T10:00:00Z")]
        jobs_mar = [self._make_job(2, "2026-03-01T10:00:00Z")]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(jobs_feb, os.path.join(tmp, "ci_jobs_2026-02.json"))
            ci_job_collector.save_data(jobs_mar, os.path.join(tmp, "ci_jobs_2026-03.json"))
            all_jobs = ci_job_collector.load_all_monthly_data(tmp)
            self.assertEqual(len(all_jobs), 2)
            self.assertEqual({j["id"] for j in all_jobs}, {1, 2})

    def test_load_recent_monthly_data_returns_latest(self):
        jobs_feb = [self._make_job(1, "2026-02-15T10:00:00Z")]
        jobs_mar = [self._make_job(2, "2026-03-01T10:00:00Z")]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(jobs_feb, os.path.join(tmp, "ci_jobs_2026-02.json"))
            ci_job_collector.save_data(jobs_mar, os.path.join(tmp, "ci_jobs_2026-03.json"))
            recent = ci_job_collector.load_recent_monthly_data(tmp)
            self.assertEqual(len(recent), 1)
            self.assertEqual(recent[0]["id"], 2)

    def test_migrate_single_to_monthly(self):
        jobs = [
            self._make_job(1, "2026-02-15T10:00:00Z"),
            self._make_job(2, "2026-03-01T10:00:00Z"),
            self._make_job(3, "2026-03-15T10:00:00Z"),
        ]
        with tempfile.TemporaryDirectory() as tmp:
            single_file = os.path.join(tmp, "ci_jobs.json")
            ci_job_collector.save_data(jobs, single_file)
            out_dir = os.path.join(tmp, "monthly")
            ci_job_collector.migrate_single_to_monthly(single_file, out_dir)
            files = ci_job_collector.find_monthly_files(out_dir)
            self.assertEqual(len(files), 2)
            self.assertEqual(files[0][0], "2026-02")
            self.assertEqual(files[1][0], "2026-03")
            all_jobs = ci_job_collector.load_all_monthly_data(out_dir)
            self.assertEqual(len(all_jobs), 3)

    def test_find_monthly_files_ignores_non_monthly(self):
        with tempfile.TemporaryDirectory() as tmp:
            # Create a monthly file and a non-monthly file
            ci_job_collector.save_data([], os.path.join(tmp, "ci_jobs_2026-03.json"))
            ci_job_collector.save_data([], os.path.join(tmp, "ci_jobs.json"))
            ci_job_collector.save_data([], os.path.join(tmp, "ci_jobs_bad.json"))
            files = ci_job_collector.find_monthly_files(tmp)
            self.assertEqual(len(files), 1)
            self.assertEqual(files[0][0], "2026-03")

    def test_visualization_load_monthly_jobs(self):
        jobs_feb = [self._make_job(1, "2026-02-15T10:00:00Z")]
        jobs_mar = [self._make_job(2, "2026-03-01T10:00:00Z")]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(jobs_feb, os.path.join(tmp, "ci_jobs_2026-02.json"))
            ci_job_collector.save_data(jobs_mar, os.path.join(tmp, "ci_jobs_2026-03.json"))
            all_jobs = ci_visualization._load_monthly_jobs(tmp)
            self.assertEqual(len(all_jobs), 2)
            self.assertEqual({j["id"] for j in all_jobs}, {1, 2})

    def test_visualization_load_monthly_jobs_ignores_legacy_and_non_monthly_files(self):
        jobs_mar = [self._make_job(2, "2026-03-01T10:00:00Z")]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(jobs_mar, os.path.join(tmp, "ci_jobs_2026-03.json"))
            ci_job_collector.save_data([self._make_job(99, "2026-01-01T10:00:00Z")], os.path.join(tmp, "ci_jobs.json"))
            ci_job_collector.save_data([self._make_job(100, "2026-01-02T10:00:00Z")], os.path.join(tmp, "ci_jobs_backup.json"))

            all_jobs = ci_visualization._load_monthly_jobs(tmp)

            self.assertEqual(len(all_jobs), 1)
            self.assertEqual(all_jobs[0]["id"], 2)

    def test_visualization_load_monthly_jobs_fails_on_invalid_file(self):
        jobs_feb = [self._make_job(1, "2026-02-15T10:00:00Z")]
        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(jobs_feb, os.path.join(tmp, "ci_jobs_2026-02.json"))
            with open(os.path.join(tmp, "ci_jobs_2026-03.json"), "w", encoding="utf-8") as f:
                f.write("{invalid json")
            with contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as exc:
                    ci_visualization._load_monthly_jobs(tmp)
            self.assertEqual(exc.exception.code, 2)

    def test_job_month_extraction(self):
        self.assertEqual(ci_job_collector.job_month({"created_at": "2026-03-15T10:00:00Z"}), "2026-03")
        self.assertEqual(ci_job_collector.job_month({"created_at": None}), "unknown")
        self.assertEqual(ci_job_collector.job_month({}), "unknown")

    def test_monthly_main_handles_cross_month_jobs(self):
        existing_feb_job = self._make_job(1, "2026-02-15T10:00:00Z")
        existing_mar_job = self._make_job(2, "2026-03-01T10:00:00Z")
        cross_month_job = self._make_job(3, "2026-03-01T00:05:00Z")
        run = {
            "id": 99,
            "name": "CI",
            "created_at": "2026-02-28T23:59:00Z",
        }

        with tempfile.TemporaryDirectory() as tmp:
            ci_job_collector.save_data(
                [existing_feb_job], os.path.join(tmp, "ci_jobs_2026-02.json")
            )
            ci_job_collector.save_data(
                [existing_mar_job], os.path.join(tmp, "ci_jobs_2026-03.json")
            )

            with mock.patch.object(
                ci_job_collector, "fetch_completed_runs", return_value=[run]
            ), mock.patch.object(
                ci_job_collector, "collect_jobs", return_value=([cross_month_job], 0, [])
            ), mock.patch.object(
                ci_job_collector, "resolve_workflow_id", return_value=123
            ), mock.patch.object(
                ci_job_collector.sys,
                "argv",
                ["ci_job_collector.py", "--output-dir", tmp, "--workflow", "CI"],
            ):
                ci_job_collector.main()

            with open(os.path.join(tmp, "ci_jobs_2026-02.json"), encoding="utf-8") as f:
                feb_data = json.load(f)
            with open(os.path.join(tmp, "ci_jobs_2026-03.json"), encoding="utf-8") as f:
                mar_data = json.load(f)

            self.assertEqual({j["id"] for j in feb_data}, {1})
            self.assertEqual({j["id"] for j in mar_data}, {2, 3})


class TestHostedRunnerUsage(unittest.TestCase):
    def test_classify_hosted_label_picks_first_hosted(self):
        self.assertEqual(
            ci_hosted_runner_usage.classify_hosted_label(["ubuntu-latest"]),
            "ubuntu-latest",
        )
        self.assertEqual(
            ci_hosted_runner_usage.classify_hosted_label(
                ["ubuntu-22.04", "x64"]
            ),
            "ubuntu-22.04",
        )

    def test_classify_hosted_label_rejects_self_hosted(self):
        # Self-hosted runners often carry an `ubuntu-*` *image* label too,
        # but the `self-hosted` marker disqualifies them from the cap.
        self.assertIsNone(
            ci_hosted_runner_usage.classify_hosted_label(
                ["self-hosted", "Linux", "SM80Plus"]
            )
        )
        self.assertIsNone(
            ci_hosted_runner_usage.classify_hosted_label(
                ["self-hosted", "ubuntu-22.04"]
            )
        )

    def test_classify_hosted_label_returns_none_for_no_hosted(self):
        self.assertIsNone(ci_hosted_runner_usage.classify_hosted_label([]))
        self.assertIsNone(ci_hosted_runner_usage.classify_hosted_label(["custom"]))

    def test_classify_hosted_label_recognizes_all_three_pools(self):
        for label in ("ubuntu-24.04-arm", "macos-latest", "windows-2022"):
            self.assertEqual(
                ci_hosted_runner_usage.classify_hosted_label([label]), label
            )

    def test_summarize_groups_and_sorts(self):
        jobs = [
            {"workflow_name": "CI", "job_name": "filter", "hosted_label": "ubuntu-latest"},
            {"workflow_name": "CI", "job_name": "label", "hosted_label": "ubuntu-latest"},
            {"workflow_name": "CMake Options", "job_name": "m1", "hosted_label": "ubuntu-22.04"},
            {"workflow_name": "CI", "job_name": "mac", "hosted_label": "macos-latest"},
        ]
        s = ci_hosted_runner_usage.summarize(jobs)
        self.assertEqual(s["total"], 4)
        # Sorted by count desc, then name asc.
        self.assertEqual(s["by_workflow"][0], {"name": "CI", "count": 3})
        self.assertEqual(s["by_workflow"][1], {"name": "CMake Options", "count": 1})
        self.assertEqual(s["by_label"][0], {"label": "ubuntu-latest", "count": 2})

    def test_summarize_handles_empty(self):
        s = ci_hosted_runner_usage.summarize([])
        self.assertEqual(s, {"total": 0, "by_workflow": [], "by_label": []})

    def test_collect_hosted_jobs_filters_status_and_self_hosted(self):
        run = {"id": 1, "name": "CI"}

        def fake_fetch(repo, run_id):
            return (
                [
                    # hosted, in_progress — kept
                    {"status": "in_progress", "labels": ["ubuntu-latest"], "name": "a"},
                    # hosted, queued — filtered out by status_filter
                    {"status": "queued", "labels": ["ubuntu-latest"], "name": "b"},
                    # self-hosted — filtered out by classifier
                    {"status": "in_progress", "labels": ["self-hosted", "Linux", "GPU"], "name": "c"},
                    # hosted windows, in_progress — kept
                    {"status": "in_progress", "labels": ["windows-latest"], "name": "d"},
                ],
                None,
            )

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_fetch
        ):
            jobs, errors = ci_hosted_runner_usage.collect_hosted_jobs(
                "shader-slang/slang", [run], "in_progress"
            )
        self.assertEqual(len(jobs), 2)
        self.assertEqual(errors, 0)
        labels = sorted(j["hosted_label"] for j in jobs)
        self.assertEqual(labels, ["ubuntu-latest", "windows-latest"])
        for j in jobs:
            self.assertEqual(j["workflow_name"], "CI")

    def test_collect_hosted_jobs_counts_fetch_errors(self):
        """Fetch failures must be surfaced, not silently undercounted."""
        runs = [{"id": 1, "name": "CI"}, {"id": 2, "name": "CI"}]

        def fake_fetch(repo, run_id):
            if run_id == 1:
                return (
                    [{"status": "in_progress", "labels": ["ubuntu-latest"], "name": "a"}],
                    None,
                )
            return (None, "HTTP 502")

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_fetch
        ):
            jobs, errors = ci_hosted_runner_usage.collect_hosted_jobs(
                "shader-slang/slang", runs, "in_progress"
            )
        self.assertEqual(len(jobs), 1)
        self.assertEqual(errors, 1)

    def test_sample_counts_queued_jobs_inside_in_progress_runs(self):
        """A run whose top-level status is `in_progress` can still carry
        `queued` jobs waiting on a hosted runner. That is exactly the
        cap-exhaustion leading edge we need to surface — make sure those
        queued jobs land in the snapshot's `queued` bucket and not in
        the void.
        """
        in_progress_run = {"id": 42, "name": "CI"}

        def fake_in_progress(repo):
            return [in_progress_run], None

        def fake_queued(repo):
            # No workflow runs have top-level status=queued yet —
            # the queueing is happening *inside* an in_progress run.
            return [], None

        def fake_jobs(repo, run_id):
            assert run_id == 42
            return (
                [
                    {"status": "in_progress", "labels": ["ubuntu-latest"], "name": "early"},
                    {"status": "queued", "labels": ["ubuntu-latest"], "name": "stuck-1"},
                    {"status": "queued", "labels": ["windows-latest"], "name": "stuck-2"},
                ],
                None,
            )

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        self.assertEqual(snap["in_progress"]["total"], 1)
        self.assertEqual(snap["queued"]["total"], 2)
        queued_labels = {x["label"]: x["count"] for x in snap["queued"]["by_label"]}
        self.assertEqual(queued_labels, {"ubuntu-latest": 1, "windows-latest": 1})

    def test_sample_dedupes_runs_listed_in_both_endpoints(self):
        """A run transitioning queued -> in_progress can appear in both
        list endpoints during sampling. Its jobs must not be counted
        twice.
        """
        run = {"id": 7, "name": "CI"}

        def fake_in_progress(repo):
            return [run], None

        def fake_queued(repo):
            return [run], None

        call_count = {"n": 0}

        def fake_jobs(repo, run_id):
            call_count["n"] += 1
            return (
                [{"status": "in_progress", "labels": ["ubuntu-latest"], "name": "a"}],
                None,
            )

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        self.assertEqual(call_count["n"], 1)
        self.assertEqual(snap["in_progress"]["total"], 1)

    def test_sample_hosted_runner_usage_marks_partial_on_errors(self):
        run = {"id": 99, "name": "CI"}

        def fake_in_progress(repo):
            return [run], None

        def fake_queued(repo):
            return [], None

        def fake_jobs(repo, run_id):
            return (None, "HTTP 500")

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        self.assertTrue(snap.get("partial"))
        self.assertEqual(snap.get("fetch_errors"), 1)

    def test_sample_marks_partial_on_listing_endpoint_failure(self):
        """A 5xx on the runs-listing endpoint must surface as partial,
        not as a false-healthy zero. Otherwise a transient API error
        masks the very incident this sampler exists to detect.
        """

        def fake_in_progress(repo):
            return [], "HTTP 502 listing in_progress runs"

        def fake_queued(repo):
            return [], None

        def fake_jobs(repo, run_id):
            return ([], None)

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        self.assertTrue(snap.get("partial"))
        self.assertEqual(snap["in_progress"]["total"], 0)
        self.assertEqual(snap["queued"]["total"], 0)
        self.assertIn("HTTP 502 listing in_progress runs", snap.get("list_errors", []))

    def test_format_summary_surfaces_partial(self):
        """`--json` and the human-readable summary must carry the same
        degraded signal — the text mode previously printed `0 / cap`
        without any indication the sample was undercounted.
        """
        snapshot = {
            "cap": 20,
            "in_progress": {"total": 0, "by_workflow": [], "by_label": []},
            "queued": {"total": 0, "by_workflow": [], "by_label": []},
            "partial": True,
            "fetch_errors": 2,
            "list_errors": ["HTTP 502 listing in_progress runs"],
        }
        out = ci_hosted_runner_usage.format_summary(snapshot)
        self.assertIn("WARNING", out)
        self.assertIn("partial", out.lower())
        self.assertIn("job fetch failures: 2", out)
        self.assertIn("HTTP 502 listing in_progress runs", out)

    def test_sample_dedupe_skips_runs_with_missing_id(self):
        """Don't collapse multiple `id: None` runs into one entry."""

        def fake_in_progress(repo):
            return [{"name": "broken-1"}, {"name": "broken-2"}], None

        def fake_queued(repo):
            return [], None

        call_count = {"n": 0}

        def fake_jobs(repo, run_id):
            call_count["n"] += 1
            return ([], None)

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        # Both malformed runs must be dropped (not merged into a single
        # None-id entry and dispatched to fetch_jobs_for_run).
        self.assertEqual(call_count["n"], 0)
        self.assertEqual(snap["in_progress"]["total"], 0)

    def test_sample_hosted_runner_usage_shape(self):
        in_progress_run = {"id": 10, "name": "CI"}
        queued_run = {"id": 11, "name": "CMake Options"}

        def fake_in_progress(repo):
            return [in_progress_run], None

        def fake_queued(repo):
            return [queued_run], None

        def fake_jobs(repo, run_id):
            if run_id == 10:
                return (
                    [
                        {"status": "in_progress", "labels": ["ubuntu-latest"], "name": "filter"},
                        {"status": "in_progress", "labels": ["self-hosted", "Linux", "GPU"], "name": "gpu"},
                    ],
                    None,
                )
            if run_id == 11:
                return (
                    [
                        {"status": "queued", "labels": ["ubuntu-latest"], "name": "matrix-0"},
                        {"status": "queued", "labels": ["ubuntu-latest"], "name": "matrix-1"},
                    ],
                    None,
                )
            return ([], None)

        with mock.patch.object(
            ci_hosted_runner_usage, "fetch_in_progress_runs", side_effect=fake_in_progress
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_queued_runs", side_effect=fake_queued
        ), mock.patch.object(
            ci_hosted_runner_usage, "fetch_jobs_for_run", side_effect=fake_jobs
        ):
            snap = ci_hosted_runner_usage.sample_hosted_runner_usage(
                "shader-slang/slang", cap=20
            )

        self.assertEqual(snap["cap"], 20)
        self.assertEqual(snap["in_progress"]["total"], 1)
        self.assertEqual(snap["in_progress"]["by_label"], [{"label": "ubuntu-latest", "count": 1}])
        self.assertEqual(snap["queued"]["total"], 2)
        self.assertEqual(snap["queued"]["by_workflow"], [{"name": "CMake Options", "count": 2}])


class TestHostedLabelPalette(unittest.TestCase):
    """Regression: variants must yield valid 6-digit `#RRGGBB` colors.

    The original implementation appended an alpha suffix (`"BB"`), then
    the chart code appended another `"55"` for the background fill,
    producing invalid 10-digit hex like `#0d6efdBB55`.
    """

    def _is_six_digit_hex(self, color):
        return (
            isinstance(color, str)
            and len(color) == 7
            and color.startswith("#")
            and all(c in "0123456789abcdefABCDEF" for c in color[1:])
        )

    def test_palette_returns_six_digit_hex_for_all_variants(self):
        labels = [
            "ubuntu-latest",
            "ubuntu-22.04",
            "ubuntu-24.04",
            "ubuntu-24.04-arm",
            "ubuntu-22.04-arm",
            "windows-latest",
            "windows-2022",
            "macos-latest",
            "macos-13",
            "self-hosted-fallback",
        ]
        palette = ci_health._hosted_label_palette(labels)
        for lbl, color in palette.items():
            self.assertTrue(
                self._is_six_digit_hex(color),
                msg=f"{lbl!r} → {color!r} is not a 6-digit #RRGGBB",
            )

    def test_palette_handles_unknown_prefix_without_keyerror(self):
        """If HOSTED_LABEL_PREFIXES gains a new entry not mirrored in
        HOSTED_LABEL_PALETTE, palette construction must fall back to a
        neutral color rather than raising KeyError mid-snapshot."""
        original = ci_hosted_runner_usage.HOSTED_LABEL_PREFIXES
        with mock.patch.object(
            ci_health,
            "HOSTED_LABEL_ORDER",
            original + ("freebsd-",),
        ):
            palette = ci_health._hosted_label_palette(["freebsd-14"])
        self.assertTrue(
            self._is_six_digit_hex(palette["freebsd-14"]),
            msg=f"unknown-prefix label got invalid color {palette['freebsd-14']!r}",
        )

    def test_variants_under_same_pool_get_distinct_colors(self):
        palette = ci_health._hosted_label_palette(
            ["ubuntu-latest", "ubuntu-22.04", "ubuntu-24.04"]
        )
        colors = list(palette.values())
        self.assertEqual(len(set(colors)), 3)


class TestHostedRunnerRender(unittest.TestCase):
    def _snapshot(self, in_use, cap=20, queued=0, by_workflow=None):
        return {
            "cap": cap,
            "in_progress": {
                "total": in_use,
                "by_workflow": by_workflow or [],
                "by_label": [],
            },
            "queued": {"total": queued, "by_workflow": [], "by_label": []},
        }

    def test_render_banner_ok_under_threshold(self):
        html = ci_health.render_hosted_runner_usage(self._snapshot(in_use=10))
        self.assertIn("OK", html)
        self.assertIn("10 / 20", html)

    def test_render_banner_high_at_or_above_80pct(self):
        html = ci_health.render_hosted_runner_usage(self._snapshot(in_use=16))
        self.assertIn("HIGH", html)

    def test_render_banner_at_cap_with_queue_is_alarm(self):
        html = ci_health.render_hosted_runner_usage(
            self._snapshot(in_use=20, queued=5)
        )
        self.assertIn("AT CAP", html)

    def test_render_banner_at_cap_with_no_queue_is_high_not_alarm(self):
        # 100% in-use is uncomfortable but not the smoking-gun signature
        # of starvation — that's 100% with backlog queued behind.
        html = ci_health.render_hosted_runner_usage(self._snapshot(in_use=20))
        self.assertIn("HIGH", html)
        self.assertNotIn("AT CAP", html)

    def test_render_handles_missing_snapshot(self):
        html = ci_health.render_hosted_runner_usage(None)
        self.assertIn("unavailable", html.lower())

    def test_render_banner_partial_overrides_severity(self):
        """A partial sample is a known undercount — render PARTIAL
        instead of computing OK/HIGH/AT CAP from numbers we know are
        low. Otherwise the dashboard reads false-healthy during an
        Actions API failure.
        """
        snap = self._snapshot(in_use=2, queued=0)
        snap["partial"] = True
        html = ci_health.render_hosted_runner_usage(snap)
        self.assertIn("PARTIAL", html)
        self.assertNotIn("OK", html)
        self.assertIn("partial", html.lower())

    def test_build_hosted_runner_chart_none_when_no_data(self):
        snapshots = [{"timestamp": "2026-05-13T10:00:00Z"}]
        self.assertIsNone(ci_health._build_hosted_runner_chart(snapshots))

    def test_build_hosted_runner_chart_stacks_by_label(self):
        snapshots = [
            {
                "timestamp": "2026-05-13T10:00:00Z",
                "hosted_runner_usage": {
                    "cap": 20,
                    "in_progress": {
                        "total": 4,
                        "by_label": [
                            {"label": "ubuntu-latest", "count": 3},
                            {"label": "macos-latest", "count": 1},
                        ],
                        "by_workflow": [],
                    },
                    "queued": {"total": 2, "by_workflow": [], "by_label": []},
                },
            },
            {
                "timestamp": "2026-05-13T10:15:00Z",
                "hosted_runner_usage": {
                    "cap": 20,
                    "in_progress": {
                        "total": 6,
                        "by_label": [
                            {"label": "ubuntu-latest", "count": 4},
                            {"label": "windows-latest", "count": 2},
                        ],
                        "by_workflow": [],
                    },
                    "queued": {"total": 0, "by_workflow": [], "by_label": []},
                },
            },
        ]
        chart = ci_health._build_hosted_runner_chart(snapshots)
        self.assertIsNotNone(chart)
        # Ordering: ubuntu, then macos, then windows.
        self.assertEqual(chart["labels"], ["ubuntu-latest", "macos-latest", "windows-latest"])
        self.assertEqual(chart["cap"], 20)
        # macos absent in the 2nd snapshot collapses to 0, not None.
        self.assertEqual(chart["label_series"]["ubuntu-latest"], [3, 4])
        self.assertEqual(chart["label_series"]["macos-latest"], [1, 0])
        self.assertEqual(chart["label_series"]["windows-latest"], [0, 2])
        self.assertEqual(chart["queued_series"], [2, 0])
        self.assertEqual(chart["total_series"], [4, 6])

    def test_build_hosted_runner_chart_partial_renders_as_gap(self):
        """A partial sample is a known undercount; the history chart
        must plot None (a gap) rather than the false-low number.
        """
        snapshots = [
            {
                "timestamp": "2026-05-13T10:00:00Z",
                "hosted_runner_usage": {
                    "cap": 20,
                    "in_progress": {
                        "total": 5,
                        "by_label": [{"label": "ubuntu-latest", "count": 5}],
                        "by_workflow": [],
                    },
                    "queued": {"total": 0, "by_workflow": [], "by_label": []},
                },
            },
            {
                "timestamp": "2026-05-13T10:15:00Z",
                "hosted_runner_usage": {
                    "cap": 20,
                    "partial": True,
                    "in_progress": {
                        "total": 0,
                        "by_label": [],
                        "by_workflow": [],
                    },
                    "queued": {"total": 0, "by_workflow": [], "by_label": []},
                },
            },
        ]
        chart = ci_health._build_hosted_runner_chart(snapshots)
        self.assertIsNotNone(chart)
        self.assertEqual(chart["total_series"], [5, None])
        self.assertEqual(chart["queued_series"], [0, None])
        self.assertEqual(chart["label_series"]["ubuntu-latest"], [5, None])

    def test_build_hosted_runner_chart_marks_missing_snapshots_as_gap(self):
        # Older snapshots without the new field should be rendered as
        # None gaps so the chart begins on the first real data point.
        snapshots = [
            {"timestamp": "2026-05-13T09:00:00Z"},
            {
                "timestamp": "2026-05-13T09:15:00Z",
                "hosted_runner_usage": {
                    "cap": 20,
                    "in_progress": {
                        "total": 1,
                        "by_label": [{"label": "ubuntu-latest", "count": 1}],
                        "by_workflow": [],
                    },
                    "queued": {"total": 0, "by_workflow": [], "by_label": []},
                },
            },
        ]
        chart = ci_health._build_hosted_runner_chart(snapshots)
        self.assertEqual(chart["label_series"]["ubuntu-latest"], [None, 1])
        self.assertEqual(chart["queued_series"], [None, 0])
        self.assertEqual(chart["total_series"], [None, 1])

    def test_build_history_chart_includes_hosted_runner_section_when_data_present(self):
        snapshots = [
            {
                "timestamp": "2026-05-13T10:00:00Z",
                "jobs_queued": 0,
                "jobs_running": 0,
                "runs_queued": 0,
                "runs_in_progress": 0,
                "runner_groups": {},
                "hosted_runner_usage": {
                    "cap": 20,
                    "in_progress": {
                        "total": 1,
                        "by_label": [{"label": "ubuntu-latest", "count": 1}],
                        "by_workflow": [],
                    },
                    "queued": {"total": 0, "by_workflow": [], "by_label": []},
                },
            }
        ]
        html = ci_health.build_history_chart(snapshots)
        self.assertIn("hostedRunnerHistory", html)
        self.assertIn("GitHub-Hosted Runner Usage", html)

    def test_build_history_chart_omits_hosted_runner_section_when_no_data(self):
        snapshots = [
            {
                "timestamp": "2026-05-13T10:00:00Z",
                "jobs_queued": 0,
                "jobs_running": 0,
                "runs_queued": 0,
                "runs_in_progress": 0,
                "runner_groups": {},
            }
        ]
        html = ci_health.build_history_chart(snapshots)
        # The <canvas id="hostedRunnerHistory_canvas"> is what gates the
        # client-side chart render. When no hosted-runner data exists,
        # the section header must be absent so the canvas isn't emitted.
        self.assertNotIn('id="hostedRunnerHistory_canvas"', html)
        self.assertNotIn("GitHub-Hosted Runner Usage", html)

    def test_record_snapshot_persists_hosted_runner_usage(self):
        usage = {
            "cap": 20,
            "in_progress": {"total": 7, "by_workflow": [], "by_label": []},
            "queued": {"total": 0, "by_workflow": [], "by_label": []},
        }
        with tempfile.TemporaryDirectory() as tmp:
            ci_health.record_snapshot(
                {"summary": {}}, tmp, hosted_runner_usage=usage
            )
            with open(os.path.join(tmp, ci_health.SNAPSHOTS_FILE), encoding="utf-8") as f:
                snapshot = json.loads(f.readline())
        self.assertEqual(snapshot["hosted_runner_usage"], usage)


if __name__ == "__main__":
    unittest.main()
