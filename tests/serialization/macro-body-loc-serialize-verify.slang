//TEST:SIMPLE(filecheck=CHECK):-verify-debug-serial-ir -target spirv -entry computeMain -stage compute

// Regression test for SerialSourceLocWriter::addSourceLoc (slang-serialize-source-loc.cpp) and
// the matching SourceLoc round-trip check in SerialContainerUtil::verifyIRSerialize
// (slang-serialize-container.cpp).
//
// Before this test's fix, addSourceLoc dropped every macro-body IR instruction's loc to
// SourceLoc(0) on serialization, because macro-body locs are per-invocation expansion-range
// locs (see MacroInvocation::_remapBodyLoc in slang-preprocessor.cpp) with no direct
// SourceView, and addSourceLoc looked them up with the non-recursive findSourceView. This
// silently discarded source locations for every instruction originating inside a macro body --
// including, since the embedded core module is compiled from hlsl.meta.slang/core.meta.slang
// macros, essentially every core-module-sourced instruction in a default (SLANG_EMBED_CORE_MODULE
// = ON) build.
//
// addSourceLoc now falls back to findSourceViewThroughExpansion to recover the definition-file
// loc instead of dropping it. -verify-debug-serial-ir additionally exercises
// SerialContainerUtil::verifyIRSerialize, which independently re-derives the same unmapped loc
// from the original (pre-serialization) IR instruction and compares offsets/humane locs against
// the deserialized copy; a mismatch here reports the "Verification of serial debug information
// failed" diagnostic (Diagnostics::SerialDebugVerificationFailed) -- which, unlike the
// SLANG_ASSERTs this code also carries, fires in release builds too, so this test's protection
// is not debug-only.

#define COMPUTE_VALUE(a) ((a) * (a) + 1)

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    // Every IR instruction COMPUTE_VALUE's body lowers to carries a macro-body
    // expansion-range loc; the debug-serial round trip must handle them all.
    uint v = COMPUTE_VALUE(x);
    outputBuffer[x] = v;
}

// CHECK-NOT: Verification of serial debug information failed
// CHECK: OpEntryPoint
