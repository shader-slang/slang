#!/usr/bin/env python3
"""Warn about source comments that carry process, history or deprecated terms.

A source comment documents the code as it stands. Reviewers refuse comments that carry
issue and PR numbers, history ("before this change", "fixes #N") or the deprecated
``ConstRef`` term for what the IR calls a ``borrow in``. This script reads a unified diff
and reports every added comment line in a C, C++, CUDA, Slang or Lua file outside ``tests/``
(a regression test names its issue by convention) that matches one of those rules, as GitHub workflow annotations plus a summary.

Usage:
    extras/source_comment_lint.py --base <sha> --head <sha>     # git diff base...head
    extras/source_comment_lint.py --diff-file <path>            # a saved unified diff
    git diff -U0 master | extras/source_comment_lint.py         # a diff on stdin

The exit status is 0 unless ``--enforce`` is given, so the workflow warns by default.
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys

SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx", ".cu", ".cuh", ".h", ".hpp", ".hxx", ".inl", ".slang", ".lua"}
# A regression test names the issue it covers by convention, so test trees are not checked.
SKIPPED_PREFIXES = ("tests/", "external/")

REFERENCE_RE = re.compile(
    r"(?<![\w/])#\d{3,}\b|\b(?:issues|pull)/\d+\b|\b(?:PR|pull request|issue)\s*#?\s*\d{3,}\b", re.IGNORECASE
)
HISTORY_RE = re.compile(
    r"\bbefore this (?:change|fix|patch|commit)\b|\bpreviously\b|\bused to\b|\bhistorically\b|\bthis PR\b|\bfixes\s*#",
    re.IGNORECASE,
)
DEPRECATED_RE = re.compile(r"\bConstRef\b|\bconst[ -]ref\b", re.IGNORECASE)
RULES = (
    ("issue or PR reference", REFERENCE_RE, "keep issue and PR numbers in the commit message and the PR description"),
    ("history", HISTORY_RE, "a comment documents the code as it stands; history belongs to source control"),
    ("deprecated term", DEPRECATED_RE, "the concept is an immutable borrow: say `borrow in`, not ConstRef"),
)


def added_lines(diff: str) -> list[tuple[str, int, str]]:
    """(path, new line number, text) for every added line of a unified diff."""
    rows: list[tuple[str, int, str]] = []
    path = None
    line = 0
    for raw in diff.splitlines():
        if raw.startswith("+++ "):
            target = raw[4:].strip()
            path = None if target == "/dev/null" else (target[2:] if target.startswith("b/") else target)
        elif raw.startswith("@@"):
            m = re.search(r"\+(\d+)(?:,(\d+))?", raw)
            line = int(m.group(1)) if m else 0
        elif raw.startswith("+") and not raw.startswith("+++"):
            if path is not None:
                rows.append((path, line, raw[1:]))
            line += 1
        elif raw.startswith("-") or raw.startswith("\\"):
            continue
        else:
            line += 1
    return rows


def comment_text(line: str, in_block: bool) -> tuple[str, bool]:
    """The comment part of one C-like source line, and whether a block comment stays open."""
    text: list[str] = []
    i = 0
    n = len(line)
    while i < n:
        if in_block:
            end = line.find("*/", i)
            if end < 0:
                text.append(line[i:])
                return " ".join(text), True
            text.append(line[i:end])
            i = end + 2
            in_block = False
            continue
        if line.startswith("//", i):
            text.append(line[i + 2 :])
            return " ".join(text), False
        if line.startswith("/*", i):
            in_block = True
            i += 2
            continue
        if line[i] in "\"'":
            quote = line[i]
            i += 1
            while i < n and line[i] != quote:
                i += 2 if line[i] == "\\" else 1
            i += 1
            continue
        i += 1
    return " ".join(text), in_block


def findings(rows: list[tuple[str, int, str]]) -> list[tuple[str, int, str, str, str]]:
    """(path, line, rule, advice, comment) for every added comment line that breaks a rule."""
    out = []
    block_state: dict[str, bool] = {}
    for path, line, text in rows:
        ext = os.path.splitext(path)[1].lower()
        if ext not in SOURCE_EXTS or path.startswith(SKIPPED_PREFIXES):
            continue
        if ext == ".lua":
            m = re.search(r"--(.*)$", text)
            comment = m.group(1) if m else ""
        else:
            comment, block_state[path] = comment_text(text, block_state.get(path, False))
        if not comment.strip():
            continue
        for name, rx, advice in RULES:
            if rx.search(comment):
                out.append((path, line, name, advice, comment.strip()))
                break
    return out


def read_diff(args: argparse.Namespace) -> str:
    if args.diff_file:
        with open(args.diff_file, encoding="utf-8", errors="replace") as fh:
            return fh.read()
    if args.base and args.head:
        out = subprocess.run(
            ["git", "diff", "-U0", "--no-color", "--no-ext-diff", f"{args.base}...{args.head}"],
            capture_output=True,
            text=True,
            check=True,
        )
        return out.stdout
    return sys.stdin.read()


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--base", help="merge-base side of the range, with --head")
    ap.add_argument("--head", help="head commit of the range, with --base")
    ap.add_argument("--diff-file", help="a saved unified diff instead of git")
    ap.add_argument("--enforce", action="store_true", help="exit 1 when there are findings")
    ap.add_argument("--no-annotations", action="store_true", help="plain lines instead of ::warning annotations")
    args = ap.parse_args(argv)
    hits = findings(added_lines(read_diff(args)))
    for path, line, rule, advice, comment in hits:
        if args.no_annotations:
            print(f"{path}:{line}: {rule}: {comment[:120]}")
        else:
            print(f"::warning file={path},line={line},title=Source comment: {rule}::{advice}. Comment: {comment[:160]}")
    if hits:
        print(f"{len(hits)} added comment line(s) carry process, history or a deprecated term.")
    else:
        print("No added comment carries process, history or a deprecated term.")
    return 1 if (hits and args.enforce) else 0


if __name__ == "__main__":
    sys.exit(main())
