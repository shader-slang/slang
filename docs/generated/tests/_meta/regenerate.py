#!/usr/bin/env python3
"""Driver for the agentic test suite under docs/generated/tests/.

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
                               //META block, doc_ref resolves, README
                               links resolve and their anchors name real
                               headings, README tables are well-formed,
                               size cap respected) on the given bundles
                               (default: all).
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
import contextlib
import datetime as _dt
import hashlib
import io
import json
import os
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable
from urllib.parse import unquote

# --------------------------------------------------------------------------
# Repo + path helpers
# --------------------------------------------------------------------------

META_DIR = Path(__file__).resolve().parent
TESTS_ROOT = META_DIR.parent
# TESTS_ROOT lives at <repo>/docs/generated/tests, so climb three levels.
REPO_ROOT = TESTS_ROOT.parent.parent.parent
MANIFEST_PATH = META_DIR / "manifest.yaml"
FRESHNESS_PATH = META_DIR / "freshness.json"
REVIEW_STATE_PATH = META_DIR / "review-state.json"
REVIEWS_DIR = META_DIR / "reviews"
REMEDIATIONS_DIR = META_DIR / "remediations"
FINDINGS_DIR = META_DIR / "findings"
FINDINGS_FILED_DIR = FINDINGS_DIR / "filed"

# Findings that stopped reproducing before anyone filed them. Kept apart
# from findings/filed/ because that directory means "an issue exists for
# this"; these have no issue and never will. Without somewhere to put
# them they stay in the pending queue forever and every triage pass
# re-tests them from scratch -- five findings were in exactly that state
# when this was added.
FINDINGS_RESOLVED_DIR = FINDINGS_DIR / "resolved"
FINDINGS_STATE_PATH = META_DIR / "findings-state.json"

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
    "characterization",  # asserts observed current behaviour of gap-selected
    # code rather than a documented claim; only valid in a `coverage` bundle
    # (see docs/generated/tests/coverage/METHODOLOGY.md)
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
    "unsupported-on-target",  # this text-emit target cannot express the claim
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
    """Return `path` relative to the repo root, for use in messages.

    A path outside the repo is returned unchanged rather than raising:
    this only labels diagnostics, so a caller reporting on a file
    elsewhere should still get a readable message.
    """
    try:
        return str(path.relative_to(REPO_ROOT)).replace(os.sep, "/")
    except ValueError:
        return str(path)


# --------------------------------------------------------------------------
# Tiny YAML loader (matches docs/generated/design/_meta/regenerate.py shape).
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
        # Preserve raw lines so block scalars (|) can grab unfiltered content.
        self.raw_lines: list[str] = text.splitlines()
        self.lines: list[tuple[int, int, str]] = []
        for raw_lineno, raw in enumerate(self.raw_lines, start=1):
            stripped = self._strip_comment(raw).rstrip()
            if not stripped.strip():
                continue
            indent = len(stripped) - len(stripped.lstrip(" "))
            self.lines.append((raw_lineno, indent, stripped))
        self.pos = 0

    @staticmethod
    def _strip_comment(raw: str) -> str:
        """Strip YAML comment per spec: `#` is a comment only when at the
        start of the line or preceded by whitespace, AND not inside a
        single/double-quoted string. `#` embedded in a token
        (`issue #5627`) or in a quoted value (`"foo #bar"`) is literal.
        """
        in_quote: str | None = None
        for i, c in enumerate(raw):
            if in_quote is not None:
                if c == in_quote:
                    in_quote = None
                continue
            if c in ('"', "'"):
                in_quote = c
                continue
            if c == "#" and (i == 0 or raw[i - 1].isspace()):
                return raw[:i]
        return raw

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
            if rest in ("|", "|-", "|+"):
                out[key] = self._consume_block_scalar(peek[0], indent, rest)
            elif rest == "":
                child = self._parse_block(indent + 1)
                out[key] = child if child is not None else {}
            else:
                out[key] = self._scalar(rest)
        return out

    def _consume_block_scalar(
        self, key_lineno: int, key_indent: int, marker: str
    ) -> str:
        """Read a `|`-style literal block scalar.

        Content is every raw line below the key whose indent is strictly
        greater than `key_indent`. Indentation of the block is determined
        by the first non-empty content line; that prefix is stripped from
        every line. Trailing newlines are handled per the chomp marker
        (`|` keeps one trailing newline, `|-` strips all, `|+` keeps all).
        """
        # raw_lines is 0-indexed; key was on raw line (key_lineno - 1).
        raw_start = key_lineno  # first candidate content line
        collected: list[str] = []
        block_indent: int | None = None
        end_raw_idx = raw_start
        for idx in range(raw_start, len(self.raw_lines)):
            raw = self.raw_lines[idx]
            if raw.strip() == "":
                collected.append("")
                end_raw_idx = idx + 1
                continue
            stripped_left = raw.lstrip(" ")
            this_indent = len(raw) - len(stripped_left)
            if this_indent <= key_indent:
                break
            if block_indent is None:
                block_indent = this_indent
            if this_indent < block_indent:
                # Re-dedent line that's less indented than the block prefix
                # (still > key_indent though): treat as part of the block,
                # preserve relative indent.
                collected.append(raw[key_indent + 1 :])
            else:
                collected.append(raw[block_indent:])
            end_raw_idx = idx + 1
        # Advance the normalized-line cursor past all consumed lines.
        while self.pos < len(self.lines) and self.lines[self.pos][0] <= end_raw_idx:
            self.pos += 1
        # Strip leading blank lines collected before any content
        while collected and collected[0] == "":
            collected.pop(0)
        # Chomp behavior
        if marker == "|-":
            while collected and collected[-1] == "":
                collected.pop()
            return "\n".join(collected)
        elif marker == "|+":
            return "\n".join(collected) + "\n"
        else:  # plain "|" — keep one final newline, strip extras
            while len(collected) > 1 and collected[-1] == "" and collected[-2] == "":
                collected.pop()
            text = "\n".join(collected)
            if not text.endswith("\n"):
                text += "\n"
            return text

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
    # Workspace-relative path to the doc this bundle's claims come from, or
    # None for `coverage` bundles, which are selected by coverage gap rather
    # than anchored to a document. See `role`.
    source_doc: str | None
    watched_paths: list[str]
    role: str = "design"
    depends_on: list[str] = field(default_factory=list)
    coverage_targets: list[str] = field(default_factory=list)
    size_cap_files: int = 30

    @property
    def dir(self) -> str:
        """Workspace-relative bundle directory."""
        return f"docs/generated/tests/{self.key}"

    @property
    def gap_group(self) -> str:
        """Label this bundle's doc-gap rows are aggregated under.

        `doc-gaps` groups rows by the document they were observed against; a
        coverage bundle has no such document, so it groups under its own key.
        """
        return self.source_doc or f"{self.key} (no source doc)"

    @property
    def is_doc_anchored(self) -> bool:
        """Whether this bundle derives its claims from a `source_doc`.

        True for `conformance/` bundles (anchored to the human-written
        language reference) and `design/` bundles (anchored to the generated
        design docs). False for `coverage/` bundles, whose tests are chosen by
        uncovered compiler code and assert observed current behaviour, so
        there is no document whose edit should invalidate them — only their
        `watched_paths` can. See `coverage/METHODOLOGY.md`.
        """
        return self.role != "coverage"

    @property
    def is_diagnostics_catalog(self) -> bool:
        """Whether this is the diagnostics catalog, the one doc-anchored
        bundle whose `doc_ref` names a source definition instead of prose.

        Its tests cite the `err()` call in `slang-diagnostics.lua` or the
        `DIAGNOSTIC()` macro in a `*-diagnostic-defs.h`, so their `doc_ref`
        carries no `#anchor` and `lint_doc_section_digests` cannot check them.
        `lint_catalog_entry_digests` checks them instead, against the catalog
        entry rather than a doc section.

        Matched by key rather than by testing `dir` for a substring, so a
        future bundle that merely has "diagnostics-catalog" in its path does
        not silently inherit catalog-only lint rules.
        """
        return self.key == "design/cross-cutting/diagnostics-catalog"

    @property
    def prompt(self) -> str:
        """Workspace-relative per-bundle prompt path. Co-located with the
        bundle as `_prompt.md` (derived from `dir`, not stored in the
        manifest)."""
        return f"{self.dir}/_prompt.md"


@dataclass
class Manifest:
    version: int
    default_size_cap_files: int
    bundles: dict[str, BundleSpec]


def load_manifest() -> Manifest:
    if not MANIFEST_PATH.exists():
        raise SystemExit(f"manifest not found: {MANIFEST_PATH}")
    raw = _load_yaml(MANIFEST_PATH.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise SystemExit("manifest must be a mapping")
    version = int(raw.get("version", 1))
    default_cap = int(raw.get("default_size_cap_files", 30))
    bundles_in = raw.get("bundles") or {}
    bundles: dict[str, BundleSpec] = {}
    for key, entry in bundles_in.items():
        if not isinstance(entry, dict):
            raise SystemExit(f"bundle entry {key!r} is not a mapping")
        watched = entry.get("watched_paths") or []
        if not watched:
            raise SystemExit(f"bundle {key!r} has no watched_paths")
        role = str(entry.get("role") or key.split("/", 1)[0])
        if role not in ("conformance", "design", "coverage"):
            raise SystemExit(f"bundle {key!r} has unknown role {role!r}")
        if not key.startswith(f"{role}/"):
            raise SystemExit(
                f"bundle {key!r} has role {role!r} that does not match its key prefix"
            )
        source_doc = entry.get("source_doc")
        # `coverage` bundles are gap-selected, not doc-anchored, so a
        # source_doc would be meaningless for them; every other role requires
        # one, because its tests must cite a claim in that document.
        if role == "coverage":
            if source_doc:
                raise SystemExit(
                    f"bundle {key!r} has role 'coverage' but declares a source_doc;"
                    " coverage bundles are selected by coverage gap, not anchored"
                    " to a document"
                )
        elif not source_doc:
            raise SystemExit(f"bundle {key!r} has no source_doc")
        spec = BundleSpec(
            key=key,
            source_doc=str(source_doc) if source_doc else None,
            role=role,
            watched_paths=[str(p) for p in watched],
            depends_on=[str(d) for d in entry.get("depends_on") or []],
            coverage_targets=[
                str(p) for p in entry.get("coverage_targets") or []
            ],
            size_cap_files=int(entry.get("size_cap_files", default_cap)),
        )
        bundles[key] = spec
    return Manifest(
        version=version,
        default_size_cap_files=default_cap,
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
# Findings (Phase F: structured records of suspected compiler bugs that
# generation agents emit; triaged + filed via `regenerate.py findings`).
# --------------------------------------------------------------------------


_REQUIRED_FINDING_KEYS = (
    "schema_version",
    "id",
    "bundle",
    "suspected_kind",
    "observed_at",
    "evidence",
    "expected",
    "provenance",
)
_REQUIRED_EVIDENCE_KEYS = ("command", "source_slang", "observed_summary")

# A path in `evidence.command` that will not exist after the session ends. A
# finding whose command names one of these cannot be re-run later, so it cannot
# be confirmed still-reproducing before filing -- which is the one thing triage
# has to do. Seventeen sigsegv findings were triaged in one pass and seven were
# untestable for exactly this reason; two of those produced convincing false
# "already fixed" results, because substituting the committed `source_slang`
# silently fed slangc a different program.
_SCRATCH_PATH_RE = re.compile(r"(?:^|[\s=])(?:/tmp/|/var/folders/)\S*|<file>|<tmp>")
_REQUIRED_EXPECTED_KEYS = ("claim", "citation_kind", "citation")
_REQUIRED_PROVENANCE_KEYS = ("agent_model", "source_commit", "doc_anchor")
_SUSPECTED_KINDS = {
    "sigsegv",
    "wrong-diagnostic",
    "wrong-codegen",
    "regression",
    "catalog-drift",
    "doc-claim-overstated",
}
_CITATION_KINDS = {"spec", "sibling-test", "older-slangc", "doc"}


def load_findings_state() -> dict:
    if not FINDINGS_STATE_PATH.exists():
        return {
            "schema_version": 1,
            "filing_defaults": {},
            "findings": {},
        }
    return json.loads(FINDINGS_STATE_PATH.read_text(encoding="utf-8"))


def save_findings_state(state: dict) -> None:
    FINDINGS_STATE_PATH.write_text(
        json.dumps(state, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def list_finding_paths(
    include_filed: bool = False, include_resolved: bool = False
) -> list[Path]:
    """Return sorted list of finding YAML paths under _meta/findings/.

    Pending findings live directly under findings/. Ones with an issue
    move to findings/filed/, ones that stopped reproducing before anyone
    filed them move to findings/resolved/; both are excluded by default,
    because the bare list is the work queue.
    """
    if not FINDINGS_DIR.exists():
        return []
    out: list[Path] = [p for p in sorted(FINDINGS_DIR.glob("*.yaml"))]
    if include_filed and FINDINGS_FILED_DIR.exists():
        out.extend(sorted(FINDINGS_FILED_DIR.glob("*.yaml")))
    if include_resolved and FINDINGS_RESOLVED_DIR.exists():
        out.extend(sorted(FINDINGS_RESOLVED_DIR.glob("*.yaml")))
    return out


def load_finding(path: Path) -> dict:
    """Load a finding, normalizing `observed_at` to the ISO string the
    schema declares.

    An unquoted ISO timestamp is a *date* in YAML, not a string, so
    `observed_at: 2026-08-04T00:00:00+00:00` loads as a `datetime` while
    the quoted spelling next to it loads as `str`. Both are the same
    instant and both look identical in the file, so the difference is
    invisible until something does string work on the value -- which is
    how `findings list` came to abort partway through the queue with
    `'datetime.datetime' object is not subscriptable`. 21 of the 83
    committed findings use the unquoted spelling.

    Normalizing here rather than at the call site keeps one in-memory
    contract for every consumer, and matches what the schema already
    says the field is. The files are left alone: 10 of the 21 are under
    findings/filed/, which is immutable history.
    """
    finding = _load_yaml(path.read_text(encoding="utf-8")) or {}
    observed = finding.get("observed_at")
    if isinstance(observed, (_dt.datetime, _dt.date)):
        finding["observed_at"] = observed.isoformat()
    return finding


def _lint_finding_reproducibility(where: str, ev: dict) -> list[LintIssue]:
    """Check that a finding can still be re-run after the session that filed it.

    Triage has exactly one job before filing: confirm the failure still
    happens. That is impossible if the recorded `command` points at a
    scratch path -- `/tmp/foo.slang`, `<file>` -- because the input is
    gone. Substituting `evidence.source_slang` is not a fix: when the
    failing input was a *variant* of the committed test, the substitution
    silently compiles a different program. In one triage pass that turned
    two live bugs into apparent "already fixed" results, which is worse
    than having no result at all.

    So: name a scratch path in the command, and the real input must be
    inlined in `evidence.repro_source`. This is a warning rather than an
    error because 28 of the 82 findings committed before the rule predate
    it; the fix is a sweep, not a gate.
    """
    issues: list[LintIssue] = []
    cmd = str(ev.get("command") or "")
    scratch = _SCRATCH_PATH_RE.search(cmd)
    has_inline = bool(str(ev.get("repro_source") or "").strip()) or bool(
        str(ev.get("minimized_repro") or "").strip()
    )
    if scratch and not has_inline:
        issues.append(
            LintIssue(
                where,
                "warning",
                f"evidence.command names a scratch path ({scratch.group(0).strip()!r})"
                " that will not exist later, and no evidence.repro_source is"
                " given, so this finding cannot be re-run to confirm it still"
                " reproduces before filing",
            )
        )
    src = str(ev.get("source_slang") or "")
    if (
        src.endswith(".slang")
        and not src.startswith(("/tmp", "<"))
        and not (REPO_ROOT / src).exists()
        and not has_inline
    ):
        issues.append(
            LintIssue(
                where,
                "warning",
                f"evidence.source_slang {src!r} does not exist and no"
                " evidence.repro_source is given; the finding has no runnable input",
            )
        )
    return issues


def validate_finding(path: Path, finding: dict) -> list[LintIssue]:
    """Structural validation. Mirrors finding.schema.json required fields."""
    issues: list[LintIssue] = []
    where = str(path.relative_to(REPO_ROOT))
    if not isinstance(finding, dict):
        issues.append(LintIssue(where, "error", "top-level must be a mapping"))
        return issues
    for k in _REQUIRED_FINDING_KEYS:
        if k not in finding:
            issues.append(LintIssue(where, "error", f"missing key: {k}"))
    if finding.get("schema_version") != 1:
        issues.append(
            LintIssue(where, "error", "schema_version must be 1")
        )
    fid = finding.get("id", "")
    if fid and not re.fullmatch(r"[a-z0-9][a-z0-9-]*", str(fid)):
        issues.append(
            LintIssue(where, "error", f"id {fid!r} not a kebab slug")
        )
    expected_filename = f"{fid}.yaml" if fid else None
    if expected_filename and path.name != expected_filename:
        issues.append(
            LintIssue(
                where,
                "error",
                f"filename must match id: expected {expected_filename!r}",
            )
        )
    sk = finding.get("suspected_kind")
    if sk and sk not in _SUSPECTED_KINDS:
        issues.append(
            LintIssue(where, "error", f"suspected_kind {sk!r} not in vocabulary")
        )
    title = finding.get("title")
    if title is not None and len(str(title)) > 80:
        issues.append(
            LintIssue(where, "error", f"title exceeds 80 chars ({len(title)})")
        )
    ev = finding.get("evidence")
    if isinstance(ev, dict):
        issues.extend(_lint_finding_reproducibility(where, ev))
    if isinstance(ev, dict):
        for k in _REQUIRED_EVIDENCE_KEYS:
            if k not in ev:
                issues.append(
                    LintIssue(where, "error", f"evidence missing key: {k}")
                )
    elif "evidence" in finding:
        issues.append(LintIssue(where, "error", "evidence must be a mapping"))
    exp = finding.get("expected")
    if isinstance(exp, dict):
        for k in _REQUIRED_EXPECTED_KEYS:
            if k not in exp:
                issues.append(
                    LintIssue(where, "error", f"expected missing key: {k}")
                )
        ck = exp.get("citation_kind")
        if ck and ck not in _CITATION_KINDS:
            issues.append(
                LintIssue(
                    where, "error", f"expected.citation_kind {ck!r} not in vocabulary"
                )
            )
        citation = exp.get("citation")
        # For path-style citations (doc, sibling-test, spec), require the
        # path to resolve on disk. older-slangc citations are git refs,
        # not paths, so we don't check them here.
        if ck in ("doc", "sibling-test", "spec") and isinstance(citation, str):
            # Strip any optional `#anchor` or `:line` suffix before
            # checking the file exists.
            base = citation.split("#", 1)[0].split(":", 1)[0]
            if base and not (REPO_ROOT / base).exists():
                issues.append(
                    LintIssue(
                        where,
                        "error",
                        f"expected.citation does not resolve: {base!r}"
                        f" (kind={ck})",
                    )
                )
    elif "expected" in finding:
        issues.append(LintIssue(where, "error", "expected must be a mapping"))
    prov = finding.get("provenance")
    if isinstance(prov, dict):
        for k in _REQUIRED_PROVENANCE_KEYS:
            if k not in prov:
                issues.append(
                    LintIssue(where, "error", f"provenance missing key: {k}")
                )
    elif "provenance" in finding:
        issues.append(LintIssue(where, "error", "provenance must be a mapping"))
    return issues


def lint_findings() -> list[LintIssue]:
    """Validate every pending finding YAML.

    Retired findings — filed ones under findings/filed/, ones that
    stopped reproducing under findings/resolved/ — are immutable history
    and are not re-validated. Their citations or referenced paths may
    have rotted since, and that is acceptable for a historical record.

    They are still checked for *parseability*, though, which is a
    different thing from rot. A retired finding that no longer loads is
    not stale history, it is unreadable history: every tool that walks
    the whole corpus trips over it, and the record it was meant to
    preserve is effectively gone. One shipped that way in the bootstrap
    commit — an unquoted `observed_summary` containing
    `spirv-emit: castToVoid`, where the colon-space turned the scalar
    into a mapping — and went unnoticed because this function skipped
    the directory entirely.
    """
    issues: list[LintIssue] = []
    pending = set(list_finding_paths(include_filed=False))
    retired = set(list_finding_paths(include_filed=True, include_resolved=True)) - pending
    for p in sorted(retired):
        try:
            load_finding(p)
        except Exception as exc:  # noqa: BLE001
            issues.append(
                LintIssue(
                    str(p.relative_to(REPO_ROOT)),
                    "error",
                    f"retired finding no longer parses as YAML: {exc}",
                )
            )
    seen_bundles_in_manifest = set(load_manifest().bundles.keys())
    for p in list_finding_paths(include_filed=False):
        try:
            finding = load_finding(p)
        except Exception as exc:  # noqa: BLE001
            issues.append(
                LintIssue(
                    str(p.relative_to(REPO_ROOT)),
                    "error",
                    f"failed to parse YAML: {exc}",
                )
            )
            continue
        issues.extend(validate_finding(p, finding))
        b = finding.get("bundle") if isinstance(finding, dict) else None
        if b and b not in seen_bundles_in_manifest:
            issues.append(
                LintIssue(
                    str(p.relative_to(REPO_ROOT)),
                    "error",
                    f"bundle {b!r} not in manifest",
                )
            )
    return issues


def lint_expected_failures() -> list[LintIssue]:
    """Validate `_meta/expected-failures.txt`.

    Every non-comment, non-blank line must:
    - be a path that resolves under the workspace,
    - be preceded somewhere above by a comment line containing a
      tracking-issue URL (or at least `#NNN` / `issues/`) — this
      enforces that entries are documented with a compiler-bug
      reference, not silent silencing.

    The file is optional; absence is fine.
    """
    issues: list[LintIssue] = []
    path = (
        REPO_ROOT
        / "docs"
        / "generated"
        / "tests"
        / "_meta"
        / "expected-failures.txt"
    )
    if not path.is_file():
        return issues
    rel = str(path.relative_to(REPO_ROOT))

    # An "active" link comment covers all paths until either a blank
    # line or another link comment resets the group. This lets one
    # tracking-issue cover a block of paths without repeating the link.
    active_link = False
    # Accept either a real tracking-issue link (URLs, github.com, #NNN,
    # issues/...) OR a campaign-mode pending-finding reference (e.g.
    # "Pending: ..._meta/findings/<id>.yaml"). The latter is converted
    # to a real URL at human-triage time; see _common.md § Campaign mode.
    link_re = re.compile(r"(https?://|github\.com|#\d+|issues/|_meta/findings/)")

    for lineno, raw in enumerate(path.read_text().splitlines(), start=1):
        s = raw.strip()
        if not s:
            # Blank line ends the current group.
            active_link = False
            continue
        if s.startswith("#"):
            if link_re.search(s):
                active_link = True
            continue
        # Non-comment, non-blank → must be a real path. slang-test matches
        # expected-failure entries against the full test name, which for
        # multi-config tests carries a trailing " (config)" suffix (e.g.
        # "foo.slang (cpu)"); strip it before resolving the file path.
        path_part = re.sub(r" \([a-z0-9-]+\)$", "", s)
        # slang-test names the Nth //TEST directive in a file
        # "<file>.slang.N" (the first directive keeps the bare file name), so
        # an entry may legitimately target one directive rather than the whole
        # file — which is what emission fan-out produces, where only one
        # target of several is known-failing. Strip that index before
        # resolving; "foo.slang.2" is the file "foo.slang".
        path_part = re.sub(r"(?<=\.slang)\.\d+$", "", path_part)
        candidate = REPO_ROOT / path_part
        if not candidate.exists():
            issues.append(
                LintIssue(
                    f"{rel}:{lineno}",
                    "error",
                    f"expected-failure path does not resolve: {s}",
                )
            )
        if not active_link:
            issues.append(
                LintIssue(
                    f"{rel}:{lineno}",
                    "error",
                    f"expected-failure entry has no tracking-issue link"
                    f" comment in its block: {s}. Add a comment like"
                    f" '# https://github.com/shader-slang/slang/issues/NNNN'"
                    f" above the entry (or above the group it belongs to)"
                    f" so the entry is removable when the bug is fixed.",
                )
            )
    return issues


def lint_agentic_coverage_excludes() -> list[LintIssue]:
    """Validate that every path in `_meta/agentic-coverage-excludes.txt` still exists.

    `tools/coverage/run-coverage.sh` turns each entry into a slang-test
    `-exclude-prefix` flag for the coverage agentic pass, which runs
    in-process (`-server-count 1`), so a test that segfaults the compiler
    segfaults the orchestrator and the rest of the suite never runs. The
    entries here are what keep that from happening.

    `-exclude-prefix` silently matches nothing when its path is wrong, so an
    entry orphaned by a regeneration round that renames or moves its test
    stops excluding anything and the crash comes back. That has happened
    twice: PR #11421's directory reorg, and the 2026-08-04 round that renamed
    `metadata/unorm-attr-on-buffer-element.slang` (coverage run 31235323797).
    Resolving each entry as a path on disk turns both into a lint error.

    Directory prefixes are legal — `-exclude-prefix` is a prefix filter, so
    `.../design/pipeline/` excludes everything under it — and resolve as
    directories, which is why this checks `exists()` rather than `is_file()`.

    The file is optional; absence is fine.
    """
    issues: list[LintIssue] = []
    path = (
        REPO_ROOT
        / "docs"
        / "generated"
        / "tests"
        / "_meta"
        / "agentic-coverage-excludes.txt"
    )
    if not path.is_file():
        return issues
    rel = str(path.relative_to(REPO_ROOT))

    for lineno, raw in enumerate(path.read_text().splitlines(), start=1):
        # Mirror run-coverage.sh's parse: strip a trailing comment, then trim.
        s = raw.split("#", 1)[0].strip()
        if not s:
            continue
        if not (REPO_ROOT / s).exists():
            issues.append(
                LintIssue(
                    f"{rel}:{lineno}",
                    "error",
                    f"agentic-coverage-exclude path does not resolve: {s}."
                    f" A stale entry excludes nothing, so the crash it was"
                    f" added for will kill the coverage agentic pass again."
                    f" Point it at the test's current path, or drop it if the"
                    f" test is gone.",
                )
            )
    return issues


def compute_finding_title(finding: dict) -> str:
    """Return either the explicit title, or derive from scope + summary."""
    explicit = finding.get("title")
    if explicit:
        return str(explicit)
    scope = finding.get("scope")
    summary = (finding.get("evidence") or {}).get("observed_summary", "")
    summary = " ".join(str(summary).split())  # collapse whitespace
    if scope:
        prefix = f"[{scope}] "
        budget = 80 - len(prefix)
        if len(summary) > budget:
            summary = summary[: budget - 1].rstrip() + "…"
        return prefix + summary
    if len(summary) > 80:
        summary = summary[:79].rstrip() + "…"
    return summary


def compute_finding_labels(finding: dict, state: dict) -> list[str]:
    cfg = (state.get("filing_defaults") or {}).get("labels") or {}
    out: list[str] = list(cfg.get("always") or [])
    by_kind = cfg.get("by_suspected_kind") or {}
    out.extend(by_kind.get(finding.get("suspected_kind"), []))
    by_scope = cfg.get("by_scope") or {}
    if finding.get("scope"):
        out.extend(by_scope.get(finding["scope"], []))
    # Dedup while preserving order
    seen: set[str] = set()
    deduped: list[str] = []
    for lbl in out:
        if lbl not in seen:
            seen.add(lbl)
            deduped.append(lbl)
    return deduped


def _repro_body(finding: dict) -> str:
    """Return the .slang source block for the issue body.

    Prefers the operator's `minimized_repro`, then the agent's inlined
    `repro_source`, and only then reads `source_slang` from disk.

    The `repro_source` step is not optional politeness: the reproducibility
    rule tells an agent whose failing input was a scratch file to inline it
    there, precisely because `source_slang` then names a *different*
    program -- the committed test the scratch input was derived from.
    Falling through to disk in that case pairs the recorded command with a
    shader that does not reproduce the bug, which is the exact confusion
    the rule exists to prevent. This function honoured `minimized_repro`
    but not `repro_source`, so until now it did just that.
    """
    ev = finding.get("evidence") or {}
    if ev.get("minimized_repro"):
        return str(ev["minimized_repro"]).rstrip() + "\n"
    if ev.get("repro_source"):
        return str(ev["repro_source"]).rstrip() + "\n"
    src = ev.get("source_slang")
    if not src:
        return "(no source_slang on finding)\n"
    p = REPO_ROOT / src
    if not p.exists():
        return f"(source_slang not found on disk: {src})\n"
    return p.read_text(encoding="utf-8")


def render_finding_md(finding: dict) -> str:
    """Render the finding YAML to the issue body markdown."""
    ev = finding.get("evidence") or {}
    exp = finding.get("expected") or {}
    prov = finding.get("provenance") or {}
    asa = finding.get("agent_self_assessment") or {}
    repro = _repro_body(finding)
    repro_lines = repro.count("\n")
    cap = (
        load_findings_state()
        .get("filing_defaults", {})
        .get("repro_line_cap", 40)
    )
    lines: list[str] = []
    lines.append("## Summary")
    lines.append("")
    lines.append(str(ev.get("observed_summary", "")).strip() or "_(no summary)_")
    lines.append("")
    lines.append("## Repro")
    lines.append("")
    lines.append("```slang")
    lines.append(repro.rstrip("\n"))
    lines.append("```")
    lines.append("")
    if repro_lines > cap:
        lines.append(
            f"_Note: repro is {repro_lines} lines; cap is {cap}."
            f" Further minimization recommended before filing._"
        )
        lines.append("")
    lines.append("Command:")
    lines.append("")
    lines.append("```")
    lines.append(str(ev.get("command", "")).strip())
    lines.append("```")
    lines.append("")
    lines.append("## Expected")
    lines.append("")
    lines.append(str(exp.get("claim", "")).strip() or "_(no claim)_")
    lines.append("")
    ck = exp.get("citation_kind") or "unspecified"
    citation = str(exp.get("citation", "")).strip() or "_(no citation)_"
    lines.append(f"**Source of expectation** (`{ck}`):")
    lines.append("")
    lines.append(f"`{citation}`")
    lines.append("")
    lines.append("## Actual")
    lines.append("")
    lines.append(str(ev.get("observed_summary", "")).strip() or "_(no summary)_")
    if ev.get("exit_code") is not None:
        lines.append("")
        lines.append(f"Exit code: `{ev['exit_code']}`")
    if ev.get("stderr_tail"):
        lines.append("")
        lines.append("```")
        lines.append(str(ev["stderr_tail"]).rstrip("\n"))
        lines.append("```")
    lines.append("")
    lines.append("## Environment")
    lines.append("")
    lines.append(f"- slangc commit: `{prov.get('source_commit', '?')}`")
    if finding.get("scope"):
        lines.append(f"- Target / subsystem: {finding['scope']}")
    lines.append("")
    lines.append("## Provenance")
    lines.append("")
    lines.append("Surfaced by agentic test generation.")
    lines.append("")
    lines.append(f"- Test: `{ev.get('source_slang', '?')}`")
    lines.append(f"- Doc anchor: `{prov.get('doc_anchor', '?')}`")
    lines.append(f"- Generation model: `{prov.get('agent_model', '?')}`")
    lines.append(f"- Finding ID: `{finding.get('id', '?')}`")
    if asa.get("confidence"):
        lines.append(f"- Agent self-confidence: {asa['confidence']}")
    lines.append("")
    return "\n".join(lines)


def _gh(args: list[str], capture: bool = True, check: bool = True) -> subprocess.CompletedProcess:
    """Invoke `gh` and return CompletedProcess."""
    cmd = ["gh", *args]
    return subprocess.run(
        cmd,
        cwd=str(REPO_ROOT),
        capture_output=capture,
        text=True,
        check=check,
    )


def _fetch_project_field_ids(owner: str, number: int) -> tuple[str, dict[str, dict]]:
    """Return (project_node_id, {field_name: {id, options:{opt_name: opt_id}}}).

    Resolves the GraphQL node IDs needed by `gh project item-edit`.
    """
    proj = _gh(
        ["project", "view", str(number), "--owner", owner, "--format", "json"]
    )
    pdata = json.loads(proj.stdout)
    project_id = pdata["id"]
    fields_raw = _gh(
        [
            "project",
            "field-list",
            str(number),
            "--owner",
            owner,
            "--format",
            "json",
        ]
    )
    fdata = json.loads(fields_raw.stdout)
    fields: dict[str, dict] = {}
    for f in fdata.get("fields", []):
        entry: dict = {"id": f["id"], "type": f.get("type", ""), "options": {}}
        for opt in f.get("options", []) or []:
            entry["options"][opt["name"]] = opt["id"]
        fields[f["name"]] = entry
    return project_id, fields


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
    if spec.source_doc is None:
        return None
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
    # Workspace-relative path of the document the Anchor cell points at, or
    # "" when the cell carries no resolvable link. This -- not the reporting
    # bundle's `source_doc` -- is the document a gap is a gap *in*, and it is
    # what `doc-gaps` groups by. See `_resolve_gap_target`.
    anchor_target: str = ""
    # Stable identity of this gap, for the consuming ledger. See
    # `compute_gap_id`.
    gap_id: str = ""


_GAP_ANCHOR_FRAG_RE = re.compile(r"(#[A-Za-z0-9][A-Za-z0-9_-]*)")
_GAP_ANCHOR_LINK_RE = re.compile(r"\[[^\]]*\]\((?P<url>[^)\s]+)(?:\s+\"[^\"]*\")?\)")


def _resolve_gap_target(bundle_dir: str, anchor_cell: str) -> str:
    """Return the workspace-relative document an Anchor cell points at.

    A gap is reported by a test bundle but is a defect *in a document*, and
    the two are not the same thing: a `coverage/` bundle has no `source_doc`
    at all, yet every gap it reports names a design page in its Anchor cell.
    Grouping such a row under the bundle leaves it unaddressable by the doc
    that has to fix it, so the link target is resolved here instead.

    The cell reads `[#some-heading](../../../design/pipeline/06-emit.md#some-heading)`;
    the link is relative to the bundle directory, so it is resolved against
    that and made repo-relative. Returns "" if the cell has no link, links
    to a bare fragment, or points outside the repository -- all of which
    `lint_bundle` reports, because a row that names no document cannot be
    routed to one.
    """
    m = _GAP_ANCHOR_LINK_RE.search(anchor_cell)
    if not m:
        return ""
    path, _, _frag = m.group("url").partition("#")
    if not path:
        return ""
    try:
        target = (REPO_ROOT / bundle_dir / unquote(path)).resolve()
        return str(target.relative_to(REPO_ROOT)).replace(os.sep, "/")
    except (ValueError, OSError):
        return ""


def _normalize_gap_prose(text: str) -> str:
    """Reduce a gap's prose to the form its identity is computed from.

    Two agents describing the same gap, or one agent re-describing it on a
    later regeneration, will not reproduce the sentence verbatim -- they
    reflow it, add a code span, or repunctuate. Identity has to survive
    that, or a gap already marked fixed comes back as a new one on the next
    bundle regeneration, which is exactly the churn the ledger exists to
    stop. So markup is unwrapped, punctuation is dropped, whitespace is
    collapsed, and the result is lowercased, leaving the words alone to
    carry the identity.
    """
    text = re.sub(r"`([^`]*)`", r"\1", text)
    text = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"[*_~]", "", text)
    # No un-escaping of `\|` here: the punctuation strip below maps both the
    # backslash and the pipe to spaces and the final collapse erases the
    # difference, so doing it first changes nothing. This is the identity
    # function for `gap_id`, so it is worth keeping free of steps that only
    # look like they matter.
    text = re.sub(r"[^\w\s]", " ", text, flags=re.UNICODE)
    return re.sub(r"\s+", " ", text).strip().lower()


_GAP_ID_LEN = 12


def compute_gap_id(
    anchor_target: str, anchor_fragment: str, kind: str, gap: str
) -> str:
    """Return the stable id a consumer tracks this gap's resolution under.

    Gap rows live in bundle READMEs, which are rewritten wholesale every
    time a bundle is regenerated, so a row has no natural key -- there is
    nothing for a ledger to say "this one is fixed" about. The id supplies
    one, derived from what makes a gap the gap it is: the document section
    it is against, its kind, and its prose (normalized, see
    `_normalize_gap_prose`).

    Deriving the id rather than writing it into the README is deliberate:
    every existing bundle gets an id without being regenerated, and a
    generating agent cannot mint a colliding or malformed one.

    The trade-off is that renaming the anchored heading, or rewriting the
    gap prose to say something materially different, mints a new id and
    retires the old one. That is the intended behaviour for a rewrite; for
    a heading rename it means one stale ledger entry, which `gap-status`
    reports as resolved-but-unseen rather than silently dropping.
    """
    payload = "\n".join(
        [anchor_target, anchor_fragment, kind, _normalize_gap_prose(gap)]
    )
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()[:_GAP_ID_LEN]


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
        anchor_target = _resolve_gap_target(bundle_key, anchor_cell)
        rows.append(
            GapRow(
                anchor=anchor_cell,
                anchor_fragment=anchor_fragment,
                kind=kind_cell,
                gap=gap_cell,
                suggested_addition=suggested_cell,
                bundle=bundle_key,
                source_doc=source_doc,
                anchor_target=anchor_target,
                gap_id=compute_gap_id(
                    anchor_target, anchor_fragment, kind_cell, gap_cell
                ),
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


# --------------------------------------------------------------------------
# Markdown link / anchor resolution
#
# Bundle READMEs and //META doc_ref fields both cite documentation as
# `<path>#<anchor>`. Both halves have to hold up on GitHub: the path has
# to resolve relative to the file that spells it (a bundle nested one
# level deeper than the prompt template's example otherwise silently
# loses a `../`), and the anchor has to match a heading slug in the
# target document.
# --------------------------------------------------------------------------

_MD_LINK_RE = re.compile(r"\[[^\]]*\]\((?P<url>[^)\s]+)(?:\s+\"[^\"]*\")?\)")
_ATX_HEADING_RE = re.compile(r"^#{1,6}\s+(?P<title>.*?)\s*#*\s*$")
_SETEXT_UNDERLINE_RE = re.compile(r"^(?:=+|-+)\s*$")


def _github_slug(title: str) -> str:
    """Return the fragment GitHub generates for a markdown heading.

    GitHub lowercases the text, drops everything that is neither a word
    character, a hyphen, nor a space, and then turns each remaining space
    into a hyphen. Note that the punctuation is dropped rather than
    replaced, so `A / B` becomes `a--b` (two hyphens, one per space) --
    the most common way a hand-written anchor ends up not resolving.
    """
    title = re.sub(r"`([^`]*)`", r"\1", title)
    title = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", title)
    title = re.sub(r"[*_~]", "", title)
    slug = re.sub(r"[^\w\- ]", "", title.strip().lower(), flags=re.UNICODE)
    return slug.replace(" ", "-")


_DOC_SECTION_DIGEST_MOD = None


def _doc_section_digest_module():
    """Return `_meta/doc-section-digest.py` loaded as a module, or None.

    That script is the canonical implementation of the `doc_section_digest`
    rule and is what agents run to produce the value, so the lint has to use
    it rather than re-deriving the rule. The two are not interchangeable:
    the script's `slug` keeps backticks and link syntax, while `_github_slug`
    here strips them for link checking, so a heading like ``## `Foo` bar``
    hashes under a different anchor in each. Importing the one that generated
    the value is what makes the comparison meaningful.

    Returns None when the script is missing, so the lint degrades to skipping
    the check rather than erroring on every test.
    """
    global _DOC_SECTION_DIGEST_MOD
    if _DOC_SECTION_DIGEST_MOD is None:
        path = REPO_ROOT / "docs/generated/tests/_meta/doc-section-digest.py"
        if not path.is_file():
            _DOC_SECTION_DIGEST_MOD = False
        else:
            import importlib.util

            spec = importlib.util.spec_from_file_location("_doc_section_digest", path)
            mod = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(mod)
            _DOC_SECTION_DIGEST_MOD = mod
    return _DOC_SECTION_DIGEST_MOD or None


def lint_doc_section_digests(specs: list) -> list[LintIssue]:
    """Report doc sections whose text has drifted from the tests anchored to them.

    `doc_section_digest` is the sha256 of the cited section's body, so editing a
    section invalidates every test that cites it. Nothing notices today:
    `_REQUIRED_TEST_META_KEYS` only asserts the key is *present*, never that it
    matches, so a doc edit silently leaves those tests claiming to have been
    written against text that no longer exists. All 16 tests anchored to
    `grammar.md#statements` went stale that way when the `ThrowStmt` production
    was corrected, and nothing flagged it.

    Reported per doc section rather than per test, because the section is the
    unit of work: one edit invalidates a whole group, and 386 individual lines
    would bury the ~40 sections that actually drifted.

    Warning, not error. A stale digest does not mean the test is wrong -- it
    means nobody has re-read the claim since the prose moved, which is a review
    task, not a build failure. Note that refreshing the digest without re-reading
    is worse than leaving it stale: it launders the drift and destroys the only
    record that the claim and the prose were ever checked against each other.
    """
    mod = _doc_section_digest_module()
    if mod is None:
        return []
    # (doc target, anchor) -> [test paths]
    stale: dict[tuple[str, str], list[str]] = {}
    for spec in specs:
        if not spec.is_doc_anchored:
            continue
        for tf in sorted((REPO_ROOT / spec.dir).glob("*.slang")):
            meta = parse_test_meta(tf.read_text(encoding="utf-8", errors="replace"))
            recorded = (meta.get("doc_section_digest") or "").strip()
            doc_ref = (meta.get("doc_ref") or "").strip()
            if not recorded or "#" not in doc_ref:
                continue
            target, _, anchor = doc_ref.partition("#")
            doc = REPO_ROOT / target
            if not doc.is_file():
                continue
            try:
                body = mod.section_text(str(doc), anchor)
            except SystemExit:
                # Unresolvable anchor; already reported by the per-file lint.
                continue
            if hashlib.sha256(body.encode("utf-8")).hexdigest() != recorded:
                stale.setdefault((target, anchor), []).append(
                    str(tf.relative_to(REPO_ROOT))
                )
    issues: list[LintIssue] = []
    for (target, anchor), tests in sorted(stale.items()):
        sample = ", ".join(Path(t).name for t in tests[:3])
        more = f", +{len(tests) - 3} more" if len(tests) > 3 else ""
        issues.append(
            LintIssue(
                f"{target}#{anchor}",
                "warning",
                f"section changed since {len(tests)} test(s) were anchored to it"
                f" ({sample}{more}). Re-read those claims against the current"
                f" text, then refresh their //META doc_section_digest with"
                f" `_meta/doc-section-digest.py {target} {anchor}`.",
            )
        )
    return issues


CATALOG_SNAPSHOT_PATH = META_DIR / "diagnostics-catalog/catalog.txt"


def parse_catalog_snapshot(path: Path | None = None) -> dict[str, tuple[str, str, str]]:
    """Return the catalog snapshot as `code -> (severity, name, message)`.

    `catalog.txt` is the committed, tab-separated extraction of every
    diagnostic the compiler defines, with the columns named in its own header:
    `code, covered, severity, name, source, message`. The message is the
    remainder of the line rather than one field, because a message may itself
    contain a tab.

    Rows are keyed by code because that is the identity a catalog test records
    (`//META: catalog_code`) and the one that survives a rename.
    """
    rows: dict[str, tuple[str, str, str]] = {}
    src = path or CATALOG_SNAPSHOT_PATH
    if not src.is_file():
        return rows
    for line in src.read_text(encoding="utf-8").splitlines():
        if line.startswith("#") or not line.strip():
            continue
        parts = line.split("\t")
        if len(parts) < 6:
            continue
        code, _covered, severity, name = parts[0], parts[1], parts[2], parts[3]
        message = "\t".join(parts[5:])
        rows[code] = (severity, name, message)
    return rows


def catalog_entry_digest(code: str, severity: str, name: str, message: str) -> str:
    """Return the `doc_section_digest` for one catalog entry.

    The catalog bundle's tests cite a diagnostic definition rather than a doc
    section, so the digest pins the entry's rendered identity -- code,
    severity, name and message joined by tabs -- instead of a section body.
    Editing a diagnostic's message or renaming it changes the digest, which is
    what lets `lint_catalog_entry_digests` tell that a test was written against
    text that has since moved.

    This is the rule stated in the bundle's README; it lives here so that lint
    and the generation prompt share one implementation rather than each
    re-deriving it. `regenerate.py catalog-digest <code>` prints it.
    """
    return hashlib.sha256(
        f"{code}\t{severity}\t{name}\t{message}".encode("utf-8")
    ).hexdigest()


def lint_catalog_entry_digests(
    specs: list,
    root: Path | None = None,
    snapshot: Path | None = None,
) -> list[LintIssue]:
    """Report catalog tests whose cited diagnostic has drifted from them.

    The counterpart to `lint_doc_section_digests` for the one bundle that
    cites source definitions instead of anchored prose. That function requires
    a `#anchor` to locate a section body, and a catalog `doc_ref` has none, so
    until now every catalog digest went unchecked -- which is what #11410
    observed, first as all-zero placeholders and then, after the corpus was
    regenerated, as well-formed values nothing recomputed.

    Warning, not error, for the same reason as the doc-section check: a
    mismatch means nobody has re-read the claim since the diagnostic moved,
    which is a review task, not a build failure. Refreshing the digest without
    re-reading launders the drift and destroys the only record that the test
    and the diagnostic were ever checked against each other.

    A code that is missing from the snapshot entirely is reported once for the
    whole group rather than per test, because the cause is shared: either the
    snapshot predates those diagnostics or they have been removed, and the
    answer in both cases is to re-extract `catalog.txt`.

    `root` and `snapshot` default to the real tree and are parameters only so
    that `selftest` can run the drift and unknown-code branches against a
    fixture, which the committed corpus cannot produce.
    """
    root = root or REPO_ROOT
    snapshot = snapshot or CATALOG_SNAPSHOT_PATH
    catalog = parse_catalog_snapshot(snapshot)
    if not catalog:
        # An absent or empty snapshot must not read as "nothing to report". Every
        # digest is checked by looking its code up in here, so returning silently
        # would leave the whole bundle unverified while lint claims success --
        # the same unnoticed-unverified state #11410 was filed about, reached by
        # deleting a file instead of by skipping a code path. The unknown-code
        # warning below cannot cover this: with no rows there are no lookups, so
        # it never fires.
        unchecked = [spec for spec in specs if spec.is_diagnostics_catalog]
        if not unchecked:
            return []
        count = sum(len(list((root / spec.dir).glob("*.slang"))) for spec in unchecked)
        return [
            LintIssue(
                _rel_to_repo(snapshot),
                "warning",
                f"catalog snapshot is missing or empty, so the doc_section_digest"
                f" of {count} catalog test(s) went unchecked. Re-extract it:"
                f" without it this lint passes whatever those tests record",
            )
        ]
    issues: list[LintIssue] = []
    unknown: list[tuple[str, str]] = []
    for spec in specs:
        if not spec.is_diagnostics_catalog:
            continue
        for tf in sorted((root / spec.dir).glob("*.slang")):
            meta = parse_test_meta(tf.read_text(encoding="utf-8", errors="replace"))
            recorded = (meta.get("doc_section_digest") or "").strip()
            code = (meta.get("catalog_code") or "").strip()
            if not recorded or not code:
                continue
            rel = str(tf.relative_to(root))
            entry = catalog.get(code)
            if entry is None:
                unknown.append((code, rel))
                continue
            if catalog_entry_digest(code, *entry) != recorded:
                issues.append(
                    LintIssue(
                        rel,
                        "warning",
                        f"catalog entry {code} changed since this test was"
                        " anchored to it. Re-read the claim against the"
                        " current entry, then refresh //META"
                        " doc_section_digest with `_meta/regenerate.py"
                        f" catalog-digest {code}`",
                    )
                )
    if unknown:
        codes = ", ".join(sorted({c for c, _ in unknown}, key=_code_sort_key))
        issues.append(
            LintIssue(
                _rel_to_repo(snapshot),
                "warning",
                f"{len(unknown)} test(s) cite catalog code(s) the snapshot does"
                f" not contain ({codes}). Either the snapshot predates those"
                " diagnostics and needs re-extracting, or the codes were"
                " removed and their tests are now testing nothing",
            )
        )
    return issues


def _code_sort_key(code: str):
    """Sort diagnostic codes numerically when they are numeric."""
    return (0, int(code)) if code.isdigit() else (1, code)


_HEADING_SLUG_CACHE: dict[Path, set[str]] = {}


def heading_slugs(doc: Path) -> set[str]:
    """Return every anchor GitHub would generate for `doc`'s headings.

    Handles both ATX (`## Title`) and setext (`Title` over `-----`)
    headings, skips fenced code blocks, and appends `-1`, `-2`, ... to
    repeated slugs the way GitHub disambiguates them.
    """
    cached = _HEADING_SLUG_CACHE.get(doc)
    if cached is not None:
        return cached
    slugs: set[str] = set()
    counts: dict[str, int] = {}
    try:
        lines = doc.read_text(encoding="utf-8").split("\n")
    except OSError:
        _HEADING_SLUG_CACHE[doc] = slugs
        return slugs
    in_fence = False
    for i, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith("```") or stripped.startswith("~~~"):
            in_fence = not in_fence
            continue
        if in_fence:
            continue
        m = _ATX_HEADING_RE.match(line)
        if m:
            title = m.group("title")
        elif (
            _SETEXT_UNDERLINE_RE.match(line)
            and i > 0
            and lines[i - 1].strip()
            and lines[i - 1].lstrip()[:1] not in ("|", ">", "#", "-", "=")
        ):
            title = lines[i - 1].strip()
        else:
            continue
        slug = _github_slug(title)
        if not slug:
            continue
        n = counts.get(slug, 0)
        counts[slug] = n + 1
        slugs.add(slug if n == 0 else f"{slug}-{n}")
    _HEADING_SLUG_CACHE[doc] = slugs
    return slugs


def lint_markdown_links(md_path: Path) -> list[LintIssue]:
    """Check that every relative link in `md_path` resolves.

    A link whose path does not exist is an error: it renders as a dead
    link on GitHub, and the usual cause is a `../` miscount, which no
    other check catches. A link whose path resolves but whose `#anchor`
    matches no heading in the target is a warning -- it lands the reader
    on the right document but the wrong place, and repairing it needs a
    judgement call about which section was meant.
    """
    issues: list[LintIssue] = []
    rel = _rel_to_repo(md_path)
    try:
        text = md_path.read_text(encoding="utf-8")
    except OSError:
        return issues
    # A coverage table cites the same anchor once per claim row, so report
    # each distinct problem once rather than once per citation.
    reported: set[str] = set()
    for m in _MD_LINK_RE.finditer(text):
        url = m.group("url")
        if url.startswith(("http://", "https://", "mailto:", "#", "<")):
            continue
        path, _, fragment = url.partition("#")
        if not path:
            continue
        if url in reported:
            continue
        target = (md_path.parent / unquote(path)).resolve()
        if not target.exists():
            reported.add(url)
            issues.append(
                LintIssue(rel, "error", f"link path does not resolve: {url}")
            )
            continue
        if fragment and target.suffix == ".md":
            if unquote(fragment) not in heading_slugs(target):
                reported.add(url)
                issues.append(
                    LintIssue(
                        rel,
                        "warning",
                        f"link anchor matches no heading in {path}: #{fragment}",
                    )
                )
    return issues


# --------------------------------------------------------------------------
# Markdown table shape
#
# A bundle README is almost entirely tables, and a claim is free text that
# can contain a `|` -- an operator (`isKhronosTarget || HLSL`), a union
# type, a shell pipe. Unescaped, it opens a new cell. GitHub only renders
# a table at all when its delimiter row has exactly as many cells as its
# header, so one stray `|` in a claim can drop a 30-row coverage table on
# the floor and render it as a wall of pipe characters.
# --------------------------------------------------------------------------

_UNESCAPED_PIPE_RE = re.compile(r"(?<!\\)\|")
_DELIM_CELL_RE = re.compile(r"^:?-+:?$")


def _is_delimiter_row(line: str) -> bool:
    """Return whether `line` is a table's delimiter row.

    GFM accepts a single dash per cell (`| - | - |`) and a one-column
    table, so this tests each cell rather than matching the row against
    one pattern -- a stricter pattern would skip such a table entirely
    and quietly leave it unvalidated.
    """
    cells = _row_cells(line)
    return bool(cells) and all(_DELIM_CELL_RE.match(c.strip()) for c in cells)


def _row_cells(line: str) -> list[str]:
    """Split one markdown table row into cells on its unescaped pipes.

    A `\\|` is content rather than a cell boundary, which is how a claim
    spells a literal pipe, so only bare `|` separates. The leading and
    trailing pipes that delimit the row are not separators and are
    dropped.
    """
    parts = _UNESCAPED_PIPE_RE.split(line.strip())
    if parts and not parts[0].strip():
        parts = parts[1:]
    if parts and not parts[-1].strip():
        parts = parts[:-1]
    return parts


def lint_markdown_tables(md_path: Path) -> list[LintIssue]:
    """Check that every markdown table in `md_path` renders on GitHub.

    A delimiter row whose cell count differs from its header's is an
    error, because GitHub then does not recognize the block as a table
    at all and renders the raw pipes. A body row that disagrees with the
    header is also an error: GitHub pads or truncates it, so a claim
    containing an unescaped `|` silently loses its trailing columns --
    which is how a coverage row can end up with no Tests link.

    Fenced code blocks are skipped, the same way `heading_slugs` skips
    them: a bundle README quotes example tables inside ``` fences, and
    those are illustrations rather than tables GitHub will render.
    """
    issues: list[LintIssue] = []
    rel = _rel_to_repo(md_path)
    try:
        lines = md_path.read_text(encoding="utf-8").split("\n")
    except OSError:
        return issues
    i = 0
    in_fence = False
    while i < len(lines):
        stripped = lines[i].lstrip()
        if stripped.startswith("```") or stripped.startswith("~~~"):
            in_fence = not in_fence
            i += 1
            continue
        header = lines[i]
        if in_fence or not header.lstrip().startswith("|") or i + 1 >= len(lines):
            i += 1
            continue
        if not _is_delimiter_row(lines[i + 1]):
            i += 1
            continue
        width = len(_row_cells(header))
        delim_width = len(_row_cells(lines[i + 1]))
        if delim_width != width:
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    f"table at line {i + 1} does not render: header has"
                    f" {width} cells but its delimiter row has {delim_width}",
                )
            )
        i += 2
        while i < len(lines) and lines[i].lstrip().startswith("|"):
            cells = _row_cells(lines[i])
            if len(cells) != width:
                # An extra cell that carries text came from a `|` inside
                # the content; an extra empty one is just a stray
                # delimiter. Naming the right cause saves the reader a
                # hunt through a 400-character claim.
                hint = ""
                if len(cells) > width:
                    hint = (
                        " (escape a literal pipe in cell text as `\\|`)"
                        if any(c.strip() for c in cells[width:])
                        else " (stray trailing delimiter)"
                    )
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        f"table row at line {i + 1} has {len(cells)} cells,"
                        f" expected {width}{hint}",
                    )
                )
            i += 1
    return issues


_LIQUID_OPENER_RE = re.compile(r"\{\{|\{%")


def lint_liquid_safe(md_path: Path) -> list[LintIssue]:
    """Check that a bundle README carries no raw Liquid opener (`{{` or `{%`).

    These READMEs have YAML front-matter, so the GitHub Pages Jekyll build
    treats each as a page and runs Liquid over its body -- before Markdown
    conversion, so a `{{` inside a code span or fence is still seen as a
    Liquid output tag. The failure mode depends on what follows the `{{`.
    In `float2x2 m = {{1,2},{3,4}}`, Liquid's non-greedy scan for the closing
    `}}` stops at the first single `}` (after `1,2`), so the tag is never
    terminated -- Liquid raises "Variable ... was not properly terminated"
    and the whole site build aborts. A properly terminated `{{...}}` (e.g. a
    FileCheck `{{.*}}` wildcard) parses instead and is evaluated as a
    template expression -- usually rendering empty -- so the literal text
    does not survive to the published page. Either way the sequence must not
    reach Liquid.

    The README is also read on github.com, where Liquid is not processed, so
    the fix cannot be a `{% raw %}` wrapper or an HTML entity inside a
    backtick span (both show literally there). Two dual-safe spellings pass:
    space the braces where whitespace is semantically irrelevant
    (`{ {1,2},{3,4} }`), or show an exact token with numeric entities inside
    a raw `<code>` element (`<code>&#123;&#123;.*&#125;&#125;</code>`), which
    renders as `{{.*}}` on both surfaces while never presenting a literal
    `{{` to Liquid.

    Front-matter is excluded: Jekyll strips it before Liquid runs, so its
    contents never reach the Liquid renderer.
    """
    issues: list[LintIssue] = []
    rel = _rel_to_repo(md_path)
    try:
        text = md_path.read_text(encoding="utf-8")
    except OSError:
        return issues
    fm = _FM_RE.match(text)
    fm_end_line = text.count("\n", 0, fm.end()) if fm else 0
    for lineno, line in enumerate(text.split("\n"), start=1):
        if lineno <= fm_end_line:
            continue
        if _LIQUID_OPENER_RE.search(line):
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    f"line {lineno} has a raw Liquid opener (`{{{{` or `{{%`)"
                    f" that breaks the GitHub Pages build; space the braces"
                    f" (`{{ {{1,2}} }}`) or show the exact token as"
                    f" `<code>&#123;&#123;...&#125;&#125;</code>`",
                )
            )
    return issues


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


# --------------------------------------------------------------------------
# Emission fan-out gate
#
# Enforcement of `_claims.md §2` "Meaningful back-ends": a target-dependent
# (emission) claim must be exercised on every feasible text-emit target, or
# account for each absent target with an Untested row. The rule existed only
# as prose when the 2026-06 doc-scope regen silently dropped it, deleting
# ~985 emission tests and dropping coverage 53.10% -> 51.77% undetected.
# Making it a lint check is what turns the rule from advisory into a gate.
#
# NOTE: rolled out as "warning" first. Flip _FANOUT_SEVERITY to "error" once
# the existing suite's violations are fixed or opted out — that flip is the
# moment the regression becomes impossible to reland.
# --------------------------------------------------------------------------
_FANOUT_SEVERITY = "warning"

# All text-emit targets (used to detect which targets a test exercises and
# which an Untested opt-out row names).
_EMIT_TEXT_TARGETS = frozenset(
    {"hlsl", "glsl", "spirv-asm", "metal", "wgsl", "cuda", "cpp"}
)
# Shader text targets. The fan-out gate enforces these as a *set*: a claim
# emitted to any one of them must cover them all (or opt out per target) —
# this is the regression class (shader emission breadth). cpp/cuda are
# general-purpose backends with different semantics and are NOT force-
# required; a claim emitted only to cpp/cuda is backend-specific (e.g. the
# C++ prelude/host-shim) and is not subject to the shader fan-out rule.
_SHADER_TEXT_TARGETS = frozenset({"hlsl", "glsl", "spirv-asm", "metal", "wgsl"})
# Untested-claims reasons that legitimately account for an absent target.
_FANOUT_OPTOUT_REASONS = frozenset(
    {"unsupported-on-target", "out-of-bundle"}
    | {r for r in _ALLOWED_OOS_REASONS if r.startswith("gpu-")}
)
_SIMPLE_TARGET_RE = re.compile(r"//TEST:SIMPLE\([^)]*\):([^\n]*)")
_TARGET_FLAG_RE = re.compile(r"-target\s+(\S+)")


def _norm_target(t: str) -> str:
    """Normalize a -target token to its text-emit spelling."""
    return {"spirv": "spirv-asm", "dxil": "dxil-asm"}.get(t, t)


def _emission_targets(text: str) -> set[str]:
    """Text-emit targets named across a test file's //TEST:SIMPLE directives."""
    out: set[str] = set()
    for line in _SIMPLE_TARGET_RE.findall(text):
        for t in _TARGET_FLAG_RE.findall(line):
            t = _norm_target(t)
            if t in _EMIT_TEXT_TARGETS:
                out.add(t)
    return out


def _lint_emission_fanout(
    spec: "BundleSpec", readme_text: str, test_files: list[Path]
) -> list[LintIssue]:
    """An emission claim must cover every feasible text-emit target, or
    account for each absent target with an Untested `unsupported-on-target`
    (or gpu-*/out-of-bundle) row whose Claim cell matches the claim.
    """
    issues: list[LintIssue] = []
    # target-pipelines/* bundles are single-target *by design* — exempt.
    if spec.dir.startswith("docs/generated/tests/design/target-pipelines/"):
        return issues

    # Group tests by claim (//META: purpose, which the README Claim cell
    # must match verbatim); collect each claim's covered emit targets.
    claim_targets: dict[str, set[str]] = {}
    claim_is_emission: dict[str, bool] = {}
    for tf in test_files:
        t = tf.read_text(encoding="utf-8")
        meta = parse_test_meta(t)
        purpose = meta.get("purpose", "").strip()
        if not purpose:
            continue
        tgts = _emission_targets(t)
        claim_targets.setdefault(purpose, set()).update(tgts)
        # Emission/target-dependent iff stamped emit AND it emits to a target.
        if meta.get("pipeline_stage") == "emit" and tgts:
            claim_is_emission[purpose] = True

    # Opt-out targets declared per claim in the Untested-claims table.
    optout: dict[str, set[str]] = {}
    if "## Untested claims" in readme_text:
        for row in _parse_tagged_table(readme_text, "## Untested claims"):
            claim_cell, reason, _anchor, why = row
            if reason not in _FANOUT_OPTOUT_REASONS:
                continue
            blob = claim_cell + " " + why
            named = {_norm_target(m) for m in _TARGET_FLAG_RE.findall(blob)}
            named |= {w for w in blob.split() if w in _EMIT_TEXT_TARGETS}
            optout.setdefault(claim_cell.strip(), set()).update(named)

    for purpose, covered in claim_targets.items():
        if not claim_is_emission.get(purpose):
            continue  # not an emission claim — fan-out rule does not apply
        if not (covered & _SHADER_TEXT_TARGETS):
            continue  # cpp/cuda-only: backend-specific, shader rule N/A
        missing = _SHADER_TEXT_TARGETS - covered - optout.get(purpose, set())
        if missing:
            issues.append(
                LintIssue(
                    spec.dir,
                    _FANOUT_SEVERITY,
                    f"emission claim {purpose!r} covers shader targets"
                    f" {sorted(covered & _SHADER_TEXT_TARGETS)} but is missing"
                    f" {sorted(missing)}; add an emission test per target or an"
                    f" Untested 'unsupported-on-target' row naming each absent"
                    f" target (_claims.md §2 Meaningful back-ends)",
                )
            )
    return issues


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
        required_fm_keys = _REQUIRED_BUNDLE_FM_KEYS
        if not spec.is_doc_anchored:
            # A coverage bundle has no source_doc, so its README must not
            # claim one; provenance rests on watched_paths_digest alone.
            required_fm_keys = tuple(
                k
                for k in required_fm_keys
                if k not in ("source_doc", "source_doc_digest")
            )
        for k in required_fm_keys:
            if k not in fm:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "error",
                        f"front-matter missing key: {k}",
                    )
                )
        if not spec.is_doc_anchored and "source_doc" in fm:
            issues.append(
                LintIssue(
                    f"{spec.dir}/README.md",
                    "error",
                    "front-matter declares source_doc, but this is a"
                    " gap-selected coverage bundle with no source document",
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

    issues.extend(lint_markdown_links(bundle_md))
    issues.extend(lint_markdown_tables(bundle_md))
    issues.extend(lint_liquid_safe(bundle_md))

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
    # Claim enumeration: `prompts/_claims.md` §4 makes `## Claims` the bundle's
    # ground-truth list, and §6 defines completeness as "every claim there is
    # either covered or classified". Without it a bundle has no way to show a
    # claim as *missing*: `## Functional coverage` only lists claims that
    # already have tests, so when the source doc gains new normative prose
    # nothing reports the shortfall. That is how the doc-gap fill (#12477) added
    # ~12k lines of newly checkable statements without a single bundle showing a
    # coverage gap.
    #
    # Warning rather than error: most bundles predate the requirement, and an
    # error would block every unrelated change until all of them are
    # re-enumerated.
    if "## Claims" not in text:
        issues.append(
            LintIssue(
                f"{spec.dir}/README.md",
                "warning",
                "no ## Claims section; the bundle has no ground-truth claim"
                " enumeration, so new normative prose in its source doc cannot"
                " surface as missing coverage (prompts/_claims.md §1, §4)",
            )
        )

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
        gap_rows = parse_gap_rows(text, spec.dir, spec.gap_group)
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
            # An Anchor cell that names no document is the one gap defect
            # that costs the row its whole purpose: `doc-gaps` groups by
            # the anchored document, so a row without one is filed under
            # the reporting bundle and never reaches a doc author. The
            # link's *path* is checked here; `lint_markdown_links` already
            # covers whether the path and its fragment resolve, so this
            # does not restate that.
            if not row.anchor_target:
                issues.append(
                    LintIssue(
                        f"{spec.dir}/README.md",
                        "error",
                        f"doc-gap row Anchor cell has no link to a document,"
                        f" so the gap cannot be routed to one:"
                        f" {row.anchor!r}",
                    )
                )

    # Emission fan-out gate (enforces _claims.md §2 "Meaningful back-ends").
    issues.extend(_lint_emission_fanout(spec, text, test_files))
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
    required_meta_keys = _REQUIRED_TEST_META_KEYS
    if not spec.is_doc_anchored:
        # A coverage test is chosen by uncovered code, not by a documented
        # claim, so it cannot be required to cite one. Most still carry a
        # doc_ref pointing at the nearest related design section for
        # orientation, and when they do it is validated below like any other.
        required_meta_keys = tuple(
            k
            for k in required_meta_keys
            if k not in ("doc_ref", "doc_section_digest")
        )
    for k in required_meta_keys:
        if k not in meta:
            issues.append(LintIssue(rel, "error", f"//META missing key: {k}"))
    # coverage/METHODOLOGY.md § Per-test contract: a coverage test names the
    # source area it characterizes in place of the doc_ref it does not have,
    # so `covers=` is what makes it traceable at all.
    if not spec.is_doc_anchored and not meta.get("covers", "").strip():
        issues.append(
            LintIssue(
                rel,
                "error",
                "//META missing key: covers (required for coverage bundles —"
                " see coverage/METHODOLOGY.md § Per-test contract)",
            )
        )
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
    # `characterization` asserts what the compiler currently does rather than
    # what a document claims, so it is only answerable inside the gap-selected
    # coverage tree. Allowing it in a doc-anchored bundle would let a test
    # escape the requirement that it cite a documented claim.
    if intent == "characterization" and spec.is_doc_anchored:
        issues.append(
            LintIssue(
                rel,
                "error",
                "//META intent='characterization' is only valid in a"
                f" coverage bundle, but {spec.key!r} is doc-anchored",
            )
        )
    requires_tool = meta.get("requires-tool", "").strip()
    if requires_tool:
        toks = [t.strip() for t in requires_tool.split(",") if t.strip()]
        bad = [t for t in toks if t not in _TOOL_BINARY]
        if bad:
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    f"//META requires-tool has unknown value(s) {bad!r};"
                    f" allowed: {sorted(_TOOL_BINARY)}",
                )
            )
    doc_ref = meta.get("doc_ref", "")
    if doc_ref:
        target, _, anchor = doc_ref.partition("#")
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
            elif anchor and candidate.suffix == ".md":
                if anchor not in heading_slugs(candidate):
                    issues.append(
                        LintIssue(
                            rel,
                            "warning",
                            f"//META doc_ref anchor matches no heading in"
                            f" {target}: #{anchor}",
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
    # Diagnostic tests (//DIAGNOSTIC_TEST) must pin the diagnostic
    # to source either by error-code substring or by a position-based
    # caret annotation. A pure message-text substring check would
    # silently pass through a diagnostic renumbering AND through a
    # message-text rewording AND through a misattribution to the
    # wrong source line; we want at least one of those three failure
    # modes to be caught at lint time.
    #
    # Accept any of:
    #   - `E####` / `W####` / `N####` / `I####` in any CHECK line
    #     (single-line or block) — pins the code explicitly.
    #   - A caret-anchored CHECK line containing `^` — pins source
    #     position. Position-based annotations are more precise than
    #     substring text and are what slang-test's "Suggested
    #     annotations" output produces.
    #   - For diagnostics-catalog bundles, a bare numeric (slang codes
    #     emit without an `E` prefix in some legacy paths).
    if "//DIAGNOSTIC_TEST" in text:
        has_prefixed_code = bool(
            re.search(r"CHECK[^:]*:\s*[^\n]*\b[EWNI]\d+\b", text)
        )
        has_block_code = False
        for m in re.finditer(r"/\*\s*CHECK:\s*(.*?)\*/", text, re.DOTALL):
            if re.search(r"\b[EWNI]\d+\b", m.group(1)):
                has_block_code = True
                break
        # Position-anchored CHECK lines (single-line or block) — a
        # caret `^` inside a CHECK comment ties the annotation to a
        # specific source column, which catches the line-misattribution
        # failure mode the code requirement is meant to catch.
        has_caret_position = bool(
            re.search(r"(?:^|\n)\s*//CHECK[^:]*:[^\n]*\^", text)
        ) or bool(
            re.search(r"/\*\s*CHECK[^:]*:.*?\^.*?\*/", text, re.DOTALL)
        )
        # Catalog bundle tests use bare numeric for some codes.
        has_bare_numeric = spec.is_diagnostics_catalog and bool(
            re.search(r"CHECK[^:]*:\s*\d{4,}\b", text)
        )
        if not (
            has_prefixed_code
            or has_block_code
            or has_caret_position
            or has_bare_numeric
        ):
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    "DIAGNOSTIC_TEST file does not pin the diagnostic to"
                    " source: needs either an explicit code (E####/W####)"
                    " or a caret-anchored CHECK line. Pure message-text"
                    " checks would silently pass through a diagnostic"
                    " renumbering, rewording, or line-misattribution",
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
    # A coverage bundle has no source_doc, so only its watched paths can
    # invalidate it; skip the document checks rather than reading the absent
    # doc as "missing on disk".
    if spec.is_doc_anchored:
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
    print(f"prompt:          {spec.prompt}")
    print(f"role:            {spec.role}")
    if spec.source_doc is None:
        print("source_doc:      (none — gap-selected coverage bundle)")
    else:
        print(f"source_doc:      {spec.source_doc}")
        doc_path = REPO_ROOT / spec.source_doc
        print(
            f"source_doc on disk: {'present' if doc_path.exists() else 'MISSING'}"
        )
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
    if spec.is_doc_anchored and cur_doc is None:
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


def cmd_index(args: argparse.Namespace) -> int:
    """Emit docs/generated/tests/INDEX.md content to stdout (or --write to file)."""
    from collections import Counter

    manifest = load_manifest()
    specs = sorted(manifest.bundles.values(), key=lambda s: s.dir)

    # Per-bundle test counts
    bundle_counts: dict[str, int] = {}
    intent_counts: Counter[str] = Counter()
    for spec in specs:
        bdir = REPO_ROOT / spec.dir
        if not bdir.exists():
            bundle_counts[spec.key] = 0
            continue
        slangs = sorted(bdir.glob("*.slang"))
        bundle_counts[spec.key] = len(slangs)
        for tf in slangs:
            meta = parse_test_meta(tf.read_text(encoding="utf-8"))
            intent = meta.get("intent", "")
            if intent:
                intent_counts[intent] += 1

    total_tests = sum(bundle_counts.values())
    total_bundles = sum(1 for c in bundle_counts.values() if c >= 0 or True)
    # Bundles that exist on disk (have at least 0 .slang files, including
    # bundles that have zero tests — count them too).
    existing_bundles = sum(
        1 for spec in specs if (REPO_ROOT / spec.dir).exists()
    )

    # Section assignment by path prefix.
    section_for = {
        "conformance/": "Conformance (language reference)",
        "design/pipeline/": "Pipeline",
        "design/syntax-reference/": "Syntax reference",
        "design/cross-cutting/": "Cross-cutting",
        "design/ast-reference/": "AST reference",
        "design/name-resolution/": "Name resolution",
        "design/ir-reference/": "IR reference",
        "design/target-pipelines/": "Target pipelines",
        "coverage/": "Coverage (white-box characterization)",
    }
    section_order = [
        "Conformance (language reference)",
        "Pipeline",
        "Syntax reference",
        "Cross-cutting",
        "AST reference",
        "Name resolution",
        "IR reference",
        "Target pipelines",
        "Coverage (white-box characterization)",
        "Top-level",
    ]

    grouped: dict[str, list] = {s: [] for s in section_order}
    for spec in specs:
        section = "Top-level"
        for prefix, sname in section_for.items():
            if spec.key.startswith(prefix):
                section = sname
                break
        grouped[section].append(spec)

    out: list[str] = []
    out.append("# docs/generated/tests — bundle index")
    out.append("")
    out.append(
        "Navigational table of contents for every bundle in `docs/generated/tests/`. Each row links"
    )
    out.append(
        "to that bundle's `README.md` and to the documentation file that anchors its tests."
    )
    out.append("")
    out.append(
        "See [`README.md`](README.md) for the framework intro and the trust model."
    )
    out.append(
        "See [`_meta/regenerate.md`](_meta/regenerate.md) for the operator workflow."
    )
    out.append("")
    out.append("## Suite totals")
    out.append("")
    out.append(f"- **Bundles:** {existing_bundles}")
    out.append(f"- **Total `.slang` tests:** {total_tests}")
    out.append("")
    out.append("| Intent | Count |")
    out.append("| --- | --- |")
    for intent, n in intent_counts.most_common():
        out.append(f"| `{intent}` | {n} |")
    out.append("")
    out.append("## Bundles by section")
    out.append("")
    for section in section_order:
        if not grouped[section]:
            continue
        out.append(f"### {section}")
        out.append("")
        out.append("| Bundle | Tests | Source doc |")
        out.append("| --- | ---: | --- |")
        for spec in grouped[section]:
            count = bundle_counts.get(spec.key, 0)
            if spec.source_doc is None:
                # A coverage bundle is gap-selected, so there is no anchoring
                # document to link; point at the methodology that governs the
                # whole tree instead.
                anchor = "[`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md)"
            else:
                # Relative link from INDEX.md (in docs/generated/tests/) to the
                # source_doc (workspace-relative). Handles both source roots:
                #   docs/generated/design/<...>  -> ../design/<...>
                #   docs/language-reference/<...> -> ../../language-reference/<...>
                doc_link = os.path.relpath(
                    spec.source_doc, "docs/generated/tests"
                )
                anchor = f"[`{spec.source_doc}`]({doc_link})"
            out.append(
                f"| [`{spec.key}`]({spec.key}/README.md) | {count} | {anchor} |"
            )
        out.append("")
    out.append("## Catalog snapshot")
    out.append("")
    out.append(
        "- [`_meta/diagnostics-catalog/catalog.txt`](_meta/diagnostics-catalog/catalog.txt)"
        " — full diagnostic-code catalog consumed by the"
        " `cross-cutting/diagnostics-catalog` bundle."
    )
    out.append("")
    out.append("## Conventions")
    out.append("")
    out.append(
        "- Every bundle's `README.md` carries YAML front-matter (`generated_at`,"
        " `source_commit`, `watched_paths_digest`, `source_doc_digest`) and four"
        " canonical sections: `## Intent`, `## Functional coverage`,"
        " `## Untested claims`, `## Doc gaps observed`."
    )
    out.append(
        "- Each `.slang` test file starts with a `//META` block declaring `doc_ref`,"
        " `intent`, `pipeline_stage`, and provenance."
    )
    out.append(
        "- Bundles are agent-generated. Hand-editing a `README.md` or a `.slang` file is"
        " an anti-pattern — file a doc-improvement or prompt-improvement task and regenerate."
    )
    out.append(
        "- Regenerate this file with `python3 docs/generated/tests/_meta/regenerate.py index"
        " --write`."
    )
    out.append("")

    text = "\n".join(out)
    if args.write:
        target = REPO_ROOT / "docs/generated/tests" / "INDEX.md"
        target.write_text(text)
        print(f"wrote: {target}")
    else:
        print(text)
    return 0


def cmd_lint(args: argparse.Namespace) -> int:
    manifest = load_manifest()
    specs = _bundles_arg(manifest, args.bundles)
    issues: list[LintIssue] = []
    for s in specs:
        issues.extend(lint_bundle(s))
    # Findings live under _meta/findings/; lint them whenever the bundle
    # set isn't narrowed to a specific list (i.e. global lint runs).
    if not args.bundles:
        issues.extend(lint_findings())
        issues.extend(lint_expected_failures())
        issues.extend(lint_agentic_coverage_excludes())
        issues.extend(lint_doc_section_digests(specs))
        issues.extend(lint_catalog_entry_digests(specs))
    errors = [i for i in issues if i.severity == "error"]
    warnings = [i for i in issues if i.severity == "warning"]
    for i in issues:
        print(f"{i.severity:7s} {i.where}: {i.message}")
    print(
        f"\nlint summary: {len(errors)} errors, {len(warnings)} warnings"
        f" across {len(specs)} bundles"
    )
    return 1 if errors else 0


# --------------------------------------------------------------------------
# verify — run slang-test against bundles and report pass/fail/ignored
# --------------------------------------------------------------------------


def _find_slang_test() -> Path | None:
    """Locate a slang-test binary the agent can run locally.

    Searches the standard build-output directories in preference order
    (Release > RelWithDebInfo > Debug — Release matches the nightly
    invocation in `_common.md`). Returns None if nothing is found; the
    caller prints a build hint.
    """
    repo_root = REPO_ROOT
    candidates = [
        repo_root / "build" / "Release" / "bin" / "slang-test",
        repo_root / "build" / "Release" / "bin" / "slang-test.exe",
        repo_root / "build" / "RelWithDebInfo" / "bin" / "slang-test",
        repo_root / "build" / "RelWithDebInfo" / "bin" / "slang-test.exe",
        repo_root / "build" / "Debug" / "bin" / "slang-test",
        repo_root / "build" / "Debug" / "bin" / "slang-test.exe",
    ]
    for p in candidates:
        if p.is_file() and os.access(p, os.X_OK):
            return p
    return None


def _which(tool: str) -> bool:
    """Return True if `tool` is on PATH and executable."""
    import shutil as _shutil
    return _shutil.which(tool) is not None


# Mapping from a `requires-tool` value to the binary name to look for
# on PATH. Keep this in sync with the table in `_common.md` under
# `### \`requires-tool\``.
_TOOL_BINARY = {
    "dxc": "dxc",
    "fxc": "fxc",
    "nvrtc": "nvcc",
    "metal-toolchain": "metal",
    "spirv-tools": "spirv-val",
    "tint": "tint",
}


def _missing_required_tools(test_file_meta: dict[str, str]) -> list[str]:
    """For a parsed //META block, return the list of declared
    requires-tool values whose backing binary is not on PATH.
    Empty list when the test has no requires-tool or all tools are
    present.
    """
    raw = test_file_meta.get("requires-tool", "").strip()
    if not raw:
        return []
    out: list[str] = []
    for tok in [t.strip() for t in raw.split(",") if t.strip()]:
        bin_name = _TOOL_BINARY.get(tok)
        if bin_name is None:
            # Unknown tool name — lint should already have flagged it.
            # Be conservative: treat as missing so the test gets ignored
            # rather than spuriously executed.
            out.append(tok)
            continue
        if not _which(bin_name):
            out.append(tok)
    return out


def _load_expected_failures() -> set[str]:
    """Read `_meta/expected-failures.txt`. Lines starting with `#` are
    comments and ignored; blank lines too. Returns a set of test paths
    (workspace-relative).
    """
    path = (
        REPO_ROOT
        / "docs"
        / "generated"
        / "tests"
        / "_meta"
        / "expected-failures.txt"
    )
    if not path.is_file():
        return set()
    out: set[str] = set()
    for line in path.read_text().splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        out.add(s)
    return out


def cmd_verify(args: argparse.Namespace) -> int:
    """Run slang-test against the named bundles (or all of them) and
    report pass / FAILED / ignored / expected-fail counts.

    This is the local-verify step prompts call out before commit. It
    surfaces the two failure modes agents miss most often: unnecessary
    `non-exhaustive` on DIAGNOSTIC_TEST, and CHECK lines that don't
    match actual compiler output. Tests whose target backend the
    runner lacks (e.g. `-target dxil` with no DXC) are reported as
    `ignored` and do not fail this command — CI nightly validates
    them.

    Tests listed in `_meta/expected-failures.txt` (filed compiler
    bugs with a tracking issue) are reported as `expected-fail` and
    excluded from the FAILED count + exit code. Removing the bug fix
    must be paired with removing the line from that file.

    Tests whose `//META: requires-tool=<tool>` names a binary absent
    from PATH are filtered out of the slang-test invocation
    altogether so they appear as `ignored` (missing-tool) rather than
    failing on the missing prerequisite.
    """
    slang_test = _find_slang_test()
    if slang_test is None:
        print("ERROR: slang-test not found in any of the standard build dirs.")
        print("Build it first, e.g.:")
        print("    cmake --build --preset release --target slang-test")
        print("Then re-run this command.")
        return 2

    manifest = load_manifest()
    if args.bundles:
        specs = _bundles_arg(manifest, args.bundles)
        prefixes = [
            f"docs/generated/tests/{s.key}/" for s in specs
        ]
        scope_desc = ", ".join(s.key for s in specs)
    else:
        prefixes = []
        scope_desc = "all bundles"

    # Compute requires-tool exclusion list by scanning every .slang
    # in the target prefixes for a //META: requires-tool key with a
    # tool that's not on PATH. slang-test's `-exclude-prefix` filters
    # by path-prefix, which is exactly what we need to skip whole
    # files.
    exclude_prefixes: list[str] = []
    missing_tool_files: list[tuple[str, list[str]]] = []
    scan_roots: list[Path]
    if args.bundles:
        scan_roots = [
            REPO_ROOT / "docs" / "generated" / "tests" / s.key
            for s in specs
        ]
    else:
        scan_roots = [REPO_ROOT / "docs" / "generated" / "tests"]
    for root in scan_roots:
        if not root.is_dir():
            continue
        for slang_path in root.rglob("*.slang"):
            try:
                text = slang_path.read_text()
            except OSError:
                continue
            meta = parse_test_meta(text)
            missing = _missing_required_tools(meta)
            if missing:
                rel = _rel_to_repo(slang_path)
                exclude_prefixes.append(rel)
                missing_tool_files.append((rel, missing))

    cmd: list[str] = [str(slang_test), "-test-dir", "docs/generated/tests"]
    if args.server_count > 1:
        cmd.extend(["-use-test-server", "-server-count", str(args.server_count)])
    for ep in exclude_prefixes:
        cmd.extend(["-exclude-prefix", ep])
    cmd.extend(prefixes)

    expected_failures = _load_expected_failures()

    print(f"verify: {scope_desc}")
    print(f"  cmd: {' '.join(cmd)}")

    # slang-test exit code is non-zero when any test fails; that's
    # expected here — we want the counts, not the exit code.
    proc = subprocess.run(
        cmd,
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
    )
    out = proc.stdout
    err = proc.stderr

    failed_all: list[str] = []
    ignored = 0
    passed = 0
    for line in out.splitlines():
        if line.startswith("FAILED test:"):
            # Format: FAILED test: 'docs/generated/tests/.../foo.slang'
            m = re.match(r"^FAILED test:\s*'([^']+)'", line)
            if m:
                failed_all.append(m.group(1))
        elif line.startswith("passed test:"):
            passed += 1
        elif line.startswith("ignored test:"):
            ignored += 1

    # Bucket failures: expected (known compiler bug, tracked) vs
    # unexpected (must fix). Normalize multi-directive variants
    # (`foo.slang.1`) to their canonical .slang path for matching.
    def canonical(p: str) -> str:
        return re.sub(r"\.slang(\.\d+)$", ".slang", p)

    expected_fail: list[str] = []
    failed: list[str] = []
    for p in failed_all:
        if canonical(p) in expected_failures:
            expected_fail.append(p)
        else:
            failed.append(p)

    # slang-test prints a summary like "98% of tests passed (3413/3482), 47 tests ignored"
    summary_line = ""
    for line in out.splitlines():
        if "of tests passed" in line:
            summary_line = line.strip()
            break

    # Tests skipped by `-exclude-prefix` don't appear in slang-test
    # output at all; surface them as a separate `ignored (missing
    # tool)` bucket so the operator can see why coverage shrank.
    skipped_by_tool = len(missing_tool_files)

    print()
    if summary_line:
        print(f"  {summary_line}")
    print(f"  passed:        {passed}")
    print(f"  ignored:       {ignored}    (backend not available in slang-test — CI validates)")
    if skipped_by_tool:
        print(
            f"  ignored-tool:  {skipped_by_tool}    (//META: requires-tool not on PATH)"
        )
    if expected_fail:
        print(
            f"  expected-fail: {len(expected_fail)}    (listed in _meta/expected-failures.txt)"
        )
    print(f"  FAILED:        {len(failed)}")

    if missing_tool_files and args.verbose:
        print()
        print("Tests skipped because their declared requires-tool is missing:")
        for path, tools in missing_tool_files:
            print(f"  {path}  (missing: {', '.join(tools)})")

    if expected_fail and args.verbose:
        print()
        print("Expected-fail tests (known compiler bugs):")
        for f in expected_fail:
            print(f"  {f}")

    if failed:
        print()
        print("Failing tests:")
        for f in failed:
            print(f"  {f}")
        print()
        print(
            "Fix every FAILED test before commit. Most common causes:"
        )
        print(
            "  - DIAGNOSTIC_TEST with unnecessary 'non-exhaustive' (omit it"
        )
        print(
            "    by default; the runner errors when all diagnostics match)."
        )
        print(
            "  - Ordered CHECK lines against tokens on one emit line — use"
        )
        print(
            "    CHECK-DAG to match unordered."
        )
        print(
            "  - CHECK line does not match actual compiler output (re-read"
        )
        print(
            "    actual-output and either fix the CHECK or rewrite the test)."
        )
        print(
            "  - `-dump-ir` at the wrong observation point — target-specific"
        )
        print(
            "    opcodes synthesized in legalization aren't in that snapshot."
        )
        print(
            "  - The compiler rejects the input the doc proposes — record a"
        )
        print(
            "    `drift-from-source` doc-gap row in the bundle README."
        )
        print(
            "See `## Verify before committing` in"
            " docs/generated/tests/_meta/prompts/_common.md."
        )

    if err.strip():
        print()
        print("stderr (first 40 lines):")
        for line in err.splitlines()[:40]:
            print(f"  {line}")

    return 1 if failed else 0


# --------------------------------------------------------------------------
# lang-ref-coverage — measure how much of docs/language-reference/ is
# anchored by tests; derive the lang-ref ↔ design-doc mapping
# --------------------------------------------------------------------------


def _enumerate_lang_ref_anchors() -> dict[str, list[tuple[int, str, str]]]:
    """Walk docs/language-reference/*.md. For each file, return a list
    of (lineno, level, anchor_id) tuples — one per heading.

    Returns a dict keyed by workspace-relative path.
    """
    lang_ref_dir = REPO_ROOT / "docs" / "language-reference"
    if not lang_ref_dir.is_dir():
        return {}
    out: dict[str, list[tuple[int, str, str]]] = {}
    for md in sorted(lang_ref_dir.glob("*.md")):
        rel = _rel_to_repo(md)
        anchors: list[tuple[int, str, str]] = []
        text = md.read_text()
        # Match both ATX headings (## Foo) and Setext (Foo\n----).
        lines = text.splitlines()
        i = 0
        while i < len(lines):
            ln = lines[i]
            # ATX heading
            m = re.match(r"^(#{1,6})\s+(.+?)\s*#*\s*$", ln)
            if m:
                level = m.group(1)
                txt = m.group(2)
                # Strip an explicit `{#anchor}` extension if present
                m2 = re.search(r"\{#([\w\-]+)\}\s*$", txt)
                if m2:
                    aid = m2.group(1).lower()
                    txt = re.sub(r"\s*\{#[\w\-]+\}\s*$", "", txt)
                else:
                    aid = _github_slug(txt)
                anchors.append((i + 1, level, aid))
            else:
                # Setext-style: a non-blank line followed by ==== or ----
                if i + 1 < len(lines):
                    nxt = lines[i + 1]
                    if ln.strip() and re.match(r"^=+\s*$", nxt):
                        anchors.append((i + 1, "#", _github_slug(ln)))
                        i += 2
                        continue
                    if ln.strip() and re.match(r"^-+\s*$", nxt) and not ln.startswith("-"):
                        anchors.append((i + 1, "##", _github_slug(ln)))
                        i += 2
                        continue
            i += 1
        out[rel] = anchors
    return out


def _collect_test_doc_refs() -> dict[str, list[str]]:
    """Walk every *.slang under docs/generated/tests/ and collect the
    `//META: doc_ref=` value. Returns a dict mapping path -> list of
    workspace-relative test paths that cite it.

    Bundle source_doc values are also collected (as bundle-level
    citations) so the mapping query can join them.
    """
    out: dict[str, list[str]] = {}
    tests_root = REPO_ROOT / "docs" / "generated" / "tests"
    for slang in tests_root.rglob("*.slang"):
        if any(part.startswith("_meta") for part in slang.parts):
            continue
        try:
            head = slang.read_text()
        except OSError:
            continue
        m = re.search(r"//META: doc_ref=(\S+)", head)
        if m:
            ref = m.group(1)
            out.setdefault(ref, []).append(_rel_to_repo(slang))
    return out


def cmd_lang_ref_coverage(args: argparse.Namespace) -> int:
    """Report how much of docs/language-reference/ has at least one test
    anchored to it. Each `##` / `###` heading is a "claim slot".

    Outputs:
    - per-file coverage (covered anchors / total anchors)
    - the uncovered work queue
    - implied lang-ref ↔ design-doc mapping (from bundle context)
    """
    anchors = _enumerate_lang_ref_anchors()
    test_refs = _collect_test_doc_refs()

    # `test_refs` maps `path#anchor` strings to test paths. Bucket
    # citations by lang-ref file and anchor id.
    cited_anchors_by_file: dict[str, dict[str, list[str]]] = {}
    for ref, paths in test_refs.items():
        if not ref.startswith("docs/language-reference/"):
            continue
        if "#" in ref:
            file_part, frag = ref.split("#", 1)
        else:
            file_part, frag = ref, ""
        cited_anchors_by_file.setdefault(file_part, {}).setdefault(frag, []).extend(paths)

    total_anchors = 0
    total_covered = 0
    per_file: list[tuple[str, int, int]] = []  # (path, covered, total)
    uncovered: list[tuple[str, str]] = []  # (path, anchor_id)
    for path, slots in anchors.items():
        slot_ids = {aid for _ln, _lv, aid in slots if aid}
        cited_here = set(cited_anchors_by_file.get(path, {}).keys())
        covered_here = slot_ids & cited_here
        n_total = len(slot_ids)
        n_cov = len(covered_here)
        total_anchors += n_total
        total_covered += n_cov
        per_file.append((path, n_cov, n_total))
        for aid in sorted(slot_ids - cited_here):
            uncovered.append((path, aid))

    # Header
    print(f"language-reference coverage: {total_covered} / {total_anchors}"
          f" anchors ({100.0 * total_covered / max(1, total_anchors):.1f}%)\n")

    print("Per-file:")
    for path, n_cov, n_total in per_file:
        if n_total == 0:
            print(f"  {n_cov:>3}/{n_total:<3}        {path}")
        else:
            pct = 100.0 * n_cov / n_total
            print(f"  {n_cov:>3}/{n_total:<3}  {pct:5.1f}%  {path}")

    if args.list_uncovered:
        print()
        print("Uncovered anchors (work queue):")
        for path, aid in uncovered:
            print(f"  {path}#{aid}")

    if args.mapping:
        print()
        print("lang-ref → design-doc mapping (from bundle context):")
        # For each test that cites a lang-ref anchor, look up the
        # bundle it lives in and that bundle's source_doc. If the
        # bundle's source_doc is a design-doc, record an edge.
        manifest = load_manifest()
        # Build path → source_doc lookup for the bundle a test lives in.
        # The bundle key is the workspace-relative directory prefix.
        def bundle_for(test_path: str) -> BundleSpec | None:
            prefix = "docs/generated/tests/"
            if not test_path.startswith(prefix):
                return None
            rest = test_path[len(prefix):]
            # Try progressively shorter prefixes against the manifest.
            parts = rest.split("/")
            for cut in range(len(parts), 0, -1):
                key = "/".join(parts[:cut])
                if key in manifest.bundles:
                    return manifest.bundles[key]
            return None

        edges: dict[tuple[str, str], int] = {}
        for ref, paths in test_refs.items():
            if not ref.startswith("docs/language-reference/"):
                continue
            for tp in paths:
                spec = bundle_for(tp)
                if spec is None:
                    continue
                bundle_doc = spec.source_doc
                if bundle_doc and bundle_doc.startswith("docs/generated/design/"):
                    edge = (ref, bundle_doc)
                    edges[edge] = edges.get(edge, 0) + 1

        if not edges:
            print("  (no edges yet — no test cites lang-ref from inside a"
                  " design-doc bundle.)")
        else:
            for (lang_ref, design_doc), n in sorted(edges.items()):
                print(f"  {lang_ref}\n    ↔ {design_doc}  ({n} test(s))")

    # Exit code 0 when called purely informationally; 1 when --gate-on
    # is set and coverage is below the gate threshold (future expansion).
    return 0


# --------------------------------------------------------------------------
# Findings subcommands
# --------------------------------------------------------------------------


def cmd_findings_list(args: argparse.Namespace) -> int:
    state = load_findings_state()
    filed_state = state.get("findings") or {}
    pending = list_finding_paths(include_filed=False)
    filed = list_finding_paths(include_filed=True) if args.include_filed else []
    filed = [p for p in filed if p.parent.name == "filed"]
    if not pending and not filed:
        print("(no findings)")
        return 0
    for p in pending:
        try:
            f = load_finding(p)
        except Exception as exc:  # noqa: BLE001
            print(f"  parse-error  {p.relative_to(REPO_ROOT)}: {exc}")
            continue
        kind = f.get("suspected_kind", "?")
        when = (f.get("observed_at") or "")[:10]
        print(f"pending   {f.get('id', '?'):42s}  {when}  {kind}")
    if args.include_filed:
        for p in filed:
            try:
                f = load_finding(p)
            except Exception:
                continue
            entry = filed_state.get(f.get("id", ""), {})
            if entry.get("issue"):
                tag = f"filed     {f.get('id', '?'):42s}  -> #{entry['issue']}"
            elif entry.get("dup_of"):
                tag = f"dup-of    {f.get('id', '?'):42s}  -> #{entry['dup_of']}"
            else:
                tag = f"filed?    {f.get('id', '?'):42s}  (state unknown)"
            print(tag)
    return 0


def cmd_findings_show(args: argparse.Namespace) -> int:
    state = load_findings_state()
    p = _resolve_finding_path(args.id)
    if p is None:
        print(f"error: no finding with id {args.id!r}", file=sys.stderr)
        return 1
    try:
        finding = load_finding(p)
    except Exception as exc:  # noqa: BLE001
        print(f"error: failed to parse {p}: {exc}", file=sys.stderr)
        return 1
    issues = validate_finding(p, finding)
    if issues:
        for i in issues:
            print(f"  validation {i.severity}: {i.message}", file=sys.stderr)
        print("error: finding fails validation; refusing to render", file=sys.stderr)
        return 1
    title = compute_finding_title(finding)
    labels = compute_finding_labels(finding, state)
    body = render_finding_md(finding)
    print(f"Title: {title}")
    print(f"Labels: {', '.join(labels)}")
    print(f"Project: shader-slang #{(state.get('filing_defaults') or {}).get('project', {}).get('number', '?')}")
    fields = (state.get("filing_defaults") or {}).get("fields") or {}
    if fields:
        kv = ", ".join(f"{k}={v}" for k, v in fields.items())
        print(f"Fields: {kv}")
    print()
    print("---")
    print()
    print(body)
    return 0


def cmd_findings_file(args: argparse.Namespace) -> int:
    state = load_findings_state()
    p = _resolve_finding_path(args.id)
    if p is None:
        print(f"error: no finding with id {args.id!r}", file=sys.stderr)
        return 1
    try:
        finding = load_finding(p)
    except Exception as exc:  # noqa: BLE001
        print(f"error: failed to parse {p}: {exc}", file=sys.stderr)
        return 1
    issues = validate_finding(p, finding)
    errors = [i for i in issues if i.severity == "error"]
    if errors:
        for i in errors:
            print(f"  validation error: {i.message}", file=sys.stderr)
        print("error: finding fails validation; refusing to file", file=sys.stderr)
        return 1
    fid = finding["id"]
    findings_map = state.setdefault("findings", {})
    if fid in findings_map and findings_map[fid].get("issue"):
        print(
            f"error: {fid} already filed as #{findings_map[fid]['issue']}",
            file=sys.stderr,
        )
        return 1
    defaults = state.get("filing_defaults") or {}
    proj_cfg = defaults.get("project") or {}
    proj_owner = proj_cfg.get("owner")
    proj_number = proj_cfg.get("number")
    if not proj_owner or not proj_number:
        print("error: filing_defaults.project not configured", file=sys.stderr)
        return 1
    title = compute_finding_title(finding)
    labels = compute_finding_labels(finding, state)
    body = render_finding_md(finding)
    target_repo = args.repo
    print(f"→ Filing {fid} to {target_repo} as: {title!r}")
    print(f"  Labels: {', '.join(labels)}")
    if args.dry_run:
        print("  --dry-run: not invoking gh")
        return 0
    # 1. Create the issue
    create_args = [
        "issue",
        "create",
        "--repo",
        target_repo,
        "--title",
        title,
        "--body-file",
        "-",
    ]
    for lbl in labels:
        create_args.extend(["--label", lbl])
    try:
        result = subprocess.run(
            ["gh", *create_args],
            cwd=str(REPO_ROOT),
            input=body,
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as exc:
        print(
            f"error: gh issue create failed (rc={exc.returncode}): {exc.stderr}",
            file=sys.stderr,
        )
        return 1
    url = result.stdout.strip().splitlines()[-1] if result.stdout.strip() else ""
    issue_number = _issue_number_from_url(url)
    print(f"  filed: {url}")
    record: dict = {"issue": issue_number, "url": url, "title": title, "labels": labels}
    findings_map[fid] = record
    save_findings_state(state)
    # 2. Add to project + set fields. Any failure here is non-fatal: the
    #    issue exists; we record what succeeded and let the operator finish
    #    manually.
    try:
        project_id, project_fields = _fetch_project_field_ids(
            proj_owner, int(proj_number)
        )
    except Exception as exc:  # noqa: BLE001
        print(
            f"  warning: could not resolve project field IDs: {exc}",
            file=sys.stderr,
        )
        record["project_warning"] = f"field-id lookup failed: {exc}"
        save_findings_state(state)
        _move_to_filed(p)
        return 0
    try:
        item_add = _gh(
            [
                "project",
                "item-add",
                str(proj_number),
                "--owner",
                proj_owner,
                "--url",
                url,
                "--format",
                "json",
            ]
        )
        item_data = json.loads(item_add.stdout)
        item_id = item_data["id"]
    except (subprocess.CalledProcessError, json.JSONDecodeError, KeyError) as exc:
        print(f"  warning: project item-add failed: {exc}", file=sys.stderr)
        record["project_warning"] = f"item-add failed: {exc}"
        save_findings_state(state)
        _move_to_filed(p)
        return 0
    print(f"  added to project: item {item_id}")
    record["project_item_id"] = item_id
    field_results: dict[str, str] = {}
    for fname, fvalue in (defaults.get("fields") or {}).items():
        finfo = project_fields.get(fname)
        if not finfo:
            field_results[fname] = "field not on project"
            continue
        try:
            edit_args = [
                "project",
                "item-edit",
                "--id",
                item_id,
                "--project-id",
                project_id,
                "--field-id",
                finfo["id"],
            ]
            opts = finfo.get("options") or {}
            if opts:
                opt_id = opts.get(fvalue)
                if not opt_id:
                    field_results[fname] = f"unknown option {fvalue!r}"
                    continue
                edit_args.extend(["--single-select-option-id", opt_id])
            else:
                # ProjectV2Field can be text OR number-typed; the GraphQL
                # type isn't distinguished in field-list output, so use the
                # value's shape to pick: if it parses as a number, send
                # --number; otherwise --text. (Project #10's Estimate is a
                # number field and rejects --text.)
                try:
                    num = float(str(fvalue))
                    edit_args.extend(["--number", str(num)])
                except ValueError:
                    edit_args.extend(["--text", str(fvalue)])
            _gh(edit_args)
            field_results[fname] = "ok"
        except subprocess.CalledProcessError as exc:
            field_results[fname] = f"failed: {exc}"
    print("  project fields:")
    for fname, status in field_results.items():
        print(f"    {fname:12s} {status}")
    record["project_fields"] = field_results
    save_findings_state(state)
    _move_to_filed(p)
    print(f"  moved finding to {FINDINGS_FILED_DIR.relative_to(REPO_ROOT)}/")
    return 0


def cmd_findings_dup(args: argparse.Namespace) -> int:
    """Mark a pending finding as duplicate of an existing GitHub issue.

    Does not file anything; records dup_of in findings-state.json and moves
    the YAML to findings/filed/. Used when the operator's pre-file search
    surfaces an existing issue covering the same bug.
    """
    state = load_findings_state()
    p = _resolve_finding_path(args.id)
    if p is None:
        print(f"error: no finding with id {args.id!r}", file=sys.stderr)
        return 1
    try:
        finding = load_finding(p)
    except Exception as exc:  # noqa: BLE001
        print(f"error: failed to parse {p}: {exc}", file=sys.stderr)
        return 1
    findings_map = state.setdefault("findings", {})
    existing = findings_map.get(args.id, {})
    if existing.get("issue"):
        print(
            f"error: {args.id} already filed as #{existing['issue']}",
            file=sys.stderr,
        )
        return 1
    if existing.get("dup_of"):
        print(
            f"error: {args.id} already marked dup-of #{existing['dup_of']}",
            file=sys.stderr,
        )
        return 1
    findings_map[args.id] = {
        "dup_of": args.of,
        "title": compute_finding_title(finding),
    }
    if args.note:
        findings_map[args.id]["note"] = args.note
    save_findings_state(state)
    _move_to_filed(p)
    print(f"  marked {args.id} as dup-of #{args.of}")
    print(f"  moved finding to {FINDINGS_FILED_DIR.relative_to(REPO_ROOT)}/")
    return 0


def cmd_findings_fixed(args: argparse.Namespace) -> int:
    """Record that a pending finding no longer reproduces, and retire it.

    Triage keeps turning up findings that were real when observed and are
    fixed by the time anyone looks. Before this existed there was nowhere
    to put that result: the finding stayed pending, so the next pass
    re-tested it from scratch. Five were in that state when this landed --
    four from the sigsegv sweep plus one that only got cleared because it
    happened to also duplicate an open issue.

    The command deliberately demands evidence. `--verified-at` is required
    and must name the compiler commit the re-test ran against, because
    "fixed" without a commit is unfalsifiable: a later regression cannot
    be distinguished from a triage that was wrong, and this whole channel
    exists because unverified claims produced false results before. The
    finding YAML is kept -- retiring is not deleting, and a fixed finding
    is the seed of the regression test nobody has written yet.
    """
    state = load_findings_state()
    p = _resolve_finding_path(args.id)
    if p is None:
        print(f"error: no finding with id {args.id!r}", file=sys.stderr)
        return 1
    try:
        finding = load_finding(p)
    except Exception as exc:  # noqa: BLE001
        print(f"error: failed to parse {p}: {exc}", file=sys.stderr)
        return 1
    findings_map = state.setdefault("findings", {})
    existing = findings_map.get(args.id, {})
    if existing.get("issue"):
        print(
            f"error: {args.id} is filed as #{existing['issue']}; close the issue"
            " instead of retiring the finding",
            file=sys.stderr,
        )
        return 1
    if existing.get("dup_of"):
        print(
            f"error: {args.id} is already marked dup-of #{existing['dup_of']}",
            file=sys.stderr,
        )
        return 1
    if existing.get("fixed_at"):
        print(
            f"error: {args.id} already retired as fixed at {existing['fixed_at']}",
            file=sys.stderr,
        )
        return 1
    findings_map[args.id] = {
        "fixed_at": args.verified_at,
        "title": compute_finding_title(finding),
    }
    if args.note:
        findings_map[args.id]["note"] = args.note
    save_findings_state(state)
    _move_to_resolved(p)
    print(f"  retired {args.id} as fixed at {args.verified_at}")
    try:
        where = FINDINGS_RESOLVED_DIR.relative_to(REPO_ROOT)
    except ValueError:
        where = FINDINGS_RESOLVED_DIR
    print(f"  moved finding to {where}/")
    return 0


def _resolve_finding_path(fid: str) -> Path | None:
    for base in (FINDINGS_DIR, FINDINGS_FILED_DIR, FINDINGS_RESOLVED_DIR):
        candidate = base / f"{fid}.yaml"
        if candidate.exists():
            return candidate
    return None


def _issue_number_from_url(url: str) -> int | None:
    m = re.search(r"/issues/(\d+)$", url)
    return int(m.group(1)) if m else None


def _move_to_filed(p: Path) -> None:
    FINDINGS_FILED_DIR.mkdir(parents=True, exist_ok=True)
    dest = FINDINGS_FILED_DIR / p.name
    p.rename(dest)


def _move_to_resolved(p: Path) -> None:
    FINDINGS_RESOLVED_DIR.mkdir(parents=True, exist_ok=True)
    dest = FINDINGS_RESOLVED_DIR / p.name
    p.rename(dest)


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


# The design driver's `load_gap_queue` matches on this text to tell "no gaps
# for this document" (benign) from a real failure. Named on both sides and
# pinned by `selftest` so a reword cannot silently break that.
_NO_GAPS_FOR_DOC = "no gap rows are anchored to {doc!r}"

_GAP_TREES = {
    "design": "docs/generated/design/",
    "language-reference": "docs/language-reference/",
}


def cmd_doc_gaps(args: argparse.Namespace) -> int:
    """Aggregate ## Doc gaps observed rows across bundles, grouped by the
    document each gap is against.

    The output is the feedback artifact the doc-regeneration workflow
    consumes: for each docs/generated/design/<doc>.md, the list of gaps
    that bundles testing against it have reported. Rows reported by
    multiple bundles against the same Anchor + Kind are merged into
    a single row with a `Reported by` cell.

    Grouping follows each row's Anchor cell, not the reporting bundle's
    `source_doc`. The two agree for a doc-anchored bundle, but a
    `coverage/` bundle has no `source_doc` while still reporting gaps
    against design pages; grouping those under the bundle put them
    somewhere `--source-doc` could not reach, so they were invisible to
    the only workflow that can act on them.
    """
    manifest = load_manifest()
    specs = list(manifest.bundles.values())

    # anchor target doc -> list[GapRow]
    by_doc: dict[str, list[GapRow]] = {}
    for spec in specs:
        bdir = REPO_ROOT / spec.dir
        readme = bdir / "README.md"
        if not readme.exists():
            continue
        text = readme.read_text(encoding="utf-8")
        for row in parse_gap_rows(text, spec.dir, spec.gap_group):
            # A row whose Anchor names no document still has to go
            # somewhere or it would vanish from the report entirely; it
            # falls back to the reporting bundle, and `lint` errors on it.
            by_doc.setdefault(row.anchor_target or spec.gap_group, []).append(row)

    if args.tree != "all":
        prefix = _GAP_TREES[args.tree]
        by_doc = {d: rows for d, rows in by_doc.items() if d.startswith(prefix)}
    if args.source_doc:
        by_doc = {d: rows for d, rows in by_doc.items() if d == args.source_doc}
        if not by_doc:
            raise SystemExit(_NO_GAPS_FOR_DOC.format(doc=args.source_doc))

    merged_by_doc = {
        doc: _merge_gap_rows(rows) for doc, rows in sorted(by_doc.items())
    }

    if args.format == "json":
        import json

        print(json.dumps(merged_by_doc, indent=2))
        return 0

    for doc, merged in merged_by_doc.items():
        reporters = sorted({b for m in merged for b in m["reported_by"]})
        print(f"# Gaps reported against {doc}")
        print()
        print(f"Bundle reports: {', '.join(reporters)}")
        print()
        print(
            "| Gap id | Anchor | Kind | Gap | Suggested addition | Reported by |"
        )
        print("| --- | --- | --- | --- | --- | --- |")
        for m in merged:
            reported = ", ".join(m["reported_by"])
            print(
                f"| {m['gap_id']} | {m['anchor']} | {m['kind']} | {m['gap']}"
                f" | {m['suggested_addition']} | {reported} |"
            )
        print()
    if not merged_by_doc:
        print("# No doc-gap rows recorded across the selected bundles.")
    return 0


def _merge_gap_rows(rows: list[GapRow]) -> list[dict]:
    """Collapse one document's gap rows to one entry per distinct gap.

    Sibling bundles routinely observe the same hole in a shared page --
    three bundles anchored to the diagnostics doc all notice the same
    missing subsection -- and the consumer wants one work item, not three.
    Rows collapse on `gap_id`, so a re-wording that survives
    `_normalize_gap_prose` merges too, which the old key (the raw prose)
    did not. Distinct `Suggested addition` texts are kept and joined,
    since each reporter saw the gap from its own test and the proposals
    are usually complementary rather than redundant.

    Sorted by the raw Anchor-cell text then kind. That is *not* document
    order -- the cell is a markdown link, so this is lexicographic on the
    link text -- but it is stable and groups rows sharing an anchor, which
    is what a reader working through one document's gaps wants.
    """
    merged: dict[str, dict] = {}
    for r in rows:
        slot = merged.setdefault(
            r.gap_id,
            {
                "gap_id": r.gap_id,
                "anchor": r.anchor,
                "anchor_fragment": r.anchor_fragment,
                "kind": r.kind,
                "gap": r.gap,
                "suggested_addition": r.suggested_addition,
                "reported_by": [],
            },
        )
        if r.bundle not in slot["reported_by"]:
            slot["reported_by"].append(r.bundle)
        if (
            r.suggested_addition
            and r.suggested_addition != slot["suggested_addition"]
            and r.suggested_addition not in slot["suggested_addition"]
        ):
            slot["suggested_addition"] = (
                slot["suggested_addition"] + " // " + r.suggested_addition
            ).strip(" /")
    return sorted(merged.values(), key=lambda m: (m["anchor"], m["kind"]))


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
        description="Driver for the agentic test suite under docs/generated/tests/.",
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

    p_verify = sub.add_parser(
        "verify",
        help=(
            "run slang-test against bundle(s); report pass/FAILED/ignored."
            " Local-verify step required before commit (see _common.md)."
        ),
    )
    p_verify.add_argument(
        "bundles",
        nargs="*",
        help="bundle keys to verify (omit for the whole suite)",
    )
    p_verify.add_argument(
        "--server-count",
        type=int,
        default=4,
        help=(
            "slang-test server count (default 4; pass 1 to disable"
            " test-server and get more readable single-test errors)"
        ),
    )
    p_verify.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help=(
            "Also list the tests that were skipped by `requires-tool`"
            " filtering and the tests that matched the expected-failures"
            " list."
        ),
    )
    p_verify.set_defaults(func=cmd_verify)

    p_lrc = sub.add_parser(
        "lang-ref-coverage",
        help=(
            "report how many heading-anchors in docs/language-reference/"
            " have at least one test anchored to them"
        ),
    )
    p_lrc.add_argument(
        "--list-uncovered",
        action="store_true",
        help="dump the full list of uncovered anchors (the work queue)",
    )
    p_lrc.add_argument(
        "--mapping",
        action="store_true",
        help=(
            "show the implied lang-ref ↔ design-doc mapping derived"
            " from bundle source_doc context"
        ),
    )
    p_lrc.set_defaults(func=cmd_lang_ref_coverage)

    p_idx = sub.add_parser(
        "index",
        help="regenerate docs/generated/tests/INDEX.md (--write to overwrite)",
    )
    p_idx.add_argument(
        "--write",
        action="store_true",
        help="overwrite docs/generated/tests/INDEX.md (otherwise print to stdout)",
    )
    p_idx.set_defaults(func=cmd_index)

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
        help="aggregate ## Doc gaps observed rows across bundles, grouped by"
        " the document each gap is anchored to",
    )
    p_dg.add_argument(
        "--source-doc",
        help="restrict to gaps anchored to a single document, by its"
        " workspace-relative path",
    )
    p_dg.add_argument(
        "--tree",
        choices=("all", "design", "language-reference"),
        default="all",
        help="restrict to gaps against one doc tree. `design` is the queue"
        " the doc-regeneration workflow can act on; `language-reference`"
        " is the human-written spec, which no agent may edit, so those"
        " gaps are a human triage queue (default: all)",
    )
    p_dg.add_argument(
        "--format",
        choices=("md", "json"),
        default="md",
        help="output format (default: md)",
    )
    p_dg.set_defaults(func=cmd_doc_gaps)

    p_cd = sub.add_parser(
        "catalog-digest",
        help="print the doc_section_digest for a diagnostics-catalog code",
    )
    p_cd.add_argument("code", help="diagnostic code, e.g. 30019")
    p_cd.set_defaults(func=cmd_catalog_digest)

    p_st = sub.add_parser(
        "selftest",
        help="unit-check the slug / link / table helpers used by lint",
    )
    p_st.set_defaults(func=cmd_selftest)

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

    p_find = sub.add_parser(
        "findings",
        help="(Phase F) list / show / file structured compiler-bug findings",
    )
    find_sub = p_find.add_subparsers(dest="findings_cmd", required=True)

    p_fl = find_sub.add_parser("list", help="list pending findings")
    p_fl.add_argument(
        "--include-filed",
        action="store_true",
        help="also list findings already filed as issues",
    )
    p_fl.set_defaults(func=cmd_findings_list)

    p_fs = find_sub.add_parser(
        "show", help="render the issue body for a finding to stdout"
    )
    p_fs.add_argument("id")
    p_fs.set_defaults(func=cmd_findings_show)

    p_ff = find_sub.add_parser(
        "file",
        help="file the finding as an issue (calls gh; updates findings-state.json)",
    )
    p_ff.add_argument("id")
    p_ff.add_argument(
        "--repo",
        default="shader-slang/slang",
        help="target repository (default: shader-slang/slang)",
    )
    p_ff.add_argument(
        "--dry-run",
        action="store_true",
        help="don't invoke gh; just show what would be filed",
    )
    p_ff.set_defaults(func=cmd_findings_file)

    p_fd = find_sub.add_parser(
        "dup",
        help="mark a pending finding as duplicate of an existing issue"
        " (no filing; moves YAML to filed/)",
    )
    p_fd.add_argument("id")
    p_fd.add_argument(
        "--of",
        type=int,
        required=True,
        help="existing issue number this finding duplicates",
    )
    p_fd.add_argument(
        "--note",
        help="optional one-line note explaining the dedup decision",
    )
    p_fd.set_defaults(func=cmd_findings_dup)

    p_fx = find_sub.add_parser(
        "fixed",
        help="retire a pending finding that no longer reproduces"
        " (no filing; moves YAML to resolved/)",
    )
    p_fx.add_argument("id")
    p_fx.add_argument(
        "--verified-at",
        required=True,
        metavar="COMMIT",
        help="compiler commit the re-test ran against; required, because"
        " 'fixed' without one cannot be told apart from a later regression",
    )
    p_fx.add_argument(
        "--note",
        help="optional one-line note on what was re-run and what it showed",
    )
    p_fx.set_defaults(func=cmd_findings_fixed)

    return p


def cmd_catalog_digest(args: argparse.Namespace, snapshot: Path | None = None) -> int:
    """Print the `doc_section_digest` a diagnostics-catalog test should carry.

    Exists so that the generation prompt can name one command instead of
    describing a hashing rule an agent then re-implements by hand -- which is
    how the corpus ended up with values nothing could reproduce (#11410).

    `snapshot` defaults to the real one and is a parameter only so that
    `selftest` can drive the missing-snapshot and success paths against a
    fixture; the argparse entry point never passes it.
    """
    snapshot = snapshot or CATALOG_SNAPSHOT_PATH
    catalog = parse_catalog_snapshot(snapshot)
    if not catalog:
        print(f"error: no catalog snapshot at {_rel_to_repo(snapshot)}")
        return 1
    entry = catalog.get(args.code)
    if entry is None:
        print(f"error: code {args.code} is not in the catalog snapshot")
        return 1
    print(catalog_entry_digest(args.code, *entry))
    return 0


def cmd_selftest(args: argparse.Namespace) -> int:
    """Unit-check the helpers `lint` depends on, on inputs the corpus does
    not contain.

    `lint` itself only ever exercises these against the committed corpus,
    which is kept clean -- so it proves the happy path and nothing else.
    The cases below are the edge ones: the punctuation rule that made
    every anchor in this corpus wrong, the escaped pipe that decides
    whether a row is well-formed, the GFM spellings (single-dash
    delimiter, one-column table) that a stricter reader would skip
    without reporting anything, and the link/anchor resolution that is
    the headline of this change -- its error, warning, and dedup
    branches, which the clean corpus never triggers.
    """
    failures: list[str] = []

    def check(what: str, got, want) -> None:
        if got != want:
            failures.append(f"{what}: got {got!r}, want {want!r}")

    # Punctuation is deleted, not replaced, so each surrounding space
    # still yields a hyphen. This is the rule the old `_gh_slug`
    # collapsed away, and collapsing it is what made the corpus's
    # anchors miss.
    check("slug plain", _github_slug("Basic scalar types"), "basic-scalar-types")
    check(
        "slug slashes",
        _github_slug("CUDA / Python / FFI attributes"),
        "cuda--python--ffi-attributes",
    )
    check("slug code span", _github_slug("The `IRFunc` type"), "the-irfunc-type")
    check("slug trailing punct", _github_slug("What now?"), "what-now")

    # Repeated headings get -1, -2; fenced `# comment` lines are not
    # headings.
    with tempfile.TemporaryDirectory() as td:
        doc = Path(td) / "d.md"
        doc.write_text(
            "# Title\n## Dup\n## Dup\nSetext\n------\n"
            "```sh\n# not a heading\n```\n",
            encoding="utf-8",
        )
        check(
            "heading slugs",
            heading_slugs(doc),
            {"title", "dup", "dup-1", "setext"},
        )

    # Only a bare pipe separates cells.
    check("row plain", _row_cells("| a | b | c |"), [" a ", " b ", " c "])
    check("row escaped", len(_row_cells(r"| a | `x \|\| y` | c |")), 3)
    check("row extra", len(_row_cells("| a | b | c | d |")), 4)

    # GFM spellings a stricter delimiter test would silently skip.
    check("delim single dash", _is_delimiter_row("| - | - |"), True)
    check("delim one column", _is_delimiter_row("| --- |"), True)
    check("delim aligned", _is_delimiter_row("| :--- | ---: | :---: |"), True)
    check("delim not a delim", _is_delimiter_row("| a | b |"), False)

    # A malformed row is reported, and a table inside a fence is not.
    with tempfile.TemporaryDirectory() as td:
        md = Path(td) / "README.md"
        md.write_text(
            "| h1 | h2 |\n| --- | --- |\n| a | b | c |\n\n"
            "```markdown\n| bad |\n| --- | --- |\n```\n",
            encoding="utf-8",
        )
        found = [i.message for i in lint_markdown_tables(md)]
        check("table one issue", len(found), 1)
        check("table names the row", "line 3" in found[0] if found else False, True)
        check(
            "table blames the pipe",
            "escape a literal pipe" in found[0] if found else False,
            True,
        )

    # lint_markdown_links -- the function this PR is named for. A dead
    # path is an error (the `../` miscount this PR set out to fix), a
    # resolved path whose `#anchor` hits no heading is a warning, a
    # citation that resolves in both halves is silent, and the same
    # broken citation repeated across rows is reported once (a coverage
    # table cites the same anchor per claim).
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        (root / "target.md").write_text("# Real Heading\n", encoding="utf-8")
        src = root / "README.md"
        src.write_text(
            "[ok](target.md#real-heading)\n"
            "[stale anchor](target.md#missing)\n"
            "[dead](nope.md#x)\n"
            "[dead again](nope.md#x)\n",
            encoding="utf-8",
        )
        issues = lint_markdown_links(src)
        errors = [i for i in issues if i.severity == "error"]
        warnings = [i for i in issues if i.severity == "warning"]
        check("links dead path is error", len(errors), 1)
        check("links stale anchor is warning", len(warnings), 1)
        check("links dedups repeated citation", len(issues), 2)
        check(
            "links error names the path",
            "nope.md" in errors[0].message if errors else False,
            True,
        )

    # lint_liquid_safe -- the Pages-build guard. A raw `{{` or `{%` in the
    # body is an error either way it fails Liquid (an unterminated tag aborts
    # the build, a terminated one is evaluated and drops the literal text);
    # the two dual-safe spellings -- spaced braces and numeric entities in a
    # `<code>` element -- pass; and a `{{` inside the front-matter is not
    # counted, since Jekyll strips front-matter before Liquid runs.
    with tempfile.TemporaryDirectory() as td:
        fm = '---\ngenerated: true\nwarning: "x"\n---\n'
        bad = Path(td) / "bad.md"
        bad.write_text(fm + "probed with `float2x2 m = {{1,2},{3,4}}`.\n", encoding="utf-8")
        tag = Path(td) / "tag.md"
        tag.write_text(fm + "a {% raw %} block.\n", encoding="utf-8")
        ok = Path(td) / "ok.md"
        ok.write_text(
            fm
            + "spaced `float2x2 m = { {1,2},{3,4} }` and"
            + " <code>&#123;&#123;.*&#125;&#125;</code> both render.\n",
            encoding="utf-8",
        )
        fm_only = Path(td) / "fm.md"
        fm_only.write_text('---\nwarning: "see {{x}}"\n---\nclean body.\n', encoding="utf-8")
        check("liquid rejects raw output tag", len(lint_liquid_safe(bad)), 1)
        check("liquid rejects raw statement tag", len(lint_liquid_safe(tag)), 1)
        check("liquid accepts spaced braces and entities", len(lint_liquid_safe(ok)), 0)
        check("liquid ignores front-matter", len(lint_liquid_safe(fm_only)), 0)
        bad_issues = lint_liquid_safe(bad)
        check(
            "liquid error names the line",
            "line 5" in bad_issues[0].message if bad_issues else False,
            True,
        )

    # Doc-gap identity. `doc-gaps` groups by the anchored document and the
    # consuming ledger keys on `gap_id`, so both have to hold still across
    # the rewording a bundle regeneration produces -- the corpus, being one
    # snapshot, cannot show that a re-worded row keeps its id.
    check(
        "gap prose ignores markup and punctuation",
        _normalize_gap_prose("The `Decl` node -- see [x](y.md) -- is *not* documented."),
        "the decl node see x is not documented",
    )
    check(
        "gap id survives rewording that normalizes away",
        compute_gap_id("d.md", "#s", "missing-example", "The `foo` case, unstated."),
        compute_gap_id("d.md", "#s", "missing-example", "The foo case unstated"),
    )
    check(
        "gap id separates distinct kinds",
        compute_gap_id("d.md", "#s", "missing-example", "x")
        == compute_gap_id("d.md", "#s", "ambiguous-claim", "x"),
        False,
    )
    check(
        "gap id separates distinct target docs",
        compute_gap_id("a.md", "#s", "missing-example", "x")
        == compute_gap_id("b.md", "#s", "missing-example", "x"),
        False,
    )

    # Anchor-cell routing: a coverage bundle has no `source_doc`, so the
    # link in the cell is the only thing that says which document the gap
    # is in. A cell with no link routes nowhere, which `lint_bundle` errors
    # on -- the empty string is that signal, not an incidental default.
    check(
        "gap target resolves relative to the bundle dir",
        _resolve_gap_target(
            "docs/generated/tests/coverage/autodiff",
            "[#x](../../../design/ir-reference/differentiation.md#x)",
        ),
        "docs/generated/design/ir-reference/differentiation.md",
    )
    check(
        "gap target empty for a bare fragment",
        _resolve_gap_target("docs/generated/tests/coverage/autodiff", "[#x](#x)"),
        "",
    )
    check(
        "gap target empty for an unlinked cell",
        _resolve_gap_target("docs/generated/tests/coverage/autodiff", "#x"),
        "",
    )

    # Sibling bundles reporting the same gap collapse to one work item,
    # keeping both suggestions; the corpus has such pairs but nothing
    # asserts the merge, and a regression would silently triple the queue.
    def _mk_gap(bundle: str, prose: str, sugg: str) -> GapRow:
        target = "docs/generated/design/d.md"
        return GapRow(
            anchor="[#s](../../../design/d.md#s)",
            anchor_fragment="#s",
            kind="missing-surface",
            gap=prose,
            suggested_addition=sugg,
            bundle=bundle,
            source_doc="",
            anchor_target=target,
            gap_id=compute_gap_id(target, "#s", "missing-surface", prose),
        )

    merged = _merge_gap_rows(
        [
            _mk_gap("b/one", "The rule is unstated.", "Add the rule."),
            _mk_gap("b/two", "The rule is unstated", "Add an example too."),
            _mk_gap("b/three", "A different hole entirely.", "Add that."),
        ]
    )
    check("gap merge collapses re-worded duplicates", len(merged), 2)
    dup = [m for m in merged if len(m["reported_by"]) > 1]
    check("gap merge records both reporters", len(dup), 1)
    check(
        "gap merge keeps both suggestions",
        dup[0]["suggested_addition"] if dup else "",
        "Add the rule. // Add an example too.",
    )

    # Finding reproducibility. A finding whose command names a scratch path
    # cannot be re-run at triage time; the committed corpus has both shapes,
    # but only the negative cases prove the check fires.
    def repro_warns(ev):
        return [i.message for i in _lint_finding_reproducibility("f", ev)]

    check(
        "scratch command without inline repro warns",
        len(repro_warns({"command": "slangc /tmp/x.slang -target hlsl"})),
        1,
    )
    check(
        "placeholder command without inline repro warns",
        len(repro_warns({"command": "slangc <file> -target hlsl"})),
        1,
    )
    check(
        "scratch command with inline repro is accepted",
        repro_warns({"command": "slangc /tmp/x.slang", "repro_source": "void main(){}"}),
        [],
    )
    check(
        "operator-minimized repro also satisfies it",
        repro_warns({"command": "slangc <file>", "minimized_repro": "void main(){}"}),
        [],
    )
    check(
        "committed input with a clean command is silent",
        repro_warns({
            "command": "slangc docs/generated/tests/x.slang -target hlsl",
            "source_slang": "docs/generated/tests/_meta/regenerate.py",
        }),
        [],
    )
    check(
        "missing source_slang with no inline repro warns",
        len(repro_warns({"command": "slangc a.slang", "source_slang": "docs/nope.slang"})),
        1,
    )

    # The lint rule and the issue renderer have to agree on what counts as
    # the repro, or the rule accepts a finding that then files an issue
    # showing a different program. `_repro_body` honoured `minimized_repro`
    # but not `repro_source`, so a finding that satisfied the rule the
    # intended way still rendered `source_slang` from disk.
    # `source_slang` here points at a real committed file so the fallback
    # path is exercised; the comparisons are truncated so that a
    # regression -- which returns that whole file -- reports a short
    # prefix instead of dumping it into the selftest output.
    inline_only = {
        "command": "slangc /tmp/x.slang -target hlsl",
        "source_slang": "docs/generated/tests/_meta/schema/finding.schema.json",
        "repro_source": "void main() { /* the scratch variant */ }",
    }
    check(
        "issue body prefers the inlined repro over source_slang on disk",
        _repro_body({"evidence": inline_only}).strip()[:41],
        "void main() { /* the scratch variant */ }",
    )
    check(
        "operator minimization still outranks the agent's inline repro",
        _repro_body({"evidence": dict(inline_only, minimized_repro="void main() {}")}).strip()[:14],
        "void main() {}",
    )
    check(
        "committed source_slang is still read when nothing is inlined",
        _repro_body({"evidence": {k: v for k, v in inline_only.items() if k != "repro_source"}})[:1],
        "{",
    )

    # `findings fixed` retires a finding that stopped reproducing. The
    # guards are the point: a finding that is filed, already deduped, or
    # already retired must not be retired again, or the state entry that
    # records where it went is silently overwritten. Exercised through
    # the real command against a temporary state file rather than by
    # re-implementing the checks here.
    with tempfile.TemporaryDirectory() as td:
        import argparse as _ap

        real_state, real_dir, real_filed, real_res = (
            FINDINGS_STATE_PATH, FINDINGS_DIR, FINDINGS_FILED_DIR, FINDINGS_RESOLVED_DIR,
        )
        g = globals()
        try:
            root = Path(td)
            g["FINDINGS_STATE_PATH"] = root / "state.json"
            g["FINDINGS_DIR"] = root / "findings"
            g["FINDINGS_FILED_DIR"] = root / "findings" / "filed"
            g["FINDINGS_RESOLVED_DIR"] = root / "findings" / "resolved"
            g["FINDINGS_DIR"].mkdir(parents=True)
            (g["FINDINGS_DIR"] / "x.yaml").write_text(
                'id: x\ntitle: a finding\nobserved_at: "2026-01-01T00:00:00+00:00"\n',
                encoding="utf-8",
            )

            def run(fid, commit="abc123"):
                return cmd_findings_fixed(
                    _ap.Namespace(id=fid, verified_at=commit, note=None)
                )

            check("fixed: unknown id rejected", run("nope"), 1)
            check("fixed: retires a pending finding", run("x"), 0)
            check(
                "fixed: YAML moved to resolved/, not deleted",
                (g["FINDINGS_RESOLVED_DIR"] / "x.yaml").exists(),
                True,
            )
            entry = json.loads(g["FINDINGS_STATE_PATH"].read_text())["findings"]["x"]
            check("fixed: records the verifying commit", entry.get("fixed_at"), "abc123")
            check("fixed: retiring twice is refused", run("x"), 1)

            # A filed finding must not be retirable -- the issue is the
            # record, and closing it is a different action.
            st = json.loads(g["FINDINGS_STATE_PATH"].read_text())
            st["findings"]["y"] = {"issue": 999}
            g["FINDINGS_STATE_PATH"].write_text(json.dumps(st), encoding="utf-8")
            (g["FINDINGS_DIR"] / "y.yaml").write_text("id: y\n", encoding="utf-8")
            check("fixed: refuses a finding that is already filed", run("y"), 1)
            check(
                "fixed: refused finding stays put",
                (g["FINDINGS_DIR"] / "y.yaml").exists(),
                True,
            )
        finally:
            g["FINDINGS_STATE_PATH"] = real_state
            g["FINDINGS_DIR"] = real_dir
            g["FINDINGS_FILED_DIR"] = real_filed
            g["FINDINGS_RESOLVED_DIR"] = real_res

    # An unquoted ISO timestamp is a YAML date, not a string, and the two
    # spellings are indistinguishable in the file. `findings list` used to
    # abort partway through the queue on the unquoted ones; `load_finding`
    # now normalizes, so consumers see one type regardless of spelling.
    with tempfile.TemporaryDirectory() as td:
        both = Path(td) / "f.yaml"
        for spelling in ("2026-08-04T00:00:00+00:00", '"2026-08-04T00:00:00+00:00"'):
            both.write_text(f"id: x\nobserved_at: {spelling}\n", encoding="utf-8")
            loaded = load_finding(both)["observed_at"]
            # Compare the type before slicing: a regression here returns a
            # datetime, and slicing that raises rather than reporting,
            # which would abort the rest of the selftest.
            check(
                f"observed_at loads as str for spelling {spelling}",
                type(loaded).__name__,
                "str",
            )
            check(
                f"observed_at keeps its date for spelling {spelling}",
                str(loaded)[:10],
                "2026-08-04",
            )

    # `--tree` routing: two production callers depend on this prefix split
    # (the workflow passes `language-reference`, the design driver always
    # passes `design`), and a mis-partition would silently send a page's
    # gaps to the wrong queue.
    check(
        "tree prefixes are the two doc trees",
        sorted(_GAP_TREES),
        ["design", "language-reference"],
    )
    check(
        "design tree prefix",
        _GAP_TREES["design"],
        "docs/generated/design/",
    )
    check(
        "language-reference tree prefix",
        _GAP_TREES["language-reference"],
        "docs/language-reference/",
    )
    _rows = {
        "docs/generated/design/a.md": [1],
        "docs/language-reference/b.md": [1],
        "coverage/orphan (no source doc)": [1],
    }
    for tree, want in (("design", 1), ("language-reference", 1)):
        pref = _GAP_TREES[tree]
        check(
            f"--tree {tree} partitions by prefix",
            len({d: r for d, r in _rows.items() if d.startswith(pref)}),
            want,
        )

    # The design driver recognizes "this document has no gaps" by matching
    # this exact substring on our stderr. Pin it here so a reword fails
    # loudly rather than making every gap-status run raise.
    check(
        "cross-driver no-gaps message is stable",
        "no gap rows are anchored to" in _NO_GAPS_FOR_DOC,
        True,
    )

    # The catalog snapshot parser and the digest rule it feeds. The committed
    # corpus only shows entries that already agree, so the drift and
    # unknown-code branches below are the ones `lint` cannot reach on its own.
    with tempfile.TemporaryDirectory() as td:
        snap = Path(td) / "catalog.txt"
        snap.write_text(
            "# header line, ignored\n"
            "\n"
            "1\tUNCOVERED\terr\tcannot-open-file\tslang-diagnostics.lua"
            "\tcannot open file '~path'\n"
            # A message containing a tab: everything past the source column is
            # the message, so this must not be truncated at the tab.
            "2\tCOVERED\twarning\ttabbed\tslang-diagnostics.lua\tone\ttwo\n"
            # Too few columns to be an entry.
            "junk\trow\n",
            encoding="utf-8",
        )
        rows = parse_catalog_snapshot(snap)
        check("catalog rows parsed", sorted(rows), ["1", "2"])
        check(
            "catalog message keeps embedded tab",
            rows["2"],
            ("warning", "tabbed", "one\ttwo"),
        )
        check(
            "catalog digest matches the documented rule",
            catalog_entry_digest("1", *rows["1"]),
            hashlib.sha256(
                b"1\terr\tcannot-open-file\tcannot open file '~path'"
            ).hexdigest(),
        )

    # lint_catalog_entry_digests over a fixture holding an agreeing test, a
    # drifted test, and one citing an unknown code. Drift and unknown-code are
    # the branches the kept-clean corpus never reaches, so the check that they
    # each warn (and that the agreeing test stays silent) lives only here. The
    # count of exactly two issues is what keeps the agreeing test out.
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        bundle = root / "docs/generated/tests/design/cross-cutting/diagnostics-catalog"
        bundle.mkdir(parents=True)
        snap_dir = root / "docs/generated/tests/_meta/diagnostics-catalog"
        snap_dir.mkdir(parents=True)
        (snap_dir / "catalog.txt").write_text(
            "1\tUNCOVERED\terr\tcannot-open-file\tsrc\tcannot open file\n",
            encoding="utf-8",
        )

        def _write(name: str, code: str, digest: str) -> None:
            (bundle / name).write_text(
                f"//META: catalog_code={code}\n"
                f"//META: doc_section_digest={digest}\n",
                encoding="utf-8",
            )

        good = catalog_entry_digest("1", "err", "cannot-open-file", "cannot open file")
        _write("agree.slang", "1", good)
        _write("drifted.slang", "1", "cd" * 32)
        _write("gone.slang", "9999", "ef" * 32)
        # A test mid-generation that has a code but no digest yet is skipped
        # rather than counted, which is what keeps the count below at two.
        (bundle / "partial.slang").write_text("//META: catalog_code=1\n", encoding="utf-8")

        # The non-catalog bundle gets a populated directory holding a test that
        # *would* be flagged, so "skips non-catalog bundles" fails when the
        # guard is dropped instead of passing on an empty glob.
        sibling = root / "docs/generated/tests/design/cross-cutting/diagnostics"
        sibling.mkdir(parents=True)
        (sibling / "drifted.slang").write_text(
            "//META: catalog_code=1\n//META: doc_section_digest=" + "cd" * 32 + "\n",
            encoding="utf-8",
        )

        spec = BundleSpec(
            key="design/cross-cutting/diagnostics-catalog",
            source_doc=None,
            watched_paths=[],
        )
        other = BundleSpec(
            key="design/cross-cutting/diagnostics",
            source_doc=None,
            watched_paths=[],
        )
        snap = snap_dir / "catalog.txt"
        found = lint_catalog_entry_digests([spec], root=root, snapshot=snap)

        # Two warnings -- the drifted test and the unknown code -- and neither
        # the agreeing test nor the digest-less one contributes.
        check("catalog lint reports two issues", len(found), 2)
        check(
            "catalog lint is warn-only",
            {i.severity for i in found},
            {"warning"},
        )
        drift = [i for i in found if i.where.endswith("drifted.slang")]
        check("catalog lint flags the drifted test", len(drift), 1)
        unknown = [i for i in found if "9999" in i.message]
        check("catalog lint groups the unknown code", len(unknown), 1)
        check(
            "catalog lint skips non-catalog bundles",
            lint_catalog_entry_digests([other], root=root, snapshot=snap),
            [],
        )

        # An absent snapshot must be reported, not silently treated as "nothing
        # to check" -- that would leave the bundle unverified while lint passes.
        missing = snap_dir / "does-not-exist.txt"
        absent = lint_catalog_entry_digests([spec], root=root, snapshot=missing)
        check("missing snapshot warns", len(absent), 1)
        check(
            "missing snapshot names the count",
            "4 catalog test(s) went unchecked" in absent[0].message if absent else False,
            True,
        )
        check(
            "missing snapshot is silent without a catalog bundle",
            lint_catalog_entry_digests([other], root=root, snapshot=missing),
            [],
        )

        # The catalog-digest subcommand is what the prompt and README tell an
        # agent to run, so its exit status and output are part of the contract.
        def _run_catalog_digest(code: str, snapshot_path: Path):
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                rc = cmd_catalog_digest(argparse.Namespace(code=code), snapshot=snapshot_path)
            return rc, buf.getvalue().strip()

        rc, out = _run_catalog_digest("1", snap)
        check("catalog-digest succeeds for a known code", rc, 0)
        check("catalog-digest prints the digest", out, good)
        rc, out = _run_catalog_digest("999999", snap)
        check("catalog-digest fails for an unknown code", rc, 1)
        check("catalog-digest names the unknown code", "999999" in out, True)
        rc, out = _run_catalog_digest("1", missing)
        check("catalog-digest fails without a snapshot", rc, 1)
        check("catalog-digest names the missing snapshot", "no catalog snapshot" in out, True)

    # The catalog bundle is matched by exact manifest key, so a bundle whose path
    # merely contains "diagnostics-catalog" does not inherit catalog-only rules.
    def _is_catalog(key: str) -> bool:
        return BundleSpec(key=key, source_doc=None, watched_paths=[]).is_diagnostics_catalog

    check("catalog matched by exact key", _is_catalog("design/cross-cutting/diagnostics-catalog"), True)
    check("lookalike suffix is not the catalog", _is_catalog("design/cross-cutting/diagnostics-catalog-extra"), False)
    check("prefix is not the catalog", _is_catalog("design/cross-cutting/diagnostics"), False)

    # _code_sort_key exists to order codes numerically; plain string sort would
    # put "100" before "99".
    check("code sort key orders numerically", sorted(["100", "99", "3001"], key=_code_sort_key), ["99", "100", "3001"])
    check("code sort key puts non-numeric last", sorted(["W123", "42"], key=_code_sort_key), ["42", "W123"])

    for f in failures:
        print(f"FAIL {f}")
    print(f"selftest: {len(failures)} failure(s)")
    return 1 if failures else 0


def main(argv: list[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
