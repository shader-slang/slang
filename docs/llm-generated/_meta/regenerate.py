#!/usr/bin/env python3
"""Driver for the LLM-generated documentation pipeline.

Tracks which documents under docs/llm-generated/ are stale relative to the
source paths they declare in manifest.yaml, and lints generated documents
for structural conformance.

This script does NOT invoke any agent. Generation itself is performed
out-of-band by an operator running an agent against the prompt template
listed in the manifest. Once a document has been (re)generated, run

    regenerate.py mark-fresh <doc>

to record its provenance in freshness.json.

Subcommands
-----------

    list                  List all documents in the manifest.
    list-stale            List documents whose watched-paths digest differs
                          from the recorded freshness entry (or that have
                          no freshness entry yet).
    digest <doc>          Print the current digest of a document's watched
                          paths.
    show <doc>            Print the manifest entry plus the resolved file
                          list for a document.
    mark-fresh <doc>      Record a fresh entry for <doc> using the current
                          digest, HEAD commit, and now() as the generation
                          timestamp. Pass --commit / --model to override.
    lint [<doc>...]       Run the structural linter (front-matter present,
                          paths resolve, size cap respected) on the given
                          documents (default: all).

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
REPO_ROOT = DOCS_ROOT.parent.parent
MANIFEST_PATH = META_DIR / "manifest.yaml"
FRESHNESS_PATH = META_DIR / "freshness.json"


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
    path: str  # workspace-relative path of the generated doc (e.g. docs/llm-generated/x.md)
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
            path=f"docs/llm-generated/{key}",
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
# Subcommand implementations
# --------------------------------------------------------------------------


def cmd_list(_args, manifest: Manifest) -> int:
    for key in sorted(manifest.docs):
        print(key)
    return 0


def cmd_list_stale(_args, manifest: Manifest) -> int:
    state = load_freshness()
    docs_state = state.get("documents", {})
    any_stale = False
    for key in sorted(manifest.docs):
        spec = manifest.docs[key]
        cur = compute_digest(spec)
        rec = docs_state.get(key)
        if rec is None:
            print(f"missing  {key}")
            any_stale = True
            continue
        if rec.get("watched_paths_digest") != cur:
            print(f"stale    {key}")
            any_stale = True
        else:
            print(f"fresh    {key}")
    return 1 if any_stale else 0


def cmd_digest(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    print(compute_digest(spec))
    return 0


def cmd_show(args, manifest: Manifest) -> int:
    spec = _require_doc(manifest, args.doc)
    print(f"doc:           {spec.path}")
    print(f"prompt:        docs/llm-generated/_meta/{spec.prompt}")
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
    if args.docs:
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
    sub.add_parser("list-stale", help="list documents whose digest has changed")

    p_digest = sub.add_parser("digest", help="print current digest of a doc")
    p_digest.add_argument("doc")

    p_show = sub.add_parser("show", help="show resolved manifest entry")
    p_show.add_argument("doc")

    p_mark = sub.add_parser("mark-fresh", help="record a fresh entry for a doc")
    p_mark.add_argument("doc")
    p_mark.add_argument("--commit", default=None, help="override source_commit")
    p_mark.add_argument("--model", default=None, help="record model identifier")

    p_lint = sub.add_parser("lint", help="run structural lint on docs")
    p_lint.add_argument("docs", nargs="*", help="doc keys; default: all")

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
    }
    return handlers[args.cmd](args, manifest)


if __name__ == "__main__":
    sys.exit(main())
