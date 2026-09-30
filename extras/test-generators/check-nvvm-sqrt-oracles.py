#!/usr/bin/env python3
"""Check frozen sqrt oracles without running a compiler or shader.

Run from any directory:
    python3 extras/test-generators/check-nvvm-sqrt-oracles.py --check

This checker imports no generator and uses no host/vendor floating sqrt. It normalizes integer
significands and compares squared midpoints. The frozen constants were generated independently
by searching IEEE encodings with exact rational arithmetic.

NVVM Half evaluates Float32 RN sqrt, then narrows RN to Half. CUDA hsqrt widens Half, executes
sqrt.approx.ftz.f32, and narrows RN. PTX 8.8 specifies maximum relative error 2^-23:
https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html#floating-point-instructions-sqrt
Positive Half inputs and their roots are normal Float32, so FTZ cannot affect this interval proof.
Squared bounds x*(1 +/- 2^-23)^2 and RN-even midpoint comparisons use integer arithmetic only.
"""

import argparse
from math import isqrt
from pathlib import Path
import re
import sys

FORMATS = {16: (10, 5), 32: (23, 8), 64: (52, 11)}


def require(condition, message):
    """Reject malformed numerical contracts even when Python assertions are disabled."""
    if not condition:
        raise ValueError(message)


def finite_pair(bits, width):
    """Decode a positive finite IEEE value as an integer times a power of two."""
    fraction_bits, exponent_bits = FORMATS[width]
    exponent = bits >> fraction_bits
    fraction = bits & ((1 << fraction_bits) - 1)
    significand = fraction + ((1 << fraction_bits) if exponent else 0)
    power = (exponent or 1) - ((1 << (exponent_bits - 1)) - 1) - fraction_bits
    return significand, power


def sqrt_ratio(numerator, denominator, result_width):
    """Round an exact rational square root by comparing the squared significand midpoint."""
    fraction_bits, exponent_bits = FORMATS[result_width]
    power = numerator.bit_length() - denominator.bit_length()
    if power >= 0:
        power -= int(numerator < (denominator << power))
    else:
        power -= int((numerator << -power) < denominator)
    result_exponent = power // 2
    shift = 2 * (fraction_bits - result_exponent)
    if shift >= 0:
        numerator <<= shift
    else:
        denominator <<= -shift
    lower = isqrt(numerator // denominator)
    comparison = 4 * numerator - denominator * (2 * lower + 1) ** 2
    rounded = lower + int(comparison > 0 or (comparison == 0 and lower & 1))
    if rounded == 1 << (fraction_bits + 1):
        rounded >>= 1
        result_exponent += 1
    biased_exponent = result_exponent + (1 << (exponent_bits - 1)) - 1
    require(biased_exponent > 0, "positive sqrt must produce a normal result")
    return (biased_exponent << fraction_bits) + rounded - (1 << fraction_bits)


def rounded_sqrt(bits, source_width, result_width):
    """Evaluate one correctly rounded sqrt at the requested destination precision."""
    significand, exponent = finite_pair(bits, source_width)
    numerator, denominator = (
        (significand << exponent, 1) if exponent >= 0 else (significand, 1 << -exponent)
    )
    return sqrt_ratio(numerator, denominator, result_width)


def narrow32_to_half(bits):
    """Round a normal Float32 sqrt result to Half using parity and discarded bits."""
    significand, exponent = finite_pair(bits, 32)
    fraction_bits = 10
    result_exponent = significand.bit_length() - 1 + exponent
    discard = significand.bit_length() - 1 - fraction_bits
    require(discard > 0, "Float32 to Half narrowing must discard precision")
    integral, remainder = divmod(significand, 1 << discard)
    halfway = 1 << (discard - 1)
    integral += int(remainder > halfway or (remainder == halfway and integral & 1))
    if integral == 1 << (fraction_bits + 1):
        integral >>= 1
        result_exponent += 1
    return ((result_exponent + 15) << fraction_bits) + integral - (1 << fraction_bits)


def expected(bits, width):
    """Preserve signed zero/infinity and classify negatives/NaNs before exact sqrt."""
    fraction_bits, exponent_bits = FORMATS[width]
    magnitude = bits & ((1 << (width - 1)) - 1)
    infinity = ((1 << exponent_bits) - 1) << fraction_bits
    if magnitude > infinity or ((bits >> (width - 1)) and magnitude):
        return None
    if magnitude in (0, infinity):
        return bits
    result = rounded_sqrt(bits, width, width)
    if width == 16:
        two_step = narrow32_to_half(rounded_sqrt(bits, 16, 32))
        require(two_step == result, f"Half two-step differs from direct sqrt at {bits:#x}")
        return two_step
    return result


def cuda_half_bounds(bits):
    """Bound RN Half narrowing of sqrt(x)*(1 +/- 2^-23) with squared rational endpoints."""
    exact = expected(bits, 16)
    if exact is None:
        return 0, 0
    if bits in (0, 0x8000, 0x7C00):
        return bits, bits
    significand, exponent = finite_pair(bits, 16)
    endpoints = []
    for step in (-1, 1):
        numerator = significand * ((1 << 23) + step) ** 2
        denominator = 1 << 46
        if exponent >= 0:
            numerator <<= exponent
        else:
            denominator <<= -exponent
        endpoints.append(sqrt_ratio(numerator, denominator, 16))
    return tuple(endpoints)


def read_buffer(text, name):
    """Read one frozen uint buffer directive and reject out-of-range words."""
    rows = re.findall(
        r"^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):(?:out,)?name="
        + re.escape(name) + r"$", text, re.MULTILINE
    )
    require(len(rows) == 1, f"{name}: expected exactly one buffer directive")
    words = list(map(int, rows[0].split()))
    require(all(0 <= word <= 0xFFFFFFFF for word in words), f"{name}: invalid uint word")
    return words


def check_fixture(path, width):
    """Recompute the oracle and verify mode markers, guards, and live comparisons."""
    text = path.read_text()
    words = read_buffer(text, "inputWords")
    require(len(words) == 128, "expected 64 pairs of input words")
    inputs = [low | (high << 32) for low, high in zip(words[::2], words[1::2])]
    require(len(set(inputs)) == 64, "expected 64 distinct input encodings")
    require(all(0 <= bits < 1 << width for bits in inputs), "input encoding exceeds width")
    fraction_bits, exponent_bits = FORMATS[width]
    bias = (1 << (exponent_bits - 1)) - 1
    sign, unit = 1 << (width - 1), 1 << fraction_bits
    infinity = ((1 << exponent_bits) - 1) << fraction_bits
    required = {
        0, 1, 2, 3, unit - 2, unit - 1, unit, unit + 1,
        infinity - 2, infinity - 1, infinity, infinity | (unit >> 1) | 0x123,
        infinity | 1, (bias << fraction_bits) + (unit >> 1),
    }
    for center in (
        (bias - 2) << fraction_bits, bias << fraction_bits,
        (bias + 1) << fraction_bits, (bias + 2) << fraction_bits,
        ((bias + 3) << fraction_bits) + (unit >> 3), (bias + fraction_bits) << fraction_bits,
    ):
        required.update((center - 1, center, center + 1))
    require(set(inputs[:32]) == required, "missing positive boundary/square/precision inputs")
    require(inputs[32:] == [bits | sign for bits in inputs[:32]], "missing signed input twins")
    results = [expected(bits, width) for bits in inputs]
    oracle = [
        word for result in results
        for word in ((0, 0, 1) if result is None else (result & 0xFFFFFFFF, result >> 32, 0))
    ]
    require(read_buffer(text, "expectedWords") == oracle, "exact oracle words differ")
    stride = 3 if width == 16 else 2
    initial_output = [0x13579BDF] + [0] * (64 * stride + int(width == 16)) + [0xDEADBEEF]
    require(read_buffer(text, "outputBuffer") == initial_output, "output initialization differs")
    if width == 16:
        bounds = [cuda_half_bounds(bits) for bits in inputs]
        require(
            read_buffer(text, "cudaBounds") == [word for pair in bounds for word in pair],
            "CUDA Half interval bounds differ",
        )
        require(
            "case nvvm: return true;" in text and "default: return false;" in text,
            "Half oracle policy must distinguish NVVM from CUDA",
        )
        require(
            "return usesPreciseNVVMOracle() ? low == expectedWords[offset] :" in text,
            "Half comparison must use the shared oracle policy",
        )
        require(
            "if (lane == 0) outputBuffer[1] = usesPreciseNVVMOracle() ? 36037u : 1209u;" in text,
            "Half mode marker must use the same policy as its comparison",
        )
        require(
            text.count("filecheck-buffer=CUDA") == 1 and text.count("filecheck-buffer=NVVM") == 2,
            "Half requires separate CUDA and NVVM FileCheck contracts",
        )
    for prefix in ("CUDA", "NVVM") if width == 16 else ("CHECK",):
        checks = re.findall(r"^// " + prefix + r"(-NEXT)?: (.+)\{\{\$\}\}$", text, re.MULTILINE)
        output_count = 195 if width == 16 else 130
        require(len(checks) == output_count, "missing output checks")
        require(
            checks[0][0] == "" and all(kind == "-NEXT" for kind, _ in checks[1:]),
            "output checks must be contiguous",
        )
        expected_checks = [str(0x13579BDF)]
        if width == 16:
            expected_checks.append("36037" if prefix == "NVVM" else "1209")
        for lane, result in enumerate(results):
            expected_checks.append("0")
            if width == 16:
                if result is None:
                    expected_checks.append("{{[0-9]+}}")
                elif prefix == "NVVM":
                    expected_checks.append(str(result))
                else:
                    low, high = bounds[lane]
                    choices = list(range(low, high + 1))
                    require(len(choices) <= 2, "unexpectedly broad CUDA interval")
                    expected_checks.append(
                        str(choices[0]) if len(choices) == 1
                        else "{{(" + "|".join(map(str, choices)) + ")}}"
                    )
            expected_checks.append(str(36000 + lane))
        expected_checks.append(str(0xDEADBEEF))
        require([word for _, word in checks] == expected_checks, f"{prefix}: output oracle differs")
    require(text.count("errors |= matchesExpected(") == 15, "expected 15 live comparisons")
    require(
        "[noinline]" in text and "sqrtThroughHelper(loadValue(lane))" in text,
        "missing live noinline helper comparison",
    )
    require(text.count("//TEST(compute):") == 3, "expected three GPU modes")
    print(
        f"PASS {path.name}: 64 inputs, 192 oracle words, 15 comparisons/lane, "
        f"{195 if width == 16 else 130} output words"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", required=True, action="store_true", help="check without writing")
    parser.add_argument(
        "--directory", type=Path, default=Path(__file__).resolve().parents[2] / "tests" / "cuda"
    )
    args = parser.parse_args()
    for suffix, width in (("half", 16), ("32", 32), ("64", 64)):
        path = args.directory / f"nvvm-sqrt-{suffix}.slang"
        try:
            check_fixture(path, width)
        except (OSError, ValueError) as error:
            print(f"FAIL {path}: {error}", file=sys.stderr)
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
