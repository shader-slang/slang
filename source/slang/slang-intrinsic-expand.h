// slang-intrinsic-expand.h
#ifndef SLANG_INTRINSIC_EXPAND_H
#define SLANG_INTRINSIC_EXPAND_H

#include "slang-emit-c-like.h"

namespace Slang
{

/// The facts that decide how a CUDA surface read or write (`surf*read`/`surf*write`) of a given
/// resource is spelled, and whether the CUDA prelude performs it correctly.
///
/// A CUDA surface access is spelled in two places: the `__intrinsic_asm` strings of the `RWTexture`
/// `Load`/`Store` accessors in hlsl.meta.slang (through `$C` and `$E`), and
/// `CUDASourceEmitter::_emitSurfaceAccess`, which spells the `kIROp_ImageLoad`/`kIROp_ImageStore`
/// produced by `legalizeImageSubscript`. Both take the `_convert` suffix and the x scale from here.
/// The function names, the coordinate order (an array texture's layer last, as CUDA's `Layered`
/// variants take it) and the boundary mode are written out at both sites
/// (shader-slang/slang#13364).
///
/// We keep this next to the `$C`/`$E` expansion so that both spellings share its private format
/// helpers. Only the image-op path acts on `isConversionAvailable`: the accessors' output predates
/// it and is unchanged.
struct CUDASurfaceAccessInfo
{
    /// The access calls the `_convert` variant, because the resource's `[format(...)]` differs
    /// from its element type.
    bool isFormatConversion = false;

    /// The `_convert` call performs the conversion correctly. The prelude's converting read
    /// reinterprets the texel as `half`, so it is correct only for `r16f`, `rg16f` and `rgba16f`;
    /// it has no converting read for array textures, and its converting writes for array textures
    /// do nothing. Always true when `isFormatConversion` is false.
    bool isConversionAvailable = true;

    /// The emitted call uses the CUDA `half` type, so the caller must enable it. Only converting
    /// reads from half-based formats need it.
    bool requiresHalf = false;

    /// The factor applied to the x coordinate. Surface reads and ordinary writes address x in
    /// bytes, so this is the size of the backing element; a converting write (`sust.p`) addresses
    /// x in elements, so it is 1.
    size_t xScale = 1;
};

/// Return the `CUDASurfaceAccessInfo` of a read (`isWrite == false`) or write of the CUDA surface
/// `resourceInst`.
CUDASurfaceAccessInfo getCUDASurfaceAccessInfo(IRInst* resourceInst, bool isWrite);

/// Return the number of dimensions of the CUDA surface functions that access a texture of
/// `shape`: 1, 2 or 3 for `surf1D*`, `surf2D*` and `surf3D*`. Return 0 for a shape we do not
/// spell on CUDA, such as a cube texture. The `RWTexture` accessors in hlsl.meta.slang spell the
/// same three shapes.
Index getCUDASurfaceDimensionCount(SlangResourceShape shape);

/* Handles all the special case handling of expansions of intrinsics. In particular handles the
expansion of the 'special cases' prefixed with '$' */
struct IntrinsicExpandContext
{
    IntrinsicExpandContext(CLikeSourceEmitter* emitter)
        : m_emitter(emitter), m_writer(emitter->getSourceWriter())
    {
    }

    void emit(
        IRCall* inst,
        IRUse* args,
        Int argCount,
        const UnownedStringSlice& intrinsicText,
        IRInst* intirnsicInst);

protected:
    const char* _emitSpecial(const char* cursor);

    SourceWriter* m_writer;
    UnownedStringSlice m_text;
    IRCall* m_callInst;
    IRInst* m_intrinsicInst = nullptr;
    IRUse* m_args = nullptr;
    Int m_argCount = 0;
    Index m_openParenCount = 0;
    CLikeSourceEmitter* m_emitter;

    // An arbitrary offset to apply to argument indices.
    //
    // Note: This is a bit of a gross hack to allow the definitions
    // of the texture-sampling operations to be easier to share
    // between combined and non-combined cases.
    //
    // TODO: It would be great to slowly migrate away from needing
    // so much complicated logic here, but if we decide to keep this
    // general approach it would be great to move some of the processing
    // to the front-end and allow things like:
    //
    //      __target_intrinsic(hlsl, "specialOp($a - $b)")
    //      int SomeCoolFunction(int a, int b);
    //
    // That is, we could try to allow direct by-name references to parameters
    // in the intrinsic strings as they appear in the front-end, and then remap
    // those to be index-based as part of translation to the IR.
    //
    Index m_argIndexOffset = 0;

    // Set by the `$q` marker when a combined texture-sampler has been lowered into a
    // `{texture, sampler}` pair, which inserts a sampler operand at index 1. Texture-only
    // queries (e.g. `GetDimensions`) take no sampler, so their positional `$N` (for N >= 1)
    // indices must skip over that injected sampler operand. This is the counterpart of the
    // `$p` marker (`m_argIndexOffset`): `$p` handles a string numbered with an absent sampler
    // slot, `$q` a string numbered without the sampler that lowering injected. See
    // shader-slang/slang#11669.
    bool m_skipCombinedSamplerOperand = false;
};

} // namespace Slang
#endif
