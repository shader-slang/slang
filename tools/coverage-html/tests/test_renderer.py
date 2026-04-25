#!/usr/bin/env python3
"""
Unit + integration tests for slang-coverage-html.

Runs with stdlib unittest; no pip deps. From the repo root:

    python3 -m unittest discover -s tools/coverage-html/tests -v

Or directly:

    python3 tools/coverage-html/tests/test_renderer.py
"""

import importlib.util
import os
import shutil
import subprocess
import sys
import tempfile
import unittest


HERE = os.path.dirname(os.path.abspath(__file__))
FIXTURES = os.path.join(HERE, "fixtures")
SCRIPT = os.path.abspath(os.path.join(HERE, os.pardir, "slang-coverage-html.py"))


def _import_renderer():
    """Import the hyphenated script file as a module for unit tests."""
    spec = importlib.util.spec_from_file_location("slang_coverage_html", SCRIPT)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


renderer = _import_renderer()


class ParserTests(unittest.TestCase):
    def test_empty_file(self):
        records = renderer.parse_lcov(os.path.join(FIXTURES, "empty.info"))
        self.assertEqual(records, [])

    def test_mixed_paths_preserved(self):
        records = renderer.parse_lcov(os.path.join(FIXTURES, "mixed-paths.info"))
        paths = [r.path for r in records]
        self.assertIn("/absolute/path/to/foo.c", paths)
        self.assertIn("relative/path/bar.c", paths)

    def test_tn_groups_aggregate_as_max(self):
        records = renderer.parse_lcov(os.path.join(FIXTURES, "tn-groups.info"))
        self.assertEqual(len(records), 1)
        shared = records[0]
        # Two TN: blocks for shared.c:
        #   alpha: L1=5, L2=0, L3=0
        #   beta : L1=2, L2=4, L3=0
        # max aggregate: L1=5, L2=4, L3=0
        self.assertEqual(shared.lines, {1: 5, 2: 4, 3: 0})
        self.assertEqual(shared.hit_lines, 2)
        self.assertEqual(shared.total_lines, 3)

    def test_unknown_records_are_tolerated(self):
        records = renderer.parse_lcov(
            os.path.join(FIXTURES, "unknown-records.info")
        )
        # The unknown FUTURETAG and the BRDA / FN / FNDA records should be
        # parsed and ignored, leaving only the DA-derived data.
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0].lines, {1: 1, 2: 0})

    def test_corrupt_raises_with_line_number(self):
        with self.assertRaises(renderer.LcovParseError) as ctx:
            renderer.parse_lcov(os.path.join(FIXTURES, "corrupt-bad-da.info"))
        self.assertIn(":3", str(ctx.exception))
        self.assertIn("non-integer DA", str(ctx.exception))

    def test_demo_lcov_has_expected_totals(self):
        records = renderer.parse_lcov(os.path.join(FIXTURES, "demo-cpu.info"))
        self.assertEqual(len(records), 2)
        total = sum(r.total_lines for r in records)
        hit = sum(r.hit_lines for r in records)
        # Matches the demo's documented ~84.6% headline.
        self.assertEqual(total, 26)
        self.assertEqual(hit, 22)


class FileRecordTests(unittest.TestCase):
    def test_percent_rounds_correctly(self):
        r = renderer.FileRecord(path="x", lines={1: 1, 2: 1, 3: 0, 4: 0})
        self.assertEqual(r.total_lines, 4)
        self.assertEqual(r.hit_lines, 2)
        self.assertAlmostEqual(r.percent, 50.0)

    def test_empty_record_is_zero_percent(self):
        r = renderer.FileRecord(path="x")
        self.assertEqual(r.percent, 0.0)


class TierTests(unittest.TestCase):
    def test_thresholds(self):
        self.assertEqual(renderer._tier(100.0), "Hi")
        self.assertEqual(renderer._tier(90.0), "Hi")
        self.assertEqual(renderer._tier(89.9), "Med")
        self.assertEqual(renderer._tier(75.0), "Med")
        self.assertEqual(renderer._tier(74.9), "Lo")
        self.assertEqual(renderer._tier(0.0), "Lo")


class ResolverTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.mkdtemp()
        self.src_root = os.path.join(self.tmp, "src")
        os.makedirs(self.src_root)
        with open(os.path.join(self.src_root, "foo.c"), "w") as f:
            f.write("line1\nline2\n")

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_resolve_via_source_root(self):
        r = renderer.SourceResolver(source_root=self.src_root, cwd=self.tmp)
        text, resolved = r.load("foo.c")
        self.assertEqual(text, "line1\nline2\n")
        self.assertTrue(resolved.endswith("foo.c"))

    def test_basename_fallback(self):
        # Simulate an LCOV SF: path with an unfamiliar prefix.
        r = renderer.SourceResolver(source_root=self.src_root, cwd=self.tmp)
        text, _ = r.load("some/elsewhere/foo.c")
        self.assertEqual(text, "line1\nline2\n")

    def test_miss_returns_none(self):
        r = renderer.SourceResolver(source_root=self.src_root, cwd=self.tmp)
        text, resolved = r.load("does-not-exist.c")
        self.assertIsNone(text)
        self.assertIsNone(resolved)

    def test_cache_hits(self):
        r = renderer.SourceResolver(source_root=self.src_root, cwd=self.tmp)
        r.load("foo.c")
        # Remove the file to prove the second call is served from cache.
        os.remove(os.path.join(self.src_root, "foo.c"))
        text, _ = r.load("foo.c")
        self.assertEqual(text, "line1\nline2\n")


class FilterTests(unittest.TestCase):
    def _records(self):
        return [
            renderer.FileRecord(path="src/a/foo.c", lines={1: 1}),
            renderer.FileRecord(path="src/b/bar.c", lines={1: 0}),
            renderer.FileRecord(path="test/t.c", lines={1: 1}),
        ]

    def test_include(self):
        out = renderer.apply_filters(self._records(), ["src/*"], [])
        self.assertEqual([r.path for r in out], ["src/a/foo.c", "src/b/bar.c"])

    def test_exclude(self):
        out = renderer.apply_filters(self._records(), [], ["*test*"])
        self.assertEqual([r.path for r in out], ["src/a/foo.c", "src/b/bar.c"])

    def test_no_filters_is_identity(self):
        recs = self._records()
        self.assertEqual(renderer.apply_filters(recs, [], []), recs)


class PathHelperTests(unittest.TestCase):
    def test_common_dir_prefix(self):
        paths = [
            "../../examples/shader-coverage-demo/physics.slang",
            "../../examples/shader-coverage-demo/simulate.slang",
        ]
        self.assertEqual(
            renderer._common_dir_prefix(paths),
            "../../examples/shader-coverage-demo/",
        )

    def test_no_common_prefix(self):
        paths = ["/abs/foo.c", "relative/bar.c"]
        self.assertEqual(renderer._common_dir_prefix(paths), "")

    def test_single_path(self):
        self.assertEqual(
            renderer._common_dir_prefix(["only/one/file.c"]),
            "only/one/",
        )

    def test_empty(self):
        self.assertEqual(renderer._common_dir_prefix([]), "")


class CliIntegrationTests(unittest.TestCase):
    """End-to-end: invoke the CLI and inspect the generated HTML."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _run(self, *args):
        env = dict(os.environ)
        return subprocess.run(
            [sys.executable, SCRIPT, *args],
            capture_output=True,
            text=True,
            env=env,
        )

    def test_demo_fixture_round_trip(self):
        out_dir = os.path.join(self.tmp, "out")
        res = self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)

        index_path = os.path.join(out_dir, "index.html")
        self.assertTrue(os.path.exists(index_path))
        with open(index_path, encoding="utf-8") as f:
            idx = f.read()

        # Overall coverage rate for the demo is 84.6%.
        self.assertIn("84.6&nbsp;%", idx)
        # Both source files listed.
        self.assertIn("physics.slang", idx)
        self.assertIn("simulate.slang", idx)
        # Two per-file pages + index + marker were written.
        entries = os.listdir(out_dir)
        html_files = [e for e in entries if e.endswith(".html")]
        self.assertEqual(len(html_files), 3)

    def test_refuses_foreign_output_dir(self):
        foreign = os.path.join(self.tmp, "foreign")
        os.makedirs(foreign)
        with open(os.path.join(foreign, "keep-me.txt"), "w") as f:
            f.write("user work")

        res = self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            foreign,
        )
        self.assertNotEqual(res.returncode, 0)
        self.assertIn("non-empty", res.stderr)
        # User file preserved.
        self.assertTrue(os.path.exists(os.path.join(foreign, "keep-me.txt")))

    def test_exit_nonzero_on_corrupt_lcov(self):
        res = self._run(
            os.path.join(FIXTURES, "corrupt-bad-da.info"),
            "--output-dir",
            os.path.join(self.tmp, "corrupt-out"),
        )
        self.assertEqual(res.returncode, 2)
        self.assertIn("non-integer DA", res.stderr)

    def test_empty_lcov_produces_empty_index(self):
        out_dir = os.path.join(self.tmp, "empty-out")
        res = self._run(
            os.path.join(FIXTURES, "empty.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            self.assertIn("No coverage data found", f.read())

    def test_idempotent_modulo_timestamp(self):
        """Two back-to-back runs differ only in the Date: row."""
        out_dir = os.path.join(self.tmp, "idempotent")
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            first = f.read()
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            second = f.read()

        # Strip the Date: row (the only intentionally non-deterministic part).
        import re as _re

        def strip_date(s: str) -> str:
            return _re.sub(
                r'<td class="headerValue">\d{4}-\d{2}-\d{2}[^<]+</td>',
                '<td class="headerValue">__DATE__</td>',
                s,
            )

        self.assertEqual(strip_date(first), strip_date(second))


if __name__ == "__main__":
    unittest.main(verbosity=2)
