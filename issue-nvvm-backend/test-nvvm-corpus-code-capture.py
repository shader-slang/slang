#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Contracts preventing incorrect runtime-PTX attribution and false qualification."""

import copy
import importlib.util
from pathlib import Path
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("capture", REPO / "extras/capture-nvvm-corpus-code.py")
CAPTURE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CAPTURE)


class CaptureContracts(unittest.TestCase):
    def manifest(self):
        current = [dict(id="first", source="a.slang", arguments="-cuda", corpus="frozen"),
                   dict(id="second", source="b.slang", arguments="-cuda", corpus="discovery")]
        manifest = dict(kind="corpus-dispatch-performance", selection="full-frozen-and-discovery",
                        debug_info="g0", selected=[dict(row, source_sha256="hash") for row in current])
        return manifest, current

    def test_selection_pins_order_and_source_before_filter(self):
        manifest, current = self.manifest()
        self.assertEqual([row["id"] for row in CAPTURE.select_manifest(manifest, current, ["second"], lambda _: "hash")], ["second"])
        with self.assertRaises(ValueError):
            CAPTURE.select_manifest(manifest, current, ["second"], lambda source: "changed" if source == "a.slang" else "hash")

    def test_changed_inventory_commands_or_protocol_rejected(self):
        manifest, current = self.manifest()
        variants = []
        for key, value in (("kind", "other"), ("selection", "subset"), ("debug_info", "g2")):
            changed = copy.deepcopy(manifest); changed[key] = value; variants.append(changed)
        changed = copy.deepcopy(manifest); changed["selected"].reverse(); variants.append(changed)
        changed = copy.deepcopy(manifest); changed["selected"].append(changed["selected"][0]); variants.append(changed)
        changed = copy.deepcopy(manifest); changed["selected"][0]["arguments"] = "-cuda -O0"; variants.append(changed)
        for variant in variants:
            with self.subTest(variant=variant), self.assertRaises(ValueError):
                CAPTURE.select_manifest(variant, current, None, lambda _: "hash")

    def test_duplicate_unknown_selector_rejected(self):
        manifest, current = self.manifest()
        for ids in (["missing"], ["first", "first"]):
            with self.assertRaises(ValueError):
                CAPTURE.select_manifest(manifest, current, ids, lambda _: "hash")

    def inspect(self, texts):
        with tempfile.TemporaryDirectory() as temporary:
            paths = []
            for name, text in texts.items():
                path = Path(temporary) / name; path.write_text(text); paths.append(dict(path=str(path)))
            return CAPTURE.inspect_ptx(paths)

    def test_unique_final_ptx_ignores_other_intermediate_ordinals(self):
        result = self.inspect({"artifact-999.llvm-ir-asm": "unrelated", "artifact-4.ptx": ".target sm_80\n.visible .entry specialized_name() {}"})
        self.assertEqual(result["status"], "captured")
        self.assertEqual(result["entry"], "specialized_name")

    def test_multiple_dumps_never_selects_highest_ordinal(self):
        ptx = ".target sm_80\n.entry kernel() {}"
        self.assertEqual(self.inspect({"1.ptx": ptx, "100.ptx": ptx})["status"], "ambiguous-ptx")
        self.assertEqual(self.inspect({"1.cu": "source"})["status"], "missing-ptx")

    def test_module_inventory_and_target_rejected(self):
        for ptx in (".target sm_89\n.entry kernel() {}", ".target sm_80\n.func helper() {}",
                    ".target sm_80\n.entry a() {}\n.entry b() {}", ".target sm_80\n.target sm_80\n.entry a() {}"):
            self.assertEqual(self.inspect({"1.ptx": ptx})["status"], "unsupported-module-inventory")

    def test_comment_entries_are_not_counted(self):
        self.assertEqual(self.inspect({"1.ptx": ".target sm_80\n// .entry fake() {}\n/* .entry fake2() {} */\n.entry kernel() {}"})["status"], "captured")

    def test_known_failure_and_regressions_cannot_qualify(self):
        captured = dict(status="captured")
        self.assertEqual(CAPTURE.qualification("passed", "correct", captured), "qualified")
        self.assertEqual(CAPTURE.qualification("FAILED", "correct", captured), "oracle-regression-review-required")
        self.assertEqual(CAPTURE.qualification("passed", "preflight-gap", captured), "oracle-transition-review-required")
        self.assertEqual(CAPTURE.qualification("FAILED", "wrong-output", captured), "known-oracle-gap")
        self.assertEqual(CAPTURE.qualification("unverified", "correct", captured), "unverified-oracle")
        self.assertEqual(CAPTURE.qualification("passed", "correct", dict(status="ambiguous-ptx")), "capture-excluded")


if __name__ == "__main__":
    unittest.main()
