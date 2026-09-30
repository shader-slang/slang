#!/usr/bin/env python3
"""Independently certify frozen rsqrt inputs and discrete admission tables.

Run standalone against repository fixtures with --check, or use --directory for proposed fixtures.
An optional --manifest additionally audits ignored certificates; ordinary checks need no manifest.

This checker never imports or invokes the generator. Its decoder uses integer significand/power
pairs and its reference uses integer normalization and isqrt of a reciprocal rational. Local
adjacency, squared-midpoint, interval endpoint and integer tenth-power certificates are checked
separately. No host floating point, vendor operation, shader output, compiler or GPU is an input.

The explicit library union is a scoped empirical/non-guaranteed qualification convention, not a
vendor-defined ULP metric. CUDA Half uses the PTX bound; NVVM Half uses the Float32 library set.
"""
import argparse
import copy
from fractions import Fraction
import hashlib
import json
from math import isqrt
from pathlib import Path
import re
import sys

FORMAT = {16: (10, 15), 32: (23, 127), 64: (52, 1023)}


def need(condition, message):
    if not condition:
        raise ValueError(message)


def pair(bits, width):
    """Decode unsigned positive finite IEEE encodings without the generator's decoder."""
    fraction_width, bias = FORMAT[width]
    exponent_field, trailing = divmod(bits, 2**fraction_width)
    need(0 <= exponent_field <= 2 * bias, 'decoder requires positive finite encoding')
    if exponent_field:
        return trailing + 2**fraction_width, exponent_field - bias - fraction_width
    return trailing, 1 - bias - fraction_width


def rational(bits, width):
    coefficient, power = pair(bits, width)
    if power < 0:
        return Fraction(coefficient, 2**(-power))
    return Fraction(coefficient * 2**power)


def reciprocal_root(bits, source_width, target_width):
    """Normalize 1/x and round its integer square-root significand independently."""
    coefficient, power = pair(bits, source_width)
    need(coefficient > 0, 'root requires a nonzero positive input')
    numerator, denominator = (1, coefficient << power) if power >= 0 else (1 << -power, coefficient)
    exponent = numerator.bit_length() - denominator.bit_length()
    if exponent >= 0:
        exponent -= int(numerator < denominator << exponent)
    else:
        exponent -= int(numerator << -exponent < denominator)
    output_exponent = exponent // 2
    fraction_width, bias = FORMAT[target_width]
    scale = 2 * (fraction_width - output_exponent)
    if scale >= 0:
        numerator <<= scale
    else:
        denominator <<= -scale
    floor_significand = isqrt(numerator // denominator)
    midpoint_sign = 4 * numerator - denominator * (2 * floor_significand + 1)**2
    result = floor_significand + int(midpoint_sign > 0 or (midpoint_sign == 0 and floor_significand % 2))
    if result == 2**(fraction_width + 1):
        result //= 2
        output_exponent += 1
    need(-bias < output_exponent <= bias, 'reciprocal root must be normal and finite')
    return ((output_exponent + bias) << fraction_width) + result - 2**fraction_width


def certify_reference(input_bits, input_width, output_width, certificate):
    """Check both integer-normalized RN and local adjacent/squared-midpoint certificates."""
    x = rational(input_bits, input_width)
    low, high, rounded = (certificate[k] for k in ('lower', 'upper', 'rounded'))
    need(high - low in (0, 1), 'root bracket must be exact or adjacent')
    a, b = rational(low, output_width), rational(high, output_width)
    if low == high:
        need(x * a * a == 1, 'singleton root bracket is not exact')
    else:
        need(x * a * a < 1 < x * b * b, 'root bracket does not strictly enclose the ideal root')
    midpoint = (a + b) / 2
    comparison = x * midpoint * midpoint - 1
    nearest = low if comparison > 0 else high
    if comparison == 0:
        nearest = low if low % 2 == 0 else high
    need(rounded == nearest == reciprocal_root(input_bits, input_width, output_width),
         'same-width RN-even reference differs')
    return rounded


def narrow(bits):
    """Round normal Float32 to normal Half using integer discarded bits and parity."""
    significand, power = pair(bits, 32)
    exponent = significand.bit_length() - 1 + power
    discard = significand.bit_length() - 11
    need(discard > 0, 'Half narrowing must discard Float32 precision')
    integer, remainder = divmod(significand, 1 << discard)
    midpoint = 1 << (discard - 1)
    integer += int(remainder > midpoint or (remainder == midpoint and integer % 2))
    if integer == 2048:
        integer //= 2
        exponent += 1
    need(-15 < exponent <= 15, 'Half result must be normal and finite')
    return ((exponent + 15) << 10) + integer - 1024


def certify_library(input_bits, input_width, library):
    width = 32 if input_width == 16 else input_width
    need(library['width'] == width, 'wrong intermediate library precision')
    r = certify_reference(input_bits, input_width, width, library['reference'])
    count = 2 if width == 32 else 1
    value = rational(r, width)
    spacing = max(value - rational(r - 1, width), rational(r + 1, width) - value)
    lower_bound, upper_bound = value - count * spacing, value + count * spacing
    left, right = library['reference_interval']
    need(rational(left - 1, width) < lower_bound <= rational(left, width),
         'reference-spacing lower endpoint or excluded neighbor differs')
    need(rational(right, width) <= upper_bound < rational(right + 1, width),
         'reference-spacing upper endpoint or excluded neighbor differs')
    need(library['steps_interval'] == [r - count, r + count], 'encoding-step interval differs')
    members = list(range(min(left, r - count), max(right, r + count) + 1))
    need(library['members'] == members, 'library must be exact union, without extra/missing encodings')
    flags = [[q, left <= q <= right, r - count <= q <= r + count] for q in members]
    need(library['membership'] == flags, 'library component membership flags differ')
    return members


def certify_ptx(input_bits, certificate):
    k = certificate['k']
    need(isinstance(k, int) and k >= 23, 'invalid epsilon denominator exponent')
    low, high = certificate['epsilon_numerators']
    need(high == low + 1 and low > 0, 'epsilon numerators must be consecutive positive integers')
    target = 1 << (10 * k - 229)
    need(low**10 < target < high**10, 'epsilon tenth-power enclosure fails')
    x = rational(input_bits, 16)
    for key, epsilon_numerator in (('inner', low), ('outer', high)):
        epsilon = Fraction(epsilon_numerator, 1 << k)
        need(0 < epsilon < 1, 'relative epsilon bound outside positive domain')
        lower_bound, upper_bound = (1 - epsilon)**2, (1 + epsilon)**2
        left, right = certificate[key]
        need(left <= right, 'empty PTX admission set')
        need(x * rational(left - 1, 32)**2 < lower_bound <= x * rational(left, 32)**2,
             key + ' PTX lower endpoint or excluded neighbor differs')
        need(x * rational(right, 32)**2 <= upper_bound < x * rational(right + 1, 32)**2,
             key + ' PTX upper endpoint or excluded neighbor differs')
    need(certificate['inner'] == certificate['outer'], 'Float32 PTX membership is still undecided')
    left, right = certificate['inner']
    members = list(range(left, right + 1))
    need(certificate['members'] == members, 'PTX Float32 candidate enumeration differs')
    mapping = [[q, narrow(q)] for q in members]
    image = sorted({h for _, h in mapping})
    need(certificate['narrowing'] == mapping and certificate['image'] == image,
         'PTX exact RN16 image differs')
    return image


def classify(bits, width):
    magnitude = bits % (1 << (width - 1))
    negative = bits >= 1 << (width - 1)
    f, bias = FORMAT[width]
    infinity = (2 * bias + 1) << f
    if magnitude == 0:
        return 'special', [infinity + ((1 << (width - 1)) if negative else 0)]
    if negative or magnitude > infinity:
        return 'nan', []
    if magnitude == infinity:
        return 'special', [0]
    return 'positive', None


def certify_row(row, width):
    bits = row['input']
    need(0 <= bits < 1 << width, 'input outside declared IEEE width')
    kind, expected = classify(bits, width)
    need(row['kind'] == kind, 'special-value classification differs')
    if kind != 'positive':
        need(row['nvvm'] == row['cuda'] == expected, 'special-value admitted bits differ')
        return
    members = certify_library(bits, width, row['library'])
    if width == 16:
        mapping = [[q, narrow(q)] for q in members]
        image = sorted({h for _, h in mapping})
        need(row['narrowing'] == mapping and row['nvvm'] == image, 'NVVM exact RN16 image differs')
        need(row['cuda'] == certify_ptx(bits, row['ptx']), 'CUDA exact RN16 image differs')
        certify_reference(bits, 16, 16, row['ideal_half'])
    else:
        need(row['nvvm'] == row['cuda'] == members, 'library final admitted bits differ')


def buffer(text, name):
    rows = re.findall(r'^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):(?:out,)?name=' +
                      re.escape(name) + '$', text, re.M)
    need(len(rows) == 1, name + ': missing or duplicate buffer')
    words = list(map(int, rows[0].split()))
    need(all(0 <= w < 1 << 32 for w in words), name + ': non-uint word')
    return words


def table(rows, policy):
    words = []
    for row in rows:
        members = row[policy]
        need(len(members) <= 7, 'fixture table capacity exceeded')
        words.extend([int(row['kind'] == 'nan'), len(members)])
        for result in members + [0] * (7 - len(members)):
            words.extend([result % 2**32, result // 2**32])
    return words


def required_base_inputs(width):
    """Define the common boundary/material corpus once for all independent checks."""
    f, bias = FORMAT[width]
    infinity = (2 * bias + 1) << f
    unit = 1 << f
    required = {0, 1, 2, 3, unit - 2, unit - 1, unit, unit + 1, infinity - 2,
                infinity - 1, infinity, infinity | (unit >> 1) | 0x123, infinity | 1,
                (bias << f) + (unit >> 1)}
    for power in (-2, 0, 1, 2):
        center = (bias + power) << f
        required.update([center - 1, center, center + 1])
    nine = ((bias + 3) << f) + (unit >> 3)
    required.update([nine - 1, nine, nine + 1])
    return required


def check_corpus(rows, width, midpoint_rows):
    inputs = [r['input'] for r in rows]
    need(len(inputs) == len(set(inputs)) == 64, 'expected 64 unique inputs')
    positive = inputs[:32]
    need(inputs[32:] == [q + (1 << (width - 1)) for q in positive], 'signed input twins differ')
    f, bias = FORMAT[width]
    required = required_base_inputs(width)
    if width == 16:
        need(len(midpoint_rows) == 3, 'three Half midpoint candidates required')
        residuals = []
        for candidate in midpoint_rows:
            bits = candidate['bits']
            row = next(r for r in rows if r['input'] == bits)
            a, b = (rational(row['ideal_half'][key], 16) for key in ('lower', 'upper'))
            need(a < b, 'midpoint candidate must have an inexact Half result')
            residual = rational(bits, 16) * ((a + b) / 2)**2 - 1
            need([residual.numerator, residual.denominator] == candidate['midpoint_residual'],
                 'Half midpoint residual certificate differs')
            need(0 < abs(residual) < Fraction(1, 2**19), 'candidate is not close to Half midpoint')
            residuals.append(residual)
            required.add(bits)
        need(min(residuals) < 0 < max(residuals), 'Half midpoint candidates need both sides')
    else:
        center = (bias + 4) << f
        required.update([center - 1, center, center + 1])
    need(set(positive) == required, 'frozen boundary/material/midpoint input inventory differs')


def check_fixture(path, width, data, midpoints):
    rows = data['rows']
    check_corpus(rows, width, midpoints)
    for row in rows:
        certify_row(row, width)
    need(data['policy_differences'] == [i for i, r in enumerate(rows) if r['nvvm'] != r['cuda']],
         'policy difference lane inventory differs')
    text = path.read_text()
    if 'sha256' in data:
        need(hashlib.sha256(path.read_bytes()).hexdigest() == data['sha256'],
             'frozen fixture SHA256 differs')
    expected_input = [w for r in rows for w in (r['input'] % 2**32, r['input'] // 2**32)]
    need(buffer(text, 'inputWords') == expected_input, 'fixture inputs differ from certified inputs')
    need(buffer(text, 'expectedWords') == table(rows, 'nvvm'), 'library fixture table differs')
    base = 2 if width == 16 else 1
    need(buffer(text, 'outputBuffer') == [0x13579bdf] + [0] * (256 + base - 1) + [0xdeadbeef],
         'output guard/layout initialization differs')
    if width == 16:
        need(buffer(text, 'cudaWords') == table(rows, 'cuda'), 'CUDA Half fixture table differs')
        for snippet in ('case nvvm: return true;', 'default: return false;',
                        'return usesLibraryPolicy() ? expectedWords[offset] : cudaWords[offset];',
                        'if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 65039u : 1209u;'):
            need(snippet in text, 'missing shared Half policy selection/marker: ' + snippet)
        need(text.count('filecheck-buffer=CUDA') == 1 and text.count('filecheck-buffer=NVVM') == 2,
             'Half filecheck mode contracts differ')
    else:
        need('return expectedWords[offset];' in text and text.count('filecheck-buffer=CHECK') == 3,
             'library word selection/mode contracts differ')
    for snippet in ('uint offset = 16 * lane;', 'if (expectedWord(offset) != 0) return nanBits;',
                    'uint count = expectedWord(offset + 1);',
                    'for (uint choice = 0; choice < count; ++choice)',
                    'if (low == expectedWord(offset + 2 + 2 * choice) &&',
                    'high == expectedWord(offset + 3 + 2 * choice)) return true;',
                    'valueBits(scalarResult, low, high);', '[noinline]'):
        need(snippet in text, 'missing discrete pair membership/live raw observation: ' + snippet)
    typ = {16: 'half', 32: 'float', 64: 'double'}[width]
    need(typ + ' scalarResult = rsqrt(loadValue(lane));' in text, 'scalar rsqrt not live')
    need(re.search(r'\[noinline\]\s+' + typ + r' rsqrtThroughHelper\(' + typ +
                   r' value\)\s*\{\s*return rsqrt\(value\);\s*\}', text), 'helper rsqrt not live')
    comparisons = ['scalarResult, lane', 'rsqrtThroughHelper(loadValue(lane)), lane']
    error_masks = [1, 2]
    for size in (2, 3, 4):
        args = ', '.join(f'loadValue((lane + {i}) % 64)' for i in range(size))
        need(f'{typ}{size} result{size} = rsqrt({typ}{size}({args}));' in text,
             f'vector{size} rsqrt not live')
        for i, component in enumerate('xyzw'[:size]):
            comparisons.append(f'result{size}.{component}, (lane + {i}) % 64')
            error_masks.append(1 << size)
    args = ', '.join(f'loadValue((lane + {i}) % 64)' for i in range(4))
    need(f'matrix<{typ}, 2, 2> resultMatrix = rsqrt(matrix<{typ}, 2, 2>({args}));' in text,
         'matrix2x2 rsqrt not live')
    for i in range(4):
        comparisons.append(f'resultMatrix[{i // 2}][{i % 2}], (lane + {i}) % 64')
        error_masks.append(32)
    need(text.count('errors |= matchesExpected(') == 15, 'expected 15 live shape observations')
    for expression, mask in zip(comparisons, error_masks):
        need(f'errors |= matchesExpected({expression}) ? 0u : {mask}u;' in text,
             'missing live shape comparison: ' + expression)
    for offset, value in enumerate(('errors', 'low', 'high', '65000 + lane')):
        need(f'outputBuffer[{base + offset} + 4 * lane] = {value};' in text,
             'raw output/completion layout differs')
    for prefix in (('CUDA', 'NVVM') if width == 16 else ('CHECK',)):
        checks = re.findall(r'^// ' + prefix + r'(-NEXT)?: (.+)\{\{\$\}\}$', text, re.M)
        expected = [str(0x13579bdf)]
        if width == 16:
            expected.append('65039' if prefix == 'NVVM' else '1209')
        for lane in range(64):
            expected += ['0', '{{[0-9]+}}', '{{[0-9]+}}', str(65000 + lane)]
        expected.append(str(0xdeadbeef))
        need([v for _, v in checks] == expected, 'full guard/error/raw/completion filecheck differs')
        need(checks[0][0] == '' and all(k == '-NEXT' for k, _ in checks[1:]),
             'filecheck output must be contiguous')
    need(text.count('//TEST(compute):') == 3, 'three GPU mode directives required')
    print(f'PASS {path.name}: 64 inputs, 15 observations/lane, {256 + base + 1} output words')


def negative_certificate_checks(manifest):
    """Prove certificate checks reject representative errors rather than merely trusting tables."""
    rows = manifest['widths']['16']['rows']
    positive = next(r for r in rows if r['kind'] == 'positive')
    mutations = []
    def add(name, mutate):
        row = copy.deepcopy(positive)
        mutate(row)
        mutations.append((name, row))
    add('reference', lambda r: r['library']['reference'].__setitem__('rounded',
              r['library']['reference']['rounded'] + 1))
    add('library-union', lambda r: r['library']['members'].pop())
    add('component-membership', lambda r: r['library']['membership'][0].__setitem__(1,
              not r['library']['membership'][0][1]))
    add('epsilon-bracket', lambda r: r['ptx']['epsilon_numerators'].__setitem__(0,
              r['ptx']['epsilon_numerators'][0] - 1))
    add('nvvm-narrowing', lambda r: r['narrowing'][0].__setitem__(1, r['narrowing'][0][1] + 1))
    add('cuda-image', lambda r: r['cuda'].__setitem__(0, r['cuda'][0] + 1))
    for name, row in mutations:
        try:
            certify_row(row, 16)
        except ValueError:
            continue
        raise ValueError('mutation incorrectly accepted: ' + name)
    print('PASS six negative numerical certificate checks')


def reference_from_input(bits, source_width, target_width):
    """Build a local certificate around the independently normalized RN-even result."""
    rounded = reciprocal_root(bits, source_width, target_width)
    product = rational(bits, source_width) * rational(rounded, target_width)**2
    return {'rounded': rounded, 'lower': rounded - int(product > 1),
            'upper': rounded + int(product < 1)}


def library_from_input(bits, width):
    """Walk the small exact rational neighborhood of the independently found reference."""
    target_width = 32 if width == 16 else width
    reference = reference_from_input(bits, width, target_width)
    rounded = reference['rounded']
    center = rational(rounded, target_width)
    count = 2 if target_width == 32 else 1
    radius = count * max(center - rational(rounded - 1, target_width),
                         rational(rounded + 1, target_width) - center)
    left = right = rounded
    while center - rational(left - 1, target_width) <= radius:
        left -= 1
    while rational(right + 1, target_width) - center <= radius:
        right += 1
    members = list(range(min(left, rounded - count), max(right, rounded + count) + 1))
    return {'width': target_width, 'reference': reference, 'reference_interval': [left, right],
            'steps_interval': [rounded - count, rounded + count], 'members': members,
            'membership': [[q, left <= q <= right, rounded - count <= q <= rounded + count]
                           for q in members]}


def tenth_root_floor(target):
    """Use decreasing integer Newton iterates, followed by exact local certification."""
    estimate = 1 << ((target.bit_length() + 9) // 10)
    while True:
        following = (9 * estimate + target // estimate**9) // 10
        if following >= estimate:
            break
        estimate = following
    need(estimate**10 <= target < (estimate + 1)**10, 'integer tenth-root normalization failed')
    return estimate


def ptx_from_input(bits):
    """Find exact Float32 PTX sets by local endpoint walks, then narrow every candidate."""
    x = rational(bits, 16)
    anchor = reciprocal_root(bits, 16, 32)
    def interval(epsilon):
        lower, upper = (1 - epsilon)**2, (1 + epsilon)**2
        left = anchor
        while x * rational(left - 1, 32)**2 >= lower:
            left -= 1
        while x * rational(left, 32)**2 < lower:
            left += 1
        right = anchor
        while x * rational(right + 1, 32)**2 <= upper:
            right += 1
        while x * rational(right, 32)**2 > upper:
            right -= 1
        need(left <= right, 'PTX interval contains no representable Float32 value')
        return [left, right]
    k = 64
    while True:
        target = 1 << (10 * k - 229)
        low = tenth_root_floor(target)
        need(low**10 < target < (low + 1)**10, 'strict epsilon bracket failed')
        inner = interval(Fraction(low, 1 << k))
        outer = interval(Fraction(low + 1, 1 << k))
        if inner == outer:
            members = list(range(inner[0], inner[1] + 1))
            narrowing = [[q, narrow(q)] for q in members]
            return {'k': k, 'epsilon_numerators': [low, low + 1], 'inner': inner,
                    'outer': outer, 'members': members, 'narrowing': narrowing,
                    'image': sorted({h for _, h in narrowing})}
        k += 32


def derive_rows(path, width):
    """Recompute every expected set from live input encodings alone, never from oracle tables."""
    words = buffer(path.read_text(), 'inputWords')
    need(len(words) == 128, 'expected 64 input word pairs')
    inputs = [words[i] + (words[i + 1] << 32) for i in range(0, 128, 2)]
    rows = []
    for bits in inputs:
        need(0 <= bits < 1 << width, 'input outside declared width')
        kind, expected = classify(bits, width)
        row = {'input': bits, 'kind': kind}
        if expected is not None:
            row.update(nvvm=expected, cuda=expected.copy())
        else:
            library = library_from_input(bits, width)
            row['library'] = library
            if width == 16:
                narrowing = [[q, narrow(q)] for q in library['members']]
                ptx = ptx_from_input(bits)
                row.update(nvvm=sorted({h for _, h in narrowing}), cuda=ptx['image'].copy(),
                           narrowing=narrowing, ptx=ptx, ideal_half=reference_from_input(bits, 16, 16))
            else:
                row.update(nvvm=library['members'].copy(), cuda=library['members'].copy())
        rows.append(row)
    return rows


def derive_midpoints(rows):
    """Recognize the three near-midpoint cases by exact inequalities in the input corpus."""
    candidates = []
    base_inputs = required_base_inputs(16)
    for row in rows[:32]:
        if row['kind'] != 'positive' or row['input'] in base_inputs:
            continue
        a, b = (rational(row['ideal_half'][key], 16) for key in ('lower', 'upper'))
        if a == b:
            continue
        residual = rational(row['input'], 16) * ((a + b) / 2)**2 - 1
        if 0 < abs(residual) < Fraction(1, 2**19):
            candidates.append({'bits': row['input'],
                               'midpoint_residual': [residual.numerator, residual.denominator]})
    return candidates


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', required=True, action='store_true')
    parser.add_argument('--directory', type=Path,
                        default=Path(__file__).resolve().parents[2] / 'tests' / 'cuda')
    parser.add_argument('--manifest', type=Path,
                        help='optionally also certify a separate ignored certificate manifest')
    args = parser.parse_args()
    try:
        derived = {'widths': {}}
        for suffix, width in (('half', 16), ('32', 32), ('64', 64)):
            path = args.directory / f'nvvm-rsqrt-{suffix}.slang'
            rows = derive_rows(path, width)
            derived['widths'][str(width)] = {
                'fixture': path.name, 'rows': rows,
                'policy_differences': [i for i, row in enumerate(rows) if row['nvvm'] != row['cuda']]}
        midpoints = derive_midpoints(derived['widths']['16']['rows'])
        for width in (16, 32, 64):
            data = derived['widths'][str(width)]
            check_fixture(args.directory / data['fixture'], width, data, midpoints)
        negative_certificate_checks(derived)
        if args.manifest:
            manifest = json.loads(args.manifest.read_text())
            need(manifest['policy'] == 'union(reference-max-adjacent-spacing-radius,encoding-steps)',
                 'wrong library policy')
            need(manifest['reference'] == 'same-width RN-even(1/sqrt(x))' and
                 manifest['library_n'] == {'32': 2, '64': 1}, 'wrong reference or library radius')
            need(manifest['observed_outputs_used'] is False, 'observed outputs must not define oracle')
            need(manifest['half_midpoints'] == midpoints, 'independently derived midpoint rows differ')
            for width in (16, 32, 64):
                data = manifest['widths'][str(width)]
                check_fixture(args.directory / data['fixture'], width, data, manifest['half_midpoints'])
                need(data['rows'] == derived['widths'][str(width)]['rows'],
                     'certificate rows differ from independently recomputed rows')
            print('PASS optional certificate manifest audit')
    except (OSError, ValueError, KeyError, StopIteration) as error:
        print('FAIL: ' + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
