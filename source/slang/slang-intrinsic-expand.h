// slang-intrinsic-expand.h
#ifndef SLANG_INTRINSIC_EXPAND_H
#define SLANG_INTRINSIC_EXPAND_H

#include "slang-emit-c-like.h"

namespace Slang
{

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

    // True when the string starts with `$q` and the call has a split-out sampler argument; see
    // `intrinsicSkipsSplitSamplerArg`.
    bool m_skipCombinedSamplerOperand = false;
};

/// Return true if `intrinsicText` is a `$q` string and the call's argument 1 is the sampler that
/// type legalization split out of a combined texture-sampler. In that case each bare `$N` with
/// N >= 1 reads argument N + 1. The expander and the emitter's fold check both use this, so
/// they agree on which argument a `$N` reads.
bool intrinsicSkipsSplitSamplerArg(
    const UnownedStringSlice& intrinsicText,
    IRUse* args,
    Int argCount);

/// Return the call argument that `$<placeholder>` reads, given the result of
/// `intrinsicSkipsSplitSamplerArg` for the call. For example, `$1` in `"$qtextureLoad($0, $1)"`
/// on a split `Sampler2D` reads argument 2.
Index getIntrinsicPlaceholderArgIndex(Index placeholder, bool skipsSplitSamplerArg);

} // namespace Slang
#endif
