#!/usr/bin/env python3
"""Regression tests for trend.py's judged-point selection.

The scenario under test is the one observed live on the 2026-07-08 nightly
(run 28921281743): daily labels are keyed by the swept commit's date, so two
points can share a date, and judging `pts[-1]` can silently judge a same-date
SIBLING of the point the workflow just registered — a regression landing that
night would pass unjudged. `--label` pins the judged point.

Run directly (stdlib only, no framework setup):

    python3 tools/compile-perf/tests/test_trend.py
"""
import json
import os
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
TREND = os.path.join(HERE, "..", "trend.py")


def make_point(label, compile_inner_ms):
    """One tracking-series point with a single module_link metric."""
    return {
        "label": label,
        "date": label[:10],
        "kind": "daily",
        "runner": "test-runner",
        "metrics": {"module_link|compileInner": compile_inner_ms},
    }


def run_trend(results_dir, *extra):
    return subprocess.run(
        [sys.executable, TREND, "--results", results_dir, *extra],
        capture_output=True,
        text=True,
    )


class TrendLabelTest(unittest.TestCase):
    def setUp(self):
        # Five clean baseline days, then a regressed point (+40%) that shares
        # its date with a clean sibling sorting after it — the live shape.
        self.tmp = tempfile.TemporaryDirectory()
        os.makedirs(os.path.join(self.tmp.name, "tracking"))
        points = [make_point(f"2026-07-0{i}-aaaaaaaaa", 100.0) for i in range(1, 6)]
        points.append(make_point("2026-07-07-regressed", 140.0))
        points.append(make_point("2026-07-07-sibling", 100.0))
        with open(os.path.join(self.tmp.name, "tracking", "tracking.json"), "w") as fh:
            json.dump({"runner": "test-runner", "points": points}, fh)

    def tearDown(self):
        self.tmp.cleanup()

    def test_without_label_sibling_shadows_regression(self):
        """Documents the failure mode --label exists for: judging pts[-1]
        judges the clean sibling and the regression passes unseen."""
        r = run_trend(self.tmp.name)
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("current=2026-07-07-sibling", r.stdout)

    def test_label_pins_judged_point_and_flags_regression(self):
        r = run_trend(self.tmp.name, "--label", "2026-07-07-regressed")
        self.assertNotEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("current=2026-07-07-regressed", r.stdout)
        self.assertIn("module_link", r.stdout)

    def test_label_baseline_is_strictly_earlier_points(self):
        """Judging a mid-series clean point must not compare it against the
        regressed point that comes AFTER it (a backfill must not be judged
        against its own future)."""
        r = run_trend(self.tmp.name, "--label", "2026-07-05-aaaaaaaaa")
        self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        self.assertIn("current=2026-07-05-aaaaaaaaa", r.stdout)

    def test_unknown_label_fails_loudly(self):
        r = run_trend(self.tmp.name, "--label", "no-such-point")
        self.assertNotEqual(r.returncode, 0)
        self.assertIn("no such point", r.stdout + r.stderr)


if __name__ == "__main__":
    unittest.main()
