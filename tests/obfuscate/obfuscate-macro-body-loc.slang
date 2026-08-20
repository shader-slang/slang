// Regression test: obfuscation pass must not assert-fail or produce duplicate LocPairs
// when IR instructions carry macro-body expansion-range locs (per-invocation tracking
// ranges allocated by registerMacroExpansion that have no backing SourceView).
//
// Before the fix, the hash loop mutated `curLoc` to the unmapped definition-file loc
// after calling findSourceViewThroughExpansion, which corrupted the dedup key. The next
// instruction with the same expansion-range loc created a duplicate LocPair, causing the
// apply loop to advance past it with a mismatched originalLoc and fire the SLANG_ASSERT.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -obfuscate

#define COMPUTE_VALUE(a) ((a) * (a) + 1)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    // COMPUTE_VALUE expands to multiple IR instructions, each carrying the
    // expansion-range loc for the `(a) * (a) + 1` expression tokens.
    // The obfuscation pass must handle these expansion-range locs without
    // hitting the duplicate-LocPair assertion.
    uint v = COMPUTE_VALUE(x);
    outputBuffer[x] = v;
}

// CHECK: OpEntryPoint
