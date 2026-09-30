#!/usr/bin/env python3
"""Independently certify exp fixtures using integers; no host transcendental or GPU input.

Run --check against repository fixtures, or supply --directory for proposed fixtures.
Optional --proposal audits ignored generator certificates; --negative-controls mutates those
certificates and numerical fields to prove rejection. Normal checks need no ignored manifest.

The reference route uses certified ln(2) reduction, directed Taylor terms without repeated
squaring, and direct IEEE midpoint-cell comparisons. Admission is a separate test convention.
"""
import argparse
import copy
from fractions import Fraction as Q
from functools import lru_cache
import hashlib
import json
import re
from pathlib import Path
import time

FORMATS = {16: (10, 15), 32: (23, 127), 64: (52, 1023)}
C_BITS = 0x3fb8aa3b
CORRECTIONS = ((0x1f79, 0x9400), (0x25cf, 0x9400), (0xc13b, 0x0400), (0xc1ef, 0x0200))


def need(test, message):
    if not test:
        raise ValueError(message)


def power2(e):
    return Q(1 << e) if e >= 0 else Q(1, 1 << -e)


def infinity(width):
    fraction, bias = FORMATS[width]
    return (2 * bias + 1) << fraction


def decode(bits, width):
    """Decode finite signed IEEE bits as an exact rational."""
    need(0 <= bits < 1 << width, 'encoding exceeds format')
    sign = -1 if bits >> (width - 1) else 1
    magnitude = bits & ((1 << (width - 1)) - 1)
    need(magnitude < infinity(width), 'finite value required')
    fraction, bias = FORMATS[width]
    e, trailing = divmod(magnitude, 1 << fraction)
    return sign * (trailing + ((1 << fraction) if e else 0)) * power2((e or 1) - bias - fraction)


def midpoint_before(bits, width):
    """Return the exact real midpoint preceding a positive encoding, including infinity."""
    need(0 < bits <= infinity(width), 'midpoint index outside positive range')
    if bits == infinity(width):
        fraction, bias = FORMATS[width]
        return power2(bias + 1) - power2(bias - fraction - 1)
    return (decode(bits - 1, width) + decode(bits, width)) / 2


def round_positive(value, width):
    """Find the rounding cell by binary-searching exact adjacent midpoint inequalities."""
    need(value >= 0, 'round_positive needs nonnegative real')
    left, right = 0, infinity(width)
    while left < right:
        mid = (left + right + 1) // 2
        boundary = midpoint_before(mid, width)
        # The upper encoding is selected at equality only when its significand is even.
        if value > boundary or (value == boundary and mid % 2 == 0):
            left = mid
        else:
            right = mid - 1
    return left


def round_signed(value, width, negative_zero=False):
    sign = (1 << (width - 1)) if value < 0 or (value == 0 and negative_zero) else 0
    return round_positive(abs(value), width) | sign


def cell_contains(low, high, bits, width):
    need(0 <= low <= high, 'nonnegative ordered enclosure required')
    need(0 <= bits <= infinity(width), 'candidate outside positive IEEE range')
    if bits:
        b = midpoint_before(bits, width)
        if low < b or (low == b and bits % 2):
            return False
    if bits != infinity(width):
        b = midpoint_before(bits + 1, width)
        if high > b or (high == b and bits % 2):
            return False
    return True


def ceil_div(a, b):
    need(b > 0, 'positive divisor required')
    return -((-a) // b)


@lru_cache(maxsize=None)
def log_two_bounds(precision):
    """Enclose atanh(1/3)*2 with term-wise directed integer rounding and a geometric tail."""
    scale = 1 << precision
    count = precision // 3 + 16
    lower = upper = 0
    power = 3
    for j in range(count):
        denominator = (2 * j + 1) * power
        lower += (2 * scale) // denominator
        upper += ceil_div(2 * scale, denominator)
        power *= 9
    # Consecutive omitted terms have ratio < 1/9. Upper tail is 9/8 * first omitted.
    tail = ceil_div(18 * scale, 8 * (2 * count + 1) * power)
    upper += tail
    need(0 < lower < upper < scale, 'invalid certified ln2 interval')
    return lower, upper


def positive_exp_bounds(argument, precision):
    """Bound exp(z), 0 <= z <= 1, with independently rounded term recurrences."""
    need(0 <= argument <= 1, 'Taylor endpoint outside certified interval')
    if not argument:
        return Q(1), Q(1)
    scale = 1 << precision
    numerator, denominator = argument.numerator, argument.denominator
    lower_term = upper_term = scale
    lower_sum = upper_sum = scale
    # This conservative fixed bound exceeds the precision needed for factorial tail decay.
    count = precision // 2 + 16
    for j in range(1, count + 1):
        lower_term = (lower_term * numerator) // (denominator * j)
        upper_term = ceil_div(upper_term * numerator, denominator * j)
        lower_sum += lower_term
        upper_sum += upper_term
    omitted = ceil_div(upper_term * numerator, denominator * (count + 1))
    # Remaining term ratios are <= z/(count+2). The omitted bound already rounds upward.
    tail = ceil_div(omitted * denominator * (count + 2), denominator * (count + 2) - numerator)
    return Q(lower_sum, scale), Q(upper_sum + tail, scale)


def exp_endpoint_bounds(argument, precision):
    if argument >= 0:
        return positive_exp_bounds(argument, precision)
    low, high = positive_exp_bounds(-argument, precision)
    return 1 / high, 1 / low


def exp_enclosure(x, precision):
    """Reduce by independently certified ln2 and bound two endpoints without squaring."""
    scale = 1 << precision
    log_low, log_high = log_two_bounds(precision)
    k = (x.numerator * scale) // (x.denominator * log_low)
    low = x - Q(max(k * log_low, k * log_high), scale)
    high = x - Q(min(k * log_low, k * log_high), scale)
    need(-1 <= low <= high <= 1, 'ln2 range reduction needs refinement')
    a, _ = exp_endpoint_bounds(low, precision)
    _, b = exp_endpoint_bounds(high, precision)
    return a * power2(k), b * power2(k)


def exp2_enclosure(x, precision):
    k = x.numerator // x.denominator
    fraction = x - k
    if not fraction:
        exact = power2(k)
        return exact, exact
    scale = 1 << precision
    log_low, log_high = log_two_bounds(precision)
    a, _ = positive_exp_bounds(fraction * Q(log_low, scale), precision)
    _, b = positive_exp_bounds(fraction * Q(log_high, scale), precision)
    return a * power2(k), b * power2(k)


@lru_cache(maxsize=None)
def reference(x, width, base_two=False):
    """Certify a mathematical RN result, separately from approximate-library admission."""
    if x == 0:
        return round_positive(Q(1), width)
    if x >= 2048:
        return infinity(width)
    if x <= -2048:
        return 0
    for precision in (128, 192, 256, 384, 512):
        low, high = (exp2_enclosure if base_two else exp_enclosure)(x, precision)
        result = round_positive(low, width)
        if cell_contains(low, high, result, width):
            return result
    raise ValueError('reference is uncertified at the frozen 512-bit resource cap')


def admission(reference_bits, width, count):
    """Test-defined nonnegative spacing/encoding union with explicit endpoint extension."""
    inf = infinity(width)
    need(0 <= reference_bits <= inf and count in (1, 2), 'invalid admission input')
    steps = set(range(max(0, reference_bits - count), min(inf, reference_bits + count) + 1))
    if reference_bits == inf:
        return sorted(steps)
    value = decode(reference_bits, width)
    previous = decode(reference_bits - 1, width) if reference_bits else -decode(1, width)
    if reference_bits + 1 == inf:
        following = power2(FORMATS[width][1] + 1)
    else:
        following = decode(reference_bits + 1, width)
    radius = count * max(value - previous, following - value)
    lower, upper = max(Q(0), value - radius), value + radius
    # At a binade boundary the radius covers at most 2*N lower and N upper steps.
    # Prove the local enumeration boundaries rather than silently trusting that bound.
    left = max(0, reference_bits - 2 * count - 2)
    right = min(inf - 1, reference_bits + 2 * count + 2)
    need(left == 0 or decode(left - 1, width) < lower, 'radius enumeration misses lower member')
    need(right == inf - 1 or decode(right + 1, width) > upper, 'radius enumeration misses upper member')
    radius_members = {b for b in range(left, right + 1) if lower <= decode(b, width) <= upper}
    return sorted(steps | radius_members)


def flush(bits, width):
    f, _ = FORMATS[width]
    sign, magnitude = bits >> (width - 1), bits & ((1 << (width - 1)) - 1)
    return sign << (width - 1) if 0 < magnitude < 1 << f else bits


def narrow(bits):
    magnitude = bits & 0x7fffffff
    if magnitude >= infinity(32):
        return (infinity(16) + int(magnitude > infinity(32))) | ((bits >> 31) << 15)
    return round_signed(decode(bits, 32), 16, bits == 0x80000000)


def half_corrections(original, bits, corrections=CORRECTIONS):
    """Execute every Half FMA, including inactive numeric-zero predicates."""
    for match, correction in corrections:
        magnitude = bits & 0x7fff
        if magnitude >= infinity(16):
            continue  # A finite product added to infinity/NaN preserves this classification.
        product = decode(correction, 16) if original == match else Q(0)
        bits = round_signed(product + decode(bits, 16), 16)
    return bits


def classify_input(bits, width):
    magnitude = bits & ((1 << (width - 1)) - 1)
    inf = infinity(width)
    if magnitude > inf:
        return 'nan', []
    if magnitude == inf:
        return 'special', [0 if bits >> (width - 1) else inf]
    if magnitude == 0:
        return 'special', [round_positive(Q(1), width)]
    return 'finite', None


def half_fma_input(bits):
    """Model exact finite Half widening and RN32(x*C + -0), including both input zeros."""
    x = decode(bits, 16)
    return round_signed(x * decode(C_BITS, 32), 32, negative_zero=(bits == 0x8000))


def bias_exp2(bits):
    """Round the installed CUDA Half output FMA exactly, before Half narrowing."""
    if bits >= infinity(32):
        return bits
    return round_positive(decode(bits, 32) * (1 + power2(-24)), 32)


def row(bits, width, operation="exp"):
    kind, special = classify_input(bits, width)
    if kind != 'finite':
        return {'input': bits, 'kind': kind, 'nvvm': special, 'cuda': special}
    x = decode(bits, width)
    evaluation_width = 32 if width == 16 else width
    ref = reference(x, evaluation_width, operation == "exp2")
    library = admission(ref, evaluation_width, 2 if evaluation_width == 32 else 1)
    result = {'input': bits, 'kind': kind, 'reference': ref, 'library': library}
    if width != 16:
        result.update(nvvm=library, cuda=library)
        return result
    nvvm = sorted({narrow(b) for b in library})
    if operation == 'exp2':
        input_bits = round_signed(x, 32)
        flushed_input = flush(input_bits, 32)
        ptx_ref = reference(decode(flushed_input, 32), 32, True)
        ptx_admitted = admission(ptx_ref, 32, 2)
        stages = [[b, flush(b, 32), bias_exp2(flush(b, 32)),
                   narrow(bias_exp2(flush(b, 32)))] for b in ptx_admitted]
        result.update(nvvm=nvvm, cuda=sorted({a[3] for a in stages}),
                      ideal_half=reference(x, 16, True), fma=input_bits, ptx_reference=ptx_ref,
                      ptx=ptx_admitted, cuda_stages=stages)
        return result
    # For nonzero finite Half the exact product is normal Float32, but keep FTZ explicit.
    product_bits = half_fma_input(bits)
    flushed_input = flush(product_bits, 32)
    ptx_ref = reference(decode(flushed_input, 32), 32, True)
    ptx_admitted = admission(ptx_ref, 32, 2)
    stages = [[b, flush(b, 32), narrow(flush(b, 32)),
               half_corrections(bits, narrow(flush(b, 32)))] for b in ptx_admitted]
    result.update(nvvm=nvvm, cuda=sorted({a[3] for a in stages}),
                  ideal_half=reference(x, 16), fma=product_bits, ptx_reference=ptx_ref,
                  ptx=ptx_admitted, cuda_stages=stages)
    return result


def encoded(bits, width):
    return format(bits, '0{}x'.format(width // 4))


def compute_record(width, input_bits, operation="exp"):
    """Reconstruct worker-schema numerical fields without importing generator code."""
    bits = int(input_bits, 16) if isinstance(input_bits, str) else input_bits
    r = row(bits, width, operation)
    out = {'input_bits': encoded(bits, width), 'special': r['kind'] != 'finite'}
    if r['kind'] == 'nan':
        out.update(reference_bits='nan', nvvm_candidates=['nan'], cuda_candidates=['nan'])
        return out
    for target in ('nvvm', 'cuda'):
        out[target + '_candidates'] = [encoded(b, width) for b in r[target]]
    if out['special']:
        out['reference_bits'] = encoded(r['nvvm'][0], width)
        return out
    out['reference_bits'] = encoded(reference(decode(bits, width), width, operation == "exp2"), width)
    ew = 32 if width == 16 else width
    if width == 16:
        out['library_reference_bits'] = encoded(r['reference'], ew)
        out['library_candidates'] = [encoded(b, ew) for b in r['library']]
        post = sorted({flush(b, 32) for b in r['ptx']})
        if operation == 'exp2':
            biased = sorted({bias_exp2(b) for b in post})
            out['cuda_half'] = {
                't_bits': encoded(r['fma'], 32),
                't_after_ftz_bits': encoded(flush(r['fma'], 32), 32),
                'exp2_reference_bits': encoded(r['ptx_reference'], 32),
                'pre_ftz_candidates': [encoded(b, 32) for b in r['ptx']],
                'post_ftz_candidates': [encoded(b, 32) for b in post],
                'biased_candidates': [encoded(b, 32) for b in biased],
                'narrowed_candidates': [encoded(b, 16) for b in sorted({narrow(b) for b in biased})],
            }
            return out
        narrowed = sorted({narrow(b) for b in post})
        stages = []
        current = narrowed
        for correction in CORRECTIONS:
            current = sorted({half_corrections(bits, b, (correction,)) for b in current})
            stages.append([encoded(b, 16) for b in current])
        out['cuda_half'] = {
            't_bits': encoded(r['fma'], 32),
            't_after_ftz_bits': encoded(flush(r['fma'], 32), 32),
            'exp2_reference_bits': encoded(r['ptx_reference'], 32),
            'pre_ftz_candidates': [encoded(b, 32) for b in r['ptx']],
            'post_ftz_candidates': [encoded(b, 32) for b in post],
            'narrowed_candidates': [encoded(b, 16) for b in narrowed],
            'correction_stages': stages,
        }
    return out


def compare_record(actual, expected, location='row'):
    """Check all reconstructed fields; certificates/categories are not numerical authority."""
    # References are checked first, so a broad admission set cannot mask a wrong RN result.
    ordered = sorted(expected, key=lambda key: (not key.endswith('reference_bits'), key))
    for key in ordered:
        need(key in actual, location + ': missing ' + key)
        if isinstance(expected[key], dict):
            compare_record(actual[key], expected[key], location + '.' + key)
        else:
            need(actual[key] == expected[key], location + '.' + key + ': independent value differs')


def dyadic_descriptor(x, precision, terms):
    """Audit an optional generator recipe, separately from the diverse main reference proof."""
    need((precision, terms) in ((192, 48), (384, 80), (768, 128)), 'unknown certificate resources')
    if x == 0:
        return Q(1), Q(1), {'kind': 'one'}
    if x <= -2048:
        return Q(0), power2(-2048), {'kind': 'coarse-negative'}
    need(x < 2048, 'positive coarse exp proof must be one-sided')
    a = abs(x)
    if a <= power2(-100):
        # exp(a) <= 1/(1-a) <= 1+2a for 0<=a<=1/2; exp(-a)>=1-a.
        return 1 - 2 * a, 1 + 2 * a, {'kind': 'tiny-linear'}
    squares = 0
    while a > Q(1, 16):
        a /= 2
        squares += 1
    need(squares <= 15, 'certificate reduction exceeds cap')
    partial, term = Q(1), Q(1)
    for j in range(1, terms + 1):
        term *= a / j
        partial += term
    omitted = term * a / (terms + 1)
    tail = omitted / (1 - a / (terms + 2))
    scale = 1 << precision
    l = (partial.numerator * scale) // partial.denominator
    upper = partial + tail
    u = ceil_div(upper.numerator * scale, upper.denominator)
    initial = [str(l), str(u)]
    for _ in range(squares):
        l, u = l * l // scale, ceil_div(u * u, scale)
    # A stored final pair always describes exp(abs(x)); reciprocation reverses endpoints.
    low, high = (Q(l, scale), Q(u, scale)) if x > 0 else (Q(scale, u), Q(scale, l))
    return low, high, {'kind': 'dyadic-taylor', 'precision': precision, 'terms': terms,
                       'squares': squares, 'initial': initial, 'final': [str(l), str(u)]}


@lru_cache(maxsize=None)
def descriptor_log_bounds(precision):
    count = (precision + 2) // 3 + 4
    lower = sum((Q(2, (2 * j + 1) * 3 ** (2 * j + 1)) for j in range(count)), Q(0))
    upper = lower + Q(18, 8 * (2 * count + 1) * 3 ** (2 * count + 1))
    scale = 1 << precision
    return Q(lower.numerator * scale // lower.denominator, scale), Q(ceil_div(upper.numerator * scale, upper.denominator), scale)


def audit_certificate(x, width, reference_bits, descriptor, base_two=False):
    """Verify every supplied proof field, its enclosing inequalities and the claimed RN cell."""
    kind = descriptor.get('kind')
    if base_two:
        if x >= 2048:
            need(descriptor == {'kind': 'coarse-positive'} and reference_bits == infinity(width), 'bad coarse exp2 upper classification')
            return
        if x <= -2048:
            need(descriptor == {'kind': 'coarse-negative'} and reference_bits == 0, 'bad coarse exp2 lower classification')
            return
        k = x.numerator // x.denominator
        fraction = x - k
        if not fraction:
            need(descriptor == {'kind': 'exact-power', 'k': k}, 'invalid exact-power descriptor')
            low = high = power2(k)
        else:
            precision, terms = descriptor.get('precision'), descriptor.get('terms')
            need((precision, terms) in ((192, 48), (384, 80), (768, 128)), 'invalid exp2 certificate resources')
            need(descriptor == {'kind': 'ln2-exp', 'precision': precision, 'terms': terms, 'k': k}, 'invalid ln2-exp descriptor')
            a, b = descriptor_log_bounds(precision)
            low, _, _ = dyadic_descriptor(fraction * a, precision, terms)
            _, high, _ = dyadic_descriptor(fraction * b, precision, terms)
            low, high = low * power2(k), high * power2(k)
    elif x >= 2048:
        need(descriptor == {'kind': 'coarse-positive-one-sided', 'strict_lower_power': 2048}, 'invalid one-sided coarse exp proof')
        need(reference_bits == infinity(width), 'coarse positive exp must round to infinity')
        # exp(x)>2^2048 exceeds every supported overflow threshold; no fake finite upper bound.
        need(power2(2048) > midpoint_before(infinity(width), width), 'coarse threshold proof failed')
        return
    else:
        precision, terms = descriptor.get('precision', 192), descriptor.get('terms', 48)
        low, high, expected = dyadic_descriptor(x, precision, terms)
        need(descriptor == expected, 'generator Taylor/tail/square/reciprocal descriptor differs')
    need(cell_contains(low, high, reference_bits, width), 'generator certificate does not fit claimed rounding cell')


def audit_record_certificates(record, width, operation="exp"):
    if record['special']:
        need(not any('certificate' in key for key in record), 'special inputs must not carry finite proofs')
        return
    x = decode(int(record['input_bits'], 16), width)
    audit_certificate(x, width, int(record['reference_bits'], 16), record['reference_certificate'], operation == 'exp2')
    if width == 16:
        audit_certificate(x, 32, int(record['library_reference_bits'], 16), record['library_certificate'], operation == 'exp2')
        cuda = record['cuda_half']
        audit_certificate(decode(int(cuda['t_after_ftz_bits'], 16), 32), 32,
                          int(cuda['exp2_reference_bits'], 16), cuda['exp2_certificate'], True)



def check_proposal(path, negative=False, operation="exp"):
    proposal = json.loads(path.read_text())
    need(proposal['schema'] == 'nvvm-' + operation + '-policy-v1', 'unrecognized proposal schema')
    need(proposal['policy_ids'] == {'nvvm_half': 56042 if operation == 'exp2' else 55040, 'cuda_half': 1209}, 'policy markers differ')
    summaries = []
    half_records = []
    for group in proposal['formats']:
        width = group['width']
        seen = set()
        for actual in group['records']:
            need(actual['input_bits'] not in seen, 'duplicate input bits')
            seen.add(actual['input_bits'])
            expected = compute_record(width, actual['input_bits'], operation)
            compare_record(actual, expected, str(width) + ':' + actual['input_bits'])
            audit_record_certificates(actual, width, operation)
            if width == 16:
                half_records.append((actual, expected))
        summaries.append({'width': width, 'records': len(seen)})
    result = {'status': 'pass', 'formats': summaries, 'manifest_sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
    if negative and operation == 'exp2':
        controls = []
        candidate = next((a, e) for a, e in half_records if not a['special'] and
                         a['reference_certificate']['kind'] == 'ln2-exp')
        for field in ('reference_bits', 'library_reference_bits', 'nvvm_candidates', 'cuda_candidates'):
            altered = copy.deepcopy(candidate[0])
            altered[field] = ['0000'] if field.endswith('candidates') else '0000'
            try:
                compare_record(altered, candidate[1])
            except ValueError as error:
                controls.append({'mutation': field, 'rejected': str(error)})
            else:
                raise ValueError('negative control admitted: ' + field)
        for field in candidate[1]['cuda_half']:
            altered = copy.deepcopy(candidate[0])
            altered['cuda_half'][field] = [] if isinstance(altered['cuda_half'][field], list) else '00000000'
            try:
                compare_record(altered, candidate[1])
            except ValueError as error:
                controls.append({'mutation': field, 'rejected': str(error)})
            else:
                raise ValueError('negative control admitted: ' + field)
        for field in ('k', 'precision', 'terms'):
            altered = copy.deepcopy(candidate[0])
            altered['reference_certificate'][field] += 1
            try:
                audit_record_certificates(altered, 16, operation)
            except ValueError as error:
                controls.append({'mutation': 'certificate-' + field, 'rejected': str(error)})
            else:
                raise ValueError('negative certificate admitted: ' + field)
        result['negative_controls'] = controls
        return result
    if negative:
        controls = []
        candidate = next((a, e) for a, e in half_records if a['input_bits'] == '1f79')
        mutations = {
            'wrong-reference': ('reference_bits', '0000'),
            'wrong-library-reference': ('library_reference_bits', '00000000'),
            'wrong-nvvm-image': ('nvvm_candidates', ['0000']),
            'missing-correction': ('cuda_candidates', candidate[0]['cuda_half']['narrowed_candidates']),
            'swapped-half-policy': ('cuda_candidates', candidate[0]['nvvm_candidates']),
        }
        for label, (key, value) in mutations.items():
            altered = copy.deepcopy(candidate[0])
            altered[key] = value
            try:
                compare_record(altered, candidate[1])
            except ValueError as error:
                controls.append({'mutation': label, 'rejected': str(error)})
            else:
                raise ValueError('negative control admitted: ' + label)
        for key, value in (('t_bits', '00000000'), ('t_after_ftz_bits', '00000000'),
                           ('exp2_reference_bits', '00000000'), ('pre_ftz_candidates', ['00000000']),
                           ('post_ftz_candidates', ['00000000']), ('correction_stages', [])):
            altered = copy.deepcopy(candidate[0])
            altered['cuda_half'][key] = value
            try:
                compare_record(altered, candidate[1])
            except ValueError as error:
                controls.append({'mutation': key, 'rejected': str(error)})
            else:
                raise ValueError('negative control admitted: ' + key)
        for label, field in (('Taylor-tail-bound', 'initial'), ('outward-square', 'final'),
                             ('reciprocal-endpoint-order', 'final'), ('square-count', 'squares')):
            source = next(a for a, _ in half_records if not a['special'] and
                          a['reference_certificate'].get('kind') == 'dyadic-taylor' and
                          a['reference_certificate']['squares'] > 0 and
                          bool(int(a['input_bits'], 16) & 0x8000) == (label == 'reciprocal-endpoint-order'))
            altered = copy.deepcopy(source)
            proof = altered['reference_certificate']
            if label == 'reciprocal-endpoint-order':
                proof[field].reverse()
            elif isinstance(proof[field], list):
                proof[field][1] = str(int(proof[field][1]) - 1)
            else:
                proof[field] += 1
            try:
                audit_record_certificates(altered, 16)
            except ValueError as error:
                controls.append({'mutation': label, 'rejected': str(error)})
            else:
                raise ValueError('negative proof control admitted: ' + label)
        altered = copy.deepcopy(candidate[0])
        altered['cuda_half']['exp2_certificate']['k'] += 1
        try:
            audit_record_certificates(altered, 16)
        except ValueError as error:
            controls.append({'mutation': 'ln2-reduction-k', 'rejected': str(error)})
        else:
            raise ValueError('ln2 certificate mutation admitted')
        result['negative_controls'] = controls
    return result


def ordering_key(bits, width):
    sign = 1 << (width - 1)
    return ((1 << width) - 1) ^ bits if bits & sign else bits + sign


def bits_from_key(key, width):
    sign = 1 << (width - 1)
    return ((1 << width) - 1) ^ key if key < sign else key - sign


def compare_exp_to_boundary(x, boundary, operation="exp"):
    if x >= 2048:
        need(boundary < power2(2048), 'coarse output boundary too large')
        return 1
    if x <= -2048:
        need(boundary > power2(-2048), 'coarse output boundary too small')
        return -1
    for precision in (128, 192, 256, 384, 512):
        low, high = (exp2_enclosure if operation == "exp2" else exp_enclosure)(x, precision)
        if high < boundary:
            return -1
        if low > boundary:
            return 1
        if low == high == boundary:
            return 0
    raise ValueError('input-boundary comparison not certified within resource cap')


@lru_cache(maxsize=None)
def frozen_inputs(width, operation="exp"):
    """Reconstruct the bounded corpus from exact classes and independently certified boundaries."""
    fraction, bias = FORMATS[width]
    inf = infinity(width)
    sign = 1 << (width - 1)
    required = set()
    for magnitude in (0, 1, 2, (1 << fraction) - 1, 1 << fraction,
                      (1 << fraction) + 1, inf - 1, inf, inf + 1, inf + (1 << (fraction - 1))):
        required.update((magnitude, magnitude | sign))
    for numerator, denominator in ((-16, 1), (-8, 1), (-4, 1), (-2, 1), (-1, 1), (-1, 2),
                                   (-1, 4), (1, 4), (1, 2), (1, 1), (2, 1), (4, 1),
                                   (8, 1), (16, 1), (-32, 1), (-64, 1), (-100, 1), (-700, 1)):
        required.add(round_signed(Q(numerator, denominator), width))
    boundaries = [(power2(-bias - fraction), (-2, -1, 0, 1)),
                  (power2(1 - bias) - power2(-bias - fraction), (-2, -1, 0, 1)),
                  (midpoint_before(inf, width), (-2, -1, 0, 1)),
                  (Q(1, 2), (-2, -1, 0, 1)), (Q(2), (-2, -1, 0, 1)),
                  (Q(4), (-2, -1, 0, 1))]
    if operation == 'exp2':
        required.update(round_signed(Q(e), width) for e in
                        (-bias - fraction, 1 - bias - fraction, 1 - bias, bias, bias + 1))
        boundaries.extend(((Q(1) + power2(-11), (-2, -1, 0, 1)),
                           (Q(1) + 3 * power2(-11), (-2, -1, 0, 1))))
    if width == 16 and operation == 'exp':
        for match, _ in CORRECTIONS:
            required.update((match - 1, match, match + 1))
    if width == 16:
        boundaries.extend(((power2(-150), (-1, 0)),
                           (power2(-126) - power2(-150), (-1, 0)),
                           (midpoint_before(infinity(32), 32), (-1, 0))))
    for threshold, neighbors in boundaries:
        left = ordering_key((inf - 1) | sign, width)
        right = ordering_key(inf - 1, width)
        while left < right:
            middle = (left + right) // 2
            x = decode(bits_from_key(middle, width), width)
            if compare_exp_to_boundary(x, threshold, operation) >= 0:
                right = middle
            else:
                left = middle + 1
        required.update(bits_from_key(left + delta, width) for delta in neighbors)
    need(len(required) <= 96, 'bounded corpus cap exceeded')
    if operation == 'exp':
        need(len(required) == {16: 80, 32: 62, 64: 62}[width], 'bounded exp corpus count changed')
    return sorted(required)


def buffer(text, name):
    matches = re.findall(r'^//TEST_INPUT: ubuffer\(data=\[([0-9 ]+)\], stride=4\):(?:out,)?name=' + re.escape(name) + '$', text, re.M)
    need(len(matches) == 1, name + ': missing or duplicate buffer')
    words = list(map(int, matches[0].split()))
    need(all(0 <= w < 1 << 32 for w in words), name + ': invalid uint word')
    return words


def fixture_table(records, target):
    words = []
    for r in records:
        candidates = r[target + '_candidates']
        is_nan = candidates == ['nan']
        bits = [] if is_nan else [int(v, 16) for v in candidates]
        need(len(bits) <= 7, 'fixture candidate capacity exceeded')
        words.extend((int(is_nan), len(bits)))
        for b in bits + [0] * (7 - len(bits)):
            words.extend((b & 0xffffffff, b >> 32))
    return words


def check_fixture(path, width, operation="exp"):
    text = path.read_text()
    marker = 56042 if operation == 'exp2' else 55040
    completion = 56000 if operation == 'exp2' else 55000
    words = buffer(text, 'inputWords')
    need(len(words) % 2 == 0, 'unpaired IEEE input words')
    inputs = [words[i] | (words[i + 1] << 32) for i in range(0, len(words), 2)]
    need(inputs == frozen_inputs(width, operation), 'fixture boundary/material/correction input inventory differs')
    records = [compute_record(width, bits, operation) for bits in inputs]
    count = len(inputs)
    need(buffer(text, 'expectedWords') == fixture_table(records, 'nvvm'), 'fixture NVVM candidates differ')
    base = 2 if width == 16 else 1
    need(buffer(text, 'outputBuffer') == [0x13579bdf] + [0] * (4 * count + base - 1) + [0xdeadbeef], 'output guards/layout differ')
    if width == 16:
        need(buffer(text, 'cudaWords') == fixture_table(records, 'cuda'), 'fixture CUDA Half candidates differ')
        for snippet in ('case nvvm: return true;', 'default: return false;',
                        'return usesLibraryPolicy() ? expectedWords[offset] : cudaWords[offset];',
                        f'if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? {marker}u : 1209u;'):
            need(snippet in text, 'missing Half policy selection/marker: ' + snippet)
        need(text.count('filecheck-buffer=CUDA') == 1 and text.count('filecheck-buffer=NVVM') == 2, 'Half mode directives differ')
    else:
        need('return expectedWords[offset];' in text and text.count('filecheck-buffer=CHECK') == 3, 'library policy/mode directives differ')
    for snippet in ('uint offset = 16 * lane;', 'if (expectedWord(offset) != 0) return nanBits;',
                    'uint count = expectedWord(offset + 1);', 'for (uint choice = 0; choice < count; ++choice)',
                    'if (low == expectedWord(offset + 2 + 2 * choice) &&',
                    'high == expectedWord(offset + 3 + 2 * choice)) return true;',
                    'valueBits(scalarResult, low, high);', '[numthreads(128, 1, 1)]', f'if (lane >= {count}) return;'):
        need(snippet in text, 'missing discrete admission or live observation: ' + snippet)
    typ = {16: 'half', 32: 'float', 64: 'double'}[width]
    need(typ + f' scalarResult = {operation}(loadValue(lane));' in text, 'scalar exp is not live')
    need(re.search(r'\[noinline\]\s+' + typ + ' ' + operation + r'ThroughHelper\(' + typ + r' value\)\s*\{\s*return ' + operation + r'\(value\);\s*\}', text), 'helper exp is not live')
    comparisons = ['scalarResult, lane', operation + 'ThroughHelper(loadValue(lane)), lane']
    masks = [1, 2]
    for size in (2, 3, 4):
        args = ', '.join(f'loadValue((lane + {i}) % {count})' for i in range(size))
        need(f'{typ}{size} result{size} = {operation}({typ}{size}({args}));' in text, 'vector exp is not live')
        for i, component in enumerate('xyzw'[:size]):
            comparisons.append(f'result{size}.{component}, (lane + {i}) % {count}')
            masks.append(1 << size)
    args = ', '.join(f'loadValue((lane + {i}) % {count})' for i in range(4))
    need(f'matrix<{typ}, 2, 2> resultMatrix = {operation}(matrix<{typ}, 2, 2>({args}));' in text, 'matrix exp is not live')
    for i in range(4):
        comparisons.append(f'resultMatrix[{i // 2}][{i % 2}], (lane + {i}) % {count}')
        masks.append(32)
    need(text.count('errors |= matchesExpected(') == 15, '15 live observations required')
    for expression, mask in zip(comparisons, masks):
        need(f'errors |= matchesExpected({expression}) ? 0u : {mask}u;' in text, 'missing comparison: ' + expression)
    for offset, value in enumerate(('errors', 'low', 'high', f'{completion} + lane')):
        need(f'outputBuffer[{base + offset} + 4 * lane] = {value};' in text, 'raw observation/completion layout differs')
    for prefix in (('CUDA', 'NVVM') if width == 16 else ('CHECK',)):
        checks = re.findall(r'^// ' + prefix + r'(-NEXT)?: (.+)\{\{\$\}\}$', text, re.M)
        expected = [str(0x13579bdf)]
        if width == 16:
            expected.append(str(marker) if prefix == 'NVVM' else '1209')
        for lane in range(count):
            expected += ['0', '{{[0-9]+}}', '{{[0-9]+}}', str(completion + lane)]
        expected.append(str(0xdeadbeef))
        need([v for _, v in checks] == expected, 'full output filecheck differs')
        need(checks[0][0] == '' and all(k == '-NEXT' for k, _ in checks[1:]), 'filecheck must be contiguous')
    prefixes = ('CUDA', 'NVVM', 'NVVM') if width == 16 else ('CHECK', 'CHECK', 'CHECK')
    modes = ('-Xslang -O3', '-Xslang -emit-cuda-via-nvvm -Xslang -O0',
             '-Xslang -emit-cuda-via-nvvm -Xslang -O3')
    directives = [f'//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer={prefix}): '
                  '-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 ' + mode
                  for prefix, mode in zip(prefixes, modes)]
    need(re.findall(r'^//TEST\(compute\):.*$', text, re.M) == directives,
         'exact NVRTC O3 / NVVM O0 / NVVM O3 directives differ')
    return {'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'inputs': count,
            'observations_per_lane': 15, 'output_words': 4 * count + base + 1,
            'policy_difference_inputs': [r['input_bits'] for r in records if r['nvvm_candidates'] != r['cuda_candidates']]}



def self_test():
    """Check proof boundary rules and independently selected negative controls."""
    checks = []
    for width, (fraction, bias) in FORMATS.items():
        inf = infinity(width)
        for b in (1, 2, 3, (1 << fraction) - 1, 1 << fraction, (bias << fraction),
                  (bias << fraction) + 1, inf - 1, inf):
            midpoint = midpoint_before(b, width)
            expected = b if b % 2 == 0 else b - 1
            need(round_positive(midpoint, width) == expected, 'midpoint parity failed')
            need(cell_contains(midpoint, midpoint, expected, width), 'correct tie cell rejected')
            need(not cell_contains(midpoint, midpoint, expected + (1 if b % 2 else -1), width),
                 'wrong midpoint parity accepted')
            checks.append(f'midpoint-{width}-{b}')
        need(admission(0, width, 2) == [0, 1, 2], 'zero extension differs')
        need(admission(inf, width, 2) == [inf - 2, inf - 1, inf], 'infinity clipping differs')
        need(admission(inf - 1, width, 2) == [inf - 3, inf - 2, inf - 1, inf], 'maxfinite differs')
        need(flush(1, width) == 0 and flush(1 | (1 << (width - 1)), width) == 1 << (width - 1),
             'FTZ sign or subnormal classification differs')
        for e in (1 - bias - fraction - 1, 1 - bias, 0, bias + 1):
            exact = power2(e)
            need(reference(Q(e), width, True) == round_positive(exact, width), 'exp2 exact power failed')
        checks.append(f'endpoints-ftz-exp2-{width}')
    # A tighter high-precision ln2 enclosure must nest inside the coarse certified interval.
    a, b = log_two_bounds(128)
    c, d = log_two_bounds(256)
    need(Q(a, 1 << 128) < Q(c, 1 << 256) < Q(d, 1 << 256) < Q(b, 1 << 128), 'ln2 tails fail nesting')
    # Deliberately remove all upper rounding/tail slack: the truncated lower sum is not an upper bound.
    need(not (Q(c, 1 << 256) <= Q(a, 1 << 128)), 'bad ln2 upper bound accepted')
    checks.append('ln2-tail-negative')
    need(round_signed(Q(0), 32, True) == 0x80000000, 'negative zero lost')
    need(round_signed(Q(0), 32) == 0, 'exact cancellation zero must be positive')
    need(half_fma_input(0) == 0 and half_fma_input(0x8000) == 0x80000000,
         'Half FMA with negative-zero addend has wrong signed-zero result')
    need(flush(half_fma_input(0x8000), 32) == 0x80000000, 'ex2 input FTZ lost negative zero')
    checks.append('signed-zero-FMA-and-FTZ')
    synthetic = [0, 1, 0x007fffff, 0x00800000, 0x80000000, 0x80000001, 0x807fffff, 0x80800000]
    correct = [0, 0, 0, 0x00800000, 0x80000000, 0x80000000, 0x80000000, 0x80800000]
    need([flush(b, 32) for b in synthetic] == correct, 'synthetic FTZ stage differs')
    need(synthetic != correct, 'missing-FTZ mutation is ineffective')
    need([b & 0x7fffffff for b in correct] != correct, 'lost-FTZ-sign mutation is ineffective')
    checks.append('synthetic-FTZ-stage-and-sign-negatives')
    correction_rows = []
    for bits, correction in CORRECTIONS:
        r = row(bits, 16)
        before = sorted({s[2] for s in r['cuda_stages']})
        need(before != r['cuda'], 'missing-correction mutation is not detected')
        correction_rows.append(r)
        checks.append(f'correction-{bits}')
    need(decode(C_BITS, 32) == Q(12102203, 8388608), 'FMA encoded constant mismatch')
    need(any(round_signed(decode(r['input'], 16) * decode(C_BITS + 1, 32), 32) != r['fma']
             for r in correction_rows), 'wrong FMA constant is not detected')
    checks.append('fma-constant-negative')
    return {'status': 'pass', 'checks': checks, 'correction_rows': correction_rows}


def exp2_self_test():
    """Cross-check exact FMA arithmetic against the independently derived encoding rule."""
    checks = []
    for bits in (0, 0x00800000, 0x00800001, 0x3f000000, 0x3f800000,
                 0x3f800001, 0x3f801000, 0x3f803000, 0x7f7fffff, 0x7f800000):
        expected = bits if bits in (0, infinity(32)) or bits & 0x7fffff == 0 else bits + 1
        need(bias_exp2(bits) == expected, 'CUDA exp2 bias analytical rule differs')
        checks.append('bias-' + encoded(bits, 32))
    midpoint = 0x3f801000
    need(narrow(bias_exp2(midpoint)) != narrow(midpoint), 'omitted bias not detected')
    rounded_factor = round_positive(1 + power2(-24), 32)
    need(round_positive(decode(midpoint, 32) * decode(rounded_factor, 32), 32) != bias_exp2(midpoint),
         'premature factor rounding not detected')
    need(narrow(bias_exp2(midpoint)) != round_positive(decode(narrow(midpoint), 16) * (1 + power2(-24)), 16),
         'bias after Half narrowing not detected')
    need(round_positive(decode(midpoint, 32) * (1 + power2(-25)), 32) != bias_exp2(midpoint),
         'wrong FMA multiplier not detected')
    need([narrow(midpoint)] != [narrow(bias_exp2(midpoint))], 'synthetic target-policy swap not detected')
    need(bias_exp2(flush(1, 32)) != bias_exp2(1), 'missing output FTZ stage not detected')
    checks.extend(('missing-bias', 'premature-factor-rounding', 'bias-after-narrow', 'wrong-multiplier',
                   'synthetic-target-policy-swap', 'missing-output-ftz'))
    for width, (fraction, bias) in FORMATS.items():
        x = Q(-bias - fraction)
        need(reference(x, width, True) == 0, 'exact exp2 underflow tie is not even zero')
        need(reference(x + 1, width, True) == 1, 'exact exp2 minimum subnormal differs')
        checks.append('exact-underflow-' + str(width))
    return checks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--operation', choices=('exp', 'exp2'), default='exp')
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--rows', type=Path, help='CPU proposal JSON with width -> array of IEEE inputs')
    parser.add_argument('--proposal', '--manifest', type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--directory', type=Path, default=Path(__file__).resolve().parents[2] / 'tests/cuda')
    parser.add_argument('--negative-controls', action='store_true')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    need(not args.negative_controls or args.proposal, '--negative-controls requires --proposal')
    start = time.monotonic()
    result = self_test() if args.self_test else {}
    if args.self_test and args.operation == 'exp2':
        result['checks'].extend(exp2_self_test())
    if args.check:
        result['fixtures'] = [check_fixture(args.directory / ('nvvm-' + args.operation + '-' + name + '.slang'), width, args.operation)
                              for width, name in ((16, 'half'), (32, '32'), (64, '64'))]
    if args.proposal:
        result['proposal'] = check_proposal(args.proposal, args.negative_controls, args.operation)
    if args.rows:
        inputs = json.loads(args.rows.read_text())
        result['rows'] = {w: [row(b, int(w), args.operation) for b in bits] for w, bits in inputs.items()}
    result['elapsed_seconds'] = time.monotonic() - start
    result['checker_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    text = json.dumps(result, indent=2) + '\n'
    if args.output:
        need(not args.output.exists(), 'preserve existing result; select a unique output')
        args.output.write_text(text)
    else:
        print(text, end='')


if __name__ == '__main__':
    main()
