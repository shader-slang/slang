#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Acceptance contracts; no compiler, CUDA or GPU is required."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock
from types import SimpleNamespace

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
    def _stage_fixture(self, directory):
        """Use independently chosen durations; inventory validation is tested separately."""
        cell = {"id": "fixture-nvrtc-o3", "backend": "nvrtc", "optimization": 3,
                "workload": "fixture", "entry": "main"}
        rows = []
        for rd, scale in ((0, 1), (1, 2)):
            for wall, builtin in ((100, 10), (200, 80), (300, 30), (10000, 999)):
                phases = {"loadBuiltinModule": builtin * scale, "frontEndExecute": 10 * scale,
                          "generateOutput": 27 * scale, "linkAndOptimizeIR": 5 * scale,
                          "loadDownstreamCompiler": 0, "emitEntryPointsSourceFromIR": 7 * scale,
                          "nvrtcDownstreamCompile": 20 * scale, "nvrtcCompileProgram": 20 * scale,
                          "nvrtcCreateProgram": 0, "nvrtcGetPTXSize": 0, "nvrtcGetPTX": 0}
                log = Path(directory) / f"{len(rows)}.log"
                log.write_text("".join(f"[*] {name} {2 if name == 'loadBuiltinModule' else 1} {value:.2f}ms\n"
                                       for name, value in phases.items()))
                rows.append({"id": cell["id"], "round": rd, "warmup": wall == 10000,
                             "elapsed_seconds": wall * scale / 1000,
                             "phase_ms": phases, "log": str(log)})
        return {"kind": "material", "cells": [cell], "samples": rows}

    def test_stage_residuals_and_percentages_are_aggregated_per_sample(self):
        with tempfile.TemporaryDirectory() as directory:
            data = results.stage_attribution(self._stage_fixture(directory))
        groups = data["cells"][0]["groups"]
        # Round0 residuals are [53,83,233]; subtracting marginal medians gives133.
        self.assertEqual(groups["0"]["stages"]["other"]["ms"]["median"], 83)
        self.assertNotEqual(groups["0"]["stages"]["other"]["ms"]["median"], 200 - 30 - 37)
        # Builtin percentages are [10,40,10]; ratio of marginal medians gives15%.
        self.assertEqual(groups["0"]["stages"]["builtin"]["percent_wall"]["median"], 10)
        self.assertNotEqual(groups["0"]["stages"]["builtin"]["percent_wall"]["median"], 100 * 30 / 200)
        self.assertEqual(groups["1"]["stages"]["other"]["ms"]["median"], 166)
        self.assertEqual(groups["combined"]["wall_ms"]["n"], 6)
        self.assertEqual(groups["0"]["wall_ms"]["n"], 3)
        self.assertEqual(groups["1"]["wall_ms"]["n"], 3)
        self.assertEqual(groups["0"]["stages"]["other"]["ms"]["q1"], 68)
        self.assertEqual(groups["0"]["stages"]["other"]["ms"]["q3"], 158)
        self.assertEqual(data["rounded_residual_count"], 0)

    def test_nvvm_stage_partition_excludes_nested_serialization(self):
        with tempfile.TemporaryDirectory() as directory:
            data = self._stage_fixture(directory)
            data["cells"][0].update(id="fixture-nvvm-o3", backend="nvvm")
            row = data["samples"][0]
            row["id"] = "fixture-nvvm-o3"
            data["samples"] = [row]
            phases = {name: value for name, value in row["phase_ms"].items()
                      if not name.startswith("nvrtc") and name != "emitEntryPointsSourceFromIR"}
            phases.update(generateOutput=41, emitNVVMForEntryPoints=41, nvvmLegalizeIR=2,
                          nvvmPlanEmission=3, nvvmLoadIRBuilder=1, nvvmEmitIR=10,
                          nvvmSerializeIR=6, nvvmSerializeQuery=3, nvvmSerializeWrite=3,
                          nvvmDownstreamCompile=20, nvvmVerifyProgram=4, nvvmCompileProgram=16)
            for name in ("nvvmReadLibdevice", "nvvmCreateProgram", "nvvmAddModule", "nvvmAddLibdevice",
                         "nvvmGetPTXSize", "nvvmGetPTX"):
                phases[name] = 0
            row["phase_ms"] = phases
            Path(row["log"]).write_text("".join(
                f"[*] {name} {2 if name == 'loadBuiltinModule' else 1} {value:.2f}ms\n"
                for name, value in phases.items()))
            stages = results.stage_attribution(data)["cells"][0]["groups"]["combined"]["stages"]
            self.assertEqual(stages["direct_host"]["ms"]["median"], 16)
            self.assertEqual(stages["vendor_verify"]["ms"]["median"], 4)
            self.assertEqual(stages["vendor_compile"]["ms"]["median"], 16)
            self.assertEqual(stages["other"]["ms"]["median"], 39)

    def test_stage_report_rejects_inconsistent_and_impossible_evidence(self):
        for fault in ("count", "missing", "mismatch", "containment", "root", "wall", "nan", "zero"):
            with self.subTest(fault=fault), tempfile.TemporaryDirectory() as directory:
                data = self._stage_fixture(directory)
                row = data["samples"][0]
                log = Path(row["log"])
                if fault == "count":
                    log.write_text(log.read_text().replace("nvrtcCompileProgram 1", "nvrtcCompileProgram 2"))
                elif fault == "missing":
                    log.write_text(log.read_text().replace("[*] nvrtcCompileProgram 1 20.00ms\n", ""))
                elif fault == "mismatch":
                    row["phase_ms"]["nvrtcCompileProgram"] = 19
                elif fault == "root":
                    row["phase_ms"]["generateOutput"] = 90
                    log.write_text(log.read_text().replace("generateOutput 1 27.00ms", "generateOutput 1 90.00ms"))
                elif fault == "containment":
                    # Both sources agree, but the parent is shorter than its child.
                    row["phase_ms"]["nvrtcDownstreamCompile"] = 19
                    log.write_text(log.read_text().replace("nvrtcDownstreamCompile 1 20.00ms", "nvrtcDownstreamCompile 1 19.00ms"))
                else:
                    row["elapsed_seconds"] = {"wall": .001, "nan": float("nan"), "zero": 0}[fault]
                with self.assertRaises(ValueError):
                    results.stage_attribution(data)

    def test_exported_provenance_size_is_independent_of_source_inventory(self):
        runtime = {
            "/build/RelWithDebInfo/bin/slangc": "compiler-hash",
            "/build/RelWithDebInfo/bin/libslang-llvm-nvvm.so": "provider-hash",
            "/build/RelWithDebInfo/lib/libslang-compiler.so": "library-hash",
            "/build/RelWithDebInfo/lib/slang-core-module.bin": "cache-hash",
            "/cuda/lib64/libnvrtc.so": "nvrtc-hash",
            "/cuda/nvvm/lib64/libnvvm.so": "nvvm-hash",
            "/cuda/bin/ptxas": "assembler-hash",
            "/usr/bin/readelf": "inspector-hash",
        }
        metadata = {"revision": "accepted-revision", "platform": "Linux", "build_label": "RelWithDebInfo",
                    "environment": {"CUDA_PATH": "/cuda"}, "device": {"log": "device.log"}}
        raw = dict(metadata, artifact_sha256=dict(runtime), runtime_artifact_sha256=dict(runtime))
        raw["artifact_sha256"].update({f"/repo/tests/input-{index}.slang": "f" * 64
                                       for index in range(7179)})
        before = copy.deepcopy(raw)
        exported = results.compact_provenance(raw)
        self.assertEqual(exported["artifact_sha256"], runtime)
        for key, value in metadata.items():
            self.assertEqual(exported[key], value)
        self.assertEqual(exported["full_identity_count"], 7179 + len(runtime))
        self.assertEqual(exported["identity_scope"], "runtime-and-tools")
        self.assertLess(len(json.dumps(exported)), len(json.dumps(raw)) // 100)
        self.assertEqual(raw, before)
        exported["artifact_sha256"]["/build/RelWithDebInfo/bin/slangc"] = "changed-export"
        self.assertEqual(raw, before)

    def test_legacy_export_preserves_available_identity_without_guessing_roles(self):
        raw = {"revision": "legacy", "artifact_sha256": {"/compiler": "hash", "/source": "hash"}}
        exported = results.compact_provenance(raw)
        self.assertEqual(exported["artifact_sha256"], raw["artifact_sha256"])
        self.assertEqual(exported["identity_scope"], "legacy-full-map")

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


class SurfaceCheckpointContracts(unittest.TestCase):
    def test_mandatory_surface_invocation_and_comparison_control_checkpoint(self):
        # Fake child processes prove orchestration only. Physical proof replay and adversarial
        # readbacks are exercised independently in test-nvvm-surface-results.py.
        for scenario in ("preserved", "bootstrap", "invalid", "timeout"):
            with self.subTest(scenario=scenario), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                output = root / "output"
                output.mkdir()
                compiler, provider = root / "slangc", root / "provider.so"
                compiler.write_bytes(b"fake compiler; never executed")
                provider.write_bytes(b"fake provider; never loaded")
                block = {"schema": 1, "status": "validated", "requested_cells": 3,
                         "identity": {"slangc": results.reference(compiler), "provider": results.reference(provider)},
                         "fresh_cell_outcomes": [{"id": "physical", "mode": mode, "status": "compile-failed"}
                                                 for mode in results.MODES]}
                baseline = {"status": "accepted-full", "runtime_input_sha256": {},
                            "corpora": {name: {"fresh_cell_outcomes": outcomes()} for name in ("frozen", "discovery")}}
                if scenario != "bootstrap":
                    baseline["surfaces"] = block
                baseline_path = root / "baseline.json"
                results.write(baseline_path, baseline)
                args = SimpleNamespace(baseline=baseline_path, output=output, slangc=compiler,
                                       provider=provider, cuda_root=root, build_label="fake", jobs=1)
                runner = SimpleNamespace(read_workloads=lambda *a: {}, load_toolkit_helpers=lambda: None)
                provenance = {"artifact_sha256": {str(compiler): results.sha(compiler), str(provider): results.sha(provider)}}
                results.write(output / "provenance.json", provenance)
                commands = []
                def fake_run(command, log, environment, timeout):
                    command = list(map(str, command)); commands.append(command)
                    name = Path(log).stem
                    folder = output / name; folder.mkdir()
                    Path(log).write_text("CPU fake process\n")
                    if name == "runtime":
                        fixtures = ["a", "b", "c", "d"]
                        results.write(folder / "results.json", {"status": "passed", "expected_fixtures": fixtures,
                            "results": [{"fixture": f, "classification": "correct", "execution_counts":
                                {"passed": 1, "executed": 1, "ignored": 0, "other_summary_status": 0}} for f in fixtures]})
                    elif name in ("frozen", "discovery"):
                        results.write(folder / "results.json", outcomes())
                    elif name == "complex":
                        results.write(folder / "results.json", {"cells": [{"id": "material", "status": "passed"}]})
                    return {"command": command, "log": str(log), "return_code": 1 if name == "surfaces" else 0,
                            "timed_out": name == "surfaces" and scenario == "timeout"}
                surface_module = results.load_surface_results()
                with mock.patch.object(results, "configure", return_value=(runner, {}, provenance)), \
                     mock.patch.object(results, "run", side_effect=fake_run), \
                     mock.patch.object(results, "cells_for", return_value=[{"id": "material"}]), \
                     mock.patch.object(results, "load_surface_results", return_value=surface_module), \
                     mock.patch.object(surface_module, "validate_report", return_value=block,
                                       side_effect=ValueError("false pass") if scenario == "invalid" else None) as validate:
                    record = results.checkpoint(args)
                surface_commands = [cmd for cmd in commands if any(x.endswith("validate-nvvm-surfaces.py") for x in cmd)]
                self.assertEqual(len(surface_commands), 1)
                self.assertNotIn("--cases", surface_commands[0])
                self.assertNotIn("--modes", surface_commands[0])
                self.assertEqual(surface_commands[0][surface_commands[0].index("--provider")+1], str(provider.parent))
                expected = {"preserved": "passed", "bootstrap": "review-required", "invalid": "failed", "timeout": "failed"}[scenario]
                self.assertEqual(record["status"], expected)
                self.assertEqual(validate.call_count, 0 if scenario == "timeout" else 1)
                if scenario in ("preserved", "bootstrap"):
                    self.assertEqual(results.read(output / "outcomes.json")["surfaces"], block)
                    self.assertEqual(results.read(output / "surfaces-validated.json"), block)
                    self.assertEqual(results.read(output / "comparison.json")["surfaces"]["status"], expected)


if __name__ == "__main__":
    unittest.main()
