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
    def test_default_prefixes_strip_clean(self):
        f = merger.normalize_path
        prefixes = merger.DEFAULT_STRIP_PREFIXES
        self.assertEqual(
            f("/__w/slang/slang/source/slang/x.cpp", prefixes),
            "source/slang/x.cpp",
        )
        self.assertEqual(
            f("/Users/runner/work/slang/slang/source/slang/x.cpp", prefixes),
            "source/slang/x.cpp",
        )

    def test_windows_backslashes_normalize_to_forward(self):
        f = merger.normalize_path
        prefixes = merger.DEFAULT_STRIP_PREFIXES
        self.assertEqual(
            f(r"D:\a\slang\slang\source\slang\x.cpp", prefixes),
            "source/slang/x.cpp",
        )

    def test_unmatched_path_passes_through(self):
        f = merger.normalize_path
        self.assertEqual(
            f("/some/unrelated/path/file.c", merger.DEFAULT_STRIP_PREFIXES),
            "/some/unrelated/path/file.c",
        )

    def test_extra_prefix(self):
        f = merger.normalize_path
        # User-supplied extras should also strip.
        self.assertEqual(
            f("/custom/repo/foo.c", ("/custom/repo/",)),
            "foo.c",
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
            "TN:\nSF:/__w/slang/slang/source/foo.c\n"
            "DA:1,1\nLF:1\nLH:1\nend_of_record\n",
        )
        b = self._write(
            "macos.info",
            "TN:\nSF:/Users/runner/work/slang/slang/source/foo.c\n"
            "DA:2,1\nLF:1\nLH:1\nend_of_record\n",
        )
        c = self._write(
            "windows.info",
            "TN:\nSF:D:\\a\\slang\\slang\\source\\foo.c\n"
            "DA:3,1\nLF:1\nLH:1\nend_of_record\n",
        )
        res = self._run(a, b, c, "--quiet")
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


if __name__ == "__main__":
    unittest.main(verbosity=2)
