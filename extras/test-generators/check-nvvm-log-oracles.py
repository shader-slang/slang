#!/usr/bin/env python3
"""Certify signed logarithm references and the bounded log migration fixtures.

No host transcendental function or GPU observation defines an expected value. CUDA Half
uses the documented RN-even result on this finite corpus, not an approximation model.
CUDA double log10 preserves its existing Float32 wrapper and is not double accuracy evidence.
"""
import argparse
from fractions import Fraction as Q
from functools import lru_cache
import hashlib
import importlib.util
import json
from pathlib import Path
import re

ROOT = Path.cwd()
spec = importlib.util.spec_from_file_location('exp_ieee', ROOT / 'extras/test-generators/check-nvvm-exp-oracles.py')
E = importlib.util.module_from_spec(spec)
spec.loader.exec_module(E)
need, decode, rn, power2 = E.need, E.decode, E.round_signed, E.power2
OPS = {'log': 60, 'log2': 61, 'log10': 62}
CORRECTIONS = {'log': ((0x160d, 0x9c00), (0x3bfe, 0x8010), (0x3c0b, 0x8080), (0x6051, 0x1c00)),
               'log2': ((0xa2e2, 0x8080), (0xbf46, 0x9400)),
               'log10': ((0x338f, 0x1000), (0x33f8, 0x9000), (0x57e1, 0x9800), (0x719d, 0x9c00))}


@lru_cache(None)
def reduced_log_bounds(x, precision):
    """Bound ln(x) for 1<=x<=2 using directed integer powers of (x-1)/(x+1)."""
    need(1 <= x <= 2, 'reduced logarithm outside [1,2]')
    if x == 1:
        return Q(0), Q(0)
    scale = 1 << precision
    z = (x - 1) / (x + 1)
    a, b = z.numerator, z.denominator
    lo = a * scale // b
    hi = E.ceil_div(a * scale, b)
    low = high = 0
    count = precision // 3 + 24
    for j in range(count):
        low += 2 * lo // (2 * j + 1)
        high += E.ceil_div(2 * hi, 2 * j + 1)
        lo = lo * a * a // (b * b)
        hi = E.ceil_div(hi * a * a, b * b)
    tail = E.ceil_div(2 * hi * b * b, (2 * count + 1) * (b * b - a * a))
    return Q(low, scale), Q(high + tail, scale)


def ln_bounds(x, precision):
    need(x > 0, 'positive log input required')
    k = x.numerator.bit_length() - x.denominator.bit_length()
    m = x / power2(k)
    if m < 1:
        m *= 2
        k -= 1
    a, b = reduced_log_bounds(m, precision)
    c, d = reduced_log_bounds(Q(2), precision)
    return a + min(k * c, k * d), b + max(k * c, k * d)


def enclosure(x, operation, precision):
    low, high = ln_bounds(x, precision)
    if operation == 'log':
        return low, high
    a, b = ln_bounds(Q(2 if operation == 'log2' else 10), precision)
    values = (low / a, low / b, high / a, high / b)
    return min(values), max(values)


@lru_cache(None)
def reference(x, width, operation):
    """Certify the signed RN-even result by containing the whole interval in one IEEE cell."""
    if x == 1:
        return 0
    if operation in ('log2', 'log10'):
        base = 2 if operation == 'log2' else 10
        y, k = x, 0
        while y > 1 and y.denominator == 1 and y.numerator % base == 0:
            y /= base
            k += 1
        while y < 1 and y.numerator == 1 and y.denominator % base == 0:
            y *= base
            k -= 1
        if y == 1:
            return rn(Q(k), width)
    for precision in (160, 256, 384, 512):
        low, high = enclosure(x, operation, precision)
        result = rn(low, width)
        if high < 0:
            contained = E.cell_contains(-high, -low, result & ((1 << (width - 1)) - 1), width)
        else:
            contained = low >= 0 and E.cell_contains(low, high, result, width)
        if contained:
            return result
    raise ValueError('uncertified signed logarithm at 512-bit cap')


def special(bits, width):
    sign = 1 << (width - 1)
    magnitude = bits & (sign - 1)
    if magnitude > E.infinity(width) or (bits & sign and magnitude):
        return 'nan'
    if not magnitude:
        return sign | E.infinity(width)
    if magnitude == E.infinity(width):
        return magnitude
    if decode(bits, width) == 1:
        return 0
    return None


def library(bits, width, operation):
    if bits == 'nan':
        return ['nan']
    sp = special(bits, width)
    if sp is not None:
        return [sp]
    result = reference(decode(bits, width), width, operation)
    sign = result & (1 << (width - 1))
    count = 2 if width == 32 and operation == 'log10' else 1
    return [b | sign for b in E.admission(result ^ sign, width, count)]


def convert(bits, source, dest):
    if bits == 'nan':
        return bits
    sign = bits >> (source - 1)
    magnitude = bits & ((1 << (source - 1)) - 1)
    if magnitude > E.infinity(source):
        return 'nan'
    if magnitude == E.infinity(source):
        return E.infinity(dest) | (sign << (dest - 1))
    return rn(decode(bits, source), dest, negative_zero=bool(sign and not magnitude))


def record(bits, width, operation):
    sp = special(bits, width)
    ref = sp if sp is not None else reference(decode(bits, width), width, operation)
    if width == 16:
        nvvm = sorted(set(convert(b, 32, 16) for b in library(convert(bits, 16, 32), 32, operation)))
        cuda = [ref]
    else:
        nvvm = library(bits, width, operation)
        cuda = ([convert(b, 32, 64) for b in library(convert(bits, 64, 32), 32, operation)]
                if width == 64 and operation == 'log10' else nvvm)
    def encoded(b):
        return b if b == 'nan' else f'{b:0{width // 4}x}'
    return {'input_bits': encoded(bits), 'reference_bits': encoded(ref),
            'nvvm_candidates': [encoded(b) for b in nvvm],
            'cuda_candidates': [encoded(b) for b in cuda]}


def inputs(width, operation):
    inf, sign = E.infinity(width), 1 << (width - 1)
    f, _ = E.FORMATS[width]
    values = {0, sign, inf, inf | sign, inf + 1, inf + (1 << (f - 1)),
              sign | (inf + 1), sign | (inf + (1 << (f - 1))), 1, 2,
              (1 << f) - 1, 1 << f, (1 << f) + 1, inf - 1}
    for value in (Q(-2), Q(-1), Q(-1, 2), Q(1, 8), Q(1, 4), Q(1, 2),
                  Q(1), Q(2), Q(4), Q(8), Q(10), Q(100), Q(1000), Q(3, 4), Q(3, 2), Q(3)):
        bits = rn(value, width)
        values.update((bits - 1, bits, bits + 1))
    if width == 16:
        for op in ('log', 'log10'):
            for bits, _ in CORRECTIONS[op]:
                values.update((bits - 1, bits, bits + 1))
        # The PTX error interval at these inputs reaches each installed output predicate.
        for bits in (0x3bed, 0x3489):
            values.update((bits - 1, bits, bits + 1))
    if width == 64 and operation == 'log10':
        for x in (power2(-150), power2(-149), power2(-126), power2(128) - power2(103),
                  1 + power2(-24), 1 - power2(-25)):
            bits = rn(x, width)
            values.update((bits - 1, bits, bits + 1))
    need(len(values) <= 96, 'input cap exceeded')
    return sorted(values)


def check_fixture(path, width, operation, proposed_text=None):
    text = path.read_text() if proposed_text is None else proposed_text
    words = E.buffer(text, 'inputWords')
    expected_inputs = inputs(width, operation)
    need(words == [word for bits in expected_inputs for word in (bits & 0xffffffff, bits >> 32)], 'input inventory differs')
    records = [record(bits, width, operation) for bits in expected_inputs]
    need(E.buffer(text, 'expectedWords') == E.fixture_table(records, 'nvvm'), 'NVVM candidates differ')
    dual = width == 16 or (width == 64 and operation == 'log10')
    if dual:
        need(E.buffer(text, 'cudaWords') == E.fixture_table(records, 'cuda'), 'CUDA candidates differ')
        need('return usesLibraryPolicy() ? expectedWords[offset] : cudaWords[offset];' in text, 'missing policy switch')
        need('case nvvm: return true;' in text and 'default: return false;' in text, 'wrong target policy')
    else:
        need('return expectedWords[offset];' in text, 'wrong library policy')
    count, base = len(records), 2 if width == 16 else 1
    need(E.buffer(text, 'outputBuffer') == [0x13579bdf] + [0] * (4 * count + base - 1) + [0xdeadbeef], 'output layout differs')
    typ = {16: 'half', 32: 'float', 64: 'double'}[width]
    common = ('uint offset = 16 * lane;', 'if (expectedWord(offset) != 0) return nanBits;',
              'uint count = expectedWord(offset + 1);', 'for (uint choice = 0; choice < count; ++choice)',
              'if (low == expectedWord(offset + 2 + 2 * choice) &&',
              'high == expectedWord(offset + 3 + 2 * choice)) return true;',
              'valueBits(scalarResult, low, high);', 'return false;', '[numthreads(128, 1, 1)]',
              f'if (lane >= {count}) return;', 'uint lane = tid.x;', 'uint errors = 0;')
    encodings = {
        16: ('return bit_cast<half>(uint16_t(inputWords[2 * lane]));',
             'low = uint(bit_cast<uint16_t>(value));', 'high = 0;',
             'bool nanBits = (low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0;'),
        32: ('return asfloat(inputWords[2 * lane]);', 'low = asuint(value);', 'high = 0;',
             'bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x7fffffu) != 0;'),
        64: ('return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);',
             'asuint(value, low, high);',
             'bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u && ((high & 0xfffffu) != 0 || low != 0);')}
    for snippet in common + encodings[width]:
        need(snippet in text, 'missing fixture contract: ' + snippet)
    comparisons = ['scalarResult, lane', operation + 'ThroughHelper(loadValue(lane)), lane']
    masks = [1, 2]
    need(f'{typ} scalarResult = {operation}(loadValue(lane));' in text, 'scalar call missing')
    need(re.search(r'\[noinline\]\s+' + typ + ' ' + operation + r'ThroughHelper\(' + typ + r' value\)\s*\{\s*return ' + operation + r'\(value\);\s*\}', text), 'helper call missing')
    for size in (2, 3, 4):
        args = ', '.join(f'loadValue((lane + {i}) % {count})' for i in range(size))
        need(f'{typ}{size} result{size} = {operation}({typ}{size}({args}));' in text, 'vector call missing')
        for i, component in enumerate('xyzw'[:size]):
            comparisons.append(f'result{size}.{component}, (lane + {i}) % {count}')
            masks.append(1 << size)
    args = ', '.join(f'loadValue((lane + {i}) % {count})' for i in range(4))
    need(f'matrix<{typ}, 2, 2> resultMatrix = {operation}(matrix<{typ}, 2, 2>({args}));' in text, 'matrix call missing')
    for i in range(4):
        comparisons.append(f'resultMatrix[{i // 2}][{i % 2}], (lane + {i}) % {count}')
        masks.append(32)
    need(text.count('errors |= matchesExpected(') == 15, 'live observation count differs')
    for value, mask in zip(comparisons, masks):
        need(f'errors |= matchesExpected({value}) ? 0u : {mask}u;' in text, 'comparison missing')
    completion = OPS[operation] * 1000
    if width == 16:
        need(f'if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? {completion + 43}u : 1209u;' in text, 'policy marker missing')
    for offset, value in enumerate(('errors', 'low', 'high', f'{completion} + lane')):
        need(f'outputBuffer[{base + offset} + 4 * lane] = {value};' in text, 'output write missing')
    for prefix in (('CUDA', 'NVVM') if width == 16 else ('CHECK',)):
        expected = [str(0x13579bdf)]
        if width == 16:
            expected.append(str(completion + 43) if prefix == 'NVVM' else '1209')
        for lane in range(count):
            expected += ['0', '{{[0-9]+}}', '{{[0-9]+}}', str(completion + lane)]
        expected.append(str(0xdeadbeef))
        checks = re.findall(r'^// ' + prefix + r'(-NEXT)?: (.+)\{\{\$\}\}$', text, re.M)
        need([v for _, v in checks] == expected, 'output oracle differs')
        need(checks[0][0] == '' and all(k == '-NEXT' for k, _ in checks[1:]), 'noncontiguous output oracle')
    prefixes = ('CUDA', 'NVVM', 'NVVM') if width == 16 else ('CHECK', 'CHECK', 'CHECK')
    modes = ('-Xslang -O3', '-Xslang -emit-cuda-via-nvvm -Xslang -O0', '-Xslang -emit-cuda-via-nvvm -Xslang -O3')
    directives = [f'//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer={prefix}): '
                  '-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 ' + mode
                  for prefix, mode in zip(prefixes, modes)]
    need(re.findall(r'^//TEST\(compute\):.*$', text, re.M) == directives, 'execution mode contract differs')
    return {'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'inputs': count, 'observations_per_lane': 15, 'output_words': 4 * count + base + 1}


def self_test():
    checks = []
    for operation in OPS:
        need(reference(Q(1), 64, operation) == 0, 'one must be +zero')
        need(reference(Q(1, 2), 64, operation) >> 63 == 1, 'negative log result lost sign')
        need(reference(Q(2), 64, operation) >> 63 == 0, 'positive log result has wrong sign')
        checks += [operation + '-one', operation + '-negative', operation + '-positive']
    need(reference(Q(8), 64, 'log2') == rn(Q(3), 64), 'exact log2 power')
    need(reference(Q(1000), 64, 'log10') == rn(Q(3), 64), 'exact log10 power')
    need(len({reference(Q(3), 64, op) for op in OPS}) == 3, 'base mutation not distinguished')
    checks += ['exact-log2', 'exact-log10', 'different-bases']
    for width in (16, 32, 64):
        for bits in (0, 1 << (width - 1)):
            need(special(bits, width) == E.infinity(width) | (1 << (width - 1)), 'zero must give negative infinity')
        need(special(rn(Q(-1), width), width) == 'nan', 'negative domain must give NaN')
        checks += [f'signed-zeros-{width}', f'negative-domain-{width}']
    # The wrapper distinction is independently visible around one and at Float32 overflow.
    for x in (1 + power2(-52), power2(128)):
        row = record(rn(x, 64), 64, 'log10')
        need(set(row['nvvm_candidates']).isdisjoint(row['cuda_candidates']), 'CUDA double narrowing witness absent')
        checks.append('double-log10-path-' + str(x))
    # For each installed log2 predicate, exhibit an allowed Float32 lg2 result whose
    # Half rounding reaches it. This bounded witness does not enumerate approximation sets.
    for bits, target in ((0x3bed, 0xa2e2), (0x3489, 0xbf46)):
        x = decode(bits, 16)
        low, high = enclosure(x, 'log2', 256)
        radius = power2(-22) if Q(1, 2) < x < 2 else min(abs(low), abs(high)) * power2(-22)
        magnitude = target & 0x7fff
        ideal = rn(-E.midpoint_before(magnitude + 1, 16), 32)
        witnesses = []
        for q in range(ideal - 16, ideal + 17):
            value = decode(q, 32)
            if rn(value, 16) == target and max(abs(value-low), abs(value-high)) <= radius:
                witnesses.append(q)
        need(witnesses, 'installed log2 predicate has no independent PTX-bound witness')
        corrected = rn(decode(target, 16) + decode(dict(CORRECTIONS['log2'])[target], 16), 16)
        need(corrected != target and corrected == reference(x, 16, 'log2'), 'log2 correction must restore RN16')
        checks.append(f'log2-output-predicate-{bits:04x}-{witnesses[0]:08x}')
    # Source-keyed correction cases are nonzero. Do not claim a general signed-zero FMA
    # model from rational addition: the exact-one override separately checks positive zero.
    for operation in ('log', 'log10'):
        for original, correction in CORRECTIONS[operation]:
            target = reference(decode(original, 16), 16, operation)
            before = rn(decode(target, 16) - decode(correction, 16), 16)
            after = rn(decode(before, 16) + decode(correction, 16), 16)
            need(after == target and before != after, 'input-keyed correction witness absent')
            need(original != before, 'input/result key distinction absent')
            checks.append(f'{operation}-input-predicate-{original:04x}')
    # The generic evolving-result dependency is observable with a synthetic pair of
    # stages; the installed two predicates are disjoint and do not themselves cascade.
    first = rn(Q(1), 16)
    second = rn(Q(2), 16)
    evolving = rn(decode(first, 16) + 1, 16)
    evolving = rn(decode(evolving, 16) + 2, 16) if evolving == second else evolving
    frozen = rn(decode(first, 16) + 1, 16)
    need(evolving == rn(Q(4), 16) and frozen == second, 'evolving-result ordering control failed')
    checks.append('synthetic-evolving-result-order')
    return checks


def source_controls(text):
    """Check the installed scalar Half source order without modeling all approximations."""
    for operation in OPS:
        match = re.search(r'__CUDA_FP16_DECL__ __half h' + operation + r'\(const __half a\) \{(.*?)\n\}', text, re.S)
        need(match, 'missing CUDA Half source function')
        body = re.sub(r'\s+', '', match[1]).lower()
        stages = ['cvt.f32.f16', 'lg2.approx.ftz.f32']
        if operation != 'log2':
            stages += ['0x3f317218u' if operation == 'log' else '0x3e9a209bu', 'mul.f32']
        stages += ['cvt.rn.f16.f32']
        key = 'r' if operation == 'log2' else 'h'
        stages += [f'__spec_case({key},r,0x{bits:04x}u,0x{correction:04x}u)' for bits, correction in CORRECTIONS[operation]]
        positions = [body.find(stage) for stage in stages]
        need(all(p >= 0 for p in positions) and positions == sorted(set(positions)), 'CUDA Half stage/key/constant order differs')
    return ['cuda-half-source-' + op for op in OPS]


def negative_controls(directory):
    path = directory / 'nvvm-log-half.slang'
    text = path.read_text()
    mutations = {
        'wrong-base': ('log(loadValue(lane))', 'log2(loadValue(lane))'),
        'missing-candidate-buffer': ('name=expectedWords', 'name=wrongWords'),
        'swapped-target-policy': ('case nvvm: return true;', 'case nvvm: return false;'),
        'bad-input-width': ('bit_cast<half>(uint16_t(inputWords[2 * lane]))', 'half(inputWords[2 * lane])'),
        'bad-output-width': ('high = 0;', 'high = 1;'),
        'bad-nan': ('(low & 0x3ffu) != 0', '(low & 0x3ffu) == 0'),
        'bypassed-admission': ('if (expectedWord(offset) != 0) return nanBits;', 'return true;'),
        'missing-live-helper': ('[noinline]', ''),
        'wrong-dispatch': ('[numthreads(128, 1, 1)]', '[numthreads(1, 1, 1)]'),
        'wrong-completion': ('60000 + lane', '60001 + lane'),
        'wrong-marker': ('60043u : 1209u', '60044u : 1209u'),
        'wrong-mode': ('-Xslang -O0', '-Xslang -O3'),
        'changed-guard': ('data=[324508639', 'data=[324508638'),
        'missing-vector-observation': ('errors |= matchesExpected(result2.x', 'errors |= ignoresExpected(result2.x'),
    }
    rejected = []
    for name, (before, after) in mutations.items():
        need(before in text, 'mutation witness missing: ' + name)
        try:
            check_fixture(path, 16, 'log', text.replace(before, after, 1))
        except ValueError:
            rejected.append(name)
        else:
            raise ValueError('mutation accepted: ' + name)
    def reject_table(name, fixture, width, operation, words):
        original = fixture.read_text()
        pattern = r'(//TEST_INPUT: ubuffer\(data=\[)[^\]]*(\], stride=4\):name=expectedWords)'
        mutated, count = re.subn(pattern, lambda m: m[1] + ' '.join(map(str, words)) + m[2], original)
        need(count == 1, 'table mutation binding differs')
        try:
            check_fixture(fixture, width, operation, mutated)
        except ValueError:
            rejected.append(name)
        else:
            raise ValueError('table mutation accepted: ' + name)
    words = E.buffer(text, 'expectedWords')
    lane = inputs(16, 'log').index(rn(Q(1, 2), 16))
    words[16 * lane + 2] ^= 0x8000
    reject_table('negative-reference-sign', path, 16, 'log', words)
    fixture = directory / 'nvvm-log10-64.slang'
    reject_table('double-log10-target-policy-swap', fixture, 64, 'log10', E.buffer(fixture.read_text(), 'cudaWords'))
    return rejected


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--directory', type=Path, default=ROOT / 'tests/cuda')
    parser.add_argument('--proposal', type=Path)
    parser.add_argument('--negative-controls', action='store_true')
    parser.add_argument('--cuda-header', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = {'status': 'passed'}
    if args.self_test:
        result['controls'] = self_test()
    if args.check:
        result['fixtures'] = [check_fixture(args.directory / f'nvvm-{op}-{suffix}.slang', width, op)
                              for op in OPS for width, suffix in ((16, 'half'), (32, '32'), (64, '64'))]
    if args.negative_controls:
        result['rejected_mutations'] = negative_controls(args.directory)
    if args.cuda_header:
        source = args.cuda_header.read_text()
        result['source_controls'] = source_controls(source)
        for before, after in (('__SPEC_CASE(r, r, 0xA2E2U', '__SPEC_CASE(h, r, 0xA2E2U'),
                              ('0x3f317218U', '0x3f317219U'),
                              ('__SPEC_CASE(r, r, 0xA2E2U, 0x8080U)\n        __SPEC_CASE(r, r, 0xBF46U, 0x9400U)',
                               '__SPEC_CASE(r, r, 0xBF46U, 0x9400U)\n        __SPEC_CASE(r, r, 0xA2E2U, 0x8080U)')):
            try:
                source_controls(source.replace(before, after))
            except ValueError:
                result['source_controls'].append('rejected-' + before)
            else:
                raise ValueError('source mutation accepted')
    if args.proposal:
        proposal = json.loads(args.proposal.read_text())
        for group in proposal['operations']:
            for fmt in group['formats']:
                width = fmt['width']
                need(fmt['records'] == [record(bits, width, group['operation']) for bits in inputs(width, group['operation'])], 'generator/reference disagreement')
        result['proposal'] = 'independently certified'
    rendered = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.write_text(rendered)
    print(rendered)


if __name__ == '__main__':
    main()
