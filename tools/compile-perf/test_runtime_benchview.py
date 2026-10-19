"""Check runtime conversion and the shared host export without a GPU or service."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import benchview
import runtime_benchview as runtime


class RuntimeBenchViewTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.manifest = dict(schema_version=1, kind="daily", label="2026-10-19-test", attempt="1",
                             commit="a" * 40, suite_commit="b" * 40, created_unix=1792368000,
                             status="ok", backends=dict(vulkan=dict(expected_cases=["arithmetic_basic"])))
        self.compile = dict(workload="runtime_arithmetic_basic", mode="cli", size=4, ok=True,
                            wall_ms=dict(samples=[10, 12]), timers=dict(compileInner=dict(samples=[5, 6])))
        self.case = dict(workload="arithmetic_basic", backend="vulkan", status="ok", validated=True,
                         elements=65536, batch=8, size=4, adapter="RTX 4090", source_sha256="source",
                         timers=dict(gpu_iteration_ms=dict(samples=[0.1, 0.2])), compile_runs=[self.compile])
        self.bundle = dict(schema_version=2, environment=dict(host="test", os="Windows-11", driver_id="driver"),
                           config=runtime.runtime_charts.perf.DEFAULTS, created_unix=1792368001, runs=[self.case])

    def write(self):
        (self.root / "vulkan").mkdir(exist_ok=True)
        (self.root / "run.json").write_text(json.dumps(self.manifest), encoding="utf-8")
        (self.root / "vulkan/results.json").write_text(json.dumps(self.bundle), encoding="utf-8")

    def convert(self):
        self.write()
        return runtime.submissions(self.root / "run.json", "Windows")

    def test_samples_compile_and_replay(self):
        payload = self.convert()[0]
        gpu, compile = payload["observations"]
        self.assertEqual(gpu["metrics"][0]["distribution"]["samples"], [0.1, 0.2])
        self.assertEqual(gpu["metrics"][0]["id"], "runtime.gpu_iteration_ms")
        self.assertNotIn("breakdown", gpu["metrics"][0])
        self.assertEqual(compile["test"], gpu["test"])
        self.assertEqual([m["name"] for m in compile["metrics"]], ["wall_ms", "compileInner"])
        self.assertEqual(payload, self.convert()[0])
        self.manifest["attempt"] = "2"
        second = self.convert()[0]
        self.assertEqual(second["run"], payload["run"])
        self.assertNotEqual(second["idempotencyKey"], payload["idempotencyKey"])

    def test_failed_runtime_keeps_valid_compile(self):
        self.case.update(status="unstable", timers={})
        gpu, compile = self.convert()[0]["observations"]
        self.assertEqual(gpu["status"], "failed")
        self.assertEqual(gpu["metrics"], [])
        self.assertEqual(compile["status"], "passed")
        self.assertTrue(compile["metrics"])

    def test_missing_case_and_setup_failure(self):
        self.bundle["runs"] = []
        item = self.convert()[0]["observations"][0]
        self.assertEqual(item["status"], "failed")
        self.manifest.update(backends={}, status="failed", reason="No GPU")
        item = self.convert()[0]["observations"][0]
        self.assertEqual(item["error"]["message"], "No GPU")

    def test_contract_changes_split_history(self):
        first = self.convert()[0]["observations"][0]["case"]
        self.bundle["environment"]["driver_id"] = "new-driver"
        second = self.convert()[0]["observations"][0]["case"]
        self.assertNotEqual(first, second)
        self.manifest["commit"] = "c" * 40
        self.assertEqual(second, self.convert()[0]["observations"][0]["case"])

    def test_release_metadata_and_unknown_revision(self):
        self.manifest.update(kind="release", label="v2026.19")
        self.assertEqual(self.convert()[0]["run"]["metadata"]["release"]["label"], "v2026.19")
        self.manifest["label"] = "v2026.18.1"
        with self.assertRaisesRegex(ValueError, "main release"):
            self.convert()
        self.manifest.update(label="v2026.19", commit="release-v2026.19")
        with self.assertRaisesRegex(ValueError, "full tested compiler"):
            self.convert()

    def test_reference_samples_are_matched_and_not_suite_samples(self):
        (self.root / "references").mkdir()
        runs = {key: [dict(status="ok", validated=True, elements=1000, batch=2, backend="vulkan", samples=[dict(gpu_iteration_ms=value)])]
                for key, value in (("slang", 1), ("reference", 2))}
        data = dict(complete=True, created_unix=1792368002, environment=self.bundle["environment"],
                    measurement=dict(elements=1000), cases=[dict(case="arithmetic_basic", backend="vulkan",
                    batch=2, runs=runs, summary={key:dict(valid=True) for key in runs})])
        (self.root / "references/results.json").write_text(json.dumps(data), encoding="utf-8")
        observations = self.convert()[0]["observations"]
        self.assertEqual([o["metrics"][0]["distribution"]["samples"] for o in observations[2:]], [[1], [2]])
        self.assertEqual(observations[2]["case"]["dimensions"]["measurement"], "matched-reference")
        self.assertNotEqual(observations[0]["case"], observations[2]["case"])
        self.assertEqual(observations[2]["case"]["dimensions"]["elements"], 1000)
        data["cases"][0]["runs"]["reference"][0]["elements"] = 2000
        (self.root / "references/results.json").write_text(json.dumps(data), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "mismatched dimensions"):
            self.convert()

    def test_invalid_samples_and_duplicate_cases_rejected(self):
        self.case["timers"]["gpu_iteration_ms"] = dict(median=1)
        with self.assertRaisesRegex(ValueError, "raw samples"):
            self.convert()
        self.bundle["runs"].append(copy.deepcopy(self.case))
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            self.convert()

    def test_shared_cli_exports_both_suites_and_invalidates_stale_ready(self):
        self.write()
        (self.root / "compile.json").write_text(json.dumps([self.compile]), encoding="utf-8")
        (self.root / "meta.json").write_text(json.dumps(dict(commit="a"*40, runner="test")), encoding="utf-8")
        output = self.root / "export"
        command = [sys.executable, str(Path(benchview.__file__)), "--results", str(self.root / "compile.json"),
                   "--meta", str(self.root / "meta.json"), "--os", "Windows", "--observed-at", "2026-10-19T00:00:00Z",
                   "--execution-id", "test", "--runtime-manifest", str(self.root / "run.json"), "--output", str(output)]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        data = json.loads((output / "submissions.json").read_text())
        self.assertEqual([p["run"]["suite"]["id"] for p in data["submissions"]], ["compile-perf", "runtime-perf"])
        self.assertTrue((output / "submissions.ready").exists())
        self.manifest["commit"] = "b" * 40
        self.write()
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("revisions differ", result.stderr)
        self.assertFalse((output / "submissions.ready").exists())


if __name__ == "__main__":
    unittest.main()
