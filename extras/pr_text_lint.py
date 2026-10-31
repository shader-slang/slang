#!/usr/bin/env python3
"""Check a pull-request title and description against the four-section format.

The description becomes the squash commit message, so AGENTS.md ("Commit & Pull
Request Guidelines") asks for four sections -- Motivation, Approach, Key choices,
Evidence -- of at most three lines each, under 1,500 characters in total, with no
Markdown table. Text inside a ``<details>`` block, HTML comments, ``Fixes #N`` lines, the
``AI assistance:`` line and a trailing ``<sub>`` disclaimer are not counted; that is where the
process report and other long material belong.

Usage::

    pr_text_lint.py --title "..." --body-file body.md [--format github] [--enforce]
    pr_text_lint.py --self-test

Prints one line per problem, each ending with the fix. Exit status is 0 unless
``--enforce`` is given and a problem was found (then 1). ``--format github``
emits ``::warning::``/``::error::`` annotations for a workflow log. Limits come
from the environment: PR_TEXT_MAX_CHARS (1500), PR_TEXT_MAX_SECTION_LINES (3),
PR_TEXT_TITLE_MAX (72). ``--require-ai-line`` also requires the one-line AI
assistance disclosure (``AI assistance: None | Editorial | Substantive ...``).
"""

from __future__ import annotations

import argparse
import os
import re
import sys

REQUIRED_SECTIONS = ("Motivation", "Approach", "Key choices", "Evidence")

HTML_COMMENT_RE = re.compile(r"<!--.*?-->", re.DOTALL)
# Block-level only: a `<details>` mentioned inline (say, in a code span) must not open a block.
DETAILS_RE = re.compile(r"^[ \t]*<details\b.*?^[ \t]*</details>[ \t]*$", re.DOTALL | re.IGNORECASE | re.MULTILINE)
HEADING_RE = re.compile(r"^\s{0,3}#{1,6}\s+(?P<label>.+?)\s*#*\s*$")
BOLD_LABEL_RE = re.compile(r"^\s*(\*\*|__)\s*(?P<label>[^*_\n]{1,80}?)\s*[.:]?\s*\1[.:]?(\s|$)")
ISSUE_REF = r"(?:(?:[\w.-]+/[\w.-]+)?#\d+|https://github\.com/[\w.-]+/[\w.-]+/(?:issues|pull)/\d+)"
CLOSING_WORD = r"(?:fix(?:es|ed)?|close[sd]?|resolve[sd]?)"
CLOSING_RE = re.compile(
    rf"^\s*{CLOSING_WORD}\s*:?\s+{ISSUE_REF}(?:\s*(?:,|and)\s*(?:{CLOSING_WORD}\s*:?\s+)?{ISSUE_REF})*\s*\.?\s*$",
    re.IGNORECASE,
)
DISCLAIMER_RE = re.compile(r"^\s*<sub>.*</sub>\s*$", re.IGNORECASE)
# The template's unfilled `Fixes #` placeholder.
CLOSING_PLACEHOLDER_RE = re.compile(rf"^\s*{CLOSING_WORD}\s*:?\s*#?\s*$", re.IGNORECASE)
DETAILS_MARKER = "\x00details\x00"
# Inline code spans are blanked before any structural match, so a `<details>` or `## x`
# quoted in prose (even one wrapped across lines) is never read as structure.
CODE_SPAN_RE = re.compile(r"`[^`]*`")
TABLE_SEP_RE = re.compile(r"^\s*\|?\s*:?-+:?\s*(\|\s*:?-+:?\s*)*\|?\s*$")
AI_LINE_RE = re.compile(
    r"^\s*(\*\*|__)?\s*AI assistance\s*(\*\*|__)?\s*[:.]\s*(?P<value>None|Editorial|Substantive)\b",
    re.IGNORECASE,
)


def env_int(name: str, default: int) -> int:
    value = os.environ.get(name, "")
    return int(value) if value.isdigit() and int(value) > 0 else default


def counted_lines(body: str) -> tuple[list[str], int]:
    """Return (section lines, counted characters) for a description.

    HTML comments (the template's hints) are removed. A block-level ``<details>``
    block is removed and the lines after it are a footer: they count toward the
    character total but belong to no section. Closing-keyword lines (filled or
    the template's bare ``Fixes #``), the ``AI assistance:`` line and a final
    ``<sub>`` disclaimer are trailers and are not counted at all.
    """
    text = body.replace("\r\n", "\n")
    text = HTML_COMMENT_RE.sub("", text)
    text = CODE_SPAN_RE.sub(lambda m: "`" + "x" * (len(m.group(0)) - 2) + "`", text)
    text = DETAILS_RE.sub(DETAILS_MARKER, text)
    lines = text.split("\n")
    last = max((i for i, ln in enumerate(lines) if ln.strip()), default=-1)
    if last >= 0 and DISCLAIMER_RE.match(lines[last]):
        lines = lines[:last] + lines[last + 1 :]
    lines = [
        ln for ln in lines
        if not (CLOSING_RE.match(ln) or CLOSING_PLACEHOLDER_RE.match(ln) or AI_LINE_RE.match(ln))
    ]
    total = len("\n".join(ln for ln in lines if DETAILS_MARKER not in ln).strip())
    section_lines = lines[: next((i for i, ln in enumerate(lines) if DETAILS_MARKER in ln), len(lines))]
    return section_lines, total


def has_table(lines: list[str]) -> bool:
    prev = ""
    for ln in lines:
        if ln.strip() and "|" in ln and TABLE_SEP_RE.match(ln) and "|" in prev:
            return True
        if ln.strip():
            prev = ln
    return False


def sections_of(lines: list[str]) -> list[tuple[str, int]]:
    """(label, content-line count) for every ``## Heading`` or ``**Label**`` section."""
    sections: list[list] = []
    for ln in lines:
        if not ln.strip():
            continue
        m = HEADING_RE.match(ln) or BOLD_LABEL_RE.match(ln)
        if m:
            sections.append([m.group("label").strip().rstrip(".:"), 0])
        elif sections:
            sections[-1][1] += 1
    return [(label, n) for label, n in sections]


def check(title: str, body: str, *, require_ai_line: bool = False) -> list[str]:
    """Return every problem as one sentence that ends with the fix; empty when fine."""
    max_chars = env_int("PR_TEXT_MAX_CHARS", 1500)
    max_lines = env_int("PR_TEXT_MAX_SECTION_LINES", 3)
    title_max = env_int("PR_TEXT_TITLE_MAX", 72)
    problems: list[str] = []

    title = title.strip()
    if len(title) > title_max:
        problems.append(f"Title is {len(title)} characters, max {title_max}: shorten it to `component: imperative summary`.")
    if title.endswith("."):
        problems.append("Title ends with a period: drop it, the title is a commit subject.")

    lines, total = counted_lines(body)
    present = {label.lower(): n for label, n in sections_of(lines)}
    missing = [s for s in REQUIRED_SECTIONS if s.lower() not in present]
    if missing:
        problems.append(
            "Missing section(s) " + ", ".join(f"`## {s}`" for s in missing)
            + ": use the four headings from .github/PULL_REQUEST_TEMPLATE.md."
        )
    for label, n in sections_of(lines):
        if n > max_lines:
            problems.append(
                f"`{label}` has {n} lines, max {max_lines}: keep the point, move the rest into the <details> block."
            )
    if has_table(lines):
        problems.append("The description contains a Markdown table: move it into the <details> block or drop it.")
    if total > max_chars:
        problems.append(
            f"Description is {total:,} characters outside <details>, max {max_chars:,}: move the process report and traces into the <details> block."
        )
    if require_ai_line and not any(AI_LINE_RE.match(ln) for ln in body.split("\n")):
        problems.append(
            "Missing the AI assistance line: add `AI assistance: None`, `Editorial`, or `Substantive: <tool>, verified by <tests or checks>`."
        )
    return problems


def self_test() -> int:
    good = (
        "## Motivation\nThe lexer dropped the last token when a file had no final newline.\n\n"
        "## Approach\nTreat end-of-input as a token boundary in the lexer, not in the parser.\n\n"
        "## Key choices\nNo new token kind; the fix lives where the boundary is decided.\n\n"
        "## Evidence\nAdded tests/lexer/no-final-newline.slang; slang-test passes on CPU.\n\n"
        "Fixes #1\n\n<details><summary>Process report</summary>\n" + "x" * 5000 + "\n</details>\n"
    )
    assert check("lexer: keep the last token without a final newline", good) == [], check("t", good)
    bad = "## Motivation\n" + "\n".join(["line"] * 4) + "\n\n| a | b |\n|---|---|\n| 1 | 2 |\n"
    problems = check("A very " + "long " * 20 + "title.", bad)
    joined = "\n".join(problems)
    for needle in ("Title is", "ends with a period", "Missing section", "has 7 lines", "table"):
        assert needle in joined, (needle, joined)
    inline = good.replace("No new token kind;", "Long material goes in `<details>`;")
    assert check("lexer: keep", inline) == [], check("lexer: keep", inline)
    wrapped = good.replace("No new token kind;", "Long material goes in `\n<details>`;")
    assert check("lexer: keep", wrapped) == [], check("lexer: keep", wrapped)
    assert check("t", good, require_ai_line=True)[0].startswith("Missing the AI assistance line")
    assert check("t", good + "\nAI assistance: None\n", require_ai_line=True) == []
    # Trailers and the footer after <details> do not count as section lines.
    tail = good.replace("Fixes #1\n", "Fixes #\n\nAI assistance: None\n") + "\nGenerated with a tool\n"
    assert check("lexer: keep", tail) == [], check("lexer: keep", tail)
    print("ok")
    return 0


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--title", default="")
    ap.add_argument("--body", default=None)
    ap.add_argument("--body-file", default=None)
    ap.add_argument("--format", choices=("text", "github"), default="text")
    ap.add_argument("--enforce", action="store_true", help="exit 1 when a problem is found")
    ap.add_argument("--require-ai-line", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        return self_test()
    body = args.body if args.body is not None else ""
    if args.body_file:
        with open(args.body_file, encoding="utf-8") as f:
            body = f.read()
    problems = check(args.title, body, require_ai_line=args.require_ai_line)
    level = "error" if args.enforce else "warning"
    for p in problems:
        print(f"::{level}::{p}" if args.format == "github" else p)
    if not problems:
        print("PR text OK" if args.format == "text" else "::notice::PR text follows the four-section format")
    return 1 if problems and args.enforce else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
