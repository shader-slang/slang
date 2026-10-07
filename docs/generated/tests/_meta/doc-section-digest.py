#!/usr/bin/env python3
"""Compute //META: doc_section_digest for a doc#anchor.

doc_section_digest is the sha256 of the cited section's text: the body
lines from the heading whose GitHub-style id is <anchor> up to (but not
including) the next same-or-higher-level heading. Matches the rule in
docs/generated/tests/_meta/prompts/_common.md.

Usage:
    doc-section-digest.py <doc-path> <anchor>
    doc-section-digest.py docs/generated/design/name-resolution/overload-resolution.md conversion-costs
"""
import hashlib
import re
import sys


def slug(heading_text: str) -> str:
    """Return the GitHub-flavored anchor slug for a heading.

    Lowercase, drop everything but word chars / space / hyphen (this strips
    backticks and punctuation), then map each remaining whitespace character to
    one hyphen.

    Each space maps to its own hyphen — runs are *not* collapsed. That is what
    GitHub does, and it is what the rest of this pipeline assumes: a heading
    like `### Downstream DXC / fxc` loses the slash and keeps both surrounding
    spaces, so its anchor is `downstream-dxc--fxc` with two hyphens. Collapsing
    the run here produced `downstream-dxc-fxc`, which matches no `doc_ref` any
    bundle records, so `section_text` raised "anchor not found" and every test
    anchored to such a heading was silently skipped by the digest lint — 104
    tests across 32 anchors were exempt from the drift tripwire because of it.
    """
    s = heading_text.strip().lower()
    s = re.sub(r"[^\w\s-]", "", s)
    s = re.sub(r"\s", "-", s)
    return s


def section_text(path: str, anchor: str) -> str:
    lines = open(path, encoding="utf-8").read().splitlines()
    start = None
    start_level = None
    for i, ln in enumerate(lines):
        m = re.match(r"^(#{1,6})\s+(.*)$", ln)
        if not m:
            continue
        level = len(m.group(1))
        if slug(m.group(2)) == anchor:
            start = i + 1
            start_level = level
            break
    if start is None:
        raise SystemExit(f"anchor not found: {anchor} in {path}")
    end = len(lines)
    for j in range(start, len(lines)):
        m = re.match(r"^(#{1,6})\s+", lines[j])
        if m and len(m.group(1)) <= start_level:
            end = j
            break
    # Body lines of the section (between the two headings).
    body = "\n".join(lines[start:end])
    return body


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    body = section_text(sys.argv[1], sys.argv[2])
    print(hashlib.sha256(body.encode("utf-8")).hexdigest())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
