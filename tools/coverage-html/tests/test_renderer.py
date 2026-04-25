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
        # The unknown FUTURETAG is ignored; BRDA / FN / FNDA are parsed.
        self.assertEqual(len(records), 1)
        self.assertEqual(records[0].lines, {1: 1, 2: 0})
        # The fixture also carries one function + two branch entries.
        self.assertEqual(records[0].total_functions, 1)
        self.assertEqual(records[0].hit_functions, 1)
        self.assertEqual(records[0].total_branches, 2)
        self.assertEqual(records[0].hit_branches, 1)

    def test_branch_and_function_records_parse(self):
        records = renderer.parse_lcov(
            os.path.join(FIXTURES, "branches-and-functions.info")
        )
        self.assertEqual(len(records), 1)
        r = records[0]
        # BRDA parsing: taken '-' maps to None; integers map to int.
        self.assertEqual(r.branches[(15, 0, 0)], 2)
        self.assertEqual(r.branches[(15, 0, 1)], 1)
        self.assertIsNone(r.branches[(16, 0, 0)])
        self.assertIsNone(r.branches[(16, 0, 1)])
        self.assertEqual(r.total_branches, 4)
        self.assertEqual(r.hit_branches, 2)
        # FN / FNDA join on function name.
        self.assertEqual(set(r.functions.keys()), {"_Z3addii", "_Z3subii", "_Z6divideii"})
        self.assertEqual(r.functions["_Z3addii"].first_line, 4)
        self.assertEqual(r.functions["_Z3addii"].hits, 100)
        self.assertEqual(r.functions["_Z3subii"].hits, 0)
        self.assertEqual(r.total_functions, 3)
        self.assertEqual(r.hit_functions, 2)

    def test_corrupt_brda_raises(self):
        import tempfile

        with tempfile.NamedTemporaryFile(
            "w", suffix=".info", delete=False
        ) as tmp:
            tmp.write("TN:test\nSF:x.c\nBRDA:5,0,0,not-an-int\nend_of_record\n")
            path = tmp.name
        try:
            with self.assertRaises(renderer.LcovParseError) as ctx:
                renderer.parse_lcov(path)
            self.assertIn("BRDA", str(ctx.exception))
        finally:
            os.remove(path)

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
        self.assertEqual(r.percent_branches, 0.0)
        self.assertEqual(r.percent_functions, 0.0)

    def test_branch_counts(self):
        r = renderer.FileRecord(
            path="x",
            branches={
                (5, 0, 0): 3,       # taken
                (5, 0, 1): 0,       # evaluated but never taken
                (6, 0, 0): None,    # not evaluated at all
                (6, 0, 1): None,
            },
        )
        self.assertEqual(r.total_branches, 4)
        self.assertEqual(r.hit_branches, 1)
        self.assertEqual(r.percent_branches, 25.0)

    def test_function_counts(self):
        r = renderer.FileRecord(
            path="x",
            functions={
                "foo": renderer.Function(first_line=1, hits=10),
                "bar": renderer.Function(first_line=5, hits=0),
                "baz": renderer.Function(first_line=9, hits=3),
            },
        )
        self.assertEqual(r.total_functions, 3)
        self.assertEqual(r.hit_functions, 2)
        self.assertAlmostEqual(r.percent_functions, 2 / 3 * 100)


class BranchCellTests(unittest.TestCase):
    """Unit tests for the phase-2b per-line branch cell."""

    def test_empty_is_plain_spaces(self):
        cell = renderer._render_branch_cell([])
        self.assertEqual(cell, " " * renderer.BRANCH_COL_WIDTH)
        self.assertNotIn("<span", cell)

    def test_all_taken_is_branchAll(self):
        cell = renderer._render_branch_cell([5, 2, 17])
        self.assertIn('class="branchAll"', cell)
        self.assertIn("(3/3)", cell)
        # The tooltip should carry per-branch details.
        self.assertIn("br0: 5", cell)
        self.assertIn("br2: 17", cell)
        # Total visual width must equal BRANCH_COL_WIDTH.
        visible = _strip_tags(cell)
        self.assertEqual(len(visible), renderer.BRANCH_COL_WIDTH)

    def test_partial_is_branchPart(self):
        cell = renderer._render_branch_cell([3, 0, None])
        self.assertIn('class="branchPart"', cell)
        self.assertIn("(1/3)", cell)
        self.assertIn("br2: -", cell)

    def test_none_taken_is_branchNone(self):
        cell = renderer._render_branch_cell([0, 0])
        self.assertIn('class="branchNone"', cell)
        self.assertIn("(0/2)", cell)

    def test_not_evaluated_is_branchNone(self):
        # All "-" (not evaluated) counts as none-taken.
        cell = renderer._render_branch_cell([None, None])
        self.assertIn('class="branchNone"', cell)
        self.assertIn("(0/2)", cell)


def _strip_tags(s: str) -> str:
    import re

    return re.sub(r"<[^>]+>", "", s)


class SlangcFilterTests(unittest.TestCase):
    """Mirrors tools/coverage/slangc-ignore-patterns.sh."""

    def test_external_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(is_slangc_filtered_out("external/cmark/src/blocks.c"))

    def test_build_prelude_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(
            is_slangc_filtered_out("build/prelude/slang-cpp-prelude.h.cpp")
        )

    def test_build_fiddle_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(
            is_slangc_filtered_out(
                "build/source/slang/fiddle/slang-rich-diagnostics.h.fiddle"
            )
        )

    def test_tools_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(is_slangc_filtered_out("tools/slang-test/main.cpp"))

    def test_language_server_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(
            is_slangc_filtered_out(
                "source/slang/slang-language-server.cpp"
            )
        )
        self.assertTrue(
            is_slangc_filtered_out(
                "source/slang/slang-language-server-protocol.h"
            )
        )

    def test_ast_decl_headers_excluded(self):
        from lcov_io import is_slangc_filtered_out

        self.assertTrue(
            is_slangc_filtered_out("source/slang/slang-ast-expr.h")
        )
        self.assertTrue(
            is_slangc_filtered_out("source/slang/slang-ast-modifier.h")
        )
        # But the .cpp counterpart is kept.
        self.assertFalse(
            is_slangc_filtered_out("source/slang/slang-ast-modifier.cpp")
        )

    def test_compiler_files_kept(self):
        from lcov_io import is_slangc_filtered_out

        self.assertFalse(is_slangc_filtered_out("source/slang/slang-check.cpp"))
        self.assertFalse(
            is_slangc_filtered_out("source/compiler-core/slang-name.cpp")
        )
        self.assertFalse(
            is_slangc_filtered_out("source/core/slang-string.cpp")
        )

    def test_backslash_paths_normalized(self):
        from lcov_io import is_slangc_filtered_out

        # Pre-normalization Windows paths still match.
        self.assertTrue(
            is_slangc_filtered_out(r"external\cmark\src\blocks.c")
        )

    def test_apply_slangc_filter_keeps_compiler_only(self):
        from lcov_io import FileRecord, apply_slangc_filter

        records = [
            FileRecord(path="source/slang/slang-check.cpp"),
            FileRecord(path="external/cmark/src/blocks.c"),
            FileRecord(path="tools/slang-test/main.cpp"),
            FileRecord(path="source/compiler-core/slang-name.cpp"),
        ]
        kept = apply_slangc_filter(records)
        kept_paths = {r.path for r in kept}
        self.assertEqual(
            kept_paths,
            {"source/slang/slang-check.cpp", "source/compiler-core/slang-name.cpp"},
        )


class FunctionDedupByLineTests(unittest.TestCase):
    """Multiple FN records at the same first_line are template
    instantiations / compiler-generated duplicates of one source-level
    function. total_functions / hit_functions dedupe by first_line so
    the published rate matches `llvm-cov report`."""

    def _r(self, fns):
        from lcov_io import FileRecord, Function

        r = FileRecord(path="x.cpp")
        for name, (fl, h) in fns.items():
            r.functions[name] = Function(first_line=fl, hits=h)
        return r

    def test_three_instantiations_at_same_line_count_once(self):
        # `foo<int>`, `foo<float>`, `foo<bool>` all declared at line 10.
        r = self._r({"foo_int": (10, 5), "foo_float": (10, 0), "foo_bool": (10, 0)})
        self.assertEqual(r.total_functions, 1)
        self.assertEqual(r.hit_functions, 1)  # any instantiation hit → counted
        self.assertEqual(r.percent_functions, 100.0)

    def test_distinct_first_lines_count_separately(self):
        r = self._r({"foo": (10, 5), "bar": (20, 0), "baz": (30, 7)})
        self.assertEqual(r.total_functions, 3)
        self.assertEqual(r.hit_functions, 2)

    def test_orphan_first_line_zero_keeps_per_name(self):
        # FNDA-without-FN: first_line=0. Can't dedupe by line; each
        # mangled name counts on its own.
        r = self._r({"orphan_a": (0, 1), "orphan_b": (0, 0), "real": (5, 1)})
        self.assertEqual(r.total_functions, 3)
        self.assertEqual(r.hit_functions, 2)


class FunctionLineCoverageTests(unittest.TestCase):
    """Per-function line coverage derived from FN: + DA: ranges."""

    def _build(self, lines, fns):
        from lcov_io import FileRecord, Function

        r = FileRecord(path="x.c")
        r.lines = dict(lines)
        r.functions = {n: Function(first_line=fl, hits=h) for n, (fl, h) in fns.items()}
        return r

    def test_simple_two_function_split(self):
        from lcov_io import function_line_coverage

        # foo: lines 1-4 (foo declared at 1; bar at 5)
        # bar: lines 5+
        r = self._build(
            lines={1: 1, 2: 1, 3: 0, 4: 1, 5: 1, 6: 0},
            fns={"foo": (1, 7), "bar": (5, 1)},
        )
        cov = function_line_coverage(r)
        self.assertEqual(cov["foo"], (4, 3))
        self.assertEqual(cov["bar"], (2, 1))

    def test_function_without_first_line_is_zero(self):
        from lcov_io import function_line_coverage

        r = self._build(
            lines={1: 1, 2: 0},
            fns={"orphan": (0, 5)},  # FNDA without FN
        )
        self.assertEqual(function_line_coverage(r)["orphan"], (0, 0))

    def test_last_function_captures_all_remaining(self):
        from lcov_io import function_line_coverage

        r = self._build(
            lines={10: 1, 100: 1, 1000: 0},
            fns={"only_one": (5, 1)},
        )
        self.assertEqual(function_line_coverage(r)["only_one"], (3, 2))


class GradientColorTests(unittest.TestCase):
    def test_endpoints(self):
        # 0% → red (hue 0), 100% → green (hue 120).
        self.assertIn("hsl(0,", renderer._gradient_color(0.0))
        self.assertIn("hsl(120,", renderer._gradient_color(100.0))

    def test_midpoint_is_yellow(self):
        # 50% → hue 60 (yellow).
        self.assertIn("hsl(60,", renderer._gradient_color(50.0))

    def test_clamps(self):
        # Out-of-range inputs clamp to [0, 100].
        self.assertIn("hsl(0,", renderer._gradient_color(-5.0))
        self.assertIn("hsl(120,", renderer._gradient_color(200.0))


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

    def test_real_lcov_with_branches_and_functions(self):
        """Render the trimmed real-world fixture and assert phase-2 output."""
        out_dir = os.path.join(self.tmp, "real")
        res = self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # New column groups appear only when data is present.
        self.assertIn("Function Coverage", idx)
        self.assertIn("Branch Coverage", idx)
        # Header summary rows include Functions and Branches lines.
        self.assertIn("Functions:", idx)
        self.assertIn("Branches:", idx)

    def test_directory_grouping_in_index(self):
        """Files are grouped by their full parent directory and the
        index emits one `dirHeader` row per directory in the
        hierarchy — each level (e.g. `source/`, `source/compiler-core/`,
        `source/slang/`) is independently collapsible."""
        out_dir = os.path.join(self.tmp, "dirs")
        # The slangc-llvm-cov sample's three files live under
        # source/compiler-core/, source/core/, source/slang/. After
        # the common-prefix strip those become compiler-core/, core/,
        # slang/, so we get a "(top level)" header at the root plus
        # one per immediate subdir.
        self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        self.assertIn('class="dirHeader"', idx)
        self.assertIn('class="dirToggle"', idx)
        self.assertRegex(idx, r"\(\d+ files?\)")
        # Each dirHeader carries data-path and data-depth so the
        # toggle JS can identify descendants and immediate children.
        self.assertIn('data-path="', idx)
        self.assertIn('data-depth="', idx)
        # Every fileSummary carries data-dir referencing its parent
        # directory's full path (not a sanitized hash).
        self.assertRegex(idx, r'class="fileSummary" data-dir="')
        # The hierarchy includes both top-level and nested dirs.
        self.assertIn('data-path=""', idx)  # root

    def test_nested_directory_levels_each_collapsible(self):
        """Deeply-nested paths produce a dirHeader at every level
        (parent + each child), so a tree like
            external/cmark/src/foo.c
        gives us four headers: '', 'external', 'external/cmark',
        'external/cmark/src'."""
        # Build a tiny synthetic LCOV with one file deep in a tree.
        deep = os.path.join(self.tmp, "deep.info")
        with open(deep, "w") as f:
            f.write(
                "TN:\nSF:external/cmark/src/blocks.c\n"
                "DA:1,1\nLF:1\nLH:1\nend_of_record\n"
            )
        out_dir = os.path.join(self.tmp, "deep-out")
        self._run(deep, "--output-dir", out_dir, "--quiet")
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # Each ancestor has its own dirHeader.
        # Note: slang-coverage-html strips the common prefix; with
        # a single-file LCOV that prefix is the file's full directory,
        # so the hierarchy collapses to "(top level)" + just the file.
        # To validate nesting we add a second file outside the deep
        # tree so the common prefix stays at the root.
        deep2 = os.path.join(self.tmp, "deep2.info")
        with open(deep2, "w") as f:
            f.write(
                "TN:\nSF:external/cmark/src/blocks.c\n"
                "DA:1,1\nLF:1\nLH:1\nend_of_record\n"
                "TN:\nSF:other.c\n"
                "DA:1,1\nLF:1\nLH:1\nend_of_record\n"
            )
        out_dir2 = os.path.join(self.tmp, "deep2-out")
        self._run(deep2, "--output-dir", out_dir2, "--quiet")
        with open(os.path.join(out_dir2, "index.html"), encoding="utf-8") as f:
            idx2 = f.read()
        self.assertIn('data-path=""', idx2)
        self.assertIn('data-path="external"', idx2)
        self.assertIn('data-path="external/cmark"', idx2)
        self.assertIn('data-path="external/cmark/src"', idx2)
        # The deepest header has data-depth="3".
        self.assertRegex(
            idx2,
            r'data-path="external/cmark/src"[^>]*data-depth="3"|'
            r'data-depth="3"[^>]*data-path="external/cmark/src"',
        )

    def test_index_uses_full_width_no_center(self):
        """Index table is full-width, not the old `<center>` 80%."""
        out_dir = os.path.join(self.tmp, "fullwidth")
        self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # No <center> wrapper around the index table anymore.
        self.assertNotIn("<center>", idx)
        # indexTable uses 100% width.
        self.assertIn("table.indexTable", idx)
        self.assertIn("width: 100%", idx)

    def test_title_no_lcov_branding(self):
        """LCOV branding stripped from the page title."""
        out_dir = os.path.join(self.tmp, "title")
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # The visible page title is no longer "LCOV - code coverage report".
        self.assertNotIn("LCOV - code coverage report", idx)
        # Default title is "Coverage report".
        self.assertIn(">Coverage report</td>", idx)

    def test_slang_brand_palette_applied(self):
        """CSS uses the Slang teal / orange tokens, not the old genhtml
        blue/red bar colors."""
        out_dir = os.path.join(self.tmp, "palette")
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # Slang teal & orange present, old hard-coded blue gone.
        self.assertIn("#105f65", idx)
        self.assertIn("#F14D1B", idx)
        self.assertNotIn("#6688d4", idx)  # old genhtml ruler/header blue

    def test_chevron_placeholder_keeps_columns_aligned(self):
        """Files without function records still get a same-width
        invisible chevron placeholder so the file-name column lines
        up across rows in mixed datasets."""
        out_dir = os.path.join(self.tmp, "placeholder")
        # mixed-paths.info has two files, neither with functions.
        # branches-and-functions.info has one file with functions.
        # We want a fixture mixing both shapes — easiest: use the
        # llvm-cov sample (functions on some files, not all).
        self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        # Both visible chevron and placeholder should be present in the
        # same index when at least one file has functions and another
        # doesn't (or, at minimum: placeholder is rendered for the
        # always-applicable case when functions are absent globally).
        # In this fixture all three files have functions, so we just
        # assert the visible chevron is present.
        self.assertIn('class="fnToggle"', idx)

        # And the demo fixture (no functions anywhere) renders
        # placeholder rows so other CSS columns stay aligned.
        out_dir2 = os.path.join(self.tmp, "no-fn")
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir2,
            "--quiet",
        )
        with open(os.path.join(out_dir2, "index.html"), encoding="utf-8") as f:
            idx2 = f.read()
        self.assertIn('class="fnTogglePlaceholder"', idx2)
        self.assertNotIn('class="fnToggle"', idx2)

    def test_inline_function_table_fixed_width_layout(self):
        """fnInner must have table-layout: fixed + colgroup so long
        mangled function names wrap inside the cell rather than
        blowing out the row width."""
        out_dir = os.path.join(self.tmp, "fixed-layout")
        self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        self.assertIn('table-layout: fixed', idx)
        self.assertIn('<colgroup>', idx)
        # fnInner subdivides the parent's File column for Name/Line/Calls,
        # then reuses the parent's colgroup classes for Bar/Rate/Total/Hit
        # so the per-function cells line up with the file row above.
        self.assertIn('class="fnNameCol"', idx)
        self.assertIn('class="fnLineCol"', idx)
        self.assertIn('class="fnCallsCol"', idx)
        self.assertIn('class="colLBar"', idx)
        self.assertIn('class="colLRate"', idx)
        # Long mangled names should wrap via word-break / overflow-wrap.
        self.assertIn('word-break: break-all', idx)

    def test_invocation_cwd_resolves_repo_relative_paths(self):
        """A merged LCOV holds repo-relative SF: paths like
        `tools/coverage-html/slang-coverage-html.py`. Running the
        renderer from the repo root must find them via invocation_cwd
        even when the LCOV file itself sits in /tmp."""
        # Build a tiny LCOV in /tmp pointing at a file that exists in
        # the repo, and assert it gets resolved.
        repo_root = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
        rel_target = "tools/coverage-html/README.md"
        self.assertTrue(os.path.exists(os.path.join(repo_root, rel_target)))

        lcov_path = os.path.join(self.tmp, "merged-style.info")
        with open(lcov_path, "w") as f:
            f.write(f"TN:\nSF:{rel_target}\nDA:1,1\nLF:1\nLH:1\nend_of_record\n")

        out_dir = os.path.join(self.tmp, "invocation-cwd")
        # Run with cwd = repo_root. Without --source-root.
        env = dict(os.environ)
        res = subprocess.run(
            [sys.executable, SCRIPT, lcov_path,
             "--output-dir", out_dir, "--quiet"],
            capture_output=True,
            text=True,
            env=env,
            cwd=repo_root,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # Find the per-file page.
        pages = [
            os.path.join(out_dir, e)
            for e in os.listdir(out_dir)
            if e.endswith(".html") and e != "index.html"
        ]
        self.assertEqual(len(pages), 1)
        with open(pages[0], encoding="utf-8") as f:
            content = f.read()
        # Source view (not placeholder) is present when the file is
        # found.
        self.assertNotIn("Source file not found", content)
        self.assertIn('class="lineNum"', content)

    def test_index_has_expandable_function_rows(self):
        """Goal 2: per-file Functions tables live inline in the index,
        wrapped in a hidden <tr class="fileFunctions"> revealed by an
        fnToggle chevron in the file row."""
        out_dir = os.path.join(self.tmp, "expand")
        res = self._run(
            os.path.join(FIXTURES, "branches-and-functions.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        self.assertIn('class="fnToggle"', idx)
        self.assertIn('aria-expanded="false"', idx)
        # fileFunctions row carries the data-dir attribute (so it can
        # be collapsed/expanded as part of its parent directory) and
        # starts hidden.
        self.assertRegex(idx, r'class="fileFunctions"[^>]*\bhidden\b')
        self.assertIn('<table class="fnInner"', idx)
        self.assertIn("addEventListener('click'", idx)
        self.assertIn("_Z3addii", idx)
        # Line column links to per-file anchor.
        self.assertIn("#L4", idx)
        # New expectations: directory header and dir toggle.
        self.assertIn('class="dirToggle"', idx)
        self.assertIn('class="dirHeader"', idx)
        self.assertIn('class="coverDirectory"', idx)
        # Per-function line-coverage cells (Bar + Rate + Total + Hit)
        # appear in each row; tier classes carry through.
        self.assertIn('Line Coverage', idx)
        self.assertIn('class="coverPer', idx)
        # Bar fills use the gradient-color inline style, not the old
        # tier classes.
        self.assertIn('background-color:hsl(', idx)
        self.assertNotIn('coverBarFillHi', idx)
        self.assertNotIn('coverBarFillMed', idx)
        self.assertNotIn('coverBarFillLo', idx)

    def test_per_file_page_no_longer_has_functions_table(self):
        """Goal 2 partner: per-file pages drop the Functions table
        (it moved into the index expansion)."""
        out_dir = os.path.join(self.tmp, "no-fn-table")
        self._run(
            os.path.join(FIXTURES, "branches-and-functions.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        per_file = [
            os.path.join(out_dir, e)
            for e in os.listdir(out_dir)
            if e.endswith(".html") and e != "index.html"
        ]
        self.assertTrue(per_file)
        for p in per_file:
            with open(p, encoding="utf-8") as f:
                text = f.read()
            # "Hit count" header is unique to the function table; its
            # absence confirms the table is gone.
            self.assertNotIn(
                '<td class="tableHead">Hit count</td>',
                text,
                msg=f"per-file page {p} still has Hit-count header",
            )
            # The header summary row for Functions remains.
            self.assertIn("Functions:", text)

    def test_real_lcov_renders_inline_branch_column(self):
        """Phase 2b: per-file source view shows the branch gutter."""
        out_dir = os.path.join(self.tmp, "real-2b")
        # Resolve sources from the repo root so the source view isn't a
        # placeholder.
        repo_root = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
        res = self._run(
            os.path.join(FIXTURES, "slangc-llvm-cov-sample.info"),
            "--output-dir",
            out_dir,
            "--source-root",
            repo_root,
            "--quiet",
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)

        # Find a per-file .html from the output and grep for branch spans.
        per_file_pages = [
            os.path.join(out_dir, e)
            for e in os.listdir(out_dir)
            if e.endswith(".html") and e != "index.html"
        ]
        self.assertTrue(per_file_pages, "expected at least one per-file page")

        found_all = False
        found_part = False
        found_none = False
        found_heading = False
        for p in per_file_pages:
            with open(p, encoding="utf-8") as f:
                text = f.read()
            if 'class="branchAll"' in text:
                found_all = True
            if 'class="branchPart"' in text:
                found_part = True
            if 'class="branchNone"' in text:
                found_none = True
            if "Branch" in text and "Source code" in text:
                found_heading = True

        # The slang-name.cpp sample carries all three tiers.
        self.assertTrue(found_all, "expected a branchAll span somewhere")
        self.assertTrue(found_part, "expected a branchPart span somewhere")
        self.assertTrue(found_none, "expected a branchNone span somewhere")
        self.assertTrue(found_heading, "expected 'Branch' column heading")

    def test_phase1_fixture_has_no_extra_columns(self):
        """Regression: the demo fixture (no branches / functions)
        must not grow extra columns when phase-2 code is present."""
        out_dir = os.path.join(self.tmp, "phase1")
        self._run(
            os.path.join(FIXTURES, "demo-cpu.info"),
            "--output-dir",
            out_dir,
            "--quiet",
        )
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            idx = f.read()
        self.assertNotIn("Function Coverage", idx)
        self.assertNotIn("Branch Coverage", idx)

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
