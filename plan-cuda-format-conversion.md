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

**Identifying surface intrinsic calls**: The pass needs to identify which `IRCall` instructions are surface read/write intrinsics. The pass must check the callee's `IRTargetIntrinsicDecoration` for `__intrinsic_asm` strings matching `surf*read$C` or `surf*write$C` patterns. This is more precise than checking only the first argument type + void/non-void return, which could incorrectly match non-surface texture operations like `GetDimensions`.

**Concrete identification algorithm**:
1. For each `IRCall`, resolve the callee to its decorations via `getResolvedInstForDecorations()`
2. Find the `IRTargetIntrinsicDecoration` for the CUDA target
3. Check if the intrinsic definition string contains `surf` and `read$C` (→ surface read) or `surf` and `write$C` (→ surface write)
4. For reads: the texture is arg 0 (`$0`), the return type is the element type
5. For writes: the texture is arg 0 (`$0`), the value is arg 2 (`$2` in the intrinsic asm), coordinates are arg 1 (`$1`)

**Format decoration lookup** (**critical**): The existing `_findImageFormatDecoration()` in `slang-intrinsic-expand.cpp` (lines 132-152) traverses `IRLoad` → `IRFieldAddress` → field key chains to find decorations on struct fields. For example, `myStruct.tex.Load(coord)` produces an `IRLoad` of an `IRFieldAddress`, and the format decoration lives on the field key, not on the `IRLoad` result. **The IR pass must replicate this traversal**, or it will miss format decorations on textures accessed as struct members. The recommended approach is to extract `_findImageFormatDecoration()` into a shared utility in `slang-ir-util.h` and call it from both `slang-intrinsic-expand.cpp` and the new pass.

**Format source duality** (**critical**): The effective format must be resolved from **two sources**:
- **`IRFormatDecoration`** on the texture variable (via the traversal above)
- **Texture type's format operand** (`IRTextureTypeBase::getFormat()`, operand index 8) — this is set when the user writes `RWTexture2D<unorm float4>` without an explicit `[format]` attribute; the format is inferred during semantic checking and baked into the type.

Since `resolveTextureFormat` does NOT run for CUDA, these two sources may not be reconciled. The decoration (if present) takes priority; otherwise fall back to the type's format operand. The pass must check both.

**Modifying the texture type**: The pass must create a new `IRTextureType` with the storage element type (e.g., `uchar4` for `rgba8`) instead of the access type (e.g., `float4`). The format decoration is removed after lowering so `$C` doesn't fire during emission.

**Why rewriting the texture type is safe for target-intrinsic calls** (**important**): CUDA surface operations are emitted via `__intrinsic_asm` string templates, not via callee function signatures. The `$T0` specifier reads the element type directly from argument 0's runtime `IRTextureTypeBase::getElementType()` at emit-time — it does **not** validate against the callee's formal generic parameter `T`. Similarly, `$E` reads the element size from the argument's type. The intrinsic expansion system (`IntrinsicExpandContext`) performs pure string template expansion without any type-checking between actual arguments and the callee's declared signature. This means rewriting the texture's element type to the raw storage type (e.g., `float4` → `uchar4`) is transparent to the emitter — `$T0` will correctly emit `uchar4`, and `$E` will correctly compute `sizeof(uchar4) = 4`. No callee respecialization is needed.

**Coordinate scaling** (**critical invariant**): The raw-typed call must use correct byte-addressing. `$E` first checks for an `IRFormatDecoration` and, if found, uses `getImageFormatInfo(format).sizeInBytes`. If no decoration is found, it falls back to computing size from the element type via `_calcBackingElementSizeInBytes()` (slang-intrinsic-expand.cpp:196-220). **After the IR pass removes the format decoration AND rewrites the element type to the raw storage type, `$E` falls through to the element-type path.** This produces the correct byte size — `sizeof(uchar4) = 4` for `rgba8`, `sizeof(uint8_t) = 1` for `r8`, etc. **The element type rewrite is therefore load-bearing for `$E` correctness, not just `$C`.** If the pass removes the decoration but fails to rewrite the element type (e.g., leaves it as `float4` for `r8`), `$E` would compute `sizeof(float4) = 16` instead of the correct `1`, causing incorrect address scaling. **The pass MUST rewrite the element type before removing the format decoration.**

**Coordinate Scaling Verification Table**:

| Format | Access Type | Raw Storage Type | Format Size (Decoration) | Element Size (After Rewrite) | `$E` Before Pass | `$E` After Pass | Correct? |
|--------|-------------|------------------|--------------------------|-------------------------------|-------------------|-----------------|----------|
| `rgba8` | `float4` | `uchar4` | 4 bytes | 4 bytes | 4 (from decoration) | 4 (from element type) | ✅ |
| `r8` | `float` | `uint8_t` | 1 byte | 1 byte | 1 (from decoration) | 1 (from element type) | ✅ |
| `rg8` | `float2` | `uchar2` | 2 bytes | 2 bytes | 2 (from decoration) | 2 (from element type) | ✅ |
| `rgba16f` | `float4` | `ushort4` | 8 bytes | 8 bytes | 8 (from decoration) | 8 (from element type) | ✅ |
| `r16f` | `float` | `uint16_t` | 2 bytes | 2 bytes | 2 (from decoration) | 2 (from element type) | ✅ |
| `rgba8_snorm` | `float4` | `char4` | 4 bytes | 4 bytes | 4 (from decoration) | 4 (from element type) | ✅ |
| `r8` | `float4` | `uint8_t` | 1 byte | 1 byte | 1 (from decoration) | 1 (from element type) | ✅ (channel mismatch handled by decode) |

**Key invariant**: For all formats, `sizeof(rawStorageType) == format.sizeInBytes`. This ensures `$E` produces the same byte offset before and after the pass, maintaining correct memory addressing.

**Removing format decoration** (**critical**): After lowering, remove the `IRFormatDecoration` from the **owner node that `_findImageFormatDecoration()` found it on** — which may be the texture variable itself, or the struct field key when the texture is a struct member (accessed via `IRLoad(IRFieldAddress(...))`). Removing from the wrong node leaves the decoration alive and causes `$C` to still emit `_convert`, producing a call to a now-deleted prelude function. The pass needs a helper `findFormatDecorationOwner(IRInst* resourceInst) -> IRInst*` (returning the field key or the resource inst itself) and must call `decoration->removeFromParent()` on the result.

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

**Half Extension Requirement**: For float16 formats, the pass must ensure the CUDA half extension is enabled. Since `$C` no longer fires after lowering (the decoration is removed), the original `extensionTracker->requireBaseType(BaseType::Half)` call in `$C` is bypassed. However, the CUDA emitter already detects `half`-typed instructions during emission and enables `#include <cuda_fp16.h>`. The lowered conversion arithmetic contains `half` intermediate values (from the `bitcast<half>` in the decode path), so the detection works automatically. **Verify during testing** that float16 format conversion generates the `#include <cuda_fp16.h>` header. If not, add explicit `extensionTracker->requireBaseType(BaseType::Half)` in the pass.

#### Channel Count Mismatch Handling

The format's channel count may differ from the access type's vector width. For example, `[format("r8")] RWTexture2D<float4>` has a 1-channel format but a 4-component access type. The existing `_isConvertRequired()` already flags this as requiring conversion (via `_isImageFormatCompatible` checking `numElems != imageFormatInfo.channelCount`).

The IR pass must handle this explicitly in `emitDecode()` and `emitEncode()`:

**Reads (fewer storage channels → more access channels)**:
- Read `N` raw channels (where `N` = format channel count)
- Convert each raw channel to the access scalar type
- Pad remaining channels with GPU-standard defaults: **(0, 0, 0, 1)**
  - Missing R/G/B channels → `0.0` (float) / `0` (int)
  - Missing A channel → `1.0` (float) / `1` (int)
- These defaults match the Vulkan spec (§16.1.5) and D3D behavior: missing components are `0` except alpha which is `1`.
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

    // Return the IRInst that owns the IRFormatDecoration for this resource
    // (field key for struct-member textures, or the resource inst itself).
    // Returns nullptr if no decoration is present.
    // Used to remove the decoration after lowering.
    IRInst* findFormatDecorationOwner(IRInst* resourceInst);

    // Check if a call is a CUDA surface read/write intrinsic
    bool isSurfaceReadCall(IRCall* call, IRInst*& outTexture);
    bool isSurfaceWriteCall(IRCall* call, IRInst*& outTexture, IRInst*& outValue);

    // Process all textures and their calls
    void processModule();

    // Process a single texture: find all calls, rewrite them, remove decoration
    void processTexture(IRInst* textureInst, ImageFormat format);

    // Rewrite a single read/write call
    void rewriteSurfaceReadCall(IRCall* call, ImageFormat format);
    void rewriteSurfaceWriteCall(IRCall* call, ImageFormat format);

    // Create a new call with raw storage type (helper)
    IRCall* createRawTypedCall(IRCall* originalCall, ImageFormat format, IRInst* convertedValue = nullptr);

    // Update texture type and propagate to uses
    void updateTextureType(IRInst* textureInst, IRType* newTextureType);
};

} // namespace Slang
```

#### SSA Maintenance and Use-Def Chain Updates

**Critical invariant**: When rewriting a surface read call, the original call returns one type (e.g., `float4`) but the rewritten call returns a different type (e.g., `uchar4`). All users of the original result must be updated to use the conversion result instead.

**Pattern for read calls**:
```cpp
void rewriteSurfaceReadCall(IRCall* originalCall, ImageFormat format) {
    // 1. Create new call with raw storage type
    IRInst* rawCall = createRawTypedCall(originalCall, format);

    // 2. Insert conversion instructions after the call
    m_builder.setInsertAfter(rawCall);
    IRInst* convertedResult = emitDecode(format, rawCall, originalCall->getDataType());

    // 3. Copy source location for debuggability
    convertedResult->sourceLoc = originalCall->sourceLoc;

    // 4. Replace all uses of original call with converted result
    originalCall->replaceUsesWith(convertedResult);

    // 5. Remove original call from IR
    originalCall->removeAndDeallocate();
}
```

**Pattern for write calls** (simpler — no return value to manage):
```cpp
void rewriteSurfaceWriteCall(IRCall* originalCall, ImageFormat format) {
    // 1. Insert conversion instructions before the call.
    //    The write intrinsic asm is e.g. "surf2Dwrite$C<$T0>($2, $0, ($1).x * $E, ...)"
    //    where $0 = this (texture), $1 = coord, $2 = newValue.
    //    So the value to write is getArg(2), NOT getArg(0).
    m_builder.setInsertBefore(originalCall);
    IRInst* originalValue = originalCall->getArg(2); // value to write ($2 in intrinsic asm)
    IRInst* convertedValue = emitEncode(format, originalValue, getStorageType(format));

    // 2. Create new call with converted value and raw-typed texture
    IRInst* rawCall = createRawTypedCall(originalCall, format, convertedValue);
    rawCall->sourceLoc = originalCall->sourceLoc;

    // 3. Remove original call (no uses to replace — void return)
    originalCall->removeAndDeallocate();
}
```

**Type propagation for texture variables**:

When changing a texture's element type, uses of the texture must have their types updated. Follow the pattern from `resolveTextureFormat`:

```cpp
void updateTextureType(IRInst* textureInst, IRType* newTextureType) {
    // Build worklist of all uses that need type updates
    List<IRUse*> typeReplacementWorkList;
    HashSet<IRUse*> typeReplacementWorkListSet;

    textureInst->setFullType(newTextureType);

    for (auto use = textureInst->firstUse; use; use = use->nextUse) {
        if (typeReplacementWorkListSet.add(use))
            typeReplacementWorkList.add(use);
    }

    // Propagate type changes through dependent instructions.
    // This must handle all the same cases as resolveTextureFormatForParameter()
    // in slang-ir-resolve-texture-format.cpp.
    for (Index i = 0; i < typeReplacementWorkList.getCount(); i++) {
        auto use = typeReplacementWorkList[i];
        auto user = use->getUser();

        switch (user->getOp()) {
        case kIROp_GetElementPtr:
        case kIROp_GetElement:
        case kIROp_Load:
        case kIROp_Var:
            {
                // Update user's type and add its uses to worklist
                auto newUserType = replaceImageElementType(user->getFullType(), newTextureType);
                if (newUserType != user->getFullType()) {
                    user->setFullType(newUserType);
                    for (auto u = user->firstUse; u; u = u->nextUse) {
                        if (typeReplacementWorkListSet.add(u))
                            typeReplacementWorkList.add(u);
                    }
                }
                break;
            }
        case kIROp_Store:
            {
                // If the texture is the *value* being stored (not the pointer),
                // update the pointer's type to match.
                auto store = as<IRStore>(user);
                if (use == store->getValUse()) {
                    auto ptr = store->getPtr();
                    auto newPtrType =
                        (IRType*)replaceImageElementType(ptr->getFullType(), newTextureType);
                    if (newPtrType != ptr->getFullType()) {
                        ptr->setFullType(newPtrType);
                        for (auto u = ptr->firstUse; u; u = u->nextUse) {
                            if (typeReplacementWorkListSet.add(u))
                                typeReplacementWorkList.add(u);
                        }
                    }
                }
                break;
            }
        }
    }
}
```

**Reference implementation**: See `resolveTextureFormatForParameter()` in [slang-ir-resolve-texture-format.cpp](source/slang/slang-ir-resolve-texture-format.cpp#L40-L122) for the complete type propagation pattern.

#### Pass Processing Order (Critical for Correctness)

The pass must process textures in a specific order to avoid missing calls due to premature decoration removal:

**Algorithm**:
```cpp
void CUDASurfaceFormatLegalizer::processModule() {
    // 1. Find all global texture parameters needing conversion
    List<Pair<IRInst*, ImageFormat>> texturesToProcess;

    for (auto globalInst : m_module->getGlobalInsts()) {
        IRInst* textureInst = globalInst;

        // Handle both direct texture globals and textures in arrays
        if (auto arrayType = as<IRArrayTypeBase>(globalInst->getDataType())) {
            if (as<IRTextureTypeBase>(arrayType->getElementType()))
                textureInst = globalInst;
        } else if (!as<IRTextureTypeBase>(globalInst->getDataType())) {
            continue;
        }

        // Resolve effective format (decoration takes priority, fall back to type operand)
        ImageFormat format = getEffectiveFormat(textureInst);
        if (format == ImageFormat::unknown)
            continue;

        // Check if conversion is needed
        if (!needsConversion(textureInst, format))
            continue;

        texturesToProcess.add(Pair<IRInst*, ImageFormat>(textureInst, format));
    }

    // 2. Process each texture atomically: find all calls, rewrite all calls, then remove decoration
    for (auto& pair : texturesToProcess) {
        processTexture(pair.first, pair.second);
    }

    // 3. Validate IR is still well-formed
    #ifdef _DEBUG
    validateIRModule(m_module, m_sink);
    #endif
}

void CUDASurfaceFormatLegalizer::processTexture(IRInst* textureInst, ImageFormat format) {
    // 1. Find ALL surface calls using this texture.
    //
    // IMPORTANT: textureInst is a global param whose direct users are IRLoad
    // instructions, NOT IRCall instructions. We must traverse two levels of
    // indirection: IRGlobalParam → IRLoad → IRCall.
    List<IRCall*> readsToRewrite;
    List<IRCall*> writesToRewrite;

    for (auto use = textureInst->firstUse; use; use = use->nextUse) {
        auto user = use->getUser();
        // The global param's direct users are typically IRLoad instructions.
        // We need to look at the users of each IRLoad to find the IRCall.
        IRInst* loadOrUser = user;
        for (auto innerUse = loadOrUser->firstUse; innerUse; innerUse = innerUse->nextUse) {
            if (auto call = as<IRCall>(innerUse->getUser())) {
                IRInst* textureArg;
                IRInst* valueArg;

                if (isSurfaceReadCall(call, textureArg)) {
                    readsToRewrite.add(call);
                } else if (isSurfaceWriteCall(call, textureArg, valueArg)) {
                    writesToRewrite.add(call);
                }
            }
        }
    }

    // 2. Rewrite texture type to use raw storage element type
    IRType* storageType = getStorageType(format, getChannelCount(format));
    IRType* newTextureType = createRawTextureType(textureInst->getDataType(), storageType);
    updateTextureType(textureInst, newTextureType);

    // 3. Rewrite ALL read calls
    for (auto call : readsToRewrite) {
        rewriteSurfaceReadCall(call, format);
    }

    // 4. Rewrite ALL write calls
    for (auto call : writesToRewrite) {
        rewriteSurfaceWriteCall(call, format);
    }

    // 5. Remove format decoration ONCE after all calls are rewritten
    IRInst* decorationOwner = findFormatDecorationOwner(textureInst);
    if (decorationOwner) {
        if (auto decoration = decorationOwner->findDecoration<IRFormatDecoration>()) {
            decoration->removeFromParent();
        }
    }
}
```

**Rationale**: This ordering ensures:
- All calls are discovered before any modifications (decoration is still present)
- All calls for a texture are rewritten before the decoration is removed
- `$C` and `$E` specifiers see consistent state (either all calls have decoration, or none do)
- No race conditions between type updates and call rewriting

---

### Step 2.5: Extract Shared IR Utility Functions

**Files**: [source/slang/slang-ir-util.h](source/slang/slang-ir-util.h) and [source/slang/slang-ir-util.cpp](source/slang/slang-ir-util.cpp)

Extract `_findImageFormatDecoration()` from `slang-intrinsic-expand.cpp` into a shared utility function so both the intrinsic expander and the IR pass can use it:

```cpp
// slang-ir-util.h
IRFormatDecoration* findImageFormatDecoration(IRInst* resourceInst);
IRInst* findFormatDecorationOwner(IRInst* resourceInst);
```

**Implementation** (move from `slang-intrinsic-expand.cpp`):

Note: The check order matches the original `_findImageFormatDecoration()` — check `IRLoad→IRFieldAddress` first, then fall back to the instruction itself. This preserves the original semantics where struct-field decorations take priority.

```cpp
// slang-ir-util.cpp
IRFormatDecoration* findImageFormatDecoration(IRInst* resourceInst)
{
    // Traverse IRLoad -> IRFieldAddress chains for struct member textures first
    // (matches original _findImageFormatDecoration order in slang-intrinsic-expand.cpp)
    if (auto load = as<IRLoad>(resourceInst))
    {
        if (auto fieldAddress = as<IRFieldAddress>(load->getPtr()))
        {
            if (auto fieldKey = fieldAddress->getField())
            {
                if (auto decoration = fieldKey->findDecoration<IRFormatDecoration>())
                    return decoration;
            }
        }
    }

    // Otherwise try directly on the instruction
    return resourceInst->findDecoration<IRFormatDecoration>();
}

IRInst* findFormatDecorationOwner(IRInst* resourceInst)
{
    // Check for decoration on struct field key first
    if (auto load = as<IRLoad>(resourceInst))
    {
        if (auto fieldAddress = as<IRFieldAddress>(load->getPtr()))
        {
            if (auto fieldKey = fieldAddress->getField())
            {
                if (fieldKey->findDecoration<IRFormatDecoration>())
                    return fieldKey;
            }
        }
    }

    // Fall back to direct decoration
    if (resourceInst->findDecoration<IRFormatDecoration>())
        return resourceInst;

    return nullptr;
}
```

**Update `slang-intrinsic-expand.cpp`**: Replace the local `_findImageFormatDecoration()` with a call to the shared utility function.

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
    // Round-trip: 0.5 → fptoui(0.5*255+0.5)=128 → 128/255 ≈ 0.50196
    outputBuffer[1] = v1.x;  // CHECK: ~0.502
    // Round-trip: 0.25 → fptoui(0.25*255+0.5)=64 → 64/255 ≈ 0.25098
    outputBuffer[2] = v1.y;  // CHECK: ~0.251

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
| [source/slang/slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp) | Remove `$E` scaling special-case; simplify `$C`; replace local `_findImageFormatDecoration()` with shared utility |
| [source/slang/slang-ir-util.h](source/slang/slang-ir-util.h) | Add shared `findImageFormatDecoration()` and `findFormatDecorationOwner()` utilities |
| [source/slang/slang-ir-util.cpp](source/slang/slang-ir-util.cpp) | Implement shared format decoration utilities |
| [prelude/slang-cuda-prelude.h](prelude/slang-cuda-prelude.h) | Remove `sust.p` code, `_convert` functions, half-read macros |
| [source/slang/CMakeLists.txt](source/slang/CMakeLists.txt) | Add new source files |
| `tests/cuda/cuda-texture-format-*.slang` | New test files |
| `tests/diagnostics/cuda-texture-unsupported-format.slang` | Diagnostic test for packed formats |

**Files NOT modified** (compared to previous plan):
- ~~`hlsl.meta.slang`~~ — no changes needed; `$C` and `$E` become no-ops after the pass
- ~~`slang-cuda-prelude.h` conversion functions~~ — no new format functions added to prelude

---

## Implementation Order

### Phase 1: Foundation (Steps 1–4)

1. **Extract utility functions** (Step 2.5):
   - Move `_findImageFormatDecoration()` to `slang-ir-util.{h,cpp}`
   - Add `findFormatDecorationOwner()` utility
   - Update `slang-intrinsic-expand.cpp` to use shared utilities
   - **Verify**: Existing CUDA tests still pass (no behavior change)

2. **Create pass skeleton** with SSA maintenance:
   - Create `slang-ir-legalize-cuda-surface-format.{h,cpp}` files
   - Implement `CUDASurfaceFormatLegalizer` struct with all declared methods
   - Implement `processModule()` with correct ordering (find all → rewrite all → remove decoration)
   - Add `validateIRModule()` call in debug builds
   - Register pass in `slang-emit.cpp` for CUDA targets
   - **Verify**: Pass runs but does nothing (no-op), existing tests pass

3. **Implement FLOAT16 conversion** (r16f, rg16f, rgba16f):
   - Implement half ↔ float bitcast + conversion in the pass
   - This must be done **before** removing prelude `_convert` functions, because
     existing tests (`tests/compute/half-rw-texture-convert.slang`,
     `tests/compute/half-rw-texture-convert2.slang`) exercise CUDA half-format
     conversion and will break if the prelude path is removed without a replacement.
   - **Verify**: `half-rw-texture-convert.slang` and `half-rw-texture-convert2.slang`
     pass with the new IR pass handling the conversion instead of the prelude

4. **Remove `sust.p` code from prelude and fix `$E`/`$C`**:
   - Delete `_convert` function macros and instantiations from prelude
   - Delete half-read macros from prelude
   - Delete empty layered write stubs from prelude
   - Remove the `elemSizeInBytes = 1` block from `$E` handler
   - Add assertion in `$C` for CUDA that conversion is never needed after pass runs
   - **Verify**: Existing CUDA tests still pass (half conversion now handled by IR pass)

### Phase 2: Single Format Implementation (Steps 5–7)

5. **Implement UNORM8 conversion** (rgba8 + float4 only):
   - Implement `getEffectiveFormat()` checking both decoration and type operand
   - Implement `needsConversion()` checking format vs access type compatibility
   - Implement `isSurfaceReadCall()` and `isSurfaceWriteCall()` detection
   - Implement `getStorageType()` returning `uchar4` for rgba8
   - Implement `emitDecode()` for rgba8: `uitofp(raw) * (1.0/255.0)` per channel
   - Implement `emitEncode()` for rgba8: `fptoui(saturate(val) * 255.0 + 0.5)` per channel
   - Implement `rewriteSurfaceReadCall()` with `replaceUsesWith()` pattern
   - Implement `rewriteSurfaceWriteCall()` with decoration removal
   - Implement `updateTextureType()` with use-worklist propagation
   - **Verify**: rgba8 format conversion test passes (create test first)

6. **Add IR dump test**:
   - Create test that dumps IR with `-dump-ir`
   - Verify conversion arithmetic is inline (no format decorations, no `_convert` calls)
   - **Verify**: IR shows `uitofp`, `mul`, `fptoui` instructions

7. **Add comprehensive UNORM8 tests**:
   - Round-trip test (write then read)
   - Multi-dimension test (1D, 2D, 3D)
   - Layered texture test
   - Struct member texture test
   - Channel count mismatch test (`r8` → `float4`)
   - **Verify**: All tests pass

### Phase 3: Format Expansion (Steps 8–9)

8. **Add remaining UNORM/SNORM formats**:
   - UNORM16 (divide by 65535)
   - SNORM8/SNORM16 (with `max(..., -1.0)` clamp on decode)
   - Update `getStorageType()` for all format variants
   - **Verify**: Format-specific tests pass

9. **Add integer formats and BGRA8**:
    - INT8/16, UINT8/16 (sign/zero extension + truncation)
    - BGRA8 (UNORM8 + `.zyxw` swizzle)
    - **Verify**: Integer and BGRA8 tests pass

### Phase 4: Polish (Steps 10–11)

10. **Add diagnostics for unsupported formats**:
    - Detect packed formats (rgb10_a2, r11f_g11f_b10f)
    - Emit diagnostic, leave call unchanged
    - **Verify**: Diagnostic test produces expected error

11. **Full regression testing**:
    - Run entire CUDA test suite
    - Verify no `sust.p` in generated code
    - Verify no `_convert` function calls in generated code
    - Verify generated CUDA compiles with nvcc
    - **Verify**: All items on verification checklist pass

### Phase 5: Packed Formats (Deferred)

12. **Implement packed formats**:
    - Implement `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f` in the pass
    - Remove corresponding diagnostics

---

## Key Design Decisions in the IR Pass

### How to Identify Surface Read/Write Intrinsic Calls

The pass must find `IRCall` instructions that are CUDA surface reads/writes. Strategy:

1. Walk all instructions in all functions
2. For each `IRCall`, resolve the callee via `getResolvedInstForDecorations()` and find its `IRTargetIntrinsicDecoration` for the CUDA target
3. Check if the intrinsic definition string matches a surface read (`surf*read$C`) or surface write (`surf*write$C`) pattern. This is more precise than checking argument types, which could match non-surface operations like `GetDimensions`.
4. Determine the effective format from **two sources** (check both):
   - **`IRFormatDecoration`** on the texture variable (via the `findImageFormatDecoration()` traversal — see "Format decoration lookup" in Step 2)
   - **Texture type's format operand** (`IRTextureTypeBase::getFormat()`, operand index 8) — this is set when the user writes `RWTexture2D<unorm float4>` without an explicit `[format]` attribute; the format is inferred during semantic checking and baked into the type. Since `resolveTextureFormat` does NOT run for CUDA, these two sources may not be reconciled; the decoration (if present) takes priority, otherwise fall back to the type's format operand.
5. Check if `_isConvertRequired()` — format storage type differs from element type. Use the effective format from step 4.
6. The intrinsic asm pattern determines the argument layout:
   - **Reads**: texture is `$0` (arg 0), coordinates are `$1` (arg 1), return type is the element type
   - **Writes**: texture is `$0` (arg 0), coordinates are `$1` (arg 1), value is `$2` (arg 2)

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

The `$C` handler currently triggers `extensionTracker->requireBaseType(BaseType::Half)` for float16 formats. After the pass lowers the call, `$C` no longer fires. The lowered IR will contain `half`-typed instructions (from the `bitcast<half>` in the decode path), which the CUDA emitter already detects and enables half support for. This is the preferred approach — the emitter already scans for half types.

**Verification**: During testing, confirm that float16 format conversion generates `#include <cuda_fp16.h>` in the emitted CUDA source. If the emitter's automatic detection fails, add explicit `extensionTracker->requireBaseType(BaseType::Half)` in the pass (requires threading the extension tracker through the pass API).

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
   e. Remove IRFormatDecoration from its owner (tex var or field key)
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
- [ ] Format decoration removed from field key (not texture var) for struct-member textures — no `_convert` suffix emitted after pass runs on struct-member texture
- [ ] `RWTexture2D<unorm float4>` (inferred format, no explicit `[format]`) triggers conversion
- [ ] Element type rewrite + decoration removal ordering is correct (type rewrite first)
- [ ] **SSA Invariants**: IR validation passes after the pass runs (`validateIRModule()` succeeds in debug builds)
- [ ] **Use-Def Chains**: All uses of original call results are updated to conversion results (checked via `-dump-ir`, no dangling uses)
- [ ] **Type Propagation**: Texture type changes propagate to all dependent instructions (`IRLoad`, `IRGetElementPtr`, etc.)
- [ ] **Decoration Removal**: Format decorations removed from correct owner (field key for struct members, var for direct globals)
- [ ] **Insertion Points**: Generated conversion instructions have correct parent blocks (verified via `-dump-ir` structure)
- [ ] **Source Locations**: Conversion instructions have source locations copied from original calls (verified via error messages if conversion traps)

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
