#!/usr/bin/env python3
"""Driver for the LLM-generated documentation pipeline.

Tracks which documents under docs/generated/design/ are stale relative to the
source paths they declare in manifest.yaml, and lints generated documents
for structural conformance.

This script does NOT invoke any agent. Generation, review, and remediation
are all performed out-of-band by an operator running an agent against the
prompt templates under prompts/. Once a document has been (re)generated,
reviewed, or remediated, run the matching mark-* subcommand to record the
result in freshness.json or review-state.json.

Subcommands
-----------

    list                       List all documents in the manifest.
    list-stale [--include-review]
                               List documents whose watched-paths digest
                               differs from the recorded freshness entry (or
                               that have no freshness entry yet). With
                               --include-review, also annotate each doc with
                               its review/remediation freshness.
    digest <doc>               Print the current digest of a document's
                               watched paths.
    show <doc>                 Print the manifest entry plus the resolved
                               file list for a document.
    mark-fresh <doc>           Record a fresh entry for <doc> using the
                               current digest, HEAD commit, and now() as the
                               generation timestamp. Pass --commit / --model
                               to override.
    lint [<doc>...]            Run the structural linter (front-matter
                               present, paths resolve, size cap respected)
                               on the given documents (default: all). Also
                               lints every review and remediation report
                               under _meta/reviews/ and _meta/remediations/.
    review-status [<doc>...] [--show-counts]
                               For each doc, print one of unreviewed,
                               review-stale, reviewed-pending-remediation,
                               remediated. With --show-counts, append the
                               recorded severity / action breakdown.
    mark-reviewed <doc> [--report <path>]
                               Record a review entry for <doc> in
                               review-state.json from the named report file
                               (default _meta/reviews/<doc>.review.md).
                               Refuses Claude reviewer_model values.
    mark-remediated <doc> [--report <path>]
                               Record a remediation entry for <doc> in
                               review-state.json from the named report file
                               (default _meta/remediations/<doc>.remediation.md).
                               Refuses non-Claude remediator_model values.

Exit codes
----------
    0  success / no stale docs / lint clean
    1  one or more checks failed
    2  invocation error (bad arguments, missing manifest, etc.)
"""

from __future__ import annotations

import argparse
import datetime as _dt
import fnmatch
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
DOCS_ROOT = META_DIR.parent
REPO_ROOT = DOCS_ROOT.parent.parent.parent
MANIFEST_PATH = META_DIR / "manifest.yaml"
FRESHNESS_PATH = META_DIR / "freshness.json"
REVIEW_STATE_PATH = META_DIR / "review-state.json"
REVIEWS_DIR = META_DIR / "reviews"
REMEDIATIONS_DIR = META_DIR / "remediations"

# Soft refusal patterns for the review / remediation model fields.
# - The reviewer (a different model family) must NOT advertise itself as
#   Claude or Anthropic.
# - The remediator (same family that generated the docs) must advertise
#   itself as Claude or Anthropic.
_CLAUDE_FAMILY_TOKENS = ("claude", "anthropic")


def _rel_to_repo(path: Path) -> str:
    return str(path.relative_to(REPO_ROOT)).replace(os.sep, "/")


# --------------------------------------------------------------------------
# Tiny YAML loader
#
# We deliberately avoid a hard PyYAML dependency. The manifest uses a
# restricted YAML subset (scalars, lists of scalars, mappings, comments,
# integers, strings) that we parse by hand. If PyYAML is installed we use
# it; otherwise we fall back to the built-in parser.
# --------------------------------------------------------------------------

try:  # pragma: no cover - presence check only
    import yaml as _pyyaml  # type: ignore
    _HAVE_PYYAML = True
except ImportError:  # pragma: no cover - exercised in stripped envs
    _HAVE_PYYAML = False


def _load_yaml(text: str):
    if _HAVE_PYYAML:
        return _pyyaml.safe_load(text)
    return _MiniYaml(text).parse()


class _MiniYaml:
    """Indentation-based parser for the restricted subset used here.

    Supports:
      - block mappings (key: value, key:\n  nested)
      - block sequences (- item)
      - scalars: bare strings, quoted strings, integers, booleans
      - line comments starting with '#'
    Does NOT support: flow style, anchors/aliases, multi-line scalars,
    explicit document markers.
    """

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
            elif ":" in item_text and not item_text.startswith(("'", '"')):
                # inline mapping start
                key, _, rest = item_text.partition(":")
                rest = rest.strip()
                entry: dict = {}
                if rest:
                    entry[self._scalar(key.strip())] = self._scalar(rest)
                else:
                    entry[self._scalar(key.strip())] = (
                        self._parse_block(indent + 2) or {}
                    )
                # Continue to consume further keys at indent+2 belonging
                # to the same mapping item.
                while True:
                    nxt = self._peek()
                    if nxt is None:
                        break
                    _, nxt_indent, nxt_content = nxt
                    if nxt_indent <= indent:
                        break
                    if nxt_content.lstrip().startswith("- "):
                        break
                    sub_key, _, sub_rest = nxt_content.partition(":")
                    sub_rest = sub_rest.strip()
                    self.pos += 1
                    if sub_rest:
                        entry[self._scalar(sub_key.strip())] = self._scalar(
                            sub_rest
                        )
                    else:
                        entry[self._scalar(sub_key.strip())] = (
                            self._parse_block(nxt_indent + 2) or {}
                        )
                out.append(entry)
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
class DocSpec:
    path: str  # workspace-relative path of the generated doc (e.g. docs/generated/design/x.md)
    manifest_key: str  # key under documents: in manifest.yaml
    prompt: str  # path relative to _meta/
    watched_paths: list[str]
    depends_on: list[str] = field(default_factory=list)
    size_cap_bytes: int = 32768


@dataclass
class Manifest:
    version: int
    default_size_cap_bytes: int
    docs: dict[str, DocSpec]


def load_manifest() -> Manifest:
    if not MANIFEST_PATH.exists():
        raise SystemExit(f"manifest not found: {MANIFEST_PATH}")
    raw = _load_yaml(MANIFEST_PATH.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise SystemExit("manifest must be a mapping")
    version = int(raw.get("version", 1))
    default_cap = int(raw.get("default_size_cap_bytes", 32768))
    docs_in = raw.get("documents") or {}
    docs: dict[str, DocSpec] = {}
    for key, entry in docs_in.items():
        if not isinstance(entry, dict):
            raise SystemExit(f"document entry {key!r} is not a mapping")
        watched = entry.get("watched_paths") or []
        if not watched:
            raise SystemExit(f"document {key!r} has no watched_paths")
        spec = DocSpec(
            path=f"docs/generated/design/{key}",
            manifest_key=key,
            prompt=str(entry["prompt"]),
            watched_paths=[str(p) for p in watched],
            depends_on=[str(d) for d in entry.get("depends_on") or []],
            size_cap_bytes=int(entry.get("size_cap_bytes", default_cap)),
        )
        docs[key] = spec
    return Manifest(version=version, default_size_cap_bytes=default_cap, docs=docs)


# --------------------------------------------------------------------------
# Freshness state
# --------------------------------------------------------------------------


def load_freshness() -> dict:
    if not FRESHNESS_PATH.exists():
        return {"schema_version": 1, "documents": {}}
    return json.loads(FRESHNESS_PATH.read_text(encoding="utf-8"))


def save_freshness(state: dict) -> None:
    FRESHNESS_PATH.write_text(
        json.dumps(state, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def load_review_state() -> dict:
    if not REVIEW_STATE_PATH.exists():
        return {"schema_version": 1, "documents": {}}
    return json.loads(REVIEW_STATE_PATH.read_text(encoding="utf-8"))


def save_review_state(state: dict) -> None:
    REVIEW_STATE_PATH.write_text(
        json.dumps(state, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def _yaml_to_str(val) -> str:
    """Coerce a value loaded from YAML front-matter to a string.

    PyYAML decodes ISO 8601 timestamps as datetime objects; we want
    plain strings in review-state.json so the file stays JSON-friendly
    and round-trips losslessly.
    """
    if isinstance(val, _dt.datetime):
        if val.tzinfo is None:
            return val.isoformat()
        return val.isoformat()
    if isinstance(val, _dt.date):
        return val.isoformat()
    return str(val)


def _is_claude_family(model: str | None) -> bool:
    """True if `model` self-identifies as part of the Claude / Anthropic family.

    Matches case-insensitive presence of either `claude` or `anthropic` as a
    substring; this catches identifiers like `claude-opus-4.7`,
    `anthropic-claude-3.5-sonnet`, etc.
    """
    if not model:
        return False
    low = model.lower()
    return any(tok in low for tok in _CLAUDE_FAMILY_TOKENS)


# --------------------------------------------------------------------------
# Path / glob resolution
# --------------------------------------------------------------------------


def _expand_glob(pattern: str) -> list[Path]:
    """Expand a workspace-relative glob into a sorted list of files.

    Supports `**` for recursive matches. Directory results are walked into
    so that `prelude/*.h` returns header files but `source/slang/` returns
    every file under it.
    """
    if any(ch in pattern for ch in "*?["):
        if "**" in pattern:
            matches = list(REPO_ROOT.glob(pattern))
        else:
            matches = list(REPO_ROOT.glob(pattern))
    else:
        matches = [REPO_ROOT / pattern]
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


def resolve_watched_files(spec: DocSpec) -> list[Path]:
    seen: dict[Path, None] = {}
    for pat in spec.watched_paths:
        for f in _expand_glob(pat):
            seen[f] = None
    return sorted(seen.keys())


def compute_digest(spec: DocSpec) -> str:
    """SHA-256 over (relpath, size, contents) of every watched file."""
    h = hashlib.sha256()
    files = resolve_watched_files(spec)
    for f in files:
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


# --------------------------------------------------------------------------
# Git helpers
# --------------------------------------------------------------------------


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
# Front-matter parsing
# --------------------------------------------------------------------------

_FM_RE = re.compile(r"^---\n(.*?\n)---\n", re.DOTALL)


def parse_front_matter(text: str) -> dict | None:
    """Flat front-matter parser used by the generated-doc lint.

    Only handles `key: scalar` lines at the top level. Use
    parse_yaml_front_matter() for nested front-matter (review and
    remediation reports).
    """
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


def parse_yaml_front_matter(text: str) -> dict | None:
    """YAML-aware front-matter parser for review and remediation reports.

    Returns the parsed mapping, or None if the file has no front-matter or
    the body cannot be parsed as a mapping.
    """
    m = _FM_RE.match(text)
    if not m:
        return None
    try:
        data = _load_yaml(m.group(1))
    except (ValueError, Exception):  # pragma: no cover - tolerant by design
        return None
    if not isinstance(data, dict):
        return None
    return data


def _split_sections(body: str) -> dict[str, str]:
    """Split a Markdown body into top-level `## ` sections.

    Returns a dict mapping the section heading text (without the leading
    `## ` and trailing whitespace) to the section body (lines below the
    heading, up to but not including the next `## ` heading or EOF).
    Anything before the first `## ` heading is keyed under the empty string.
    """
    sections: dict[str, str] = {}
    current = ""
    buf: list[str] = []
    for line in body.splitlines():
        if line.startswith("## ") and not line.startswith("### "):
            sections[current] = "\n".join(buf)
            current = line[3:].strip()
            buf = []
        else:
            buf.append(line)
    sections[current] = "\n".join(buf)
    return sections


def _strip_md_cell(cell: str) -> str:
    return cell.strip()


def parse_markdown_table(section_body: str) -> list[dict[str, str]] | None:
    """Parse the first Markdown pipe table in `section_body`.

    Returns a list of row dicts (column header -> cell text) or None if no
    table is found. Tolerates leading/trailing pipes and the standard
    `| --- | --- |` separator row.
    """
    lines = [ln for ln in section_body.splitlines()]
    # Find the first header row (a `|`-delimited line) followed by a
    # separator row whose cells are all dashes / colons.
    n = len(lines)
    i = 0
    while i < n - 1:
        hdr = lines[i].strip()
        sep = lines[i + 1].strip()
        if "|" in hdr and "|" in sep and _is_separator_row(sep):
            headers = _split_md_row(hdr)
            rows: list[dict[str, str]] = []
            j = i + 2
            while j < n:
                rl = lines[j].strip()
                if not rl or "|" not in rl:
                    break
                cells = _split_md_row(rl)
                if len(cells) != len(headers):
                    break
                rows.append({h: _strip_md_cell(c) for h, c in zip(headers, cells)})
                j += 1
            return rows
        i += 1
    return None


def _is_separator_row(line: str) -> bool:
    cells = _split_md_row(line)
    if not cells:
        return False
    for c in cells:
        s = c.strip().replace(":", "")
        if not s or any(ch != "-" for ch in s):
            return False
    return True


def _split_md_row(line: str) -> list[str]:
    s = line.strip()
    if s.startswith("|"):
        s = s[1:]
    if s.endswith("|"):
        s = s[:-1]
    return [c.strip() for c in s.split("|")]


# --------------------------------------------------------------------------
# Lint
# --------------------------------------------------------------------------

_LINK_RE = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
_REQUIRED_FM_KEYS = (
    "generated",
    "model",
    "generated_at",
    "source_commit",
    "watched_paths_digest",
    "warning",
)


@dataclass
class LintIssue:
    doc: str
    severity: str  # "error" | "warning"
    message: str


def lint_doc(spec: DocSpec) -> list[LintIssue]:
    issues: list[LintIssue] = []
    p = REPO_ROOT / spec.path
    if not p.exists():
        issues.append(LintIssue(spec.path, "error", "document does not exist"))
        return issues
    text = p.read_text(encoding="utf-8")
    fm = parse_front_matter(text)
    if fm is None:
        issues.append(
            LintIssue(spec.path, "error", "missing or malformed YAML front-matter")
        )
    else:
        for k in _REQUIRED_FM_KEYS:
            if k not in fm:
                issues.append(
                    LintIssue(spec.path, "error", f"front-matter missing key: {k}")
                )
        if fm.get("generated", "").lower() != "true":
            issues.append(
                LintIssue(spec.path, "error", "front-matter generated must be true")
            )
    size = len(text.encode("utf-8"))
    if size > spec.size_cap_bytes:
        issues.append(
            LintIssue(
                spec.path,
                "warning",
                f"size {size} exceeds cap {spec.size_cap_bytes}",
            )
        )
    for link in _LINK_RE.findall(text):
        target = link.split("#", 1)[0].split("?", 1)[0]
        if not target:
            continue
        if target.startswith(("http://", "https://", "mailto:")):
            continue
        # Resolve relative to the document's directory
        candidate = (p.parent / target).resolve()
        if not candidate.exists():
            issues.append(
                LintIssue(
                    spec.path, "error", f"link target does not resolve: {target}"
                )
            )
    return issues


# --------------------------------------------------------------------------
# Review / remediation report parsing
# --------------------------------------------------------------------------

_REVIEW_REQUIRED_FM_KEYS = (
    "review_report",
    "reviewer_model",
    "reviewed_at",
    "target_doc",
    "target_doc_source_commit",
    "target_doc_watched_paths_digest",
    "source_commit",
    "checklist",
    "finding_count",
    "severity_breakdown",
)

_REMEDIATION_REQUIRED_FM_KEYS = (
    "remediation_report",
    "remediator_model",
    "remediated_at",
    "target_doc",
    "review_report",
    "target_doc_source_commit_before",
    "target_doc_source_commit_after",
    "actions",
)

_REVIEW_CHECKLIST_KEYS = (
    "factual_accuracy",
    "cross_references",
    "completeness",
    "style_consistency",
    "source_alignment",
    "front_matter_validity",
)

_REVIEW_SEVERITIES = ("critical", "major", "minor", "nit")
_REMEDIATION_ACTION_KEYS = (
    "fixed",
    "rejected_bogus",
    "rejected_out_of_scope",
    "deferred",
    "escalated",
)

# Allowed action values in the Actions table body. Keys are kebab-case here
# (e.g. `rejected-bogus`) but the front-matter counts use snake_case keys
# (`rejected_bogus`); we map between the two.
_ACTION_BODY_TO_FM = {
    "fixed": "fixed",
    "rejected-bogus": "rejected_bogus",
    "rejected-out-of-scope": "rejected_out_of_scope",
    "deferred": "deferred",
    "escalated": "escalated",
}


def _report_rel(path: Path) -> str:
    return _rel_to_repo(path)


def _coerce_int(val) -> int | None:
    if isinstance(val, bool):
        return None
    if isinstance(val, int):
        return val
    if isinstance(val, str) and val.strip().lstrip("-").isdigit():
        return int(val.strip())
    return None


def lint_review_report(
    path: Path, manifest: Manifest
) -> tuple[list[LintIssue], dict | None]:
    """Validate a review report file.

    Returns (issues, front_matter_dict_or_None). The caller can use the
    returned front-matter to drive `mark-reviewed`.
    """
    rel = _report_rel(path)
    issues: list[LintIssue] = []
    if not path.exists():
        return [LintIssue(rel, "error", "review report does not exist")], None
    text = path.read_text(encoding="utf-8")
    fm = parse_yaml_front_matter(text)
    if fm is None:
        return [
            LintIssue(rel, "error", "missing or malformed YAML front-matter")
        ], None

    for k in _REVIEW_REQUIRED_FM_KEYS:
        if k not in fm:
            issues.append(LintIssue(rel, "error", f"front-matter missing key: {k}"))

    sentinel = fm.get("review_report")
    if sentinel is not True and str(sentinel).lower() != "true":
        issues.append(
            LintIssue(rel, "error", "front-matter review_report must be true")
        )

    reviewer = fm.get("reviewer_model")
    if not isinstance(reviewer, str) or not reviewer.strip():
        issues.append(LintIssue(rel, "error", "reviewer_model must be a non-empty string"))
    elif _is_claude_family(reviewer):
        issues.append(
            LintIssue(
                rel,
                "error",
                f"reviewer_model {reviewer!r} appears to be a Claude/Anthropic"
                " model; the review step requires a different model family",
            )
        )

    target = fm.get("target_doc")
    if not isinstance(target, str) or target not in manifest.docs:
        issues.append(
            LintIssue(
                rel,
                "error",
                f"target_doc {target!r} is not a known manifest key",
            )
        )

    checklist = fm.get("checklist")
    if not isinstance(checklist, dict):
        issues.append(LintIssue(rel, "error", "checklist must be a mapping"))
    else:
        for k in _REVIEW_CHECKLIST_KEYS:
            v = checklist.get(k)
            if v not in ("pass", "partial", "fail"):
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        f"checklist.{k} must be one of pass/partial/fail (got {v!r})",
                    )
                )

    finding_count = _coerce_int(fm.get("finding_count"))
    if finding_count is None or finding_count < 0:
        issues.append(
            LintIssue(rel, "error", "finding_count must be a non-negative integer")
        )

    sev = fm.get("severity_breakdown")
    sev_sum: int | None = 0
    if not isinstance(sev, dict):
        issues.append(LintIssue(rel, "error", "severity_breakdown must be a mapping"))
        sev_sum = None
    else:
        for k in _REVIEW_SEVERITIES:
            v = _coerce_int(sev.get(k))
            if v is None or v < 0:
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        f"severity_breakdown.{k} must be a non-negative integer",
                    )
                )
                sev_sum = None
            elif sev_sum is not None:
                sev_sum += v
    if (
        finding_count is not None
        and sev_sum is not None
        and finding_count != sev_sum
    ):
        issues.append(
            LintIssue(
                rel,
                "error",
                f"severity_breakdown sums to {sev_sum} but finding_count is"
                f" {finding_count}",
            )
        )

    # Validate the Findings table in the body.
    body = text[text.find("\n---\n") + 5 :] if "\n---\n" in text else ""
    sections = _split_sections(body)
    if "Findings" not in sections:
        issues.append(LintIssue(rel, "error", "missing `## Findings` section"))
    else:
        findings_body = sections["Findings"].strip()
        if findings_body.startswith("(no findings)"):
            if finding_count not in (None, 0):
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        "Findings section is `(no findings)` but"
                        f" finding_count is {finding_count}",
                    )
                )
        else:
            table = parse_markdown_table(findings_body)
            if table is None:
                issues.append(
                    LintIssue(
                        rel, "error", "`## Findings` must contain a Markdown table"
                    )
                )
            else:
                if finding_count is not None and len(table) != finding_count:
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            f"Findings table has {len(table)} rows but"
                            f" finding_count is {finding_count}",
                        )
                    )
                seen_ids: set[str] = set()
                for idx, row in enumerate(table, start=1):
                    rid = row.get("ID", "")
                    if not rid:
                        issues.append(
                            LintIssue(rel, "error", f"row {idx}: missing ID")
                        )
                        continue
                    if rid in seen_ids:
                        issues.append(
                            LintIssue(rel, "error", f"duplicate finding ID: {rid}")
                        )
                    seen_ids.add(rid)
                    sev_v = row.get("Severity", "").lower()
                    if sev_v not in _REVIEW_SEVERITIES:
                        issues.append(
                            LintIssue(
                                rel,
                                "error",
                                f"{rid}: Severity {sev_v!r} not in"
                                f" {list(_REVIEW_SEVERITIES)}",
                            )
                        )
                    for col in ("Description", "Evidence", "Recommendation"):
                        if not row.get(col, "").strip():
                            issues.append(
                                LintIssue(
                                    rel,
                                    "error",
                                    f"{rid}: {col} cell is empty",
                                )
                            )
    return issues, fm


def lint_remediation_report(
    path: Path, manifest: Manifest
) -> tuple[list[LintIssue], dict | None]:
    """Validate a remediation report file.

    Returns (issues, front_matter_dict_or_None).
    """
    rel = _report_rel(path)
    issues: list[LintIssue] = []
    if not path.exists():
        return [LintIssue(rel, "error", "remediation report does not exist")], None
    text = path.read_text(encoding="utf-8")
    fm = parse_yaml_front_matter(text)
    if fm is None:
        return [
            LintIssue(rel, "error", "missing or malformed YAML front-matter")
        ], None

    for k in _REMEDIATION_REQUIRED_FM_KEYS:
        if k not in fm:
            issues.append(LintIssue(rel, "error", f"front-matter missing key: {k}"))

    sentinel = fm.get("remediation_report")
    if sentinel is not True and str(sentinel).lower() != "true":
        issues.append(
            LintIssue(rel, "error", "front-matter remediation_report must be true")
        )

    remediator = fm.get("remediator_model")
    if not isinstance(remediator, str) or not remediator.strip():
        issues.append(
            LintIssue(rel, "error", "remediator_model must be a non-empty string")
        )
    elif not _is_claude_family(remediator):
        issues.append(
            LintIssue(
                rel,
                "error",
                f"remediator_model {remediator!r} does not look like a"
                " Claude/Anthropic model; remediation must be performed by"
                " the same family that generated the docs",
            )
        )

    target = fm.get("target_doc")
    if not isinstance(target, str) or target not in manifest.docs:
        issues.append(
            LintIssue(
                rel,
                "error",
                f"target_doc {target!r} is not a known manifest key",
            )
        )

    review_ref = fm.get("review_report")
    review_fm: dict | None = None
    if not isinstance(review_ref, str) or not review_ref.strip():
        issues.append(
            LintIssue(rel, "error", "review_report must be a non-empty string")
        )
    else:
        # Resolve relative to the remediation report's own directory.
        review_path = (path.parent / review_ref).resolve()
        if not review_path.exists():
            issues.append(
                LintIssue(
                    rel, "error", f"review_report path does not resolve: {review_ref}"
                )
            )
        else:
            review_text = review_path.read_text(encoding="utf-8")
            review_fm = parse_yaml_front_matter(review_text)
            if review_fm is None:
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        "linked review report has malformed front-matter",
                    )
                )
            else:
                if review_fm.get("target_doc") != target:
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            "linked review report's target_doc"
                            f" {review_fm.get('target_doc')!r} does not match"
                            f" remediation target_doc {target!r}",
                        )
                    )

    actions = fm.get("actions")
    action_sum: int | None = 0
    if not isinstance(actions, dict):
        issues.append(LintIssue(rel, "error", "actions must be a mapping"))
        action_sum = None
    else:
        for k in _REMEDIATION_ACTION_KEYS:
            v = _coerce_int(actions.get(k))
            if v is None or v < 0:
                issues.append(
                    LintIssue(
                        rel,
                        "error",
                        f"actions.{k} must be a non-negative integer",
                    )
                )
                action_sum = None
            elif action_sum is not None:
                action_sum += v

    if review_fm is not None and action_sum is not None:
        review_fc = _coerce_int(review_fm.get("finding_count"))
        if review_fc is not None and review_fc != action_sum:
            issues.append(
                LintIssue(
                    rel,
                    "error",
                    f"actions sum to {action_sum} but linked review"
                    f" finding_count is {review_fc}",
                )
            )

    # Cross-check the Actions table body against the linked review's
    # Findings table.
    body = text[text.find("\n---\n") + 5 :] if "\n---\n" in text else ""
    sections = _split_sections(body)
    if "Actions" not in sections:
        issues.append(LintIssue(rel, "error", "missing `## Actions` section"))
    else:
        actions_table = parse_markdown_table(sections["Actions"])
        if actions_table is None:
            issues.append(
                LintIssue(rel, "error", "`## Actions` must contain a Markdown table")
            )
        else:
            review_ids: set[str] = set()
            if review_fm is not None and isinstance(review_ref, str):
                # Pull finding IDs from the review's Findings table.
                rv_body = review_text[review_text.find("\n---\n") + 5 :]
                rv_sections = _split_sections(rv_body)
                rv_findings_body = rv_sections.get("Findings", "").strip()
                if not rv_findings_body.startswith("(no findings)"):
                    rv_table = parse_markdown_table(rv_findings_body)
                    if rv_table is not None:
                        review_ids = {r.get("ID", "") for r in rv_table if r.get("ID")}
            seen: set[str] = set()
            for idx, row in enumerate(actions_table, start=1):
                fid = row.get("Finding ID", "")
                if not fid:
                    issues.append(
                        LintIssue(rel, "error", f"row {idx}: missing Finding ID")
                    )
                    continue
                if fid in seen:
                    issues.append(
                        LintIssue(rel, "error", f"duplicate Finding ID: {fid}")
                    )
                seen.add(fid)
                action_v = row.get("Action", "").lower()
                if action_v not in _ACTION_BODY_TO_FM:
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            f"{fid}: Action {action_v!r} not in"
                            f" {list(_ACTION_BODY_TO_FM)}",
                        )
                    )
                if not row.get("Rationale", "").strip():
                    issues.append(
                        LintIssue(rel, "error", f"{fid}: Rationale cell is empty")
                    )
                if action_v == "fixed" and not row.get("Fix summary", "").strip():
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            f"{fid}: Fix summary required when Action is `fixed`",
                        )
                    )
            if review_ids:
                missing = sorted(review_ids - seen)
                extra = sorted(seen - review_ids)
                for fid in missing:
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            f"finding {fid} from linked review has no action row",
                        )
                    )
                for fid in extra:
                    issues.append(
                        LintIssue(
                            rel,
                            "error",
                            f"action row {fid} does not correspond to any"
                            " finding in the linked review",
                        )
                    )
    return issues, fm


def _iter_report_files(root: Path, suffix: str) -> list[Path]:
    if not root.exists():
        return []
    return sorted(p for p in root.rglob(f"*{suffix}") if p.is_file())


# --------------------------------------------------------------------------
# Subcommand implementations
# --------------------------------------------------------------------------


def cmd_list(_args, manifest: Manifest) -> int:
    for key in sorted(manifest.docs):
        print(key)
    return 0


def cmd_list_stale(args, manifest: Manifest) -> int:
    state = load_freshness()
    docs_state = state.get("documents", {})
    review_state = load_review_state() if args.include_review else None
    review_docs = (review_state or {}).get("documents", {})
    any_stale = False
    for key in sorted(manifest.docs):
        spec = manifest.docs[key]
        cur = compute_digest(spec)
        rec = docs_state.get(key)
        if rec is None:
            base = f"missing  {key}"
            any_stale = True
        elif rec.get("watched_paths_digest") != cur:
            base = f"stale    {key}"
            any_stale = True
        else:
            base = f"fresh    {key}"
        if args.include_review:
            status = _compute_review_status(key, manifest, review_docs)
            print(f"{base}  [{status}]")
        else:
            print(base)
    return 1 if any_stale else 0


def _doc_front_matter_digest(spec: DocSpec) -> str | None:
    p = REPO_ROOT / spec.path
    if not p.exists():
        return None
    fm = parse_front_matter(p.read_text(encoding="utf-8"))
    if fm is None:
        return None
    return fm.get("watched_paths_digest") or None


def _compute_review_status(
    key: str, manifest: Manifest, review_docs: dict
) -> str:
    """Return one of:

        unreviewed
        review-stale
        reviewed-pending-remediation
        remediated
    """
    entry = review_docs.get(key)
    if not entry or "last_reviewed" not in entry:
        return "unreviewed"
    spec = manifest.docs[key]
    doc_digest = _doc_front_matter_digest(spec)
    last_reviewed = entry["last_reviewed"]
    if (
        doc_digest is None
        or last_reviewed.get("target_doc_watched_paths_digest") != doc_digest
    ):
        return "review-stale"
    last_remediated = entry.get("last_remediated")
    if last_remediated is None:
        return "reviewed-pending-remediation"
    if last_remediated.get("review_report_ref") != last_reviewed.get("report_path"):
        return "reviewed-pending-remediation"
    return "remediated"


def cmd_review_status(args, manifest: Manifest) -> int:
    review_state = load_review_state()
    review_docs = review_state.get("documents", {})
    keys: list[str]
    if args.docs:
        for d in args.docs:
            if d not in manifest.docs:
                print(f"error: unknown doc {d!r}", file=sys.stderr)
                return 2
        keys = list(args.docs)
    else:
        keys = sorted(manifest.docs)
    for key in keys:
        status = _compute_review_status(key, manifest, review_docs)
        line = f"{status:<32}{key}"
        if args.show_counts:
            entry = review_docs.get(key, {})
            lr = entry.get("last_reviewed")
            lm = entry.get("last_remediated")
            extras: list[str] = []
            if lr:
                sev = lr.get("severity_breakdown", {})
                extras.append(
                    "review:"
                    f" findings={lr.get('finding_count', 0)}"
                    f" critical={sev.get('critical', 0)}"
                    f" major={sev.get('major', 0)}"
                    f" minor={sev.get('minor', 0)}"
                    f" nit={sev.get('nit', 0)}"
                )
            if lm:
                act = lm.get("actions", {})
                extras.append(
                    "remediation:"
                    f" fixed={act.get('fixed', 0)}"
                    f" bogus={act.get('rejected_bogus', 0)}"
                    f" oos={act.get('rejected_out_of_scope', 0)}"
                    f" deferred={act.get('deferred', 0)}"
                    f" escalated={act.get('escalated', 0)}"
                )
            if extras:
                line += "  " + "  ".join(extras)
        print(line)
    return 0


def _default_review_report_path(doc_key: str) -> Path:
    return REVIEWS_DIR / f"{doc_key}.review.md"


def _default_remediation_report_path(doc_key: str) -> Path:
    return REMEDIATIONS_DIR / f"{doc_key}.remediation.md"


def cmd_mark_reviewed(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    report_path = (
        Path(args.report).resolve() if args.report
        else _default_review_report_path(args.doc)
    )
    issues, fm = lint_review_report(report_path, manifest)
    if issues:
        for issue in issues:
            print(f"{issue.severity}: {issue.doc}: {issue.message}", file=sys.stderr)
        if any(i.severity == "error" for i in issues):
            return 1
    if fm is None:
        print("error: review report could not be parsed", file=sys.stderr)
        return 1
    if fm.get("target_doc") != args.doc:
        print(
            f"error: review report target_doc {fm.get('target_doc')!r} does not"
            f" match <doc> {args.doc!r}",
            file=sys.stderr,
        )
        return 1
    state = load_review_state()
    state.setdefault("schema_version", 1)
    state.setdefault("documents", {})
    entry = state["documents"].setdefault(args.doc, {})
    entry["last_reviewed"] = {
        "reviewer_model": _yaml_to_str(fm["reviewer_model"]),
        "reviewed_at": _yaml_to_str(fm["reviewed_at"]),
        "target_doc_source_commit": _yaml_to_str(fm["target_doc_source_commit"]),
        "target_doc_watched_paths_digest": _yaml_to_str(
            fm["target_doc_watched_paths_digest"]
        ),
        "finding_count": int(fm["finding_count"]),
        "severity_breakdown": {
            k: int(fm["severity_breakdown"][k]) for k in _REVIEW_SEVERITIES
        },
        "report_path": _report_rel(report_path),
    }
    save_review_state(state)
    print(f"marked reviewed: {args.doc}")
    _ = spec  # spec is required to validate args.doc; not otherwise used here.
    return 0


def cmd_mark_remediated(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    report_path = (
        Path(args.report).resolve() if args.report
        else _default_remediation_report_path(args.doc)
    )
    issues, fm = lint_remediation_report(report_path, manifest)
    if issues:
        for issue in issues:
            print(f"{issue.severity}: {issue.doc}: {issue.message}", file=sys.stderr)
        if any(i.severity == "error" for i in issues):
            return 1
    if fm is None:
        print("error: remediation report could not be parsed", file=sys.stderr)
        return 1
    if fm.get("target_doc") != args.doc:
        print(
            f"error: remediation report target_doc {fm.get('target_doc')!r} does"
            f" not match <doc> {args.doc!r}",
            file=sys.stderr,
        )
        return 1
    review_ref = fm.get("review_report") or ""
    # Store review_report_ref as a workspace-relative path so the
    # ledger comparison in _compute_review_status is unambiguous.
    review_path = (report_path.parent / review_ref).resolve()
    review_ref_rel = _report_rel(review_path)
    state = load_review_state()
    state.setdefault("schema_version", 1)
    state.setdefault("documents", {})
    entry = state["documents"].setdefault(args.doc, {})
    entry["last_remediated"] = {
        "remediator_model": _yaml_to_str(fm["remediator_model"]),
        "remediated_at": _yaml_to_str(fm["remediated_at"]),
        "review_report_ref": review_ref_rel,
        "actions": {
            k: int(fm["actions"][k]) for k in _REMEDIATION_ACTION_KEYS
        },
        "report_path": _report_rel(report_path),
    }
    save_review_state(state)
    print(f"marked remediated: {args.doc}")
    _ = spec
    return 0


def cmd_digest(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    print(compute_digest(spec))
    return 0


def cmd_show(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    print(f"doc:           {spec.path}")
    print(f"prompt:        docs/generated/design/_meta/{spec.prompt}")
    print(f"size_cap:      {spec.size_cap_bytes}")
    print(f"depends_on:    {', '.join(spec.depends_on) or '(none)'}")
    print(f"watched_paths:")
    for p in spec.watched_paths:
        print(f"  - {p}")
    print(f"resolved files:")
    for f in resolve_watched_files(spec):
        print(f"  {_rel_to_repo(f)}")
    return 0


def cmd_mark_fresh(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    state = load_freshness()
    state.setdefault("schema_version", 1)
    state.setdefault("documents", {})
    state["documents"][args.doc] = {
        "source_commit": args.commit or head_commit(),
        "watched_paths_digest": compute_digest(spec),
        "generated_at": _dt.datetime.now(_dt.timezone.utc).isoformat(timespec="seconds"),
        "model": args.model or "unspecified",
    }
    save_freshness(state)
    print(f"marked fresh: {args.doc}")
    return 0


def cmd_lint(args, manifest: Manifest) -> int:
    targets: Iterable[str]
    explicit_targets = bool(args.docs)
    if explicit_targets:
        for d in args.docs:
            if d not in manifest.docs:
                print(f"error: unknown doc {d!r}", file=sys.stderr)
                return 2
        targets = args.docs
    else:
        targets = sorted(manifest.docs.keys())
    any_error = False
    for key in targets:
        spec = manifest.docs[key]
        for issue in lint_doc(spec):
            print(f"{issue.severity}: {issue.doc}: {issue.message}")
            if issue.severity == "error":
                any_error = True
    # Lint review and remediation reports. When the caller passed explicit
    # doc keys we restrict the report lint to the matching report files;
    # otherwise we lint every report file found under _meta/reviews/ and
    # _meta/remediations/.
    review_files: list[Path] = []
    remediation_files: list[Path] = []
    if explicit_targets:
        for d in args.docs:
            rp = _default_review_report_path(d)
            if rp.exists():
                review_files.append(rp)
            mp = _default_remediation_report_path(d)
            if mp.exists():
                remediation_files.append(mp)
    else:
        review_files = _iter_report_files(REVIEWS_DIR, ".review.md")
        remediation_files = _iter_report_files(REMEDIATIONS_DIR, ".remediation.md")
    for rp in review_files:
        issues, _ = lint_review_report(rp, manifest)
        for issue in issues:
            print(f"{issue.severity}: {issue.doc}: {issue.message}")
            if issue.severity == "error":
                any_error = True
    for mp in remediation_files:
        issues, _ = lint_remediation_report(mp, manifest)
        for issue in issues:
            print(f"{issue.severity}: {issue.doc}: {issue.message}")
            if issue.severity == "error":
                any_error = True
    return 1 if any_error else 0


def _require_doc(manifest: Manifest, key: str) -> DocSpec:
    if key not in manifest.docs:
        raise SystemExit(f"unknown doc: {key}")
    return manifest.docs[key]


# --------------------------------------------------------------------------
# Argument parser
# --------------------------------------------------------------------------


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="regenerate.py",
        description="Driver for the LLM-generated documentation pipeline.",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("list", help="list documents in the manifest")

    p_stale = sub.add_parser(
        "list-stale", help="list documents whose digest has changed"
    )
    p_stale.add_argument(
        "--include-review",
        action="store_true",
        help="also annotate each row with its review/remediation status",
    )

    p_digest = sub.add_parser("digest", help="print current digest of a doc")
    p_digest.add_argument("doc")

    p_show = sub.add_parser("show", help="show resolved manifest entry")
    p_show.add_argument("doc")

    p_mark = sub.add_parser("mark-fresh", help="record a fresh entry for a doc")
    p_mark.add_argument("doc")
    p_mark.add_argument("--commit", default=None, help="override source_commit")
    p_mark.add_argument("--model", default=None, help="record model identifier")

    p_lint = sub.add_parser("lint", help="run structural lint on docs and reports")
    p_lint.add_argument("docs", nargs="*", help="doc keys; default: all")

    p_review = sub.add_parser(
        "review-status",
        help="show review / remediation freshness for documents",
    )
    p_review.add_argument("docs", nargs="*", help="doc keys; default: all")
    p_review.add_argument(
        "--show-counts",
        action="store_true",
        help="append severity / action counts for documents with state",
    )

    p_mark_rev = sub.add_parser(
        "mark-reviewed",
        help="record a review entry for <doc> from a review report file",
    )
    p_mark_rev.add_argument("doc")
    p_mark_rev.add_argument(
        "--report",
        default=None,
        help="path to the review report; default _meta/reviews/<doc>.review.md",
    )

    p_mark_rem = sub.add_parser(
        "mark-remediated",
        help="record a remediation entry for <doc> from a remediation report",
    )
    p_mark_rem.add_argument("doc")
    p_mark_rem.add_argument(
        "--report",
        default=None,
        help="path to the remediation report; default"
        " _meta/remediations/<doc>.remediation.md",
    )

    return p


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    manifest = load_manifest()
    handlers = {
        "list": cmd_list,
        "list-stale": cmd_list_stale,
        "digest": cmd_digest,
        "show": cmd_show,
        "mark-fresh": cmd_mark_fresh,
        "lint": cmd_lint,
        "review-status": cmd_review_status,
        "mark-reviewed": cmd_mark_reviewed,
        "mark-remediated": cmd_mark_remediated,
    }
    return handlers[args.cmd](args, manifest)


if __name__ == "__main__":
    sys.exit(main())
