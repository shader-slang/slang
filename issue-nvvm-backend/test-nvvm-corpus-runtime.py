#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Negative contracts for complete, oracle-qualified CUDA dispatch measurements."""

import contextlib
import io
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("corpus_runtime", REPO / "extras/measure-nvvm-corpus-runtime.py")
RUNTIME = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RUNTIME)
CENSUS = RUNTIME.load("test_census", REPO / "issue-nvvm-backend/run-compute-census.py")


def profile():
    rows = [dict(record="header", schema_version=1, scope=RUNTIME.SCOPE, timing="cuda-event-ms",
                 warmups=3, samples=9),
            dict(record="resources", allocations=2, snapshots=2, reset_bytes=128, outputs=1)]
    rows.extend(dict(record="launch", phase=phase, index=index, device_ms=0.004,
                     output_equal=True, start_callbacks=1, end_callbacks=1)
                for phase, count in (("reference", 1), ("warmup", 3), ("sample", 9))
                for index in range(count))
    rows.append(dict(record="summary", status="completed", launches=13, warmups=3, samples=9))
    return rows


class CorpusRuntimeContracts(unittest.TestCase):
    def test_complete_profile_and_zero_observations(self):
        rows = profile()
        self.assertEqual(RUNTIME.validate_profile(rows), [0.004] * 9)
        rows[-2]["device_ms"] = 0
        self.assertEqual(RUNTIME.validate_profile(rows)[-1], 0)

    def test_rejects_partial_duplicate_and_reordered_launches(self):
        for rows in (profile()[:-1], profile() + [profile()[-1]],
                     profile()[:3] + profile()[4:], profile()[:2] + list(reversed(profile()[2:-1])) + profile()[-1:]):
            with self.subTest(rows=rows), self.assertRaises(ValueError):
                RUNTIME.validate_profile(rows)

    def test_rejects_bad_events_and_output(self):
        for field, value in (("device_ms", float("nan")), ("device_ms", float("inf")),
                             ("device_ms", -1), ("device_ms", True), ("output_equal", False),
                             ("output_equal", 1), ("start_callbacks", 0), ("end_callbacks", 2),
                             ("index", 4)):
            rows = profile()
            rows[2][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                RUNTIME.validate_profile(rows)

    def test_rejects_missing_reset_or_wrong_scope(self):
        for index, field, value in ((1, "reset_bytes", 0), (1, "outputs", 0),
                                    (1, "snapshots", -1), (0, "scope", "kernel-only"),
                                    (0, "timing", "host-ms"), (-1, "status", "failed")):
            rows = profile()
            rows[index][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                RUNTIME.validate_profile(rows)

    def test_authoritative_batch_with_ordinary_failure(self):
        output = "passed test: 'a' (1 ms)\nFAILED test: 'b' (2 ms)\n50% of tests passed (1/2)\n"
        result = RUNTIME.parse_batch(output, ["a", "b"], dict(return_code=1, timed_out=False), CENSUS)
        self.assertEqual(result, {"a": "passed", "b": "FAILED"})

    def test_rejects_missing_duplicate_extra_or_wrong_batch_summary(self):
        valid = "passed test: 'a'\n100% of tests passed (1/1)\n"
        bad = ["100% of tests passed (1/1)\n", valid + "passed test: 'a'\n",
               valid.replace("'a'", "'b'"), valid.replace("(1/1)", "(2/2)"),
               valid + "100% of tests passed (1/1)\n", valid.replace("passed test", "ignored test"),
               valid.replace("(1/1)", "(1/1), 1 tests ignored"), valid + "test-server loss detected\n"]
        for output in bad:
            with self.subTest(output=output), self.assertRaises(ValueError):
                RUNTIME.parse_batch(output, ["a"], dict(return_code=0, timed_out=False), CENSUS)

    def test_rejects_batch_exit_mismatch_and_timeout(self):
        output = "passed test: 'a'\n100% of tests passed (1/1)\n"
        for result in (dict(return_code=1, timed_out=False), dict(return_code=0, timed_out=True),
                       dict(return_code=-9, timed_out=False)):
            with self.subTest(result=result), self.assertRaises(ValueError):
                RUNTIME.parse_batch(output, ["a"], result, CENSUS)

    def test_rejects_reordered_execution(self):
        output = "passed test: 'b'\npassed test: 'a'\n100% of tests passed (2/2)\n"
        with self.assertRaises(ValueError):
            RUNTIME.parse_batch(output, ["a", "b"], dict(return_code=0, timed_out=False), CENSUS)

    def make_report(self, root):
        nr = RUNTIME.load("test_results", REPO / "issue-nvvm-backend/nvvm-results.py")
        selected = [dict(id="case", corpus="frozen", source="case.slang")]
        cells, results, batches, mirrors = [], [], [], {}
        for i, (case_id, mode, rd) in enumerate(RUNTIME.expected_schedule(selected)):
            generated = root / f"shader{i}.slang"
            generated.write_text("fixture")
            mirrors[str(generated)] = nr.sha(generated)
            actual = Path(str(generated) + ".actual.txt")
            actual.write_text("correct output")
            sidecar = root / f"profile{i}.jsonl"
            sidecar.write_text("\n".join(json.dumps(row) for row in profile()) + "\n")
            name = generated.name + " (cuda)"
            log = root / f"batch{i}.log"
            log.write_text(f"passed test: '{name}'\n100% of tests passed (1/1)\n")
            cell = dict(id=case_id, corpus="frozen", source="case.slang", mode=mode, round=rd,
                        generated=str(generated), profile=str(sidecar))
            cells.append(cell)
            results.append(dict(cell, status="measured", sample_ms=[0.004] * 9, oracle="passed",
                                log=str(log), profile_sha256=nr.sha(sidecar), reference_output=nr.reference(actual)))
            batches.append(dict(log=str(log), log_sha256=nr.sha(log), names=[name], return_code=0, timed_out=False))
        provenance = root / "provenance.json"
        RUNTIME.save(provenance, {})
        RUNTIME.save(root / "manifest.json", dict(selected=selected, cells=cells,
                     provenance=nr.reference(provenance), mirror_input_sha256=mirrors))
        report = dict(status="completed", manifest=nr.reference(root / "manifest.json"), cells=results, batches=batches)
        RUNTIME.save(root / "results.json", report)
        return report

    def test_short_intervals_remain_visible_without_ratios(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.make_report(root)
            with patch.object(RUNTIME, "REPO", root), contextlib.redirect_stdout(io.StringIO()):
                RUNTIME.summarize(root)
            summary = json.loads((root / "summary.json").read_text())
            self.assertEqual(summary["corpora"]["frozen"], {"short-interval": 1})
            self.assertIsNone(summary["pairs"][0]["nvrtc_over_nvvm_o3"])
            self.assertEqual(summary["pairs"][0]["nvrtc_o3_ms"], .004)

    def test_report_rejects_swapped_artifacts_false_pass_and_missing_round(self):
        for mutation in ("profile", "generated", "reference", "log", "oracle", "excluded", "missing"):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                report = self.make_report(root)
                first, second = report["cells"][:2]
                if mutation in ("profile", "generated", "log"):
                    first[mutation] = second[mutation]
                elif mutation == "reference":
                    first["reference_output"] = second["reference_output"]
                elif mutation == "oracle":
                    first["oracle"] = "FAILED"
                elif mutation == "excluded":
                    first["status"] = "excluded"
                else:
                    report["cells"].pop()
                RUNTIME.save(root / "results.json", report)
                with patch.object(RUNTIME, "REPO", root), self.assertRaises(ValueError):
                    RUNTIME.summarize(root)

    def test_report_rejects_changed_mirror_dependency(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            report = self.make_report(root)
            Path(report["cells"][0]["generated"]).write_text("changed oracle or include")
            with patch.object(RUNTIME, "REPO", root), self.assertRaises(ValueError):
                RUNTIME.summarize(root)

    def test_fixed_corpora_selection(self):
        discovery = RUNTIME.load("test_discovery", REPO / "issue-nvvm-backend/run-compute-discovery.py")
        rows = RUNTIME.select(CENSUS, discovery, None)
        self.assertEqual(len(rows), 580)
        self.assertEqual(sum(row["corpus"] == "frozen" for row in rows), 452)
        self.assertEqual(len({row["source"] for row in rows}), 576)
        with self.assertRaises(ValueError):
            RUNTIME.select(CENSUS, discovery, ["missing-case"])

    def test_mirror_preserves_inputs_and_reference_with_explicit_g0(self):
        discovery = RUNTIME.load("test_discovery_mirror", REPO / "issue-nvvm-backend/run-compute-discovery.py")
        row = RUNTIME.select(CENSUS, discovery, None)[0]
        with tempfile.TemporaryDirectory() as directory:
            root, cells = RUNTIME.prepare_mirror(CENSUS, Path(directory), [row], "nvvm-o3", 0)
            generated = Path(cells[0]["generated"]).read_text()
            directive, source = generated.split("\n", 1)
            original = CENSUS.source_without_test_directives(CENSUS._read_text(REPO / "tests" / row["source"]))
            self.assertEqual(source, original)
            self.assertIn(" -g0 -compile-arg -g0 -compile-arg -O3 ", directive)
            self.assertIn(" -cuda-dispatch-warmups 3 -cuda-dispatch-samples 9", directive)
            self.assertEqual(root, Path(directory) / "round-0/mirrors/nvvm-o3")


if __name__ == "__main__":
    unittest.main()
