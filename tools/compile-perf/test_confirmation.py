"""Test two-batch alert decisions and the archive crossing the CI job boundary."""
import contextlib
import copy
import io
import json
import os
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import bench
import confirm
from lib import analyze
import trend


def record(value, workload="minimal", size=64):
    return {"workload": workload, "size": size, "ok": True, "samples": 5, "warmup": 1,
            "sampling_strategy": "interleaved", "timer_schema": "detailed",
            "timers": {"compileInner": bench.stats([value] * 5)}}


def plan(value=120.0, counter="compileInner", workload="minimal", baseline=100.0):
    tier = trend.classify_metric(value / baseline, value - baseline, 1.10, 1.05,
                                 trend.abs_floor_for(counter, 2.0))
    return {"label": "2026-01-04-test", "runner": "test-runner", "metrics": {},
            "thresholds": {"rel": 1.10, "warn_rel": 1.05, "abs": 2.0},
            "baseline_labels": ["a", "b", "c"], "notes": [],
            "regressions": [[workload, counter, baseline, value,
                             value / baseline, value - baseline]] if tier == "error" else [],
            "warnings": [[workload, counter, baseline, value,
                          value / baseline, value - baseline]] if tier == "warning" else []}


class ConfirmationDecisionTests(unittest.TestCase):
    def test_both_batches_must_cross_each_severity_threshold(self):
        for first, second, expected in (
                (120, 120, (1, 0, 0)), (120, 100, (0, 0, 1)),
                (107, 107, (0, 1, 0)), (107, 100, (0, 0, 1)),
                (120, 107, (0, 1, 0)), (107, 120, (0, 1, 0))):
            with self.subTest(first=first, second=second):
                result = confirm.confirmed_changes(plan(first), [record(first)], [record(second)])
                self.assertEqual(tuple(map(len, result)), expected)

    def test_another_counter_cannot_confirm_the_original_counter(self):
        second = record(100)
        second["timers"]["SemanticChecking"] = bench.stats([500] * 5)
        result = confirm.confirmed_changes(plan(), [record(120)], [second])
        self.assertEqual(tuple(map(len, result)), (0, 0, 1))

    def test_failed_missing_incomplete_and_incompatible_reruns_are_not_recovery(self):
        variants = [[], [dict(record(100), ok=False)]]
        for field, value in (("size", 128), ("timer_schema", "coarse"),
                             ("sampling_strategy", "consecutive"), ("samples", 4),
                             ("warmup", 0)):
            variants.append([dict(record(100), **{field: value})])
        for stat in (None, bench.stats([100] * 4),
                     {"n": 5, "samples": [100] * 5, "median": float("nan")}):
            variants.append([dict(record(100), timers={"compileInner": stat})])
        for repeated in variants:
            with self.subTest(repeated=repeated), self.assertRaises(ValueError):
                confirm.confirmed_changes(plan(), [record(120)], repeated)

    def test_memory_uses_existing_unit_floor(self):
        # minimal promotes RSS to a judged memory counter.
        workload = "minimal"
        first = record(100, workload)
        first["rss_kb"] = bench.stats([12000] * 5)
        second = copy.deepcopy(first)
        second["rss_kb"] = bench.stats([10700] * 5)
        result = confirm.confirmed_changes(plan(12000, "peakRssKb", workload, 10000),
                                           [first], [second])
        self.assertEqual(tuple(map(len, result)), (0, 0, 1))

    def test_reruns_each_affected_workload_once_at_the_original_size(self):
        original = [record(120), record(120, "parse", 128), record(100, "serialize")]
        candidates = plan()
        candidates["warnings"] = [["minimal", "SemanticChecking", 100, 107, 1.07, 7],
                                   ["parse", "compileInner", 100, 107, 1.07, 7]]
        with patch.object(bench, "run_workloads", return_value=[]) as run:
            confirm.rerun(candidates, original, "slangc")
        run.assert_called_once()
        args = run.call_args.args
        self.assertEqual([(spec.name, size) for spec, size in args[1]],
                         [("minimal", 64), ("parse", 128)])
        self.assertEqual(args[2:4], (5, 1))

    def test_clean_run_does_not_start_a_compiler(self):
        with patch.object(bench, "run_workloads") as run:
            self.assertEqual(confirm.rerun(plan(100), [record(100)], "slangc"), [])
        run.assert_not_called()

    def test_api_workload_receives_driver_and_library(self):
        workload = "api_module_graph_bin"
        with patch.object(bench, "build_api_driver", return_value="driver"), \
                patch.object(bench, "find_libslang", return_value="library"), \
                patch.object(bench, "run_workloads", return_value=[]) as run:
            confirm.rerun(plan(workload=workload), [record(120, workload)], "slangc")
        self.assertEqual(run.call_args.kwargs["api"],
                         {"driver": "driver", "libslang": "library"})


class ConfirmationArchiveTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="confirmation_test_")
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.label = "2026-01-04-test"
        self.directory = self.root / "daily" / self.label
        self.directory.mkdir(parents=True)
        self.args = SimpleNamespace(results=self.root, label=self.label, slangc=Path("slangc"))
        self.write("results.json", [record(120)])
        self.write("meta.json", {"commit": "test", "runner": "test-runner"})
        metrics = analyze.point_metrics(str(self.directory / "results.json"))
        points = []
        for day in range(1, 5):
            points.append({"label": f"2026-01-0{day}-test", "date": f"2026-01-0{day}",
                           "kind": "daily", "runner": "test-runner", "metrics": {
                               **metrics, "minimal|compileInner": 120 if day == 4 else 100}})
        (self.root / "tracking").mkdir()
        self.tracking = self.root / "tracking/tracking.json"
        self.tracking.write_text(json.dumps({"runner": "test-runner", "points": points}))
        env = {"GITHUB_OUTPUT": str(self.root / "output"),
               "GITHUB_STEP_SUMMARY": str(self.root / "summary"), "GITHUB_ACTIONS": "true"}
        mock = patch.dict(os.environ, env)
        mock.start()
        self.addCleanup(mock.stop)

    def write(self, name, value):
        (self.directory / name).write_text(json.dumps(value), encoding="utf-8")

    def collect(self, repeated):
        with patch.object(confirm, "rerun", return_value=repeated) as run:
            confirm.measure(self.args, self.directory)
        run.assert_called_once()

    def render(self):
        output = io.StringIO()
        code = 0
        with contextlib.redirect_stdout(output):
            try:
                confirm.report(self.args, self.directory)
            except SystemExit as exc:
                code = exc.code
        return code, output.getvalue()

    def test_frozen_baseline_and_original_samples_survive_rerun(self):
        before = (self.directory / "results.json").read_bytes()
        self.collect([record(120)])
        self.assertEqual((self.directory / "results.json").read_bytes(), before)
        # Planning must not publish candidate warnings or regression annotations.
        self.assertFalse((self.root / "output").exists())
        self.assertFalse((self.root / "summary").exists())
        # Remote reporting must not consult changed rolling history.
        self.tracking.write_text("not even JSON")
        code, output = self.render()
        self.assertEqual(code, trend.EXIT_REGRESSION)
        self.assertIn("1 confirmed regression(s)", output)
        self.assertIn("::error title=Perf regressions", output)
        archive = analyze.read_json(self.directory / "confirmation.json")
        self.assertEqual(archive["plan"]["regressions"][0][2], 100)
        self.assertEqual(archive["repeated"][0]["timers"]["compileInner"]["samples"], [120]*5)

    def test_recovery_is_archived_without_performance_alarm(self):
        self.collect([record(100)])
        code, output = self.render()
        self.assertEqual(code, 0)
        self.assertIn("1 not reproduced", output)
        self.assertNotIn("::error", output)
        self.assertEqual((self.root / "output").read_text().strip(), "warnings=0")

    def test_warning_confirmation_keeps_warning_output_contract(self):
        self.collect([record(107)])
        code, output = self.render()
        self.assertEqual(code, 0)
        self.assertIn("1 confirmed warning(s)", output)
        self.assertEqual((self.root / "output").read_text().strip(), "warnings=1")

    def test_failed_rerun_is_preserved_and_cannot_report_clean(self):
        failed = dict(record(100), ok=False)
        with self.assertRaises(SystemExit) as error:
            self.collect([failed])
        self.assertEqual(error.exception.code, trend.EXIT_CANNOT_EVALUATE)
        archive = analyze.read_json(self.directory / "confirmation.json")
        self.assertEqual(archive["repeated"], [failed])
        self.assertEqual(archive["status"], "incomplete")
        with self.assertRaises(ValueError):
            self.render()

    def test_stale_confirmation_is_rejected(self):
        self.collect([record(100)])
        self.write("results.json", [record(101)])
        with self.assertRaisesRegex(ValueError, "does not belong"):
            self.render()

    def test_archive_from_previous_workflow_attempt_is_rejected(self):
        self.collect([record(100)])
        with patch.dict(os.environ, {"GITHUB_RUN_ATTEMPT": "other-attempt"}), \
                self.assertRaisesRegex(ValueError, "workflow attempt"):
            self.render()

    def test_missing_archive_is_evaluation_failure_not_regression(self):
        with patch.object(sys, "argv", ["confirm.py", "report", "--results", str(self.root),
                                       "--label", self.label]), self.assertRaises(SystemExit) as error:
            confirm.main()
        self.assertEqual(error.exception.code, trend.EXIT_CANNOT_EVALUATE)

    def test_interruption_cannot_reuse_previous_successful_archive(self):
        self.collect([record(100)])
        with patch.object(confirm, "rerun", side_effect=KeyboardInterrupt), \
                self.assertRaises(KeyboardInterrupt):
            confirm.measure(self.args, self.directory)
        with self.assertRaisesRegex(ValueError, "did not complete"):
            self.render()

    def test_clean_plan_preserves_coverage_notes_without_running_compiler(self):
        series = analyze.read_json(self.tracking)
        # Three baseline points exist, but their missing sampling provenance
        # prevents comparison; the absence of candidates is not hidden.
        for point in series["points"][:-1]:
            point["metrics"].pop(f"minimal|{analyze.SAMPLING_MARKER}")
        self.tracking.write_text(json.dumps(series))
        with patch.object(bench, "run_workloads") as run:
            confirm.measure(self.args, self.directory)
        run.assert_not_called()
        code, output = self.render()
        self.assertEqual(code, 0)
        self.assertIn("not judged", output)
        self.assertIn("::warning title=Perf comparison coverage", output)


if __name__ == "__main__":
    unittest.main()
