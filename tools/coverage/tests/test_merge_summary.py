#!/usr/bin/env python3
"""Tests for tools/coverage/merge_summary.py.

The module parses ``llvm-cov report``-style TOTAL lines into a JSON
summary that drives the "Merged (cross-OS)" row on the landing page.
The parser is column-positional and brittle against future llvm-cov
column-layout changes, so the tests pin the contract:

- A valid TOTAL line maps to the right fields with `%` preserved.
- Malformed TOTAL (too few columns) and missing TOTAL both return
  None with an informative stderr message.
- An unmeasured metric (``-`` instead of a percent) decodes as empty
  string, matching how the landing page treats missing data.
- The combined full + slangc invocation produces the schema the
  landing-page generator expects.

Run from repo root:

    python3 -m unittest discover -s tools/coverage/tests -v
"""

import json
import os
import subprocess
import sys
import tempfile
import unittest


HERE = os.path.dirname(os.path.abspath(__file__))
SLANG_TOOL_DIR = os.path.abspath(os.path.join(HERE, os.pardir))
MERGE_SUMMARY = os.path.join(SLANG_TOOL_DIR, "merge_summary.py")

sys.path.insert(0, SLANG_TOOL_DIR)
import merge_summary  # noqa: E402


VALID_TOTAL = (
    "Filename ...\n"
    "----\n"
    "source/foo.cpp  100  10  90.00%  20  5  75.00%  500  50  90.00%  200  20  90.00%\n"
    "----\n"
    "TOTAL           160  36  77.50%  44  11  75.00%  325  91  72.00%  130  34  73.85%\n"
)


def write_tmp(content: str) -> str:
    fd, path = tempfile.mkstemp(suffix=".txt")
    with os.fdopen(fd, "w") as f:
        f.write(content)
    return path


class ParseTotalTests(unittest.TestCase):
    """Unit-level coverage of parse_total."""

    def test_valid_total_preserves_percent_suffix(self):
        path = write_tmp(VALID_TOTAL)
        try:
            r = merge_summary.parse_total(path)
            self.assertIsNotNone(r)
            self.assertEqual(r["line_coverage"], "72.00%")
            self.assertEqual(r["region_coverage"], "77.50%")
            self.assertEqual(r["function_coverage"], "75.00%")
            self.assertEqual(r["branch_coverage"], "73.85%")
            self.assertEqual(r["lines_hit"], 325 - 91)
            self.assertEqual(r["lines_found"], 325)
            self.assertEqual(r["regions_hit"], 160 - 36)
            self.assertEqual(r["functions_hit"], 44 - 11)
            self.assertEqual(r["branches_hit"], 130 - 34)
        finally:
            os.unlink(path)

    def test_missing_file_returns_none(self):
        self.assertIsNone(merge_summary.parse_total("/no/such/file"))

    def test_none_path_returns_none(self):
        self.assertIsNone(merge_summary.parse_total(None))

    def test_no_total_line_returns_none(self):
        path = write_tmp("Filename ...\nsome/file.cpp 1 0 100.00% ...\n")
        try:
            self.assertIsNone(merge_summary.parse_total(path))
        finally:
            os.unlink(path)

    def test_short_total_returns_none(self):
        path = write_tmp("TOTAL 1 2 3\n")
        try:
            self.assertIsNone(merge_summary.parse_total(path))
        finally:
            os.unlink(path)

    def test_dash_cover_decodes_as_empty(self):
        """llvm-cov emits ``-`` when a metric has zero total. The
        landing page should treat that the same as missing data."""
        path = write_tmp(
            "TOTAL  0  0  -  44  11  75.00%  325  91  72.00%  130  34  73.85%\n"
        )
        try:
            r = merge_summary.parse_total(path)
            self.assertIsNotNone(r)
            self.assertEqual(r["region_coverage"], "")
            self.assertEqual(r["regions_hit"], 0)
            self.assertEqual(r["regions_found"], 0)
            # Other metrics still parse normally.
            self.assertEqual(r["line_coverage"], "72.00%")
        finally:
            os.unlink(path)


class BuildSummaryTests(unittest.TestCase):
    """build_summary combines full + slangc parses with metadata."""

    def test_full_only(self):
        full = write_tmp(VALID_TOTAL)
        try:
            s = merge_summary.build_summary(full, None, "2026-05-22", "abc1234")
            self.assertEqual(s["platform"], "merged")
            self.assertEqual(s["platform_detail"], "linux+macos+windows")
            self.assertEqual(s["date"], "2026-05-22")
            self.assertEqual(s["commit"], "abc1234")
            self.assertEqual(s["_version"], 2)
            self.assertEqual(s["line_coverage"], "72.00%")
            self.assertNotIn("slangc_line_coverage", s)
        finally:
            os.unlink(full)

    def test_full_plus_slangc(self):
        full = write_tmp(VALID_TOTAL)
        slangc = write_tmp(
            "TOTAL  100  20  80.00%  20  5  75.00%  200  20  90.00%  100  10  90.00%\n"
        )
        try:
            s = merge_summary.build_summary(full, slangc, "2026-05-22", "abc")
            self.assertEqual(s["line_coverage"], "72.00%")
            self.assertEqual(s["slangc_line_coverage"], "90.00%")
            self.assertEqual(s["slangc_lines_hit"], 180)
            self.assertEqual(s["slangc_lines_found"], 200)
        finally:
            os.unlink(full)
            os.unlink(slangc)

    def test_full_missing_yields_metadata_only(self):
        """When parse_total returns None for full, the summary still
        contains the metadata fields the landing-page generator reads.
        The landing-page guard then suppresses the row because
        line_coverage is absent."""
        s = merge_summary.build_summary(
            "/no/such/file", None, "2026-05-22", "abc"
        )
        self.assertEqual(set(s.keys()), {
            "date", "commit", "platform", "platform_detail", "_version",
        })

    def test_malformed_full_still_includes_slangc(self):
        """A partial signal is better than dropping everything: if the
        slangc parse succeeds but the full parse fails, slangc fields
        are still emitted."""
        slangc = write_tmp(VALID_TOTAL)
        try:
            s = merge_summary.build_summary(
                "/no/such/file", slangc, "2026-05-22", "abc"
            )
            self.assertNotIn("line_coverage", s)
            self.assertIn("slangc_line_coverage", s)
        finally:
            os.unlink(slangc)


class CliTests(unittest.TestCase):
    """End-to-end via the script's argv interface, matching how the
    workflow invokes it."""

    def test_cli_emits_json_with_percent_preserved(self):
        full = write_tmp(VALID_TOTAL)
        slangc = write_tmp(VALID_TOTAL)
        try:
            out = subprocess.check_output(
                [sys.executable, MERGE_SUMMARY, full, slangc, "2026-05-22", "abc"],
                text=True,
            )
            parsed = json.loads(out)
            self.assertEqual(parsed["line_coverage"], "72.00%")
            self.assertEqual(parsed["slangc_line_coverage"], "72.00%")
            self.assertEqual(parsed["_version"], 2)
            self.assertEqual(parsed["platform"], "merged")
        finally:
            os.unlink(full)
            os.unlink(slangc)

    def test_cli_wrong_argc_exits_nonzero(self):
        result = subprocess.run(
            [sys.executable, MERGE_SUMMARY, "only", "one"],
            capture_output=True, text=True,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("usage:", result.stderr)


if __name__ == "__main__":
    unittest.main()
