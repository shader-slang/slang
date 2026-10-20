"""Exercise conversion, API limits and atomic host-collector exports."""

import copy
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import benchview


class BenchViewTests(unittest.TestCase):
    """Keep the reporting adapter independent of a live server or write key."""

    def setUp(self):
        """Use overlapping timers to exercise the shared allocation model."""
        self.record = {"workload": "mdl_dxr", "mode": "cli", "size": 1, "ok": True,
                       "timers": {"compileInner": {"samples": [10, 20]},
                                  "linkAndOptimizeIR": {"samples": [12, 25]}}}
        self.meta = {"commit": "a" * 40, "runner": "perf-runner", "kind": "daily"}

    def convert(self, records=None, **kwargs):
        """Export a fixed execution so equality checks cover replay identity."""
        return benchview.submissions(records or [self.record], self.meta, "Windows",
                                     "2026-10-14T05:00:00Z", "job-1", **kwargs)

    def test_release_and_daily(self):
        """Release metadata belongs to the Slang version, and daily runs omit it."""
        daily = self.convert()[0]
        release = self.convert(release={"label": "v2026.19"})[0]
        self.assertEqual(daily["project"], {"id": "slang", "name": "Slang",
                         "metadata": {"defaultBranch": "master", "breakdownTooltipMaxItems": 1}})
        self.assertNotIn("metadata", daily["run"])
        self.assertEqual(release["run"]["metadata"]["release"]["label"], "v2026.19")
        self.assertNotEqual(daily["run"]["key"], release["run"]["key"])
        self.assertEqual(self.convert()[0], daily)

    def test_primary_metric_and_total_labels(self):
        """Select each workload's total without discarding diagnostic measurements."""
        for mode, timer, label in (("target", "compileInner", "Total compile time"),
                                   ("api", "apiTotal", "Total API time")):
            with self.subTest(mode=mode):
                record = copy.deepcopy(self.record)
                record["mode"] = mode
                record["timers"][timer] = {"samples": [10, 20]}
                observation = self.convert([record])[0]["observations"][0]
                primary = "compile.timer." + timer.lower()
                self.assertEqual(observation["test"]["metadata"]["primaryMetric"], primary)
                metric = next(m for m in observation["metrics"] if m["id"] == primary)
                self.assertEqual(metric["name"], label)
                self.assertEqual(metric["distribution"]["samples"], [10, 20])
                self.assertIn("compile.timer.linkandoptimizeir",
                              [m["id"] for m in observation["metrics"]])
                self.assertEqual(json.loads(observation["artifacts"][0]["content"]["text"]), record)
                record["ok"] = False
                failed = self.convert([record])[0]["observations"][0]
                self.assertEqual(failed["test"], observation["test"])
                self.assertEqual(failed["metrics"], [])

    def test_branch_preserves_run_identity(self):
        """Branch display metadata must not create a different logical run."""
        detached = self.convert()[0]
        self.assertNotIn("branch", detached["run"]["vcs"])
        for branch in ("master", "test/perf"):
            tagged = self.convert(branch=branch)[0]
            self.assertEqual(tagged["run"]["vcs"]["branch"], branch)
            self.assertEqual(tagged["run"]["key"], detached["run"]["key"])

    def test_missing_primary_does_not_abort_export(self):
        """Partial passed records keep measurements without declaring a missing total."""
        for mode, primary in (("target", "compileInner"), ("api", "apiTotal")):
            with self.subTest(mode=mode):
                partial = dict(self.record, workload="partial", mode=mode,
                               timers={"linkAndOptimizeIR": {"samples": [12, 25]}})
                with patch("sys.stderr", new=io.StringIO()) as warnings:
                    observations = self.convert([self.record, partial])[0]["observations"]
                self.assertIn(primary, warnings.getvalue())
                self.assertIn("metadata", observations[0]["test"])
                self.assertNotIn("metadata", observations[1]["test"])
                self.assertEqual(observations[1]["status"], "passed")
                self.assertEqual(observations[1]["metrics"][0]["distribution"]["samples"],
                                 [12, 25])
                self.assertEqual(json.loads(observations[1]["artifacts"][0]["content"]["text"]),
                                 partial)

    def test_primary_omitted_by_metric_limit(self):
        """A total excluded by the metric cap cannot be advertised as exported."""
        self.record["memory"] = {f"memory{i}": {"samples": [1, 2]} for i in range(64)}
        with patch("sys.stderr", new=io.StringIO()) as warnings:
            observation = self.convert()[0]["observations"][0]
        self.assertEqual(len(observation["metrics"]), 64)
        self.assertNotIn("metadata", observation["test"])
        self.assertIn("compileInner", warnings.getvalue())

    def test_daily_and_new_releases_export_together(self):
        """New releases carry their own revisions, tags and only current default cases."""
        size = benchview.BY_NAME["mdl_dxr"].default_size
        default = dict(self.record, size=size)
        records = [default, dict(default, size=size + 1),
                   dict(default, size=False), dict(default, size=float(size)),
                   dict(default, workload="retired_workload")]
        completed = [
            {"tag": tag, "commit": revision * 40, "commit_time": "2026-10-17T00:00:00Z",
             "released_at": "2026-10-17T04:00:00Z", "observed_at": "2026-10-20T06:00:00Z"}
            for tag, revision in (("v2026.20", "b"), ("v2026.20.1", "c"))]
        completed[1]["branch"] = "release/2026.20"
        self.meta.update(corpus_sha="corpus", label="daily-label", slangc="daily/compiler")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "daily.json").write_text(json.dumps([default]))
            (root / "meta.json").write_text(json.dumps(self.meta))
            for info in completed:
                folder = root / info["tag"]
                folder.mkdir()
                (folder / "results.json").write_text(json.dumps(records))
            for releases in (completed, []):
                (root / "completed.json").write_text(json.dumps(releases))
                argv = ["benchview.py", "--results", str(root / "daily.json"),
                        "--meta", str(root / "meta.json"), "--os", "Windows",
                        "--observed-at", "2026-10-20T05:00:00Z", "--execution-id", "job-1",
                        "--branch", "master", "--completed-releases", str(root / "completed.json"),
                        "--release-results", str(root), "--output", str(root / "export")]
                with patch.object(sys, "argv", argv), patch("sys.stdout", new=io.StringIO()):
                    benchview.main()
                payloads = json.loads((root / "export/submissions.json").read_text())["submissions"]
                self.assertEqual(len(payloads), 1 + len(releases))
                self.assertNotIn("metadata", payloads[0]["run"])
                for payload, info in zip(payloads[1:], releases):
                    self.assertEqual(payload["execution"]["metadata"], {
                        "runner": self.meta["runner"], "corpus_sha": "corpus",
                        "commit": info["commit"], "commit_time": info["commit_time"]})
                    self.assertEqual(payload["run"]["vcs"]["revision"], info["commit"])
                    self.assertEqual(payload["run"]["vcs"].get("branch"), info.get("branch"))
                    self.assertEqual(payload["run"]["metadata"]["release"],
                                     {"label": info["tag"], "releasedAt": info["released_at"]})
                    self.assertEqual(len(payload["observations"]), 1)
                    observation = payload["observations"][0]
                    self.assertEqual(observation["case"]["dimensions"]["size"], size)
                    self.assertEqual(observation["observedAt"], info["observed_at"])
                    self.assertEqual(json.loads(observation["artifacts"][0]["content"]["text"]),
                                     default)
                self.assertTrue((root / "export/submissions.ready").exists())
        self.assertEqual(self.meta["commit"], "a" * 40)

    def test_release_paths_reject_non_tags(self):
        """Completed metadata cannot address paths outside the release result directory."""
        with patch("sys.stderr", new=io.StringIO()) as warnings:
            self.assertEqual(benchview.release_submissions(
                [{"tag": "../daily"}], Path("."), self.meta, "Windows", "job-1"), [])
        self.assertIn("Invalid completed release tag", warnings.getvalue())

    def test_release_failures_preserve_daily_export(self):
        """Missing, malformed and oversized releases must not block the daily handoff."""
        default = dict(self.record, size=benchview.BY_NAME["mdl_dxr"].default_size)
        info = {"tag": "v2026.20", "commit": "b" * 40,
                "commit_time": "2026-10-17T00:00:00Z",
                "released_at": "2026-10-17T04:00:00Z",
                "observed_at": "2026-10-20T06:00:00Z"}
        oversized = dict(default, extra="x" * 262144)
        for failure in ("missing", "empty", "date", "artifact", "manifest", "count", "bytes"):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                (root / "daily.json").write_text(json.dumps([default]))
                (root / "meta.json").write_text(json.dumps(self.meta))
                folder = root / info["tag"]
                folder.mkdir()
                records = ([dict(default, size=999)] if failure == "empty" else
                           [oversized] if failure == "artifact" else [default])
                if failure != "missing":
                    (folder / "results.json").write_text(json.dumps(records))
                release = dict(info, released_at="invalid") if failure == "date" else info
                (root / "completed.json").write_text(
                    "{" if failure == "manifest" else json.dumps([release]))
                argv = ["benchview.py", "--results", str(root / "daily.json"),
                        "--meta", str(root / "meta.json"), "--os", "Windows",
                        "--observed-at", "2026-10-20T05:00:00Z", "--execution-id", "job-1",
                        "--completed-releases", str(root / "completed.json"),
                        "--release-results", str(root), "--output", str(root / "export")]
                convert = benchview.submissions

                def oversized_release(*args, **kwargs):
                    """Exercise real collector validation without thousands of fixture records."""
                    payloads = convert(*args, **kwargs)
                    if payloads[0]["run"]["dimensions"]["kind"] == "release":
                        if failure == "count":
                            return payloads * 64
                        if failure == "bytes":
                            payloads[0]["execution"]["metadata"]["extra"] = "x" * (8 * 1024 * 1024)
                    return payloads

                with patch.object(sys, "argv", argv), patch("sys.stdout", new=io.StringIO()), \
                     patch("sys.stderr", new=io.StringIO()) as warnings, \
                     patch.object(benchview, "submissions", side_effect=oversized_release):
                    benchview.main()
                self.assertIn("Warning: skipped BenchView release", warnings.getvalue())
                payloads = json.loads((root / "export/submissions.json").read_text())["submissions"]
                self.assertEqual(len(payloads), 1)
                self.assertEqual(payloads[0]["run"]["dimensions"]["kind"], "daily")
                self.assertTrue((root / "export/submissions.ready").exists())

    def test_samples_and_breakdown(self):
        """Allocated phases conserve each root sample despite overlapping timers."""
        metrics = self.convert()[0]["observations"][0]["metrics"]
        root = next(m for m in metrics if m["id"] == "compile.timer.compileinner")
        self.assertEqual(root["distribution"], {"samples": [10, 20]})
        components = root["breakdown"]["components"]
        for i, total in enumerate([10, 20]):
            self.assertAlmostEqual(sum(c["distribution"]["samples"][i] for c in components), total)

    def test_unaligned_samples_omit_only_breakdown(self):
        """Keep passed measurements without inventing phase sample alignment."""
        self.record["timers"]["linkAndOptimizeIR"]["samples"] = [12]
        with patch('sys.stderr', new_callable=io.StringIO) as stderr:
            observation = self.convert()[0]["observations"][0]
        self.assertEqual(observation["status"], "passed")
        root = next(m for m in observation["metrics"] if m["id"] == "compile.timer.compileinner")
        self.assertNotIn("breakdown", root)
        self.assertEqual(root["distribution"]["samples"], [10, 20])
        self.assertIn("breakdownOmitted", root["metadata"])
        self.assertIn("omitted unaligned breakdown", stderr.getvalue())

    def test_source_emission_tree_and_priority(self):
        """Source emission contains linking, including when the metric cap is reached."""
        values = {"compileInner": 100, "generateOutput": 90,
                  "emitEntryPointsSourceFromIR": 80, "linkAndOptimizeIR": 60}
        self.record["timers"] = {name: {"samples": [value, value * 2]}
                                 for name, value in values.items()}
        self.record["timers"].update({f"aaa{i}": {"samples": [1, 2]} for i in range(80)})
        metrics = self.convert()[0]["observations"][0]["metrics"]
        self.assertIn("emitEntryPointsSourceFromIR", [m["name"] for m in metrics])
        root = next(m for m in metrics if m["id"] == "compile.timer.compileinner")
        actual = {c["name"]: c["distribution"]["samples"]
                  for c in root["breakdown"]["components"]}
        for i in range(2):
            expected = benchview.buckets({name: value * (i + 1) for name, value in values.items()})
            self.assertEqual({name: samples[i] for name, samples in actual.items()}, expected)
        self.assertEqual(actual["emitEntryPointsSourceFromIR (self)"], [20, 40])

    def test_component_id_collision(self):
        """Reject normalized component collisions before contacting the server."""
        tree = ("root", [("Foo", []), ("foo", [])])
        timers = {"root": {"samples": [10]}, "Foo": {"samples": [2]},
                  "foo": {"samples": [3]}}
        with self.assertRaisesRegex(ValueError, "component names collide"):
            benchview.phase_breakdown(timers, tree)

    def test_runner_os_mismatch(self):
        """A changed runner platform cannot silently inherit the Windows label."""
        self.meta["runner"] = "Linux-x86_64-cpu"
        with self.assertRaisesRegex(ValueError, "OS disagrees"):
            self.convert()

    def test_malformed_inputs_have_single_line_errors(self):
        """CLI input errors should not leak tracebacks into the nightly log."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'meta.json').write_text(json.dumps(self.meta))
            malformed = copy.deepcopy(self.record)
            malformed['timers']['compileInner'] = 5
            for records in ([None], [malformed]):
                (root / 'out').mkdir(exist_ok=True)
                (root / 'out' / 'submissions.ready').write_text('ready\n')
                (root / 'results.json').write_text(json.dumps(records))
                result = subprocess.run(
                    [sys.executable, str(Path(benchview.__file__)),
                     '--results', str(root / 'results.json'), '--meta', str(root / 'meta.json'),
                     '--os', 'Windows', '--observed-at', '2026-10-15T00:00:00Z',
                     '--execution-id', 'test', '--output', str(root / 'out')],
                    capture_output=True, text=True)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(len(result.stderr.splitlines()), 1)
                self.assertIn('BenchView export failed:', result.stderr)
                self.assertFalse((root / 'out' / 'submissions.ready').exists())
        with self.assertRaises(ValueError):
            benchview.timestamp(123)

    def test_zero_root(self):
        """A zero-duration root still exports a valid complete breakdown."""
        self.record["timers"] = {"compileInner": {"samples": [0, 0]}}
        metric = self.convert()[0]["observations"][0]["metrics"][0]
        components = metric["breakdown"]["components"]
        self.assertEqual(len(components), 1)
        self.assertEqual(components[0]["distribution"]["samples"], [0, 0])

    def test_summary_only_rejected(self):
        """Old summaries cannot recover missing raw samples or maxima."""
        self.record["timers"]["compileInner"] = {"median": 10, "n": 2}
        with self.assertRaisesRegex(ValueError, "samples"):
            self.convert()

    def test_metric_limit_retains_source(self):
        """Overflow timers remain in the original record and are listed explicitly."""
        self.record["timers"].update({f"extra{i}": {"samples": [1, 2]} for i in range(80)})
        observation = self.convert()[0]["observations"][0]
        self.assertEqual(len(observation["metrics"]), 64)
        self.assertEqual(len(observation["metadata"]["ungraphedTimers"]), 18)
        self.assertIn('extra79', observation["artifacts"][0]["content"]["text"])

    def test_batches_keep_release(self):
        """The 501st observation starts a new batch with identical run metadata."""
        records = [dict(self.record, size=i) for i in range(501)]
        payloads = self.convert(records, release={"label": "v2026.19"})
        self.assertEqual([len(p["observations"]) for p in payloads], [500, 1])
        self.assertEqual(payloads[0]["run"], payloads[1]["run"])
        self.assertNotEqual(payloads[0]["idempotencyKey"], payloads[1]["idempotencyKey"])

    def test_failed_workload(self):
        """Failed workloads can be reported without timing samples."""
        self.record.update(ok=False, timers={})
        observation = self.convert()[0]["observations"][0]
        self.assertEqual(observation["status"], "failed")
        self.assertIn("error", observation)

    def test_failed_partial_samples_do_not_abort_nightly(self):
        """One failed workload exports its source alongside unaffected successful results."""
        failed = copy.deepcopy(self.record)
        failed.update(workload="failed", ok=False, error="sample failed")
        failed['timers']['linkAndOptimizeIR']['samples'] = [12]
        observations = self.convert([self.record, failed])[0]['observations']
        self.assertEqual([o['status'] for o in observations], ['passed', 'failed'])
        self.assertTrue(observations[0]['metrics'])
        self.assertEqual(observations[1]['metrics'], [])
        self.assertEqual(json.loads(observations[1]['artifacts'][0]['content']['text']), failed)

    def test_invalid_dates_and_duplicates(self):
        """Reject impossible release dates and ambiguous observation identities."""
        with self.assertRaises(ValueError):
            self.convert(release={"label": "v1", "releasedAt": "2026-02-30T00:00:00Z"})
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            self.convert([self.record, copy.deepcopy(self.record)])

    def test_export_preserves_batches_and_replay(self):
        """Collector delivery reuses the exact submissions and release metadata."""
        payloads = self.convert(release={"label": "v2026.19"})
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            benchview.export(payloads, output)
            first = (output / 'submissions.json').read_bytes()
            self.assertEqual(json.loads(first), {
                'schemaVersion': 1, 'kind': 'slang-compile-perf-benchview-export',
                'submissionSchema': 'benchmark-submission-v1', 'submissions': payloads})
            self.assertEqual((output / 'submissions.ready').read_bytes(), b'ready\n')
            benchview.export(payloads, output)
            self.assertEqual((output / 'submissions.json').read_bytes(), first)

    def test_export_failure_never_leaves_ready_marker(self):
        """Failed payload or marker replacement invalidates old readiness and cleans temp files."""
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            for failed_name in ('submissions.json', 'submissions.ready'):
                benchview.export(self.convert(), output)
                replace = benchview.os.replace

                def fail_replace(source, destination):
                    """Inject a disk failure at either publication boundary."""
                    if destination.name == failed_name:
                        raise OSError('disk failure')
                    replace(source, destination)

                with patch('benchview.os.replace', side_effect=fail_replace):
                    with self.assertRaises(OSError):
                        benchview.export(self.convert(), output)
                self.assertFalse((output / 'submissions.ready').exists())
                self.assertEqual({p.name for p in output.iterdir()}, {'submissions.json'})

    def test_export_limits_clear_stale_readiness(self):
        """Reject empty, excessive-batch and oversized bundles before publishing readiness."""
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp)
            for payloads in ([], [{}] * 65, [{'oversized': 'x' * (8 * 1024 * 1024)}]):
                (output / 'submissions.ready').write_text('ready\n')
                with self.assertRaises(ValueError):
                    benchview.export(payloads, output)
                self.assertFalse((output / 'submissions.ready').exists())


if __name__ == "__main__":
    unittest.main()
