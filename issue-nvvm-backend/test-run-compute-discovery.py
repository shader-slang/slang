#!/usr/bin/env python3
"""Check discovery capacity, target normalization and frozen source preservation."""

import csv
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "discovery", ROOT / "issue-nvvm-backend/run-compute-discovery.py"
)
DISCOVERY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(DISCOVERY)
CENSUS = DISCOVERY._load_census_module(ROOT / "issue-nvvm-backend")
MANIFEST = ROOT / "issue-nvvm-backend/discovery-corpus.manifest.tsv"


class DiscoveryContractTests(unittest.TestCase):
    def test_colored_unavailable_entry_point_keeps_discovery_diagnostic(self):
        census = DISCOVERY._load_census_module(ROOT / "issue-nvvm-backend")
        DISCOVERY._install_discovery_classifier(census)
        plain = "error[E36107]: unavailable features in entry point 'main'\n"
        colored = "\x1b[31merror[E36107]:\x1b[0m unavailable features in entry point 'main'\n"
        expected = (
            "infrastructure",
            "E36107: target capability requirements make the entry point unavailable to CUDA",
            "target capability requirement",
        )
        for mode in census.MODES:
            with self.subTest(mode=mode):
                self.assertEqual(census._classify_result(1, plain, mode), expected)
                self.assertEqual(census._classify_result(1, colored, mode), expected)

    def _write_synthetic_manifest(self, directory, count):
        """Write real unique source contracts so capacity checks exercise the complete loader."""
        tests = Path(directory) / "tests"
        tests.mkdir()
        manifest = Path(directory) / "manifest.tsv"
        tags = ",".join(sorted(DISCOVERY.REQUIRED_SELECTION_TAGS))
        with manifest.open("w", newline="") as stream:
            writer = csv.DictWriter(
                stream,
                fieldnames=["source", "source_test_ordinal", "selection_tags", "rationale"],
                delimiter="\t",
            )
            writer.writeheader()
            for index in range(count):
                source = f"case-{index}.slang"
                (tests / source).write_text(
                    "//TEST:COMPARE_COMPUTE(filecheck-buffer=EXPECTED):"
                    "-cuda -output-using-type\n",
                    encoding="utf-8",
                )
                writer.writerow(dict(source=source, source_test_ordinal=0,
                                     selection_tags=tags, rationale="capacity boundary"))
        return tests, manifest

    def test_selection_uses_native_indices_and_rejects_disabled_or_ignored(self):
        text = ("//DISABLE_TEST:COMPARE_COMPUTE:-cuda\n"
                "//DIAGNOSTIC_TEST:SIMPLE:-target spirv\n"
                "//TEST:SIMPLE:-target hlsl\n"
                "// TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type\n")
        directive = DISCOVERY._find_compare_directive(text, 3, CENSUS)
        self.assertEqual(directive["line"], 4)
        self.assertEqual(directive["arguments"], "-cpu -output-using-type")
        for ordinal in (0, 1, 2):
            with self.subTest(ordinal=ordinal), self.assertRaises(ValueError):
                DISCOVERY._find_compare_directive(text, ordinal, CENSUS)
        with self.assertRaises(ValueError):
            DISCOVERY._find_compare_directive(text + "// TEST_IGNORE_FILE\n", 3, CENSUS)

    def test_manifest_capacity_accepts_boundaries(self):
        for count in (50, 100, 101, 128):
            with self.subTest(count=count), tempfile.TemporaryDirectory() as directory:
                tests, manifest = self._write_synthetic_manifest(directory, count)
                workloads, tags = DISCOVERY._load_discovery_workloads(
                    tests, manifest, set(), CENSUS
                )
                self.assertEqual(
                    [row["id"] for row in workloads],
                    [f"case-{index}.slang#discovery-1" for index in range(count)],
                )
                self.assertEqual(tags, {tag: count for tag in DISCOVERY.REQUIRED_SELECTION_TAGS})

    def test_manifest_capacity_rejects_outside_bounds(self):
        for count in (49, 129):
            with self.subTest(count=count), tempfile.TemporaryDirectory() as directory:
                tests, manifest = self._write_synthetic_manifest(directory, count)
                with self.assertRaisesRegex(
                    SystemExit, f"50--128 workloads, found {count}"
                ):
                    DISCOVERY._load_discovery_workloads(tests, manifest, set(), CENSUS)

    def test_native_cuda_contract_uses_each_requested_mode(self):
        arguments = DISCOVERY._adapt_arguments_to_cuda(
            "-cuda -output-using-type -dispatch-size 4,1,1 -capability cuda_sm_8_0 "
            "-Xslang -emit-cuda-via-nvvm -Xslang -O3"
        )
        workload = dict(arguments=arguments, capability="cuda_sm_8_0", categories="(compute)",
                        command="COMPARE_COMPUTE(filecheck-buffer=EXPECTED)")
        for mode, (direct, optimization) in CENSUS.MODES.items():
            with self.subTest(mode=mode):
                directive = CENSUS._directive_for_mode(workload, mode)
                tokens = directive.split(":", 2)[2].split()
                self.assertEqual(tokens.count("-cuda"), 1)
                self.assertEqual([t for t in tokens if t in ("-O0", "-O3")],
                                 [f"-O{optimization}"])
                self.assertEqual(tokens.count("-emit-cuda-via-nvvm"), int(direct))
                self.assertIn("-output-using-type -dispatch-size 4,1,1", directive)
                self.assertIn("COMPARE_COMPUTE(filecheck-buffer=EXPECTED)", directive)

    def test_existing_non_cuda_contract_keeps_oracle_arguments(self):
        self.assertEqual(
            DISCOVERY._adapt_arguments_to_cuda(
                "-vk -profile cs_6_0 -shaderobj -output-using-type -dispatch-size 2,1,1"
            ),
            "-shaderobj -output-using-type -dispatch-size 2,1,1 -cuda",
        )

    def test_frozen_source_overlap_is_rejected_even_for_native_cuda(self):
        frozen, _ = DISCOVERY._audit_frozen_v1(ROOT / "issue-nvvm-backend/census.slice-146.tsv")
        workloads, _ = DISCOVERY._load_discovery_workloads(ROOT / "tests", MANIFEST, frozen, CENSUS)
        native = next(row for row in workloads if "-cuda" in row["original_arguments"].split())
        with self.assertRaisesRegex(SystemExit, "source overlaps frozen corpus v1"):
            DISCOVERY._load_discovery_workloads(
                ROOT / "tests", MANIFEST, frozen | {native["source"].lower()}, CENSUS
            )

    def test_duplicate_source_is_rejected(self):
        rows = DISCOVERY._read_tsv(MANIFEST)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.tsv"
            with path.open("w", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=list(rows[0]), delimiter="\t")
                writer.writeheader()
                writer.writerows(rows[:-1] + [rows[0]])
            with self.assertRaisesRegex(SystemExit, "duplicate discovery source"):
                DISCOVERY._load_discovery_workloads(ROOT / "tests", path, set(), CENSUS)


if __name__ == "__main__":
    unittest.main()
