# Plan: CUDA RWTexture Format Conversion Support

## Problem Statement

CUDA's `surf2Dread`/`surf2Dwrite` (and 1D/3D variants) perform **raw byte operations** without automatic format conversion. When a shader declares `RWTexture2D<float4>` bound to an 8-bit UNORM texture, other backends (SPIRV, HLSL, Metal) rely on **hardware** to convert `[0,255]` → `[0.0,1.0]`. CUDA does not.

**Current partial implementation**: The `$C` intrinsic specifier in `slang-intrinsic-expand.cpp` can append `_convert` suffix to surface functions when format decoration differs from element type. However, only **half→float** conversions are implemented in `slang-cuda-prelude.h` (lines 1517-1625). There is no support for:
- unorm/snorm formats (rgba8, rgba16, rgba8_snorm, etc.)
- Packed formats (rgb10_a2, r11f_g11f_b10f)
- 8/16-bit integer formats

**Current `$E` bug**: The `$E` intrinsic specifier (slang-intrinsic-expand.cpp:490-515) returns `elemSizeInBytes = 1` for format-converted writes because the old `sust.p` was sample-addressed. With the move to raw byte stores, this is wrong — all writes must be byte-addressed. `rgba8` must write at `x * 4`, `rgba16f` at `x * 8`, etc.

---

## Design Principles

1. **Treat CUDA RWTexture storage as raw bytes only.** Do not rely on any formatted PTX surface ops.
2. **Do software decode on every formatted `Load`.**
3. **Do software encode on every formatted `Store`.**
4. **Remove all `sust.p` reliance** from the CUDA prelude.

All reads and writes uniformly follow this pattern:

```cpp
// Load
raw = slang_cuda_surf_read_raw<StorageRep<F>>(surf, coord * sizeof(StorageRep<F>));
value = slang_cuda_decode_texel<F, T>(raw);

// Store
raw = slang_cuda_encode_texel<F, T>(value);
slang_cuda_surf_write_raw<StorageRep<F>>(raw, surf, coord * sizeof(StorageRep<F>));
```

This means byte-coordinate scaling (`$E`) is always `sizeof(StorageRep<F>)` — for both reads and writes. The current special case that sets `$E = 1` for converted writes must be removed.

---

## Research Findings

### How Other Backends Handle Format Conversion

| Backend | Format Declaration | Where Conversion Happens |
|---------|-------------------|-------------------------|
| **SPIRV** | `OpTypeImage` with format enum | Hardware via format capability |
| **HLSL** | None in shader code | Hardware at binding time |
| **GLSL** | `layout(rgba8) uniform image2D` | Hardware via format qualifier |
| **Metal** | Texture type encodes format | Hardware via texture type |

**Key insight**: All GPU backends rely on **dedicated texture units** that perform format conversion in hardware. CUDA surfaces use generic memory hardware without this capability.

### CUDA PTX Surface Instructions

| Instruction | Description | Format Conversion |
|-------------|-------------|-------------------|
| `suld.b` | Surface load, byte-addressed | **No** - raw bits |
| ~~`suld.p`~~ | ~~Surface load, packed~~ | ❌ **Removed in PTX 3.0** |
| `sust.b` | Surface store, byte-addressed | **No** - raw bits |
| ~~`sust.p`~~ | ~~Surface store, packed~~ | ❌ **Removed in PTX 3.0** |

**Critical finding**: PTX spec states:
> "Unimplemented instructions suld.p and sust.p.{u32,s32,f32} have been removed." (PTX 3.0)

**Nuance**: Current NVIDIA docs still describe a `sust.p...b32` formatted-store form, while the above quote is about removed/unimplemented `suld.p` and `sust.p.{u32,s32,f32}` typed forms. But this distinction is not worth building correctness on. If a path depends on "formatted surface store" semantics, it is fragile. `suld` is definitely only unformatted binary load, and `surf*read/write` use byte coordinates in CUDA's own docs. Sources: [PTX surface docs](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html), [CUDA surface API docs](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html).

⚠️ The existing `sust.p` code in the prelude is the wrong foundation for correctness.
✅ **Software conversion with raw byte I/O is the ONLY reliable path.**

### Format Categories Requiring Conversion

From `slang-image-format-defs.h`:

**Phase 1 — Scalar-type-backed formats** (implement first):

1. **UNORM formats**: `rgba8`, `rgba16`, `rg8`, `rg16`, `r8`, `r16` (UINT8/UINT16 → float [0,1])
2. **SNORM formats**: `rgba8_snorm`, `rgba16_snorm`, `rg8_snorm`, `rg16_snorm`, `r8_snorm`, `r16_snorm` (INT8/INT16 → float [-1,1])
3. **Float16 formats**: `rgba16f`, `rg16f`, `r16f` (half → float)
4. **Sub-32-bit integers**: `rgba8i`, `rgba16i`, `rgba8ui`, `rgba16ui`, `rg8i`, `rg16i`, `r8i`, `r16i`, `rg8ui`, `rg16ui`, `r8ui`, `r16ui` (sign/zero extension)

**Phase 2 — Packed formats** (deferred, emit diagnostics initially):

5. **Packed formats**: `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f` (complex bit packing)

**Special case — `bgra8`**: Treat as `rgba8` read/write with b↔r channel swizzle (`.zyxw`), not a separate code path.

---

## Detailed Implementation Plan

### Overview

Implement software format conversion functions in the CUDA prelude that convert between:
- **Storage format** (how bytes are stored in surface memory, e.g., `rgba8` = 4 bytes)
- **Access type** (shader type, e.g., `float4`)

The conversion happens at load/store time via format-aware `surf*Dread_format`/`surf*Dwrite_format` functions that use raw byte I/O + software encode/decode.

---

### Step 1: Remove `sust.p` PTX Code and `_convert` Functions

**File**: [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h)

**Remove entirely**:
- All `SLANG_SURF1DWRITE_CONVERT_IMPL` / `SLANG_SURF2DWRITE_CONVERT_IMPL` / `SLANG_SURF3DWRITE_CONVERT_IMPL` macros and their instantiations (lines 1573-1817) — these emit `sust.p` PTX instructions
- All `surf*Layeredwrite_convert` stub functions (empty bodies that silently drop writes)
- The `SLANG_SURFACE_READ_HALF_CONVERT` macro and its instantiations (lines 1517-1559) — these will be superseded by the general format-aware read path

**Rationale**: The `sust.p` instructions are fragile/removed. The half-read macros are a subset of the new format-aware path and keeping them creates two parallel code paths. Clean removal first, then add the unified replacement.

---

### Step 2: Fix `$E` Byte-Address Scaling for Converted Writes

**File**: [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

**Remove the special case** in the `$E` handler (lines 500-509) that sets `elemSizeInBytes = 1` for format-converted writes:

```cpp
// REMOVE THIS BLOCK:
// If we have a format conversion and its a *write* we don't need to scale
if (IRFormatDecoration* formatDecoration = _findImageFormatDecoration(resourceInst))
{
    const ImageFormat imageFormat = formatDecoration->getFormat();
    if (_isConvertRequired(imageFormat, resourceInst) && _isResourceWrite(m_callInst))
    {
        // If there is a conversion *and* it's a write we don't need to scale.
        elemSizeInBytes = 1;
    }
}
```

**After removal**, `$E` always returns `_calcBackingElementSizeInBytes()` — the byte size of the storage format. This is correct for raw byte-addressed `surf*write` calls.

This is a **critical correctness fix**. Without it, every format-converted store will write to the wrong byte offset (e.g., `rgba8` stores at `x * 1` instead of `x * 4`).

---

### Step 3: Add Format Conversion Math Functions to Prelude

**File**: [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h)

Add conversion helper functions. Note: SNORM encode functions must use rounding (not truncation) to match GPU hardware behavior.

```cpp
// === Format enum (must match slang-image-format-defs.h order) ===
// Use SlangImageFormat from slang.h to avoid enum drift.
// If slang.h is not included in the prelude, replicate from the canonical
// SLANG_IMAGE_FORMAT_* values in include/slang.h.

// === UNORM Conversion ===
// UNORM8: [0,255] → [0.0, 1.0]
SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_unorm8_to_float(uint8_t v)
{
    return v * (1.0f / 255.0f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint8_t _slang_float_to_unorm8(float v)
{
    return (uint8_t)(__saturatef(v) * 255.0f + 0.5f);
}

// UNORM16: [0,65535] → [0.0, 1.0]
SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_unorm16_to_float(uint16_t v)
{
    return v * (1.0f / 65535.0f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint16_t _slang_float_to_unorm16(float v)
{
    return (uint16_t)(__saturatef(v) * 65535.0f + 0.5f);
}

// === SNORM Conversion ===
// SNORM8: [-128,127] → [-1.0, 1.0], with -128 and -127 both mapping to -1.0
SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_snorm8_to_float(int8_t v)
{
    return fmaxf(v * (1.0f / 127.0f), -1.0f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int8_t _slang_float_to_snorm8(float v)
{
    // Use roundf() to avoid truncation bias toward zero
    return (int8_t)roundf(fminf(fmaxf(v, -1.0f), 1.0f) * 127.0f);
}

// SNORM16: [-32768,32767] → [-1.0, 1.0]
SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_snorm16_to_float(int16_t v)
{
    return fmaxf(v * (1.0f / 32767.0f), -1.0f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int16_t _slang_float_to_snorm16(float v)
{
    // Use roundf() to avoid truncation bias toward zero
    return (int16_t)roundf(fminf(fmaxf(v, -1.0f), 1.0f) * 32767.0f);
}

// === Half Conversion ===
SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_half_to_float(uint16_t v)
{
    return __half2float(__ushort_as_half(v));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint16_t _slang_float_to_half(float v)
{
    return __half_as_ushort(__float2half(v));
}

// === BGRA8 ===
// Handled as rgba8 unorm with b↔r swizzle (.zyxw), not a separate code path.
```

**Note on `__saturatef()`**: This is a CUDA device intrinsic. Verify it's available in all prelude compilation modes (device code only — which is the only context where surface functions are called, so this is fine).

---

### Step 4: Add Format-Aware Surface Read/Write Functions

**File**: [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h)

Use switch-based dispatch by format enum. nvcc will constant-fold the switch since the format value is a compile-time constant emitted by `$F`.

Both reads and writes use raw byte I/O (`surf*Dread<RawT>` / `surf*Dwrite<RawT>`) — no `sust.p`, no formatted PTX ops.

```cpp
// Format-converting 2D surface read (representative example)
template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T surf2Dread_format(
    cudaSurfaceObject_t surfObj,
    int x, int y,
    SlangImageFormat format,
    cudaSurfaceBoundaryMode boundaryMode);

// Specialization for float4 access
template<>
SLANG_FORCE_INLINE SLANG_CUDA_CALL float4 surf2Dread_format<float4>(
    cudaSurfaceObject_t surfObj,
    int x, int y,
    SlangImageFormat format,
    cudaSurfaceBoundaryMode boundaryMode)
{
    switch (format)
    {
    case SLANG_IMAGE_FORMAT_rgba8:
        {
            uchar4 raw = surf2Dread<uchar4>(surfObj, x, y, boundaryMode);
            return float4{
                _slang_unorm8_to_float(raw.x),
                _slang_unorm8_to_float(raw.y),
                _slang_unorm8_to_float(raw.z),
                _slang_unorm8_to_float(raw.w)
            };
        }
    case SLANG_IMAGE_FORMAT_bgra8:
        {
            // Same as rgba8 but with b↔r swizzle
            uchar4 raw = surf2Dread<uchar4>(surfObj, x, y, boundaryMode);
            return float4{
                _slang_unorm8_to_float(raw.z),  // b → r
                _slang_unorm8_to_float(raw.y),
                _slang_unorm8_to_float(raw.x),  // r → b
                _slang_unorm8_to_float(raw.w)
            };
        }
    case SLANG_IMAGE_FORMAT_rgba8_snorm:
        {
            char4 raw = surf2Dread<char4>(surfObj, x, y, boundaryMode);
            return float4{
                _slang_snorm8_to_float(raw.x),
                _slang_snorm8_to_float(raw.y),
                _slang_snorm8_to_float(raw.z),
                _slang_snorm8_to_float(raw.w)
            };
        }
    case SLANG_IMAGE_FORMAT_rgba16f:
        {
            ushort4 raw = surf2Dread<ushort4>(surfObj, x, y, boundaryMode);
            return float4{
                _slang_half_to_float(raw.x),
                _slang_half_to_float(raw.y),
                _slang_half_to_float(raw.z),
                _slang_half_to_float(raw.w)
            };
        }
    // ... more formats (rgba16 unorm/snorm, rg*/r* variants, integer formats)
    default:
        return surf2Dread<float4>(surfObj, x, y, boundaryMode);
    }
}

// Format-converting 2D surface write (representative example)
template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_format(
    T val,
    cudaSurfaceObject_t surfObj,
    int x, int y,
    SlangImageFormat format,
    cudaSurfaceBoundaryMode boundaryMode);

template<>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_format<float4>(
    float4 val,
    cudaSurfaceObject_t surfObj,
    int x, int y,
    SlangImageFormat format,
    cudaSurfaceBoundaryMode boundaryMode)
{
    switch (format)
    {
    case SLANG_IMAGE_FORMAT_rgba8:
        {
            uchar4 raw = {
                _slang_float_to_unorm8(val.x),
                _slang_float_to_unorm8(val.y),
                _slang_float_to_unorm8(val.z),
                _slang_float_to_unorm8(val.w)
            };
            surf2Dwrite(raw, surfObj, x, y, boundaryMode);
            break;
        }
    case SLANG_IMAGE_FORMAT_bgra8:
        {
            uchar4 raw = {
                _slang_float_to_unorm8(val.z),  // b
                _slang_float_to_unorm8(val.y),
                _slang_float_to_unorm8(val.x),  // r
                _slang_float_to_unorm8(val.w)
            };
            surf2Dwrite(raw, surfObj, x, y, boundaryMode);
            break;
        }
    // ... more formats
    default:
        surf2Dwrite(val, surfObj, x, y, boundaryMode);
        break;
    }
}
```

**Surface dimension coverage**: Implement `_format` variants for all 7 surface function types:
- `surf1Dread_format` / `surf1Dwrite_format`
- `surf2Dread_format` / `surf2Dwrite_format`
- `surf3Dread_format` / `surf3Dwrite_format`
- `surf1DLayeredread_format` / `surf1DLayeredwrite_format`
- `surf2DLayeredread_format` / `surf2DLayeredwrite_format`
- `surfCubemapread_format` / `surfCubemapwrite_format`
- `surfCubemapLayeredread_format` / `surfCubemapLayeredwrite_format`

**Layered textures now work**: Since we use raw `surf*Layeredwrite` (byte-addressed) + software encode, layered writes are no longer blocked. Remove the old empty stub functions.

**Access type specializations needed**: `float`, `float2`, `float4`, `int`, `int2`, `int4`, `uint`, `uint2`, `uint4` (matching current `SLANG_SURFACE_READ`/`SLANG_SURFACE_WRITE` macro coverage). Use macros to reduce boilerplate across dimensions.

---

### Step 5: Modify Intrinsic Expansion — `$C` and new `$F`

**File**: [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

The main compiler change: the format-aware prelude functions need the actual image format, not just a `_convert` flag. The current `$C` mechanism only says "conversion needed" and appends `_convert`; it does not select `rgba8` vs `rg16f` vs `r8_snorm`.

**Changes to `$C` specifier** (line 442):

Current behavior: Appends `_convert` suffix
New behavior: Appends `_format` suffix AND stores format for `$F` to emit

```cpp
case 'C':
{
    if (isCUDATarget(m_emitter->getTargetReq()))
    {
        IRInst* resourceInst = m_callInst->getArg(0);
        if (IRFormatDecoration* formatDecoration = _findImageFormatDecoration(resourceInst))
        {
            const ImageFormat imageFormat = formatDecoration->getFormat();
            if (_isConvertRequired(imageFormat, resourceInst))
            {
                // If reading a half-derived format, still require half extension
                if (_isResourceRead(m_callInst))
                {
                    switch (imageFormat)
                    {
                    case ImageFormat::r16f:
                    case ImageFormat::rg16f:
                    case ImageFormat::rgba16f:
                        {
                            CUDAExtensionTracker* extensionTracker =
                                as<CUDAExtensionTracker>(m_emitter->getExtensionTracker());
                            if (extensionTracker)
                                extensionTracker->requireBaseType(BaseType::Half);
                            break;
                        }
                    default: break;
                    }
                }

                // Append _format suffix (replaces old _convert)
                m_writer->emit("_format");
                // Store format for $F to emit later
                m_pendingFormat = imageFormat;
                m_hasFormatConversion = true;
            }
        }
    }
    break;
}
```

**Add new `$F` specifier** to emit the format enum value:

```cpp
case 'F':
{
    // Emit format enum value as extra argument for format-converting functions.
    // When no conversion is needed, $F emits nothing (function has no format param).
    if (m_hasFormatConversion)
    {
        m_writer->emit("(SlangImageFormat)");
        m_writer->emitUInt64((uint64_t)m_pendingFormat);
    }
    break;
}
```

Using `(SlangImageFormat)N` (cast of integer literal) rather than `SLANG_IMAGE_FORMAT_name` avoids string table lookups and is immune to naming mismatches between the enum and `getImageFormatInfo().name`.

**State tracking**: Add `m_pendingFormat` (ImageFormat) and `m_hasFormatConversion` (bool) as member variables to the intrinsic expander class, initialized to `ImageFormat::unknown` / `false` at the start of each expansion.

---

### Step 6: Update hlsl.meta.slang CUDA Intrinsics

**File**: [source/slang/hlsl.meta.slang](source/slang/hlsl.meta.slang)

The intrinsic asm strings need the `$F` format argument added for format-converting functions. The challenge: when no conversion is needed, the function is `surf2Dread<T>(...)` with no format argument; when conversion is needed, it becomes `surf2Dread_format<T>(..., FORMAT, ...)`.

**Approach**: Have `$F` emit `, (SlangImageFormat)N` (with leading comma) when conversion is active, and emit nothing when inactive. This way the intrinsic string doesn't change shape:

**Current** (line ~4905):
```slang
case $(SLANG_TEXTURE_2D):
    __intrinsic_asm "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
```

**New**:
```slang
case $(SLANG_TEXTURE_2D):
    __intrinsic_asm "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y$F, SLANG_CUDA_BOUNDARY_MODE)";
```

Where `$F` expands to:
- `, (SlangImageFormat)10` when format conversion is needed (e.g., format enum value 10 = rgba8)
- Empty string when no conversion needed

This approach keeps a single intrinsic definition per dimension and avoids splitting into converting/non-converting variants.

**Apply the same pattern to all write intrinsics** (line ~5075):
```slang
// Current:
__intrinsic_asm "surf2Dwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
// New:
__intrinsic_asm "surf2Dwrite$C<$T0>($2, $0, ($1).x * $E, ($1).y$F, SLANG_CUDA_BOUNDARY_MODE)";
```

---

### Step 7: Add Slang Tests

**New file**: `tests/cuda/cuda-texture-format-conversion.slang`

```slang
//TEST(compute):COMPARE_COMPUTE:-cuda -compute -output-using-type

// Test UNORM8 format conversion
//TEST_INPUT:RWTexture2D(format=RGBA8, size=4, content=one):name=tex_unorm8
[format("rgba8")]
RWTexture2D<float4> tex_unorm8;

// Test SNORM8 format conversion
//TEST_INPUT:RWTexture2D(format=RGBA8SNorm, size=4, content=zero):name=tex_snorm8
[format("rgba8_snorm")]
RWTexture2D<float4> tex_snorm8;

// Test FLOAT16 format conversion
//TEST_INPUT:RWTexture2D(format=RGBA16Float, size=4, content=one):name=tex_f16
[format("rgba16f")]
RWTexture2D<float4> tex_f16;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Test Load from UNORM8 texture (should return ~1.0 for content=one)
    float4 v0 = tex_unorm8.Load(int2(0, 0));
    outputBuffer[0] = v0.x;  // CHECK: 1.0

    // Test Store to UNORM8 texture, then Load back
    tex_unorm8.Store(int2(1, 0), float4(0.5, 0.25, 0.75, 1.0));
    float4 v1 = tex_unorm8.Load(int2(1, 0));
    outputBuffer[1] = v1.x;  // CHECK: ~0.498 (127/255)
    outputBuffer[2] = v1.y;  // CHECK: ~0.247 (63/255)

    // Test SNORM8 Store and Load
    tex_snorm8.Store(int2(0, 0), float4(-0.5, 0.5, -1.0, 1.0));
    float4 v2 = tex_snorm8.Load(int2(0, 0));
    outputBuffer[3] = v2.x;  // CHECK: ~-0.496
    outputBuffer[4] = v2.z;  // CHECK: -1.0

    // Test FLOAT16 round-trip
    tex_f16.Store(int2(0, 0), float4(0.123, 456.789, -0.001, 65504.0));
    float4 v3 = tex_f16.Load(int2(0, 0));
    outputBuffer[5] = v3.x;  // CHECK: ~0.123
}
```

**Additional test files**:
- `tests/cuda/cuda-texture-format-1d.slang` — 1D texture format conversion
- `tests/cuda/cuda-texture-format-3d.slang` — 3D texture format conversion
- `tests/cuda/cuda-texture-format-layered.slang` — Layered texture format conversion (now supported)
- `tests/cuda/cuda-texture-format-integer.slang` — 8/16-bit integer formats

**Negative test**: Assert generated CUDA code contains no `sust.p`:
```slang
// Compile to CUDA source, grep output for sust.p — should not appear
```

---

### Step 8: Add Diagnostic for Unsupported Packed Formats

**New file**: `tests/diagnostics/cuda-texture-unsupported-format.slang`

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda

[format("r11f_g11f_b10f")]
RWTexture2D<float3> tex;
// CHECK: error

void main() {
    tex.Store(int2(0,0), float3(1,2,3));
}
```

Emit explicit diagnostics for these formats until bespoke packing is implemented:
- `r11f_g11f_b10f`
- `rgb10_a2`
- `rgb10_a2ui`

---

## Files to Modify Summary

| File | Changes |
|------|---------|
| [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h) | Remove `sust.p` code and `_convert` functions; add conversion math; add `_format` surface functions |
| [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp) | Remove `$E` scaling special-case for converted writes; modify `$C` to emit `_format`; add `$F` specifier |
| [source/slang/hlsl.meta.slang](source/slang/hlsl.meta.slang) | Add `$F` to CUDA intrinsic strings |
| `tests/cuda/cuda-texture-format-*.slang` | New test files |
| `tests/diagnostics/cuda-texture-unsupported-format.slang` | Diagnostic test for packed formats |

---

## Implementation Order

1. **Remove `$E` scaling special-case** for converted writes — restore byte addressing for all surface access. This is a correctness fix independent of everything else.

2. **Remove all `sust.p` code and `_convert` functions** from the prelude. Remove `SLANG_SURFACE_READ_HALF_CONVERT` macro. Remove empty layered write stubs.

3. **Add conversion math functions** to prelude:
   - UNORM8/16, SNORM8/16 (with correct rounding), Half↔Float converters
   - Verify `__saturatef()` availability in device code

4. **Add format-aware surface functions** to prelude:
   - `surf*read_format<T>()` / `surf*write_format<T>()` with switch on `SlangImageFormat`
   - Cover all 7 surface dimensions (1D, 2D, 3D, 1DLayered, 2DLayered, Cubemap, CubemapLayered)
   - Start with `float4` + `rgba8`, expand to all scalar-type-backed formats

5. **Modify intrinsic expansion**:
   - Update `$C` to emit `_format` suffix and store format
   - Add `$F` specifier to emit format enum value with leading comma
   - Add `m_pendingFormat` / `m_hasFormatConversion` state to expander class

6. **Update hlsl.meta.slang**:
   - Add `$F` to all CUDA read/write intrinsic asm strings

7. **Add tests**:
   - Numeric round-trip tests for rgba8, rgba8_snorm, rgba16f, integer formats
   - Multi-dimension tests (1D, 2D, 3D, layered)
   - Negative test asserting no `sust.p` in generated CUDA source
   - Diagnostic test for unsupported packed formats

8. **Expand format coverage** (Phase 2):
   - Implement packed format codecs: `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f`
   - Remove corresponding diagnostics once implemented

---

## Verification Checklist

- [ ] UNORM8 load returns correct [0,1] float values
- [ ] UNORM8 store+load round-trips correctly (within 1/255 tolerance)
- [ ] SNORM8 handles -1.0 correctly (both -127 and -128 map to -1.0)
- [ ] SNORM encode uses rounding (not truncation)
- [ ] Float16 round-trips preserve precision
- [ ] BGRA8 swizzles b↔r channels correctly
- [ ] All 1D/2D/3D variants work
- [ ] Layered texture reads and writes work
- [ ] Byte-address scaling (`$E`) is correct for both reads AND writes
- [ ] Generated CUDA code contains NO `sust.p` instructions
- [ ] Unsupported packed formats emit explicit diagnostics
- [ ] No regressions in existing CUDA tests
- [ ] Generated CUDA code compiles with nvcc
- [ ] `SlangImageFormat` enum in prelude matches `slang.h` canonical values

---

## Resolved Questions

1. ~~**Existing `sust.p` code**~~: **Remove it entirely.** Do not keep behind version guard. Relying on formatted surface store semantics is fragile regardless of PTX version.
2. **TEST_INPUT format syntax**: Need to verify `RWTexture2D(format=RGBA8, ...)` is correct test input syntax. **Blocker for Step 7** — verify before writing tests.
3. ~~**BGRA format**~~: Handle as `rgba8` with b↔r swizzle in the same switch case. No separate code path.
4. ~~**Layered textures**~~: **Now fully supported.** Raw byte I/O works identically for layered and non-layered surfaces once the texel is software-encoded/decoded.
