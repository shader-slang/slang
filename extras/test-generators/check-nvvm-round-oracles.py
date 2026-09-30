#!/usr/bin/env python3
"""Check the frozen NVVM round test constants without a compiler or floating-point arithmetic.

Run from any directory:
    python3 extras/test-generators/check-nvvm-round-oracles.py --check

Reads inputWords from the three tests/cuda/nvvm-round-{32,64,half}.slang files. Recomputes
expectedWords, initial output guards and both backends' FileCheck masks/completion words.
Uses exact integer IEEE decoding and Fraction rounding; never changes files or runs shaders.
NaN results promise only classification. Half intentionally preserves different tie rules:
NVVM rounds away from zero; CUDA rounds to even. Both comparisons execute in the same shader.
"""

import argparse
from fractions import Fraction
from pathlib import Path
import re
import sys


def require_equal(actual, expected, label):
    """Report the first differing word, including unequal sequence lengths."""
    if actual == expected:
        return
    for index, (left, right) in enumerate(zip(actual, expected)):
        if left != right:
            raise ValueError(f"{label}[{index}]: stored {left}, expected {right}")
    raise ValueError(f"{label}: stored {len(actual)} words, expected {len(expected)}")


def read_buffer(source, name):
    """Read exactly one uint buffer directive from the fixed numerical fixture format."""
    matches = re.findall(
        r"^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):"
        r"(?:out,)?name=" + re.escape(name) + r"$",
        source,
        re.MULTILINE,
    )
    if len(matches) != 1:
        raise ValueError(f"{name}: expected exactly one uint buffer directive")
    words = [int(word) for word in matches[0].split()]
    if any(word > 0xFFFFFFFF for word in words):
        raise ValueError(f"{name}: value outside a uint word")
    return words


def read_checks(source, prefix):
    """Read the complete FileCheck output sequence for one backend oracle."""
    matches = re.findall(
        r"^// " + re.escape(prefix) + r"(-NEXT)?: ([0-9]+)\{\{\$\}\}$",
        source,
        re.MULTILINE,
    )
    if not matches or matches[0][0] or any(kind != "-NEXT" for kind, _ in matches[1:]):
        raise ValueError(f"{prefix}: expected one initial check followed by NEXT checks")
    return [int(word) for _, word in matches]


def rounded_bits(bits, width, ties_even=False):
    """Round an IEEE magnitude as an exact rational, preserving its sign on zero."""
    fraction_bits, exponent_bits = {16: (10, 5), 32: (23, 8), 64: (52, 11)}[width]
    exponent_mask = (1 << exponent_bits) - 1
    bias = (1 << (exponent_bits - 1)) - 1
    sign = bits & (1 << (width - 1))
    exponent = (bits >> fraction_bits) & exponent_mask
    fraction = bits & ((1 << fraction_bits) - 1)
    if exponent == exponent_mask:
        return None if fraction else bits
    significand = fraction if exponent == 0 else (1 << fraction_bits) + fraction
    power = (1 - bias if exponent == 0 else exponent - bias) - fraction_bits
    magnitude = (
        Fraction(significand << power, 1)
        if power >= 0
        else Fraction(significand, 1 << -power)
    )
    integer, remainder = divmod(magnitude.numerator, magnitude.denominator)
    twice = 2 * remainder
    if twice > magnitude.denominator or (
        twice == magnitude.denominator and (not ties_even or integer % 2 != 0)
    ):
        integer += 1
    if integer == 0:
        return sign
    result_exponent = integer.bit_length() - 1
    shift = fraction_bits - result_exponent
    if shift >= 0:
        significand = integer << shift
    else:
        significand, residue = divmod(integer, 1 << -shift)
        if residue:
            raise ValueError("Rounded integer is not exactly representable in the input format")
    return sign | ((result_exponent + bias) << fraction_bits) | (significand - (1 << fraction_bits))


def half_disagreement_masks(away, even):
    """Combine the scalar/helper/vector/matrix comparison bits for each live input lane."""
    count = len(away)
    different = [left != right for left, right in zip(away, even)]
    masks = []
    for lane in range(count):
        mask = 3 if different[lane] else 0
        for lanes, flag in ((2, 4), (3, 8), (4, 16), (4, 32)):
            if any(different[(lane + offset) % count] for offset in range(lanes)):
                mask |= flag
        masks.append(mask)
    return masks


def check_fixture(path, width):
    """Validate one frozen fixture's expected values and full observable output sequence."""
    source = path.read_text()
    words = read_buffer(source, "inputWords")
    if width == 16:
        if any(word > 0xFFFF for word in words):
            raise ValueError("Half input exceeds sixteen bits")
        inputs = words
    else:
        if len(words) % 2:
            raise ValueError("Float32/64 fixture requires low/high input word pairs")
        if width == 32 and any(words[index] for index in range(1, len(words), 2)):
            raise ValueError("Float32 fixture high words must be zero")
        inputs = [words[index] | (words[index + 1] << 32) for index in range(0, len(words), 2)]
    if len(inputs) != 52:
        raise ValueError(f"Expected 52 frozen input lanes, got {len(inputs)}")
    away = [rounded_bits(bits, width) for bits in inputs]
    even = [rounded_bits(bits, width, True) for bits in inputs] if width == 16 else []
    expected = []
    for lane, result in enumerate(away):
        value = result if result is not None else 0
        expected.extend(
            (value, even[lane] if even[lane] is not None else 0, int(result is None))
            if width == 16
            else (value & 0xFFFFFFFF, value >> 32, int(result is None))
        )
    require_equal(read_buffer(source, "expectedWords"), expected, "expectedWords")
    stride = 3 if width == 16 else 2
    require_equal(
        read_buffer(source, "outputBuffer"),
        [0x13579BDF] + [0] * (stride * len(inputs)) + [0xDEADBEEF],
        "outputBuffer",
    )
    masks = half_disagreement_masks(away, even) if width == 16 else []
    for prefix in (("CUDA", "NVVM") if width == 16 else ("CHECK",)):
        output = [0x13579BDF]
        for lane in range(len(inputs)):
            if width == 16:
                output.extend((masks[lane], 0) if prefix == "CUDA" else (0, masks[lane]))
                output.append(12000 + lane)
            else:
                output.extend((0, 10000 + lane))
        output.append(0xDEADBEEF)
        require_equal(read_checks(source, prefix), output, prefix)
    print(f"PASS {path.name}: 52 input lanes, exact oracle words, guards and completion")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", required=True, help="check without writing")
    parser.parse_args()
    tests = Path(__file__).resolve().parents[2] / "tests" / "cuda"
    for name, width in (("32", 32), ("64", 64), ("half", 16)):
        path = tests / f"nvvm-round-{name}.slang"
        try:
            check_fixture(path, width)
        except (OSError, ValueError) as error:
            print(f"FAIL {path}: {error}", file=sys.stderr)
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
