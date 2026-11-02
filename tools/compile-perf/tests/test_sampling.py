"""Exercise sampling order and failure/provenance contracts without a compiler.

The fake compiler reports a distinct value on every visit, so the tests can
detect extra setup, repeated warmups, lost samples, and accidental aggregation
across different workload sizes. Real compiler smoke runs complement these
deterministic checks.
"""
import contextlib
import io
import json
import os
from pathlib import Path
import tempfile
import sys
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import bench
from lib import analyze
import trend


def spec(name, mode="target", expected_diags=()):
    """Make a workload whose fake compiler only needs to report compileInner."""
    return SimpleNamespace(name=name, bucket="test", mode=mode,
                           primary_timers=["compileInner"], expected_diagnostics=expected_diags)


class SamplingTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="perf_sampling_test_")
        self.addCleanup(self.directory.cleanup)
        self.events = []
        self.visits = {}
        self.responses = {}
        self.setup_failures = set()
        self.prepare_failures = set()
        for name, replacement in (
                ("run_once", self.compile), ("build_commands", self.commands),
                ("resolve_perf_flag", lambda _: "-report-detailed-perf-benchmark")):
            mock = patch.object(bench, name, side_effect=replacement)
            mock.start()
            self.addCleanup(mock.stop)
        for target, name, replacement in (
                (bench.corpus, "materialize", self.materialize),
                (bench.corpus, "prepared_files", lambda _: ["test.slang"]),
                (bench.subprocess, "run", self.setup_command)):
            mock = patch.object(target, name, side_effect=replacement)
            mock.start()
            self.addCleanup(mock.stop)

    def materialize(self, workload, size, directory):
        self.events.append(("prepare", workload.name, size))
        if workload.name in self.prepare_failures:
            raise FileNotFoundError("missing corpus")
        return ["test.slang"]

    def commands(self, slangc, workload, src_dir, files, out_dir, size, api, perf_flag):
        return {"setup": [["setup", workload.name, str(size)]],
                "timed": [workload.name, str(size), perf_flag]}

    def setup_command(self, command, **kwargs):
        name, size = command[1], int(command[2])
        self.events.append(("setup", name, size))
        if name in self.setup_failures:
            raise bench.subprocess.TimeoutExpired(command, 600)
        return SimpleNamespace(returncode=0)

    def compile(self, command):
        key = command[0], int(command[1])
        visit = self.visits.get(key, 0) + 1
        self.visits[key] = visit
        self.events.append(("compile", *key))
        response = self.responses.get((*key, visit))
        if isinstance(response, Exception):
            raise response
        if response is not None:
            return response
        value = visit + key[1] / 1000
        return (0, value + 10, f"[*] compileInner 1 {value}ms\n"
                f"[MEM] sessionKb\t{visit * 100}kb\n", visit * 1000)

    def measure(self, cases, samples=5, warmup=1, **kwargs):
        return bench.run_workloads("slangc", cases, samples, warmup,
                                   self.directory.name, self.directory.name, **kwargs)

    def test_interleaves_sizes_and_preserves_raw_samples(self):
        a, b = spec("A"), spec("B", mode="link")
        cases = [(a, 1), (a, 2), (b, 1)]
        results = self.measure(cases)
        keys = [(s.name, n) for s, n in cases]
        self.assertEqual([e for e in self.events if e[0] == "compile"],
                         [("compile", *key) for _ in range(6) for key in keys])
        self.assertEqual(self.events[:6],
                         [event for key in keys for event in (("prepare", *key), ("setup", *key))])
        self.assertEqual(len(self.events), 6 + 6 * len(cases))
        for record, (_, size) in zip(results, cases):
            self.assertTrue(record["ok"])
            self.assertEqual(record["sampling_strategy"], "interleaved")
            self.assertEqual(record["timer_schema"], "detailed")
            self.assertEqual(record["timers"]["compileInner"]["samples"],
                             [v + size / 1000 for v in range(2, 7)])
            self.assertEqual(record["timers"]["compileInner"]["median"], 4 + size / 1000)
            self.assertEqual(record["memory"]["sessionKb"]["samples"], [200, 300, 400, 500, 600])
            self.assertEqual(record["rss_kb"]["samples"], [2000, 3000, 4000, 5000, 6000])
            self.assertEqual(record["wall_ms"]["n"], 5)

    def test_multiple_warmup_passes_precede_all_timed_passes(self):
        records = self.measure([(spec("A"), 1), (spec("B"), 1)], samples=2, warmup=2)
        self.assertEqual([e[1] for e in self.events if e[0] == "compile"], list("ABABABAB"))
        self.assertEqual(records[0]["timers"]["compileInner"]["samples"], [3.001, 4.001])

    def test_warmup_exception_fails_but_returned_crash_code_is_excluded(self):
        self.responses[("crash", 1, 1)] = (-11, 500, "warmup crash", 1000)
        self.responses[("exception", 1, 1)] = OSError("cannot start warmup")
        recovered, failed, healthy = self.measure(
            [(spec(name), 1) for name in ("crash", "exception", "healthy")], samples=2)
        self.assertTrue(recovered["ok"])
        self.assertIsNone(recovered["crash_codes"])
        self.assertEqual(recovered["timers"]["compileInner"]["samples"], [2.001, 3.001])
        self.assertFalse(failed["ok"])
        self.assertEqual(failed["error"], "cannot start warmup")
        self.assertEqual(self.visits[("exception", 1)], 1)
        self.assertEqual(failed["timers"], {})
        self.assertTrue(healthy["ok"])

    def test_short_disturbance_cannot_fill_one_workloads_median_window(self):
        cases = [(spec(name), 1) for name in "ABC"]

        def disturbed_samples():
            # The same first three compiler invocations are slow in both schedules.
            for invocation in range(15):
                value = 100 if invocation < 3 else 10
                yield 0, value, f"[*] compileInner 1 {value}ms\n", 1000

        medians = []
        for interleave in (False, True):
            with patch.object(bench, "run_once", side_effect=disturbed_samples()):
                records = self.measure(cases, warmup=0, interleave=interleave)
            medians.append([r["timers"]["compileInner"]["median"] for r in records])
        self.assertEqual(medians[0], [100, 10, 10])
        self.assertEqual(medians[1], [10, 10, 10])

    def test_persistent_workload_slowdown_survives_interleaving(self):
        def persistent_slowdown(command):
            value = 20 if command[0] == "B" else 10
            return 0, value, f"[*] compileInner 1 {value}ms\n", 1000

        with patch.object(bench, "run_once", side_effect=persistent_slowdown):
            records = self.measure([(spec(name), 1) for name in "ABC"])
        self.assertEqual([r["timers"]["compileInner"]["median"] for r in records], [10, 20, 10])

    def test_consecutive_control_and_single_workload_api(self):
        self.measure([(spec("A"), 1), (spec("B"), 1)], samples=2, warmup=1, interleave=False)
        self.assertEqual([e[1] for e in self.events if e[0] == "compile"], list("AAABBB"))
        record = bench.run_spec("slangc", spec("C"), 1, 2, 0,
                                self.directory.name, self.directory.name)
        self.assertTrue(record["ok"])
        self.assertEqual(record["sampling_strategy"], "consecutive")
        self.assertEqual(record["timers"]["compileInner"]["samples"], [1.001, 2.001])

    def test_early_crash_cannot_be_hidden_by_later_success(self):
        for code in (-11, 0xC0000005):
            with self.subTest(code=code):
                name = str(code)
                self.responses[(name, 1, 1)] = (code, 500, "crash", 1000)
                failed, healthy = self.measure([(spec(name), 1), (spec("healthy"), 1)],
                                               samples=3, warmup=0)
                self.assertFalse(failed["ok"])
                self.assertEqual(failed["crash_codes"], [code])
                self.assertEqual(failed["wall_ms"]["n"], 2)
                self.assertTrue(healthy["ok"])

    def test_expected_diagnostic_must_occur_in_every_timed_pass(self):
        workload = spec("diagnostics", expected_diags=["E30019"])
        for visit in (2, 3):
            self.responses[(workload.name, 1, visit)] = (
                1, 10, "error[E30019]: expected\n[*] compileInner 1 5ms\n", 1000)
        record = self.measure([(workload, 1)], samples=3, warmup=0)[0]
        self.assertFalse(record["ok"])
        self.assertIn("expected diagnostics absent: E30019", record["error"])
        self.assertEqual(record["timers"]["compileInner"]["n"], 3)

    def test_execution_exception_retains_partial_samples_and_other_workloads(self):
        self.responses[("A", 1, 2)] = OSError("cannot start compiler")
        failed, healthy = self.measure([(spec("A"), 1), (spec("B"), 1)], samples=3, warmup=0)
        self.assertFalse(failed["ok"])
        self.assertEqual(failed["error"], "cannot start compiler")
        self.assertEqual(failed["timers"]["compileInner"]["n"], 1)
        self.assertEqual(self.visits[("A", 1)], 2)
        self.assertTrue(healthy["ok"])
        self.assertEqual(healthy["timers"]["compileInner"]["n"], 3)

    def test_setup_timeout_and_missing_corpus_remain_failures(self):
        self.prepare_failures.add("missing")
        self.setup_failures.add("timeout")
        missing, timeout, healthy = self.measure(
            [(spec("missing"), 1), (spec("timeout", mode="link"), 1), (spec("B"), 1)])
        self.assertFalse(missing["ok"])
        self.assertEqual(missing["error"], "missing corpus")
        self.assertNotIn(("missing", 1), self.visits)
        self.assertFalse(timeout["setup_ok"])
        self.assertFalse(timeout["ok"])
        self.assertTrue(healthy["ok"])
        self.assertEqual(self.events.count(("setup", "timeout", 1)), 1)

    def test_prepared_corpus_and_api_dependency_handling(self):
        missing, healthy = self.measure([(spec("api", mode="api"), 1), (spec("B"), 1)],
                                       prepared=True)
        self.assertFalse(missing["ok"])
        self.assertIn("api-driver", missing["error"])
        self.assertTrue(healthy["ok"])
        self.assertFalse(any(e[0] == "prepare" for e in self.events))
        record = self.measure([(spec("ready_api", mode="api"), 1)],
                              prepared=True, api={"driver": "driver", "libslang": "library"})[0]
        self.assertTrue(record["ok"])

    def test_invalid_sample_counts_do_not_prepare_any_workload(self):
        for samples, warmup in ((0, 1), (-1, 1), (1, -1)):
            with self.assertRaises(ValueError):
                self.measure([(spec("A"), 1)], samples=samples, warmup=warmup)
        self.assertEqual(self.events, [])


class SamplingProvenanceTests(unittest.TestCase):
    def test_trend_skips_sampling_transition_and_resumes_with_matching_history(self):
        with tempfile.TemporaryDirectory(prefix="sampling_trend_test_") as directory:
            root = Path(directory)
            (root / "tracking").mkdir()

            def point(day, strategy, value):
                return {"label": f"2026-01-0{day}", "date": f"2026-01-0{day}",
                        "kind": "daily", "runner": "test-runner", "metrics": {
                            "minimal|compileInner": value,
                            f"minimal|{analyze.SCHEMA_MARKER}": 1.0,
                            f"minimal|{analyze.SIZE_MARKER}": 64.0,
                            f"minimal|{analyze.SAMPLING_MARKER}": strategy}}

            current = point(6, 1.0, 120.0)
            for strategy, expected_code in ((0.0, 0), (1.0, trend.EXIT_REGRESSION)):
                with self.subTest(strategy=strategy):
                    points = [point(day, strategy, 100.0) for day in range(1, 6)] + [current]
                    (root / "tracking" / "tracking.json").write_text(
                        json.dumps({"runner": "test-runner", "points": points}), encoding="utf-8")
                    output = io.StringIO()
                    env = {"GITHUB_OUTPUT": str(root / "output"),
                           "GITHUB_STEP_SUMMARY": str(root / "summary"), "GITHUB_ACTIONS": "true"}
                    with patch.dict(os.environ, env), patch.object(sys, "argv", [
                            "trend.py", "--results", directory, "--label", current["label"]
                    ]), contextlib.redirect_stdout(output):
                        code = 0
                        try:
                            trend.main()
                        except SystemExit as exc:
                            code = exc.code or 0
                    self.assertEqual(code, expected_code)
                    self.assertEqual("not judged" in output.getvalue(), strategy == 0.0)

    def test_only_known_matching_strategies_enter_a_baseline(self):
        with tempfile.TemporaryDirectory(prefix="sampling_provenance_test_") as directory:
            path = Path(directory) / "results.json"
            points = []
            for strategy in ("interleaved", "consecutive", None, "future-unknown"):
                record = {"workload": "minimal", "size": 64, "timer_schema": "detailed",
                          "timers": {"compileInner": {"median": 100.0}}}
                if strategy is not None:
                    record["sampling_strategy"] = strategy
                path.write_text(json.dumps([record]), encoding="utf-8")
                points.append({"metrics": analyze.point_metrics(str(path))})
        keys = tuple(f"minimal|{m}" for m in
                     (analyze.SCHEMA_MARKER, analyze.SIZE_MARKER, analyze.SAMPLING_MARKER))
        self.assertEqual(points[0]["metrics"][keys[-1]], 1.0)
        self.assertEqual(points[1]["metrics"][keys[-1]], 0.0)
        for point in points[2:]:
            self.assertNotIn(keys[-1], point["metrics"])
        self.assertEqual(trend.comparable_metric_values(
            points[0], points, "minimal|compileInner", keys), [100.0])
        self.assertEqual(trend.comparable_metric_values(
            points[2], points, "minimal|compileInner", keys), [])


if __name__ == "__main__":
    unittest.main()
