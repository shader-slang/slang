#!/usr/bin/env python3
"""
Tests for the llvm_cov_json module — `llvm-cov export -format=json`
parser. Verifies FileRecord shape, segment-derived line hits, native
branch / function / region population, and the per-file `summary` →
`auth_override` route that drives index totals.

    python3 -m unittest tools/coverage-html/tests/test_llvm_cov_json.py
"""

import gzip
import json
import os
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
FIXTURES = os.path.join(HERE, "fixtures")
sys.path.insert(0, os.path.abspath(os.path.join(HERE, os.pardir)))

from lcov_io import LcovParseError  # noqa: E402
from llvm_cov_json import parse_llvm_cov_json  # noqa: E402

SAMPLE = os.path.join(FIXTURES, "llvm-cov-json-sample.json")


class FixtureParserTests(unittest.TestCase):
    """End-to-end parser checks against the hand-crafted fixture."""

    def setUp(self):
        self.records = parse_llvm_cov_json(SAMPLE)
        self.by_path = {r.path: r for r in self.records}

    def test_two_files_present(self):
        self.assertEqual(set(self.by_path), {"src/foo.cpp", "src/bar.cpp"})

    def test_summary_drives_totals(self):
        # Numbers below mirror the per-file `summary` block in
        # llvm-cov-json-sample.json — they're what the auth_override
        # promotes into the FileRecord properties.
        foo = self.by_path["src/foo.cpp"]
        self.assertEqual(foo.total_lines, 12)
        self.assertEqual(foo.hit_lines, 9)
        self.assertEqual(foo.total_functions, 2)
        self.assertEqual(foo.hit_functions, 2)
        self.assertEqual(foo.total_branches, 6)
        self.assertEqual(foo.hit_branches, 3)
        self.assertEqual(foo.total_regions, 8)
        self.assertEqual(foo.hit_regions, 6)

    def test_zero_summary_zeros_out(self):
        bar = self.by_path["src/bar.cpp"]
        self.assertEqual(bar.total_lines, 4)
        self.assertEqual(bar.hit_lines, 0)
        self.assertEqual(bar.hit_functions, 0)
        self.assertEqual(bar.total_regions, 1)
        self.assertEqual(bar.hit_regions, 0)

    def test_branches_dict_has_two_arms_per_branch(self):
        foo = self.by_path["src/foo.cpp"]
        # Three JSON branches → six dict entries (true + false arm).
        self.assertEqual(len(foo.branches), 6)
        # Each LLVM branch maps to (line, idx, 0) and (line, idx, 1).
        for line in (10, 12, 14):
            self.assertIn((line, 0 if line == 10 else 1 if line == 12 else 2, 0), foo.branches)
            self.assertIn((line, 0 if line == 10 else 1 if line == 12 else 2, 1), foo.branches)

    def test_functions_attached_to_primary_file(self):
        foo = self.by_path["src/foo.cpp"]
        self.assertIn("_Z3fooi", foo.functions)
        self.assertIn("_Z3barv", foo.functions)
        # `first_line` comes from the first non-skipped region.
        self.assertEqual(foo.functions["_Z3fooi"].first_line, 1)
        self.assertEqual(foo.functions["_Z3barv"].first_line, 9)

    def test_regions_filter_to_code_kind(self):
        foo = self.by_path["src/foo.cpp"]
        # Two functions, each contributing one CodeRegion (kind 0).
        # Branch regions (kind 4) are filtered out.
        self.assertEqual(len(foo.regions), 2)
        spans = sorted((r.line_start, r.line_end) for r in foo.regions)
        self.assertEqual(spans, [(1, 6), (9, 16)])
        for r in foo.regions:
            self.assertGreater(r.count, 0)

    def test_segments_drive_per_line_hits(self):
        foo = self.by_path["src/foo.cpp"]
        # Lines 1, 5, 9 are region entry points with count=5 — the
        # walker propagates that count over the segment-covered range.
        self.assertEqual(foo.lines[1], 5)
        self.assertEqual(foo.lines[5], 5)
        self.assertEqual(foo.lines[9], 5)

    def test_lcov_specific_fields_unset(self):
        # The JSON parser doesn't populate LCOV's reported_* fields,
        # so validate_totals should treat them as a no-op.
        for r in self.records:
            self.assertIsNone(r.reported_lf)
            self.assertIsNone(r.reported_lh)
            self.assertIsNone(r.reported_brf)


class SegmentBoundaryTests(unittest.TestCase):
    """Direct tests on _line_hits_from_segments — the half-open
    interval rule prevents a deactivating successor from inheriting
    the previous segment's active count."""

    def _walk(self, segments):
        from llvm_cov_json import _line_hits_from_segments
        return _line_hits_from_segments(segments)

    def test_half_open_at_segment_boundary(self):
        # Segment 1: line 10 with count=5 active.
        # Segment 2: line 20 with count=0 (deactivation-style).
        # Line 20 belongs to segment 2, not segment 1, so count=0.
        segments = [
            [10, 1, 5, True, True, False],
            [20, 1, 0, True, True, False],
            [25, 1, 0, False, False, False],  # closing marker
        ]
        hits = self._walk(segments)
        # Lines 10..19 covered by segment 1 with count=5.
        for ln in range(10, 20):
            self.assertEqual(hits.get(ln), 5, f"line {ln}")
        # Line 20 is segment 2's domain (count=0).
        self.assertEqual(hits.get(20), 0)

    def test_final_segment_attributed_only_to_its_own_line(self):
        # When no successor exists, the segment covers just its own
        # start line — not an unbounded suffix of the file.
        segments = [
            [5, 1, 7, True, True, False],
        ]
        hits = self._walk(segments)
        self.assertEqual(hits, {5: 7})

    def test_five_element_segments_accepted(self):
        # LLVM versions predating IsGapRegion (~March 2020) emit only
        # five fields per segment. The walker should treat the missing
        # is_gap_region as False rather than dropping the segment.
        segments = [
            [1, 1, 3, True, True],
            [5, 1, 0, False, False],  # closing marker, also 5-element
        ]
        hits = self._walk(segments)
        for ln in range(1, 5):
            self.assertEqual(hits.get(ln), 3, f"line {ln}")
        self.assertNotIn(5, hits)

    def test_deactivating_segment_skipped(self):
        segments = [
            [10, 1, 0, False, False, False],  # has_count=False
            [11, 1, 5, True, True, False],
            [13, 1, 0, False, False, False],  # closing marker
        ]
        hits = self._walk(segments)
        # The has_count=False segment contributes nothing; only
        # segment 2 fires, covering lines 11..12.
        self.assertNotIn(10, hits)
        self.assertEqual(hits.get(11), 5)
        self.assertEqual(hits.get(12), 5)
        self.assertNotIn(13, hits)


class FormatGuardTests(unittest.TestCase):
    def test_missing_file(self):
        with self.assertRaises(LcovParseError):
            parse_llvm_cov_json("/nonexistent/path/coverage.json")

    def test_malformed_json(self):
        with tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False
        ) as fh:
            fh.write("{ this is not json")
            tmp = fh.name
        try:
            with self.assertRaises(LcovParseError):
                parse_llvm_cov_json(tmp)
        finally:
            os.remove(tmp)

    def test_wrong_type_field(self):
        with tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False
        ) as fh:
            json.dump({"type": "something.else", "data": []}, fh)
            tmp = fh.name
        try:
            with self.assertRaises(LcovParseError) as ctx:
                parse_llvm_cov_json(tmp)
            self.assertIn("not an llvm-cov JSON export", str(ctx.exception))
        finally:
            os.remove(tmp)


class GzipInputTests(unittest.TestCase):
    def test_gz_extension_auto_decompresses(self):
        with open(SAMPLE, "rb") as fh:
            payload = fh.read()
        with tempfile.NamedTemporaryFile(
            suffix=".json.gz", delete=False
        ) as fh:
            tmp = fh.name
        try:
            with gzip.open(tmp, "wb") as gz:
                gz.write(payload)
            recs = parse_llvm_cov_json(tmp)
            paths = sorted(r.path for r in recs)
            self.assertEqual(paths, ["src/bar.cpp", "src/foo.cpp"])
        finally:
            os.remove(tmp)


class IsJsonInputTests(unittest.TestCase):
    """The shared extension policy used by both CLI tools."""

    def test_json_extensions(self):
        from llvm_cov_json import is_json_input
        self.assertTrue(is_json_input("coverage.json"))
        self.assertTrue(is_json_input("coverage.json.gz"))
        self.assertTrue(is_json_input("/abs/path/COVERAGE.JSON"))
        self.assertTrue(is_json_input("snake_case.json.GZ"))

    def test_non_json_extensions(self):
        from llvm_cov_json import is_json_input
        self.assertFalse(is_json_input("coverage.lcov"))
        self.assertFalse(is_json_input("coverage.info"))
        self.assertFalse(is_json_input("coverage.lcov.gz"))
        self.assertFalse(is_json_input("coverage.info.gz"))
        self.assertFalse(is_json_input("plain.txt"))
        # No extension at all → not JSON. Renderer still treats it
        # as LCOV (the historical fallback).
        self.assertFalse(is_json_input("coverage"))


class DuplicateFilenameTests(unittest.TestCase):
    """`llvm-cov export` emits one `data[]` block per binary; if a
    shared header appears in multiple blocks the parser silently
    keeps the *last* occurrence. Slang's pipeline never invokes
    `llvm-cov export` with multiple binaries so duplicates don't
    arise in practice — this test pins the documented
    last-wins precondition so a future refactor can't change it
    without an explicit signal."""

    def test_last_block_wins_for_duplicate_filenames(self):
        with tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False
        ) as fh:
            json.dump(
                {
                    "type": "llvm.coverage.json.export",
                    "data": [
                        {
                            "files": [
                                {
                                    "filename": "shared.h",
                                    "branches": [],
                                    "expansions": [],
                                    "mcdc_records": [],
                                    "segments": [],
                                    "summary": {
                                        "lines": {"count": 10, "covered": 3},
                                        "functions": {"count": 1, "covered": 0},
                                        "branches": {"count": 0, "covered": 0},
                                        "regions": {"count": 5, "covered": 1},
                                    },
                                }
                            ],
                            "functions": [],
                        },
                        {
                            "files": [
                                {
                                    "filename": "shared.h",
                                    "branches": [],
                                    "expansions": [],
                                    "mcdc_records": [],
                                    "segments": [],
                                    "summary": {
                                        "lines": {"count": 10, "covered": 9},
                                        "functions": {"count": 1, "covered": 1},
                                        "branches": {"count": 0, "covered": 0},
                                        "regions": {"count": 5, "covered": 5},
                                    },
                                }
                            ],
                            "functions": [],
                        },
                    ],
                },
                fh,
            )
            tmp = fh.name
        try:
            recs = parse_llvm_cov_json(tmp)
            self.assertEqual(len(recs), 1)
            rec = recs[0]
            self.assertEqual(rec.path, "shared.h")
            # Last block wins: covered=9 hits, not 3 from the first.
            self.assertEqual(rec.hit_lines, 9)
            self.assertEqual(rec.hit_regions, 5)
        finally:
            os.remove(tmp)


class EmptyDocumentTests(unittest.TestCase):
    def test_no_files(self):
        with tempfile.NamedTemporaryFile(
            "w", suffix=".json", delete=False
        ) as fh:
            json.dump(
                {
                    "type": "llvm.coverage.json.export",
                    "data": [{"files": [], "functions": []}],
                },
                fh,
            )
            tmp = fh.name
        try:
            self.assertEqual(parse_llvm_cov_json(tmp), [])
        finally:
            os.remove(tmp)


if __name__ == "__main__":
    unittest.main()
