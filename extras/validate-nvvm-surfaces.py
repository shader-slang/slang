#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compare physical CUDA surface bytes against independent, generated host expectations.

This bounded harness uses 1D/2D Float32, Half and native integer32 storage with
1/2/4 channels, including independently initialized mixed-format resources and
explicit signed/unsigned8/16 formats with logical32 values and saturating stores,
plus native narrow integer values with bit-preserving stores.
It records failures as failures, including NVRTC component compilation and formatted
store rounding differences. NaN conversions require class only; untouched bits are exact.
Run --self-test for CPU oracle/ABI/reflection contracts without a compiler or GPU.
"""
import argparse
from collections import Counter
import ctypes as C
import copy
import hashlib
import json
import math
from fractions import Fraction
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import time

REPO = Path(__file__).resolve().parents[1]
FIXTURES = ["matrix", "half-load", "half-nan", "boundaries", "integers", "mixed", "layered",
            "half-layered", "half-volume", "integer-formats", "integer-spatial", "native-narrow", "normalized"]
INTEGER_VALUES = [0, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0x01020304, 0x89ABCDEF,
                  0xFFFFFF80, 0x00000080, 0xFFFF8000, 0x00008000, 0x55555555, 0xAAAAAAAA]
FORMATS = {"half": (0x10, 2), "float32": (0x20, 4), "int32": (0x0A, 4), "uint32": (0x03, 4),
           "int8": (0x08, 1), "uint8": (0x01, 1), "int16": (0x09, 2), "uint16": (0x02, 2)}
VALUES = [
    0x00000000, 0x80000000, 0x3F800000, 0xBF800000,
    0x3F800FFF, 0x3F801000, 0x3F801001, 0x3F803000,
    0x33000000, 0x33000001, 0xB3000000, 0x33800000,
    0x387FC000, 0x387FDFFF, 0x387FE000, 0x38800000,
    0x477FE000, 0x477FEFFF, 0x477FF000, 0xC77FF000,
    0x7F800000, 0xFF800000, 0x3EAAAAAB, 0xBEAAAAAB,
]
INITIAL_HALF = [0x3555, 0x7D55, 0xB955, 0xFE01, 0x8000, 1, 0x7E13, 0xBC00]
INITIAL_FLOAT = [0x3EAAAAAB, 0x7FA12345, 0xBEAAAAAB, 0xFFC54321,
                 0x80000000, 1, 0x7FC12345, 0xBF800000]
LOAD_HALF = [0, 0x8000, 1, 0x3FF, 0x400, 0x3555, 0x3C00, 0xBC00,
             0x7BFF, 0xFBFF, 0x7C00, 0xFC00]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def inventory_paths(args, provider):
    """Inventory the selected installation; this is not a trace of dynamically loaded libraries."""
    paths = {args.slangc, provider / "libslang-llvm-nvvm.so", Path(__file__).resolve(),
             Path(sys.executable).resolve(), args.cuda_root / "nvvm/lib64/libnvvm.so",
             args.cuda_root / "lib64/libnvrtc.so", args.cuda_root / "nvvm/libdevice/libdevice.10.bc",
             args.cuda_root / "bin/ptxas", args.cuda_root / "include/cuda.h",
             args.cuda_root / "version.json"}
    for directory in [args.slangc.parent, args.slangc.parent.parent / "lib"]:
        paths.update(x for x in directory.glob("*.so*") if not x.name.endswith(".dwarf"))
        paths.update(directory.glob("*.bin"))
        paths.update(directory.rglob("*.slang-module"))
    paths.update(args.cuda_root.joinpath("lib64").glob("libnvrtc*.so*"))
    paths.update(REPO / "tests/cuda" / ("nvvm-surface-physical-" + name + ".slang")
                 for name in FIXTURES)
    if args.provenance:
        paths.add(args.provenance.resolve())
    return paths


def freeze_identity(paths):
    """Bind bytes and resolved paths, including a symlink's selected library target."""
    return {str(path): dict(sha256=sha(path.read_bytes()), resolved_path=str(path.resolve()))
            for path in sorted(paths)}


def verify_identity(identity, paths=None):
    """Reject deleted, changed, redirected or newly inventoried inputs."""
    selected = set(identity) if paths is None else {str(path) for path in paths}
    require(selected <= set(identity), "New input appeared after the initial inventory")
    changed = [path for path in sorted(selected) if not Path(path).is_file() or
               str(Path(path).resolve()) != identity[path]["resolved_path"] or
               sha(Path(path).read_bytes()) != identity[path]["sha256"]]
    require(not changed, "Inputs changed during execution: " + str(changed))


def narrow_half(bits):
    """Round binary32 to binary16, using integer RN-even including gradual underflow."""
    sign, exponent, fraction = (bits >> 16) & 0x8000, (bits >> 23) & 255, bits & 0x7FFFFF
    if exponent == 255:
        require(fraction == 0, "Written NaNs use classification, not an exact payload oracle")
        return sign | 0x7C00
    if exponent == 0:
        return sign
    exponent -= 127
    if exponent > 15:
        return sign | 0x7C00
    significand = 0x800000 | fraction
    shift = 13 if exponent >= -14 else -exponent - 1
    quotient, remainder = divmod(significand, 1 << shift)
    midpoint = 1 << (shift - 1)
    rounded = quotient + int(remainder > midpoint or (remainder == midpoint and quotient & 1))
    return sign | (rounded if exponent < -14 else ((exponent + 14) << 10) + rounded)


def widen_half(bits):
    """Decode binary16 exactly; canonical NaN bits are a reference, never a payload obligation."""
    sign, exponent, fraction = (bits & 0x8000) << 16, (bits >> 10) & 31, bits & 1023
    if exponent == 31:
        return 0x7FC00000 if fraction else sign | 0x7F800000
    if exponent == 0:
        if not fraction:
            return sign
        exponent = -14
        while fraction < 1024:
            fraction <<= 1
            exponent -= 1
        return sign | ((exponent + 127) << 23) | ((fraction & 1023) << 13)
    return sign | ((exponent + 112) << 23) | (fraction << 13)


def cases():
    """Enumerate the bounded matrix; selectors never silently drop requested cells."""
    rows = []
    def add(name, fixture, entry, width, height, lanes, half, shape=2, defines=None):
        rows.append(dict(case=name, fixture=fixture, entry=entry, width=width, height=height,
                         lanes=lanes, half=half, shape=shape, defines=defines or {}))
    for shape in (1, 2):
        for lanes in (1, 2, 4):
            for half in (False, True):
                for entry in ("wholeStore", "componentStore", "loadValues"):
                    add(f'{"half" if half else "float32"}-{shape}d-{lanes}-{entry}',
                        "matrix", entry, 32, 1 if shape == 1 else 3, lanes, half, shape,
                        dict(SURFACE_DIM=shape, SURFACE_LANES=lanes, SURFACE_HALF=int(half)))
    add("half-all-bits-load", "half-load", "loadValues", 256, 257, 1, True)
    add("half-written-nans", "half-nan", "wholeStore", 16, 3, 1, True)
    for half in (False, True):
        add(f'{"half" if half else "float32"}-dynamic-component', "boundaries",
            "componentStore", 16, 3, 4, half, defines=dict(SURFACE_HALF=int(half), PROBE_OOB=0))
    for entry in ("loadValues", "wholeStore"):
        add("half-oob-" + entry, "boundaries", entry, 8, 3, 1, True, defines=dict(PROBE_OOB=1))
    add("half-1d-1-literal-store", "matrix", "literalStore", 32, 1, 1, True, 1,
        dict(SURFACE_DIM=1, SURFACE_LANES=1, SURFACE_HALF=1, SURFACE_LITERAL_STORE=1))
    for shape in (1, 2):
        for lanes in (1, 2, 4):
            for scalar in ("int32", "uint32"):
                for entry in ("wholeStore", "componentStore", "loadValues"):
                    add(f"{scalar}-{shape}d-{lanes}-{entry}", "integers", entry,
                        32, 1 if shape == 1 else 3, lanes, False, shape,
                        dict(SURFACE_DIM=shape, SURFACE_LANES=lanes,
                             SURFACE_SIGNED=int(scalar == "int32")))
                    rows[-1]["scalar"] = scalar
        for entry in ("wholeCopies", "componentCopies"):
            add(f"mixed-{shape}d-{entry}", "mixed", entry, 32,
                1 if shape == 1 else 3, 4, False, shape, dict(SURFACE_DIM=shape))
    # Keep old row dictionaries unchanged. Layers are a separate coordinate, never height.
    add("native32-1d-array", "layered", "exercise", 11, 1, 1, False, 1,
        dict(SURFACE_DIM=1))
    rows[-1]["array_layers"] = 3
    add("uint32-2d-array-order", "layered", "exercise", 11, 7, 1, False, 2,
        dict(SURFACE_DIM=2))
    rows[-1]["array_layers"] = 7
    for shape in (1, 2):
        for label, entry in (("whole", "wholeCopies"), ("components", "componentCopies")):
            add(f"half-{shape}d-array-{label}", "half-layered", entry, 11,
                1 if shape == 1 else 5, 4, True, shape, dict(SURFACE_DIM=shape))
            rows[-1]["array_layers"] = 3
    for label, entry in (("whole", "wholeCopies"), ("components", "componentCopies")):
        add(f"half-3d-{label}", "half-volume", entry, 11, 5, 4, True, 3)
        rows[-1]["volume_depth"] = 3
    for shape in (1, 2):
        for operation, label in enumerate(("whole", "static-components", "dynamic-components")):
            add(f"integer-format-{shape}d-{label}", "integer-formats", "exercise", 67,
                1 if shape == 1 else 5, 4, False, shape,
                dict(SURFACE_DIM=shape, SURFACE_OPERATION=operation))
    for shape, geometry in ((1, "1d-array"), (2, "2d-array"), (3, "3d")):
        for operation, label in enumerate(("whole", "static-components", "dynamic-components")):
            add(f"integer-format-{geometry}-{label}", "integer-spatial", "exercise", 67,
                1 if shape == 1 else 5, 4, False, shape,
                dict(SURFACE_DIM=shape, SURFACE_OPERATION=operation))
            rows[-1]["volume_depth" if shape == 3 else "array_layers"] = 4
    for shape, is_array, geometry in ((1, False, "1d"), (2, False, "2d"),
                                      (1, True, "1d-array"), (2, True, "2d-array"),
                                      (3, False, "3d")):
        for operation, label in enumerate(("whole", "static-components", "dynamic-components")):
            add(f"native-narrow-{geometry}-{label}", "native-narrow", "exercise", 67,
                1 if shape == 1 else 5, 4, False, shape,
                dict(SURFACE_DIM=shape, SURFACE_ARRAY=int(is_array), SURFACE_OPERATION=operation))
            if is_array or shape == 3:
                rows[-1]["array_layers" if is_array else "volume_depth"] = 4
    for shape, is_array, geometry in ((1, False, "1d"), (2, False, "2d"),
                                      (1, True, "1d-array"), (2, True, "2d-array"),
                                      (3, False, "3d")):
        for operation, label in enumerate(("whole", "static-components", "dynamic-components")):
            for half in (False, True):
                add(f"normalized-{geometry}-{label}-{'half' if half else 'float'}",
                    "normalized", "exercise", 259, 1 if shape == 1 else 5, 4, False, shape,
                    dict(SURFACE_DIM=shape, SURFACE_ARRAY=int(is_array),
                         SURFACE_OPERATION=operation, SURFACE_LOGICAL_HALF=int(half)))
                if is_array or shape == 3:
                    rows[-1]["array_layers" if is_array else "volume_depth"] = 4
    return rows


def resource_specs(row):
    """Describe each resource's physical channel format and reflected logical type in ABI order."""
    def spec(name, storage, lanes, scalar="float32"):
        # Integer RW textures carry an inferred format even without a source annotation.
        # Reflection exposes checkVarDeclCommon's inferredFormatAttribute, so check it exactly.
        suffix = {"half": "16f", "int32": "32i", "uint32": "32ui", "int8": "8i",
                  "uint8": "8ui", "int16": "16i", "uint16": "16ui"}.get(storage)
        reflected_format = {1: "r", 2: "rg", 4: "rgba"}[lanes] + suffix if suffix else None
        return dict(name=name, storage=storage, lanes=lanes, scalar=scalar,
                    format=reflected_format)
    if row["fixture"] == "mixed":
        return [spec(prefix + suffix, storage, lanes)
                for prefix in ("source", "result")
                for suffix, storage, lanes in (("Native", "float32", 4), ("R", "half", 1),
                                               ("RG", "half", 2), ("RGBA", "half", 4))]
    if row["fixture"] == "layered":
        families = [(scalar, lanes) for scalar in ("float32", "int32", "uint32")
                    for lanes in (1, 2, 4)] if row["shape"] == 1 else [("uint32", 1)]
        return [spec(prefix + str(index), scalar, lanes, scalar)
                for index, (scalar, lanes) in enumerate(families)
                for prefix in ("source", "observed")]
    if row["fixture"] in ("half-layered", "half-volume"):
        return [spec(prefix + family + suffix, "half", lanes, scalar)
                for family, scalar in (("Half", "float16"), ("Format", "float32"))
                for suffix, lanes in (("R", 1), ("RG", 2), ("RGBA", 4))
                for prefix in ("source", "result")] + [
                    spec("sourceFloat", "float32", 4), spec("resultFloat", "float32", 4)]
    if row["fixture"] in ("integer-formats", "integer-spatial", "native-narrow"):
        return [spec(prefix + family + suffix, storage if prefix == "surface" else scalar,
                     lanes, storage if row["fixture"] == "native-narrow" and prefix == "surface" else scalar)
                for family, storage, scalar in (("Signed8", "int8", "int32"),
                                                ("Unsigned8", "uint8", "uint32"),
                                                ("Signed16", "int16", "int32"),
                                                ("Unsigned16", "uint16", "uint32"))
                for suffix, lanes in (("R", 1), ("RG", 2), ("RGBA", 4))
                for prefix in ("surface", "observed")]
    if row["fixture"] == "normalized":
        result = []
        logical = "float16" if row["defines"]["SURFACE_LOGICAL_HALF"] else "float32"
        for family, storage, suffix in (("Signed8", "int8", "8_snorm"),
                                         ("Unsigned8", "uint8", "8"),
                                         ("Signed16", "int16", "16_snorm"),
                                         ("Unsigned16", "uint16", "16")):
            for name, lanes in (("R", 1), ("RG", 2), ("RGBA", 4)):
                surface = spec("surface" + family + name, storage, lanes, logical)
                surface["format"] = {1: "r", 2: "rg", 4: "rgba"}[lanes] + suffix
                result += [surface, spec("observed" + family + name, "float32", lanes)]
        return result
    scalar = row.get("scalar", "float32")
    return [spec("surface", "half" if row["half"] else scalar, row["lanes"], scalar),
            spec("observed", scalar, row["lanes"], scalar)]


def resource_buffers(row, buffers):
    """Adapt the original two-array contract without changing its persisted oracle identity."""
    if "resources" in buffers:
        return buffers["resources"]
    return [dict(initial=buffers[a], expected=buffers[b],
                 nan_positions=buffers["nan_positions"][i],
                 active_texels=buffers["active_texels"], guard_texels=buffers["guard_texels"])
            for i, (a, b) in enumerate((("initial", "expected"),
                                        ("output_initial", "output_expected")))]


def oracle_files(row, buffers):
    """Keep original file keys stable; name added multi-resource inputs by their shader binding."""
    if "resources" not in buffers:
        return {key: buffers[key] for key in
                ("initial", "expected", "output_initial", "output_expected")}
    return {spec["name"] + "-" + key: data[key]
            for spec, data in zip(resource_specs(row), buffers["resources"])
            for key in ("initial", "expected")}


def expanded_oracle(row):
    """Compute new integer and mixed-copy expectations from host inputs, never shader readback."""
    specs = resource_specs(row)
    values, expected, exceptions = [], [], []
    for index, spec in enumerate(specs):
        n = row["width"] * row["height"] * spec["lanes"]
        if row["fixture"] == "integers":
            pattern = INTEGER_VALUES if index == 0 else [0x13579BDF]
        elif index == 0:
            pattern = VALUES
        elif index < 4:
            pattern = LOAD_HALF
        else:
            pattern = INITIAL_HALF if spec["storage"] == "half" else INITIAL_FLOAT
        # Distinct source phase offsets expose resource substitutions and lane-order mistakes.
        phase = index * 3 if row["fixture"] == "mixed" else 0
        values.append([pattern[(i + phase) % len(pattern)] for i in range(n)])
        expected.append(list(values[-1]))
        exceptions.append(set())
    active = 0
    for y in range(row["height"]):
        for x in range(row["width"]):
            if x >= 24 or y >= 2:
                continue
            active += 1
            texel = y * row["width"] + x
            if row["fixture"] == "integers":
                for lane in range(row["lanes"]):
                    i = texel * row["lanes"] + lane
                    expected[1][i] = values[0][i] if row["entry"] == "loadValues" else 0x12345678
                    if row["entry"] == "wholeStore" or (row["entry"] == "componentStore" and
                            lane in ((0, 2) if row["lanes"] == 4 else (0,))):
                        expected[0][i] = INTEGER_VALUES[(x + y * 24 + 5 * lane) % len(INTEGER_VALUES)]
            else:
                partial = row["entry"] == "componentCopies"
                # Half sources feed distinct native result lanes; destinations are never sources.
                for lane, (source, source_lane) in enumerate(((1, 0), (2, 1), (3, 2), (3, 3))):
                    if not partial or lane in (1, 3):
                        i = texel * specs[source]["lanes"] + source_lane
                        expected[4][texel * 4 + lane] = widen_half(values[source][i])
                for target in (5, 6, 7):
                    lanes = specs[target]["lanes"]
                    for lane in range(lanes):
                        if not partial or lane in ((1, 3) if lanes == 4 else (lanes - 1,)):
                            expected[target][texel * lanes + lane] = narrow_half(values[0][texel * 4 + lane])
    resources = []
    for i, spec in enumerate(specs):
        size = FORMATS[spec["storage"]][1]
        def pack(data):
            return b"".join(v.to_bytes(size, "little") for v in data)
        resources.append(dict(initial=pack(values[i]), expected=pack(expected[i]),
                              nan_positions=exceptions[i], active_texels=active,
                              guard_texels=row["width"] * row["height"] - active))
    return dict(resources=resources)


def layered_oracle(row):
    """Keep layer/address/resource identity independent of both shader reads and writes."""
    specs = resource_specs(row)
    width, height, layers = row["width"], row["height"], row["array_layers"]
    initial, expected = [], []
    for resource, spec in enumerate(specs):
        base = 0x3F000000 if spec["scalar"] == "float32" else 0x80000000
        words = [base | (resource << 16) | (layer << 12) | (y << 8) | (x << 2) | lane
                 for layer in range(layers) for y in range(height) for x in range(width)
                 for lane in range(spec["lanes"])]
        initial.append(words)
        expected.append(list(words))
    active = 0
    for layer in range(layers):
        for y in range(height):
            for x in range(width):
                live = (1 <= x < 9) if row["shape"] == 1 else (x, y, layer) == (1, 4, 2)
                if not live:
                    continue
                active += 1
                for resource in range(0, len(specs), 2):
                    spec = specs[resource]
                    base = 0x40000000 if spec["scalar"] == "float32" else 0xA0000000
                    for lane in range(spec["lanes"]):
                        index = ((layer * height + y) * width + x) * spec["lanes"] + lane
                        expected[resource + 1][index] = initial[resource][index]
                        expected[resource][index] = (base | (resource << 16) | (layer << 12) |
                                                     (y << 8) | (x << 2) | lane)
    return dict(resources=[dict(initial=struct.pack("<" + "I" * len(a), *a),
                                expected=struct.pack("<" + "I" * len(b), *b),
                                nan_positions=set(), active_texels=active,
                                guard_texels=width * height * layers - active)
                           for a, b in zip(initial, expected)])


def half_surface_oracle(row):
    """Reuse Half conversion rules for independent array-layer or spatial-Z marker stores."""
    specs = resource_specs(row)
    width, height = row["width"], row["height"]
    layers = row["volume_depth"] if "volume_depth" in row else row["array_layers"]
    initial, expected = [], []
    exceptions = [set() for _ in specs]
    for resource, spec in enumerate(specs):
        if resource == 12:
            pattern = VALUES + [INITIAL_FLOAT[1], INITIAL_FLOAT[3]]
        elif resource == 13:
            pattern = INITIAL_FLOAT
        elif resource % 2:
            pattern = INITIAL_HALF
        else:
            pattern = LOAD_HALF + [INITIAL_HALF[1], INITIAL_HALF[3]]
        # Explicit layer/resource phase plus spatial terms avoid repeating at a layer boundary.
        words = [pattern[(resource * 3 + layer * 5 + y * 7 + x + lane * 3) % len(pattern)]
                 for layer in range(layers) for y in range(height) for x in range(width)
                 for lane in range(spec["lanes"])]
        initial.append(words)
        expected.append(list(words))
    partial = row["entry"] == "componentCopies"
    active = 0
    for layer in range(layers):
        for y in range(height):
            for x in range(width):
                if not (1 <= x < 9 and layer < 2 and (row["shape"] == 1 or 1 <= y < 4)):
                    continue
                active += 1
                texel = (layer * height + y) * width + x
                for source in range(0, 12, 2):
                    lanes = specs[source]["lanes"]
                    for lane in range(lanes):
                        if partial and lane not in ((1, 3) if lanes == 4 else (lanes - 1,)):
                            continue
                        index = texel * lanes + lane
                        if source < 6:
                            expected[source + 1][index] = initial[source][index]
                        else:
                            value = initial[12][texel * 4 + lane]
                            if value & 0x7FFFFFFF > 0x7F800000:
                                expected[source + 1][index] = 0x7E00
                                exceptions[source + 1].add(index)
                            else:
                                expected[source + 1][index] = narrow_half(value)
                        # This store does not derive from a shader load, so wrong address pairs
                        # cannot cancel. The marker is a distinct finite, normal Half bit pattern.
                        code = (((source // 2 * 3 + layer) * 5 + y) * 11 + x) * 4 + lane
                        require(code < 4096, "Half marker coordinate exceeds its bounded encoding")
                        expected[source][index] = 0x3000 | code
                for lane, (source, source_lane) in enumerate(((6, 0), (8, 1), (10, 2), (10, 3))):
                    if not partial or lane in (1, 3):
                        index = texel * specs[source]["lanes"] + source_lane
                        expected[13][texel * 4 + lane] = widen_half(initial[source][index])
                        if initial[source][index] & 0x7FFF > 0x7C00:
                            exceptions[13].add(texel * 4 + lane)
    resources = []
    for spec, before, after, nan_positions in zip(specs, initial, expected, exceptions):
        size = FORMATS[spec["storage"]][1]
        resources.append(dict(initial=b"".join(v.to_bytes(size, "little") for v in before),
                              expected=b"".join(v.to_bytes(size, "little") for v in after),
                              nan_positions=nan_positions, active_texels=active,
                              guard_texels=width * height * layers - active))
    return dict(resources=resources)


def integer_store_edges(bits, signed):
    """Include both sides of each physical bound and the logical32 extremes."""
    if signed:
        low, high = -(1 << (bits - 1)), (1 << (bits - 1)) - 1
        return [-(1 << 31), low - 1, low, low + 1, -2, -1, 0, 1,
                2, high - 1, high, high + 1, 255, 256, 65536, (1 << 31) - 1]
    high = (1 << bits) - 1
    return [0, 1, 2, 127, 128, 255, 256, 32767, 32768, 65535, 65536,
            high - 1, high, high + 1, 0x80000000, 0xFFFFFFFF]


def narrow_integer(value, bits, signed):
    """Clamp a logical32 mathematical integer before encoding the physical channel."""
    low = -(1 << (bits - 1)) if signed else 0
    high = (1 << (bits - int(signed))) - 1
    return min(max(value, low), high) & ((1 << bits) - 1)


def widen_integer(value, bits, signed):
    """Extend the original physical bits independently of the shader's stores."""
    if signed and value & (1 << (bits - 1)):
        value -= 1 << bits
    return value & 0xFFFFFFFF


def integer_format_oracle(row):
    """Observe original narrow inputs; native stores preserve bits, formatted32 stores saturate."""
    width, height = row["width"], row["height"]
    spatial = "volume_depth" in row or "array_layers" in row
    native = row["fixture"] == "native-narrow"
    depth = row.get("volume_depth", row.get("array_layers", 1))
    operation = row["defines"]["SURFACE_OPERATION"]
    resources = []
    for resource, spec in enumerate(resource_specs(row)[::2]):
        lanes, bits = spec["lanes"], FORMATS[spec["storage"]][1] * 8
        signed = spec["storage"].startswith("int")
        mask, midpoint = (1 << bits) - 1, 1 << (bits - 1)
        pattern = [0, 1, midpoint - 1, midpoint, midpoint + 1, mask, mask - 1, 2,
                   0x55, 0xAA, mask // 3, (mask // 3) * 2, 3, mask - 2, 7, mask - 7]
        initial, expected, observed, output = [], [], [], []
        active = 0
        edges = pattern if native else integer_store_edges(bits, signed)
        for z in range(depth):
            for y in range(height):
                for x in range(width):
                    live = (1 <= x <= 64 and (row["shape"] == 1 or 1 <= y <= 3) and
                            (not spatial or 1 <= z <= 2))
                    active += int(live)
                    for lane in range(lanes):
                        raw = (pattern[(x + y * 7 + resource * 3 + lane * 5) % 16] if live else
                               ((0x55 + resource * 13 + x * 3 + y * 7 + z * 61 + lane * 17) & mask) or 1)
                        if live and spatial:
                            # Alternate fixed extension anchors and coordinate markers. These
                            # are bounded patterns, not a uniqueness claim for 8-bit channels.
                            raw = ((resource * 29 + z * 61 + y * 17 + x * 3 + lane * 7) & mask
                                   if (x - 1) % 2 else
                                   pattern[((x - 1) // 2 + resource * 3 + lane * 5) % 16])
                        sentinel = (0x13579BDF ^ (resource << 16) ^ (z << 20) ^
                                    (y << 12) ^ (x << 4) ^ lane)
                        initial.append(raw)
                        observed.append(sentinel)
                        output.append(widen_integer(raw, bits, signed) if live else sentinel)
                        selected = (operation == 0 or lanes == 1 or
                                    (operation == 1 and lane in ((1, 3) if lanes == 4 else (1,))) or
                                    (operation == 2 and lane == (x - 1) % lanes))
                        edge = (x - 1) // 4 if operation == 2 else x - 1
                        value = edges[(edge + z * 5 + y * 7 + resource * 3 + lane * 5) % 16]
                        stored = value if native else narrow_integer(value, bits, signed)
                        expected.append(stored if live and selected else raw)
        for before, after, size in ((initial, expected, bits // 8), (observed, output, 4)):
            resources.append(dict(initial=b"".join(v.to_bytes(size, "little") for v in before),
                                  expected=b"".join(v.to_bytes(size, "little") for v in after),
                                  nan_positions=set(), active_texels=active,
                                  guard_texels=width * height * depth - active))
    return dict(resources=resources)


def float32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def normalized_inputs():
    """Input bit patterns include both neighbors of several format-dependent thresholds."""
    bits = [0, 0x80000000, 0x3F800000, 0xBF800000, 0x40000000, 0xC0000000,
            0x7F800000, 0xFF800000, 0x7FC12345, 0xFFC12345, 1, 0x80000001,
            0x3F000000, 0xBF000000, 0x3EAAAAAB, 0xBEAAAAAB]
    for scale in (127, 255, 32767, 65535):
        for integer in (0, scale // 2):
            middle = struct.unpack("<I", struct.pack("<f", (integer + 0.5) / scale))[0]
            bits += [middle - 1, middle, middle + 1]
            bits += [x | 0x80000000 for x in (middle - 1, middle, middle + 1)]
    require(len(bits) == 64, "Normalized input index contract changed")
    return bits


def encode_normalized(value, bits, signed):
    """Quantize a Float32-scaled value with an exact rational nearest/ties-away oracle."""
    if math.isnan(value):
        return 0
    maximum = (1 << (bits - int(signed))) - 1
    scaled = float32(min(max(value, -1 if signed else 0), 1) * maximum)
    exact = Fraction.from_float(abs(scaled))
    quotient, remainder = divmod(exact.numerator, exact.denominator)
    result = quotient + int(2 * remainder >= exact.denominator)
    return (-result if scaled < 0 else result) & ((1 << bits) - 1)


def normalized_oracle(row):
    """Decode original raw channels independently; stores never depend on those observations."""
    width, height = row["width"], row["height"]
    depth = row.get("volume_depth", row.get("array_layers", 1))
    spatial = "volume_depth" in row or "array_layers" in row
    operation = row["defines"]["SURFACE_OPERATION"]
    half = row["defines"]["SURFACE_LOGICAL_HALF"]
    inputs = normalized_inputs()
    resources = []
    for resource, spec in enumerate(resource_specs(row)[::2]):
        lanes, bits = spec["lanes"], FORMATS[spec["storage"]][1] * 8
        signed = spec["storage"].startswith("int")
        mask, midpoint = (1 << bits) - 1, 1 << (bits - 1)
        maximum = (1 << (bits - int(signed))) - 1
        pattern = [0, 1, midpoint - 1, midpoint, midpoint + 1, mask, mask - 1, 2]
        initial, expected, observed, output = [], [], [], []
        active = 0
        for z in range(depth):
            for y in range(height):
                for x in range(width):
                    live = (1 <= x <= 256 and (row["shape"] == 1 or 1 <= y <= 3) and
                            (not spatial or 1 <= z <= 2))
                    active += int(live)
                    for lane in range(lanes):
                        raw = (pattern[(x + y * 7 + resource * 3 + lane * 5) % 8]
                               if x % 2 else (resource * 29 + z * 61 + y * 17 + x * 3 + lane * 7) & mask)
                        # Every signed format explicitly includes its duplicate -1 encoding in
                        # every lane, including lanes left untouched by partial stores.
                        if signed and x == 1:
                            raw = midpoint
                        sentinel = 0x3F000000 | (resource << 16) | (z << 12) | (y << 9) | x
                        decoded = raw - (1 << bits) if signed and raw >= midpoint else raw
                        decoded = max(float32(decoded / maximum), -1 if signed else 0)
                        if half:
                            decoded = struct.unpack("<e", struct.pack("<e", decoded))[0]
                        decoded_bits = struct.unpack("<I", struct.pack("<f", decoded))[0]
                        initial.append(raw)
                        observed.append(sentinel)
                        output.append(decoded_bits if live else sentinel)
                        selected = (operation == 0 or lanes == 1 or
                                    (operation == 1 and lane in ((1, 3) if lanes == 4 else (1,))) or
                                    (operation == 2 and lane == (x - 1) % lanes))
                        edge = (x - 1) // 4 if operation == 2 else x - 1
                        value = struct.unpack("<f", struct.pack("<I",
                            inputs[(edge + z * 5 + y * 7 + resource * 3 + lane * 5) % 64]))[0]
                        if half:
                            value = struct.unpack("<e", struct.pack("<e", value))[0]
                        stored = encode_normalized(value, bits, signed)
                        expected.append(stored if live and selected else raw)
        for before, after, size in ((initial, expected, bits // 8), (observed, output, 4)):
            resources.append(dict(initial=b"".join(v.to_bytes(size, "little") for v in before),
                                  expected=b"".join(v.to_bytes(size, "little") for v in after),
                                  nan_positions=set(), active_texels=active,
                                  guard_texels=width * height * depth - active))
    return dict(resources=resources)


def oracle(row):
    """Generate physical inputs, expectations and the exact converted-NaN exception positions."""
    if row["fixture"] == "normalized":
        return normalized_oracle(row)
    if row["fixture"] == "layered":
        return layered_oracle(row)
    if row["fixture"] in ("integer-formats", "integer-spatial", "native-narrow"):
        return integer_format_oracle(row)
    if row["fixture"] in ("half-layered", "half-volume"):
        return half_surface_oracle(row)
    if row["fixture"] in ("integers", "mixed"):
        return expanded_oracle(row)
    n = row["width"] * row["height"] * row["lanes"]
    half, lanes = row["half"], row["lanes"]
    pattern = INITIAL_HALF if half else INITIAL_FLOAT
    if row["fixture"] == "matrix" and row["entry"] == "loadValues":
        pattern = LOAD_HALF if half else [widen_half(x) for x in LOAD_HALF]
    if "dynamic" in row["case"]:
        pattern = pattern[:4]
    if "oob" in row["case"] or row["fixture"] == "half-nan":
        pattern = [0x3555, 0xBC00, 0x7D55, 0xFE01]
    initial = [pattern[i % len(pattern)] for i in range(n)]
    if row["fixture"] == "half-load":
        initial = list(range(65536)) + [0x3555] * 256
    expected, observed = list(initial), [0x4F123456] * n
    nan_positions = [set(), set()]
    active = 0
    for y in range(row["height"]):
        for x in range(row["width"]):
            if row["fixture"] == "matrix":
                live = x < 24 and y < 2
            elif row["fixture"] == "half-load":
                live = y < 256
            elif "oob" in row["case"]:
                live = x < 4 and y == 0
            else:
                live = x < (8 if row["fixture"] == "half-nan" else 12) and y < 2
            if not live:
                continue
            active += 1
            for lane in range(lanes):
                i = (y * row["width"] + x) * lanes + lane
                if "oob" in row["case"]:
                    observed[i] = 0 if row["entry"] == "loadValues" else 0x449A4000
                elif row["entry"] == "loadValues":
                    observed[i] = widen_half(initial[i]) if half else initial[i]
                    if half and initial[i] & 0x7C00 == 0x7C00 and initial[i] & 1023:
                        nan_positions[1].add(i)
                else:
                    observed[i] = 0x449A4000
                    if row["fixture"] == "half-nan":
                        expected[i] = 0x7E00
                        nan_positions[0].add(i)
                    elif "dynamic" in row["case"]:
                        if lane == x & 3:
                            expected[i] = 0x3C01 if half else 0x3F801001
                    elif row["entry"] in ("wholeStore", "literalStore") or lane in ((0, 2) if lanes == 4 else (0,)):
                        value = VALUES[(x + y * 24 + 7 * lane) % 24]
                        expected[i] = narrow_half(value) if half else value
    def pack(values, size):
        return b"".join(struct.pack("<H" if size == 2 else "<I", v) for v in values)
    bpc = 2 if half else 4
    return dict(initial=pack(initial, bpc), expected=pack(expected, bpc),
                output_initial=pack([0x4F123456] * n, 4), output_expected=pack(observed, 4),
                nan_positions=nan_positions, active_texels=active,
                guard_texels=row["width"] * row["height"] - active)


def compare(row, buffers, index, actual):
    """Check exact channels and only the explicitly designated converted-NaN classes."""
    resource = resource_buffers(row, buffers)[index]
    expected = resource["expected"]
    size = FORMATS[resource_specs(row)[index]["storage"]][1]
    require(len(actual) == len(expected), "Wrong physical readback length")
    failures, nan_bits = [], Counter()
    for i in range(len(actual) // size):
        a = int.from_bytes(actual[i * size:(i + 1) * size], "little")
        e = int.from_bytes(expected[i * size:(i + 1) * size], "little")
        if i in resource["nan_positions"]:
            mask, fraction = (0x7C00, 1023) if size == 2 else (0x7F800000, 0x7FFFFF)
            good = a & mask == mask and a & fraction != 0
            nan_bits[hex(a)] += 1
        else:
            good = a == e
        if not good:
            failures.append(dict(channel=i, actual=hex(a), expected=hex(e)))
    return dict(actual_sha256=sha(actual), expected_sha256=sha(expected),
                mismatch_count=len(failures), mismatches=failures,
                classified_nan_channels=sum(nan_bits.values()), nan_payload_counts=dict(nan_bits),
                exact_channels=len(actual) // size - sum(nan_bits.values()))


U, I, U64, SZ, VP = C.c_uint, C.c_int, C.c_uint64, C.c_size_t, C.c_void_p


class Array3DDesc(C.Structure):
    _fields_ = [("Width", SZ), ("Height", SZ), ("Depth", SZ),
                ("Format", I), ("NumChannels", U), ("Flags", U)]


class ResourceUnion(C.Union):
    _fields_ = [("array", VP), ("reserved", I * 32)]


class ResourceDesc(C.Structure):
    _fields_ = [("resType", I), ("res", ResourceUnion), ("flags", U)]


class Copy2D(C.Structure):
    _fields_ = [("srcXInBytes", SZ), ("srcY", SZ), ("srcMemoryType", I),
                ("srcHost", VP), ("srcDevice", U64), ("srcArray", VP), ("srcPitch", SZ),
                ("dstXInBytes", SZ), ("dstY", SZ), ("dstMemoryType", I),
                ("dstHost", VP), ("dstDevice", U64), ("dstArray", VP), ("dstPitch", SZ),
                ("WidthInBytes", SZ), ("Height", SZ)]


class Copy3D(C.Structure):
    _fields_ = [("srcXInBytes", SZ), ("srcY", SZ), ("srcZ", SZ), ("srcLOD", SZ),
                ("srcMemoryType", I), ("srcHost", VP), ("srcDevice", U64), ("srcArray", VP),
                ("reserved0", VP), ("srcPitch", SZ), ("srcHeight", SZ),
                ("dstXInBytes", SZ), ("dstY", SZ), ("dstZ", SZ), ("dstLOD", SZ),
                ("dstMemoryType", I), ("dstHost", VP), ("dstDevice", U64), ("dstArray", VP),
                ("reserved1", VP), ("dstPitch", SZ), ("dstHeight", SZ),
                ("WidthInBytes", SZ), ("Height", SZ), ("Depth", SZ)]


def validate_host_abi():
    """Match the CUDA 64-bit Driver API structures before passing any foreign pointers."""
    require(sys.byteorder == "little" and C.sizeof(VP) == C.sizeof(SZ) == 8, "64-bit LE host required")
    require(C.sizeof(Array3DDesc) == 40 and Array3DDesc.Flags.offset == 32, "Array ABI mismatch")
    require(C.sizeof(ResourceDesc) == 144 and ResourceDesc.res.offset == 8 and
            ResourceDesc.flags.offset == 136, "Resource ABI mismatch")
    require(C.sizeof(Copy2D) == 128 and Copy2D.srcHost.offset == 24 and
            Copy2D.dstMemoryType.offset == 72 and Copy2D.dstHost.offset == 80 and
            Copy2D.WidthInBytes.offset == 112 and Copy2D.Height.offset == 120, "Copy ABI mismatch")
    require(C.sizeof(Copy3D) == 200 and Copy3D.srcZ.offset == 16 and
            Copy3D.srcHost.offset == 40 and Copy3D.srcArray.offset == 56 and
            Copy3D.srcPitch.offset == 72 and Copy3D.srcHeight.offset == 80 and
            Copy3D.dstMemoryType.offset == 120 and Copy3D.dstHost.offset == 128 and
            Copy3D.dstArray.offset == 144 and Copy3D.dstPitch.offset == 160 and
            Copy3D.dstHeight.offset == 168 and Copy3D.WidthInBytes.offset == 176 and
            Copy3D.Height.offset == 184 and Copy3D.Depth.offset == 192, "Layered copy ABI mismatch")


def run_device(ptx, row, buffers, output):
    """Initialize each physical array, execute once, and independently copy all bytes back."""
    validate_host_abi()
    result = dict(status="running", cleanup=[])
    def save():
        write_json(output / "runtime.json", result)
    context, module = VP(), VP()
    specs, data = resource_specs(row), resource_buffers(row, buffers)
    arrays, surfaces = [VP() for _ in specs], [U64() for _ in specs]
    driver = None
    try:
        driver = C.CDLL('libcuda.so.1')
        signatures = {
            'cuInit': [U], 'cuDeviceGet': [C.POINTER(I), I],
            'cuDriverGetVersion': [C.POINTER(I)],
            'cuDeviceGetName': [C.c_char_p, I, I],
            'cuDeviceComputeCapability': [C.POINTER(I), C.POINTER(I), I],
            'cuCtxCreate_v2': [C.POINTER(VP), U, I], 'cuCtxDestroy_v2': [VP],
            'cuArray3DCreate_v2': [C.POINTER(VP), C.POINTER(Array3DDesc)],
            'cuArray3DGetDescriptor_v2': [C.POINTER(Array3DDesc), VP],
            'cuArrayDestroy': [VP],
            'cuSurfObjectCreate': [C.POINTER(U64), C.POINTER(ResourceDesc)],
            'cuSurfObjectGetResourceDesc': [C.POINTER(ResourceDesc), U64],
            'cuSurfObjectDestroy': [U64], 'cuMemcpy2D_v2': [C.POINTER(Copy2D)],
            'cuMemcpy3D_v2': [C.POINTER(Copy3D)],
            'cuModuleLoadData': [C.POINTER(VP), VP],
            'cuModuleGetFunction': [C.POINTER(VP), VP, C.c_char_p],
            'cuModuleGetGlobal_v2': [C.POINTER(U64), C.POINTER(SZ), VP, C.c_char_p],
            'cuModuleUnload': [VP], 'cuMemcpyHtoD_v2': [U64, VP, SZ],
            'cuLaunchKernel': [VP, U, U, U, U, U, U, U, VP, C.POINTER(VP), C.POINTER(VP)],
            'cuCtxSynchronize': [], 'cuGetErrorName': [I, C.POINTER(C.c_char_p)],
            'cuGetErrorString': [I, C.POINTER(C.c_char_p)],
        }
        for name, params in signatures.items():
            getattr(driver, name).argtypes = params
            getattr(driver, name).restype = I
        def check(name, *params):
            code = getattr(driver, name)(*params)
            if code:
                label, message = C.c_char_p(), C.c_char_p()
                driver.cuGetErrorName(code, C.byref(label))
                driver.cuGetErrorString(code, C.byref(message))
                raise RuntimeError(f'{name}: {code} {label.value!r} {message.value!r}')
        def copy_array(array, data, spec, upload):
            host = C.create_string_buffer(data, len(data))
            depth = row.get('volume_depth', row.get('array_layers', 1))
            is_array = 'array_layers' in row
            copy_3d = is_array or 'volume_depth' in row
            copy = Copy3D() if copy_3d else Copy2D()
            pitch = row['width'] * spec['lanes'] * FORMATS[spec['storage']][1]
            require(len(data) == pitch * row['height'] * depth, 'Host array copy extent mismatch')
            copy.WidthInBytes, copy.Height = pitch, row['height']
            if copy_3d:
                copy.Depth = depth
                copy.srcHeight = copy.dstHeight = row['height']
            if upload:
                copy.srcMemoryType, copy.srcHost, copy.srcPitch = 1, C.addressof(host), pitch
                copy.dstMemoryType, copy.dstArray = 3, array.value
            else:
                copy.srcMemoryType, copy.srcArray = 3, array.value
                copy.dstMemoryType, copy.dstHost, copy.dstPitch = 1, C.addressof(host), pitch
            check('cuMemcpy3D_v2' if copy_3d else 'cuMemcpy2D_v2', C.byref(copy))
            return host.raw
        check('cuInit', 0)
        device = I()
        check('cuDeviceGet', C.byref(device), 0)
        name, version, major, minor = C.create_string_buffer(256), I(), I(), I()
        check('cuDeviceGetName', name, 256, device)
        check('cuDriverGetVersion', C.byref(version))
        check('cuDeviceComputeCapability', C.byref(major), C.byref(minor), device)
        result['device'] = dict(name=name.value.decode(), driver_api_version=version.value,
                                compute_capability=[major.value, minor.value])
        check('cuCtxCreate_v2', C.byref(context), 0, device)
        result['actual_array_descriptors'] = []
        for i, spec in enumerate(specs):
            fmt = FORMATS[spec['storage']][0]
            depth = row.get('volume_depth', row.get('array_layers', 0))
            is_array = 'array_layers' in row
            desc = Array3DDesc(row['width'], row['height'] if row['shape'] >= 2 else 0,
                               depth, fmt, spec['lanes'],
                               3 if is_array else 2)
            check('cuArray3DCreate_v2', C.byref(arrays[i]), C.byref(desc))
            actual_desc = Array3DDesc()
            check('cuArray3DGetDescriptor_v2', C.byref(actual_desc), arrays[i])
            actual = {key: getattr(actual_desc, key) for key, _ in Array3DDesc._fields_}
            require(actual == {key: getattr(desc, key) for key, _ in Array3DDesc._fields_},
                    'CUDA array descriptor mismatch')
            result['actual_array_descriptors'].append(actual)
            resource = ResourceDesc()
            resource.resType, resource.res.array = 0, arrays[i]
            check('cuSurfObjectCreate', C.byref(surfaces[i]), C.byref(resource))
            queried = ResourceDesc()
            check('cuSurfObjectGetResourceDesc', C.byref(queried), surfaces[i])
            require(queried.resType == 0 and queried.res.array == arrays[i].value and
                    queried.flags == 0, 'CUDA surface resource binding mismatch')
            initial = data[i]['initial']
            copy_array(arrays[i], initial, spec, True)
            require(copy_array(arrays[i], bytes(len(initial)), spec, False) == initial,
                    'Host upload did not preserve the initial array bytes')
        result['surface_bindings_verified'] = True
        result['initial_host_copies_verified'] = True
        ptx_buffer = C.create_string_buffer(ptx)
        check('cuModuleLoadData', C.byref(module), ptx_buffer)
        globals_pointer, globals_size = U64(), SZ()
        check('cuModuleGetGlobal_v2', C.byref(globals_pointer), C.byref(globals_size),
              module, b'SLANG_globalParams')
        require(globals_size.value == 8 * len(specs), 'Loaded CUDA module has wrong global size')
        global_data = (U64 * len(specs))(*(surface.value for surface in surfaces))
        check('cuMemcpyHtoD_v2', globals_pointer, global_data, C.sizeof(global_data))
        result['global_surface_handles_uploaded'] = True
        function = VP()
        check('cuModuleGetFunction', C.byref(function), module, row['entry'].encode())
        check('cuLaunchKernel', function, row['width'], row['height'],
              row.get('volume_depth', row.get('array_layers', 1)),
              1, 1, 1, 0, None, None, None)
        check('cuCtxSynchronize')
        result['launched_and_synchronized'] = True
        result['readbacks'] = []
        for i, spec in enumerate(specs):
            expected = data[i]['expected']
            actual = copy_array(arrays[i], bytes(len(expected)), spec, False)
            name = spec['name']
            (output / (name + '-actual.bin')).write_bytes(actual)
            checked = compare(row, buffers, i, actual)
            checked.update(array=name, active_texels=data[i]["active_texels"],
                           guard_texels=data[i]["guard_texels"])
            result['readbacks'].append(checked)
        result['status'] = ('runtime-mismatch' if any(x['mismatch_count'] for x in
                            result['readbacks']) else 'passed')
    except Exception as error:
        result.update(status='runtime-failed', error=str(error))
    finally:
        if driver is not None:
            cleanup = [('cuModuleUnload', module)]
            cleanup += [('cuSurfObjectDestroy', h) for h in reversed(surfaces)]
            cleanup += [('cuArrayDestroy', h) for h in reversed(arrays)]
            cleanup += [('cuCtxDestroy_v2', context)]
            for operation, handle in cleanup:
                if handle.value:
                    code = getattr(driver, operation)(handle)
                    result['cleanup'].append(dict(operation=operation, return_code=code))
            if any(x['return_code'] for x in result['cleanup']):
                result['status'] = 'failed-cleanup'
        save()
    return result


def validate_bindings(reflection, row, ptx, architecture):
    """Verify every resource descriptor against reflection and the actual PTX launch ABI."""
    if "volume_depth" in row:
        require(row["shape"] == 3 and "array_layers" not in row and row["volume_depth"] > 0,
                "Volume depth requires a non-array 3D surface")
    params = reflection.get("parameters", [])
    specs = resource_specs(row)
    require(len(params) == len(specs), "Wrong global surface binding count")
    for i, (parameter, spec) in enumerate(zip(params, specs)):
        name, offset = spec["name"], i * 8
        binding, ty = parameter["binding"], parameter["type"]
        require(parameter["name"] == name and binding["kind"] == "uniform" and
                binding["offset"] == offset and binding["size"] == 8, "Surface binding mismatch")
        require(ty["kind"] == "resource" and ty["baseShape"] == f'texture{row["shape"]}D' and
                ty["access"] == "readWrite", "Surface shape/access mismatch")
        require(ty.get("array", False) == ("array_layers" in row),
                "Surface array role mismatch")
        result = ty["resultType"]
        if spec["lanes"] > 1:
            require(result["kind"] == "vector" and result["elementCount"] == spec["lanes"],
                    "Surface lane count mismatch")
            result = result["elementType"]
        require(result["kind"] == "scalar" and result["scalarType"] == spec["scalar"],
                "Surface logical element mismatch")
        require(parameter.get("format") == spec["format"], "Surface format annotation mismatch")
    entries = [x for x in reflection.get("entryPoints", []) if x.get("name") == row["entry"]]
    require(len(entries) == 1 and entries[0]["stage"] == "compute" and
            entries[0]["threadGroupSize"] == [1, 1, 1], "Entry reflection mismatch")
    require(re.search(r"\.entry\s+" + re.escape(row["entry"]) + r"\s*\(\s*\)", ptx),
            "PTX entry must have no explicit parameters")
    require(re.search(r"\.const\s+\.align\s+8\s+\.b8\s+SLANG_globalParams\[" +
                      str(8 * len(specs)) + r"\]", ptx),
            "PTX globals must contain exactly the reflected 64-bit surface handles")
    require(re.search(r"\.target\s+sm_" + str(architecture) + r"\b", ptx), "PTX target mismatch")


def run_command(command, path, environment, timeout):
    """Retain command diagnostics, timeout and failure-to-start separately from successful exit."""
    started = time.monotonic()
    record = dict(command=[str(x) for x in command], return_code=None)
    try:
        with path.open("w", encoding="utf-8") as stream:
            record["return_code"] = subprocess.run(record["command"], cwd=REPO, env=environment,
                stdout=stream, stderr=subprocess.STDOUT, timeout=timeout, check=False).returncode
    except (OSError, subprocess.TimeoutExpired) as error:
        record["error"] = str(error)
    record.update(elapsed_seconds=time.monotonic() - started, log=str(path))
    return record


def self_test():
    """CPU-only contracts for finite rounding, NaN boundaries, guarded bytes and reflection."""
    validate_host_abi()
    for value in VALUES:
        floating = struct.unpack("<f", struct.pack("<I", value))[0]
        try:
            expected = struct.unpack("<H", struct.pack("<e", floating))[0]
        except OverflowError:
            expected = 0xFC00 if floating < 0 else 0x7C00
        require(narrow_half(value) == expected, "Independent binary16 narrowing cross-check failed")
    for value in range(65536):
        if value & 0x7C00 != 0x7C00 or value & 1023 == 0:
            expected = struct.unpack("<I", struct.pack("<f", struct.unpack("<e",
                                    struct.pack("<H", value))[0]))[0]
            require(widen_half(value) == expected, "Independent exhaustive widening check failed")
    for row in cases():
        buffers = oracle(row)
        for index, data in enumerate(resource_buffers(row, buffers)):
            require(compare(row, buffers, index, data["expected"])["mismatch_count"] == 0,
                    "Reference oracle rejected")
    # Each added resource has independent storage, including source-only arrays and guard texels.
    for row in (x for x in cases() if x["fixture"] in
                ("integers", "mixed", "layered", "half-layered", "half-volume",
                 "integer-formats", "integer-spatial", "native-narrow", "normalized")):
        buffers = oracle(row)
        specs = resource_specs(row)
        for index, data in enumerate(resource_buffers(row, buffers)):
            size = FORMATS[specs[index]["storage"]][1]
            for position in (0, 24 * specs[index]["lanes"]):
                damaged = bytearray(data["expected"])
                damaged[position * size] ^= 1
                require(compare(row, buffers, index, damaged)["mismatch_count"] == 1,
                        "Resource or guard corruption was ignored")
        params = []
        for index, spec in enumerate(specs):
            ty = dict(kind="scalar", scalarType=spec["scalar"])
            if spec["lanes"] > 1:
                ty = dict(kind="vector", elementCount=spec["lanes"], elementType=ty)
            params.append(dict(name=spec["name"], format=spec["format"],
                               binding=dict(kind="uniform", offset=index * 8, size=8),
                               type=dict(kind="resource", baseShape=f'texture{row["shape"]}D',
                                         access="readWrite", resultType=ty)))
            if "array_layers" in row:
                params[-1]["type"]["array"] = True
        reflection = dict(parameters=params, entryPoints=[dict(name=row["entry"],
                          stage="compute", threadGroupSize=[1, 1, 1])])
        ptx = f'.target sm_80\n.const .align 8 .b8 SLANG_globalParams[{len(specs) * 8}]\n'
        ptx += f'.entry {row["entry"]}()'
        validate_bindings(reflection, row, ptx, 80)
        if "array_layers" in row:
            # An explicit singleton keeps its array role, independently of extent.
            validate_bindings(reflection, dict(row, array_layers=1), ptx, 80)
        if "volume_depth" in row:
            for invalid in (dict(row, array_layers=3), dict(row, shape=2), dict(row, volume_depth=0)):
                try:
                    validate_bindings(reflection, invalid, ptx, 80)
                except ValueError:
                    pass
                else:
                    raise ValueError("Invalid spatial depth/array role combination was ignored")
        for index in range(len(specs)):
            for field, value in (("name", "wrongResource"),
                                 ("format", "rgba16" if specs[index]["format"] == "rgba8" else "rgba8")):
                damaged = copy.deepcopy(reflection)
                damaged["parameters"][index][field] = value
                try:
                    validate_bindings(damaged, row, ptx, 80)
                except ValueError:
                    pass
                else:
                    raise ValueError("Resource binding/format substitution was ignored")
            damaged = copy.deepcopy(reflection)
            damaged["parameters"][index]["type"]["array"] = "array_layers" not in row
            try:
                validate_bindings(damaged, row, ptx, 80)
            except ValueError:
                pass
            else:
                raise ValueError("Surface array role substitution was ignored")
            if row["fixture"] == "normalized":
                scalar = specs[index]["scalar"]
                for replacement in ("float32" if scalar == "float16" else "float16", "int32"):
                    damaged = copy.deepcopy(reflection)
                    result = damaged["parameters"][index]["type"]["resultType"]
                    if specs[index]["lanes"] > 1:
                        result["elementType"]["scalarType"] = replacement
                    else:
                        result["scalarType"] = replacement
                    try:
                        validate_bindings(damaged, row, ptx, 80)
                    except ValueError:
                        pass
                    else:
                        raise ValueError("Normalized logical type substitution was ignored")
                if index % 2 == 0:
                    original = specs[index]["format"]
                    alternatives = (original.replace("_snorm", "i") if "_snorm" in original else original + "ui",
                                    original.replace("_snorm", "") if "_snorm" in original else original + "_snorm")
                    for replacement in alternatives:
                        damaged = copy.deepcopy(reflection)
                        damaged["parameters"][index]["format"] = replacement
                        try:
                            validate_bindings(damaged, row, ptx, 80)
                        except ValueError:
                            pass
                        else:
                            raise ValueError("Normalized encoding/signedness substitution was ignored")
            if row["fixture"] in ("integer-formats", "integer-spatial", "native-narrow"):
                scalar = specs[index]["scalar"]
                opposite = scalar[1:] if scalar.startswith("u") else "u" + scalar
                wrong_width = ("uint" if scalar.startswith("u") else "int") + (
                    "16" if scalar.endswith("32") else "32")
                for invalid_type in (dict(kind="scalar", scalarType="float32"),
                                     dict(kind="scalar", scalarType=wrong_width),
                                     dict(kind="scalar", scalarType=opposite)):
                    damaged = copy.deepcopy(reflection)
                    result = damaged["parameters"][index]["type"]["resultType"]
                    if specs[index]["lanes"] > 1:
                        result["elementType"] = invalid_type
                    else:
                        damaged["parameters"][index]["type"]["resultType"] = invalid_type
                    try:
                        validate_bindings(damaged, row, ptx, 80)
                    except ValueError:
                        pass
                    else:
                        raise ValueError("Narrow format logical type substitution was ignored")
    for bits in (8, 16):
        for signed in (False, True):
            maximum = (1 << (bits - int(signed))) - 1
            require(encode_normalized(float("nan"), bits, signed) == 0, "NaN did not encode zero")
            require(encode_normalized(float("inf"), bits, signed) == maximum, "Upper clamp failed")
            require(encode_normalized(-float("inf"), bits, signed) ==
                    ((-maximum) & ((1 << bits) - 1) if signed else 0), "Lower clamp failed")
            require(encode_normalized(0.5, bits, signed) == (maximum + 1) // 2,
                    "Ties-away conversion failed")
    normalized_rows = [x for x in cases() if x["fixture"] == "normalized"]
    require(len(normalized_rows) == 30, "Incomplete normalized geometry/operation/type family")
    # Each input reaches every dynamic component, even though different lanes see shifted inputs.
    for resource in range(12):
        for lanes in (1, 2, 4):
            for lane in range(lanes):
                seen = {((x - 1) // 4 + resource * 3 + lane * 5) % 64
                        for x in range(1, 257) if (x - 1) % lanes == lane}
                require(len(seen) == 64, "Normalized dynamic lane misses an input boundary")
    partial = next(x for x in normalized_rows if x["case"] == "normalized-1d-static-components-float")
    buffers = oracle(partial)
    for index, spec in enumerate(resource_specs(partial)):
        if not spec["storage"].startswith("int") or spec["lanes"] == 1:
            continue
        size = FORMATS[spec["storage"]][1]
        minimum = (1 << (size * 8 - 1)).to_bytes(size, "little")
        data = buffers["resources"][index]
        candidates = [position for position in range(len(data["initial"]) // size)
                      if position % spec["lanes"] not in (1, 3) and
                      data["initial"][position * size:(position + 1) * size] == minimum]
        require(candidates, "Missing untouched SNORM minimum input")
        for position in candidates:
            require(data["expected"][position * size:(position + 1) * size] == minimum,
                    "Untouched SNORM minimum was re-encoded")
        damaged = bytearray(data["expected"])
        damaged[candidates[0] * size] ^= 1
        require(compare(partial, buffers, index, damaged)["mismatch_count"] == 1,
                "SNORM equivalent-value bit corruption ignored")
    # Verify source input constants independently of any generated output or compiler result.
    source = (REPO / "tests/cuda/nvvm-surface-physical-normalized.slang").read_text()
    encoded = [int(v, 16) for v in re.findall(r"return asfloat\(0x([0-9a-f]+)u\)", source)]
    require(encoded == normalized_inputs(), "Shader/host normalized input selection differs")
    for value, bits, signed, expected in (
            (128, 8, True, 0x7F), (-129, 8, True, 0x80),
            (32768, 16, True, 0x7FFF), (-32769, 16, True, 0x8000),
            (256, 8, False, 0xFF), (65536, 16, False, 0xFFFF),
            (0xFFFFFFFF, 8, False, 0xFF), (0x80000000, 16, False, 0xFFFF)):
        require(narrow_integer(value, bits, signed) == expected, "Integer saturation anchor failed")
    for value, bits, signed, expected in (
            (0x80, 8, True, 0xFFFFFF80), (0x80, 8, False, 0x80),
            (0xFF, 8, True, 0xFFFFFFFF), (0xFF, 8, False, 0xFF),
            (0x8000, 16, True, 0xFFFF8000), (0x8000, 16, False, 0x8000),
            (0xFFFF, 16, True, 0xFFFFFFFF), (0xFFFF, 16, False, 0xFFFF)):
        require(widen_integer(value, bits, signed) == expected, "Integer extension anchor failed")
    for row in (x for x in cases() if x["fixture"] in
                ("integer-formats", "integer-spatial", "native-narrow")):
        buffers = oracle(row)
        specs, data = resource_specs(row), resource_buffers(row, buffers)
        operation = row["defines"]["SURFACE_OPERATION"]
        y = 0 if row["shape"] == 1 else 1
        z = 1 if "volume_depth" in row or "array_layers" in row else 0
        require(len(specs) == 24 and len(data) == 24, "Integer format resource inventory changed")
        for resource in range(12):
            spec, result = specs[resource * 2], data[resource * 2]
            lanes, size = spec["lanes"], FORMATS[spec["storage"]][1]
            covered = [set() for _ in range(lanes)]
            high_bits = [set() for _ in range(lanes)]
            untouched = []
            for x in range(1, 65):
                for lane in range(lanes):
                    channel = ((z * row["height"] + y) * row["width"] + x) * lanes + lane
                    position = channel * size
                    before = result["initial"][position:position + size]
                    high_bits[lane].add(bool(before[-1] & 0x80))
                    selected = (operation == 0 or lanes == 1 or
                                (operation == 1 and lane in ((1, 3) if lanes == 4 else (1,))) or
                                (operation == 2 and lane == (x - 1) % lanes))
                    if selected:
                        edge = (x - 1) // 4 if operation == 2 else x - 1
                        covered[lane].add((edge + z * 5 + y * 7 + resource * 3 + lane * 5) % 16)
                    else:
                        require(result["expected"][position:position + size] == before,
                                "Component oracle overwrote an unselected lane")
                        untouched.append(position)
            for lane in range(lanes):
                wanted = set(range(16)) if (operation != 1 or lanes == 1 or
                            lane in ((1, 3) if lanes == 4 else (1,))) else set()
                require(covered[lane] == wanted, "Integer edge/lane coverage incomplete")
                require(high_bits[lane] == {False, True}, "Integer load sign-boundary coverage missing")
            if operation and lanes > 1:
                require(untouched, "Component row lacks preserved lanes")
                damaged = bytearray(result["expected"])
                damaged[untouched[0]] ^= 1
                require(compare(row, buffers, resource * 2, damaged)["mismatch_count"] == 1,
                        "Untouched integer lane corruption was ignored")
            # Original input differs from independent stores, and every observed
            # array contains widened input bits rather than narrowed-store readback.
            require(compare(row, buffers, resource * 2, result["initial"])["mismatch_count"] > 0,
                    "Integer store oracle permits a kernel that performs no stores")
            observed = data[resource * 2 + 1]
            require(compare(row, buffers, resource * 2 + 1, observed["initial"])["mismatch_count"] > 0,
                    "Integer load oracle permits a kernel that performs no loads")
    for row in (x for x in cases() if x["fixture"] in ("integer-spatial", "native-narrow") and
                ("volume_depth" in x or "array_layers" in x)):
        buffers = oracle(row)
        specs, data = resource_specs(row), resource_buffers(row, buffers)
        width, height = row["width"], row["height"]
        operation = row["defines"]["SURFACE_OPERATION"]
        for resource, (spec, result) in enumerate(zip(specs, data)):
            lanes, size = spec["lanes"], FORMATS[spec["storage"]][1]
            plane_size = width * height * lanes * size
            # Both entire exterior planes must retain their nonzero initialized bytes.
            for plane in (0, 3):
                start = plane * plane_size
                require(result["expected"][start:start + plane_size] ==
                        result["initial"][start:start + plane_size], "Integer guard plane changed")
                damaged = bytearray(result["expected"])
                damaged[start] ^= 1
                require(compare(row, buffers, resource, damaged)["mismatch_count"] == 1,
                        "Integer guard plane corruption was ignored")
            y = 0 if row["shape"] == 1 else 1
            start = ((height + y) * width + 1) * lanes * size
            damaged = bytearray(result["expected"])
            damaged[start] ^= 1
            require(compare(row, buffers, resource, damaged)["mismatch_count"] == 1,
                    "Integer active plane corruption was ignored")
            if lanes > 1:
                damaged = bytearray(result["expected"])
                wrong = start - (lanes - 1) * size
                damaged[wrong:wrong + lanes * size] = damaged[start:start + lanes * size]
                require(compare(row, buffers, resource, damaged)["mismatch_count"] > 0,
                        "Integer scalar-sized X byte scaling was ignored")
            if height > 1:
                damaged = bytearray(result["expected"])
                row_size = width * lanes * size
                a, b = plane_size + row_size, plane_size + 2 * row_size
                damaged[a:a + row_size], damaged[b:b + row_size] = (
                    damaged[b:b + row_size], damaged[a:a + row_size])
                require(compare(row, buffers, resource, damaged)["mismatch_count"] > 0,
                        "Integer spatial row permutation was ignored")
            if resource % 2:
                continue
            damaged = bytearray(result["expected"])
            for z in (1, 2):
                for y in range(height):
                    if row["shape"] != 1 and not 1 <= y <= 3:
                        continue
                    covered = [set() for _ in range(lanes)]
                    for x in range(1, 65):
                        for lane in range(lanes):
                            selected = (operation == 0 or lanes == 1 or
                                        (operation == 1 and lane in ((1, 3) if lanes == 4 else (1,))) or
                                        (operation == 2 and lane == (x - 1) % lanes))
                            if not selected:
                                continue
                            edge = (x - 1) // 4 if operation == 2 else x - 1
                            covered[lane].add((edge + z * 5 + y * 7 + resource // 2 * 3 + lane * 5) % 16)
                            target = (((z * height + y) * width + x) * lanes + lane) * size
                            other = ((((3 - z) * height + y) * width + x) * lanes + lane) * size
                            # Model a shared load/output/store plane permutation: observed
                            # original values can cancel, but generated source stores cannot.
                            damaged[target:target + size] = result["expected"][other:other + size]
                    for lane in range(lanes):
                        wanted = set(range(16)) if (operation != 1 or lanes == 1 or
                                    lane in ((1, 3) if lanes == 4 else (1,))) else set()
                        require(covered[lane] == wanted, "Integer plane edge/lane coverage incomplete")
            require(compare(row, buffers, resource, damaged)["mismatch_count"] > 0,
                    "Matching integer load/store plane permutation escaped independent stores")
    for row in (x for x in cases() if x["fixture"] == "layered"):
        buffers = oracle(row)
        specs = resource_specs(row)
        data = resource_buffers(row, buffers)
        for resource, (spec, result) in enumerate(zip(specs, data)):
            lanes = spec["lanes"]
            active_texel = 1 if row["shape"] == 1 else (2 * row["height"] + 4) * row["width"] + 1
            for channel in (0, active_texel * lanes):
                damaged = bytearray(result["expected"])
                damaged[channel * 4] ^= 1
                require(compare(row, buffers, resource, damaged)["mismatch_count"] == 1,
                        "Layered active/guard corruption was ignored")
            stride = row["width"] * row["height"] * lanes * 4
            damaged = bytearray(result["expected"])
            damaged[:stride], damaged[stride:2 * stride] = damaged[stride:2 * stride], damaged[:stride]
            require(compare(row, buffers, resource, damaged)["mismatch_count"] > 0,
                    "Layer permutation was hidden by repeated input patterns")
            if resource % 2 == 0:
                require(compare(row, buffers, resource, data[resource + 1]["expected"])["mismatch_count"] > 0,
                        "Source/observed resource substitution was ignored")
            if lanes > 1:
                damaged = bytearray(result["expected"])
                # Model using scalar-sized X byte scaling for a vector texel write.
                start = active_texel * lanes * 4
                wrong = start - (lanes - 1) * 4
                damaged[wrong:wrong + lanes * 4] = damaged[start:start + lanes * 4]
                require(compare(row, buffers, resource, damaged)["mismatch_count"] > 0,
                        "Wrong vector X byte scale was ignored")
    for row in (x for x in cases() if x["fixture"] in ("half-layered", "half-volume")):
        buffers = oracle(row)
        specs = resource_specs(row)
        data = resource_buffers(row, buffers)
        width, height = row["width"], row["height"]
        partial = row["entry"] == "componentCopies"
        for source in range(0, 12, 2):
            lanes = specs[source]["lanes"]
            result = data[source]
            damaged = bytearray(result["expected"])
            # Simulate matching load/store layer permutation: result copies could cancel,
            # but source stores carry shader-generated logical-layer markers independently.
            for layer in (0, 1):
                for y in range(height):
                    if row["shape"] != 1 and not 1 <= y < 4:
                        continue
                    for x in range(1, 9):
                        for lane in range(lanes):
                            if partial and lane not in ((1, 3) if lanes == 4 else (lanes - 1,)):
                                continue
                            index = ((layer * height + y) * width + x) * lanes + lane
                            other = (((1 - layer) * height + y) * width + x) * lanes + lane
                            damaged[index * 2:index * 2 + 2] = result["expected"][other * 2:other * 2 + 2]
            require(compare(row, buffers, source, damaged)["mismatch_count"] > 0,
                    "Matching Half load/store layer permutation escaped independent markers")
            require(compare(row, buffers, source, data[source + 1]["expected"])["mismatch_count"] > 0,
                    "Half source/result substitution was ignored")
            if partial and lanes > 1:
                destination = data[source + 1]
                candidates = [i for i in range(len(destination["expected"]) // 2)
                              if i % lanes not in ((1, 3) if lanes == 4 else (lanes - 1,)) and
                              int.from_bytes(destination["expected"][i * 2:i * 2 + 2], "little") & 0x7FFF > 0x7C00]
                require(candidates, "Component fixture lacks untouched Half NaN payload controls")
                damaged = bytearray(destination["expected"])
                damaged[candidates[0] * 2] ^= 1
                require(compare(row, buffers, source + 1, damaged)["mismatch_count"] == 1,
                        "Untouched Half NaN payload was treated as a converted NaN")
        converted_sizes = set()
        for resource, result in enumerate(data):
            if not result["nan_positions"]:
                continue
            size = FORMATS[specs[resource]["storage"]][1]
            converted_sizes.add(size)
            position = min(result["nan_positions"])
            for bits, failures in ((0xFE13 if size == 2 else 0xFFC12345, 0), (0, 1)):
                damaged = bytearray(result["expected"])
                damaged[position * size:(position + 1) * size] = bits.to_bytes(size, "little")
                require(compare(row, buffers, resource, damaged)["mismatch_count"] == failures,
                        "Layered converted-NaN class check is incorrect")
        require(converted_sizes == {2, 4}, "Layered fixture lacks both converted-NaN directions")
        native = data[1]
        copied_nan = next((i for i in range(len(native["expected"]) // 2)
                           if native["expected"][i * 2:i * 2 + 2] != native["initial"][i * 2:i * 2 + 2]
                           and int.from_bytes(native["expected"][i * 2:i * 2 + 2], "little") & 0x7FFF > 0x7C00), None)
        require(copied_nan is not None, "Native Half copy lacks exact NaN payload coverage")
        damaged = bytearray(native["expected"])
        damaged[copied_nan * 2] ^= 1
        require(compare(row, buffers, 1, damaged)["mismatch_count"] == 1,
                "Native Half copy incorrectly admitted NaN payload changes")
    literal = oracle(next(x for x in cases() if x["case"] == "half-1d-1-literal-store"))
    dynamic = oracle(next(x for x in cases() if x["case"] == "half-1d-1-wholeStore"))
    require(literal == dynamic, "Literal stores changed the frozen whole-store oracle")
    row = next(x for x in cases() if x["case"] == "half-all-bits-load")
    buffers = oracle(row)
    require(len(buffers["nan_positions"][1]) == 2046, "Wrong Half NaN cardinality")
    for bits, bad in [(0xFF800001, False), (0x7F800000, True), (0, True)]:
        data = bytearray(buffers["output_expected"])
        data[0x7C01 * 4:0x7C02 * 4] = bits.to_bytes(4, "little")
        require(bool(compare(row, buffers, 1, data)["mismatch_count"]) == bad,
                "NaN exception admitted a non-NaN or rejected a NaN")
    for position in [0x3C00, 65536]:
        data = bytearray(buffers["output_expected"])
        data[position * 4] ^= 1
        require(compare(row, buffers, 1, data)["mismatch_count"] == 1,
                "Finite/guard corruption was ignored")
    # Empty reflection must not become a successful GPU invocation.
    try:
        validate_bindings({"parameters": []}, row, "", 80)
    except ValueError:
        pass
    else:
        raise ValueError("Invalid reflection accepted")
    with tempfile.TemporaryDirectory(prefix="nvvm-surface-identity-") as directory:
        path = Path(directory) / "input"
        path.write_bytes(b"before")
        identity = freeze_identity({path})
        verify_identity(identity)
        path.write_bytes(b"after")
        try:
            verify_identity(identity)
        except ValueError:
            pass
        else:
            raise ValueError("Identity mutation was ignored")
        path.unlink()
        try:
            verify_identity(identity)
        except ValueError:
            pass
        else:
            raise ValueError("Deleted input was ignored")
    print(f"CPU contracts passed: {len(cases())} cases, exhaustive Half widening, finite narrowing, "
          "literal-store oracle identity, NaN/guard/ABI checks")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--slangc", type=Path)
    p.add_argument("--cuda-root", type=Path)
    p.add_argument("--provenance", type=Path, help="Optional externally captured build/source provenance JSON")
    p.add_argument("--provider", type=Path, help="Directory containing the NVVM provider library")
    p.add_argument("--architecture", type=int, default=80)
    p.add_argument("--modes", nargs="+", choices=["nvrtc-o3", "nvvm-o0", "nvvm-o3"],
                   default=["nvrtc-o3", "nvvm-o0", "nvvm-o3"])
    p.add_argument("--cases", nargs="+", help="Exact case names, listed by --list")
    p.add_argument("--output", type=Path, help="Fresh output directory; existing results are never overwritten")
    p.add_argument("--timeout", type=int, default=120)
    p.add_argument("--list", action="store_true")
    p.add_argument("--self-test", action="store_true")
    p.add_argument("--device-case", type=Path, help=argparse.SUPPRESS)
    args = p.parse_args()
    if args.self_test:
        self_test()
        return 0
    if args.list:
        print("\n".join(x["case"] for x in cases()))
        return 0
    if args.device_case:
        config = json.loads(args.device_case.read_text())
        row = config["row"]
        buffers = oracle(row)
        require(all(sha(oracle_files(row, buffers)[key]) == value for key, value in config["oracle_sha256"].items()),
                "Worker oracle differs from the frozen pre-compilation inputs")
        result = run_device(Path(config["ptx"]).read_bytes(), row, buffers, args.device_case.parent)
        return 0 if result["status"] == "passed" else 1
    require(args.slangc and args.cuda_root and args.output, "--slangc, --cuda-root and --output required")
    require(args.timeout > 0, "Timeout must be positive")
    require(len(set(args.modes)) == len(args.modes), "Repeated modes are not distinct cells")
    args.slangc, args.cuda_root, args.output = (x.resolve() for x in
                                              [args.slangc, args.cuda_root, args.output])
    rows = cases()
    if args.cases:
        require(set(args.cases) <= {x["case"] for x in rows}, "Unknown case selector")
        rows = [x for x in rows if x["case"] in args.cases]
    args.output.mkdir(parents=True, exist_ok=False)
    environment = os.environ.copy()
    environment["CUDA_PATH"] = str(args.cuda_root)
    environment["CUDA_HOME"] = str(args.cuda_root)
    environment["LIBNVVM_HOME"] = str(args.cuda_root)
    paths = [(args.cuda_root / "nvvm/lib64/libnvvm.so").resolve().parent,
             (args.cuda_root / "lib64/libnvrtc.so").resolve().parent,
             args.slangc.parent, args.slangc.parent.parent / "lib"]
    environment["LD_LIBRARY_PATH"] = ":".join(map(str, paths)) + ":" + environment.get("LD_LIBRARY_PATH", "")
    provider = (args.provider or args.slangc.parent).resolve()
    environment["SLANG_NVVM_BUILDER_PATH"] = str(provider)
    libnvvm = (args.cuda_root / "nvvm/lib64/libnvvm.so").resolve()
    ptxas = args.cuda_root / "bin/ptxas"
    identity = freeze_identity(inventory_paths(args, provider))
    checked_each_cell = {Path(__file__).resolve()} | {
        REPO / "tests/cuda" / ("nvvm-surface-physical-" + name + ".slang")
        for name in FIXTURES}
    provenance = dict(scope="Selected runtime installation and fixture inventory, not a loaded-library trace",
                      artifacts=identity, environment={key: environment.get(key) for key in
                      ["CUDA_PATH", "CUDA_HOME", "LIBNVVM_HOME", "SLANG_NVVM_BUILDER_PATH",
                       "LD_LIBRARY_PATH", "CUDA_VISIBLE_DEVICES"]})
    if args.provenance:
        provenance["external_provenance"] = dict(path=str(args.provenance.resolve()),
                                                 sha256=sha(args.provenance.read_bytes()))
    write_json(args.output / "provenance.json", provenance)
    report = dict(status="running", provenance_sha256=sha((args.output / "provenance.json").read_bytes()), tool_sha256=sha(Path(__file__).read_bytes()),
                  slangc=dict(path=str(args.slangc), sha256=sha(args.slangc.read_bytes())),
                  libnvvm=dict(path=str(libnvvm), sha256=sha(libnvvm.read_bytes())),
                  provider=dict(path=str(provider / "libslang-llvm-nvvm.so"),
                                sha256=sha((provider / "libslang-llvm-nvvm.so").read_bytes())),
                  ptxas=dict(path=str(ptxas), sha256=sha(ptxas.read_bytes())),
                  semantic_contract="RN-even finite Half conversions; converted NaN class only; "
                                    "logical32 explicit narrow formats extend on load and saturate on store; "
                                    "native narrow integer stores preserve bits; untouched bits exact",
                  requested_cells=len(rows) * len(args.modes), cells=[])
    for row in rows:
        source = REPO / "tests/cuda" / ("nvvm-surface-physical-" + row["fixture"] + ".slang")
        buffers = oracle(row)
        frozen = args.output / row["case"]
        frozen.mkdir()
        files = oracle_files(row, buffers)
        for key, data in files.items():
            (frozen / (key + ".bin")).write_bytes(data)
        refs = {key: sha(data) for key, data in files.items()}
        write_json(frozen / "oracle.json", dict(row=row, source_sha256=sha(source.read_bytes()),
                   file_sha256=refs, converted_nan_positions=[sorted(x["nan_positions"]) for x in resource_buffers(row, buffers)]))
        for mode in args.modes:
            output = frozen / mode
            output.mkdir()
            cell = dict(case=row["case"], entry=row["entry"], mode=mode, source=str(source),
                        source_sha256=sha(source.read_bytes()), oracle_sha256=sha((frozen / "oracle.json").read_bytes()))
            report["cells"].append(cell)
            try:
                verify_identity(identity, checked_each_cell)
                command = [args.slangc, source, "-target", "ptx", "-entry", row["entry"],
                           "-stage", "compute", "-capability",
                           f'cuda_sm_{args.architecture // 10}_{args.architecture % 10}',
                           "-O" + mode[-1], "-o", output / "code.ptx",
                           "-reflection-json", output / "reflection.json"]
                command += [f"-D{key}={value}" for key, value in row["defines"].items()]
                if mode.startswith("nvvm"):
                    command += ["-emit-cuda-via-nvvm", "-nvvm-path", libnvvm]
                cell["compile"] = run_command(command, output / "compile.log", environment, args.timeout)
                if cell["compile"]["return_code"] != 0:
                    cell["status"] = "compile-failed"
                else:
                    ptx = (output / "code.ptx").read_text()
                    reflection = json.loads((output / "reflection.json").read_text())
                    validate_bindings(reflection, row, ptx, args.architecture)
                    cell.update(ptx_sha256=sha(ptx.encode()), binding_verified=True,
                                reflection_sha256=sha((output / "reflection.json").read_bytes()))
                    cell["assemble"] = run_command([ptxas, f"-arch=sm_{args.architecture}",
                        output / "code.ptx", "-o", output / "code.cubin"],
                        output / "assemble.log", environment, args.timeout)
                    if cell["assemble"]["return_code"] != 0:
                        cell["status"] = "assemble-failed"
                    else:
                        config = output / "device-case.json"
                        write_json(config, dict(row=row, ptx=str(output / "code.ptx"), oracle_sha256=refs))
                        cell["runtime_process"] = run_command([sys.executable, Path(__file__),
                            "--device-case", config], output / "runtime.log", environment, args.timeout)
                        runtime = output / "runtime.json"
                        if runtime.exists():
                            cell["runtime"] = json.loads(runtime.read_text())
                            cell["status"] = cell["runtime"]["status"]
                            if cell["runtime_process"]["return_code"] is None or (
                                cell["runtime_process"]["return_code"] != 0 and cell["status"] == "passed"
                            ):
                                cell["status"] = "runtime-process-failed"
                        else:
                            cell["status"] = "runtime-process-failed"
            except (OSError, ValueError, KeyError) as error:
                cell.update(status="validation-failed", error=str(error))
            write_json(args.output / "results.json", report)
            print(row["case"], mode, cell["status"], flush=True)
    report["status"] = "passed" if all(x["status"] == "passed" for x in report["cells"]) else "failed"
    try:
        require({str(path) for path in inventory_paths(args, provider)} == set(identity),
                "The runtime inventory gained or lost files during execution")
        verify_identity(identity)
        report["identity_verification"] = "unchanged"
    except (OSError, ValueError) as error:
        report.update(status="failed", identity_verification="failed", identity_error=str(error))
    report["counts"] = dict(Counter(x["status"] for x in report["cells"]))
    write_json(args.output / "results.json", report)
    return 0 if report["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
