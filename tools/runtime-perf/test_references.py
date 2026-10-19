"""Comparison guards for independently compiled sensitive workloads."""
import copy
import unittest

import references


class ReferenceTests(unittest.TestCase):
    def fixture(self):
        summary = {impl: dict(valid=True, median_ms=1.0) for impl in ("slang", "reference")}
        row = dict(backend="vulkan", case="sensitive_loops_8", slang_source_sha256="slang",
                   reference_source_sha256="glsl", batch=64, adapter="GPU", rhi_revision="rhi",
                   reference_compilation=dict(tool_sha256="tool", source_sha256="source", binary_sha256="binary"),
                   summary=summary)
        return dict(schema_version=1, workload_version=1, complete=True, environment={"driver_id": "driver"},
                    measurement={"elements": 262144}, runner_sources={"runner.cpp": "runner"}, cases=[row])

    def test_compiler_performance_change_can_be_compared(self):
        old = self.fixture()
        new = copy.deepcopy(old)
        new["cases"][0]["summary"]["slang"]["median_ms"] = 1.25
        self.assertEqual(references.comparison(old, new)[0]["ratio"], 1.25)

    def test_history_rejects_environment_or_work_changes(self):
        old = self.fixture()
        for key in ("environment", "measurement", "runner_sources"):
            new = copy.deepcopy(old)
            new[key] = {"changed": True}
            with self.subTest(key=key), self.assertRaises(ValueError):
                references.comparison(old, new)
        for key in ("batch", "adapter", "slang_source_sha256", "rhi_revision"):
            new = copy.deepcopy(old)
            new["cases"][0][key] = "changed"
            with self.subTest(key=key), self.assertRaises(ValueError):
                references.comparison(old, new)

    def test_reference_compiler_changes_require_new_baseline(self):
        old = self.fixture()
        for key in ("tool_sha256", "source_sha256", "binary_sha256"):
            new = copy.deepcopy(old)
            new["cases"][0]["reference_compilation"][key] = "changed"
            with self.subTest(key=key), self.assertRaises(ValueError):
                references.comparison(old, new)

    def test_unstable_results_cannot_be_compared(self):
        old = self.fixture()
        new = copy.deepcopy(old)
        new["cases"][0]["summary"]["reference"]["valid"] = False
        with self.assertRaisesRegex(ValueError, "unstable"):
            references.comparison(old, new)

    def test_drift_between_stable_rounds_is_not_hidden(self):
        runs = [dict(status="ok", samples=[{"gpu_iteration_ms": n}] * 3,
                     timers={"gpu_iteration_ms": {"median": n}}) for n in (1.0, 1.2)]
        self.assertFalse(references.summarize(runs)["valid"])

    def test_partial_reports_cannot_be_compared(self):
        old = self.fixture()
        old["complete"] = False
        with self.assertRaisesRegex(ValueError, "incomplete"):
            references.comparison(old, self.fixture())


if __name__ == "__main__":
    unittest.main()
