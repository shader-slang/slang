#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""CPU-only contracts for directive selection, oracle copying and result classification."""
import importlib.machinery
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest import mock

runner_path = Path(__file__).with_name("run-compute-census.py")
loader = importlib.machinery.SourceFileLoader("census_contract_runner", str(runner_path))
spec = importlib.util.spec_from_loader(loader.name, loader)
runner = importlib.util.module_from_spec(spec)
loader.exec_module(runner)


class CensusClassificationContracts(unittest.TestCase):
    def test_colored_filecheck_matches_plain_failure(self):
        # FileCheck's styled CHECK-NEXT diagnostic from the corrupt-output290 control.
        plain = (
            "slang-test: fixture.slang:54:36: error: "
            "CHECK-NEXT: expected string not found in input\n"
        )
        colored = (
            "\x1b[1mslang-test: fixture.slang:54:36: \x1b[0m\x1b[0;1;31merror: "
            "\x1b[0m\x1b[1mCHECK-NEXT: expected string not found in input\n\x1b[0m"
        )
        for mode in runner.MODES:
            with self.subTest(mode=mode):
                self.assertEqual(runner._classify_result(1, plain, mode),
                                 ("runtime-mismatch", "", ""))
                self.assertEqual(runner._classify_result(1, colored, mode),
                                 runner._classify_result(1, plain, mode))

    def test_colored_failure_phase_and_shape_match_plain(self):
        wrapper = (
            "slang-test: fixture:20: error: CHECK: expected string not found in input\n"
            "EXPECTED{{{7}}}\nACTUAL{{{9}}}\n"
        )
        cases = (
            ("error[E30001]: invalid source", "infrastructure"),
            ("error[E52017]: direct NVVM lowering does not support Slang IR instruction "
             "or shape 'helper function parameter: unsupported'", "preflight"),
            ("error[E52018]: downstream compiler failed", "provider"),
            ("NVVM IR verification failed", "provider"),
            ("libNVVM failed", "provider"),
            ("no CUDA device", "infrastructure"),
        )
        for mode in runner.MODES:
            for diagnostic, expected in cases:
                with self.subTest(mode=mode, diagnostic=diagnostic):
                    plain = diagnostic + "\n" + wrapper
                    # Styling can divide tokens as well as surround entire lines.
                    colored = "\x1b[1m" + "\x1b[0m\x1b[31m".join(diagnostic) + "\x1b[m\n" + wrapper
                    result = runner._classify_result(1, plain, mode)
                    self.assertEqual(result[0], expected)
                    self.assertEqual(runner._classify_result(1, colored, mode), result)

    def test_styled_summary_counts_and_strict_success(self):
        cases = (
            ("100% of tests passed (1/1)\n", "correct"),
            ("100% of tests passed (1/1) 1 tests ignored\n", "infrastructure"),
            ("100% of tests passed (1/1) unexpected status\n", "infrastructure"),
            ("100% of tests passed (1/1)\n100% of tests passed (1/1)\n", "infrastructure"),
            ("100% of tests passed (1/)\n", "infrastructure"),
            ("0% of tests passed (0/0)\n", "infrastructure"),
            ("0% of tests passed (0/1)\n", "infrastructure"),
            ("100% of tests passed (2/2)\n", "infrastructure"),
            ("arbitrary output\n", "infrastructure"),
        )
        for plain, expected in cases:
            colored = "\x1b[32m" + plain.replace("tests passed", "tests \x1b[1mpassed") + "\x1b[0m"
            with self.subTest(plain=plain):
                self.assertEqual(runner.execution_counts(colored), runner.execution_counts(plain))
                self.assertEqual(runner._classify_result(0, plain, "nvvm-o0")[0], expected)
                self.assertEqual(runner._classify_result(0, colored, "nvvm-o0"),
                                 runner._classify_result(0, plain, "nvvm-o0"))
        self.assertNotEqual(
            runner._classify_result(1, "\x1b[32m100% of tests passed (1/1)\x1b[0m\n",
                                    "nvvm-o0")[0], "correct")

    def test_non_sgr_text_is_preserved(self):
        # Cursor commands, OSC titles, malformed escapes and printable escape spellings
        # are not styling; removing them could turn invalid text into a passing summary.
        controls = ("\x1b[2J", "\x1b]0;title\x07", "\x1b[31", r"\x1b[31m", "[31m")
        for control in controls:
            with self.subTest(control=control):
                text = "100% of tests " + control + "passed (1/1)\n"
                self.assertEqual(runner.normalize_diagnostic_output(text), text)
                self.assertIsNone(runner.execution_counts(text))
                self.assertEqual(runner._classify_result(0, text, "nvvm-o0")[0],
                                 "infrastructure")

    def test_run_one_preserves_raw_log_and_return_code(self):
        output = (
            "slang-test: fixture:54:36: \x1b[31merror: \x1b[0m"
            "CHECK-NEXT: expected string not found in input\n"
            "\x1b[1m0% of tests passed (0/1)\x1b[0m\n"
        )
        workload = dict(id="case#1", source="case.slang", source_line=1,
                        source_test_ordinal=0, cuda_ordinal=0, capability="cuda_sm_8_0",
                        reference_derived_from_direct=False)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with mock.patch.object(runner.subprocess, "run", return_value=
                                   runner.subprocess.CompletedProcess([], 1, output)):
                result = runner._run_one(root, root / "results", root / "mirror", workload,
                                         "nvvm-o0", root / "provider", root / "slang-test")
            self.assertEqual((root / result["log"]).read_bytes(), output.encode("utf-8"))
            self.assertEqual(result["return_code"], 1)
            self.assertEqual(result["classification"], "runtime-mismatch")
            self.assertEqual(result["execution_counts"],
                             dict(passed=0, executed=1, ignored=0, other_summary_status=0))

    def test_failure_phase_precedes_secondary_output_comparison(self):
        wrapper = "EXPECTED{{{\nexpected value\n}}}\nACTUAL{{{\nactual value\n}}}\n"
        cases = (
            ("compiler-rejection", "error[E55215]: multisampled texture is not supported on this target", "infrastructure"),
            ("other-compiler-error", "error[E30001]: invalid source", "infrastructure"),
            ("compiler-abort", "error[E99997]: compiler aborted", "infrastructure"),
            ("preflight", "error[E52017]: direct NVVM lowering does not support input", "preflight"),
            ("provider", "error[E52018]: downstream compiler failed", "provider"),
            ("provider-marker", "NVVM IR verification failed", "provider"),
            ("buffer-mismatch", "", "runtime-mismatch"),
            ("warning-with-mismatch", "warning[W12345]: warning only", "runtime-mismatch"),
        )
        for mode in ("nvrtc-o3", "nvvm-o0", "nvvm-o3"):
            for name, diagnostic, expected in cases:
                with self.subTest(mode=mode, name=name):
                    self.assertEqual(runner._classify_result(1, diagnostic + "\n" + wrapper, mode)[0], expected)
            with self.subTest(mode=mode, name="filecheck-without-compiler-error"):
                output = "slang-test: fixture.slang:20: error: BUF: expected string not found in input\n"
                self.assertEqual(runner._classify_result(1, output, mode)[0], "runtime-mismatch")
            with self.subTest(mode=mode, name="successful-executed-test"):
                self.assertEqual(runner._classify_result(0, "100% of tests passed (1/1)\n", mode)[0], "correct")


class CensusEnumerationContracts(unittest.TestCase):
    def test_native_indices_select_indexed_oracle_in_both_mirrors(self):
        # Disabled and diagnostic entries still occupy native test indices.
        source = (
            "//DISABLE_TEST:COMPARE_COMPUTE:-cuda\n"
            "//DIAGNOSTIC_TEST:SIMPLE:-target spirv\n"
            "//TEST:SIMPLE:-target hlsl\n"
            "  //// TEST(compute):COMPARE_COMPUTE:-cuda -output-using-type\n"
            "//TEST_INPUT:ubuffer(data=[7]):name=output\n"
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            tests = root / "tests"
            tests.mkdir()
            original = tests / "case.slang"
            original.write_text(source)
            for suffix, text in (("", "default"), (".1", "wrong old index"), (".3", "native oracle")):
                Path(str(original) + suffix + ".expected.txt").write_text(text)
            workloads, _ = runner.discover_workloads(tests)
            self.assertEqual(len(workloads), 1)
            workload = workloads[0]
            self.assertEqual((workload["source_line"], workload["source_test_ordinal"]), (4, 3))
            output = root / "census"
            mirror = output / "mirrors/nvrtc-o3"
            runner.prepare_mode(tests, output, mirror, workloads, "nvrtc-o3")
            discovery_path = Path(__file__).with_name("run-compute-discovery.py")
            spec = importlib.util.spec_from_file_location("oracle_discovery", discovery_path)
            discovery = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(discovery)
            discovery_mirror = discovery._prepare_mirror_tree(tests, root / "discovery")
            discovery._populate_mirror_for_mode(tests, discovery_mirror, workloads, "nvrtc-o3", runner)
            for generated_root in (mirror, discovery_mirror):
                generated = generated_root / runner._generated_relative_path(workload)
                self.assertEqual(Path(str(generated) + ".expected.txt").read_text(), "native oracle")
                contents = generated.read_text()
                self.assertIn("//TEST_INPUT:", contents)
                self.assertNotIn("DIAGNOSTIC_TEST", contents)
                self.assertNotIn("DISABLE_TEST", contents)
                self.assertEqual(len(runner.enumerate_test_directives(contents)), 1)

    def test_spacing_disabling_and_whole_file_ignore(self):
        with tempfile.TemporaryDirectory() as directory:
            tests = Path(directory)
            (tests / "active.slang").write_text(
                "//test:COMPARE_COMPUTE:-cuda\n"
                "// TEST :COMPARE_COMPUTE:-cuda\n"
                "//DISABLED_TEST:COMPARE_COMPUTE:-cuda\n"
                "// /TEST:COMPARE_COMPUTE:-cuda\n"
                "//DISABLE_TEST:COMPARE_COMPUTE:-cuda\n"
                "//DISABLE_DIAGNOSTIC_TEST:SIMPLE:-target hlsl\n"
                "//TEST_CATEGORY(compute)\n"
                "\t/// TEST:COMPARE_COMPUTE:-cuda\n"
            )
            (tests / "ignored.slang").write_text(
                "// TEST:COMPARE_COMPUTE:-cuda\n//// TEST_IGNORE_FILE\n"
            )
            workloads, _ = runner.discover_workloads(tests)
            self.assertEqual([(w["source"], w["source_test_ordinal"]) for w in workloads],
                             [("active.slang", 2)])


if __name__ == "__main__":
    unittest.main()
