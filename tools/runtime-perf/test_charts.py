"""Checks that runtime charts preserve cadence, compatibility, and timing semantics."""
import copy
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import perf
import runtime_charts as charts
import runtime_report
import test_perf
import nightly


class ChartTests(unittest.TestCase):
    def test_release_collector_uses_release_archive_and_rejects_patches(self):
        from argparse import Namespace
        with tempfile.TemporaryDirectory() as directory:
            args = Namespace(results=directory, label="v2026.19", kind="release", attempt="1",
                             commit="release", suite_commit="suite", failure="setup failed")
            self.assertEqual(nightly.collect(args), 1)
            self.assertTrue((Path(directory) / "releases/v2026.19/runtime/1/run.json").exists())
            args.label = "v2026.18.1"
            with self.assertRaisesRegex(ValueError, "main release"):
                nightly.collect(args)

    def event(self, label="2026-10-15-abc", kind="daily", timestamp=1):
        bundle = test_perf.RuntimePerfTests().fixture()
        case = bundle["runs"][0]
        case["timers"]["gpu_iteration_ms"] = {"median": 10.0}
        case["compile_runs"] = [dict(workload="runtime_arithmetic_basic", ok=True, got_timers=True,
                                    wall_ms={"median": 80}, timers={"compileInner": {"median": 25}})]
        return dict(id=label, kind=kind, backend="vulkan", bundle=bundle, case=case,
                    href="run.html", record=dict(label=label, attempt="1", created_unix=timestamp),
                    references={}, reference=None)

    def test_release_order_is_numeric_and_excludes_patches(self):
        events = [self.event(tag, "release") for tag in ("v2026.9", "v2026.18.1", "v2026.19", "v2026.10")]
        self.assertEqual([e["record"]["label"] for e in charts.ordered(events, "release")],
                         ["v2026.9", "v2026.10", "v2026.19"])

    def test_failed_latest_attempt_is_not_replaced_by_an_old_success(self):
        first = self.event()
        second = self.event(timestamp=2)
        second["case"]["status"] = "failed"
        self.assertEqual(charts.ordered([first, second], "daily"), [second])

    def test_changed_dimensions_and_driver_get_separate_graphs(self):
        first = self.event()
        for field in ("elements", "driver"):
            second = copy.deepcopy(first)
            second["id"] = "next"
            if field == "elements":
                second["case"]["elements"] *= 2
            else:
                second["bundle"]["environment"]["driver_id"] = "different"
            self.assertEqual(len(charts.groups([first, second])), 2)

    def test_unstable_point_leaves_a_gap(self):
        events = [self.event(str(i), timestamp=i) for i in range(3)]
        events[1]["case"]["status"] = "unstable"
        group = next(iter(charts.groups(events).values()))
        with patch.object(charts, "graph", return_value="") as graph:
            charts.cohort(events, group)
        self.assertEqual(graph.call_args_list[0].args[1][0][2], [10000, None, 10000])

    def test_reference_graph_uses_its_matched_slang_not_full_suite_timing(self):
        event = self.event()
        event["references"] = dict(complete=True, measurement={"elements": 4194304}, environment={})
        event["reference"] = dict(batch=16, summary={
            "slang": {"valid": True, "median_ms": 1.0},
            "reference": {"valid": True, "median_ms": 1.2}})
        group = next(iter(charts.groups([event], reference=True).values()))
        with patch.object(charts, "graph", return_value="") as graph:
            charts.cohort([event], group, reference=True)
        series = graph.call_args.args[1]
        self.assertEqual([s[2] for s in series], [[1000], [1200]])
        self.assertNotEqual(series[0][2], [10000])

    def test_compile_stages_are_not_added_together(self):
        event = self.event()
        event["case"]["workload"] = "image_pipeline"
        event["case"]["compile_runs"] = [dict(workload="runtime_image_pipeline_" + stage,
            ok=True, got_timers=True, wall_ms={"median": value}, timers={"compileInner": {"median": value / 2}})
            for stage, value in (("horizontal", 10), ("vertical", 20), ("tone", 30))]
        group = next(iter(charts.groups([event]).values()))
        with patch.object(charts, "graph", return_value="") as graph:
            charts.cohort([event], group)
        self.assertEqual(sorted(s[2][0] for s in graph.call_args_list[1].args[1]), [10, 20, 30])

    def test_each_test_gets_a_page_with_both_cadences_and_compile_graphs(self):
        events = [self.event(), self.event("v2026.19", "release")]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            index = charts.write_subtests(root, events, runtime_report.page)
            page = (root / "test-arithmetic_basic.html").read_text(encoding="utf-8")
            self.assertIn('test-arithmetic_basic.html#vulkan', index)
            for title in ("Daily runs", "Last five main releases", "Full-suite runtime", "Paired CLI compile wall time"):
                self.assertIn(title, page)
            self.assertEqual(len(list(root.glob("test-*.html"))), len(perf.workloads.CASES))


if __name__ == "__main__":
    unittest.main()
