#!/usr/bin/env python3
"""Driver for the agentic test suite under tests-agentic/.

Tracks which test bundles are stale relative to their source documentation
and the source paths they declare in manifest.yaml, and lints generated
test bundles for structural conformance.

This script does NOT invoke any agent. Generation, review, and remediation
are all performed out-of-band by an operator running an agent against the
prompt templates under prompts/. Once a bundle has been (re)generated,
reviewed, or remediated, run the matching mark-* subcommand to record the
result in freshness.json or review-state.json.

Coverage data is never fed into a test-writing prompt as line-level
detail. The `expansion-candidates` subcommand outputs only bundle keys
and ranking scores, so that the expansion loop (Phase E) stays
doc-anchored.

Subcommands
-----------

    list                       List all bundles in the manifest.
    list-stale [--include-review]
                               List bundles whose source_doc or watched
                               paths have changed since the recorded
                               freshness entry (or that have no entry
                               yet). With --include-review, annotate
                               each bundle with its review state.
    digest <bundle>            Print the current watched-paths digest
                               and source-doc digest of a bundle.
    show <bundle>              Print the manifest entry plus the
                               resolved source files and the source doc
                               for a bundle.
    mark-fresh <bundle>        Record a fresh entry for <bundle> using
                               the current digests, HEAD commit, and
                               now() as the generation timestamp. Pass
                               --commit / --model to override.
    lint [<bundle>...]         Structural linter (README.md front-matter
                               present + valid, every .slang file has a
                               //META block, doc_ref resolves, size cap
                               respected) on the given bundles (default:
                               all).
    expansion-candidates [--from <report.json>]
                               Rank bundles by how lightly their
                               coverage_targets are exercised by the
                               last nightly coverage report. Output is
                               bundle keys + scores; source-line detail
                               is intentionally suppressed so it cannot
                               leak into an expansion prompt.
    coverage-gaps <bundle> [--from <slangc-report.txt>]
                               Show which of <bundle>'s coverage_targets
                               are most uncovered, given a per-file
                               llvm-cov report text dump. Output is
                               bundle-level only (file names + missed-
                               line counts), never source-line content.
                               Used as a priority signal in expansion
                               briefings while staying doc-anchored.
    review-status [<bundle>...]
                               (Phase D) Per-bundle review/remediation
                               freshness. Currently prints a not-yet-
                               implemented notice.
    mark-reviewed <bundle> [--report <path>]
                               (Phase D) Record a review entry for
                               <bundle>. Currently prints a not-yet-
                               implemented notice.
    mark-remediated <bundle> [--report <path>]
                               (Phase D) Record a remediation entry for
                               <bundle>. Currently prints a not-yet-
                               implemented notice.

Exit codes
----------
    0  success / no stale bundles / lint clean
    1  one or more checks failed
    2  invocation error (bad arguments, missing manifest, etc.)
"""

from __future__ import annotations

import argparse
import datetime as _dt
import hashlib
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

# --------------------------------------------------------------------------
# Repo + path helpers
# --------------------------------------------------------------------------

META_DIR = Path(__file__).resolve().parent
TESTS_ROOT = META_DIR.parent
REPO_ROOT = TESTS_ROOT.parent
MANIFEST_PATH = META_DIR / "manifest.yaml"
FRESHNESS_PATH = META_DIR / "freshness.json"
REVIEW_STATE_PATH = META_DIR / "review-state.json"
REVIEWS_DIR = META_DIR / "reviews"
REMEDIATIONS_DIR = META_DIR / "remediations"

# Required META keys on every generated .slang test file. The block lives
# as `//META: key=value` lines at the top of the file, since .slang has
# no native front-matter syntax.
_REQUIRED_TEST_META_KEYS = (
    "generated",
    "model",
    "generated_at",
    "source_commit",
    "doc_ref",
    "doc_section_digest",
    "purpose",
    "intent",
    "pipeline_stage",
    "warning",
)

_ALLOWED_INTENTS = (
    "functional",  # canonical smoke test, one per claim
    "boundary",  # boundary-value probe (min/max/overflow/underflow/zero/inf/NaN/empty/MAX+1 etc.)
    "stress",  # pattern-bloat / corner-case probe (deep nesting, many-arg packs, etc.)
    "negative",  # documented diagnostic / "is rejected" probe
    "expansion",  # added during a Phase E expansion pass
    "regression",  # anchored to a fixed compiler issue
)

_ALLOWED_GAP_KINDS = (
    "missing-example",  # doc names a claim but no minimal example
    "missing-surface",  # doc names an internal construct but no user-level syntax
    "undocumented-behavior",  # observable behavior, doc silent
    "cascading-only-mention",  # diagnostic / behavior shadowed in practice
    "ambiguous-claim",  # claim has >1 reasonable interpretation
    "drift-from-source",  # observed behavior contradicts the doc
)

_ALLOWED_OOS_REASONS = (
    # Test-harness alternatives — testable, just not via slang-test //TEST
    "needs-unit-test",  # observable in C++ but not via slangc CLI
    "needs-multi-file-test",  # cross-module / multi-translation-unit setup
    "needs-cli-test",  # CLI invocation / env / exit code / flag mapping
    # Runner-capability alternatives — testable with a different runner
    "gpu-dxr",
    "gpu-mesh-shader",
    "gpu-dxc-dxil",
    "gpu-fxc-dxbc",
    "gpu-cuda",
    "gpu-metal-toolchain",
    "gpu-wgsl-tint",
    "gpu-spirv-tools",
    "gpu-non-compute",
    "gpu-bindless",
    "gpu-cooperative",
    "gpu-vulkan-extension",
    "gpu-cross-api-flag",
    "gpu-other",
    # Truly terminal — no harness or runner upgrade unblocks
    "link-stage-only",
    "out-of-bundle",
    "deprecated",
    "process-doc",
    "internal-source-fact",
    "compile-time-toggle",
    "requires-external-tool",
    "implementation-detail",
    "(unclassified)",  # accepted in migrated rows; next regen should refine
)


def _rel_to_repo(path: Path) -> str:
    return str(path.relative_to(REPO_ROOT)).replace(os.sep, "/")


# --------------------------------------------------------------------------
# Tiny YAML loader (matches docs/llm-generated/_meta/regenerate.py shape).
# Avoids a hard PyYAML dependency; uses PyYAML if available.
# --------------------------------------------------------------------------

try:  # pragma: no cover - presence check only
    import yaml as _pyyaml  # type: ignore

    _HAVE_PYYAML = True
except ImportError:  # pragma: no cover
    _HAVE_PYYAML = False


def _load_yaml(text: str):
    if _HAVE_PYYAML:
        return _pyyaml.safe_load(text)
    return _MiniYaml(text).parse()


class _MiniYaml:
    """Indentation-based parser for the restricted YAML subset used here."""

    _NUM_RE = re.compile(r"^-?\d+$")

    def __init__(self, text: str):
        self.lines: list[tuple[int, int, str]] = []
        for raw_lineno, raw in enumerate(text.splitlines(), start=1):
            stripped = raw.split("#", 1)[0].rstrip()
            if not stripped.strip():
                continue
            indent = len(stripped) - len(stripped.lstrip(" "))
            self.lines.append((raw_lineno, indent, stripped))
        self.pos = 0

    def parse(self):
        if not self.lines:
            return None
        return self._parse_block(0)

    def _peek(self):
        if self.pos >= len(self.lines):
            return None
        return self.lines[self.pos]

    def _parse_block(self, indent: int):
        peek = self._peek()
        if peek is None:
            return None
        _, cur_indent, content = peek
        if cur_indent < indent:
            return None
        if content.lstrip().startswith("- "):
            return self._parse_sequence(cur_indent)
        return self._parse_mapping(cur_indent)

    def _parse_mapping(self, indent: int):
        out: dict = {}
        while True:
            peek = self._peek()
            if peek is None:
                break
            _, cur_indent, content = peek
            if cur_indent < indent:
                break
            if cur_indent > indent:
                raise ValueError(
                    f"unexpected indent at line {peek[0]}: {content!r}"
                )
            if ":" not in content:
                raise ValueError(
                    f"expected mapping at line {peek[0]}: {content!r}"
                )
            key, _, rest = content.partition(":")
            key = self._scalar(key.strip())
            rest = rest.strip()
            self.pos += 1
            if rest == "":
                child = self._parse_block(indent + 1)
                out[key] = child if child is not None else {}
            else:
                out[key] = self._scalar(rest)
        return out

    def _parse_sequence(self, indent: int):
        out: list = []
        while True:
            peek = self._peek()
            if peek is None:
                break
            _, cur_indent, content = peek
            if cur_indent < indent or not content.lstrip().startswith("- "):
                break
            if cur_indent > indent:
                raise ValueError(
                    f"unexpected indent in sequence at line {peek[0]}"
                )
            item_text = content.lstrip()[2:].strip()
            self.pos += 1
            if item_text == "":
                child = self._parse_block(indent + 2)
                out.append(child if child is not None else None)
            else:
                out.append(self._scalar(item_text))
        return out

    @staticmethod
    def _scalar(text: str):
        if text == "" or text == "~" or text.lower() == "null":
            return None
        if text.lower() == "true":
            return True
        if text.lower() == "false":
            return False
        if _MiniYaml._NUM_RE.match(text):
            return int(text)
        if (text.startswith("'") and text.endswith("'")) or (
            text.startswith('"') and text.endswith('"')
        ):
            return text[1:-1]
        return text


# --------------------------------------------------------------------------
# Manifest data model
# --------------------------------------------------------------------------


@dataclass
class BundleSpec:
    key: str
    prompt: str  # path relative to _meta/
    source_doc: str  # workspace-relative
    watched_paths: list[str]
    depends_on: list[str] = field(default_factory=list)
    coverage_targets: list[str] = field(default_factory=list)
    runner: dict = field(default_factory=dict)
    size_cap_files: int = 30

    @property
    def dir(self) -> str:
        """Workspace-relative bundle directory."""
        return f"tests-agentic/{self.key}"


@dataclass
class Manifest:
    version: int
    default_size_cap_files: int
    default_runner: dict
    bundles: dict[str, BundleSpec]


def load_manifest() -> Manifest:
    if not MANIFEST_PATH.exists():
        raise SystemExit(f"manifest not found: {MANIFEST_PATH}")
    raw = _load_yaml(MANIFEST_PATH.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise SystemExit("manifest must be a mapping")
    version = int(raw.get("version", 1))
    default_cap = int(raw.get("default_size_cap_files", 30))
    default_runner = raw.get("default_runner") or {}
    bundles_in = raw.get("bundles") or {}
    bundles: dict[str, BundleSpec] = {}
    for key, entry in bundles_in.items():
        if not isinstance(entry, dict):
            raise SystemExit(f"bundle entry {key!r} is not a mapping")
        watched = entry.get("watched_paths") or []
        if not watched:
            raise SystemExit(f"bundle {key!r} has no watched_paths")
        source_doc = entry.get("source_doc")
        if not source_doc:
            raise SystemExit(f"bundle {key!r} has no source_doc")
        runner = dict(default_runner)
        runner.update(entry.get("runner") or {})
        spec = BundleSpec(
            key=key,
            prompt=str(entry["prompt"]),
            source_doc=str(source_doc),
            watched_paths=[str(p) for p in watched],
            depends_on=[str(d) for d in entry.get("depends_on") or []],
            coverage_targets=[
                str(p) for p in entry.get("coverage_targets") or []
            ],
            runner=runner,
            size_cap_files=int(entry.get("size_cap_files", default_cap)),
        )
        bundles[key] = spec
    return Manifest(
        version=version,
        default_size_cap_files=default_cap,
        default_runner=default_runner,
        bundles=bundles,
    )


# --------------------------------------------------------------------------
# Freshness state
# --------------------------------------------------------------------------


def load_freshness() -> dict:
    if not FRESHNESS_PATH.exists():
        return {"schema_version": 1, "bundles": {}}
    return json.loads(FRESHNESS_PATH.read_text(encoding="utf-8"))


def save_freshness(state: dict) -> None:
    FRESHNESS_PATH.write_text(
        json.dumps(state, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def load_review_state() -> dict:
    if not REVIEW_STATE_PATH.exists():
        return {"schema_version": 1, "bundles": {}}
    return json.loads(REVIEW_STATE_PATH.read_text(encoding="utf-8"))


# --------------------------------------------------------------------------
# Path / glob resolution + digests
# --------------------------------------------------------------------------


def _expand_glob(pattern: str) -> list[Path]:
    """Expand a workspace-relative glob into a sorted list of files.

    Supports `**` for recursive matches. Directory results are walked into
    so that `prelude/*.h` returns header files but `source/slang/` returns
    every file under it.
    """
    matches = list(REPO_ROOT.glob(pattern)) if any(
        ch in pattern for ch in "*?["
    ) else [REPO_ROOT / pattern]
    files: list[Path] = []
    for m in matches:
        if not m.exists():
            continue
        if m.is_file():
            files.append(m)
        elif m.is_dir():
            for sub in m.rglob("*"):
                if sub.is_file():
                    files.append(sub)
    return sorted(set(files))


def resolve_watched_files(spec: BundleSpec) -> list[Path]:
    seen: dict[Path, None] = {}
    for pat in spec.watched_paths:
        for f in _expand_glob(pat):
            seen[f] = None
    return sorted(seen.keys())


def compute_watched_digest(spec: BundleSpec) -> str:
    """SHA-256 over (relpath, size, contents) of every watched file."""
    h = hashlib.sha256()
    for f in resolve_watched_files(spec):
        rel = _rel_to_repo(f).encode("utf-8")
        try:
            data = f.read_bytes()
        except OSError as exc:
            raise SystemExit(f"cannot read {f}: {exc}")
        h.update(b"P\x00")
        h.update(len(rel).to_bytes(4, "big"))
        h.update(rel)
        h.update(b"S\x00")
        h.update(len(data).to_bytes(8, "big"))
        h.update(hashlib.sha256(data).digest())
    return h.hexdigest()


def compute_source_doc_digest(spec: BundleSpec) -> str | None:
    p = REPO_ROOT / spec.source_doc
    if not p.exists():
        return None
    return hashlib.sha256(p.read_bytes()).hexdigest()


def head_commit() -> str:
    try:
        out = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=REPO_ROOT,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.CalledProcessError):
        return "unknown"
    return out.decode("ascii").strip()


# --------------------------------------------------------------------------
# Front-matter / META parsing
# --------------------------------------------------------------------------

_FM_RE = re.compile(r"^---\n(.*?\n)---\n", re.DOTALL)
_TEST_META_LINE_RE = re.compile(r"^//META:\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$")


def parse_bundle_front_matter(text: str) -> dict | None:
    """Parse README.md front-matter as a flat mapping."""
    m = _FM_RE.match(text)
    if not m:
        return None
    body = m.group(1)
    out: dict = {}
    for line in body.splitlines():
        if not line.strip() or line.strip().startswith("#"):
            continue
        if ":" not in line:
            return None
        key, _, val = line.partition(":")
        out[key.strip()] = val.strip().strip('"').strip("'")
    return out


@dataclass
class GapRow:
    """One row from a bundle's ## Doc gaps observed table."""

    anchor: str  # raw cell text, e.g. "[#vec-and-mat](../../docs/.../types.md#vec-and-mat)"
    anchor_fragment: str  # just "#vec-and-mat", extracted for grouping
    kind: str  # one of _ALLOWED_GAP_KINDS
    gap: str
    suggested_addition: str
    bundle: str  # bundle key that reported this row
    source_doc: str  # bundle's source_doc, for aggregation


_GAP_ANCHOR_FRAG_RE = re.compile(r"(#[A-Za-z0-9][A-Za-z0-9_-]*)")


def parse_gap_rows(text: str, bundle_key: str, source_doc: str) -> list[GapRow]:
    """Parse the ## Doc gaps observed section of a bundle README.

    Returns an empty list if the section is missing, has no rows, or
    cannot be parsed. The caller decides whether the empty result is
    expected or a lint failure.
    """
    rows: list[GapRow] = []
    lines = text.splitlines()
    i = 0
    n = len(lines)
    while i < n:
        if lines[i].strip() == "## Doc gaps observed":
            i += 1
            break
        i += 1
    else:
        return rows
    # Skip blank lines + header rows; collect any line that starts with `|`
    # and is not a divider (`| --- | --- |`).
    while i < n:
        line = lines[i]
        stripped = line.strip()
        if stripped.startswith("## "):
            break
        if not stripped.startswith("|"):
            i += 1
            continue
        # Split by `|` while honoring `\|` escapes; markdown tables
        # allow escaping a pipe inside a cell as `\|`.
        SENTINEL = "\x00PIPE\x00"
        protected = stripped.replace("\\|", SENTINEL)
        raw_cells = protected.split("|")[1:-1]
        cells = [c.replace(SENTINEL, "|").strip() for c in raw_cells]
        # Skip header row (contains "Anchor" / "Kind") and divider row.
        if not cells or all(set(c) <= set("- ") for c in cells):
            i += 1
            continue
        if cells[0].lower() == "anchor":
            i += 1
            continue
        if len(cells) < 4:
            i += 1
            continue
        anchor_cell, kind_cell, gap_cell, suggested_cell = cells[:4]
        frag_match = _GAP_ANCHOR_FRAG_RE.search(anchor_cell)
        anchor_fragment = frag_match.group(1) if frag_match else ""
        rows.append(
            GapRow(
                anchor=anchor_cell,
                anchor_fragment=anchor_fragment,
                kind=kind_cell,
                gap=gap_cell,
                suggested_addition=suggested_cell,
                bundle=bundle_key,
                source_doc=source_doc,
            )
        )
        i += 1
    return rows


def parse_test_meta(text: str) -> dict[str, str]:
    """Parse leading //META lines from a .slang test file.

    Reads //META: key=value lines at the top of the file. Stops at the
    first non-empty, non-//META, non-blank-comment line.
    """
    out: dict[str, str] = {}
    for line in text.splitlines():
        s = line.rstrip()
        if not s:
            continue
        if s.startswith("//META:"):
            m = _TEST_META_LINE_RE.match(s)
            if m:
                out[m.group(1)] = m.group(2).strip().strip('"').strip("'")
            continue
        # Allow blank lines and other // comments above the //META block.
        if s.startswith("//"):
            if out:
                # //META block ended; treat subsequent // as test body
                break
            continue
        # First non-comment line ends the header.
        break
    return out


# --------------------------------------------------------------------------
# Lint
# --------------------------------------------------------------------------


@dataclass
class LintIssue:
    where: str
    severity: str  # "error" | "warning"
    message: str


_REQUIRED_BUNDLE_FM_KEYS = (
    "generated",
    "model",
    "generated_at",
    "source_commit",
    "watched_paths_digest",
    "source_doc",
    "source_doc_digest",
    "warning",
)


def lint_bundle(spec: BundleSpec) -> list[LintIssue]:
    issues: list[LintIssue] = []
    bdir = REPO_ROOT / spec.dir
    bundle_md = bdir / "README.md"
    if not bdir.exists():
        # Missing bundle is reported by list-stale, not lint. Lint only
        # checks bundles that exist on disk.
        return issues
    if not bundle_md.exists():
        issues.append(LintIssue(spec.dir, "error", "README.md missing"))
        return issues
    text = bundle_md.read_text(encoding="utf-8")
    fm = parse_bundle_front_matter(text)
    if fm is None:
        issues.append(
            LintIssue(
                f"{spec.dir}/README.md",
                "error",
                "missing or malformed YAML front-matter",
            )
        )
    else:
        for k in _REQUIRED_BUNDLE_FM_KEYS:
            if k not in fm:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "error",
                        f"front-matter missing key: {k}",
                    )
                )
        if fm.get("generated", "").lower() != "true":
            issues.append(
                LintIssue(
                    f"{spec.dir}/README.md",
                    "error",
                    "front-matter generated must be true",
                )
            )
        if fm.get("source_doc") and fm["source_doc"] != spec.source_doc:
            issues.append(
                LintIssue(
                    f"{spec.dir}/README.md",
                    "error",
                    "front-matter source_doc"
                    f" {fm['source_doc']!r} does not match manifest source_doc"
                    f" {spec.source_doc!r}",
                )
            )

    test_files = sorted(bdir.glob("*.slang"))
    if len(test_files) > spec.size_cap_files:
        issues.append(
            LintIssue(
                spec.dir,
                "warning",
                f"bundle has {len(test_files)} .slang files; cap is"
                f" {spec.size_cap_files}",
            )
        )
    for tf in test_files:
        for issue in _lint_test_file(spec, tf):
            issues.append(issue)
    # Untested-claims table: optional section, but if present must be a
    # table with the controlled Reason vocabulary.
    heading = "## Untested claims"
    if heading in text:
        rows = _parse_tagged_table(text, heading)
        if not rows and not _section_explicitly_empty(text, heading):
            issues.append(
                LintIssue(
                    f"{spec.dir}/README.md",
                    "error",
                    f"{heading} section present but no table rows parsed"
                    f" (expected | Claim | Reason | Anchor | Why untested | columns)",
                )
            )
        for row in rows:
            tag = row[1]
            if tag not in _ALLOWED_OOS_REASONS:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "error",
                        f"{heading} Reason={tag!r} not in {list(_ALLOWED_OOS_REASONS)}",
                    )
                )

    # Doc-gaps table: optional section, but if present must be a table
    # with the controlled Kind vocabulary. Free-form bullets are no
    # longer accepted; the migration to the table format is one-shot.
    if "## Doc gaps observed" in text:
        gap_rows = parse_gap_rows(text, spec.dir, spec.source_doc)
        if not gap_rows and not _gap_section_explicitly_empty(text):
            issues.append(
                LintIssue(
                    f"{spec.dir}/README.md",
                    "error",
                    "## Doc gaps observed section present but no table"
                    " rows parsed (expected | Anchor | Kind | Gap |"
                    " Suggested addition | columns)",
                )
            )
        for row in gap_rows:
            if row.kind not in _ALLOWED_GAP_KINDS:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "error",
                        f"doc-gap Kind={row.kind!r} not in"
                        f" {list(_ALLOWED_GAP_KINDS)}",
                    )
                )
            if not row.anchor_fragment:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "warning",
                        f"doc-gap row Anchor cell has no #fragment:"
                        f" {row.anchor!r}",
                    )
                )
    return issues


def _parse_tagged_table(text: str, heading: str) -> list[tuple[str, str, str, str]]:
    """Parse a 4-column markdown table that lives under `heading`.

    Returns a list of (col1, col2, col3, col4) tuples. Pipe characters
    inside cells must be escaped as `\\|`; the parser unescapes them.
    Header and divider rows are skipped.
    """
    rows: list[tuple[str, str, str, str]] = []
    lines = text.splitlines()
    n = len(lines)
    i = 0
    while i < n and lines[i].strip() != heading:
        i += 1
    i += 1
    while i < n:
        line = lines[i]
        stripped = line.strip()
        if stripped.startswith("## "):
            break
        if not stripped.startswith("|"):
            i += 1
            continue
        SENTINEL = "\x00PIPE\x00"
        protected = stripped.replace("\\|", SENTINEL)
        raw_cells = protected.split("|")[1:-1]
        cells = [c.replace(SENTINEL, "|").strip() for c in raw_cells]
        if not cells or all(set(c) <= set("- ") for c in cells):
            i += 1
            continue
        # Header detection: any of the known column header tokens
        if cells[0].lower() in ("anchor", "claim"):
            i += 1
            continue
        if len(cells) >= 4:
            rows.append((cells[0], cells[1], cells[2], cells[3]))
        i += 1
    return rows


def _section_explicitly_empty(text: str, heading: str) -> bool:
    """True if `heading` exists and its body is an explicit empty marker.

    Recognised markers: `NA`, `(none)`, or `(none) — <prose>`.
    """
    in_section = False
    for raw in text.splitlines():
        s = raw.strip()
        if s == heading:
            in_section = True
            continue
        if in_section:
            if not s:
                continue
            if s.startswith("## "):
                return False
            if s == "NA" or s.startswith("(none)"):
                return True
            return False
    return False


def _gap_section_explicitly_empty(text: str) -> bool:
    """True if the gap section is explicitly marked empty.

    Recognised markers: `NA`, `(none)`, or `(none) — <prose>`.
    """
    in_section = False
    for raw in text.splitlines():
        line = raw.strip()
        if line == "## Doc gaps observed":
            in_section = True
            continue
        if in_section:
            if not line:
                continue
            if line.startswith("## "):
                return False
            if line == "NA" or line.startswith("(none)"):
                return True
            # Any other content -> not explicitly empty.
            return False
    return False


def _lint_test_file(spec: BundleSpec, tf: Path) -> list[LintIssue]:
    issues: list[LintIssue] = []
    rel = _rel_to_repo(tf)
    text = tf.read_text(encoding="utf-8")
    meta = parse_test_meta(text)
    if not meta:
        issues.append(LintIssue(rel, "error", "missing //META block"))
        return issues
    for k in _REQUIRED_TEST_META_KEYS:
        if k not in meta:
            issues.append(LintIssue(rel, "error", f"//META missing key: {k}"))
    if meta.get("generated", "").lower() != "true":
        issues.append(LintIssue(rel, "error", "//META generated must be true"))
    intent = meta.get("intent", "")
    if intent and intent not in _ALLOWED_INTENTS:
        issues.append(
            LintIssue(
                rel,
                "error",
                f"//META intent={intent!r} not in {list(_ALLOWED_INTENTS)}",
            )
        )
    doc_ref = meta.get("doc_ref", "")
    if doc_ref:
        target = doc_ref.split("#", 1)[0]
        if not target:
            issues.append(LintIssue(rel, "error", "//META doc_ref has empty path"))
        else:
            candidate = REPO_ROOT / target
            if not candidate.exists():
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        f"//META doc_ref path does not resolve: {target}",
                    )
                )
    # Every test file must contain at least one //TEST or //DIAGNOSTIC_TEST
    # directive — otherwise slang-test will silently skip it.
    if "//TEST" not in text and "//DIAGNOSTIC_TEST" not in text:
        issues.append(
            LintIssue(
                rel,
                "error",
                "no //TEST or //DIAGNOSTIC_TEST directive; slang-test would skip this file",
            )
        )
    # Each //TEST or //DIAGNOSTIC_TEST directive that declares a
    # matcher (filecheck=NAME, filecheck-buffer=NAME, or diag=NAME)
    # must have at least one matching `// NAME:` / `//NAME:` /
    # `/*NAME:` line in the file. Otherwise the directive runs but
    # verifies nothing.
    matcher_names = set()
    for m in re.finditer(
        r"//(?:DIAGNOSTIC_)?TEST[^\n]*?(?:filecheck|filecheck-buffer|diag)=([A-Za-z_][A-Za-z0-9_-]*)",
        text,
    ):
        matcher_names.add(m.group(1))
    for name in sorted(matcher_names):
        # Accept `// NAME:`, `//NAME:`, `/*NAME:`, plus the FileCheck
        # variants `NAME-DAG:`, `NAME-NEXT:`, `NAME-NOT:`, `NAME-SAME:`,
        # `NAME-LABEL:`, `NAME-EMPTY:`, `NAME-COUNT-N:`.
        body_has_pattern = bool(
            re.search(
                rf"(?://\s*|/\*\s*){re.escape(name)}(?:-(?:DAG|NEXT|NOT|SAME|LABEL|EMPTY|COUNT(?:-\d+)?))?:",
                text,
            )
        )
        if not body_has_pattern:
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    f"//TEST declares matcher {name!r} but no `// {name}:`"
                    f" pattern is present; the directive would verify nothing",
                )
            )
    return issues


# --------------------------------------------------------------------------
# Subcommand implementations
# --------------------------------------------------------------------------


def _bundles_arg(manifest: Manifest, names: list[str]) -> list[BundleSpec]:
    if not names:
        return list(manifest.bundles.values())
    out: list[BundleSpec] = []
    for n in names:
        if n not in manifest.bundles:
            raise SystemExit(f"unknown bundle: {n}")
        out.append(manifest.bundles[n])
    return out


def cmd_list(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    for key in sorted(manifest.bundles.keys()):
        print(key)
    return 0


def _classify(
    spec: BundleSpec, freshness: dict
) -> tuple[str, str, str | None, str | None]:
    """Return (status, reason, current_watched_digest, current_doc_digest).

    status ∈ {missing, stale, fresh}.
    """
    cur_watched = compute_watched_digest(spec)
    cur_doc = compute_source_doc_digest(spec)
    entry = freshness.get("bundles", {}).get(spec.key)
    bdir = REPO_ROOT / spec.dir
    if entry is None or not (bdir / "README.md").exists():
        return "missing", "no freshness entry or README.md", cur_watched, cur_doc
    if cur_doc is None:
        return "stale", "source_doc missing on disk", cur_watched, cur_doc
    if entry.get("source_doc_digest") != cur_doc:
        return "stale", "source_doc changed since last regen", cur_watched, cur_doc
    if entry.get("watched_paths_digest") != cur_watched:
        return "stale", "watched paths changed since last regen", cur_watched, cur_doc
    return "fresh", "", cur_watched, cur_doc


def cmd_list_stale(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    freshness = load_freshness()
    any_stale = False
    for key in sorted(manifest.bundles.keys()):
        spec = manifest.bundles[key]
        status, reason, _, _ = _classify(spec, freshness)
        if status != "fresh":
            any_stale = True
        if reason:
            print(f"{status:8s} {key}  ({reason})")
        else:
            print(f"{status:8s} {key}")
    return 1 if any_stale else 0


def cmd_digest(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    spec = manifest.bundles.get(args.bundle)
    if not spec:
        raise SystemExit(f"unknown bundle: {args.bundle}")
    print(f"watched_paths_digest: {compute_watched_digest(spec)}")
    doc_digest = compute_source_doc_digest(spec)
    print(f"source_doc_digest:    {doc_digest or '(source_doc missing)'}")
    return 0


def cmd_show(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    spec = manifest.bundles.get(args.bundle)
    if not spec:
        raise SystemExit(f"unknown bundle: {args.bundle}")
    print(f"bundle:          {spec.key}")
    print(f"dir:             {spec.dir}")
    print(f"prompt:          _meta/{spec.prompt}")
    print(f"source_doc:      {spec.source_doc}")
    doc_path = REPO_ROOT / spec.source_doc
    print(
        f"source_doc on disk: {'present' if doc_path.exists() else 'MISSING'}"
    )
    print(f"runner.category: {spec.runner.get('category', '(none)')}")
    print(f"size_cap_files:  {spec.size_cap_files}")
    print(f"depends_on:")
    for d in spec.depends_on:
        print(f"  - {d}")
    print(f"coverage_targets:")
    for t in spec.coverage_targets:
        print(f"  - {t}")
    files = resolve_watched_files(spec)
    print(f"watched_paths ({len(files)} files):")
    for f in files:
        print(f"  - {_rel_to_repo(f)}")
    return 0


def cmd_mark_fresh(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    spec = manifest.bundles.get(args.bundle)
    if not spec:
        raise SystemExit(f"unknown bundle: {args.bundle}")
    cur_watched = compute_watched_digest(spec)
    cur_doc = compute_source_doc_digest(spec)
    if cur_doc is None:
        raise SystemExit(
            f"cannot mark fresh: source_doc {spec.source_doc} not on disk"
        )
    freshness = load_freshness()
    freshness.setdefault("bundles", {})[spec.key] = {
        "generated_at": _dt.datetime.now(_dt.timezone.utc)
        .isoformat(timespec="seconds")
        .replace("+00:00", "+00:00"),
        "model": args.model or "unspecified",
        "source_commit": args.commit or head_commit(),
        "watched_paths_digest": cur_watched,
        "source_doc_digest": cur_doc,
    }
    save_freshness(freshness)
    print(f"marked fresh: {spec.key}")
    return 0


def cmd_lint(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    specs = _bundles_arg(manifest, args.bundles)
    issues: list[LintIssue] = []
    for s in specs:
        issues.extend(lint_bundle(s))
    errors = [i for i in issues if i.severity == "error"]
    warnings = [i for i in issues if i.severity == "warning"]
    for i in issues:
        print(f"{i.severity:7s} {i.where}: {i.message}")
    print(
        f"\nlint summary: {len(errors)} errors, {len(warnings)} warnings"
        f" across {len(specs)} bundles"
    )
    return 1 if errors else 0


def cmd_expansion_candidates(args: argparse.Namespace) -> int:
    """Rank bundles by under-coverage of their coverage_targets.

    Reads a nightly coverage report (JSON) and outputs ranked bundle keys
    only. The output is intentionally line-detail-free so it can't leak
    into a test-writing prompt.

    Coverage report shape (expected, lenient):
        { "files": {
            "source/slang/<file>.cpp": {
              "line_coverage_percent": 47.3,
              "uncovered_lines": <int>            # may be absent
            },
            ...
          }
        }
    """
    if not args.from_report:
        print(
            "expansion-candidates: no --from <report.json> provided.\n"
            "This subcommand is a hook for the Phase E nightly job. When a"
            " coverage report is available, pass it via --from and the"
            " driver will rank bundles by under-coverage. The output is"
            " intentionally bundle-level only.",
            file=sys.stderr,
        )
        return 0
    report_path = Path(args.from_report)
    if not report_path.exists():
        raise SystemExit(f"coverage report not found: {report_path}")
    try:
        report = json.loads(report_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise SystemExit(f"coverage report is not valid JSON: {exc}")
    files = report.get("files") or {}
    manifest = load_manifest()
    ranked: list[tuple[float, str]] = []
    for key, spec in manifest.bundles.items():
        if not spec.coverage_targets:
            continue
        pcts: list[float] = []
        for tgt in spec.coverage_targets:
            entry = files.get(tgt)
            if entry is None:
                continue
            pct = entry.get("line_coverage_percent")
            if isinstance(pct, (int, float)):
                pcts.append(float(pct))
        if not pcts:
            continue
        score = sum(pcts) / len(pcts)
        ranked.append((score, key))
    ranked.sort()  # ascending: lowest coverage first
    for score, key in ranked:
        print(f"{score:6.2f}  {key}")
    return 0


def cmd_coverage_gaps(args: argparse.Namespace) -> int:
    """Show which coverage_targets of <bundle> are most uncovered.

    Input is a per-file `llvm-cov report` text dump (e.g. the
    `*-slangc-report.txt` produced by
    `tmp/run-coverage-comparison.sh`). Output is bundle-level —
    file names + missed-line counts — never line-level. The agent
    receiving this hint in an expansion briefing remains
    doc-anchored; the gap data is just a priority signal.

    The report's columns are space-separated; the relevant ones
    are Filename (col 0) and Missed Lines (-5), Cover-% (-4).
    """
    manifest = load_manifest()
    spec = manifest.bundles.get(args.bundle)
    if not spec:
        raise SystemExit(f"unknown bundle: {args.bundle}")
    if not spec.coverage_targets:
        print(f"{args.bundle}: no coverage_targets declared.")
        return 0
    if not args.from_report:
        print(
            f"{args.bundle}: coverage_targets (no report supplied via --from):",
            file=sys.stderr,
        )
        for tgt in spec.coverage_targets:
            print(f"  - {tgt}")
        return 0

    report_path = Path(args.from_report)
    if not report_path.exists():
        raise SystemExit(f"report not found: {report_path}")

    # Parse the per-file report. Columns at end of each row:
    # ... <Lines> <Missed-Lines> <Cover%> <Branches> <Missed-Br> <Br-Cover%>
    file_data: dict[str, tuple[int, int, float]] = {}
    for raw in report_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if not raw or raw.startswith(("Filename", "----", "TOTAL", "warning:")):
            continue
        fields = raw.split()
        if len(fields) < 11:
            continue
        try:
            missed = int(fields[-5])
            total = int(fields[-6])
            pct_str = fields[-4]
            if not pct_str.endswith("%"):
                continue
            pct = float(pct_str.rstrip("%"))
        except (ValueError, IndexError):
            continue
        file_data[fields[0]] = (total, missed, pct)

    rows = []
    for tgt in spec.coverage_targets:
        d = file_data.get(tgt)
        if d is None:
            rows.append((tgt, None))
        else:
            rows.append((tgt, d))
    # Sort: highest missed-lines first; unowned ("not in report") last.
    rows.sort(key=lambda r: (-(r[1][1]) if r[1] else 0))

    print(f"# coverage-gaps for {args.bundle}")
    print(f"# report: {report_path}")
    print(f"# {len(spec.coverage_targets)} coverage_target file(s)")
    print()
    print(f"{'file':<60} {'total':>8} {'missed':>8} {'cov%':>7}")
    print("-" * 86)
    for tgt, d in rows:
        if d is None:
            print(f"{tgt:<60} {'(no data)':>8}")
        else:
            total, missed, pct = d
            print(f"{tgt:<60} {total:>8} {missed:>8} {pct:>6.1f}%")
    print()
    print("Use this list as a priority signal in an expansion briefing.")
    print("DO NOT show the agent any source-line content; the agent stays")
    print("doc-anchored. See _meta/prompts/_expand.md.")
    return 0


def cmd_doc_gaps(args: argparse.Namespace) -> int:
    """Aggregate ## Doc gaps observed rows across bundles, grouped by source_doc.

    The output is the feedback artifact the doc-regeneration workflow
    consumes: for each docs/llm-generated/<doc>.md, the list of gaps
    that bundles testing against it have reported. Rows reported by
    multiple bundles against the same Anchor + Kind are merged into
    a single row with a `Reported by` cell.
    """
    manifest = load_manifest()
    specs = list(manifest.bundles.values())
    if args.source_doc:
        specs = [s for s in specs if s.source_doc == args.source_doc]
        if not specs:
            raise SystemExit(
                f"no bundle has source_doc={args.source_doc!r}"
            )

    # source_doc -> list[GapRow]
    by_doc: dict[str, list[GapRow]] = {}
    for spec in specs:
        bdir = REPO_ROOT / spec.dir
        readme = bdir / "README.md"
        if not readme.exists():
            continue
        text = readme.read_text(encoding="utf-8")
        rows = parse_gap_rows(text, spec.dir, spec.source_doc)
        if rows:
            by_doc.setdefault(spec.source_doc, []).extend(rows)

    if args.format == "json":
        import json

        payload = {
            doc: [
                {
                    "anchor": r.anchor,
                    "anchor_fragment": r.anchor_fragment,
                    "kind": r.kind,
                    "gap": r.gap,
                    "suggested_addition": r.suggested_addition,
                    "reported_by": r.bundle,
                }
                for r in rows
            ]
            for doc, rows in sorted(by_doc.items())
        }
        print(json.dumps(payload, indent=2))
        return 0

    for doc in sorted(by_doc):
        rows = by_doc[doc]
        # Merge rows by (anchor_fragment, kind, gap-prose), tracking
        # which bundles reported them.
        merged: dict[tuple[str, str, str], dict] = {}
        for r in rows:
            key = (r.anchor_fragment, r.kind, r.gap)
            slot = merged.setdefault(
                key,
                {
                    "anchor": r.anchor,
                    "kind": r.kind,
                    "gap": r.gap,
                    "suggested_addition": r.suggested_addition,
                    "reported_by": [],
                },
            )
            if r.bundle not in slot["reported_by"]:
                slot["reported_by"].append(r.bundle)
            # If multiple bundles offer Suggested-addition text,
            # concatenate distinct contributions.
            if (
                r.suggested_addition
                and r.suggested_addition != slot["suggested_addition"]
                and r.suggested_addition not in slot["suggested_addition"]
            ):
                slot["suggested_addition"] = (
                    slot["suggested_addition"] + " // " + r.suggested_addition
                ).strip(" /")

        print(f"# Gaps reported against {doc}")
        print()
        print(f"Bundle reports: {', '.join(sorted({r.bundle for r in rows}))}")
        print()
        print(
            "| Anchor | Kind | Gap | Suggested addition | Reported by |"
        )
        print("| --- | --- | --- | --- | --- |")
        # Sort by anchor fragment, then kind.
        sorted_rows = sorted(
            merged.values(), key=lambda m: (m["anchor"], m["kind"])
        )
        for m in sorted_rows:
            reported = ", ".join(m["reported_by"])
            print(
                f"| {m['anchor']} | {m['kind']} | {m['gap']}"
                f" | {m['suggested_addition']} | {reported} |"
            )
        print()
    if not by_doc:
        print("# No doc-gap rows recorded across the selected bundles.")
    return 0


def cmd_review_status(args: argparse.Namespace) -> int:
    print(
        "review-status: not yet implemented.\n"
        "The two-stage review/remediation flow is a Phase D deliverable."
        " The schemas under _meta/schema/ define the report contracts;"
        " the wiring lands once bootstrap bundles exist.",
        file=sys.stderr,
    )
    return 0


def cmd_mark_reviewed(args: argparse.Namespace) -> int:
    print(
        "mark-reviewed: not yet implemented (Phase D).",
        file=sys.stderr,
    )
    return 0


def cmd_mark_remediated(args: argparse.Namespace) -> int:
    print(
        "mark-remediated: not yet implemented (Phase D).",
        file=sys.stderr,
    )
    return 0


# --------------------------------------------------------------------------
# argparse wiring
# --------------------------------------------------------------------------


def _build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="regenerate.py",
        description="Driver for the agentic test suite under tests-agentic/.",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("list", help="list every bundle").set_defaults(func=cmd_list)

    p_stale = sub.add_parser(
        "list-stale", help="classify each bundle as missing/stale/fresh"
    )
    p_stale.add_argument(
        "--include-review",
        action="store_true",
        help="(Phase D) annotate each row with its review state",
    )
    p_stale.set_defaults(func=cmd_list_stale)

    p_dig = sub.add_parser("digest", help="print current digests for a bundle")
    p_dig.add_argument("bundle")
    p_dig.set_defaults(func=cmd_digest)

    p_show = sub.add_parser("show", help="show manifest entry + resolved files")
    p_show.add_argument("bundle")
    p_show.set_defaults(func=cmd_show)

    p_mf = sub.add_parser("mark-fresh", help="record a fresh entry")
    p_mf.add_argument("bundle")
    p_mf.add_argument("--commit", help="override source_commit")
    p_mf.add_argument("--model", help="override model identifier")
    p_mf.set_defaults(func=cmd_mark_fresh)

    p_lint = sub.add_parser("lint", help="structural lint")
    p_lint.add_argument("bundles", nargs="*")
    p_lint.set_defaults(func=cmd_lint)

    p_exp = sub.add_parser(
        "expansion-candidates",
        help="rank bundles by under-coverage of their coverage_targets",
    )
    p_exp.add_argument(
        "--from",
        dest="from_report",
        help="path to a coverage report JSON",
    )
    p_exp.set_defaults(func=cmd_expansion_candidates)

    p_cg = sub.add_parser(
        "coverage-gaps",
        help="show which coverage_targets of a bundle are most uncovered "
        "(bundle-level only; no source-line content)",
    )
    p_cg.add_argument("bundle")
    p_cg.add_argument(
        "--from",
        dest="from_report",
        help="path to a per-file llvm-cov report .txt (e.g. *-slangc-report.txt)",
    )
    p_cg.set_defaults(func=cmd_coverage_gaps)

    p_dg = sub.add_parser(
        "doc-gaps",
        help="aggregate ## Doc gaps observed rows across bundles, grouped by source_doc",
    )
    p_dg.add_argument(
        "--source-doc",
        help="restrict to a single docs/llm-generated/<...>.md path",
    )
    p_dg.add_argument(
        "--format",
        choices=("md", "json"),
        default="md",
        help="output format (default: md)",
    )
    p_dg.set_defaults(func=cmd_doc_gaps)

    p_rs = sub.add_parser(
        "review-status",
        help="(Phase D) per-bundle review/remediation freshness",
    )
    p_rs.add_argument("bundles", nargs="*")
    p_rs.set_defaults(func=cmd_review_status)

    p_mr = sub.add_parser(
        "mark-reviewed", help="(Phase D) record a review entry"
    )
    p_mr.add_argument("bundle")
    p_mr.add_argument("--report")
    p_mr.set_defaults(func=cmd_mark_reviewed)

    p_mre = sub.add_parser(
        "mark-remediated", help="(Phase D) record a remediation entry"
    )
    p_mre.add_argument("bundle")
    p_mre.add_argument("--report")
    p_mre.set_defaults(func=cmd_mark_remediated)

    return p


def main(argv: list[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
