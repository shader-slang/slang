#!/usr/bin/env python3
"""Regenerate selected-library ABI controls with llvmlite 0.42.0 / LLVM 14.0.6.

This is fixture-maintenance tooling, not a build or unit-test dependency.
"""
from pathlib import Path
import llvmlite
from llvmlite import binding as llvm

assert llvmlite.__version__ == "0.42.0"
assert llvm.llvm_version_info == (14, 0, 6)
root = Path(__file__).resolve().parent
names = [
    ("available-externally", "AvailableExternally"),
    ("fastcc", "Fastcc"),
    ("hidden", "Hidden"),
    ("internal", "Internal"),
    ("invalid-fastcc-variadic", "InvalidFastccVariadic"),
    ("parameter-inreg", "ParameterInreg"),
    ("return-inreg", "ReturnInreg"),
    ("variadic", "Variadic"),
]
lines = [
    "// Generated with LLVM 14.0.6; see test-data/nvvm/README.md and library-*.ll sources.",
    "#ifndef SLANG_UNIT_TEST_NVVM_LIBRARY_SIGNATURE_FIXTURES_H",
    "#define SLANG_UNIT_TEST_NVVM_LIBRARY_SIGNATURE_FIXTURES_H",
    "",
    "#include <stdint.h>",
    "",
]
for name, identifier in names:
    module = llvm.parse_assembly((root / ("library-" + name + ".ll")).read_text())
    try:
        module.verify()
    except RuntimeError as error:
        assert name == "invalid-fastcc-variadic"
        assert "Calling convention does not support" in str(error)
    else:
        assert name != "invalid-fastcc-variadic"
    data = module.as_bitcode()
    # The intentionally invalid module must remain parseable bitcode; all others also verify.
    restored = llvm.parse_bitcode(data)
    if name != "invalid-fastcc-variadic":
        restored.verify()
    lines.append("static const uint8_t kDeviceLibrary" + identifier + "Bitcode[] = {")
    for offset in range(0, len(data), 12):
        lines.append("    " + ", ".join(f"0x{byte:02x}" for byte in data[offset:offset + 12]) + ",")
    lines.extend(["};", ""])
lines.extend(["#endif", ""])
(root.parent.parent / "unit-test-nvvm-library-signature-fixtures.h").write_text("\n".join(lines))
