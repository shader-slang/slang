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
            "DA:12,12\n"
            "end_of_record\n",
        )
        self.assertEqual(result.stderr, "")

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
        self.assertIn("error: manifest is missing a valid counter count", result.stderr)

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
        self.assertIn("error: invalid v2 line entry in manifest", result.stderr)


if __name__ == "__main__":
    unittest.main()
