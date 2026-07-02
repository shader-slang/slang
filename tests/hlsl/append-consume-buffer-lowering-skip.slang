// Companion to append-consume-buffer-lowering-gated.slang for issue #11917.
//
// When no Append/Consume structured buffer is present, `lowerAppendConsumeStructuredBuffers`
// has nothing to lower (whether it is skipped by the gate or runs as a no-op), so its
// synthesized accessor functions must never appear in the output. This guards against
// over-broad detection that would spuriously rewrite plain buffers. (That the pass is
// actually *skipped* here — a compile-time-only property — is not observable in emitted
// output; it is shown by the byte-identical gated-vs-ungated revert-drill in the PR.)
// The leading CHECK-NOT directives cover the region up to the positive `main` anchor
// (helper functions are emitted before `main`), and the anchor guards against the check
// matching empty output when emission breaks.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute

RWStructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outBuf[i] = inBuf[i] * 2;
}

// CHECK-NOT: AppendStructuredBuffer_Append
// CHECK-NOT: ConsumeStructuredBuffer_Consume
// CHECK: void main(
