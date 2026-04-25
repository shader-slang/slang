#!/usr/bin/env python3
"""
Tests for the lcov_io module — parser, data model, source resolver,
function-coverage helpers (line / branch partitioning, effective hit,
first-line dedup), and the slangc filter.

Runs with stdlib unittest; no pip deps. From the repo root:

    python3 -m unittest discover -s tools/coverage-html/tests -v

Or directly:

    python3 tools/coverage-html/tests/test_renderer.py
"""

import os
import shutil
import sys
import tempfile
import unittest


HERE = os.path.dirname(os.path.abspath(__file__))
FIXTURES = os.path.join(HERE, "fixtures")
sys.path.insert(0, os.path.abspath(os.path.join(HERE, os.pardir)))

import lcov_io  # noqa: E402

# The moved test bodies were originally written against `renderer.…`
# (when the parser, FileRecord, SourceResolver, etc. lived in the
# renderer module). Alias here so test code keeps reading naturally.
renderer = lcov_io




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




class EffectiveFunctionHitTests(unittest.TestCase):
    """A function is "effectively hit" if FNDA reports calls OR any
    DA line in its source range was hit. The latter half handles the
    inlined-only callee case (RAII helpers in headers): FNDA stays
    0 because no separate function-entry counter fires, but the body
    code ran via inlining and the DA lines reflect that."""

    def _build(self, lines, fns):
        from lcov_io import FileRecord, Function

        r = FileRecord(path="x.cpp")
        r.lines = dict(lines)
        r.functions = {n: Function(first_line=fl, hits=h) for n, (fl, h) in fns.items()}
        return r

    def test_inlined_only_callee_counts_as_hit(self):
        # ctor at line 5 with 3 hit body lines but FNDA=0 (inlined).
        r = self._build(
            lines={5: 1, 6: 1, 7: 1},
            fns={"ctor": (5, 0)},
        )
        self.assertEqual(r.hit_functions, 1)
        self.assertEqual(r.percent_functions, 100.0)

    def test_uncalled_no_lines_stays_uncovered(self):
        # Function declared at line 10 but no DA records in its range
        # AND FNDA=0 → not effectively hit.
        r = self._build(
            lines={5: 1, 6: 1},  # belong to a hypothetical function before line 10
            fns={"a": (5, 1), "b": (10, 0)},
        )
        self.assertEqual(r.hit_functions, 1)  # only `a`
        self.assertEqual(r.total_functions, 2)

    def test_fnda_called_is_always_hit(self):
        # FNDA > 0 always counts, regardless of line coverage.
        r = self._build(
            lines={},
            fns={"a": (1, 5)},
        )
        self.assertEqual(r.hit_functions, 1)




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




class FunctionBranchCoverageTests(unittest.TestCase):
    """Per-function branch coverage uses the same first_line-based
    range partition as `function_line_coverage` to assign each BRDA
    record to the function whose source range it falls within."""

    def _build(self, lines, branches, fns):
        from lcov_io import FileRecord, Function

        r = FileRecord(path="x.cpp")
        r.lines = dict(lines)
        r.branches = dict(branches)
        for n, (fl, h) in fns.items():
            r.functions[n] = Function(first_line=fl, hits=h)
        return r

    def test_two_function_branch_split(self):
        from lcov_io import function_branch_coverage

        # foo: lines 1-9 (foo at 1, bar at 10)
        # bar: lines 10+
        # Branches: BRDA at line 5 (in foo) and BRDA at line 12 (in bar).
        r = self._build(
            lines={1: 1, 5: 1, 10: 1, 12: 1},
            branches={
                (5, 0, 0): 7,
                (5, 0, 1): 0,
                (12, 0, 0): None,
                (12, 0, 1): 3,
            },
            fns={"foo": (1, 1), "bar": (10, 1)},
        )
        cov = function_branch_coverage(r)
        # foo has the (5,0,0) and (5,0,1): one taken (>0), one zero.
        self.assertEqual(cov["foo"], (2, 1))
        # bar has the (12,0,0) and (12,0,1): one None (no info), one taken.
        self.assertEqual(cov["bar"], (2, 1))

    def test_orphan_function_zero(self):
        from lcov_io import function_branch_coverage

        r = self._build(
            lines={1: 1},
            branches={(1, 0, 0): 5},
            fns={"orphan": (0, 1)},
        )
        self.assertEqual(function_branch_coverage(r)["orphan"], (0, 0))

    def test_no_branches_in_file(self):
        from lcov_io import function_branch_coverage

        r = self._build(
            lines={1: 1},
            branches={},
            fns={"foo": (1, 1)},
        )
        self.assertEqual(function_branch_coverage(r)["foo"], (0, 0))




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




if __name__ == "__main__":
    unittest.main(verbosity=2)
