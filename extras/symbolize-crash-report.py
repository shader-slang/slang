#!/usr/bin/env python3
"""Add function names and source lines to the stack of a Slang crash report.

A crash report prints each stack frame as `module+0xoffset`, because release builds of Slang
cannot name most of their own functions. This script turns those frames into names with the
binaries and the debug files of the same release.

Consider a report from slangc 2026.22.1 on Linux:

    [stack]
    #0  libslang-compiler.so.0.2026.22.1+0x3a1786
    #1  libslang-compiler.so.0.2026.22.1+0x3a09f1

Download the release package and its `-debug-info` package, unpack both into one directory,
and run:

    extras/symbolize-crash-report.py report.txt --symbols slang-2026.22.1-linux-x86_64

The script prints the report with each frame followed by its function and, where the debug
files have it, its source line:

    #0  libslang-compiler.so.0.2026.22.1+0x3a1786  Slang::emitInst(...)  slang-emit-spirv.cpp:1234

The script uses `llvm-symbolizer` if it can find it, which reads ELF, Mach-O and PDB. Without
it, it uses `addr2line` for ELF and `atos` for Mach-O. It reads the report from a file or from
standard input and never changes the `[report]` or `[details]` parts.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

# A frame line of the `[stack]` part: `#3  libslang-compiler.so+0x1f0c2a`.
FRAME_RE = re.compile(r"^(?P<prefix>#(?P<index>\d+)\s+)(?P<module>[^\s+][^\s]*?)\+0x(?P<offset>[0-9a-fA-F]+)\s*$")

# A `key: value` line of the `[report]` part.
FIELD_RE = re.compile(r"^(?P<key>[a-z][a-z-]*):\s*(?P<value>.*?)\s*$")


@dataclass
class Frame:
    """One frame of the `[stack]` part, with the line it came from."""

    line_number: int
    index: int
    module: str
    offset: int


@dataclass
class Report:
    """The parts of a crash report that symbolizing needs."""

    lines: List[str]
    fields: Dict[str, str]
    frames: List[Frame]


def parse_report(text: str) -> Report:
    """Return the lines of `text`, the fields of its `[report]` part and its stack frames.

    Only the first report in `text` is read, from its `==== Slang crash report` line on.
    """
    lines = text.splitlines()
    fields: Dict[str, str] = {}
    frames: List[Frame] = []
    section = None
    started = False
    for number, raw in enumerate(lines):
        line = raw.rstrip("\r")
        if line.startswith("==== Slang crash report"):
            if started:
                break
            started = True
            continue
        if not started:
            continue
        stripped = line.strip()
        if stripped.startswith("[") and stripped.endswith("]"):
            section = stripped
            continue
        if section == "[report]":
            match = FIELD_RE.match(stripped)
            if match and match.group("key") not in fields:
                fields[match.group("key")] = match.group("value")
        elif section == "[stack]":
            match = FRAME_RE.match(stripped)
            if match:
                frames.append(
                    Frame(
                        line_number=number,
                        index=int(match.group("index")),
                        module=match.group("module"),
                        offset=int(match.group("offset"), 16),
                    )
                )
    return Report(lines=lines, fields=fields, frames=frames)


def lookup_address(frame: Frame, kind: str) -> int:
    """Return the offset to look up for `frame`, relative to the start of its module.

    Every frame is a return address, which points after the call, so we look up the byte
    before it to land on the call itself. The one exception is the first frame of a crash: it
    is the faulting instruction, whose address is exact.
    """
    if kind == "crash" and frame.index == 0:
        return frame.offset
    return max(frame.offset - 1, 0)


# --- Binary formats ---------------------------------------------------------------------------


def read_image_base(path: Path) -> Tuple[str, int]:
    """Return the format of the binary at `path` and the address that a module offset counts from.

    A report's offset is the distance from where the loader placed the start of the module. For
    ELF that start is the first loaded segment, for Mach-O the `__TEXT` segment, and for PE the
    image itself, whose offsets are relative virtual addresses. The returned base turns an
    offset into the address that the debug information uses.
    """
    with open(path, "rb") as file:
        header = file.read(64)
        if header[:4] == b"\x7fELF":
            return "elf", _read_elf_base(file, header)
        magic = struct.unpack("<I", header[:4])[0]
        if magic in (0xFEEDFACF, 0xFEEDFACE):
            return "macho", _read_macho_text_base(file, header, magic == 0xFEEDFACF)
        if header[:2] == b"MZ":
            return "pe", 0
    raise ValueError(f"{path}: not an ELF, Mach-O or PE file")


def _read_elf_base(file, header: bytes) -> int:
    """Return the page-aligned virtual address of the first loadable segment of an ELF file."""
    is64 = header[4] == 2
    endian = "<" if header[5] == 1 else ">"
    if is64:
        phoff, = struct.unpack(endian + "Q", header[32:40])
        phentsize, phnum = struct.unpack(endian + "HH", header[54:58])
    else:
        phoff, = struct.unpack(endian + "I", header[28:32])
        phentsize, phnum = struct.unpack(endian + "HH", header[42:46])
    lowest = None
    for i in range(phnum):
        file.seek(phoff + i * phentsize)
        entry = file.read(phentsize)
        p_type, = struct.unpack(endian + "I", entry[:4])
        if p_type != 1:  # PT_LOAD
            continue
        if is64:
            p_vaddr, = struct.unpack(endian + "Q", entry[16:24])
            p_align, = struct.unpack(endian + "Q", entry[48:56])
        else:
            p_vaddr, = struct.unpack(endian + "I", entry[8:12])
            p_align, = struct.unpack(endian + "I", entry[28:32])
        base = p_vaddr & ~(max(p_align, 1) - 1)
        lowest = base if lowest is None else min(lowest, base)
    return lowest or 0


def _read_macho_text_base(file, header: bytes, is64: bool) -> int:
    """Return the virtual address of the `__TEXT` segment of a thin Mach-O file."""
    ncmds, = struct.unpack("<I", header[16:20])
    offset = 32 if is64 else 28
    for _ in range(ncmds):
        file.seek(offset)
        cmd, cmdsize = struct.unpack("<II", file.read(8))
        if cmd in (0x19, 0x1):  # LC_SEGMENT_64, LC_SEGMENT
            name = file.read(16).rstrip(b"\0")
            if name == b"__TEXT":
                return struct.unpack("<Q" if is64 else "<I", file.read(8 if is64 else 4))[0]
        offset += cmdsize
    return 0


# --- Finding the files of a module --------------------------------------------------------------


def find_module_files(module: str, directories: Sequence[Path]) -> Tuple[Optional[Path], Optional[Path]]:
    """Return the binary named `module` and its debug file, searching `directories` recursively.

    The release packages keep the debug information of a binary in a file of its own:
    `<binary>.dwarf` on Linux and macOS, and `<binary stem>.pdb` on Windows.
    """
    stem = os.path.splitext(module)[0]
    wanted_debug = {module + ".dwarf", stem + ".pdb"}
    binary = None
    debug = None
    for directory in directories:
        for root, _, files in os.walk(directory):
            for name in files:
                if binary is None and name == module:
                    binary = Path(root, name)
                elif debug is None and name in wanted_debug:
                    debug = Path(root, name)
            if binary and debug:
                return binary, debug
    return binary, debug


# --- Symbolizers --------------------------------------------------------------------------------


@dataclass
class Symbol:
    """What a symbolizer found for one address."""

    function: str
    location: str = ""


def parse_llvm_symbolizer_json(output: str) -> List[Optional[Symbol]]:
    """Return one symbol for each address in `llvm-symbolizer --output-style=JSON` output.

    Given addresses on its command line, llvm-symbolizer prints one JSON array with an object
    for each address; given them on standard input, it prints one object per line. We accept
    both. When the address is in inlined code, the innermost function comes first and the
    function that it was inlined into is added to the name.
    """
    results: List[dict] = []
    for line in output.splitlines():
        line = line.strip()
        if not line:
            continue
        value = json.loads(line)
        results.extend(value if isinstance(value, list) else [value])

    symbols: List[Optional[Symbol]] = []
    for result in results:
        entries = result.get("Symbol") or []
        names = [entry.get("FunctionName") for entry in entries if entry.get("FunctionName")]
        if not names or names[0] == "??":
            symbols.append(None)
            continue
        first = entries[0]
        location = ""
        if first.get("FileName") and first.get("Line"):
            location = f"{os.path.basename(first['FileName'])}:{first['Line']}"
        function = names[0]
        if len(names) > 1:
            function += f" (inlined into {names[-1]})"
        symbols.append(Symbol(function, location))
    return symbols


def parse_addr2line(output: str, count: int) -> List[Optional[Symbol]]:
    """Return one symbol for each of `count` addresses from `addr2line -f -C` output, which
    prints a function line and a `file:line` line for each address."""
    lines = output.splitlines()
    symbols: List[Optional[Symbol]] = []
    for i in range(count):
        function = lines[2 * i].strip() if 2 * i < len(lines) else "??"
        location = lines[2 * i + 1].strip() if 2 * i + 1 < len(lines) else ""
        if function == "??":
            symbols.append(None)
            continue
        if location.startswith("??"):
            location = ""
        symbols.append(Symbol(function, os.path.basename(location.split(" ")[0]) if location else ""))
    return symbols


ATOS_RE = re.compile(r"^(?P<function>.+?) \(in [^)]+\)(?: \((?P<location>[^)]+)\))?(?: \+ \d+)?$")


def parse_atos(output: str, count: int) -> List[Optional[Symbol]]:
    """Return one symbol for each of `count` addresses from `atos` output, which prints lines
    such as `Slang::emitInst() (in libslang-compiler.dylib) (slang-emit.cpp:12)`."""
    lines = output.splitlines()
    symbols: List[Optional[Symbol]] = []
    for i in range(count):
        line = lines[i].strip() if i < len(lines) else ""
        match = ATOS_RE.match(line)
        if not match or line.startswith("0x"):
            symbols.append(None)
            continue
        symbols.append(Symbol(match.group("function"), match.group("location") or ""))
    return symbols


def find_tool(name: str, explicit: Optional[str] = None) -> Optional[str]:
    """Return the path of the tool `name`, preferring `explicit`, then `PATH`, then the
    versioned names that LLVM packages install, such as `llvm-symbolizer-17`."""
    if explicit:
        return shutil.which(explicit)
    found = shutil.which(name)
    if found:
        return found
    for version in range(30, 9, -1):
        found = shutil.which(f"{name}-{version}")
        if found:
            return found
    return None


def symbolize_module(
    binary: Path,
    debug: Optional[Path],
    offsets: List[int],
    llvm_symbolizer: Optional[str],
) -> List[Optional[Symbol]]:
    """Return a symbol for each of `offsets` in the module whose binary is `binary`."""
    kind, base = read_image_base(binary)
    if llvm_symbolizer:
        command = [llvm_symbolizer, "--output-style=JSON", "--functions=linkage", "--demangle", "--inlines"]
        if kind == "pe":
            command += ["--relative-address", f"--obj={binary}"]
            if debug:
                command.append(f"--pdb={debug}")
        else:
            command.append(f"--obj={debug or binary}")
        command += [hex(base + offset) for offset in offsets]
        try:
            result = subprocess.run(command, capture_output=True, text=True, check=False)
        except OSError:
            result = None
        # An llvm-symbolizer that cannot run, or that is too old for JSON output, leaves the
        # work to the tools of the platform below.
        if result is not None and result.returncode == 0:
            try:
                return parse_llvm_symbolizer_json(result.stdout)
            except ValueError:
                pass
    addresses = [hex(base + offset) for offset in offsets]
    if kind == "elf" and shutil.which("addr2line"):
        result = subprocess.run(
            ["addr2line", "-f", "-C", "-e", str(debug or binary)] + addresses,
            capture_output=True,
            text=True,
            check=False,
        )
        return parse_addr2line(result.stdout, len(offsets))
    if kind == "macho" and shutil.which("atos"):
        result = subprocess.run(
            ["atos", "-o", str(debug or binary), "-l", hex(base)] + addresses,
            capture_output=True,
            text=True,
            check=False,
        )
        return parse_atos(result.stdout, len(offsets))
    return [None] * len(offsets)


# --- Putting it together ------------------------------------------------------------------------


def annotate(report: Report, symbols: Dict[int, Symbol]) -> str:
    """Return the report text with each frame line that has a symbol followed by it."""
    lines = list(report.lines)
    for frame in report.frames:
        symbol = symbols.get(frame.line_number)
        if symbol is None:
            continue
        suffix = f"  {symbol.function}"
        if symbol.location:
            suffix += f"  {symbol.location}"
        lines[frame.line_number] = lines[frame.line_number].rstrip() + suffix
    return "\n".join(lines) + "\n"


def symbolize(
    text: str,
    directories: Sequence[Path],
    llvm_symbolizer: Optional[str],
    log=sys.stderr,
) -> str:
    """Return `text` with the frames of its crash report symbolized from the files in
    `directories`. A module whose binary is not found is reported on `log` and left as it is."""
    report = parse_report(text)
    if not report.frames:
        raise ValueError("no stack frames found: is this a Slang crash report?")
    kind = report.fields.get("kind", "")

    by_module: Dict[str, List[Frame]] = {}
    for frame in report.frames:
        by_module.setdefault(frame.module, []).append(frame)

    symbols: Dict[int, Symbol] = {}
    for module, frames in by_module.items():
        binary, debug = find_module_files(module, directories)
        if binary is None:
            print(f"note: {module}: not found in the symbol directories", file=log)
            continue
        offsets = [lookup_address(frame, kind) for frame in frames]
        try:
            results = symbolize_module(binary, debug, offsets, llvm_symbolizer)
        except (OSError, ValueError) as error:
            print(f"note: {module}: {error}", file=log)
            continue
        for frame, symbol in zip(frames, results):
            if symbol is not None:
                symbols[frame.line_number] = symbol
    return annotate(report, symbols)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("report", nargs="?", help="the report file; standard input if omitted")
    parser.add_argument(
        "--symbols",
        action="append",
        default=[],
        metavar="DIR",
        help="a directory that holds the binaries and debug files of the release; repeatable",
    )
    parser.add_argument("--llvm-symbolizer", help="the llvm-symbolizer to use")
    args = parser.parse_args(argv)

    if not args.symbols:
        parser.error("give at least one --symbols directory")
    text = Path(args.report).read_text() if args.report else sys.stdin.read()
    try:
        sys.stdout.write(
            symbolize(text, [Path(d) for d in args.symbols], find_tool("llvm-symbolizer", args.llvm_symbolizer))
        )
    except ValueError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
