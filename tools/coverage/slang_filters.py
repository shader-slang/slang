"""Slang-specific filter data for the generic coverage tools.

The customer-facing tools at `tools/coverage-html/` (the renderer
`slang-coverage-html.py` and the merger `slang-coverage-merge.py`)
are deliberately neutral — they accept generic
`--filter-include-regex` / `--filter-exclude-regex` /
`--strip-prefix` flags but don't bake in any project-specific
defaults. This module is the canonical home for Slang's CI
defaults, and the wrappers in this directory consume it.

Two pieces of data:

- `SLANG_CI_STRIP_PREFIXES` — the absolute path roots seen on the
  three Slang GitHub-Actions runners (Linux / macOS / Windows).
  Stripping them collapses per-OS LCOVs to a stable repo-relative
  shape that the merger can max-aggregate cleanly.

- `SLANGC_EXCLUDE_PATTERNS` — regexes that exclude files from the
  "slangc compiler-only" coverage view. Mirrors the patterns in
  `tools/coverage/slangc-ignore-patterns.sh`, the source of truth
  for the existing bash-side CI. Keep these two in sync.
"""

from typing import Tuple


SLANG_CI_STRIP_PREFIXES: Tuple[str, ...] = (
    "/__w/slang/slang/",                  # Linux GitHub Actions
    "/Users/runner/work/slang/slang/",    # macOS GitHub Actions
    "D:/a/slang/slang/",                  # Windows GitHub Actions
)


# Patterns are matched via `re.search()` on repo-relative paths
# (post `--strip-prefix`). Top-level segments (`external/`,
# `include/`, `tools/`, `source/...`) are anchored with `^` so nested
# directories like `tests/preprocessor/include/` aren't swept up by
# an unanchored substring match — those are test fixtures, not
# top-level non-pipeline source segments.
SLANGC_EXCLUDE_PATTERNS: Tuple[str, ...] = (
    r"^build/prelude/",
    r"^build/source/slang/(capability|fiddle|slang-lookup-tables)/",
    r"^build/source/slang-core-module/",
    r"^external/",
    r"^include/",
    r"^source/slang-core-module/",
    r"^source/slang-glslang/",
    r"^source/slang-record-replay/",
    r"^tools/",
    r"^source/slang/slang-(language-server|doc-markdown-writer|doc-ast|ast-dump|repro|workspace-version)[.\-]",
    # Language-server / doc-only files that live in compiler-core
    # (LSP protocol structs, JSON-RPC framing, doc-comment extraction).
    r"^source/compiler-core/slang-(language-server-protocol|json-rpc|doc-extractor)[.\-]",
    r"^source/slang/slang-ast-(expr|modifier|stmt)\.h$",
)
