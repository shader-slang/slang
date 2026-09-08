// Regression test: obfuscation pass must not assert-fail or produce duplicate LocPairs
// when IR instructions carry macro-body expansion-range locs (per-invocation tracking
// ranges allocated by registerMacroExpansion that have no backing SourceView).
//
// Before the fix, the hash loop mutated `curLoc` to the unmapped definition-file loc
// after calling findSourceViewThroughExpansion, which corrupted the dedup key. The next
// instruction with the same expansion-range loc created a duplicate LocPair, causing the
// apply loop to advance past it with a mismatched originalLoc and fire the SLANG_ASSERT.
//
// Two invocations of COMPUTE_VALUE below give the obfuscation pass two distinct
// expansion ranges to walk through in the same pass, increasing the chance that a given
// run's IR instruction order reproduces the duplicate-LocPair scenario (which requires
// two IR instructions to land on the exact same expansion-range loc; the precise
// instruction sequence a body expression like `(a) * (a) + 1` lowers to is an IR
// codegen detail this test doesn't control directly). The primary crash this test
// reliably reproduces is the simpler one: obfuscateModuleLocs's hash loop calling
// findSourceViewRecursively on an unresolved expansion-range loc at all, which is a
// SLANG_ASSERT (debug-only) -- a regression here is not observable in a release build,
// where SLANG_ASSERT compiles out and this test would still compile successfully and
// satisfy the check directive below, with no protection.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -obfuscate

#define COMPUTE_VALUE(a) ((a) * (a) + 1)
#define SCALE_VALUE(a) ((a) * 2)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    // COMPUTE_VALUE and SCALE_VALUE each expand to multiple IR instructions, each
    // carrying an expansion-range loc for their own body tokens. The obfuscation pass
    // must handle these expansion-range locs, across both invocations' distinct
    // expansion ranges, without hitting the duplicate-LocPair assertion.
    uint v = COMPUTE_VALUE(x);
    uint w = SCALE_VALUE(v);
    outputBuffer[x] = w;
}

// CHECK: OpEntryPoint
