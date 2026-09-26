#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""CPU-only contracts for distinguishing compiler errors from GPU output mismatches."""
import importlib.machinery
import importlib.util
from pathlib import Path
import unittest

runner_path = Path(__file__).with_name("run-compute-census.py")
loader = importlib.machinery.SourceFileLoader("census_contract_runner", str(runner_path))
spec = importlib.util.spec_from_loader(loader.name, loader)
runner = importlib.util.module_from_spec(spec)
loader.exec_module(runner)


class CensusClassificationContracts(unittest.TestCase):
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


if __name__ == "__main__":
    unittest.main()
