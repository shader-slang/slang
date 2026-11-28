#!/usr/bin/env python3
"""Tests for extras/symbolize-crash-report.py."""

from __future__ import annotations

import importlib.util
import io
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

EXTRAS_DIR = Path(__file__).resolve().parents[1]
SCRIPT_PATH = EXTRAS_DIR / "symbolize-crash-report.py"
SPEC = importlib.util.spec_from_file_location("symbolize_crash_report", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"could not load {SCRIPT_PATH}")
symbolizer = importlib.util.module_from_spec(SPEC)
# The script's dataclasses look their module up by name while the class is created, so the
# module has to be registered before it runs.
sys.modules[SPEC.name] = symbolizer
SPEC.loader.exec_module(symbolizer)


REPORT = """\
some diagnostics before the report
==== Slang crash report (format 1) ====
Slang stopped because of a bug in Slang, not in your code.

[report]
kind:        crash
signal:      SIGSEGV
signature:   crash-7c41d9e2

[stack]
#0  libslang-compiler.so+0x3a1786
#1  libslang-compiler.so+0x3a09f1
#2  slangc+0x4d12

[details]
This part can hold names from your code. Read it before you share the report.
message:     #9  forged+0x1

[how to report]
"""


def _elf64(load_vaddrs, align=0x1000):
    """Return the bytes of a minimal little-endian ELF64 file with one PT_LOAD per address."""
    phoff = 64
    phentsize = 56
    header = bytearray(64)
    header[0:4] = b"\x7fELF"
    header[4] = 2  # ELFCLASS64
    header[5] = 1  # little-endian
    struct.pack_into("<Q", header, 32, phoff)
    struct.pack_into("<HH", header, 54, phentsize, len(load_vaddrs))
    entries = bytearray()
    for vaddr in load_vaddrs:
        entry = bytearray(phentsize)
        struct.pack_into("<I", entry, 0, 1)  # PT_LOAD
        struct.pack_into("<Q", entry, 16, vaddr)
        struct.pack_into("<Q", entry, 48, align)
        entries += entry
    return bytes(header + entries)


def _macho64(text_vmaddr):
    """Return the bytes of a minimal thin Mach-O 64 file with a `__PAGEZERO` and a `__TEXT`
    segment."""
    def segment(name, vmaddr):
        command = bytearray(72)
        struct.pack_into("<II", command, 0, 0x19, 72)  # LC_SEGMENT_64
        command[8 : 8 + len(name)] = name
        struct.pack_into("<Q", command, 24, vmaddr)
        return command

    commands = segment(b"__PAGEZERO", 0) + segment(b"__TEXT", text_vmaddr)
    header = bytearray(32)
    struct.pack_into("<I", header, 0, 0xFEEDFACF)
    struct.pack_into("<I", header, 16, 2)  # ncmds
    struct.pack_into("<I", header, 20, len(commands))
    return bytes(header + commands)


class ParseReportTest(unittest.TestCase):
    def test_frames_and_fields(self):
        report = symbolizer.parse_report(REPORT)
        self.assertEqual(report.fields["kind"], "crash")
        self.assertEqual(report.fields["signature"], "crash-7c41d9e2")
        self.assertEqual(
            [(f.index, f.module, f.offset) for f in report.frames],
            [(0, "libslang-compiler.so", 0x3A1786), (1, "libslang-compiler.so", 0x3A09F1), (2, "slangc", 0x4D12)],
        )

    def test_details_are_not_frames(self):
        # A line in `[details]` that looks like a frame is user text and is never symbolized.
        report = symbolizer.parse_report(REPORT)
        self.assertNotIn("forged", [f.module for f in report.frames])

    def test_only_the_first_report_and_crlf(self):
        text = (REPORT + REPORT.replace("slangc", "other")).replace("\n", "\r\n")
        report = symbolizer.parse_report(text)
        self.assertEqual(len(report.frames), 3)
        self.assertEqual(report.frames[2].module, "slangc")


class LookupAddressTest(unittest.TestCase):
    def test_return_addresses_step_back_but_the_fault_does_not(self):
        frame0 = symbolizer.Frame(0, 0, "m", 0x100)
        frame1 = symbolizer.Frame(0, 1, "m", 0x100)
        self.assertEqual(symbolizer.lookup_address(frame0, "crash"), 0x100)
        self.assertEqual(symbolizer.lookup_address(frame1, "crash"), 0xFF)
        # In an internal-error report, frame 0 is a return address too.
        self.assertEqual(symbolizer.lookup_address(frame0, "internal-error"), 0xFF)


class ImageBaseTest(unittest.TestCase):
    def _write(self, data):
        handle = tempfile.NamedTemporaryFile(delete=False)
        handle.write(data)
        handle.close()
        self.addCleanup(lambda: Path(handle.name).unlink())
        return Path(handle.name)

    def test_elf_shared_library_starts_at_zero(self):
        self.assertEqual(symbolizer.read_image_base(self._write(_elf64([0, 0x10000]))), ("elf", 0))

    def test_elf_executable_starts_at_its_first_segment(self):
        path = self._write(_elf64([0x401000, 0x400000]))
        self.assertEqual(symbolizer.read_image_base(path), ("elf", 0x400000))

    def test_macho_uses_the_text_segment(self):
        path = self._write(_macho64(0x100000000))
        self.assertEqual(symbolizer.read_image_base(path), ("macho", 0x100000000))

    def test_pe_offsets_are_relative(self):
        self.assertEqual(symbolizer.read_image_base(self._write(b"MZ" + bytes(62))), ("pe", 0))

    def test_other_files_are_rejected(self):
        with self.assertRaises(ValueError):
            symbolizer.read_image_base(self._write(b"not a binary" + bytes(60)))


class SymbolizerOutputTest(unittest.TestCase):
    def test_llvm_symbolizer_array_with_inlining(self):
        output = json.dumps(
            [
                {
                    "Address": "0x1",
                    "Symbol": [
                        {"FunctionName": "inner()", "FileName": "/build/src/a.cpp", "Line": 12},
                        {"FunctionName": "outer()", "FileName": "/build/src/b.cpp", "Line": 30},
                    ],
                },
                {"Address": "0x2", "Symbol": [{"FunctionName": "??", "FileName": "", "Line": 0}]},
            ]
        )
        symbols = symbolizer.parse_llvm_symbolizer_json(output)
        self.assertEqual(symbols[0].function, "inner() (inlined into outer())")
        self.assertEqual(symbols[0].location, "a.cpp:12")
        self.assertIsNone(symbols[1])

    def test_llvm_symbolizer_one_object_per_line(self):
        output = '{"Symbol": [{"FunctionName": "f", "FileName": "", "Line": 0}]}\n'
        symbols = symbolizer.parse_llvm_symbolizer_json(output)
        self.assertEqual((symbols[0].function, symbols[0].location), ("f", ""))

    def test_addr2line(self):
        output = "Slang::emitInst()\n/build/source/slang/slang-emit.cpp:1234\n??\n??:0\n"
        symbols = symbolizer.parse_addr2line(output, 2)
        self.assertEqual((symbols[0].function, symbols[0].location), ("Slang::emitInst()", "slang-emit.cpp:1234"))
        self.assertIsNone(symbols[1])

    def test_atos(self):
        output = "main (in test-process) (test-process-main.cpp:78)\n0x100001a54\n"
        symbols = symbolizer.parse_atos(output, 2)
        self.assertEqual((symbols[0].function, symbols[0].location), ("main", "test-process-main.cpp:78"))
        self.assertIsNone(symbols[1])


class AnnotateTest(unittest.TestCase):
    def test_symbols_follow_their_frames_and_nothing_else_changes(self):
        report = symbolizer.parse_report(REPORT)
        first = report.frames[0]
        text = symbolizer.annotate(report, {first.line_number: symbolizer.Symbol("f()", "a.cpp:1")})
        lines = text.splitlines()
        self.assertEqual(lines[first.line_number], "#0  libslang-compiler.so+0x3a1786  f()  a.cpp:1")
        self.assertEqual(
            [line for i, line in enumerate(lines) if i != first.line_number],
            [line for i, line in enumerate(REPORT.splitlines()) if i != first.line_number],
        )

    def test_missing_modules_are_noted_and_left_alone(self):
        log = io.StringIO()
        with tempfile.TemporaryDirectory() as directory:
            text = symbolizer.symbolize(REPORT, [Path(directory)], None, log=log)
        self.assertEqual(text, REPORT)
        self.assertIn("libslang-compiler.so: not found", log.getvalue())

    def test_text_without_a_report_is_an_error(self):
        with self.assertRaises(ValueError):
            symbolizer.symbolize("no report here\n", [], None)


if __name__ == "__main__":
    unittest.main()
