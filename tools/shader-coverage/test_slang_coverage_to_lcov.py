#!/usr/bin/env python3

import json
import pathlib
import subprocess
import sys
import tempfile
import unittest


SCRIPT = pathlib.Path(__file__).with_name("slang-coverage-to-lcov.py")


class SlangCoverageToLcovTests(unittest.TestCase):
    def run_converter(self, manifest, counters_text="", check=True):
        with tempfile.TemporaryDirectory() as td:
            td_path = pathlib.Path(td)
            manifest_path = td_path / "shader.coverage-mapping.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            return subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT),
                    "--manifest",
                    str(manifest_path),
                    "--counters-text",
                    "-",
                    "--test-name",
                    "shader_coverage",
                ],
                input=counters_text,
                capture_output=True,
                text=True,
                check=check,
            )

    def test_reads_v1_manifest_without_version_field(self):
        manifest = {
            "binding": {"space": 0, "binding": 0},
            "counters": 2,
            "entries": [
                {"index": 0, "file": "shader.slang", "line": 12},
                {"index": 1, "file": "shader.slang", "line": 12},
            ],
        }

        result = self.run_converter(manifest, "5 7\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "DA:12,12\n"
            "end_of_record\n",
        )
        self.assertEqual(result.stderr, "")

    def test_filters_unattributable_entries_from_lcov(self):
        manifest = {
            "version": 1,
            "binding": {"space": 0, "binding": 0},
            "counters": 3,
            "entries": [
                {"index": 0, "file": "shader.slang", "line": 12},
                {"index": 1, "file": "", "line": 44},
                {"index": 2, "file": "shader.slang", "line": 0},
            ],
        }

        result = self.run_converter(manifest, "5 7 11\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "DA:12,5\n"
            "end_of_record\n",
        )
        self.assertIn(
            "note: skipped 2 coverage entries without attributable source location",
            result.stderr,
        )

    def test_reads_v2_source_entry_manifest(self):
        manifest = {
            "version": 2,
            "counter_count": 4,
            "buffer": {"name": "__slang_coverage"},
            "entries": [
                {
                    "kind": "line",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                },
                {
                    "kind": "line",
                    "counter": 1,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                },
                {
                    "kind": "branch",
                    "counter": 2,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 13,
                    "branch_site": 1,
                    "branch_arm": 2,
                },
                {
                    "kind": "line",
                    "counter": None,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 14,
                },
            ],
        }

        result = self.run_converter(manifest, "5 7 11 13\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "BRDA:13,1,2,11\n"
            "BRF:1\n"
            "BRH:1\n"
            "DA:12,12\n"
            "end_of_record\n",
        )
        # Counterless source entries stay in the manifest/metadata, but
        # cannot be represented as LCOV records.
        self.assertIn(
            "note: skipped 1 line entries without a runtime counter not representable in LCOV",
            result.stderr,
        )

    def test_reads_v2_function_and_branch_entries(self):
        manifest = {
            "version": 2,
            "counter_count": 5,
            "entries": [
                {
                    "kind": "line",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 10,
                },
                {
                    "kind": "function",
                    "counter": 1,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 11,
                    "function": "helper",
                    "function_mangled": "_S6helper",
                },
                {
                    "kind": "branch",
                    "counter": 2,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                    "branch_site": 4,
                    "branch_arm": 1,
                },
                {
                    "kind": "branch",
                    "counter": 3,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                    "branch_site": 4,
                    "branch_arm": 2,
                },
            ],
        }

        result = self.run_converter(manifest, "5 3 2 0 99\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "FN:11,helper\n"
            "FNDA:3,helper\n"
            "FNF:1\n"
            "FNH:1\n"
            "BRDA:12,4,1,2\n"
            "BRDA:12,4,2,0\n"
            "BRF:2\n"
            "BRH:1\n"
            "DA:10,5\n"
            "end_of_record\n",
        )
        self.assertEqual(result.stderr, "")

    def test_reads_v2_function_mangled_name_fallback(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "function",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 11,
                    "function_mangled": "_S6helper",
                },
            ],
        }

        result = self.run_converter(manifest, "3\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "FN:11,_S6helper\n"
            "FNDA:3,_S6helper\n"
            "FNF:1\n"
            "FNH:1\n"
            "end_of_record\n",
        )
        self.assertEqual(result.stderr, "")

    def test_emits_multiple_functions_with_stable_ordering(self):
        manifest = {
            "version": 2,
            "counter_count": 3,
            "entries": [
                {
                    "kind": "function",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 10,
                    "function": "zeta",
                },
                {
                    "kind": "function",
                    "counter": 1,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 20,
                    "function": "beta",
                },
                {
                    "kind": "function",
                    "counter": 2,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 20,
                    "function": "alpha",
                },
            ],
        }

        result = self.run_converter(manifest, "3 0 5\n")

        self.assertEqual(
            result.stdout,
            "TN:shader_coverage\n"
            "SF:shader.slang\n"
            "FN:10,zeta\n"
            "FN:20,alpha\n"
            "FN:20,beta\n"
            "FNDA:3,zeta\n"
            "FNDA:5,alpha\n"
            "FNDA:0,beta\n"
            "FNF:3\n"
            "FNH:2\n"
            "end_of_record\n",
        )
        self.assertEqual(result.stderr, "")

    def test_skips_v2_function_entries_without_names(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "function",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 11,
                },
            ],
        }

        result = self.run_converter(manifest, "3\n")

        self.assertEqual(result.stdout, "TN:shader_coverage\n")
        self.assertIn(
            "note: skipped 1 function entries without a name not representable in LCOV",
            result.stderr,
        )

    def test_reports_unrepresented_v2_kinds(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "region",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 13,
                },
            ],
        }

        result = self.run_converter(manifest, "5\n")

        self.assertIn(
            "note: skipped 1 entries of kind 'region' not representable in LCOV",
            result.stderr,
        )

    def test_rejects_non_string_v2_kind(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": ["line"],
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 13,
                },
            ],
        }

        result = self.run_converter(manifest, "5\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "error: manifest v2 entry kind must be a string",
            result.stderr,
        )

    def test_rejects_negative_v2_branch_ids(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "branch",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 13,
                    "branch_site": -1,
                    "branch_arm": 0,
                },
            ],
        }

        result = self.run_converter(manifest, "5\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "error: manifest v2 entry branch_site and branch_arm must be non-negative",
            result.stderr,
        )

    def test_emits_zero_hit_branch_arm_as_zero(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "branch",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 13,
                    "branch_site": 1,
                    "branch_arm": 1,
                },
            ],
        }

        result = self.run_converter(manifest, "0\n")

        self.assertIn("BRDA:13,1,1,0\n", result.stdout)
        self.assertIn("BRH:0\n", result.stdout)

    def test_rejects_out_of_range_v2_counter_index(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "line",
                    "counter": 1,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                },
            ],
        }

        result = self.run_converter(manifest, "7\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("error: counter index 1 out of range [0, 1)", result.stderr)

    def test_accepts_empty_v2_entries(self):
        manifest = {
            "version": 2,
            "counter_count": 0,
            "entries": [],
        }

        result = self.run_converter(manifest)

        self.assertEqual(result.stdout, "TN:shader_coverage\n")
        self.assertEqual(result.stderr, "")

    def test_emits_multiple_branch_sites_on_same_line(self):
        manifest = {
            "version": 2,
            "counter_count": 2,
            "entries": [
                {
                    "kind": "branch",
                    "counter": 0,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                    "branch_site": 1,
                    "branch_arm": 1,
                },
                {
                    "kind": "branch",
                    "counter": 1,
                    "mode": "count",
                    "file": "shader.slang",
                    "line": 12,
                    "branch_site": 2,
                    "branch_arm": 1,
                },
            ],
        }

        result = self.run_converter(manifest, "3 4\n")

        self.assertIn("BRDA:12,1,1,3\n", result.stdout)
        self.assertIn("BRDA:12,2,1,4\n", result.stdout)
        self.assertIn("BRF:2\n", result.stdout)
        self.assertIn("BRH:2\n", result.stdout)

    def test_rejects_unknown_manifest_version(self):
        manifest = {
            "version": 999,
            "counter_count": 1,
            "entries": [],
        }

        result = self.run_converter(manifest, check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("error: unsupported manifest version 999", result.stderr)

    def test_reports_malformed_manifest_counter_count(self):
        manifest = {
            "version": 2,
            "counter_count": "not-an-int",
            "entries": [],
        }

        result = self.run_converter(manifest, check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "error: manifest counter count must be an integer", result.stderr
        )

    def test_reports_non_integral_manifest_counter_count(self):
        manifest = {
            "version": 2,
            "counter_count": 1.9,
            "entries": [],
        }

        result = self.run_converter(manifest, check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "error: manifest counter count must be an integer", result.stderr
        )

    def test_reports_boolean_v2_counter(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "line",
                    "counter": True,
                    "file": "shader.slang",
                    "line": 12,
                },
            ],
        }

        result = self.run_converter(manifest, "0\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "error: manifest v2 entry counter must be an integer", result.stderr
        )

    def test_reports_missing_manifest_entries(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
        }

        result = self.run_converter(manifest, "0\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("error: manifest is missing 'entries'", result.stderr)

    def test_reports_invalid_v2_line_entry(self):
        manifest = {
            "version": 2,
            "counter_count": 1,
            "entries": [
                {
                    "kind": "line",
                    "counter": 0,
                    "file": "shader.slang",
                },
            ],
        }

        result = self.run_converter(manifest, "0\n", check=False)

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("error: invalid v2 entry in manifest", result.stderr)


if __name__ == "__main__":
    unittest.main()
