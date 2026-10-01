#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""CPU contracts for selection, acceptance ownership and changed input handling."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("tiers", HERE / "nvvm-corpus-tiers.py")
tiers = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tiers)
census = tiers.load("tier_test_census", HERE / "run-compute-census.py")


class TierContracts(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name)
        (self.repo / "tests").mkdir()
        (self.repo / "tests/a.slang").write_text("// oracle is part of the source\n")
        (self.repo / "tests/b.slang").write_text("// new permanent input\n")
        self.workloads = [dict(id=f"{name}.slang#cuda-1", source=f"{name}.slang",
                               source_test_ordinal=0, origin=origin)
                          for name, origin in (("a", "frozen"), ("b", "candidate"))]
        self.passed = dict(id=self.workloads[0]["id"], mode="nvvm-o3", classification="correct",
                           return_code=0, execution_counts=tiers.CORRECT_COUNTS.copy())
        self.failed = dict(self.passed, mode="nvrtc-o3", classification="infrastructure", return_code=1,
                           execution_counts=dict(passed=0, executed=1, ignored=0, other_summary_status=0))
        self.baseline = dict(corpora={"frozen": {"fresh_cell_outcomes": [self.passed, self.failed]}},
                             runtime_input_sha256={"tests/a.slang": tiers.sha(self.repo / "tests/a.slang")})
        self.focused = dict(features={})
        self.metadata = dict(admissions=[], smoke=[dict(id=self.passed["id"], mode="nvvm-o3")],
                             exploratory_priority={}, oracle_inputs={})

    def inventory(self):
        return tiers.build_inventory(self.repo, self.baseline, self.focused, self.metadata, self.workloads)

    def test_smoke_is_exact_working_subset_and_failures_remain_independent(self):
        before = copy.deepcopy(self.baseline)
        cells = self.inventory()
        smoke = tiers.select(cells, "smoke")
        self.assertEqual([(r["id"], r["mode"]) for r in smoke], [(self.passed["id"], "nvvm-o3")])
        self.assertEqual(smoke[0]["tier"], "working")
        first = tiers.select(cells, "exploratory", limit=1)[0]
        self.assertEqual((first["mode"], first["reason"]), ("nvrtc-o3", "known-failure"))
        self.assertEqual(self.baseline, before)

    def test_changed_source_stays_working_with_unresolved_input_identity(self):
        (self.repo / "tests/a.slang").write_text("changed source/oracle\n")
        cell = tiers.select(self.inventory(), "working")[0]
        self.assertEqual(cell["tier"], "working")
        self.assertEqual(len(cell["input_changes"]), 1)
        self.assertNotEqual(cell["input_changes"][0]["accepted_sha256"], cell["input_changes"][0]["current_sha256"])

    def test_external_oracle_needs_original_identity_and_detects_later_changes(self):
        oracle = self.repo / "tests/a.slang.expected.txt"
        oracle.write_text("expected\n")
        self.assertTrue(tiers.select(self.inventory(), "working")[0]["input_changes"])
        self.metadata["oracle_inputs"]["tests/a.slang.expected.txt"] = tiers.sha(oracle)
        self.assertFalse(tiers.select(self.inventory(), "working")[0]["input_changes"])
        oracle.write_text("weakened oracle\n")
        self.assertTrue(tiers.select(self.inventory(), "working")[0]["input_changes"])
        oracle.unlink()
        changes = tiers.select(self.inventory(), "working")[0]["input_changes"]
        self.assertEqual(changes[0]["current_sha256"], None)

    def test_focused_admission_binds_exact_runtime_cell_and_source(self):
        result = dict(self.passed, id=self.workloads[1]["id"])
        self.focused["features"]["feature"] = dict(status="accepted-focused", result=result,
            identity={"source_sha256": {"tests/b.slang": tiers.sha(self.repo / "tests/b.slang")}})
        admission = dict(id=result["id"], mode=result["mode"], feature="feature", outcome_path=["result"],
                         source_hash_path=["identity", "source_sha256", "tests/b.slang"])
        self.metadata["admissions"] = [admission]
        self.assertEqual(len(tiers.select(self.inventory(), "working")), 2)
        admission["mode"] = "nvvm-o0"
        with self.assertRaises(ValueError):
            self.inventory()

    def test_unknown_duplicate_and_unaccepted_smoke_cells_reject(self):
        for extra in (self.metadata["smoke"][0], dict(id="unknown", mode="nvvm-o3"),
                      dict(id=self.failed["id"], mode="nvrtc-o3")):
            with self.subTest(extra=extra):
                original = self.metadata["smoke"]
                self.metadata["smoke"] = original + [extra]
                with self.assertRaises(ValueError):
                    self.inventory()
                self.metadata["smoke"] = original
        self.passed["execution_counts"]["ignored"] = 1
        with self.assertRaises(ValueError):
            self.inventory()

    def test_modes_and_limits_do_not_expand_selection(self):
        cells = self.inventory()
        self.assertEqual(len(tiers.select(cells, "exploratory", modes=["nvvm-o0"], limit=1)), 1)
        for tier, modes, limit in (("smoke", None, 1), ("working", ["bad"], None),
                                   ("exploratory", ["nvvm-o0", "nvvm-o0"], None)):
            with self.assertRaises(ValueError):
                tiers.select(cells, tier, modes, limit)

    def test_exact_runner_cells_prevent_cartesian_expansion_and_path_substitution(self):
        rows = [dict(id="a", source="a.slang", source_test_ordinal=0),
                dict(id="b", source="b.slang", source_test_ordinal=1)]
        path = self.repo / "cells.json"
        valid = [dict(rows[0], mode="nvvm-o0"), dict(rows[1], mode="nvvm-o3")]
        path.write_text(json.dumps(valid))
        selected = census.read_cell_selection(path, rows, ["nvvm-o0", "nvvm-o3"])
        self.assertEqual(census.workloads_for_mode(rows, "nvvm-o0", selected), [rows[0]])
        self.assertTrue(census.inventory_matches(valid, rows, tiers.MODES, selected))
        self.assertFalse(census.inventory_matches(valid + [dict(rows[0], mode="nvvm-o3")], rows, tiers.MODES, selected))
        for bad in ([valid[0], valid[0]], [dict(valid[0], source="../outside.slang")],
                    [dict(valid[0], source_test_ordinal=1)], [dict(valid[0], id="unknown")],
                    [dict(valid[0], mode="nvrtc-o3")]):
            path.write_text(json.dumps(bad))
            with self.assertRaises(ValueError):
                census.read_cell_selection(path, rows, ["nvvm-o0", "nvvm-o3"])

    def test_repository_inventory_preserves_baseline_and_smoke_input_proofs(self):
        repo = HERE.parent
        baseline = tiers.read(HERE / "accepted-baseline.json")
        cells = tiers.build_inventory(repo, baseline, tiers.read(HERE / "focused-evidence.json"),
                                      tiers.read(HERE / "corpus-tiers.json"))
        indexed = {(r["id"], r["mode"]): r for r in cells}
        self.assertEqual(len(indexed), len(cells))
        for corpus in baseline["corpora"].values():
            for row in corpus["fresh_cell_outcomes"]:
                self.assertEqual(indexed[row["id"], row["mode"]]["accepted_outcome"], row)
                if tiers.correct(row):
                    self.assertEqual(indexed[row["id"], row["mode"]]["tier"], "working")
        self.assertFalse(any(r["input_changes"] for r in tiers.select(cells, "smoke")))


if __name__ == "__main__":
    unittest.main()
