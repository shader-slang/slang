import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


SPEC = importlib.util.spec_from_file_location(
    "summarize_tsan", Path(__file__).parents[1] / "ci" / "summarize-tsan.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class TSanSummaryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.logs = Path(self.temp.name)
        self.env = {
            "BUILD_OUTCOME": "success",
            "TSAN_CONCURRENT_EXIT": "0",
            "TSAN_TEST_EXIT": "0",
            "TSAN_SLANGC_TEST_EXIT": "0",
        }
        for name in ("concurrent-compile.txt", "slang-test.txt"):
            (self.logs / name).write_text("100% of tests passed (1/1), 4 tests ignored\n")
        (self.logs / "slangc-test.txt").write_text("0 failed, out of 1 tests\n")

    def failures(self):
        return MODULE.summarize(self.logs, self.env)[0]

    def test_completed_clean_run(self):
        self.assertEqual(self.failures(), [])

    def test_child_report_fails_even_when_harness_returns_zero(self):
        (self.logs / "tsan.123").write_text("WARNING: ThreadSanitizer: data race\n")
        self.assertIn("1 nonempty TSan report file(s).", self.failures())

    def test_fatal_runtime_report_without_summary_fails(self):
        (self.logs / "tsan.123").write_text(
            "FATAL: ThreadSanitizer: unexpected memory mapping\n"
        )
        self.assertTrue(self.failures())

    def test_crash_without_report_fails(self):
        self.env["TSAN_TEST_EXIT"] = "139"
        self.assertTrue(self.failures())

    def test_missing_test_step_fails(self):
        del self.env["TSAN_CONCURRENT_EXIT"]
        self.assertTrue(self.failures())

    def test_asan_results_cannot_satisfy_missing_tsan_results(self):
        del self.env["TSAN_TEST_EXIT"]
        del self.env["TSAN_SLANGC_TEST_EXIT"]
        self.env.update(SLANG_TEST_EXIT="0", SLANGC_TEST_EXIT="0")
        failures = self.failures()
        self.assertIn("slang-test: exit status missing (not completed).", failures)
        self.assertIn("slangc tests: exit status missing (not completed).", failures)

    def test_ignored_or_unfinished_suite_fails(self):
        for output in ("no tests run\n", "100% of tests passed (0/0)\n", ""):
            with self.subTest(output=output):
                (self.logs / "slang-test.txt").write_text(output)
                self.assertTrue(self.failures())

    def test_stderr_runtime_failure_fails(self):
        with (self.logs / "slang-test.txt").open("a") as stream:
            stream.write("FATAL: ThreadSanitizer: unexpected memory mapping\n")
        self.assertTrue(self.failures())

    def test_slangc_suite_must_complete(self):
        for output in ("", "0 failed, out of 0 tests\n", "1 failed, out of 1 tests\n"):
            with self.subTest(output=output):
                (self.logs / "slangc-test.txt").write_text(output)
                self.assertTrue(self.failures())

    def test_build_failure_fails(self):
        self.env["BUILD_OUTCOME"] = "failure"
        self.assertTrue(self.failures())

    def test_concurrent_test_cannot_be_only_an_expected_failure(self):
        (self.logs / "concurrent-compile.txt").write_text("0% of tests passed (0/1)\n")
        self.assertTrue(self.failures())

    def test_slangc_harness_propagates_compiler_failures(self):
        compiler = self.logs / "slangc"
        script = Path(__file__).parents[2] / "tools/slangc-test/test.sh"
        for compiler_status, harness_status in ((0, 0), (66, 1), (139, 1)):
            with self.subTest(compiler_status=compiler_status):
                compiler.write_text(f"#!/bin/sh\nexit {compiler_status}\n")
                compiler.chmod(0o755)
                env = {**os.environ, "PATH": str(self.logs) + os.pathsep + os.environ["PATH"]}
                result = subprocess.run(
                    ["bash", str(script)], env=env, capture_output=True, text=True
                )
                self.assertEqual(result.returncode, harness_status, result.stdout + result.stderr)
                self.assertIn(f"{harness_status} failed, out of 1 tests", result.stdout)


if __name__ == "__main__":
    unittest.main()
