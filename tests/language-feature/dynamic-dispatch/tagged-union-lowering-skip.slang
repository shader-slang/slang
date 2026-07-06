// Companion to tagged-union-lowering-gated.slang for issue #11917.
//
// When no tagged-union IR is present (no dynamic dispatch through an interface value that
// resolves to multiple concrete types), `lowerTaggedUnionTypes` has nothing to lower.
// This is a plain compute baseline that still emits valid output on both textual targets.
// Whether the pass is actually *skipped* here rather than run as a no-op is a
// compile-time-only property that is not observable in emitted output; that gated-vs-
// ungated equivalence is shown by the byte-identical revert-drill described in the PR,
// not by this test. The positive anchors guard against the checks matching empty output
// when emission breaks.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outBuf[i] = inBuf[i] * 2;
}

// A plain-buffer module emits a normal GLSL entry point and nothing else.
// CHECK: void main(

// Same baseline on SPIR-V.
// SPV: OpEntryPoint
