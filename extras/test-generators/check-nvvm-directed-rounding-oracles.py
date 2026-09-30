#!/usr/bin/env python3
"""Check the frozen directed-rounding oracles without running a compiler or shader.

Run from any directory:
    python3 extras/test-generators/check-nvvm-directed-rounding-oracles.py --check

This checker does not import the fixture generator. It removes fractional IEEE bits,
conditionally increments the integral magnitude, and retains the sign bit on zero.
The frozen constants were generated using exact rational arithmetic and signed integer division.
"""

import argparse
from pathlib import Path
import re
import sys


def require(condition, message):
    """Reject a malformed numerical contract even when Python assertions are disabled."""
    if not condition:
        raise ValueError(message)


def read_buffer(text, name):
    """Read exactly one uint buffer directive from the fixed numerical fixture format."""
    matches = re.findall(
        r"^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):(?:out,)?name="
        + re.escape(name)
        + r"$",
        text,
        re.MULTILINE,
    )
    require(len(matches) == 1, f"{name}: expected exactly one buffer directive")
    words = [int(word) for word in matches[0].split()]
    require(all(word <= 0xFFFFFFFF for word in words), f"{name}: word exceeds uint range")
    return words


def rounded_bits(bits, width, operation):
    """Round an IEEE magnitude by clearing fractional bits and applying the directed increment."""
    fraction_bits, exponent_bits = {16: (10, 5), 32: (23, 8), 64: (52, 11)}[width]
    exponent_mask = (1 << exponent_bits) - 1
    fraction_mask = (1 << fraction_bits) - 1
    sign_mask = 1 << (width - 1)
    sign = bits & sign_mask
    magnitude = bits & (sign_mask - 1)
    biased_exponent = magnitude >> fraction_bits
    fraction = magnitude & fraction_mask
    bias = exponent_mask >> 1
    if biased_exponent == exponent_mask:
        return None if fraction else bits
    exponent = biased_exponent - bias
    increment = (operation == "ceil" and not sign) or (operation == "floor" and sign)
    if exponent < 0:
        return sign | ((bias << fraction_bits) if magnitude and increment else 0)
    if exponent >= fraction_bits:
        return bits
    unit = 1 << (fraction_bits - exponent)
    discarded = magnitude & (unit - 1)
    result = magnitude & ~(unit - 1)
    if discarded and increment:
        result += unit
    return sign | result


def check_fixture(path, width):
    """Validate frozen inputs, exact oracles, live comparison masks, guards and completion words."""
    text = path.read_text()
    words = read_buffer(text, "inputWords")
    require(len(words) == 128, "expected 64 low/high input word pairs")
    values = [low | (high << 32) for low, high in zip(words[::2], words[1::2])]
    require(len(set(values)) == 64, "expected 64 distinct IEEE inputs")
    require(all(value < 1 << width for value in values), "input exceeds IEEE width")
    sign = 1 << (width - 1)
    require(
        all(values[i + 32] == (values[i] | sign) for i in range(32)),
        "expected exactly mirrored positive/negative inputs",
    )
    fraction_bits, exponent_bits = {16: (10, 5), 32: (23, 8), 64: (52, 11)}[width]
    bias = (1 << (exponent_bits - 1)) - 1
    one = bias << fraction_bits
    two = (bias + 1) << fraction_bits
    infinity = ((1 << exponent_bits) - 1) << fraction_bits
    required = {
        0, 1, (1 << fraction_bits) - 2, (1 << fraction_bits) - 1,
        1 << fraction_bits, (1 << fraction_bits) + 1,
        one - 1, one, one + 1, two - 1, two, two + 1,
        infinity - 1, infinity, infinity | (1 << (fraction_bits - 1)) | 0x123, infinity | 1,
    }
    for exponent in (-1, fraction_bits - 1, fraction_bits):
        threshold = (bias + exponent) << fraction_bits
        required.update((threshold - 1, threshold, threshold + 1))
    require(required <= set(values[:32]), "missing IEEE boundary coverage")
    expected = []
    for bits in values:
        for operation in ("ceil", "floor", "trunc"):
            result = rounded_bits(bits, width, operation)
            expected.extend(
                (0, 0, 1) if result is None else (result & 0xFFFFFFFF, result >> 32, 0)
            )
    require(read_buffer(text, "expectedWords") == expected, "expectedWords mismatch")
    require(
        read_buffer(text, "outputBuffer") == [0x13579BDF] + [0] * 128 + [0xDEADBEEF],
        "initial output guards or zero words changed",
    )
    checks = re.findall(r"^// CHECK(-NEXT)?: ([0-9]+)\{\{\$\}\}$", text, re.MULTILINE)
    require(
        len(checks) == 130 and checks[0][0] == ""
        and all(kind == "-NEXT" for kind, _ in checks[1:]),
        "expected one initial FileCheck word followed by 129 NEXT words",
    )
    output = [0x13579BDF]
    for lane in range(64):
        output.extend((0, 16000 + lane))
    output.append(0xDEADBEEF)
    require([int(word) for _, word in checks] == output, "FileCheck output mismatch")
    require(text.count("errors |= matchesExpected(") == 45, "expected 45 live comparisons")
    for operation in ("ceil", "floor", "trunc"):
        require(
            f"{operation}ThroughHelper(loadValue(lane))" in text,
            f"{operation}: missing live helper comparison",
        )
    print(f"PASS {path.name}: 64 inputs, 576 oracle words, 45 comparisons/lane, 130 outputs")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", required=True, help="check without writing")
    parser.add_argument(
        "--directory", type=Path, default=Path(__file__).resolve().parents[2] / "tests" / "cuda"
    )
    args = parser.parse_args()
    for suffix, width in (("half", 16), ("32", 32), ("64", 64)):
        path = args.directory / f"nvvm-directed-rounding-{suffix}.slang"
        try:
            check_fixture(path, width)
        except (OSError, ValueError) as error:
            print(f"FAIL {path}: {error}", file=sys.stderr)
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
