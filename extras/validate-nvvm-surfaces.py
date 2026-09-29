#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compare physical CUDA surface bytes against independent, generated host expectations.

This bounded two-array harness uses 1D/2D Float32 or Half storage and 1/2/4 channels.
It records failures as failures, including NVRTC component compilation and formatted
store rounding differences. NaN conversions require class only; untouched bits are exact.
Run --self-test for CPU oracle/ABI/reflection contracts without a compiler or GPU.
"""
import argparse
from collections import Counter
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import time

REPO = Path(__file__).resolve().parents[1]
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
                 for name in ["matrix", "half-load", "half-nan", "boundaries"])
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
    return rows


def oracle(row):
    """Generate physical inputs, expectations and the exact converted-NaN exception positions."""
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
    expected = buffers["expected" if index == 0 else "output_expected"]
    size = 2 if index == 0 and row["half"] else 4
    require(len(actual) == len(expected), "Wrong physical readback length")
    failures, nan_bits = [], Counter()
    for i in range(len(actual) // size):
        a = int.from_bytes(actual[i * size:(i + 1) * size], "little")
        e = int.from_bytes(expected[i * size:(i + 1) * size], "little")
        if i in buffers["nan_positions"][index]:
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


def validate_host_abi():
    """Match the CUDA 64-bit Driver API structures before passing any foreign pointers."""
    require(sys.byteorder == "little" and C.sizeof(VP) == C.sizeof(SZ) == 8, "64-bit LE host required")
    require(C.sizeof(Array3DDesc) == 40 and Array3DDesc.Flags.offset == 32, "Array ABI mismatch")
    require(C.sizeof(ResourceDesc) == 144 and ResourceDesc.res.offset == 8 and
            ResourceDesc.flags.offset == 136, "Resource ABI mismatch")
    require(C.sizeof(Copy2D) == 128 and Copy2D.srcHost.offset == 24 and
            Copy2D.dstMemoryType.offset == 72 and Copy2D.dstHost.offset == 80 and
            Copy2D.WidthInBytes.offset == 112 and Copy2D.Height.offset == 120, "Copy ABI mismatch")


def run_device(ptx, row, buffers, output):
    """Initialize two physical arrays, execute once, and independently copy all bytes back."""
    validate_host_abi()
    result = dict(status="running", cleanup=[])
    def save():
        write_json(output / "runtime.json", result)
    context, module = VP(), VP()
    arrays, surfaces = [VP(), VP()], [U64(), U64()]
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
        def copy_array(array, data, bytes_per_channel, upload):
            host = C.create_string_buffer(data, len(data))
            copy = Copy2D()
            pitch = row['width'] * row['lanes'] * bytes_per_channel
            require(len(data) == pitch * row['height'], 'Host array copy extent mismatch')
            copy.WidthInBytes, copy.Height = pitch, row['height']
            if upload:
                copy.srcMemoryType, copy.srcHost, copy.srcPitch = 1, C.addressof(host), pitch
                copy.dstMemoryType, copy.dstArray = 3, array.value
            else:
                copy.srcMemoryType, copy.srcArray = 3, array.value
                copy.dstMemoryType, copy.dstHost, copy.dstPitch = 1, C.addressof(host), pitch
            check('cuMemcpy2D_v2', C.byref(copy))
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
        for i, fmt in enumerate([(0x10 if row['half'] else 0x20), 0x20]):
            desc = Array3DDesc(row['width'], row['height'] if row['shape'] == 2 else 0,
                               0, fmt, row['lanes'], 2)
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
            initial = buffers['initial' if i == 0 else 'output_initial']
            bpc = (2 if row['half'] else 4) if i == 0 else 4
            copy_array(arrays[i], initial, bpc, True)
            require(copy_array(arrays[i], bytes(len(initial)), bpc, False) == initial,
                    'Host upload did not preserve the initial array bytes')
        result['initial_host_copies_verified'] = True
        ptx_buffer = C.create_string_buffer(ptx)
        check('cuModuleLoadData', C.byref(module), ptx_buffer)
        globals_pointer, globals_size = U64(), SZ()
        check('cuModuleGetGlobal_v2', C.byref(globals_pointer), C.byref(globals_size),
              module, b'SLANG_globalParams')
        require(globals_size.value == 16, 'Loaded CUDA module has wrong global size')
        global_data = (U64 * 2)(surfaces[0].value, surfaces[1].value)
        check('cuMemcpyHtoD_v2', globals_pointer, global_data, 16)
        function = VP()
        check('cuModuleGetFunction', C.byref(function), module, row['entry'].encode())
        check('cuLaunchKernel', function, row['width'], row['height'], 1,
              1, 1, 1, 0, None, None, None)
        check('cuCtxSynchronize')
        result['launched_and_synchronized'] = True
        result['readbacks'] = []
        for i in range(2):
            key = 'expected' if i == 0 else 'output_expected'
            expected = buffers[key]
            bpc = (2 if row['half'] else 4) if i == 0 else 4
            actual = copy_array(arrays[i], bytes(len(expected)), bpc, False)
            name = 'surface' if i == 0 else 'observed'
            (output / (name + '-actual.bin')).write_bytes(actual)
            checked = compare(row, buffers, i, actual)
            checked.update(array=name, active_texels=buffers["active_texels"],
                           guard_texels=buffers["guard_texels"])
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
    """Verify each compiled entry's two named resource bindings and actual PTX launch ABI."""
    params = reflection.get("parameters", [])
    require(len(params) == 2, "Expected exactly two global surface bindings")
    expected_format = {1: "r16f", 2: "rg16f", 4: "rgba16f"}[row["lanes"]]
    for parameter, name, offset in zip(params, ["surface", "observed"], [0, 8]):
        binding, ty = parameter["binding"], parameter["type"]
        require(parameter["name"] == name and binding["kind"] == "uniform" and
                binding["offset"] == offset and binding["size"] == 8, "Surface binding mismatch")
        require(ty["kind"] == "resource" and ty["baseShape"] == f'texture{row["shape"]}D' and
                ty["access"] == "readWrite", "Surface shape/access mismatch")
        result = ty["resultType"]
        if row["lanes"] > 1:
            require(result["kind"] == "vector" and result["elementCount"] == row["lanes"],
                    "Surface lane count mismatch")
            result = result["elementType"]
        require(result["kind"] == "scalar" and result["scalarType"] == "float32",
                "Surface logical element mismatch")
        expected = expected_format if name == "surface" and row["half"] else None
        require(parameter.get("format") == expected, "Surface format annotation mismatch")
    entries = [x for x in reflection.get("entryPoints", []) if x.get("name") == row["entry"]]
    require(len(entries) == 1 and entries[0]["stage"] == "compute" and
            entries[0]["threadGroupSize"] == [1, 1, 1], "Entry reflection mismatch")
    require(re.search(r"\.entry\s+" + re.escape(row["entry"]) + r"\s*\(\s*\)", ptx),
            "PTX entry must have no explicit parameters")
    require(re.search(r"\.const\s+\.align\s+8\s+\.b8\s+SLANG_globalParams\[16\]", ptx),
            "PTX globals must contain exactly two 64-bit surface handles")
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
        for index, key in enumerate(["expected", "output_expected"]):
            require(compare(row, buffers, index, buffers[key])["mismatch_count"] == 0,
                    "Reference oracle rejected")
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
        require(all(sha(buffers[key]) == value for key, value in config["oracle_sha256"].items()),
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
        for name in ["matrix", "half-load", "half-nan", "boundaries"]}
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
                  semantic_contract="RN-even finite Half conversions; converted NaN class only; untouched bits exact",
                  requested_cells=len(rows) * len(args.modes), cells=[])
    for row in rows:
        source = REPO / "tests/cuda" / ("nvvm-surface-physical-" + row["fixture"] + ".slang")
        buffers = oracle(row)
        frozen = args.output / row["case"]
        frozen.mkdir()
        for key in ["initial", "expected", "output_initial", "output_expected"]:
            (frozen / (key + ".bin")).write_bytes(buffers[key])
        refs = {key: sha(buffers[key]) for key in ["initial", "expected", "output_initial", "output_expected"]}
        write_json(frozen / "oracle.json", dict(row=row, source_sha256=sha(source.read_bytes()),
                   file_sha256=refs, converted_nan_positions=[sorted(x) for x in buffers["nan_positions"]]))
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
