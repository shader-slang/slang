#!/usr/bin/env python3
"""Check frozen frac/fract numerical contracts without a compiler or vendor arithmetic.

Run with --check, optionally --directory PATH. Integer ratios and normalized quotient/remainder
rounding independently recompute the fixture constants. The original generator instead searched
adjacent IEEE encodings with exact rational midpoint comparisons.

Every finite Half residual is an integer multiple of 2^-24 in [0,1), hence exactly Float32.
Therefore RN16(RN32(x-floor(x))) equals direct RN16 for finite Half. Float32/64 residuals still
require their own RN rounding; tiny negative inputs legitimately produce positive one.
"""
import argparse
from pathlib import Path
import re
import sys

FORMATS = {16: (10, 5), 32: (23, 8), 64: (52, 11)}


def require(condition, reason):
    if not condition:
        raise ValueError(reason)


def pair(bits, width):
    """Decode a signed finite encoding to an integer ratio without Fraction."""
    f, e = FORMATS[width]
    exponent = (bits >> f) & ((1 << e) - 1)
    if exponent == (1 << e) - 1:
        return None
    significand = bits & ((1 << f) - 1)
    if exponent:
        significand |= 1 << f
    power = max(exponent, 1) - ((1 << (e - 1)) - 1) - f
    numerator, denominator = (significand << power, 1) if power >= 0 else (significand, 1 << -power)
    return (-numerator if bits >> (width - 1) else numerator), denominator


def encode(numerator, denominator, width):
    """Normalize and round a nonnegative rational using discarded quotient bits and parity."""
    require(numerator >= 0 and denominator > 0, 'invalid nonnegative rational')
    if numerator == 0:
        return 0
    f, e = FORMATS[width]
    bias = (1 << (e - 1)) - 1
    exponent = numerator.bit_length() - denominator.bit_length()
    if exponent >= 0:
        exponent -= int(numerator < denominator << exponent)
    else:
        exponent -= int(numerator << -exponent < denominator)
    exponent = max(exponent, 1 - bias)
    shift = f - exponent
    if shift >= 0:
        numerator <<= shift
    else:
        denominator <<= -shift
    significand, remainder = divmod(numerator, denominator)
    significand += int(2 * remainder > denominator or
                       (2 * remainder == denominator and significand & 1))
    if significand == 1 << (f + 1):
        significand >>= 1
        exponent += 1
    if significand < 1 << f:
        require(exponent == 1 - bias, 'invalid subnormal exponent')
        return significand
    return ((exponent + bias) << f) + significand - (1 << f)


def expected(bits, width):
    value = pair(bits, width)
    if value is None:
        return None
    numerator, denominator = value
    return encode(numerator % denominator, denominator, width)


def read_buffer(text, name):
    rows = re.findall(
        r'^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):(?:out,)?name=' +
        re.escape(name) + r'$', text, re.MULTILINE)
    require(len(rows) == 1, 'expected exactly one buffer ' + name)
    words = [int(word) for word in rows[0].split()]
    require(all(0 <= word <= 0xffffffff for word in words), 'invalid uint word')
    return words


def check_fixture(path, width):
    """Check exact inputs, scalar oracles, live shape checks and every output word."""
    text = path.read_text()
    input_words = read_buffer(text, 'inputWords')
    require(len(input_words) == 128, 'expected 64 input pairs')
    inputs = [low | high << 32 for low, high in zip(input_words[::2], input_words[1::2])]
    require(len(set(inputs)) == 64, 'expected 64 unique encodings')
    require(all(0 <= bits < 1 << width for bits in inputs), 'input exceeds width')
    f, e = FORMATS[width]
    bias = (1 << (e - 1)) - 1
    unit = 1 << f
    infinity = ((1 << e) - 1) << f
    required = {0, 1, 2, unit - 1, unit, unit + 1,
                (bias - f - 1) << f, infinity - 1, infinity,
                infinity + (unit >> 1) + 0x23, infinity + 1}
    centers = [(bias - f - 2) << f, (bias - 1) << f, bias << f,
               (bias + 1) << f, ((bias + 1) << f) + (unit >> 1),
               (bias + f) << f, (bias + f + 1) << f]
    for center in centers:
        required.update((center - 1, center, center + 1))
    require(set(inputs[:32]) == required, 'boundary inputs differ')
    require(inputs[32:] == [bits | (1 << (width - 1)) for bits in inputs[:32]], 'signed twins differ')
    results = [expected(bits, width) for bits in inputs]
    oracle = [word for answer in results for word in
              ([0, 0, 1] if answer is None else [answer & 0xffffffff, answer >> 32, 0])]
    require(read_buffer(text, 'expectedWords') == oracle, 'exact oracle differs')
    require(read_buffer(text, 'outputBuffer') == [324508639] + [0] * 384 + [3735928559],
            'output initialization differs')
    if width == 16:
        for bits in inputs:
            value = pair(bits, width)
            if value is None:
                continue
            n, d = value
            residual = n % d
            widened = pair(encode(residual, d, 32), 32)
            require(widened[0] * d == residual * widened[1], 'Half residual is not exact Float32')
            require(encode(*widened, 16) == expected(bits, 16), 'Half promoted route differs')
    checks = re.findall(r'^// CHECK(-NEXT)?: (.+)\{\{\$\}\}$', text, re.MULTILINE)
    require(len(checks) == 386 and checks[0][0] == '' and
            all(kind == '-NEXT' for kind, _ in checks[1:]), 'output checks must be 386 contiguous words')
    expected_checks = ['324508639']
    for lane, answer in enumerate(results):
        raw = ['{{[0-9]+}}'] * 2 if answer is None else [str(answer & 0xffffffff), str(answer >> 32)]
        expected_checks += ['0'] + raw + raw + [str(59000 + lane)]
    expected_checks.append('3735928559')
    require([word for _, word in checks] == expected_checks, 'FileCheck oracle differs')
    modes = re.findall(r'^//TEST\(compute\):(.+)$', text, re.MULTILINE)
    prefix = ('COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj '
              '-output-using-type -capability cuda_sm_8_0 -Xslang ')
    require(modes == [prefix + '-O3', prefix + '-emit-cuda-via-nvvm -Xslang -O0',
                      prefix + '-emit-cuda-via-nvvm -Xslang -O3'], 'GPU modes differ')
    compact = ' '.join(text.split())
    typ = {16: 'half', 32: 'float', 64: 'double'}[width]
    bit_statements = {16: 'low = uint(bit_cast<uint16_t>(value)); high = 0;',
                      32: 'low = asuint(value); high = 0;',
                      64: 'asuint(value, low, high);'}[width]
    load = {16: 'bit_cast<half>(uint16_t(inputWords[2 * lane]))',
            32: 'asfloat(inputWords[2 * lane])',
            64: 'asdouble(inputWords[2 * lane], inputWords[2 * lane + 1])'}[width]
    nan = {16: '(low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0',
           32: '(low & 0x7f800000u) == 0x7f800000u && (low & 0x7fffffu) != 0',
           64: '(high & 0x7ff00000u) == 0x7ff00000u && ((high & 0xfffffu) != 0 || low != 0)'}[width]
    for statement in [f'{typ} loadValue(uint lane) {{ return {load}; }}',
                      f'void valueBits({typ} value, out uint low, out uint high) {{ {bit_statements} }}',
                      f'bool matchesExpected({typ} value, uint lane) {{ uint low, high; '
                      f'valueBits(value, low, high); bool nanBits = {nan}; uint offset = 3 * lane; '
                      'return expectedWords[offset + 2] != 0 ? nanBits : '
                      'low == expectedWords[offset] && high == expectedWords[offset + 1]; }',
                      '[numthreads(64, 1, 1)] void computeMain(uint3 tid : SV_DispatchThreadID) '
                      '{ uint lane = tid.x; if (lane >= 64) return; uint errors = 0;']:
        require(statement in compact, 'live bit observation/classification contract differs')
    comparisons = []
    flag = 1
    for function in ('frac', 'fract'):
        require(f'[noinline] {typ} {function}ThroughHelper({typ} value) {{ return {function}(value); }}'
                in compact, 'noinline helper differs')
        require(f'{typ} {function}Scalar = {function}(loadValue(lane));' in compact, 'scalar call differs')
        comparisons.append(f'errors |= matchesExpected({function}Scalar, lane) ? 0u : {flag}u;')
        flag <<= 1
        comparisons.append(f'errors |= matchesExpected({function}ThroughHelper(loadValue(lane)), lane) ? 0u : {flag}u;')
        flag <<= 1
        for count in (2, 3, 4):
            args = ', '.join(f'loadValue((lane + {index}) % 64)' for index in range(count))
            require(f'{typ}{count} {function}Result{count} = {function}({typ}{count}({args}));'
                    in compact, 'vector mapping differs')
            for index in range(count):
                comparisons.append(f'errors |= matchesExpected({function}Result{count}.{"xyzw"[index]}, (lane + {index}) % 64) ? 0u : {flag}u;')
            flag <<= 1
        if function == 'frac':
            args = ', '.join(f'loadValue((lane + {index}) % 64)' for index in range(4))
            require(f'matrix<{typ}, 2, 2> resultMatrix = frac(matrix<{typ}, 2, 2>({args}));'
                    in compact, 'matrix mapping differs')
            for index in range(4):
                comparisons.append(f'errors |= matchesExpected(resultMatrix[{index // 2}][{index % 2}], (lane + {index}) % 64) ? 0u : {flag}u;')
            flag <<= 1
    require(len(comparisons) == 26 and flag == 2048, 'shape coverage differs')
    require(re.findall(r'errors \|= matchesExpected\([^;]+;', compact) == comparisons,
            'live comparison dataflow differs')
    outputs = ['errors', 'fracLow', 'fracHigh', 'fractLow', 'fractHigh', '59000 + lane']
    require(re.findall(r'outputBuffer\[[^;]+;', compact) ==
            [f'outputBuffer[{index + 1} + 6 * lane] = {value};' for index, value in enumerate(outputs)],
            'raw scalar/completion output mapping differs')
    require('valueBits(fracScalar, fracLow, fracHigh);' in compact and
            'valueBits(fractScalar, fractLow, fractHigh);' in compact, 'raw scalar values differ')
    print(f'PASS {path.name}: 64 inputs, 192 oracle words, 26 comparisons/lane, 386 output words')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', required=True, action='store_true')
    parser.add_argument('--directory', type=Path,
                        default=Path(__file__).resolve().parents[2] / 'tests' / 'cuda')
    args = parser.parse_args()
    for suffix, width in (('half', 16), ('32', 32), ('64', 64)):
        path = args.directory / f'nvvm-frac-{suffix}.slang'
        try:
            check_fixture(path, width)
        except (OSError, ValueError) as error:
            print(f'FAIL {path}: {error}', file=sys.stderr)
            return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
