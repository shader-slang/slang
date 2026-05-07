# Plan: CUDA RWTexture Format Conversion Support

## Problem Statement

CUDA's `surf2Dread`/`surf2Dwrite` (and 1D/3D variants) perform **raw byte operations** without automatic format conversion. When a shader declares `RWTexture2D<float4>` bound to an 8-bit UNORM texture, other backends (SPIRV, HLSL, Metal) rely on **hardware** to convert `[0,255]` → `[0.0,1.0]`. CUDA does not.

**Current partial implementation**: The `$C` intrinsic specifier in `slang-intrinsic-expand.cpp` can append `_convert` suffix to surface functions when format decoration differs from element type. However, only **half→float** conversions are implemented in `slang-cuda-prelude.h` (lines 1517-1625). There is no support for:
- unorm/snorm formats (rgba8, rgba16, rgba8_snorm, etc.)
- Packed formats (rgb10_a2, r11f_g11f_b10f)
- 8/16-bit integer formats

**Current `$E` bug**: The `$E` intrinsic specifier (slang-intrinsic-expand.cpp:490-515) returns `elemSizeInBytes = 1` for format-converted writes because the old `sust.p` was sample-addressed. With raw byte stores, this is wrong — all writes must be byte-addressed. `rgba8` must write at `x * 4`, `rgba16f` at `x * 8`, etc.

---

## Design Principles

1. **Treat CUDA RWTexture storage as raw bytes only.** Do not rely on any formatted PTX surface ops.
2. **Do software decode on every formatted `Load`.**
3. **Do software encode on every formatted `Store`.**
4. **Remove all `sust.p` reliance** from the CUDA prelude.
5. **Perform format conversion as an IR lowering pass**, not via prelude templates or emit-time hacks.

The compiler philosophy from CLAUDE.md:
> "Keep emission simple and do heavy transforms in IR passes."

---

## Approach: IR Lowering Pass

### Why Not a Prelude-Based Approach

The previous plan used switch-dispatched template specializations in the CUDA prelude. This requires **7 surface dimensions × ~9 access types × 2 directions = ~126 specializations**, each with a ~15-case format switch — easily 2000+ lines of prelude code. Every new format means touching every specialization. The `$C`/`$F` specifier mechanism adds complexity to intrinsic expansion.

### Why an IR Pass Is Better

An IR pass intercepts texture load/store at the IR level and lowers them to raw-typed access + inline conversion arithmetic **before** code emission. This eliminates:

- **All prelude format functions** — conversion math becomes inline IR arithmetic at each call site
- **The `$C`/`$F` specifier mechanism** — the emitter sees raw-typed access with no format mismatch
- **The `$E` scaling bug** — the lowered IR uses the storage type, so byte sizing is naturally correct
- **Dimension-specific specializations** — the surface dimension is orthogonal to format conversion; the pass handles it once

| | Prelude approach | IR lowering pass |
|---|---|---|
| Code volume | ~2000+ lines of templates | ~400-600 lines in one pass file |
| Adding a format | Touch every specialization | Add one case in one function |
| Debuggability | Read emitted CUDA source | `-dump-ir` shows conversion inline |
| Dimensions | Must handle each variant separately | Orthogonal — handled once |
| Emit complexity | Needs `$C`/`$F` specifier hacks | No emit changes |
| Layered support | Must wire up each variant | Free — same IR transform |

### Architectural Constraint: CUDA Bypasses `IRImageLoad`/`IRImageStore`

A critical discovery during research: **For CUDA, `Load()`/`Store()` do NOT produce `kIROp_ImageLoad`/`kIROp_ImageStore` instructions.** Instead, they use `__intrinsic_asm` strings that expand directly during code emission via `$C`/`$E` specifiers. This differs from SPIRV/GLSL/Metal which use `IRImageLoad`/`IRImageStore` and handle them in their emitters.

The CUDA IR for `tex.Load(coord)` contains an `IRCall` to an intrinsic function with an `__intrinsic_asm` body — it never becomes an `IRImageLoad`.

This means a naive IR pass that looks for `kIROp_ImageLoad`/`kIROp_ImageStore` would see nothing for CUDA.

**Two sub-approaches to handle this:**

#### Approach A: Rewrite CUDA `Load()`/`Store()` to Use `IRImageLoad`/`IRImageStore`

Change `hlsl.meta.slang` so the CUDA path goes through `kIROp_ImageLoad`/`kIROp_ImageStore` like other backends, then add a CUDA legalization pass that lowers these to `surf*read`/`surf*write` calls with format conversion inlined.

**Pros**: Unifies the IR representation across all backends. The existing `legalizeImageSubscript` pass (currently SPIRV/GLSL/Metal-only) could be extended to cover CUDA.
**Cons**: Larger refactor of the CUDA codegen path. Must add `kIROp_ImageLoad`/`kIROp_ImageStore` handling to the CUDA emitter or fully lower them before emission. Risk of regressions in non-format-conversion cases.

#### Approach B: IR Pass That Intercepts Intrinsic Calls

Add an IR pass that runs before CUDA emission, identifies intrinsic calls to surface read/write functions on textures with format decorations, and rewrites them:
1. Changes the texture's element type to the raw storage type
2. Inserts encode/decode arithmetic around the call
3. Removes the format decoration so `$C` doesn't fire

**Pros**: Smaller, more targeted change. Doesn't disturb the existing CUDA codegen path for non-converting cases.
**Cons**: Must pattern-match on intrinsic call structure rather than clean IR opcodes.

**Recommendation**: **Approach B** for Phase 1. It's lower risk and delivers the same correctness. Approach A is a nice-to-have cleanup that can happen later.

---

## Research Findings

### How Other Backends Handle Format Conversion

| Backend | Format Declaration | Where Conversion Happens |
|---------|-------------------|-------------------------|
| **SPIRV** | `OpTypeImage` with format enum | Hardware via format capability |
| **HLSL** | None in shader code | Hardware at binding time |
| **GLSL** | `layout(rgba8) uniform image2D` | Hardware via format qualifier |
| **Metal** | Texture type encodes format | Hardware via texture type |
| **WGSL** | `texture_storage_2d<rgba8unorm, read_write>` | Hardware via storage texel format; limited format set (only `rgba16float` and a few others support read-write per `wgslFormatSupportsReadWrite()` in `slang-emit-wgsl.cpp`) |

**Key insight**: All GPU backends rely on **dedicated texture units** that perform format conversion in hardware. CUDA surfaces use generic memory hardware without this capability.

**Note**: `resolveTextureFormat` at `slang-emit.cpp:1905` runs for GLSL, SPIRV, and WGSL (not `SPIRVAssembly` — handled by SPIRV emitter directly; not CUDA/HLSL/Metal).

### CUDA PTX Surface Instructions

| Instruction | Description | Format Conversion |
|-------------|-------------|-------------------|
| `suld.b` | Surface load, byte-addressed | **No** - raw bits |
| ~~`suld.p`~~ | ~~Surface load, packed~~ | ❌ **Removed in PTX 3.0** |
| `sust.b` | Surface store, byte-addressed | **No** - raw bits |
| ~~`sust.p`~~ | ~~Surface store, packed~~ | ❌ **Removed in PTX 3.0** |

**Nuance**: Current NVIDIA docs still describe a `sust.p...b32` formatted-store form, while the removed quote is about `suld.p` and `sust.p.{u32,s32,f32}` typed forms. But this distinction is not worth building correctness on. `suld` is definitely only unformatted binary load, and `surf*read/write` use byte coordinates in CUDA's own docs.

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

### Existing IR Infrastructure

Key IR components that the pass will interact with:

| Component | Location | Role |
|-----------|----------|------|
| `IRImageLoad` / `IRImageStore` | [slang-ir-insts.h:1878-1903](source/slang/slang-ir-insts.h) | NOT used for CUDA (only SPIRV/GLSL/Metal) |
| `IRTextureTypeBase` | [slang-ir.h:1444](source/slang/slang-ir.h) | Texture type with operand 8 = format |
| `IRFormatDecoration` | [slang-ir-insts.h:476](source/slang/slang-ir-insts.h) | Format attached to texture vars |
| `resolveTextureFormat` pass | [slang-ir-resolve-texture-format.cpp](source/slang/slang-ir-resolve-texture-format.cpp) | Propagates format from `IRFormatDecoration` to texture type's format operand; runs for GLSL/SPIRV/WGSL only (not `SPIRVAssembly` — handled by SPIRV emitter directly; not CUDA/HLSL/Metal) |
| `legalizeImageSubscript` pass | [slang-ir-legalize-image-subscript.cpp](source/slang/slang-ir-legalize-image-subscript.cpp) | Lowers `ImageSubscript` → `ImageLoad`/`ImageStore`; Metal/GLSL/SPIRV only |
| `$C` / `$E` specifiers | [slang-intrinsic-expand.cpp:442-520](source/slang/slang-intrinsic-expand.cpp) | Emit-time format conversion for CUDA |
| CUDA `Load()` / `Store()` | [hlsl.meta.slang:4862-5110](source/slang/hlsl.meta.slang) | `__intrinsic_asm` strings → direct text emission |

### Current CUDA Flow (Before This Change)

```
1. AST: [format("rgba8")] RWTexture2D<float4> tex;
2. Lower-to-IR: addFormatDecoration(var, rgba8)
3. IR: IRVar with IRFormatDecoration(rgba8) + IRTextureType(elementType=float4, format=unknown)
4. resolveTextureFormat: SKIPPED for CUDA
5. CUDA emission: tex.Load(coord) →
   - Intrinsic expansion of "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y, ...)"
   - $C: detects format mismatch → emits "_convert"
   - $E: computes backing element size (but returns 1 for writes — BUG)
   - Final: "surf2Dread_convert<float4>(surfObj, coord.x * 4, coord.y, ...)"
6. Prelude: surf2Dread_convert<float4> handles ONLY half→float
```

---

## Detailed Implementation Plan

### Step 1: Remove `sust.p` PTX Code and `_convert` Functions from Prelude

**File**: [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h)

**Remove entirely**:
- All `SLANG_SURF1DWRITE_CONVERT_IMPL` / `SLANG_SURF2DWRITE_CONVERT_IMPL` / `SLANG_SURF3DWRITE_CONVERT_IMPL` macros and their instantiations (lines 1573-1817) — these emit `sust.p` PTX instructions
- All `surf*Layeredwrite_convert` stub functions (empty bodies that **silently drop writes** — this is a **pre-existing data loss bug**: any CUDA shader using format-converted writes on 1D/2D array textures has its writes silently discarded today). The IR pass fixes this because it is dimension-agnostic — layered format conversion works automatically.
- The `SLANG_SURFACE_READ_HALF_CONVERT` macro and its instantiations (lines 1517-1559) — superseded by the IR pass

---

### Step 2: Create the IR Lowering Pass — `slang-ir-legalize-cuda-surface-format.cpp`

**New file**: `source/slang/slang-ir-legalize-cuda-surface-format.cpp`

This is the core of the approach. The pass:
1. Scans all functions for calls to intrinsic functions whose `__intrinsic_asm` body matches `surf*read$C` or `surf*write$C` patterns
2. For each such call, checks if the texture argument has an `IRFormatDecoration`
3. If format conversion is needed (format's storage type ≠ element type), rewrites the call:

#### For Reads (`surf*Dread`):

```
BEFORE:
  %result = call surf2Dread_intrinsic(%tex, %coord) : float4  // format=rgba8

AFTER:
  // 1. Change texture element type to raw storage type (uchar4 for rgba8)
  // 2. Read raw storage bytes
  %raw = call surf2Dread_intrinsic(%tex_raw, %coord_scaled) : uchar4

  // 3. Convert each channel: uitofp + multiply by 1/255.0
  %r_uint = extractElement(%raw, 0) : uint8
  %r_f    = uitofp(%r_uint) : float
  %r      = mul(%r_f, 1.0/255.0) : float
  // ... repeat for g, b, a
  %result = makeVector(%r, %g, %b, %a) : float4
```

#### For Writes (`surf*Dwrite`):

```
BEFORE:
  call surf2Dwrite_intrinsic(%value, %tex, %coord) : void  // format=rgba8, value:float4

AFTER:
  // 1. Convert each channel: saturate + multiply by 255 + round + fptoui
  %r_sat   = call __saturatef(%value.x) : float
  %r_scaled= mul(%r_sat, 255.0) : float
  %r_round = add(%r_scaled, 0.5) : float
  %r_uint  = fptoui(%r_round) : uint8
  // ... repeat for g, b, a
  %raw     = makeVector(%r_uint, %g_uint, %b_uint, %a_uint) : uchar4

  // 2. Write raw bytes
  call surf2Dwrite_intrinsic(%raw, %tex_raw, %coord_scaled) : void
```

#### Key Implementation Details

**Identifying surface intrinsic calls**: The pass needs to identify which `IRCall` instructions are surface read/write intrinsics. Options:
- **Pattern match the `__intrinsic_asm` string** for `surf*read` / `surf*write` patterns — fragile but works
- **Check the callee's decoration** — intrinsic functions carry decorations that identify them
- **Best approach**: Look at the call's first argument type. If it's an `IRTextureTypeBase` and the texture has a format that requires conversion, and the call is to an intrinsic function, it's a candidate. Then check if the call has a non-void return type (read) or void (write).

**Format decoration lookup** (**critical**): The existing `_findImageFormatDecoration()` in `slang-intrinsic-expand.cpp` (lines 132-152) traverses `IRLoad` → `IRFieldAddress` → field key chains to find decorations on struct fields. For example, `myStruct.tex.Load(coord)` produces an `IRLoad` of an `IRFieldAddress`, and the format decoration lives on the field key, not on the `IRLoad` result. **The IR pass must replicate this traversal**, or it will miss format decorations on textures accessed as struct members. The recommended approach is to extract `_findImageFormatDecoration()` into a shared utility in `slang-ir-util.h` and call it from both `slang-intrinsic-expand.cpp` and the new pass.

**Format source duality** (**critical**): The effective format must be resolved from **two sources**:
- **`IRFormatDecoration`** on the texture variable (via the traversal above)
- **Texture type's format operand** (`IRTextureTypeBase::getFormat()`, operand index 8) — this is set when the user writes `RWTexture2D<unorm float4>` without an explicit `[format]` attribute; the format is inferred during semantic checking and baked into the type.

Since `resolveTextureFormat` does NOT run for CUDA, these two sources may not be reconciled. The decoration (if present) takes priority; otherwise fall back to the type's format operand. The pass must check both.

**Modifying the texture type**: The pass must create a new `IRTextureType` with the storage element type (e.g., `uchar4` for `rgba8`) instead of the access type (e.g., `float4`). The format decoration is removed after lowering so `$C` doesn't fire during emission.

**Coordinate scaling** (**critical invariant**): The raw-typed call must use correct byte-addressing. `$E` first checks for an `IRFormatDecoration` and, if found, uses `getImageFormatInfo(format).sizeInBytes`. If no decoration is found, it falls back to computing size from the element type via `_calcBackingElementSizeInBytes()` (slang-intrinsic-expand.cpp:196-220). **After the IR pass removes the format decoration AND rewrites the element type to the raw storage type, `$E` falls through to the element-type path.** This produces the correct byte size — `sizeof(uchar4) = 4` for `rgba8`, `sizeof(uint8_t) = 1` for `r8`, etc. **The element type rewrite is therefore load-bearing for `$E` correctness, not just `$C`.** If the pass removes the decoration but fails to rewrite the element type (e.g., leaves it as `float4` for `r8`), `$E` would compute `sizeof(float4) = 16` instead of the correct `1`, causing incorrect address scaling. **The pass MUST rewrite the element type before removing the format decoration.**

**Removing format decoration**: After lowering, remove the `IRFormatDecoration` from the texture variable. This ensures `$C` sees no format mismatch and emits the plain function name (no `_convert` suffix). `$E` computes the correct byte size from the raw type.

#### Format Conversion Functions (generated as IR)

For each format category, the pass generates inline IR instructions:

| Format | Read Conversion (storage → access) | Write Conversion (access → storage) |
|--------|-------------------------------------|--------------------------------------|
| **UNORM8** | `uitofp(raw) * (1.0/255.0)` | `fptoui(saturate(val) * 255.0 + 0.5)` |
| **UNORM16** | `uitofp(raw) * (1.0/65535.0)` | `fptoui(saturate(val) * 65535.0 + 0.5)` |
| **SNORM8** | `max(sitofp(raw) * (1.0/127.0), -1.0)` | `fptosi(round(clamp(val,-1,1) * 127.0))` |
| **SNORM16** | `max(sitofp(raw) * (1.0/32767.0), -1.0)` | `fptosi(round(clamp(val,-1,1) * 32767.0))` |
| **FLOAT16** | `half_to_float(bitcast<half>(raw))` | `bitcast<uint16>(float_to_half(val))` |
| **INT8/16** | `sext(raw)` to int32 | `trunc(val)` to int8/16 |
| **UINT8/16** | `zext(raw)` to uint32 | `trunc(val)` to uint8/16 |
| **BGRA8** | Same as UNORM8 with `.zyxw` swizzle | Same as UNORM8 with `.zyxw` swizzle |

**SNORM convention**: The SNORM decode formula uses the OpenGL/Vulkan convention where both -128 and -127 map to -1.0 (via the `max(..., -1.0)` clamp). This matches the Vulkan spec (§16.1.3) and OpenGL spec (§2.3.5). DirectX uses the same formula. The encode formula `fptosi(round(clamp(val,-1,1) * 127.0))` produces -127 for input -1.0, never -128, which is correct per both conventions. The asymmetry (128 negative vs 127 positive representable values) is intentional — -128 is treated as an alias for -1.0 on decode but is never produced by encode.

The pass builds these as IR instruction sequences using `IRBuilder`. This is standard practice for IR passes in Slang — see `slang-ir-legalize-image-subscript.cpp` for a similar pattern.

#### Channel Count Mismatch Handling

The format's channel count may differ from the access type's vector width. For example, `[format("r8")] RWTexture2D<float4>` has a 1-channel format but a 4-component access type. The existing `_isConvertRequired()` already flags this as requiring conversion (via `_isImageFormatCompatible` checking `numElems != imageFormatInfo.channelCount`).

The IR pass must handle this explicitly in `emitDecode()` and `emitEncode()`:

**Reads (fewer storage channels → more access channels)**:
- Read `N` raw channels (where `N` = format channel count)
- Convert each raw channel to the access scalar type
- Pad remaining channels with GPU-standard defaults: **(0, 0, 0, 1)**
  - Missing R/G/B channels → `0.0` (float) / `0` (int)
  - Missing A channel → `1.0` (float) / `1` (int for UNORM/SNORM) / max value (for UINT)
- Example: `r8` → `float4` produces `(uitofp(raw)/255.0, 0.0, 0.0, 1.0)`

**Writes (more access channels → fewer storage channels)**:
- Extract only the first `N` channels from the access value
- Convert and write `N` raw channels; remaining access channels are discarded
- Example: `float4` → `r8` writes only `fptoui(saturate(val.x) * 255.0 + 0.5)`

**Raw storage type**: When the format has fewer channels than the access type, the raw storage type is a vector of `N` elements (or scalar if `N=1`), not a vector matching the access width. For `r8` the raw type is `uint8_t` (scalar), for `rg8` it is `uchar2`, etc.

#### Pass Placement in Pipeline

**File**: [source/slang/slang-emit.cpp](source/slang/slang-emit.cpp)

Add the pass in the CUDA-specific legalization section (around line 1960):

```cpp
case CodeGenTarget::CUDASource:
case CodeGenTarget::CUDAHeader:
    {
        SLANG_PASS(legalizeCUDASurfaceFormat, codeGenContext->getSink());
        SLANG_PASS(legalizeEntryPointVaryingParamsForCUDA, codeGenContext->getSink());
    }
    break;
```

The pass must run:
- **After** format decorations are attached (lower-to-IR)
- **After** generic specialization (so texture types are concrete)
- **Before** CUDA emission (so the emitter sees raw-typed accesses)
- **Before** `legalizeEntryPointVaryingParamsForCUDA` (which may modify function signatures)

The location at line ~1960 satisfies all of these — it's in the target-specific legalization section that runs after all common passes.

#### Pass Structure

```cpp
// slang-ir-legalize-cuda-surface-format.h
void legalizeCUDASurfaceFormat(IRModule* module, DiagnosticSink* sink);

// slang-ir-legalize-cuda-surface-format.cpp
namespace Slang {

struct CUDASurfaceFormatLegalizer {
    IRModule* m_module;
    DiagnosticSink* m_sink;
    IRBuilder m_builder;

    // Get the raw storage type for a given image format + channel count
    IRType* getStorageType(ImageFormat format, int channelCount);

    // Emit IR instructions to decode raw storage bytes → access type value
    IRInst* emitDecode(ImageFormat format, IRInst* rawValue, IRType* accessType);

    // Emit IR instructions to encode access type value → raw storage bytes
    IRInst* emitEncode(ImageFormat format, IRInst* accessValue, IRType* storageType);

    // Resolve the effective image format for a texture resource, checking both
    // IRFormatDecoration (via IRLoad→IRFieldAddress traversal) and the texture
    // type's format operand. Decoration takes priority.
    ImageFormat getEffectiveFormat(IRInst* resourceInst);

    // Check if a call is a CUDA surface read/write intrinsic
    bool isSurfaceReadCall(IRCall* call, IRInst*& outTexture);
    bool isSurfaceWriteCall(IRCall* call, IRInst*& outTexture, IRInst*& outValue);

    // Main entry: scan and rewrite
    void processModule();
};

} // namespace Slang
```

---

### Step 3: Fix `$E` Byte-Address Scaling for Converted Writes

**File**: [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

**Remove the special case** in the `$E` handler (lines 500-509) that sets `elemSizeInBytes = 1` for format-converted writes. After the IR pass, format-converted calls no longer have format decorations, so this code path is dead. But removing it is still necessary for correctness if any edge case bypasses the IR pass.

```cpp
// REMOVE THIS BLOCK from case 'E':
if (IRFormatDecoration* formatDecoration = _findImageFormatDecoration(resourceInst))
{
    const ImageFormat imageFormat = formatDecoration->getFormat();
    if (_isConvertRequired(imageFormat, resourceInst) && _isResourceWrite(m_callInst))
    {
        elemSizeInBytes = 1;
    }
}
```

---

### Step 4: Clean Up `$C` Specifier

**File**: [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

After the IR pass, `$C` should never encounter a format mismatch for CUDA (the pass removes the decoration after lowering). The `$C` code can be simplified or left as-is — it becomes a no-op for format-converted calls since the decoration is gone.

Optionally, add an assertion in `$C` for CUDA targets that format conversion is never needed (to catch cases where the IR pass didn't run):

```cpp
case 'C':
{
    if (isCUDATarget(m_emitter->getTargetReq()))
    {
        IRInst* resourceInst = m_callInst->getArg(0);
        if (IRFormatDecoration* formatDecoration = _findImageFormatDecoration(resourceInst))
        {
            const ImageFormat imageFormat = formatDecoration->getFormat();
            SLANG_ASSERT(!_isConvertRequired(imageFormat, resourceInst) &&
                "CUDA surface format conversion should have been lowered by IR pass");
        }
    }
    break;
}
```

---

### Step 5: Add Slang Tests

**New file**: `tests/cuda/cuda-texture-format-conversion.slang`

```slang
//TEST(compute):COMPARE_COMPUTE:-cuda -compute -output-using-type

//TEST_INPUT:RWTexture2D(format=RGBA8, size=4, content=one):name=tex_unorm8
[format("rgba8")]
RWTexture2D<float4> tex_unorm8;

//TEST_INPUT:RWTexture2D(format=RGBA8SNorm, size=4, content=zero):name=tex_snorm8
[format("rgba8_snorm")]
RWTexture2D<float4> tex_snorm8;

//TEST_INPUT:RWTexture2D(format=RGBA16Float, size=4, content=one):name=tex_f16
[format("rgba16f")]
RWTexture2D<float4> tex_f16;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 v0 = tex_unorm8.Load(int2(0, 0));
    outputBuffer[0] = v0.x;  // CHECK: 1.0

    tex_unorm8.Store(int2(1, 0), float4(0.5, 0.25, 0.75, 1.0));
    float4 v1 = tex_unorm8.Load(int2(1, 0));
    outputBuffer[1] = v1.x;  // CHECK: ~0.498
    outputBuffer[2] = v1.y;  // CHECK: ~0.247

    tex_snorm8.Store(int2(0, 0), float4(-0.5, 0.5, -1.0, 1.0));
    float4 v2 = tex_snorm8.Load(int2(0, 0));
    outputBuffer[3] = v2.x;  // CHECK: ~-0.496
    outputBuffer[4] = v2.z;  // CHECK: -1.0

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

**IR dump test**: Verify the IR pass produces correct conversion arithmetic:
```bash
slangc -dump-ir -target cuda -o /dev/null tests/cuda/cuda-texture-format-conversion.slang
# Should show inline uitofp/mul/fptoui instructions, no format decorations
```

---

### Step 6: Add Diagnostic for Unsupported Packed Formats

Add a diagnostic in the IR pass for formats it doesn't yet handle:

```cpp
case ImageFormat::rgb10_a2:
case ImageFormat::rgb10_a2ui:
case ImageFormat::r11f_g11f_b10f:
    m_sink->diagnose(call, Diagnostics::unsupportedCUDATextureFormat, formatName);
    return; // Leave call unchanged
```

**New file**: `tests/diagnostics/cuda-texture-unsupported-format.slang`

---

## Files to Modify/Create Summary

| File | Changes |
|------|---------|
| **NEW** `source/slang/slang-ir-legalize-cuda-surface-format.cpp` | IR lowering pass (core implementation) |
| **NEW** `source/slang/slang-ir-legalize-cuda-surface-format.h` | Pass declaration |
| [source/slang/slang-emit.cpp](source/slang/slang-emit.cpp) | Register pass in CUDA pipeline |
| [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp) | Remove `$E` scaling special-case; simplify `$C` |
| [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h) | Remove `sust.p` code, `_convert` functions, half-read macros |
| [source/slang/CMakeLists.txt](source/slang/CMakeLists.txt) | Add new source files |
| `tests/cuda/cuda-texture-format-*.slang` | New test files |
| `tests/diagnostics/cuda-texture-unsupported-format.slang` | Diagnostic test for packed formats |

**Files NOT modified** (compared to previous plan):
- ~~`hlsl.meta.slang`~~ — no changes needed; `$C` and `$E` become no-ops after the pass
- ~~`slang-cuda-prelude.h` conversion functions~~ — no new format functions added to prelude

---

## Implementation Order

1. **Remove `sust.p` code** from prelude — clean deletion of `_convert` functions, half-read macros, empty layered stubs.

2. **Create the IR pass** — `slang-ir-legalize-cuda-surface-format.cpp`:
   - Implement intrinsic call detection (identify surface read/write calls on format-decorated textures)
   - Implement `emitDecode()` for reads: raw→access conversion as inline IR
   - Implement `emitEncode()` for writes: access→raw conversion as inline IR
   - Implement texture type rewriting (change element type to raw storage type, remove format decoration)
   - Start with UNORM8 (`rgba8`) + `float4` access only

3. **Register the pass** in `slang-emit.cpp` for CUDA targets.

4. **Remove `$E` scaling special-case** — the dead code for `elemSizeInBytes = 1` on converted writes.

5. **Simplify `$C`** — add assertion that format conversion is never needed for CUDA after the pass runs.

6. **Expand format coverage** in the pass:
   - UNORM16, SNORM8/16 (with correct rounding)
   - Float16 (half↔float bitcast + conversion)
   - Sub-32-bit integers (sign/zero extension)
   - BGRA8 (UNORM8 + swizzle)

7. **Add tests**:
   - Numeric round-trip tests for rgba8, rgba8_snorm, rgba16f, integer formats
   - Multi-dimension tests (1D, 2D, 3D, layered)
   - IR dump verification
   - Diagnostic test for unsupported packed formats

8. **Phase 2 — Packed formats**:
   - Implement `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f` in the pass
   - Remove corresponding diagnostics

---

## Key Design Decisions in the IR Pass

### How to Identify Surface Read/Write Intrinsic Calls

The pass must find `IRCall` instructions that are CUDA surface reads/writes. Strategy:

1. Walk all instructions in all functions
2. For each `IRCall`, check if the callee is an `IRFunc` with an `__intrinsic_asm` body
3. Determine the effective format from **two sources** (check both):
   - **`IRFormatDecoration`** on the texture variable (via the `_findImageFormatDecoration()` traversal — see "Format decoration lookup" in Step 2)
   - **Texture type's format operand** (`IRTextureTypeBase::getFormat()`, operand index 8) — this is set when the user writes `RWTexture2D<unorm float4>` without an explicit `[format]` attribute; the format is inferred during semantic checking and baked into the type. Since `resolveTextureFormat` does NOT run for CUDA, these two sources may not be reconciled; the decoration (if present) takes priority, otherwise fall back to the type's format operand.
4. Check if `_isConvertRequired()` — format storage type differs from element type. Use the effective format from step 3.
5. Determine read vs write: non-void return type = read, void = write

Alternatively, the pass can look at the `IRFormatDecoration` on global texture variables and trace their uses to find all load/store calls — this is similar to how `resolveTextureFormat` works.

### How to Rewrite the Call's Element Type

When the pass finds a read call like `surf2Dread<float4>(surfObj, x, y, boundary)`:

1. Create a new texture type with `elementType = uchar4` (the raw storage type for `rgba8`)
2. The `$T0` specifier in the intrinsic asm reads the element type — after rewriting, it emits `uchar4` instead of `float4`
3. The `$E` specifier computes byte size from the element type — `sizeof(uchar4) = 4`, same as `sizeof(float4)`, so no issue for `rgba8`. But for `r8` (1 byte vs 4 bytes), this matters and is now correct.
4. The call's return type changes to `uchar4`
5. Insert conversion instructions after the call to produce `float4`

For writes: reverse the process — insert conversion before the call, change the value argument type.

### What About the `$C` and `$E` Specifiers After the Pass?

After the pass:
- **`$C`**: The format decoration is removed. `_isConvertRequired()` returns false. `$C` emits nothing. The function name stays `surf2Dread` (no `_convert` suffix). ✅
- **`$E`**: The format decoration is removed, so `_calcBackingElementSizeInBytes()` falls through to the element-type codepath. The element type is now the raw storage type, so it computes the correct byte size. **Invariant**: the element type rewrite must happen before the decoration removal, and the raw storage type must have the same byte size as the original format. For single-channel formats like `r8` (1 byte) accessed as `float4` (16 bytes), the rewrite changes the element type to `uint8_t` (1 byte) — matching the format's `sizeInBytes`. ✅

So both specifiers work correctly without any changes to `hlsl.meta.slang`.

### Half Extension Tracking

The `$C` handler currently triggers `extensionTracker->requireBaseType(BaseType::Half)` for float16 formats. After the pass lowers the call, `$C` no longer fires. The IR pass must instead ensure the half extension is required. This can be done by:
- Having the pass call the extension tracker directly, OR
- The lowered IR will contain `half`-typed instructions, which the CUDA emitter already detects and enables half support for

The second approach is more robust — the emitter already scans for half types.

---

## New CUDA Flow (After This Change)

```
1. AST: [format("rgba8")] RWTexture2D<float4> tex;
2. Lower-to-IR: addFormatDecoration(var, rgba8)
3. IR: IRVar with IRFormatDecoration(rgba8) + IRTextureType(elementType=float4)
4. ... common passes ...
5. legalizeCUDASurfaceFormat pass:
   a. Find: call surf2Dread_intrinsic(%tex, %coord) → float4  [tex has format=rgba8]
   b. Rewrite tex type: elementType = uchar4 (raw storage for rgba8)
   c. Rewrite call: surf2Dread_intrinsic(%tex_raw, %coord) → uchar4
   d. Insert decode: uitofp + mul(1/255) per channel → float4
   e. Remove IRFormatDecoration from tex
6. CUDA emission:
   - $C: no format decoration → no suffix → "surf2Dread"
   - $T0: element type = uchar4 → "surf2Dread<uchar4>"
   - $E: element size = 4 → correct byte scaling
   - Final: "surf2Dread<uchar4>(surfObj, coord.x * 4, coord.y, ...)"
   - Followed by inline conversion arithmetic in emitted CUDA
```

---

## Verification Checklist

- [ ] UNORM8 load returns correct [0,1] float values
- [ ] UNORM8 store+load round-trips correctly (within 1/255 tolerance)
- [ ] SNORM8 handles -1.0 correctly (both -127 and -128 map to -1.0)
- [ ] SNORM encode uses rounding (not truncation)
- [ ] Float16 round-trips preserve precision
- [ ] BGRA8 swizzles b↔r channels correctly
- [ ] All 1D/2D/3D variants work
- [ ] Layered texture reads and writes work (no longer blocked)
- [ ] Byte-address scaling is correct for both reads AND writes
- [ ] Generated CUDA code contains NO `sust.p` instructions
- [ ] Generated CUDA code contains NO `_convert` function calls
- [ ] IR dump shows inline conversion arithmetic (no format decorations remain)
- [ ] Unsupported packed formats emit explicit diagnostics
- [ ] No regressions in existing CUDA tests
- [ ] Generated CUDA code compiles with nvcc
- [ ] Half extension is properly required for float16 format conversions
- [ ] Channel count mismatch works: `[format("r8")] RWTexture2D<float4>` reads `(x, 0, 0, 1)`
- [ ] Channel count mismatch works: `[format("rg16f")] RWTexture2D<float4>` reads `(x, y, 0, 1)`
- [ ] Format on struct field members detected correctly (`IRLoad`→`IRFieldAddress` chain)
- [ ] `RWTexture2D<unorm float4>` (inferred format, no explicit `[format]`) triggers conversion
- [ ] Element type rewrite + decoration removal ordering is correct (type rewrite first)

---

## Resolved Questions

1. ~~**Existing `sust.p` code**~~: **Remove it entirely.** Do not keep behind version guard.
2. **TEST_INPUT format syntax**: Need to verify `RWTexture2D(format=RGBA8, ...)` is correct test input syntax. **Blocker for Step 7** — verify before writing tests.
3. ~~**BGRA format**~~: Handle as `rgba8` with b↔r swizzle in the pass's `emitDecode`/`emitEncode`.
4. ~~**Layered textures**~~: **Now fully supported.** The pass is dimension-agnostic — it rewrites types and inserts conversion regardless of surface shape.
5. ~~**Prelude code volume**~~: **Eliminated.** No format-specific functions in the prelude. All conversion is inline IR.

---

## Future Work (Approach A)

After Phase 1 is stable, consider making CUDA use `kIROp_ImageLoad`/`kIROp_ImageStore` like other backends. This would:
- Unify the IR representation across all targets
- Allow `legalizeImageSubscript` to cover CUDA
- Eliminate the `__intrinsic_asm` surface calls in `hlsl.meta.slang` for CUDA
- Make the format conversion pass cleaner (match on `IRImageLoad`/`IRImageStore` instead of pattern-matching intrinsic calls)
