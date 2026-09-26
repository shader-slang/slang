#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Acceptance contracts; no compiler, CUDA or GPU is required."""
import copy
import importlib.util
import os
from pathlib import Path
import sys
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("results", Path(__file__).with_name("nvvm-results.py"))
results = importlib.util.module_from_spec(spec)
spec.loader.exec_module(results)


def outcomes(name="fixture"):
    return [{"id": name, "mode": mode, "classification": "correct", "return_code": 0,
             "execution_counts": {"passed": 1, "executed": 1, "ignored": 0,
                                  "other_summary_status": 0},
             "diagnostic": "", "canonical_shape": ""} for mode in results.MODES]


def measurements():
    manifest = {"architecture": 80, "workloads": [{"name": "a", "entry_points": ["main"]}]}
    cells = results.cells_for(manifest, results.load_runner())
    ids = [cell["id"] for cell in cells]
    samples, assembly = results.expected_inventory(ids, "material")
    common = {"return_code": 0, "timed_out": False, "elapsed_seconds": 0.5, "log": "unused"}
    return {"kind": "material", "cells": cells, "manifest": manifest,
            "samples": [dict(common, id=name, round=rd, index=index, warmup=warmup,
                             ptx_sha256=name, phase_ms={"SemanticChecking": 2})
                        for name, rd, index, warmup in samples],
            "assembly": [dict(common, id=name, index=index, warmup=warmup, cubin_sha256=name)
                         for name, index, warmup in assembly]}


class Contracts(unittest.TestCase):
    def test_unreviewed_baseline_cannot_erase_regression(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "baseline.json"
            for status in ("review-required", "failed", "running", "comparison-passed", "corpus-passed"):
                results.write(path, {"status": status})
                with self.assertRaises(ValueError):
                    results.accepted_baseline(path)
            results.write(path, {"status": "accepted-full"})
            self.assertEqual(results.accepted_baseline(path)["status"], "accepted-full")

    def test_exact_five_fields(self):
        old = outcomes()
        self.assertEqual(results.compare_rows(old, copy.deepcopy(old))["status"], "passed")
        for key, value in (("classification", "preflight"), ("diagnostic", "changed"),
                           ("canonical_shape", "changed")):
            new = copy.deepcopy(old)
            new[0][key] = value
            self.assertEqual(results.compare_rows(old, new)["status"], "review-required")

    def test_missing_duplicate_and_empty(self):
        old = outcomes()
        self.assertEqual(results.compare_rows(old, old[:-1])["status"], "review-required")
        with self.assertRaises(ValueError):
            results.compare_rows(old, old + old[:1])
        with self.assertRaises(ValueError):
            results.compare_rows(old, [])

    def test_false_pass(self):
        for field, value in (("executed", 0), ("passed", 0), ("ignored", 1),
                             ("other_summary_status", 1)):
            new = outcomes()
            new[0]["execution_counts"][field] = value
            with self.assertRaises(ValueError):
                results.compare_rows(outcomes(), new)
        new = outcomes()
        new[0]["return_code"] = 1
        with self.assertRaises(ValueError):
            results.compare_rows(outcomes(), new)

    def test_additions_need_explicit_complete_correct_inventory(self):
        old, new = outcomes(), outcomes() + outcomes("new")
        self.assertEqual(results.compare_rows(old, new)["status"], "review-required")
        self.assertEqual(results.compare_rows(old, new, True)["status"], "passed")
        self.assertEqual(results.compare_rows(old, new[:-1], True)["status"], "review-required")
        new[-1]["classification"] = "preflight"
        self.assertEqual(results.compare_rows(old, new, True)["status"], "review-required")

    def test_exact_benchmark_inventory(self):
        report = measurements()
        results.validate_measurements(report, False)
        self.assertEqual(len(report["samples"]), 66)
        self.assertEqual(sum(not r["warmup"] for r in report["samples"]), 54)
        for group in ("samples", "assembly"):
            for mutation in ("missing", "duplicate", "reorder", "warmup"):
                altered = copy.deepcopy(report)
                if mutation == "missing":
                    altered[group].pop()
                elif mutation == "duplicate":
                    altered[group].append(altered[group][0])
                elif mutation == "reorder":
                    altered[group].reverse()
                else:
                    altered[group][0]["warmup"] = False
                with self.assertRaises(ValueError):
                    results.validate_measurements(altered, False)

    def test_failed_or_nondeterministic_benchmark(self):
        for field, value in (("return_code", 1), ("timed_out", True), ("ptx_sha256", "other"),
                             ("phase_ms", {}), ("elapsed_seconds", 0), ("elapsed_seconds", float("nan"))):
            report = measurements()
            report["samples"][0][field] = value
            with self.assertRaises(ValueError):
                results.validate_measurements(report, False)

    def test_entire_cell_cannot_disappear(self):
        report = measurements()
        removed = report["cells"].pop()["id"]
        for group in ("samples", "assembly"):
            report[group] = [row for row in report[group] if row["id"] != removed]
        with self.assertRaises(ValueError):
            results.validate_measurements(report, False)

    def test_process_failure_and_timeout_retained(self):
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / "failed.log"
            row = results.run([sys.executable, "-c", "print('retained');raise SystemExit(7)"],
                              log, os.environ, 5)
            self.assertEqual(row["return_code"], 7)
            self.assertIn("retained", log.read_text())
            with self.assertRaises(ValueError):
                results.require_success(row)
            row = results.run([sys.executable, "-c", "import time;time.sleep(10)"], log,
                              os.environ, 0.05)
            self.assertTrue(row["timed_out"])
            with self.assertRaises(ValueError):
                results.require_success(row)

    def test_identity_mutation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "compiler"
            path.write_text("original")
            provenance = {"artifact_sha256": {str(path): results.sha(path)}}
            results.verify_identity(provenance)
            path.write_text("mutated")
            with self.assertRaises(ValueError):
                results.verify_identity(provenance)

    def test_resource_function_boundaries(self):
        rows = results.parse_resources("Function properties for helper\n0 bytes stack frame, "
                                       "0 bytes spill stores, 0 bytes spill loads\n"
                                       "Function properties for kernel\n16 bytes stack frame, "
                                       "4 bytes spill stores, 8 bytes spill loads\n"
                                       "ptxas info : Used 23 registers, 32 bytes smem")
        self.assertIsNone(rows[0]["registers"])
        self.assertEqual(rows[1]["registers"], 23)
        self.assertEqual(rows[1]["stack_bytes"], 16)
        self.assertEqual(rows[1]["spill_load_bytes"], 8)


if __name__ == "__main__":
    unittest.main()
