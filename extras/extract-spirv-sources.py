#!/usr/bin/env python3
"""
Extract embedded source code from a SPIR-V disassembly (text dump) and write them to files.

Supported patterns:
- OpString: collects string literals, often filenames.
- OpSource (+ OpSourceContinued): collects source text and maps to optional file id.
- NonSemantic.Shader.DebugInfo.100: DebugSource (+ DebugSourceContinued) basic support.

Usage:
  python bin/extract-spirv-sources.py path/to/spirv_disasm.txt -o out_dir [--overwrite] [--verbose]

Assumptions:
- Input is a text disassembly like from `spirv-dis` (SPIRV-Tools).
- Filenames come via OpString ids or quoted strings in DebugSource; otherwise generated.

Limitations:
- Partial support for DebugSourceComposite and other advanced debug constructs.
- If multiple distinct sources map to the same filename, numeric suffixes will be added.
"""
from __future__ import annotations

import argparse
import os
import re
import sys
import ast
from pathlib import Path
from typing import Dict, List, Tuple, Optional

# -----------------
# Utilities
# -----------------

_QUOTED_RE = re.compile(r'"([^"\\]*(?:\\.[^"\\]*)*)"')
_ID_RE = re.compile(r"%\d+")

LANG_EXTS = {
    "GLSL": "glsl",
    "ESSL": "glsl",
    "HLSL": "hlsl",
    "OpenCL_C": "cl",
    "OpenCL_CPP": "cl",
    "Unknown": "txt",
}

DEBUG_IMPORT_NAMES = {
    "NonSemantic.Shader.DebugInfo.100",
    "OpenCL.DebugInfo.100",
}


def _unescape_spirv_string(s: str) -> str:
    """Unescape SPIR-V quoted string content to Python str."""
    try:
        # Use literal_eval on a quoted string to handle escapes safely.
        return ast.literal_eval('"' + s + '"')
    except Exception:
        # Fallback minimal unescapes
        return (
            s.replace(r"\n", "\n")
             .replace(r"\t", "\t")
             .replace(r"\r", "\r")
             .replace(r"\"", '"')
             .replace(r"\\", "\\")
        )


def _find_quoted_strings(line: str) -> List[str]:
    return [m for m in _QUOTED_RE.findall(line)]


def _looks_like_path(s: str) -> bool:
    if not s:
        return False
    # Multi-line strings are source content, not paths
    if "\n" in s or "\r" in s:
        return False
    # URLs are not file paths
    if s.startswith("http://") or s.startswith("https://"):
        return False
    # Windows drive path or backslashes
    if re.match(r"^[A-Za-z]:[\\/]", s):
        return True
    if "\\" in s:
        return True
    # Forward-slash paths: require no spaces and a plausible extension at the end
    if "/" in s and " " not in s:
        name = os.path.basename(s)
        if re.search(r"\.[A-Za-z0-9]{1,5}$", name):
            return True
    return False


def _safe_target_path(out_dir: Path, proposed: str) -> Path:
    """Join a proposed path under out_dir safely (avoid absolute drives and parent escapes).

    On Windows, absolute paths like "J:\\..." or drive-qualified segments like "J:" must not
    break the out_dir anchor. We'll flatten to a relative path by splitting on separators and
    filtering unsafe segments ("..", ".", empty, any containing ':').
    """
    # Split on both kinds of separators and filter
    tokens = re.split(r"[\\\\/]+", proposed.strip())
    safe_parts: List[str] = []
    for part in tokens:
        if not part or part in (".", ".."):
            continue
        # Drop drive letters or segments containing ':'
        if ":" in part:
            continue
        safe_parts.append(part)
    if not safe_parts:
        # Fallback to basename without any ':' characters
        base = os.path.basename(proposed) or "source.txt"
        base = base.replace(":", "_")
        safe_parts = [base]
    target = out_dir.joinpath(*safe_parts)
    # Ensure target resides within out_dir
    try:
        target_resolved = target.resolve(strict=False)
        out_resolved = out_dir.resolve(strict=False)
        try:
            same_root = os.path.commonpath([str(target_resolved), str(out_resolved)]) == str(out_resolved)
        except Exception:
            # Different drives or invalid common path -> treat as unsafe
            same_root = False
        if not same_root:
            target = out_dir / (Path(proposed).name.replace(":", "_") or "source.txt")
    except Exception:
        pass
    return target


def _unique_path(base: Path) -> Path:
    if not base.exists():
        return base
    stem = base.stem
    suffix = base.suffix
    parent = base.parent
    i = 1
    while True:
        cand = parent / f"{stem}_{i}{suffix}"
        if not cand.exists():
            return cand
        i += 1


# -----------------
# Parsing logic
# -----------------

class SpirvSourceExtractor:
    def __init__(self, verbose: bool = False) -> None:
        self.verbose = verbose
        self.id_to_string: Dict[str, str] = {}
        self.debug_import_ids: set[str] = set()
        self.lang_counters: Dict[str, int] = {}
        # Map from DebugFile id -> resolved file path string (dir/name)
        self.debug_file_map: Dict[str, str] = {}

    def _log(self, msg: str) -> None:
        if self.verbose:
            print(msg, file=sys.stderr)

    def first_pass_collect(self, lines: List[str]) -> None:
        # Helpers to parse a possibly multi-line quoted string starting in tail
        def parse_quoted(start_tail: str, idx: int) -> Tuple[str, int]:
            # Find the first quote if not present in start_tail, skip until found
            tail = start_tail
            i = idx
            # Move to first quote
            while True:
                qpos = tail.find('"')
                if qpos != -1:
                    break
                # Advance to next line if no quote found
                i += 1
                if i >= len(lines):
                    return "", i
                tail = lines[i]
            # Consume starting quote
            content_chars: List[str] = []
            escaped = False
            # Process characters after the opening quote on current line
            j = qpos + 1
            while True:
                if j >= len(tail):
                    # End of line -> add newline and continue with next line
                    content_chars.append("\n")
                    i += 1
                    if i >= len(lines):
                        break
                    tail = lines[i]
                    j = 0
                    continue
                ch = tail[j]
                if escaped:
                    content_chars.append(ch)
                    escaped = False
                    j += 1
                else:
                    if ch == '\\':
                        escaped = True
                        j += 1
                    elif ch == '"':
                        # End of quoted string
                        j += 1
                        break
                    else:
                        content_chars.append(ch)
                        j += 1
            return "".join(content_chars), i

        # Patterns
        import_re = re.compile(r"^\s*(%\d+)\s*=\s*OpExtInstImport\s+\"([^\"]+)\"\s*$")
        debug_file_re = re.compile(r"^\s*(%\d+)\s*=\s*OpExtInst\s+%\w+\s+%\d+\s+DebugFile\b(.*)$")

        i = 0
        n = len(lines)
        while i < n:
            line = lines[i]
            # OpString with possibly multi-line quoted content
            m_os = re.match(r"^\s*(%\d+)\s*=\s*OpString\s+(.*)$", line)
            if m_os:
                sid, tail = m_os.groups()
                s, new_i = parse_quoted(tail, i)
                self.id_to_string[sid] = s
                i = new_i + 1
                continue
            # DebugString with possibly multi-line quoted content
            m_ds = re.match(r"^\s*(%\d+)\s*=\s*OpExtInst\s+%\w+\s+%\d+\s+DebugString\s+(.*)$", line)
            if m_ds:
                sid, tail = m_ds.groups()
                s, new_i = parse_quoted(tail, i)
                self.id_to_string[sid] = s
                i = new_i + 1
                continue
            # OpExtInstImport
            m2 = import_re.match(line)
            if m2:
                id_, name = m2.groups()
                if name in DEBUG_IMPORT_NAMES:
                    self.debug_import_ids.add(id_)
                i += 1
                continue
            # DebugFile
            m4 = debug_file_re.match(line)
            if m4:
                file_id, tail = m4.groups()
                q = _find_quoted_strings(tail)
                ids = _ID_RE.findall(tail)
                dir_str: Optional[str] = None
                name_str: Optional[str] = None
                if ids:
                    if len(ids) >= 1:
                        dir_str = self.id_to_string.get(ids[0])
                    if len(ids) >= 2:
                        name_str = self.id_to_string.get(ids[1])
                if (not dir_str or not name_str) and q:
                    if dir_str is None and len(q) >= 1:
                        dir_str = _unescape_spirv_string(q[0])
                    if name_str is None and len(q) >= 2:
                        name_str = _unescape_spirv_string(q[1])
                if name_str:
                    if dir_str and dir_str not in (".", ""):
                        fp = os.path.join(dir_str, name_str)
                    else:
                        fp = name_str
                    self.debug_file_map[file_id] = fp
                i += 1
                continue
            i += 1
        if self.verbose:
            self._log(f"Collected strings: {len(self.id_to_string)}; Debug imports: {self.debug_import_ids}; DebugFile entries: {len(self.debug_file_map)}")

    def _gen_filename(self, lang: str) -> str:
        ext = LANG_EXTS.get(lang, LANG_EXTS["Unknown"]) or LANG_EXTS["Unknown"]
        n = self.lang_counters.get(lang, 0) + 1
        self.lang_counters[lang] = n
        return f"source_{n}.{ext}"

    def _concat_quoted_chunks(self, chunks: List[str]) -> str:
        # Concatenate without injecting newlines; string chunks may contain \n
        return "".join(_unescape_spirv_string(c) for c in chunks)

    def parse_op_source_blocks(self, lines: List[str]) -> List[Tuple[str, str]]:
        """Return list of (filename, content) from OpSource/Continued blocks."""
        results: List[Tuple[str, str]] = []
        i = 0
        # OpSource GLSL 450 [%id] ["..."]
        head_re = re.compile(r"^\s*OpSource\s+(\w+)\s+(\d+)(?:\s+(%\d+))?(.*)$")
        cont_re = re.compile(r"^\s*OpSourceContinued\s+(.*)$")
        total = len(lines)
        while i < total:
            line = lines[i]
            mh = head_re.match(line)
            if not mh:
                i += 1
                continue
            lang, _ver, file_id, tail = mh.groups()
            chunks = _find_quoted_strings(tail or "")
            # Collect continuations
            j = i + 1
            while j < total:
                mcont = cont_re.match(lines[j])
                if not mcont:
                    break
                cont_tail = mcont.group(1)
                chunks.extend(_find_quoted_strings(cont_tail or ""))
                j += 1
            content = self._concat_quoted_chunks(chunks)
            # Determine filename
            filename: str
            if file_id and file_id in self.id_to_string:
                filename = self.id_to_string[file_id]
            else:
                filename = self._gen_filename(lang)
            results.append((filename, content))
            self._log(f"OpSource -> {filename} ({len(content)} chars)")
            i = j
        return results

    def parse_debug_sources(self, lines: List[str]) -> List[Tuple[str, str]]:
        """Return list of (filename, content) from NonSemantic DebugInfo DebugSource blocks."""
        results: List[Tuple[str, str]] = []
        if not self.debug_import_ids:
            return results
        # Example: %31 = OpExtInst %void %100 DebugSource %24 "..."
        # Continued: OpExtInst %void %100 DebugSourceContinued "..."
        ext_line_re = re.compile(r"^\s*(?:%\d+\s*=\s*)?OpExtInst\b(.*)$")
        # We'll parse tokens manually to find ext id and instruction name
        i = 0
        total = len(lines)
        while i < total:
            line = lines[i]
            if not ext_line_re.match(line):
                i += 1
                continue
            # Tokenize (preserve %ids and words)
            tokens = re.findall(r"%\d+|\w+|\"[^\"]*(?:\\.[^\"]*)*\"", line)
            try:
                # Expect pattern: [...]= OpExtInst %type %extId InstName ...
                # Find index of 'OpExtInst'
                oi = tokens.index('OpExtInst')
                # ext id should be two positions after: %type, then %extId
                ext_id = None
                if oi + 2 < len(tokens) and tokens[oi + 2].startswith('%'):
                    ext_id = tokens[oi + 2]
                inst_name = tokens[oi + 3] if oi + 3 < len(tokens) else None
            except ValueError:
                i += 1
                continue
            if not ext_id or ext_id not in self.debug_import_ids:
                i += 1
                continue
            if inst_name not in ('DebugSource', 'DebugSourceContinued'):
                i += 1
                continue
            if inst_name == 'DebugSource':
                # Determine filename and initial content
                # Operands start after inst name position
                operands = tokens[oi + 4:]
                # Extract ids and quoted strings among operands only
                id_ops = [t for t in operands if t.startswith('%')]
                q_ops = [t.strip('"') for t in operands if t.startswith('"')]
                # Candidate filename: DebugFile id, else any string-looking id/path
                filename: Optional[str] = None
                for id_ in id_ops:
                    if id_ in self.debug_file_map:
                        filename = self.debug_file_map[id_]
                        break
                if filename is None:
                    for id_ in id_ops:
                        s = self.id_to_string.get(id_)
                        if s and _looks_like_path(s):
                            filename = s
                            break
                if filename is None:
                    # Try quoted operands for a path
                    for q in q_ops:
                        uq = _unescape_spirv_string(q)
                        if _looks_like_path(uq):
                            filename = uq
                            break
                # Content: all quoted strings after inst name plus any id operands that map to strings
                content_chunks: List[str] = []
                # Add string ids (that are not used as file mapping via DebugFile) and do not look like paths
                for id_ in id_ops:
                    if id_ in self.debug_file_map:
                        continue
                    s = self.id_to_string.get(id_)
                    if s and not _looks_like_path(s) and (filename is None or s != filename):
                        content_chunks.append(s)
                # Add quoted chunks
                for q in q_ops:
                    uq = _unescape_spirv_string(q)
                    if not (_looks_like_path(uq) and filename and uq == filename):
                        content_chunks.append(uq)
                # Collect continuations
                j = i + 1
                while j < total:
                    nxt = lines[j]
                    if 'OpExtInst' not in nxt or 'DebugSourceContinued' not in nxt:
                        break
                    # Ensure same ext id is used on continuation line
                    # Parse continuation tokens
                    tokens2 = re.findall(r"%\d+|\w+|\"[^\"]*(?:\\.[^\"]*)*\"", nxt)
                    try:
                        oi2 = tokens2.index('OpExtInst')
                        ext2 = tokens2[oi2 + 2] if oi2 + 2 < len(tokens2) else None
                        name2 = tokens2[oi2 + 3] if oi2 + 3 < len(tokens2) else None
                    except ValueError:
                        break
                    if ext2 != ext_id or name2 != 'DebugSourceContinued':
                        break
                    ops2 = tokens2[oi2 + 4:]
                    # Append any string id contents and quoted strings
                    for tkn in ops2:
                        if tkn.startswith('%'):
                            s2 = self.id_to_string.get(tkn)
                            if s2 and not _looks_like_path(s2) and (filename is None or s2 != filename):
                                content_chunks.append(s2)
                        elif tkn.startswith('"'):
                            uq2 = _unescape_spirv_string(tkn.strip('"'))
                            if not (_looks_like_path(uq2) and filename and uq2 == filename):
                                content_chunks.append(uq2)
                    j += 1
                content = "".join(content_chunks)
                if not filename:
                    filename = self._gen_filename('Unknown')
                results.append((filename, content))
                self._log(f"DebugSource -> {filename} ({len(content)} chars)")
                i = j
                continue
            # If we encounter a bare DebugSourceContinued without a preceding DebugSource in our scan, skip
            i += 1
        return results

    def extract_sources_from_text(self, text: str) -> List[Tuple[str, str]]:
        lines = text.splitlines()
        self.first_pass_collect(lines)
        out: List[Tuple[str, str]] = []
        out.extend(self.parse_op_source_blocks(lines))
        out.extend(self.parse_debug_sources(lines))
        return out


# -----------------
# File I/O helpers
# -----------------

def write_files(pairs: List[Tuple[str, str]], out_dir: Path, overwrite: bool = False, verbose: bool = False) -> List[Path]:
    written: List[Path] = []
    out_dir.mkdir(parents=True, exist_ok=True)
    seen_targets: Dict[str, int] = {}
    for fname, content in pairs:
        target = _safe_target_path(out_dir, fname)
        # Ensure directory exists
        target.parent.mkdir(parents=True, exist_ok=True)
        write_path = target
        if not overwrite and target.exists():
            # If same content, skip
            try:
                existing = target.read_text(encoding='utf-8', errors='ignore')
                if existing == content:
                    if verbose:
                        print(f"Skip (unchanged) {target}", file=sys.stderr)
                    continue
            except Exception:
                pass
            write_path = _unique_path(target)
        write_path.write_text(content, encoding='utf-8', newline='')
        if verbose:
            print(f"Wrote {write_path}", file=sys.stderr)
        written.append(write_path)
    return written


# -----------------
# CLI
# -----------------

def main(argv: Optional[List[str]] = None) -> int:
    p = argparse.ArgumentParser(description="Extract embedded source code from SPIR-V disassembly text.")
    p.add_argument('input', help='Path to SPIR-V disassembly text file (e.g., from spirv-dis)')
    p.add_argument('-o', '--out-dir', help='Output directory for extracted sources (default: <input>_sources)')
    p.add_argument('--overwrite', action='store_true', help='Overwrite existing files when names collide')
    p.add_argument('--list', action='store_true', help='List extracted files without writing to disk')
    p.add_argument('-v', '--verbose', action='store_true', help='Verbose logging to stderr')
    args = p.parse_args(argv)

    in_path = Path(args.input)
    if not in_path.exists():
        print(f"Input not found: {in_path}", file=sys.stderr)
        return 2
    text = in_path.read_text(encoding='utf-8', errors='ignore')
    extractor = SpirvSourceExtractor(verbose=args.verbose)
    pairs = extractor.extract_sources_from_text(text)

    if args.list:
        for fname, content in pairs:
            print(f"{fname} : {len(content)} bytes")
        return 0

    out_dir = Path(args.out_dir) if args.out_dir else in_path.with_suffix('')
    # If default, append suffix to avoid overwriting input stem
    if not args.out_dir:
        out_dir = Path(str(out_dir) + '_sources')
    written = write_files(pairs, out_dir, overwrite=args.overwrite, verbose=args.verbose)
    print(f"Extracted {len(written)} file(s) to {out_dir}")
    return 0


if __name__ == '__main__':
    sys.exit(main())
