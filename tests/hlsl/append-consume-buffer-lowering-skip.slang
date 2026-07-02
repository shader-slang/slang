// Companion to append-consume-buffer-lowering-gated.slang for issue #11917.
//
// When no Append/Consume structured buffer is present, the gate skips
// `lowerAppendConsumeStructuredBuffers`, and its synthesized accessor functions must
// never appear in the output. The leading CHECK-NOT directives cover the region up to
// the positive `main` anchor (helper functions are emitted before `main`), and the
// anchor guards against the check matching empty output when emission breaks.

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
