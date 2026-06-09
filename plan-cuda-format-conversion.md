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

## Approach: IR Lowering Pass (Approach B)

An IR pass intercepts texture load/store at the IR level and lowers them to raw-typed access + inline conversion arithmetic **before** code emission. This eliminates:

- **All prelude format functions** — conversion math becomes inline IR arithmetic at each call site
- **The `$C`/`$F` specifier mechanism** — the emitter sees raw-typed access with no format mismatch
- **The `$E` scaling bug** — the lowered IR uses the storage type, so byte sizing is naturally correct
- **Dimension-specific specializations** — the surface dimension is orthogonal to format conversion; the pass handles it once

### Why Not a Prelude-Based Approach

The previous plan used switch-dispatched template specializations in the CUDA prelude. This requires **7 surface dimensions × ~9 access types × 2 directions = ~126 specializations**, each with a ~15-case format switch — easily 2000+ lines of prelude code. The IR pass approach is ~400-600 lines in one file.

### CUDA's IR Architecture

**Critical context**: For CUDA, `Load()`/`Store()` do NOT produce `kIROp_ImageLoad`/`kIROp_ImageStore` instructions. Instead, they use `__intrinsic_asm` strings that expand directly during code emission via `$C`/`$E` specifiers. The pass must pattern-match on intrinsic call structure rather than clean IR opcodes.

A future refactor (Approach A) could unify CUDA to use `IRImageLoad`/`IRImageStore` like other backends, but that is a larger change deferred to later.

---

## Reference

### How Other Backends Handle Format Conversion

All GPU backends rely on **dedicated texture units** that perform format conversion in hardware. CUDA surfaces use generic memory hardware without this capability. Software conversion with raw byte I/O is the only reliable path.

`resolveTextureFormat` at `slang-emit.cpp:1905` runs for GLSL, SPIRV, and WGSL (not `SPIRVAssembly` — handled by SPIRV emitter directly; not CUDA/HLSL/Metal).

### CUDA PTX Surface Instructions

| Instruction | Description | Format Conversion |
|-------------|-------------|-------------------|
| `suld.b` | Surface load, byte-addressed | **No** - raw bits |
| `sust.b` | Surface store, byte-addressed | **No** - raw bits |
| ~~`suld.p`~~ / ~~`sust.p`~~ | ~~packed~~ | ❌ **Removed in PTX 3.0** |

The existing `sust.p` code in the prelude is the wrong foundation for correctness.

### Format Categories

From `slang-image-format-defs.h`:

**Scalar-type-backed formats** (Phase 2–4):

1. **UNORM formats**: `rgba8`, `rgba16`, `rg8`, `rg16`, `r8`, `r16` (UINT8/UINT16 → float [0,1])
2. **SNORM formats**: `rgba8_snorm`, `rgba16_snorm`, `rg8_snorm`, `rg16_snorm`, `r8_snorm`, `r16_snorm` (INT8/INT16 → float [-1,1])
3. **Float16 formats**: `rgba16f`, `rg16f`, `r16f` (half → float)
4. **Sub-32-bit integers**: `rgba8i`, `rgba16i`, `rgba8ui`, `rgba16ui`, `rg8i`, `rg16i`, `r8i`, `r16i`, `rg8ui`, `rg16ui`, `r8ui`, `r16ui` (sign/zero extension)

**Packed formats** (Phase 5, deferred — emit diagnostics initially):

5. **Packed formats**: `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f` (complex bit packing)

**Special case — `bgra8`**: Treat as `rgba8` read/write with b↔r channel swizzle (`.zyxw`).

### Existing IR Infrastructure

| Component | Location | Role |
|-----------|----------|------|
| `IRImageLoad` / `IRImageStore` | [slang-ir-insts.h:1878-1903](source/slang/slang-ir-insts.h) | NOT used for CUDA (only SPIRV/GLSL/Metal) |
| `IRTextureTypeBase` | [slang-ir.h:1444](source/slang/slang-ir.h) | Texture type with operand 8 = format |
| `IRFormatDecoration` | [slang-ir-insts.h:476](source/slang/slang-ir-insts.h) | Format attached to texture vars |
| `resolveTextureFormat` pass | [slang-ir-resolve-texture-format.cpp](source/slang/slang-ir-resolve-texture-format.cpp) | Propagates format from `IRFormatDecoration` to texture type's format operand; GLSL/SPIRV/WGSL only |
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

### New CUDA Flow (After This Change)

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

### Format Conversion Formulas

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

**SNORM convention**: Uses OpenGL/Vulkan convention where both -128 and -127 map to -1.0 (via `max(..., -1.0)` clamp). Encode uses `fptosi(round(clamp(val,-1,1) * 127.0))` — produces -127 for -1.0, never -128.

### Coordinate Scaling Verification

**Key invariant**: For all formats, `sizeof(rawStorageType) == format.sizeInBytes`. After the pass removes the format decoration AND rewrites the element type, `$E` falls through to the element-type path and computes the correct byte size.

| Format | Access Type | Raw Storage Type | `$E` After Pass | Correct? |
|--------|-------------|------------------|-----------------|----------|
| `rgba8` | `float4` | `uchar4` | 4 bytes | ✅ |
| `r8` | `float` | `uint8_t` | 1 byte | ✅ |
| `rg8` | `float2` | `uchar2` | 2 bytes | ✅ |
| `rgba16f` | `float4` | `ushort4` | 8 bytes | ✅ |
| `r16f` | `float` | `uint16_t` | 2 bytes | ✅ |
| `rgba8_snorm` | `float4` | `char4` | 4 bytes | ✅ |

**The pass MUST rewrite the element type before removing the format decoration**, otherwise `$E` would compute size from the original access type (e.g., `sizeof(float4) = 16` for `r8` instead of `1`).

### Channel Count Mismatch Handling

**Reads (fewer storage channels → more access channels)**:
- Read `N` raw channels, pad remaining with GPU-standard defaults: **(0, 0, 0, 1)**
- Example: `r8` → `float4` produces `(uitofp(raw)/255.0, 0.0, 0.0, 1.0)`

**Writes (more access channels → fewer storage channels)**:
- Extract only the first `N` channels, discard the rest
- Example: `float4` → `r8` writes only `fptoui(saturate(val.x) * 255.0 + 0.5)`

---

## Implementation Phases

### Phase 1: Extract Shared Utilities

**Goal**: Move format decoration lookup to shared code without changing behavior.

**Files**: [slang-ir-util.h](source/slang/slang-ir-util.h), [slang-ir-util.cpp](source/slang/slang-ir-util.cpp), [slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

**Steps**:

1.1. Add `findImageFormatDecoration(IRInst*)` and `findFormatDecorationOwner(IRInst*)` to `slang-ir-util.{h,cpp}`:

```cpp
IRFormatDecoration* findImageFormatDecoration(IRInst* resourceInst)
{
    // Traverse IRLoad -> IRFieldAddress chains for struct member textures first
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
    return resourceInst->findDecoration<IRFormatDecoration>();
}

IRInst* findFormatDecorationOwner(IRInst* resourceInst)
{
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
    if (resourceInst->findDecoration<IRFormatDecoration>())
        return resourceInst;
    return nullptr;
}
```

1.2. Replace `_findImageFormatDecoration()` in `slang-intrinsic-expand.cpp` with calls to the shared utility.

**Verify**: Existing CUDA tests still pass (no behavior change).

---

### Phase 2: Create IR Pass Skeleton + FLOAT16 Conversion

**Goal**: Create the pass, register it, and implement float16 conversion (replacing prelude `_convert` functions).

**Files**:
- **NEW** `source/slang/slang-ir-legalize-cuda-surface-format.{h,cpp}`
- [slang-emit.cpp](source/slang/slang-emit.cpp)
- [source/slang/CMakeLists.txt](source/slang/CMakeLists.txt)

**Steps**:

2.1. Create pass files with `CUDASurfaceFormatLegalizer` struct:

```cpp
// slang-ir-legalize-cuda-surface-format.h
void legalizeCUDASurfaceFormat(IRModule* module, DiagnosticSink* sink);

// slang-ir-legalize-cuda-surface-format.cpp
struct CUDASurfaceFormatLegalizer {
    IRModule* m_module;
    DiagnosticSink* m_sink;
    IRBuilder m_builder;

    IRType* getStorageType(ImageFormat format, int channelCount);
    IRInst* emitDecode(ImageFormat format, IRInst* rawValue, IRType* accessType);
    IRInst* emitEncode(ImageFormat format, IRInst* accessValue, IRType* storageType);
    ImageFormat getEffectiveFormat(IRInst* resourceInst);
    bool isSurfaceReadCall(IRCall* call, IRInst*& outTexture);
    bool isSurfaceWriteCall(IRCall* call, IRInst*& outTexture, IRInst*& outValue);
    void processModule();
    void processTexture(IRInst* textureInst, ImageFormat format);
    void rewriteSurfaceReadCall(IRCall* call, ImageFormat format);
    void rewriteSurfaceWriteCall(IRCall* call, ImageFormat format);
    void updateTextureType(IRInst* textureInst, IRType* newTextureType);
};
```

2.2. **Implement surface intrinsic call identification**:

For each `IRCall`, resolve the callee to its decorations via `getResolvedInstForDecorations()`, find the `IRTargetIntrinsicDecoration` for the CUDA target, and check if the intrinsic definition string contains `surf` + `read$C` (→ surface read) or `surf` + `write$C` (→ surface write). This is more precise than checking argument types, which could match non-surface operations like `GetDimensions`.

The intrinsic asm pattern determines the argument layout:
- **Reads**: texture is `$0` (arg 0), coordinates are `$1` (arg 1), return type is the element type
- **Writes**: texture is `$0` (arg 0), coordinates are `$1` (arg 1), value is `$2` (arg 2)

```cpp
bool isSurfaceReadCall(IRCall* call, IRInst*& outTexture) {
    auto callee = getResolvedInstForDecorations(call->getCallee());
    if (auto intrinsicDecor = findTargetIntrinsicDecoration(callee, "cuda")) {
        auto defStr = intrinsicDecor->getDefinition();
        if (defStr.indexOf("surf") != -1 && defStr.indexOf("read$C") != -1) {
            outTexture = call->getArg(0);
            return true;
        }
    }
    return false;
}
```

2.3. **Implement `getEffectiveFormat()`**:

The effective format must be resolved from **two sources**:
- **`IRFormatDecoration`** on the texture variable (via the `findImageFormatDecoration()` traversal — handles struct member textures accessed via `IRLoad(IRFieldAddress(...))`)
- **Texture type's format operand** (`IRTextureTypeBase::getFormat()`, operand index 8) — set when the user writes `RWTexture2D<unorm float4>` without an explicit `[format]` attribute; the format is inferred during semantic checking and baked into the type.

Since `resolveTextureFormat` does NOT run for CUDA, these two sources may not be reconciled. The decoration (if present) takes priority; otherwise fall back to the type's format operand. The pass must check both.

2.4. **Implement `processModule()` with correct ordering**:

The pass must process textures atomically to avoid missing calls due to premature decoration removal:

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

        // Check if conversion is needed (format storage type != element type)
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
```

**`processTexture()`** — find all calls via two-level traversal, then rewrite atomically:

```cpp
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

    // 5. Remove format decoration ONCE after all calls are rewritten.
    //    Must remove from the OWNER node (field key for struct members, var for globals).
    IRInst* decorationOwner = findFormatDecorationOwner(textureInst);
    if (decorationOwner) {
        if (auto decoration = decorationOwner->findDecoration<IRFormatDecoration>()) {
            decoration->removeFromParent();
        }
    }
}
```

**Rationale for ordering**: All calls are discovered before any modifications (decoration is still present), all calls for a texture are rewritten before the decoration is removed, and `$C`/`$E` specifiers see consistent state.

2.5. **Implement SSA-correct call rewriting**:

When rewriting a surface read call, the original call returns one type (e.g., `float4`) but the rewritten call returns a different type (e.g., `uchar4`). All users of the original result must be updated to use the conversion result instead.

**IR before/after for reads**:
```
BEFORE:
  %result = call surf2Dread_intrinsic(%tex, %coord) : float4  // format=rgba8

AFTER:
  // 1. Change texture element type to raw storage type (uchar4 for rgba8)
  // 2. Read raw storage bytes
  %raw = call surf2Dread_intrinsic(%tex_raw, %coord) : uchar4

  // 3. Convert each channel: uitofp + multiply by 1/255.0
  %r_uint = extractElement(%raw, 0) : uint8
  %r_f    = uitofp(%r_uint) : float
  %r      = mul(%r_f, 1.0/255.0) : float
  // ... repeat for g, b, a
  %result = makeVector(%r, %g, %b, %a) : float4
```

**Code pattern for reads**:
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

**IR before/after for writes**:
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
  call surf2Dwrite_intrinsic(%raw, %tex_raw, %coord) : void
```

**Code pattern for writes** (simpler — no return value to manage):
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

2.6. **Implement `updateTextureType()`** using use-worklist propagation (follow pattern from `resolveTextureFormatForParameter()` in [slang-ir-resolve-texture-format.cpp](source/slang/slang-ir-resolve-texture-format.cpp#L40-L122)):

When changing a texture's element type, uses of the texture must have their types updated. Handle `kIROp_GetElementPtr`, `kIROp_GetElement`, `kIROp_Load`, `kIROp_Var`, `kIROp_Store` — propagate type changes through dependent instructions:

```cpp
void updateTextureType(IRInst* textureInst, IRType* newTextureType) {
    List<IRUse*> typeReplacementWorkList;
    HashSet<IRUse*> typeReplacementWorkListSet;

    textureInst->setFullType(newTextureType);

    for (auto use = textureInst->firstUse; use; use = use->nextUse) {
        if (typeReplacementWorkListSet.add(use))
            typeReplacementWorkList.add(use);
    }

    for (Index i = 0; i < typeReplacementWorkList.getCount(); i++) {
        auto use = typeReplacementWorkList[i];
        auto user = use->getUser();

        switch (user->getOp()) {
        case kIROp_GetElementPtr:
        case kIROp_GetElement:
        case kIROp_Load:
        case kIROp_Var:
            {
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

**Why rewriting the texture type is safe for target-intrinsic calls**: CUDA surface operations are emitted via `__intrinsic_asm` string templates, not via callee function signatures. The `$T0` specifier reads the element type directly from argument 0's runtime `IRTextureTypeBase::getElementType()` at emit-time — it does **not** validate against the callee's formal generic parameter `T`. Similarly, `$E` reads the element size from the argument's type. The intrinsic expansion system (`IntrinsicExpandContext`) performs pure string template expansion without any type-checking between actual arguments and the callee's declared signature. This means rewriting the texture's element type to the raw storage type (e.g., `float4` → `uchar4`) is transparent to the emitter — `$T0` will correctly emit `uchar4`, and `$E` will correctly compute `sizeof(uchar4) = 4`. No callee respecialization is needed.

**Coordinate scaling critical invariant**: `$E` first checks for an `IRFormatDecoration` and, if found, uses `getImageFormatInfo(format).sizeInBytes`. If no decoration is found, it falls back to computing size from the element type via `_calcBackingElementSizeInBytes()` (slang-intrinsic-expand.cpp:196-220). **After the IR pass removes the format decoration AND rewrites the element type to the raw storage type, `$E` falls through to the element-type path.** This produces the correct byte size — `sizeof(uchar4) = 4` for `rgba8`, `sizeof(uint8_t) = 1` for `r8`, etc. **The element type rewrite is therefore load-bearing for `$E` correctness, not just `$C`.** If the pass removes the decoration but fails to rewrite the element type (e.g., leaves it as `float4` for `r8`), `$E` would compute `sizeof(float4) = 16` instead of the correct `1`, causing incorrect address scaling. **The pass MUST rewrite the element type before removing the format decoration.**

**Half extension tracking**: The `$C` handler currently triggers `extensionTracker->requireBaseType(BaseType::Half)` for float16 formats. After the pass lowers the call, `$C` no longer fires. The lowered IR will contain `half`-typed instructions (from the `bitcast<half>` in the decode path), which the CUDA emitter already detects and enables half support for. **Verify during testing** that float16 format conversion generates `#include <cuda_fp16.h>` in the emitted CUDA source. If the emitter's automatic detection fails, add explicit `extensionTracker->requireBaseType(BaseType::Half)` in the pass (requires threading the extension tracker through the pass API).

2.7. **Implement FLOAT16 conversion** (`emitDecode`/`emitEncode` for r16f, rg16f, rgba16f):
- Decode: `half_to_float(bitcast<half>(raw))`
- Encode: `bitcast<uint16>(float_to_half(val))`

This must be done **before** removing prelude `_convert` functions — existing tests (`tests/compute/half-rw-texture-convert.slang`, `tests/compute/half-rw-texture-convert2.slang`) exercise CUDA half-format conversion.

2.8. Register pass in `slang-emit.cpp` for CUDA targets (before `legalizeEntryPointVaryingParamsForCUDA`):

```cpp
case CodeGenTarget::CUDASource:
case CodeGenTarget::CUDAHeader:
    {
        SLANG_PASS(legalizeCUDASurfaceFormat, codeGenContext->getSink());
        SLANG_PASS(legalizeEntryPointVaryingParamsForCUDA, codeGenContext->getSink());
    }
    break;
```

2.9. Add new source files to `source/slang/CMakeLists.txt`.

**Verify**: `half-rw-texture-convert.slang` and `half-rw-texture-convert2.slang` pass with the new IR pass handling conversion. Confirm `#include <cuda_fp16.h>` appears in emitted code.

---

### Phase 3: Remove Legacy Prelude Code + Fix `$E`/`$C`

**Goal**: Delete dead prelude code and fix/simplify intrinsic specifiers.

**Files**: [slang-cuda-prelude.h](prelude/slang-cuda-prelude.h), [slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp)

**Steps**:

3.1. **Remove from prelude** ([slang-cuda-prelude.h](prelude/slang-cuda-prelude.h)):
- All `SLANG_SURF{1,2,3}DWRITE_CONVERT_IMPL` macros and instantiations (lines 1573-1817) — these emit `sust.p` PTX
- All `surf*Layeredwrite_convert` stub functions (empty bodies = **pre-existing silent data loss bug**)
- `SLANG_SURFACE_READ_HALF_CONVERT` macro and instantiations (lines 1517-1559)

3.2. **Fix `$E`** — remove the special case (lines 500-509) that sets `elemSizeInBytes = 1` for format-converted writes:

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

3.3. **Add `$C` assertion** for CUDA — format conversion should never be needed after the IR pass:

```cpp
case 'C':
{
    if (isCUDATarget(...))
    {
        SLANG_ASSERT(!_isConvertRequired(...) &&
            "CUDA surface format conversion should have been lowered by IR pass");
    }
    break;
}
```

**Verify**: All existing CUDA tests still pass.

---

### Phase 4: UNORM/SNORM/Integer Format Conversion

**Goal**: Implement all scalar-type-backed format conversions and comprehensive tests.

**Files**: `slang-ir-legalize-cuda-surface-format.cpp`, new test files

**Steps**:

4.1. **Implement UNORM8** (rgba8, rg8, r8):
- Decode: `uitofp(raw) * (1.0/255.0)` per channel
- Encode: `fptoui(saturate(val) * 255.0 + 0.5)` per channel
- Handle channel count mismatch (pad reads with 0,0,0,1; truncate writes)

4.2. **Implement UNORM16** (rgba16, rg16, r16):
- Decode: `uitofp(raw) * (1.0/65535.0)`
- Encode: `fptoui(saturate(val) * 65535.0 + 0.5)`

4.3. **Implement SNORM8/SNORM16** (rgba8_snorm, rg8_snorm, r8_snorm, etc.):
- Decode: `max(sitofp(raw) * (1.0/127.0), -1.0)` (SNORM8) / `max(sitofp(raw) * (1.0/32767.0), -1.0)` (SNORM16)
- Encode: `fptosi(round(clamp(val,-1,1) * 127.0))` / `fptosi(round(clamp(val,-1,1) * 32767.0))`

4.4. **Implement INT8/16, UINT8/16** (sign/zero extension + truncation)

4.5. **Implement BGRA8** (UNORM8 + `.zyxw` swizzle)

4.6. **Add tests**:
- `tests/cuda/cuda-texture-format-conversion.slang` — 2D UNORM8 round-trip
- `tests/cuda/cuda-texture-format-1d.slang` — 1D texture
- `tests/cuda/cuda-texture-format-3d.slang` — 3D texture
- `tests/cuda/cuda-texture-format-layered.slang` — Layered texture (now supported)
- `tests/cuda/cuda-texture-format-integer.slang` — 8/16-bit integer formats
- IR dump test: verify `uitofp`/`mul`/`fptoui` instructions, no format decorations

**Example test** (`tests/cuda/cuda-texture-format-conversion.slang`):

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

**IR dump verification**:
```bash
slangc -dump-ir -target cuda -o /dev/null tests/cuda/cuda-texture-format-conversion.slang
# Should show inline uitofp/mul/fptoui instructions, no format decorations
```

**Verify**: All format conversion tests pass. Existing CUDA test suite passes.

---

### Phase 5: Diagnostics + Regression Sweep

**Goal**: Handle unsupported formats gracefully and verify full correctness.

**Steps**:

5.1. **Add diagnostic for unsupported packed formats**:

```cpp
case ImageFormat::rgb10_a2:
case ImageFormat::rgb10_a2ui:
case ImageFormat::r11f_g11f_b10f:
    m_sink->diagnose(call, Diagnostics::unsupportedCUDATextureFormat, formatName);
    return; // Leave call unchanged
```

5.2. **Add diagnostic test**: `tests/diagnostics/cuda-texture-unsupported-format.slang`

5.3. **Full regression testing**:
- Run entire CUDA test suite
- Verify no `sust.p` in generated code
- Verify no `_convert` function calls in generated code
- Verify generated CUDA compiles with nvcc

**Verify**: All items on verification checklist pass.

---

### Phase 6: Packed Formats (Deferred)

**Goal**: Implement complex bit-packed formats.

6.1. Implement `rgb10_a2`, `rgb10_a2ui`, `r11f_g11f_b10f` in the pass.

6.2. Remove corresponding diagnostics.

---

## Files to Modify/Create Summary

| File | Phase | Changes |
|------|-------|---------|
| [slang-ir-util.h](source/slang/slang-ir-util.h) | 1 | Add `findImageFormatDecoration()` and `findFormatDecorationOwner()` |
| [slang-ir-util.cpp](source/slang/slang-ir-util.cpp) | 1 | Implement shared format decoration utilities |
| [slang-intrinsic-expand.cpp](source/slang/slang-intrinsic-expand.cpp) | 1, 3 | Use shared utilities; remove `$E` special-case; add `$C` assertion |
| **NEW** `slang-ir-legalize-cuda-surface-format.h` | 2 | Pass declaration |
| **NEW** `slang-ir-legalize-cuda-surface-format.cpp` | 2, 4 | IR lowering pass (core implementation) |
| [slang-emit.cpp](source/slang/slang-emit.cpp) | 2 | Register pass in CUDA pipeline |
| [CMakeLists.txt](source/slang/CMakeLists.txt) | 2 | Add new source files |
| [slang-cuda-prelude.h](prelude/slang-cuda-prelude.h) | 3 | Remove `sust.p` code, `_convert` functions, half-read macros |
| `tests/cuda/cuda-texture-format-*.slang` | 4 | New test files |
| `tests/diagnostics/cuda-texture-unsupported-format.slang` | 5 | Diagnostic test |

**Files NOT modified**:
- `hlsl.meta.slang` — `$C` and `$E` become no-ops after the pass
- `slang-cuda-prelude.h` — no new format functions added (conversion is inline IR)

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
