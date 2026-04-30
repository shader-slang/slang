#!/usr/bin/env python3
"""
Tests for slang-coverage-merge — the multi-LCOV merger.

Run with:
    python3 -m unittest discover -s tools/coverage-html/tests -v
"""

import gzip
import importlib.util
import io
import os
import shutil
import subprocess
import sys
import tempfile
import unittest


HERE = os.path.dirname(os.path.abspath(__file__))
TOOL_DIR = os.path.abspath(os.path.join(HERE, os.pardir))
MERGE_SCRIPT = os.path.join(TOOL_DIR, "slang-coverage-merge.py")
RENDER_SCRIPT = os.path.join(TOOL_DIR, "slang-coverage-html.py")
FIXTURES = os.path.join(HERE, "fixtures")

# Make tools/coverage-html/ importable so we can hit the merge module
# directly for unit-level tests.
sys.path.insert(0, TOOL_DIR)


def _import_merger():
    spec = importlib.util.spec_from_file_location(
        "slang_coverage_merge", MERGE_SCRIPT
    )
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


merger = _import_merger()


# ---------------------------------------------------------------------------
# Path normalization
# ---------------------------------------------------------------------------


class NormalizePathTests(unittest.TestCase):
    def test_strip_prefix_basic(self):
        f = merger.normalize_path
        self.assertEqual(
            f("/repo/root/source/slang/x.cpp", ("/repo/root/",)),
            "source/slang/x.cpp",
        )

    def test_windows_backslashes_normalize_to_forward(self):
        f = merger.normalize_path
        self.assertEqual(
            f(r"D:\repo\src\x.cpp", ("D:/repo/",)),
            "src/x.cpp",
        )

    def test_unmatched_path_passes_through(self):
        f = merger.normalize_path
        self.assertEqual(
            f("/some/unrelated/path/file.c", ("/repo/root/",)),
            "/some/unrelated/path/file.c",
        )

    def test_no_prefixes_passes_through(self):
        f = merger.normalize_path
        # Empty prefix tuple → only backslash normalization is applied.
        self.assertEqual(
            f(r"D:\a\slang\slang\source\slang\x.cpp", ()),
            "D:/a/slang/slang/source/slang/x.cpp",
        )

    def test_longest_prefix_wins(self):
        f = merger.normalize_path
        # When two prefixes both match, the longer one wins so we
        # never leave a partial repo prefix in the output.
        prefixes = ("/a/", "/a/b/")
        self.assertEqual(f("/a/b/c.c", prefixes), "c.c")


# ---------------------------------------------------------------------------
# Aggregation rules
# ---------------------------------------------------------------------------


class MergeRecordsTests(unittest.TestCase):
    def _record(self, path, **kw):
        from lcov_io import FileRecord, Function

        r = FileRecord(path=path)
        r.lines = dict(kw.get("lines", {}))
        r.branches = dict(kw.get("branches", {}))
        r.functions = {
            name: Function(first_line=fl, hits=h)
            for name, (fl, h) in kw.get("functions", {}).items()
        }
        return r

    def test_lines_max(self):
        a = self._record("x.c", lines={1: 5, 2: 0})
        b = self._record("x.c", lines={1: 3, 2: 7})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(len(merged), 1)
        self.assertEqual(merged[0].lines, {1: 5, 2: 7})

    def test_lines_union(self):
        # Lines present in only one input survive.
        a = self._record("x.c", lines={1: 1})
        b = self._record("x.c", lines={2: 1})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].lines, {1: 1, 2: 1})

    def test_lines_zero_hit_only_in_later_input_is_preserved(self):
        # Regression: a 0-hit DA line present only in input B (which
        # is processed second) must still survive the merge. The
        # earlier `dst.lines.get(ln, 0)` default conflated "absent"
        # with "previously 0 hits", silently dropping the record.
        a = self._record("x.c", lines={1: 1})
        b = self._record("x.c", lines={1: 0, 2: 0, 3: 0})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].lines, {1: 1, 2: 0, 3: 0})
        self.assertEqual(merged[0].total_lines, 3)
        # And the bug is order-independent: same outcome reversed.
        merged_rev = merger.merge_records([[b], [a]])
        self.assertEqual(merged_rev[0].lines, {1: 1, 2: 0, 3: 0})

    def test_branches_int_beats_dash(self):
        # `-` (None) loses to any integer; absent input contributes
        # nothing.
        a = self._record("x.c", branches={(5, 0, 0): None})
        b = self._record("x.c", branches={(5, 0, 0): 42})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].branches[(5, 0, 0)], 42)

    def test_branches_int_max(self):
        a = self._record("x.c", branches={(5, 0, 0): 3})
        b = self._record("x.c", branches={(5, 0, 0): 9})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].branches[(5, 0, 0)], 9)

    def test_branches_absent_treated_as_no_info(self):
        # File present in input A but no branches; input B has branches.
        # Merged should carry B's branches (NOT zeroed by A's absence).
        a = self._record("x.c", lines={1: 1})
        b = self._record("x.c", branches={(5, 0, 0): 7})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].branches[(5, 0, 0)], 7)

    def test_functions_max_hits(self):
        a = self._record("x.c", functions={"foo": (10, 5)})
        b = self._record("x.c", functions={"foo": (10, 50)})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].functions["foo"].hits, 50)

    def test_functions_first_line_fallback(self):
        # If one input declared FN with first_line=0 (e.g. FNDA-only,
        # which can happen if the producer drops FN), and another
        # input has the real first_line, the merger should adopt it.
        a = self._record("x.c", functions={"foo": (0, 1)})
        b = self._record("x.c", functions={"foo": (10, 0)})
        merged = merger.merge_records([[a], [b]])
        self.assertEqual(merged[0].functions["foo"].first_line, 10)
        self.assertEqual(merged[0].functions["foo"].hits, 1)

    def test_files_unique_per_input_are_preserved(self):
        a = self._record("a.c", lines={1: 1})
        b = self._record("b.c", lines={1: 1})
        merged = merger.merge_records([[a], [b]])
        paths = {r.path for r in merged}
        self.assertEqual(paths, {"a.c", "b.c"})


# ---------------------------------------------------------------------------
# Function synthesis (--synthesize-functions code path)
# ---------------------------------------------------------------------------


class SynthesizeMissingFunctionsTests(unittest.TestCase):
    """`synthesize_missing_functions` fills FN/FNDA on inputs that
    have DA records but no functions, using sibling inputs' (name,
    first_line) map and the silent input's own DA hit count at
    first_line."""

    def _record(self, path, **kw):
        from lcov_io import FileRecord, Function

        r = FileRecord(path=path)
        r.lines = dict(kw.get("lines", {}))
        r.functions = {
            name: Function(first_line=fl, hits=h)
            for name, (fl, h) in kw.get("functions", {}).items()
        }
        return r

    def test_synthesizes_from_sibling_function_map(self):
        # Input A has FN/FNDA, B only has DA. After synthesis, B
        # gains the function map from A with hit counts taken from
        # B's own DA at first_line.
        a = self._record(
            "x.c",
            functions={"foo": (10, 5), "bar": (20, 0)},
        )
        b = self._record("x.c", lines={10: 7, 20: 0})
        synthesized = merger.synthesize_missing_functions([[a], [b]])
        self.assertEqual(synthesized, 2)
        self.assertEqual(b.functions["foo"].first_line, 10)
        self.assertEqual(b.functions["foo"].hits, 7)   # from DA[10]
        self.assertEqual(b.functions["bar"].first_line, 20)
        self.assertEqual(b.functions["bar"].hits, 0)   # DA[20] = 0

    def test_no_op_when_input_already_has_functions(self):
        a = self._record(
            "x.c", functions={"foo": (10, 5)}, lines={10: 5},
        )
        b = self._record(
            "x.c", functions={"foo": (10, 100)}, lines={10: 100},
        )
        synthesized = merger.synthesize_missing_functions([[a], [b]])
        self.assertEqual(synthesized, 0)
        self.assertEqual(b.functions["foo"].hits, 100)  # unchanged

    def test_no_op_when_no_sibling_has_functions(self):
        # Both inputs have only DA records → nothing to synthesize.
        a = self._record("x.c", lines={1: 1})
        b = self._record("x.c", lines={2: 1})
        synthesized = merger.synthesize_missing_functions([[a], [b]])
        self.assertEqual(synthesized, 0)
        self.assertEqual(a.functions, {})
        self.assertEqual(b.functions, {})

    def test_only_applies_to_files_with_no_functions(self):
        # File present in only one input that has functions; no
        # silent sibling means nothing to synthesize.
        a = self._record("x.c", functions={"foo": (10, 5)}, lines={10: 5})
        synthesized = merger.synthesize_missing_functions([[a]])
        self.assertEqual(synthesized, 0)
        self.assertEqual(a.functions["foo"].hits, 5)


# ---------------------------------------------------------------------------
# LCOV writer round-trip
# ---------------------------------------------------------------------------


class WriteLcovRoundTripTests(unittest.TestCase):
    """Parse → write → re-parse should preserve all coverage data
    we model in FileRecord (lines, branches, functions). Reported
    LF/LH/BRF/BRH/FNF/FNH are recomputed by the writer."""

    def test_round_trip_preserves_coverage(self):
        from lcov_io import (
            FileRecord, Function, parse_lcov, write_lcov,
        )
        original = FileRecord(path="src/foo.c")
        # foo lives at lines 1-2 (DA hits there); bar is at line 100
        # with no DA in its range — so its effective-hit stays False.
        original.lines = {1: 5, 2: 0, 100: 0}
        original.branches = {
            (1, 0, 0): 3, (1, 0, 1): 0, (100, 0, 0): None,
        }
        original.functions = {
            "foo": Function(first_line=1, hits=5),
            "bar": Function(first_line=100, hits=0),
        }

        # Use a unique per-call tempdir so concurrent test runs
        # (CI matrix on a shared runner, pytest-xdist) don't race on
        # the same path.
        tmpdir = tempfile.mkdtemp(prefix="scv-rt-")
        path = os.path.join(tmpdir, "rt.lcov")
        try:
            with open(path, "w", encoding="utf-8") as f:
                write_lcov([original], f)
            roundtrip = parse_lcov(path)
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

        self.assertEqual(len(roundtrip), 1)
        r = roundtrip[0]
        self.assertEqual(r.path, "src/foo.c")
        self.assertEqual(r.lines, {1: 5, 2: 0, 100: 0})
        self.assertEqual(r.branches[(1, 0, 0)], 3)
        self.assertEqual(r.branches[(1, 0, 1)], 0)
        self.assertEqual(r.branches[(100, 0, 0)], None)
        self.assertEqual(r.functions["foo"].first_line, 1)
        self.assertEqual(r.functions["foo"].hits, 5)
        self.assertEqual(r.functions["bar"].first_line, 100)
        self.assertEqual(r.functions["bar"].hits, 0)
        # Recomputed totals match the in-memory model.
        self.assertEqual(r.reported_lf, 3)
        self.assertEqual(r.reported_lh, 1)   # only line 1 had hits>0
        self.assertEqual(r.reported_brf, 3)
        self.assertEqual(r.reported_brh, 1)
        self.assertEqual(r.reported_fnf, 2)
        self.assertEqual(r.reported_fnh, 1)


# ---------------------------------------------------------------------------
# CLI integration
# ---------------------------------------------------------------------------


class CliIntegrationTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        import shutil

        shutil.rmtree(self.tmp, ignore_errors=True)

    def _run(self, *args):
        return subprocess.run(
            [sys.executable, MERGE_SCRIPT, *args],
            capture_output=True,
            text=True,
        )

    def _write(self, name, content):
        path = os.path.join(self.tmp, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        return path

    def _write_gz(self, name, content):
        path = os.path.join(self.tmp, name)
        with gzip.open(path, "wt", encoding="utf-8") as f:
            f.write(content)
        return path

    def test_two_inputs_merge_to_stdout(self):
        a = self._write(
            "a.info",
            "TN:\nSF:foo.c\nDA:1,3\nDA:2,0\nLF:2\nLH:1\nend_of_record\n",
        )
        b = self._write(
            "b.info",
            "TN:\nSF:foo.c\nDA:1,1\nDA:2,5\nLF:2\nLH:2\nend_of_record\n",
        )
        res = self._run(a, b, "--quiet")
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # Stdout is a valid LCOV with the line-max aggregate.
        self.assertIn("DA:1,3", res.stdout)
        self.assertIn("DA:2,5", res.stdout)
        # Recomputed totals.
        self.assertIn("LF:2", res.stdout)
        self.assertIn("LH:2", res.stdout)

    def test_path_normalization_collapses_per_os_paths(self):
        a = self._write(
            "linux.info",
            "TN:\nSF:/runner-a/repo/source/foo.c\n"
            "DA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        b = self._write(
            "macos.info",
            "TN:\nSF:/runner-b/repo/source/foo.c\n"
            "DA:2,1\nLF:1\nLH:1\nend_of_record\n",
        )
        c = self._write(
            "windows.info",
            "TN:\nSF:D:\\runner-c\\repo\\source\\foo.c\n"
            "DA:3,1\nLF:1\nLH:1\nend_of_record\n",
        )
        res = self._run(
            a, b, c,
            "--strip-prefix=/runner-a/repo/",
            "--strip-prefix=/runner-b/repo/",
            "--strip-prefix=D:/runner-c/repo/",
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # All three OS path variants collapse to one repo-relative SF:.
        self.assertEqual(res.stdout.count("SF:source/foo.c"), 1)
        self.assertIn("DA:1,1", res.stdout)
        self.assertIn("DA:2,1", res.stdout)
        self.assertIn("DA:3,1", res.stdout)
        self.assertIn("LF:3", res.stdout)

    def test_extra_strip_prefix_via_flag(self):
        a = self._write(
            "x.info",
            "TN:\nSF:/custom/repo/foo.c\n"
            "DA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        res = self._run(a, "--strip-prefix=/custom/repo/", "--quiet")
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        self.assertIn("SF:foo.c", res.stdout)

    def test_gzip_input_auto_decompressed(self):
        a = self._write_gz(
            "a.info.gz",
            "TN:\nSF:foo.c\nDA:1,2\nLF:1\nLH:1\nend_of_record\n",
        )
        res = self._run(a, "--quiet")
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        self.assertIn("DA:1,2", res.stdout)

    def test_output_to_file(self):
        a = self._write(
            "a.info",
            "TN:\nSF:foo.c\nDA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        out = os.path.join(self.tmp, "merged.lcov")
        res = self._run(a, "-o", out, "--quiet")
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # No body output; file written.
        self.assertEqual(res.stdout, "")
        with open(out) as f:
            self.assertIn("DA:1,1", f.read())

    def test_exit_nonzero_on_corrupt_lcov(self):
        # Mirrors the renderer's corrupt-input test. parse_lcov should
        # raise LcovParseError, the merger should print a diagnostic
        # and exit with code 2.
        corrupt = os.path.join(FIXTURES, "corrupt-bad-da.info")
        res = self._run(corrupt, "--quiet")
        self.assertEqual(res.returncode, 2)
        self.assertIn("non-integer DA fields", res.stderr)

    def test_renderer_consumes_merged_output(self):
        """Smoke: pipe merged output into the renderer; no errors."""
        a = self._write(
            "a.info",
            "TN:\nSF:foo.c\nDA:1,1\nDA:2,0\nFN:1,foo\nFNDA:1,foo\n"
            "BRDA:2,0,0,0\nBRDA:2,0,1,1\nLF:2\nLH:1\nend_of_record\n",
        )
        b = self._write(
            "b.info",
            "TN:\nSF:foo.c\nDA:1,2\nDA:3,1\nFN:1,foo\nFNDA:5,foo\n"
            "BRDA:2,0,0,0\nBRDA:2,0,1,3\nLF:2\nLH:2\nend_of_record\n",
        )
        merged_path = os.path.join(self.tmp, "merged.lcov")
        res = self._run(a, b, "-o", merged_path, "--quiet")
        self.assertEqual(res.returncode, 0, msg=res.stderr)

        out_dir = os.path.join(self.tmp, "out")
        render = subprocess.run(
            [sys.executable, RENDER_SCRIPT, merged_path,
             "--output-dir", out_dir, "--quiet"],
            capture_output=True,
            text=True,
        )
        self.assertEqual(render.returncode, 0, msg=render.stderr)
        self.assertTrue(os.path.exists(os.path.join(out_dir, "index.html")))


class AuthSummaryCliTests(unittest.TestCase):
    """End-to-end: merger consumes per-OS llvm-cov report text dumps,
    writes a merged report-text file, and the renderer reads it."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        import shutil
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _write(self, name, content):
        path = os.path.join(self.tmp, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        return path

    def _report(self, name, rows, total_row):
        """Build a minimal `llvm-cov report`-shaped text file."""
        header = (
            "Filename                                                                                             "
            "Regions    Missed Regions     Cover   Functions  Missed Functions  Executed       Lines      "
            "Missed Lines     Cover    Branches   Missed Branches     Cover\n"
        )
        sep = "-" * 200 + "\n"
        body = "".join(rows) + sep + total_row
        return self._write(name, header + sep + body)

    def test_merge_two_reports_writes_combined(self):
        # Per-OS reports for the same file; merge takes max(total) /
        # min(missed). Stub LCOV input is required by the merger.
        a_lcov = self._write(
            "a.info",
            "TN:\nSF:source/foo.c\nDA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        a_rep = self._report(
            "a-report.txt",
            [
                "source/foo.c"
                "                                                                                         "
                "      100                20    80.00%          10                 2    80.00%         "
                "100                20    80.00%          50                10    80.00%\n"
            ],
            "TOTAL                                                                                                "
            "      100                20    80.00%          10                 2    80.00%         "
            "100                20    80.00%          50                10    80.00%\n",
        )
        b_rep = self._report(
            "b-report.txt",
            [
                "source/foo.c"
                "                                                                                         "
                "      110                15    86.36%          12                 1    91.67%         "
                "110                15    86.36%          55                 5    90.91%\n"
            ],
            "TOTAL                                                                                                "
            "      110                15    86.36%          12                 1    91.67%         "
            "110                15    86.36%          55                 5    90.91%\n",
        )
        out_lcov = os.path.join(self.tmp, "merged.lcov")
        out_auth = os.path.join(self.tmp, "merged-auth.txt")
        res = subprocess.run(
            [sys.executable, MERGE_SCRIPT, a_lcov,
             "-o", out_lcov,
             "--auth-summary", a_rep,
             "--auth-summary", b_rep,
             "--auth-summary-out", out_auth,
             "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)

        # Re-parse the merged auth summary and confirm max/min rules.
        import lcov_io
        merged = lcov_io.parse_llvm_cov_report(out_auth)
        foo = merged.files["source/foo.c"]
        self.assertEqual(foo.line_total, 110)   # max
        self.assertEqual(foo.line_missed, 15)   # min
        self.assertEqual(foo.func_total, 12)
        self.assertEqual(foo.func_missed, 1)
        self.assertEqual(foo.branch_total, 55)
        self.assertEqual(foo.branch_missed, 5)
        self.assertEqual(merged.total.line_total, 110)
        self.assertEqual(merged.total.line_missed, 15)

    def test_auth_total_recomputed_from_files_no_filter(self):
        # Regression: when no filter is passed, the auth-summary
        # TOTAL row used to fall back to merge_auth_summaries' max/min
        # reduction, which can disagree with sum(merged_auth.files).
        # The merger now always recomputes TOTAL from the per-file
        # rows, which is the consistent answer for the merged output.
        a_lcov = self._write(
            "a.info",
            "TN:\nSF:source/foo.c\nDA:1,1\nLF:1\nLH:1\nend_of_record\n"
            "TN:\nSF:source/bar.c\nDA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        a_rep = self._report(
            "a-report.txt",
            [
                "source/foo.c"
                "                                                                                         "
                "      100                20    80.00%          10                 2    80.00%         "
                "100                20    80.00%          50                10    80.00%\n",
                "source/bar.c"
                "                                                                                         "
                "       50                 5    90.00%           4                 0   100.00%          "
                "50                 5    90.00%          20                 2    90.00%\n",
            ],
            "TOTAL                                                                                                "
            "      150                25    83.33%          14                 2    85.71%         "
            "150                25    83.33%          70                12    82.86%\n",
        )
        out_lcov = os.path.join(self.tmp, "merged.lcov")
        out_auth = os.path.join(self.tmp, "merged-auth.txt")
        res = subprocess.run(
            [sys.executable, MERGE_SCRIPT, a_lcov,
             "-o", out_lcov,
             "--auth-summary", a_rep,
             "--auth-summary-out", out_auth,
             "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        import lcov_io
        merged = lcov_io.parse_llvm_cov_report(out_auth)
        # TOTAL must equal sum of foo + bar (150 lines, 25 missed).
        self.assertEqual(merged.total.line_total, 150)
        self.assertEqual(merged.total.line_missed, 25)
        self.assertEqual(merged.total.func_total, 14)
        self.assertEqual(merged.total.func_missed, 2)
        self.assertEqual(merged.total.branch_total, 70)
        self.assertEqual(merged.total.branch_missed, 12)

    def test_auth_summary_gz_input(self):
        # Auth-summary inputs are auto-decompressed the same way LCOV
        # inputs are.
        import gzip
        a_lcov = self._write(
            "a.info",
            "TN:\nSF:source/foo.c\nDA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        report_text = (
            "Filename                                                                                             "
            "Regions    Missed Regions     Cover   Functions  Missed Functions  Executed       Lines      "
            "Missed Lines     Cover    Branches   Missed Branches     Cover\n"
            + "-" * 200 + "\n"
            "source/foo.c"
            "                                                                                         "
            "      100                20    80.00%          10                 2    80.00%         "
            "100                20    80.00%          50                10    80.00%\n"
            + "-" * 200 + "\n"
            "TOTAL                                                                                                "
            "      100                20    80.00%          10                 2    80.00%         "
            "100                20    80.00%          50                10    80.00%\n"
        )
        gz_path = os.path.join(self.tmp, "report.txt.gz")
        with gzip.open(gz_path, "wt", encoding="utf-8") as f:
            f.write(report_text)
        out_auth = os.path.join(self.tmp, "merged-auth.txt")
        res = subprocess.run(
            [sys.executable, MERGE_SCRIPT, a_lcov,
             "--auth-summary", gz_path,
             "--auth-summary-out", out_auth,
             "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        import lcov_io
        merged = lcov_io.parse_llvm_cov_report(out_auth)
        self.assertIn("source/foo.c", merged.files)
        self.assertEqual(merged.files["source/foo.c"].line_total, 100)

    def test_auth_summary_requires_out_path(self):
        a_lcov = self._write(
            "a.info",
            "TN:\nSF:foo.c\nDA:1,1\nend_of_record\n",
        )
        a_rep = self._report(
            "a-report.txt",
            [],
            "TOTAL                                                                                                "
            "        0                 0         -           0                 0         -           "
            "  0                 0         -           0                 0         -\n",
        )
        res = subprocess.run(
            [sys.executable, MERGE_SCRIPT, a_lcov,
             "--auth-summary", a_rep, "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 2)
        self.assertIn("--auth-summary-out is required", res.stderr)


class RendererAuthSummaryCliTests(unittest.TestCase):
    """End-to-end: rendered HTML reflects authoritative numbers."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        import shutil
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_overrides_show_in_index(self):
        lcov = os.path.join(self.tmp, "in.info")
        # LCOV says foo.c has 1 line, 1 hit.
        with open(lcov, "w") as f:
            f.write(
                "TN:\nSF:source/foo.c\nDA:1,1\n"
                "LF:1\nLH:1\nend_of_record\n"
            )
        # Auth summary says 200 lines, 50 missed (150 hit, 75.0%).
        rep = os.path.join(self.tmp, "report.txt")
        with open(rep, "w") as f:
            f.write(
                "Filename                                                                                             "
                "Regions    Missed Regions     Cover   Functions  Missed Functions  Executed       Lines      "
                "Missed Lines     Cover    Branches   Missed Branches     Cover\n"
            )
            f.write("-" * 200 + "\n")
            f.write(
                "source/foo.c                                                                                         "
                "      200                50    75.00%           5                 1    80.00%         "
                "200                50    75.00%          40                10    75.00%\n"
            )
            f.write("-" * 200 + "\n")
            f.write(
                "TOTAL                                                                                                "
                "      200                50    75.00%           5                 1    80.00%         "
                "200                50    75.00%          40                10    75.00%\n"
            )

        out_dir = os.path.join(self.tmp, "out")
        res = subprocess.run(
            [sys.executable, RENDER_SCRIPT, lcov,
             "--output-dir", out_dir,
             "--auth-summary", rep,
             "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)

        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            html = f.read()
        # Expected per-file row totals come from the auth summary, not
        # from LCOV's "1 line, 1 hit".
        self.assertIn(">200<", html)  # line total
        self.assertIn(">150<", html)  # line hit
        # LCOV's pre-auth 1/1 values must not leak into any rendered cell.
        body = html.split("<tbody>", 1)[1].split("</tbody>", 1)[0]
        self.assertNotRegex(body, r">\s*1\s*</(td|span)>")


if __name__ == "__main__":
    unittest.main(verbosity=2)
