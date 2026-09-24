#!/usr/bin/env python3
"""Check discovery target normalization and frozen source preservation."""

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
                writer.writerows(rows + [rows[0]])
            with self.assertRaisesRegex(SystemExit, "duplicate discovery source"):
                DISCOVERY._load_discovery_workloads(ROOT / "tests", path, set(), CENSUS)


if __name__ == "__main__":
    unittest.main()
