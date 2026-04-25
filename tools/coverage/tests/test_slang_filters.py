#!/usr/bin/env python3
"""Tests for the Slang-specific filter data and wrapper scripts.

The customer-facing tools at `tools/coverage-html/` are deliberately
neutral — these tests verify the Slang wrappers translate
SLANGC_EXCLUDE_PATTERNS / SLANG_CI_STRIP_PREFIXES correctly into the
generic regex / strip-prefix flags.

Run from repo root:

    python3 -m unittest discover -s tools/coverage/tests -v
"""

import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


HERE = os.path.dirname(os.path.abspath(__file__))
SLANG_TOOL_DIR = os.path.abspath(os.path.join(HERE, os.pardir))
RENDER_WRAPPER = os.path.join(SLANG_TOOL_DIR, "slang-render.py")
MERGE_WRAPPER = os.path.join(SLANG_TOOL_DIR, "slang-merge.py")

sys.path.insert(0, SLANG_TOOL_DIR)
import slang_filters  # noqa: E402


class SlangcExcludePatternsTests(unittest.TestCase):
    """Mirrors tools/coverage/slangc-ignore-patterns.sh."""

    def setUp(self):
        self.combined = re.compile(
            "|".join(slang_filters.SLANGC_EXCLUDE_PATTERNS)
        )

    def _excluded(self, path: str) -> bool:
        # Backslashes are normalized by the merger before applying
        # filters; do the same here for parity.
        return bool(self.combined.search(path.replace("\\", "/")))

    def test_external_excluded(self):
        self.assertTrue(self._excluded("external/cmark/src/blocks.c"))

    def test_build_prelude_excluded(self):
        self.assertTrue(self._excluded("build/prelude/slang-cpp-prelude.h.cpp"))

    def test_build_fiddle_excluded(self):
        self.assertTrue(self._excluded(
            "build/source/slang/fiddle/slang-rich-diagnostics.h.fiddle"
        ))

    def test_tools_excluded(self):
        self.assertTrue(self._excluded("tools/slang-test/main.cpp"))

    def test_language_server_excluded(self):
        self.assertTrue(
            self._excluded("source/slang/slang-language-server.cpp")
        )
        self.assertTrue(
            self._excluded("source/slang/slang-language-server-protocol.h")
        )

    def test_ast_decl_headers_excluded(self):
        self.assertTrue(self._excluded("source/slang/slang-ast-expr.h"))
        self.assertTrue(self._excluded("source/slang/slang-ast-modifier.h"))
        # The .cpp counterpart is kept.
        self.assertFalse(self._excluded("source/slang/slang-ast-modifier.cpp"))

    def test_compiler_files_kept(self):
        self.assertFalse(self._excluded("source/slang/slang-check.cpp"))
        self.assertFalse(
            self._excluded("source/compiler-core/slang-name.cpp")
        )
        self.assertFalse(self._excluded("source/core/slang-string.cpp"))

    def test_compiler_core_lsp_excluded(self):
        # source/compiler-core/ also has LSP / doc-only files that
        # used to leak into the slangc report; they're excluded too.
        self.assertTrue(self._excluded(
            "source/compiler-core/slang-language-server-protocol.cpp"
        ))
        self.assertTrue(self._excluded(
            "source/compiler-core/slang-language-server-protocol.h"
        ))
        self.assertTrue(self._excluded(
            "source/compiler-core/slang-json-rpc.cpp"
        ))
        self.assertTrue(self._excluded(
            "source/compiler-core/slang-json-rpc-connection.cpp"
        ))
        self.assertTrue(self._excluded(
            "source/compiler-core/slang-doc-extractor.cpp"
        ))
        # General JSON helpers used by the pipeline (e.g. SPIRV
        # grammar parsing, source maps) are kept.
        self.assertFalse(self._excluded(
            "source/compiler-core/slang-json-native.cpp"
        ))
        self.assertFalse(self._excluded(
            "source/compiler-core/slang-json-source-map-util.cpp"
        ))
        self.assertFalse(self._excluded(
            "source/compiler-core/slang-pretty-writer.cpp"
        ))

    def test_backslash_paths_normalized(self):
        self.assertTrue(self._excluded(r"external\cmark\src\blocks.c"))


class SlangCiStripPrefixesTests(unittest.TestCase):
    def test_three_runner_roots(self):
        self.assertEqual(len(slang_filters.SLANG_CI_STRIP_PREFIXES), 3)

    def test_each_prefix_ends_with_slash(self):
        for p in slang_filters.SLANG_CI_STRIP_PREFIXES:
            self.assertTrue(p.endswith("/"), msg=p)


class WrapperCliTests(unittest.TestCase):
    """End-to-end: invoke the wrapper, verify it pre-injects the
    expected flags by observing the renderer/merger's output."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _write(self, name, content):
        path = os.path.join(self.tmp, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        return path

    def test_render_wrapper_drops_external(self):
        # An LCOV with one excluded path (external/) and two kept
        # (source/slang/, source/core/). Two kept files force the
        # common-prefix logic to leave path components visible so the
        # exclusion is observable in the rendered HTML.
        lcov = self._write(
            "in.info",
            "TN:\nSF:external/foo/bar.c\nDA:1,1\nend_of_record\n"
            "TN:\nSF:source/slang/x.cpp\nDA:1,1\nend_of_record\n"
            "TN:\nSF:source/core/y.cpp\nDA:1,1\nend_of_record\n",
        )
        out_dir = os.path.join(self.tmp, "out")
        res = subprocess.run(
            [sys.executable, RENDER_WRAPPER, lcov,
             "--output-dir", out_dir, "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        with open(os.path.join(out_dir, "index.html"), encoding="utf-8") as f:
            html = f.read()
        self.assertIn("x.cpp", html)
        self.assertIn("y.cpp", html)
        # The external/ path should NOT appear as a file row.
        self.assertNotIn("bar.c", html)
        self.assertNotIn("external", html)

    def test_merge_wrapper_strips_runner_prefixes(self):
        a = self._write(
            "linux.info",
            "TN:\nSF:/__w/slang/slang/source/foo.c\n"
            "DA:1,1\nend_of_record\n",
        )
        b = self._write(
            "macos.info",
            "TN:\nSF:/Users/runner/work/slang/slang/source/foo.c\n"
            "DA:2,1\nend_of_record\n",
        )
        c = self._write(
            "windows.info",
            "TN:\nSF:D:\\a\\slang\\slang\\source\\foo.c\n"
            "DA:3,1\nend_of_record\n",
        )
        res = subprocess.run(
            [sys.executable, MERGE_WRAPPER, a, b, c, "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # All three OS paths collapse to one entry.
        self.assertEqual(res.stdout.count("SF:source/foo.c"), 1)

    def test_merge_wrapper_drops_external(self):
        a = self._write(
            "linux.info",
            "TN:\nSF:/__w/slang/slang/external/foo.c\n"
            "DA:1,1\nend_of_record\n"
            "TN:\nSF:/__w/slang/slang/source/slang/x.cpp\n"
            "DA:1,1\nend_of_record\n",
        )
        res = subprocess.run(
            [sys.executable, MERGE_WRAPPER, a, "--quiet"],
            capture_output=True, text=True,
        )
        self.assertEqual(res.returncode, 0, msg=res.stderr)
        # external/foo.c is dropped; source/slang/x.cpp survives.
        self.assertNotIn("external/foo.c", res.stdout)
        self.assertIn("source/slang/x.cpp", res.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
