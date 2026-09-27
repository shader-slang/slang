#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Qualify unchanged tiled-brass eval or sample entries with two synthetic live CUDA textures.

This fixed graph contract covers finite front-facing reflection at fixed texture coordinates,
not original assets, LUT reads, arbitrary material graphs, or GPU performance. Run on native
Linux with the selected CUDA headers. Each invocation requires a new artifact directory.
"""

import argparse
from datetime import datetime, timezone
import importlib.util
import json
import math
import os
from pathlib import Path
import platform
import re
import shutil
import struct
import subprocess
import sys

REPO = Path(__file__).resolve().parents[1]
CONTRACT = "tiled-brass-eval-synthetic-textures-v1"
ACTIVE_COUNT, OUTPUT_COUNT = 65, 128
INPUT = struct.Struct("<8fI4x")
OUTPUT = struct.Struct("<4f")
SENTINEL = b"\xa5" * OUTPUT.size
# Frozen before GPU comparison. Independent source-style float32/FMA CPU arithmetic over the
# original texel-center inputs differed by <8.5e-7 absolute and <5.6e-7 relative. This leaves
# >400x observed arithmetic margin for CUDA normalization/transcendentals, while perturbations
# exceed the comparison budget by >349x. This empirical finite-input envelope is not a proof
# about arbitrary inputs or a promise of correctly rounded CUDA transcendental instructions.
ABS_TOL, REL_TOL = 1e-5, 2e-4
COLORS = ((.02, .35, .8, 1), (.8, .15, .04, 1), (.25, .6, .1, 1), (.7, .4, .9, 1))
ROUGHNESS = (.25, .4, .6, .8)
DIRECTIONS = (((0, 0, 1), (0, 0, 1)),
              ((.3, .4, 1), (-.2, .1, 1)),
              ((-.5, .2, 1), (.4, -.3, 1)))
# MaterialX analytic directional-albedo fit, expressed as coefficients of
# [1, x, y, xy, x², y², x²y, xy², x²y²]. This is the graph's selected policy.
ALBEDO_COEFFICIENTS = (
    (.1003, .9345, 1, 1), (-.6303, -2.323, -1.765, .2281),
    (9.748, 2.229, 8.263, 15.94), (-2.038, -3.748, 11.53, -55.83),
    (29.34, 1.424, 28.96, 13.08), (-8.245, -.7684, -7.507, 41.26),
    (-26.44, 1.436, -36.11, 54.9), (19.99, .2913, 15.86, 300.2),
    (-5.448, .6286, 33.37, -285.1),
)


def load_tool(name, relative):
    spec = importlib.util.spec_from_file_location(name, REPO / relative)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def f32(value):
    """Round uploaded values once; the independent equations then use double precision."""
    return struct.unpack("<f", struct.pack("<f", value))[0]


def normalize(vector):
    length = math.sqrt(sum(value * value for value in vector))
    return tuple(value / length for value in vector)


def fresnel(index, cosine):
    """Evaluate unpolarized nonabsorbing Fresnel using Snell's law and s/p amplitudes."""
    transmitted = math.sqrt(1 - (1 - cosine * cosine) / (index * index))
    s = (cosine - index * transmitted) / (cosine + index * transmitted)
    p = (index * cosine - transmitted) / (index * cosine + transmitted)
    return (s * s + p * p) / 2


def albedo(cosine, roughness):
    """Evaluate the selected rational polynomial without reproducing shader aggregate logic."""
    x, y = cosine, roughness
    terms = (1, x, y, x*y, x*x, y*y, x*x*y, x*y*y, x*x*y*y)
    parts = [sum(row[column] * term for row, term in zip(ALBEDO_COEFFICIENTS, terms))
             for column in range(4)]
    return sum(min(1, max(0, parts[i] / parts[i + 2])) for i in (0, 1))


def linear_color(color):
    return tuple(value / 12.92 if value <= .04045 else ((value + .055) / 1.055) ** 2.4
                 for value in color[:3])


def reference(color, roughness, incoming, outgoing, omit=None):
    """Reduce this fixed two-lobe graph to isotropic GGX, Fresnel and analytic compensation.

    Both lobes share alpha=r² and their PDF, so normalized selection weights cancel. The color
    texture attenuates the conductor under the dielectric coat. The artistic IOR with reflectivity
    .99 and edge color zero has zero extinction in exact arithmetic; its large real index still
    gives a .99 normal reflectance. Source float cancellation in extinction is inside the frozen
    finite-input budget. This oracle neither compiles Slang nor consumes NVRTC output.
    """
    i, o = normalize(incoming), normalize(outgoing)
    h = normalize(tuple(a + b for a, b in zip(i, o)))
    c = i[2]
    a2 = roughness ** 4
    distribution = a2 / (math.pi * (h[2] * h[2] * (a2 - 1) + 1) ** 2)
    def smith_lambda(v):
        return (math.sqrt(1 + a2 * (1 - v[2] * v[2]) / (v[2] * v[2])) - 1) / 2
    visibility = 1 / (1 + smith_lambda(i) + smith_lambda(o))
    s2 = (1 + math.hypot(i[0], i[1])) ** 2
    k = (1 - a2) * s2 / (s2 + a2 * c * c)
    t = math.sqrt(a2 * (i[0] * i[0] + i[1] * i[1]) + c * c)
    pdf = distribution / (2 * (k * c + t))
    energy = albedo(c, roughness)
    compensation = (1 - energy) / max(energy, 1e-6)
    micro_cosine = sum(a * b for a, b in zip(h, i))
    dielectric = fresnel(1.5, micro_cosine)
    metal_index = (1 + math.sqrt(.99)) / (1 - math.sqrt(.99))
    conductor = fresnel(metal_index, micro_cosine)
    coat = dielectric * (1 + dielectric * compensation) if omit != "coat" else 0
    base = conductor * (1 + conductor * compensation) if omit != "base" else 0
    macro_transmission = 1 - fresnel(1.5, c)
    scale = distribution * visibility / (4 * c)
    return tuple(scale * (coat + component * macro_transmission * base)
                 for component in linear_color(color)) + (pdf,)


INPUT_PROFILES = ("texel-centers", "linear-filtering")
FILTERING_UVS = ((3/8, 1/4), (1/4, 5/8), (3/8, 5/8), (7/8, 1/8))


def input_locations(input_profile):
    """Choose the four fixed sample locations without changing the default input contract."""
    if input_profile == "texel-centers":
        return tuple((.25 + .5*(index % 2), .25 + .5*(index // 2)) for index in range(4))
    if input_profile == "linear-filtering":
        return FILTERING_UVS
    raise ValueError("unknown material input profile: " + input_profile)


def filtered_texture_inputs(uv, address_mode="wrap"):
    """Interpolate uploaded Float32 texels before the graph decodes sampled sRGB color.

    The normalized 2x2 footprint is centered at (2u-1/2, 2v-1/2). The frozen profile
    uses exact quarter weights, so CUDA's eight fractional weight bits do not quantize
    them. Clamp is used only as a counterfactual for checking seam discrimination.
    """
    if address_mode not in ("wrap", "clamp"):
        raise ValueError("unknown texture address mode")
    x, y = (2*coordinate - .5 for coordinate in uv)
    left, bottom = math.floor(x), math.floor(y)
    a, b = x-left, y-bottom
    color, roughness = [0.0]*4, 0.0
    for dx, wx in ((0, 1-a), (1, a)):
        for dy, wy in ((0, 1-b), (1, b)):
            column, row = left+dx, bottom+dy
            if address_mode == "wrap":
                column, row = column % 2, row % 2
            else:
                column, row = min(1, max(0, column)), min(1, max(0, row))
            index, weight = row*2+column, wx*wy
            for channel in range(4):
                color[channel] += weight*f32(COLORS[index][channel])
            roughness += weight*f32(ROUGHNESS[index])
    return tuple(color), roughness


def texture_inputs(row, input_profile="texel-centers"):
    """Resolve one profile's sampled color and roughness for references and seed selection."""
    input_locations(input_profile)
    if input_profile == "linear-filtering":
        return filtered_texture_inputs(row["uv"])
    return tuple(map(f32, COLORS[row["texel"]])), f32(ROUGHNESS[row["texel"]])


def filtering_oracle_checks():
    """Require the finite profile to distinguish common texture and decode mistakes.

    Each mistake must change at least one observed eval component by over 100 tolerance
    units. The seam case distinguishes clamp; axis-only cases distinguish exchanged UVs.
    Decode-before-filter is computed from the graph's linear dependence on decoded color,
    without changing the actual reference or reproducing shader aggregate operations.
    """
    separation = dict.fromkeys(("point", "clamp", "swapped_axes", "decode_before_filter"), 0.0)
    # Independently enumerated row-major interpolation weights for the four frozen UVs.
    weights = ((12, 4, 0, 0), (4, 0, 12, 0), (3, 1, 9, 3), (3, 9, 1, 3))
    for uv, numerators in zip(FILTERING_UVS, weights):
        color, roughness = filtered_texture_inputs(uv)
        point_index = (math.floor(2*uv[1]) % 2)*2 + math.floor(2*uv[0]) % 2
        alternatives = {
            "point": (tuple(map(f32, COLORS[point_index])), f32(ROUGHNESS[point_index])),
            "clamp": filtered_texture_inputs(uv, "clamp"),
            "swapped_axes": filtered_texture_inputs(tuple(reversed(uv))),
        }
        wrong_linear = tuple(sum(n*linear_color(tuple(map(f32, texel)))[channel]/16
                                 for n, texel in zip(numerators, COLORS)) for channel in range(3))
        for incoming, outgoing in DIRECTIONS:
            incoming, outgoing = tuple(map(f32, incoming)), tuple(map(f32, outgoing))
            wanted = reference(color, roughness, incoming, outgoing)
            changed = {name: reference(c, r, incoming, outgoing)
                       for name, (c, r) in alternatives.items()}
            coat = reference((0., 0., 0., 1.), roughness, incoming, outgoing)
            decoded = linear_color(color)
            changed["decode_before_filter"] = tuple(
                coat[channel] + (wanted[channel]-coat[channel])*wrong_linear[channel]/decoded[channel]
                for channel in range(3)) + (wanted[3],)
            for name, output in changed.items():
                separation[name] = max(separation[name], max(
                    abs(actual-expected)/tolerance(expected)
                    for actual, expected in zip(output, wanted)))
    if min(separation.values()) <= 100:
        raise ValueError("filtering counterfactual separation is below 100 tolerance units")
    return {"maximum_counterfactual_tolerance_multiples": separation}


def cases(input_profile="texel-centers"):
    """Use twelve location/direction pairs and a wrapped UV, repeated with different seeds."""
    locations = input_locations(input_profile)
    base = []
    for texel in range(4):
        for direction, (incoming, outgoing) in enumerate(DIRECTIONS):
            base.append(dict(texel=texel, direction=direction,
                             uv=locations[texel],
                             incoming=tuple(map(f32, incoming)), outgoing=tuple(map(f32, outgoing))))
    base.append(dict(base[0], uv=(locations[0][0]+1, locations[0][1]-1)))
    return [dict(base[index % len(base)], seed=17 + index * 7919) for index in range(ACTIVE_COUNT)]


def expected_outputs(inputs, input_profile="texel-centers"):
    return [reference(*texture_inputs(row, input_profile), row["incoming"], row["outgoing"])
            for row in inputs]


def tolerance(value):
    return ABS_TOL + REL_TOL * abs(value)


def oracle_checks(inputs, expected, input_profile="texel-centers"):
    """Require positive finite outputs and distinguish omitted lobes and wrong texture inputs."""
    if len(inputs) != ACTIVE_COUNT or len(expected) != ACTIVE_COUNT:
        raise ValueError("oracle requires all 65 records")
    if not all(math.isfinite(v) and v > 0 for row in expected for v in row):
        raise ValueError("oracle outputs must all be finite and positive")
    minimum = dict.fromkeys(("missing_coat", "missing_base", "wrong_color", "wrong_roughness"), math.inf)
    for row, output in zip(inputs[:12], expected[:12]):
        texel = row["texel"]
        color, roughness = texture_inputs(row, input_profile)
        for name in minimum:
            altered = reference(
                tuple(map(f32, COLORS[(texel + 1) % 4])) if name == "wrong_color" else color,
                f32(ROUGHNESS[(texel + 1) % 4]) if name == "wrong_roughness" else roughness,
                row["incoming"], row["outgoing"],
                omit={"missing_coat": "coat", "missing_base": "base"}.get(name))
            distance = max(abs(a - b) / tolerance(b) for a, b in zip(altered, output))
            minimum[name] = min(minimum[name], distance)
    if min(minimum.values()) <= 100:
        raise ValueError("oracle perturbation separation is below the frozen 100x minimum")
    return {"positive_finite_components": ACTIVE_COUNT * 4,
            "minimum_perturbation_tolerance_multiples": minimum}


# Sampling uses the same unchanged textures, directions and numerical budget as eval.
# The original texel-center pre-GPU float/FMA study consumed <0.014 of this budget.
# IOR cancellation is represented explicitly by two candidates instead of widening tolerance.
SAMPLE_CONTRACT = "tiled-brass-sample-synthetic-textures-v1"
SAMPLE_INPUT, SAMPLE_OUTPUT = struct.Struct("<5fI"), struct.Struct("<7fI")
SAMPLE_SENTINEL = b"\xa5" * SAMPLE_OUTPUT.size
SAMPLE_MODELS = ("float", "fma")


def sample_draws(seed):
    """Generate the selection draw and the lobe's three draws with exact uint32 arithmetic.

    The registered reflection graph ignores draw four, so its consumption is source-reviewed
    but cannot be verified from sample_buffer outputs.
    """
    values = []
    for _ in range(4):
        seed = (1664525 * seed + 1013904223) & 0xffffffff
        values.append((seed >> 8) / 16777216)
    return values

def sample_constants(model):
    """Derive the two prequalified source-rounding hypotheses for artistic IOR.

    Consider mx_artistic_ior(.99, 0): n is about 398, and the extinction numerator
    subtracts two almost equal values. Separate float operations clamp it to zero;
    contracting the last multiply/subtract leaves positive extinction. Both are legal
    source arithmetic. These hypotheses were frozen before GPU execution, and one must
    explain every record in a mode. They do not enumerate all legal compiler arithmetic.

    Each rounded operand has 24 significant bits. Their product has at most 48 bits,
    exactly representable in binary64; this particular near-equal subtraction is exact
    there too. A single final f32 rounding therefore models the contracted operation.
    """
    if model not in SAMPLE_MODELS:
        raise ValueError('unknown IOR hypothesis')
    r = f32(.99)
    sr = f32(math.sqrt(r))
    n = f32(f32(1 + sr) / f32(1 - sr))
    p, m = f32(n + 1), f32(n - 1)
    pp, mm = f32(p * p), f32(m * m)
    numerator = f32(pp * r - mm) if model == 'fma' else f32(f32(pp * r) - mm)
    k = f32(math.sqrt(max(0, f32(numerator / f32(1 - r)))))
    return n, k

def sample_conductor(n, k, c):
    """Independent complex-index Fresnel from scalar s/p reflectances."""
    c2, s2 = c*c, 1-c*c
    t = n*n-k*k-s2
    q = math.sqrt(t*t+4*n*n*k*k)
    a = math.sqrt((q+t)/2)
    rs = (q+c2-2*c*a)/(q+c2+2*c*a)
    rp = rs*(c2*q+s2*s2-2*c*a*s2)/(c2*q+s2*s2+2*c*a*s2)
    return (rs+rp)/2

def sample_probability(color, incoming, model):
    """Compute the coat selection probability from albedo-weighted Rec.709 luminance."""
    n, k = sample_constants(model)
    p = -.32775145+.18346033*n+.61146583*k-.07785134*n*k
    average = max(0, min(1, p/(1+p)))
    coat = fresnel(1.5, normalize(incoming)[2])
    luminance = sum(a*b for a,b in zip(linear_color(color), (.2126,.7152,.0722)))
    return coat/(coat+(1-coat)*luminance*average)

def sample_direction(incoming, roughness, random):
    """Invert the bounded visible-normal distribution in stretched isotropic coordinates."""
    i = normalize(incoming)
    a = roughness*roughness
    v = normalize((a*i[0], a*i[1], i[2]))
    s2 = (1+math.hypot(i[0], i[1]))**2
    b = v[2]*(1-a*a)*s2/(s2+a*a*i[2]*i[2])
    z = (1-random[2])*(1+b)-b
    phi = 2*math.pi*random[1]
    radial = math.sqrt(max(0, 1-z*z))
    m = normalize((a*(v[0]+radial*math.cos(phi)),
                   a*(v[1]+radial*math.sin(phi)), v[2]+z))
    dot = sum(x*y for x,y in zip(i,m))
    return tuple(2*dot*x-y for x,y in zip(m,i))

def sample_reference(row, model, mutation=None, input_profile="texel-centers"):
    """Compute direction, mixture PDF and the selected branch's estimator in double precision.

    Consider a colored conductor below the dielectric coat. MxMaterialInstance.sample first
    chooses a lobe using albedo-weighted luminance, then sample_for_refl draws a visible normal.
    Both lobes share its direction density, so mixture probabilities cancel in the PDF. The
    unchanged FIXED_NON_DELTA_MIXTURE_WEIGHT=0 path retains only the selected lobe's throughput,
    divided by that lobe's selection probability; full-material eval/pdf is a different value.

    The final row is a deliberate (1,0,0) incoming direction: the inner sampler rejects wi.z=0
    before MxMaterialInstance copies any fields, leaving its initialized output exactly zero.
    """
    incoming = row['incoming']
    if normalize(incoming)[2] <= 0:
        return (0.,)*7+(0,)
    color, roughness = texture_inputs(row, input_profile)
    random = sample_draws(row['seed'])
    if mutation == 'shift_draws':
        random = random[1:] + random[:1]
    prob = sample_probability(color, incoming, model)
    lobe = int(random[0] >= prob)
    if mutation == 'wrong_lobe':
        lobe = 1-lobe
    o = sample_direction(incoming, roughness, random)
    if o[2] <= 0:
        return (0.,)*7+(0,)
    i = normalize(incoming)
    h = normalize(tuple(x+y for x,y in zip(i,o)))
    c, a2 = i[2], roughness**4
    D = a2/(math.pi*(h[2]*h[2]*(a2-1)+1)**2)
    def smith_lambda(v):
        return (math.sqrt(1+a2*(1-v[2]*v[2])/(v[2]*v[2]))-1)/2
    G2 = 1/(1+smith_lambda(i)+smith_lambda(o))
    s2 = (1+math.hypot(i[0],i[1]))**2
    k = (1-a2)*s2/(s2+a2*c*c)
    pdf = D/(2*(k*c+math.sqrt(a2*(i[0]*i[0]+i[1]*i[1])+c*c)))
    energy = albedo(c, roughness)
    compensation = (1-energy)/max(energy, 1e-6)
    micro = sum(x*y for x,y in zip(i,h))
    n, extinction = sample_constants(model)
    f = fresnel(1.5, micro) if lobe == 0 else sample_conductor(n, extinction, micro)
    weights = (1,)*3 if lobe == 0 else tuple(x*(1-fresnel(1.5,c)) for x in linear_color(color))
    p = prob if lobe == 0 else 1-prob
    t = math.sqrt(a2*(i[0]*i[0]+i[1]*i[1])+c*c)
    # D cancels analytically against the bounded-VNDF PDF in the branch estimator.
    scale = G2*(k*c+t)/(2*c)*f*(1+f*compensation)/p
    weight = tuple(x*scale for x in weights)
    if mutation == 'collapsed':
        weight = tuple(x/pdf for x in reference(color,roughness,incoming,o)[:3])
    return o+(pdf,)+weight+(2,)

def sample_cases(input_profile="texel-centers"):
    """Select finite seeds using CPU equations alone, well away from either model's branch edge."""
    locations = input_locations(input_profile)
    rows = []
    for texel in range(4):
        for di, (incoming, _) in enumerate(DIRECTIONS):
            incoming = tuple(map(f32,incoming))
            color, roughness = texture_inputs(
                dict(texel=texel, uv=locations[texel]), input_profile)
            probabilities = [sample_probability(color,incoming,m) for m in SAMPLE_MODELS]
            for lobe in range(2):
                for seed in range(100000):
                    random = sample_draws(seed)
                    if min(abs(random[0]-p) for p in probabilities) <= .03:
                        continue
                    if any(int(random[0]>=p) != lobe for p in probabilities):
                        continue
                    if sample_direction(incoming,roughness,random)[2] <= .2:
                        continue
                    rows.append(dict(texel=texel,direction=di,lobe=lobe,
                        uv=locations[texel],incoming=incoming,seed=seed))
                    break
                else:
                    raise ValueError('no branch-stable seed')
    # First 24 exercise every pair/lobe. Then repeats and four exact wrapped-coordinate controls.
    rows += [dict(row) for row in rows]
    rows += [dict(row,uv=(row['uv'][0]+1,row['uv'][1]-1)) for row in rows[:4]]
    rows += [dict(row) for row in rows[:12]]
    rows.append(dict(texel=0,direction=3,lobe=0,uv=locations[0],incoming=(1.,0.,0.),seed=17))
    return rows

def sample_expected_outputs(inputs, input_profile="texel-centers"):
    """Build complete output tables for both coherent IOR hypotheses."""
    return {m:[sample_reference(row,m,input_profile=input_profile) for row in inputs]
            for m in SAMPLE_MODELS}

def sample_oracle_checks(inputs, expected, input_profile="texel-centers"):
    """Check finite geometry, branch margins and sensitivity to meaningful estimator mistakes."""
    if len(inputs)!=ACTIVE_COUNT or set(expected)!=set(SAMPLE_MODELS):
        raise ValueError('incomplete oracle')
    if sample_draws(0) != [(1013904223>>8)/16777216, (1196435762>>8)/16777216,
                     (3519870697>>8)/16777216, (2868466484>>8)/16777216]:
        raise ValueError('LCG known sequence failed')
    margins=[]
    separation={name:math.inf for name in ('wrong_lobe','shift_draws','collapsed')}
    for model in SAMPLE_MODELS:
        for index,row in enumerate(inputs):
            ref=expected[model][index]
            if index==64:
                if ref!=(0.,)*7+(0,): raise ValueError('early rejection must zero all fields')
                continue
            if not all(math.isfinite(x) for x in ref) or ref[2]<=.2 or ref[3]<=0 or min(ref[4:7])<=0 or ref[7]!=2:
                raise ValueError('invalid finite oracle')
            margin=abs(sample_draws(row['seed'])[0]-sample_probability(texture_inputs(row,input_profile)[0],row['incoming'],model))
            margins.append(margin)
            if margin<=.03: raise ValueError('branch margin too small')
            for name in separation:
                altered=sample_reference(row,model,mutation=name,input_profile=input_profile)
                distance=max(abs(a-b)/tolerance(b) for a,b in zip(altered[:7],ref[:7]))
                separation[name]=min(separation[name],distance)
    if min(separation.values())<=5:
        raise ValueError('perturbation separation too small: '+str(separation))
    return dict(minimum_selection_margin=min(margins),
                minimum_perturbation_tolerance_multiples=separation,constants={m:sample_constants(m) for m in SAMPLE_MODELS},
                source_arithmetic_scope='two prequalified IOR hypotheses with a finite residual budget; not all legal optimizations')

def sample_compare_outputs(data, expected):
    """Require one hypothesis for every record, exact flags and untouched guard/reject records."""
    if len(data)!=OUTPUT_COUNT*SAMPLE_OUTPUT.size:
        return dict(status='failed',reason='output byte count mismatch',bytes=len(data))
    if (set(expected)!=set(SAMPLE_MODELS) or
            any(len(rows)!=ACTIVE_COUNT or any(len(row)!=8 for row in rows) for rows in expected.values())):
        raise ValueError('incomplete expected candidates')
    candidates={}
    for model, rows in expected.items():
        failed=[]; worst=0.; detail=[]
        for index,want in enumerate(rows):
            got=SAMPLE_OUTPUT.unpack_from(data,index*SAMPLE_OUTPUT.size)
            for component,(a,b) in enumerate(zip(got,want)):
                ok=(a==b) if component==7 or index==64 else math.isfinite(a) and abs(a-b)<=tolerance(b)
                if not ok: failed.append([index,component])
                if component<7 and math.isfinite(a): worst=max(worst,abs(a-b)/tolerance(b))
            detail.append(dict(index=index,expected=want,actual=[v if math.isfinite(v) else str(v) for v in got]))
        candidates[model]=dict(status='passed' if not failed else 'failed',failed_components=failed,
                               max_tolerance_fraction=worst,rows=detail)
    matching=[m for m,v in candidates.items() if v['status']=='passed']
    tail=data[ACTIVE_COUNT*SAMPLE_OUTPUT.size:]==SAMPLE_SENTINEL*(OUTPUT_COUNT-ACTIVE_COUNT)
    zero_record=data[64*SAMPLE_OUTPUT.size:65*SAMPLE_OUTPUT.size]==bytes(SAMPLE_OUTPUT.size)
    def same(a, b):
        stride = SAMPLE_OUTPUT.size
        return data[a*stride:(a+1)*stride] == data[b*stride:(b+1)*stride]
    repeat=all(same(i,i+24) for i in range(24)) and all(same(i,i+52) for i in range(12))
    wrap=all(same(i,i+48) for i in range(4))
    return dict(status='passed' if matching and tail and repeat and wrap and zero_record else 'failed',
                matching_global_models=matching,candidates=candidates,compared_records=ACTIVE_COUNT,
                compared_components=ACTIVE_COUNT*8,tail_sentinels_unchanged=tail,
                identical_input_repeats=repeat,wrapped_uv_output_identical=wrap,early_rejection_bytes_zero=zero_record)



def validate_ptx(text, architecture, entry="eval_buffer"):
    """Reject incompatible fresh entry/global/stride contracts before loading their cubin."""
    layout = entry_layout(entry)
    requirements = {
        "target": rf"(?m)^\s*\.target\s+sm_{architecture}\b",
        "parameterless_entry": rf"\.entry\s+{entry}\s*\(\s*\)",
        "globals_168_align8": r"\.const\s+\.align\s+8\s+\.b8\s+SLANG_globalParams\[168\]",
        "input_stride": rf"\[SLANG_globalParams\+{layout['input_offset']}\];[^{{}}]{{0,200}}?mul\.(?:wide\.[su]32|lo\.s64)\s+[^;\n]+,\s*{layout['input_stride']}\s*;",
        "output_stride": rf"\[SLANG_globalParams\+{layout['output_offset']}\];[^{{}}]{{0,200}}?shl\.b64\s+[^;\n]+,\s*{layout['output_shift']}\s*;",
    }
    for name, offset in (("material", 80), ("input", layout["input_offset"]), ("output", layout["output_offset"]), ("count", 160)):
        requirements[name + "_offset"] = rf"ld\.const\.u(?:32|64)\s+[^;\n]*\[SLANG_globalParams\+{offset}\]"
    entry = re.search(rf"\.entry\s+{entry}\s*\(\s*\)\s*\{{", text)
    body = ""
    if entry:
        depth = 1
        for end in range(entry.end(), len(text)):
            depth += (text[end] == "{") - (text[end] == "}")
            if depth == 0:
                body = text[entry.start():end + 1]
                break
    missing = [name for name, pattern in requirements.items()
               if not re.search(pattern, text if name in ("target", "globals_168_align8") else body)]
    if missing:
        raise ValueError("fresh PTX ABI check failed: " + ", ".join(missing))
    return list(requirements)


def reviewed_abi_hashes(path, source_sha256, modes, entry="eval_buffer", input_profile="texel-centers"):
    """Require complete, exact PTX identities from a separately reviewed preparation run.

    The small automatic checks deliberately do not trace arbitrary PTX register dataflow. The
    preparation artifacts expose input field loads, both material handles, and output stores for
    explicit ABI review. The execution run then demands byte-identical fresh PTX in every mode.
    """
    report = json.loads(path.read_text())
    if (report.get("status") != "prepared" or report.get("contract") != entry_layout(entry,input_profile)["contract"] or
            report.get("input_profile", "texel-centers") != input_profile or
            report.get("source", {}).get("source_sha256") != source_sha256):
        raise ValueError("ABI reference must be a successful preparation of this exact source contract")
    cells = report.get("cells", [])
    wanted = {f"{backend}-o{optimization}" for backend, optimization in modes}
    if len(cells) != len(wanted) or {row.get("id") for row in cells} != wanted:
        raise ValueError("ABI reference modes are missing or duplicated")
    if any(row.get("status") != "prepared" or not re.fullmatch(r"[0-9a-f]{64}", row.get("ptx_sha256", ""))
           for row in cells):
        raise ValueError("ABI reference contains a failed or unidentified mode")
    return {row["id"]: row["ptx_sha256"] for row in cells}


def compare_outputs(data, expected):
    """Compare every component and preserve nonfinite, zero, missing and tail-write failures."""
    if len(data) != OUTPUT_COUNT * OUTPUT.size:
        return {"status": "failed", "reason": "output byte count mismatch", "bytes": len(data)}
    if len(expected) != ACTIVE_COUNT:
        raise ValueError("expected output count mismatch")
    rows, failures = [], []
    max_absolute = max_relative = max_fraction = 0
    for index, reference_values in enumerate(expected):
        actual = OUTPUT.unpack_from(data, index * OUTPUT.size)
        errors = []
        for component, (got, want) in enumerate(zip(actual, reference_values)):
            finite = math.isfinite(got)
            error = abs(got - want) if finite else None
            passed = finite and got > 0 and error <= tolerance(want)
            if not passed:
                failures.append([index, component])
            errors.append(error)
            if finite:
                max_absolute = max(max_absolute, error)
                max_relative = max(max_relative, error / abs(want))
                max_fraction = max(max_fraction, error / tolerance(want))
        rows.append({"index": index, "expected": reference_values,
                     "actual": [v if math.isfinite(v) else str(v) for v in actual],
                     "absolute_error": errors})
    tail_ok = data[ACTIVE_COUNT * OUTPUT.size:] == SENTINEL * (OUTPUT_COUNT - ACTIVE_COUNT)
    seed_repeat_ok = all(data[index * 16:(index + 1) * 16] == data[(index % 13) * 16:(index % 13 + 1) * 16]
                         for index in range(13, ACTIVE_COUNT))
    wrap_ok = data[:16] == data[12 * 16:13 * 16]
    return {"status": "passed" if not failures and tail_ok and seed_repeat_ok and wrap_ok else "failed",
            "compared_records": ACTIVE_COUNT, "compared_components": ACTIVE_COUNT * 4,
            "tail_records": OUTPUT_COUNT - ACTIVE_COUNT, "tail_sentinels_unchanged": tail_ok,
            "different_seed_outputs_identical": seed_repeat_ok, "wrapped_uv_output_identical": wrap_ok,
            "max_absolute_error": max_absolute, "max_relative_error": max_relative,
            "max_tolerance_fraction": max_fraction, "failed_components": failures, "rows": rows}


def validate_driver_log(text, entry="eval_buffer"):
    """Require one real launch, cleanup and both full-width texture handle checks."""
    handles = re.findall(r"texture=(color|roughness) handle_decimal=(\d+) handle_hex=(0x[0-9a-f]{16}) low30_fit=true nonzero=true", text)
    if len(handles) != 2 or {row[0] for row in handles} != {"color", "roughness"}:
        raise ValueError("missing or duplicate valid texture handles")
    for _, decimal, hexadecimal in handles:
        value = int(decimal)
        if value != int(hexadecimal, 16) or not value or value & ~0x3fffffff:
            raise ValueError("texture handle did not round-trip through low30")
    execution = ("execution launches=1 active=65 output_capacity=128 global_bytes=168 "
                 f"input_stride={entry_layout(entry)['input_stride']}")
    if text.splitlines().count(execution) != 1 or text.splitlines().count("cleanup=PASS") != 1:
        raise ValueError("missing real execution or successful cleanup")
    device = re.search(r"cuda_header_version=(\d+) driver_api_version=(\d+) device_ordinal=0 device=(.+) sm=(\d+)", text)
    if not device:
        raise ValueError("missing CUDA device identity")
    return {"launches": 1, "active_records": ACTIVE_COUNT, "output_capacity": OUTPUT_COUNT,
            "cuda_header_version": int(device[1]), "driver_api_version": int(device[2]),
            "device": device[3], "device_ordinal": 0, "compute_capability": int(device[4]),
            "texture_handles": {name: {"decimal": decimal, "hex": hexadecimal}
                                for name, decimal, hexadecimal in handles}}


def entry_layout(entry, input_profile="texel-centers"):
    """Keep each entry's descriptor and packing contract in one explicit table."""
    input_locations(input_profile)
    layouts = {
        "eval_buffer": dict(contract=CONTRACT, input_offset=96, output_offset=112,
                            input_stride=INPUT.size, output_shift=4, input_struct=INPUT,
                            cases=cases, expected=expected_outputs, check=oracle_checks,
                            compare=compare_outputs),
        "sample_buffer": dict(contract=SAMPLE_CONTRACT, input_offset=128, output_offset=144,
                              input_stride=SAMPLE_INPUT.size, output_shift=5, input_struct=SAMPLE_INPUT,
                              cases=sample_cases, expected=sample_expected_outputs, check=sample_oracle_checks,
                              compare=sample_compare_outputs),
    }
    if entry not in layouts:
        raise ValueError("unknown material entry: " + entry)
    layout = layouts[entry]
    if input_profile == "linear-filtering":
        layout["contract"] = layout["contract"].replace("synthetic-textures-v1", "linear-filtering-v1")
    return layout


def validate_oracle_hash(reference_path, oracle_sha256):
    """Require execution to use the exact inputs, candidates and budget reviewed in preparation."""
    prepared = json.loads(reference_path.read_text())
    if not re.fullmatch(r"[0-9a-f]{64}", oracle_sha256) or oracle_sha256 != prepared.get("oracle_sha256"):
        raise ValueError("oracle/inputs/tolerance differ from reviewed preparation")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangc", type=Path, required=True)
    parser.add_argument("--provider", type=Path, required=True)
    parser.add_argument("--cuda-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    phase = parser.add_mutually_exclusive_group(required=True)
    phase.add_argument("--prepare-only", action="store_true",
                       help="compile all modes without GPU execution, for explicit ABI review")
    phase.add_argument("--abi-reference", type=Path,
                       help="results.json from an explicitly reviewed --prepare-only run")
    parser.add_argument("--entry", choices=("eval_buffer", "sample_buffer"), default="eval_buffer")
    parser.add_argument("--input-profile", choices=INPUT_PROFILES, default="texel-centers")
    parser.add_argument("--cxx", default="c++")
    parser.add_argument("--timeout", type=int, default=180)
    args = parser.parse_args()
    if args.timeout <= 0 or args.timeout > 1800:
        parser.error("timeout must be between 1 and 1800 seconds")
    for name in ("slangc", "provider", "cuda_root", "output"):
        setattr(args, name, getattr(args, name).resolve())
    if args.output.exists():
        parser.error("output directory already exists; use a new directory to preserve every attempt")
    args.output.mkdir(parents=True)
    layout = entry_layout(args.entry, args.input_profile)
    report = {"schema": 1, "contract": layout["contract"], "entry": args.entry, "input_profile": args.input_profile, "status": "infrastructure-failed", "cells": [],
              "started_utc": datetime.now(timezone.utc).isoformat(), "platform": platform.platform(),
              "absolute_tolerance": ABS_TOL, "relative_tolerance": REL_TOL,
              "scope": f"unchanged {args.entry}, two synthetic textures, finite front-facing reflection",
              "limitations": ["no original assets", "no LUT read coverage",
                              "no arbitrary inputs or branch-boundary distribution proof",
                              "no arbitrary graph or GPU performance claim"]}
    def save():
        (args.output / "results.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    save()
    try:
        if platform.system() != "Linux" or "microsoft" in platform.release().lower() or sys.byteorder != "little":
            raise ValueError("this bounded driver harness requires native little-endian Linux")
        corpus = load_tool("material_corpus", "issue-nvvm-backend/run-complex-corpus.py")
        toolkit = corpus.load_toolkit_helpers()
        manifest_path = REPO / "issue-nvvm-backend/complex-corpus.manifest.json"
        manifest = corpus.read_workloads(manifest_path, toolkit)
        workload = next(row for row in manifest["workloads"] if row["name"] == "tiled-brass-material")
        if args.entry not in workload["entry_points"]:
            raise ValueError("registered entry is absent: " + args.entry)
        report.update(source=workload, architecture=manifest["architecture"], manifest=str(manifest_path))
        abi_hashes = None
        if args.abi_reference:
            args.abi_reference = args.abi_reference.resolve()
            abi_hashes = reviewed_abi_hashes(args.abi_reference, workload["source_sha256"], corpus.MODES, args.entry, args.input_profile)
            report["reviewed_abi_reference"] = {"path": str(args.abi_reference),
                                               "sha256": toolkit.sha256(args.abi_reference),
                                               "ptx_sha256": abi_hashes}
        libnvvm = toolkit.select_libnvvm(args.cuda_root)
        nvrtc = (args.cuda_root / "lib64/libnvrtc.so").resolve()
        compiler_library = args.slangc.parent.parent / "lib/libslang-compiler.so"
        driver_source = REPO / "extras/nvvm-material-runtime-driver.cpp"
        cxx = shutil.which(args.cxx)
        if not cxx:
            raise ValueError("C++ compiler is unavailable: " + args.cxx)
        ptxas = args.cuda_root / "bin/ptxas"
        nvcc = args.cuda_root / "bin/nvcc"
        artifacts = (args.slangc, compiler_library, args.provider, libnvvm, nvrtc, ptxas,
                     args.cuda_root / "nvvm/libdevice/libdevice.10.bc", args.cuda_root / "include/cuda.h",
                     args.cuda_root / "include/cudaTypedefs.h", driver_source, Path(__file__),
                     manifest_path, REPO / workload["source"], Path(cxx),
                     REPO / "issue-nvvm-backend/run-complex-corpus.py",
                     REPO / "extras/validate-nvvm-toolkit.py")
        report["artifact_sha256"] = {str(path): toolkit.sha256(toolkit.require_file(path)) for path in artifacts}
        environment = dict(os.environ, CUDA_PATH=str(args.cuda_root), CUDA_HOME=str(args.cuda_root),
                           LIBNVVM_HOME=str(args.cuda_root), SLANG_NVVM_BUILDER_PATH=str(args.provider))
        environment["LD_LIBRARY_PATH"] = os.pathsep.join(
            [str(compiler_library.parent), str(libnvvm.parent), str(nvrtc.parent),
             environment.get("LD_LIBRARY_PATH", "")])
        report["environment"] = {key: environment.get(key) for key in
                                 ("CUDA_PATH", "CUDA_HOME", "LIBNVVM_HOME", "SLANG_NVVM_BUILDER_PATH",
                                  "LD_LIBRARY_PATH", "CUDA_VISIBLE_DEVICES")}
        for name, command in (("compiler_version", [str(args.slangc), "-version"]),
                              ("toolkit_version", [str(nvcc), "--version"]),
                              ("host_compiler_version", [cxx, "--version"])):
            report[name] = subprocess.check_output(command, env=environment, text=True,
                                                   stderr=subprocess.STDOUT, timeout=args.timeout).strip()
        driver_version = Path("/proc/driver/nvidia/version")
        report["driver_version"] = driver_version.read_text() if driver_version.exists() else None
        helper = args.output / "material-driver"
        report["helper_build"] = toolkit.run(
            [cxx, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-O2",
             "-I" + str(args.cuda_root / "include"), str(driver_source), "-ldl", "-o", str(helper)],
            args.output / "helper-build.log", environment, args.timeout)
        save()
        if report["helper_build"]["return_code"] != 0:
            raise ValueError("material driver helper build failed")
        report["artifact_sha256"][str(helper)] = toolkit.sha256(helper)
        inputs = layout["cases"](args.input_profile)
        expected = layout["expected"](inputs, args.input_profile)
        report["oracle_checks"] = layout["check"](inputs, expected, args.input_profile)
        if args.input_profile == "linear-filtering":
            report["filtering_oracle_checks"] = filtering_oracle_checks()
        payloads = {
            "inputs.bin": b"".join(layout["input_struct"].pack(*row["uv"], *row["incoming"], *row.get("outgoing", ()), row["seed"]) for row in inputs),
            "color.bin": struct.pack("<16f", *(v for color in COLORS for v in color)),
            "roughness.bin": struct.pack("<16f", *(v for roughness in ROUGHNESS for v in (roughness, 0, 0, 1))),
        }
        # Persist the oracle and its tolerance before any shader launch or observed GPU output.
        oracle_path = args.output / "expected.json"
        oracle_path.write_text(json.dumps(dict(inputs=inputs, expected=expected, absolute_tolerance=ABS_TOL,
                                              relative_tolerance=REL_TOL), indent=2, allow_nan=False) + "\n")
        report["oracle_sha256"] = toolkit.sha256(oracle_path)
        if args.abi_reference:
            validate_oracle_hash(args.abi_reference, report["oracle_sha256"])
        for backend, optimization in corpus.MODES:
            report["cells"].append(dict(id=f"{backend}-o{optimization}", backend=backend,
                optimization=optimization, architecture=manifest["architecture"], entry=args.entry, status="pending"))
        report["status"] = "running"
        save()
        for cell in report["cells"]:
            directory = args.output / cell["id"]
            directory.mkdir()
            try:
                for name, data in payloads.items():
                    (directory / name).write_bytes(data)
                cell["input_sha256"] = {name: toolkit.sha256(directory / name) for name in payloads}
                ptx, cubin = directory / "shader.ptx", directory / "shader.cubin"
                cell["compile"] = toolkit.run(corpus.compile_command(cell, workload, args) + ["-o", str(ptx)],
                                               directory / "compile.log", environment, args.timeout)
                cell["status"] = "compile-failed"
                if cell["compile"]["return_code"] == 0:
                    cell["ptx_abi_checks"] = validate_ptx(toolkit.require_file(ptx).read_text(), cell["architecture"], args.entry)
                    cell["ptx_sha256"] = toolkit.sha256(ptx)
                    if abi_hashes is not None and cell["ptx_sha256"] != abi_hashes[cell["id"]]:
                        raise ValueError("fresh PTX differs from the reviewed ABI artifact")
                    cell["assembly"] = toolkit.run([str(ptxas), "-v", f"-arch=sm_{cell['architecture']}", str(ptx), "-o", str(cubin)],
                                                    directory / "assembly.log", environment, args.timeout)
                    cell["status"] = "assembly-failed"
                    if cell["assembly"]["return_code"] == 0:
                        cell["cubin_sha256"] = toolkit.sha256(toolkit.require_file(cubin))
                        if args.prepare_only:
                            cell["status"] = "prepared"
                        else:
                            cell["execution"] = toolkit.run([str(helper), str(cubin), str(directory), args.entry],
                                                             directory / "execution.log", environment, args.timeout)
                            cell["status"] = "execution-failed"
                            if cell["execution"]["return_code"] == 0:
                                cell["runtime"] = validate_driver_log((directory / "execution.log").read_text(), args.entry)
                                cell["comparison"] = layout["compare"]((directory / "outputs.bin").read_bytes(), expected)
                                cell["status"] = "passed" if cell["comparison"]["status"] == "passed" else "output-mismatch"
                cell["runtime_artifact_sha256"] = {name: toolkit.sha256(directory / name)
                    for name in ("outputs.bin", "material.bin", "globals.bin") if (directory / name).exists()}
            except (OSError, ValueError) as error:
                cell.update(status="infrastructure-failed", error=str(error))
            print(f"{cell['id']}: {cell['status']}", flush=True)
            save()
        # A mutable input or executable changing during a run invalidates the recorded provenance.
        if any(toolkit.sha256(Path(path)) != digest for path, digest in report["artifact_sha256"].items()):
            raise ValueError("an input or executable changed during validation")
        success = "prepared" if args.prepare_only else "passed"
        report["status"] = success if all(cell["status"] == success for cell in report["cells"]) else "failed"
        save()
        return 0 if report["status"] == success else 1
    except (OSError, ValueError, KeyError, TypeError, StopIteration, subprocess.SubprocessError) as error:
        report.update(status="infrastructure-failed", error=str(error))
        save()
        print(str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
