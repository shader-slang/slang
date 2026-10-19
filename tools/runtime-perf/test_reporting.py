"""Exercise history selection, failure retention and report compatibility without a GPU."""
import copy
import json
import os
from pathlib import Path
import tempfile
import subprocess
import sys
import unittest
from unittest.mock import patch

import nightly
import perf
import runtime_report
import test_perf


class ReportingTests(unittest.TestCase):
    def test_archive_does_not_change_when_checkout_timestamps_change(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "bundle"
            source.mkdir()
            (source / "results.json").write_text("{}")
            runtime_report.archive(source, root / "first.zip")
            os.utime(source / "results.json", (1000000000, 1000000000))
            runtime_report.archive(source, root / "second.zip")
            self.assertEqual((root / "first.zip").read_bytes(), (root / "second.zip").read_bytes())

    def test_history_rejects_different_gpu_and_skips_invalid_daily(self):
        good = test_perf.RuntimePerfTests().fixture()
        noisy = copy.deepcopy(good)
        noisy["runs"][0]["status"] = "unstable"
        changed = copy.deepcopy(good)
        changed["runs"][0]["adapter"] = "other GPU"
        history = [("valid", good), ("noisy", noisy), ("different", changed)]
        delta, label = runtime_report.previous_case(history, good, good["runs"][0], "today")
        self.assertEqual((delta, label), ("+0.00%", "valid"))

    def test_same_daily_attempt_is_not_its_own_baseline(self):
        data = test_perf.RuntimePerfTests().fixture()
        delta, reason = runtime_report.previous_case([("today", data)], data, data["runs"][0], "today")
        self.assertEqual(delta, "—")
        self.assertIn("No previous", reason)

    def test_cuda_excludes_only_raytracing(self):
        self.assertEqual(set(nightly.coverage("cuda")),
                         {n for n, f in perf.workloads.CASES.items() if f != "raytracing"})
        self.assertEqual(set(nightly.coverage("d3d12")), set(perf.workloads.CASES))

    def test_empty_history_still_has_report(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.assertEqual(runtime_report.write_report(root, root / "analysis"), 0)
            self.assertIn("No runtime nightlies", (root / "analysis/runtime/index.html").read_text())

    def test_malformed_manifest_is_visible_without_breaking_publication(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            perf.write_json(root / "daily/bad/runtime/1/run.json", {"schema_version": 1})
            runtime_report.write_report(root, root / "analysis")
            self.assertIn("Unreadable history", (root / "analysis/runtime/index.html").read_text())

    def test_setup_failure_is_archived_and_visible(self):
        from argparse import Namespace
        with tempfile.TemporaryDirectory() as directory:
            args = Namespace(results=directory, label="today", attempt="1", commit="abc",
                             suite_commit="def", failure="Missing GPU <driver>")
            self.assertEqual(nightly.collect(args), 1)
            path = Path(directory) / "daily/today/runtime/1/run.json"
            self.assertEqual(json.loads(path.read_text())["status"], "failed")
            runtime_report.write_report(directory, Path(directory) / "analysis")
            report = (Path(directory) / "analysis/runtime/index.html").read_text()
            self.assertIn("Missing GPU &lt;driver&gt;", report)
            with self.assertRaises(FileExistsError):
                nightly.collect(args)

    def test_missing_case_is_visible_and_has_no_delta(self):
        data = test_perf.RuntimePerfTests().fixture()
        page = runtime_report.case_table(data, ["missing"], [], "today")
        self.assertIn("missing", page)
        self.assertNotIn("+0.00%", page)

    def test_combined_report_survives_first_run_compile_failure(self):
        from argparse import Namespace
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "index.json").write_text("[]")
            nightly.collect(Namespace(results=directory, label="failed", attempt="1",
                                     commit="abc", suite_commit="def", failure="GPU setup failed"))
            subprocess.run([sys.executable, str(perf.COMPILE_PERF / "report.py"),
                            "--results", directory, "--index", str(root / "index.json")], check=True)
            self.assertIn("No valid compile measurements", (root / "analysis/index.html").read_text())

    def test_missing_native_results_fail_but_do_not_skip_other_backends(self):
        from argparse import Namespace
        with tempfile.TemporaryDirectory() as directory:
            args = Namespace(results=directory, label="today", attempt="1", commit="abc",
                             suite_commit="def", failure=None, driver_id="driver", references=False,
                             backends="vulkan,d3d12", samples=20, runner="runner", slangc="slangc",
                             environment_id="test", timeout=30)
            with patch.object(nightly.subprocess, "check_output", return_value="rhi"), \
                    patch.object(nightly.subprocess, "run", return_value=Namespace(returncode=0)) as run:
                self.assertEqual(nightly.collect(args), 1)
                self.assertEqual(run.call_count, 2)
            manifest = json.loads((Path(directory) / "daily/today/runtime/1/run.json").read_text())
            self.assertEqual({v["status"] for v in manifest["backends"].values()}, {"failed"})

    def test_newest_valid_calibration_uses_measurement_time_not_checkout_time(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config = dict(perf.DEFAULTS)
            for label, timestamp in (("z-old", 1), ("a-new", 2)):
                path = root / "daily" / label / "runtime/1/vulkan"
                perf.write_json(path / "results.json", dict(created_unix=timestamp,
                                runs=[dict(workload="case", status="ok", validated=True)]))
                perf.write_json(path / "calibration.json", {})
            with patch.object(perf, "check_calibration"):
                result = nightly.baseline(root, config, {"case": {}})
            self.assertIn("a-new", str(result))


if __name__ == "__main__":
    unittest.main()
