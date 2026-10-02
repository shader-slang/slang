"""Exercise shadow result handling with a subprocess standing in for slang-test."""

import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


HELPER = Path(__file__).resolve().parents[1] / "ci" / "l0-shadow.py"
spec = importlib.util.spec_from_file_location("shadow", HELPER)
shadow = importlib.util.module_from_spec(spec)
spec.loader.exec_module(shadow)


class ShadowTests(unittest.TestCase):
    def observe(self, body, timeout=10):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            fake = root / "fake.py"
            fake.write_text("import sys, time\n" + body)
            result = subprocess.run(
                [sys.executable, str(HELPER), "run", "--output-dir", str(root / "output"),
                 "--test-timeout", str(timeout), "--", sys.executable, str(fake)],
                capture_output=True, text=True, timeout=20,
            )
            return result.returncode, json.loads((root / "output" / "result.json").read_text())

    def test_candidate_count_is_distinct_from_execution(self):
        code, result = self.observe(
            "if '-dry-run' in sys.argv:\n"
            " print('tests/cpu.slang\\ntests/gpu.slang\\nslang-unit-test-tool/example')\n"
            "else:\n print('50% of tests passed (1/2), 1 tests ignored, 1 tests failed expectedly')\n"
        )
        self.assertEqual(code, 0)
        self.assertEqual(result["candidate_count_before_runtime_filtering"], 3)
        self.assertEqual(result["counts"], dict(passed=1, executed=2, expected_failed=1,
                                               unexpected_failed=0, ignored=1, dispatch_failures=0))

    def test_failed_exit_is_preserved(self):
        code, result = self.observe(
            "print('tests/example' if '-dry-run' in sys.argv else '0% of tests passed (0/1)')\n"
            "sys.exit(0 if '-dry-run' in sys.argv else 7)\n"
        )
        self.assertEqual(code, 1)
        self.assertEqual(result["status"], "failed")
        self.assertEqual(result["test_run"]["exit_code"], 7)

    def test_empty_selection_is_incomplete(self):
        code, result = self.observe("print('no tests run')\n")
        self.assertEqual(code, 1)
        self.assertEqual(result["status"], "incomplete")
        self.assertIsNone(result["counts"])

    def test_dispatch_failure_is_not_a_clean_pass(self):
        code, result = self.observe(
            "print('tests/example' if '-dry-run' in sys.argv else "
            "'100% of tests passed (1/1), 2 test-server dispatch failure(s) -- a connection died mid-run')\n"
        )
        self.assertEqual(code, 1)
        self.assertEqual(result["counts"]["dispatch_failures"], 2)

    def test_timeout_is_recorded(self):
        code, result = self.observe(
            "if '-dry-run' in sys.argv: print('tests/example')\n"
            "else: time.sleep(10)\n", timeout=0.1
        )
        self.assertEqual(code, 1)
        self.assertEqual(result["status"], "infrastructure_failure")
        self.assertEqual(result["test_run"]["error"], "timeout")

    def test_missing_binary_is_recorded(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            result = shadow.execute([str(root / "missing")], root / "log", 1)
            self.assertIsNone(result["exit_code"])
            self.assertIn("error", result)

    def test_setup_failure_report_does_not_invent_counts(self):
        with tempfile.TemporaryDirectory() as temporary:
            environment = dict(os.environ, SETUP_OUTCOME="failure", TEST_OUTCOME="skipped",
                               CLEANLINESS_OUTCOME="success", GITHUB_STEP_SUMMARY=str(Path(temporary) / "step.md"))
            result = subprocess.run([sys.executable, str(HELPER), "report", "--output-dir", temporary],
                                    env=environment, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0)
            observation = json.loads((Path(temporary) / "result.json").read_text())
            self.assertEqual(observation["status"], "infrastructure_failure")
            self.assertEqual(observation["step_outcomes"]["test"], "skipped")
            self.assertNotIn("counts", observation)
            self.assertTrue((Path(temporary) / "step.md").exists())


if __name__ == "__main__":
    unittest.main()
