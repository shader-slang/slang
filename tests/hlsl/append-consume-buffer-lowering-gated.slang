// Regression for https://github.com/shader-slang/slang/issues/11917.
//
// `lowerAppendConsumeStructuredBuffers` is gated on
// `RequiredLoweringPassSet::appendConsumeStructuredBuffer`. This test locks in that the
// gate FIRES when the module contains an Append/Consume structured buffer: on a non-HLSL
// target the types must be lowered to plain buffers plus synthesized accessor functions.
// If the gate predicate were too narrow (a false-negative), the un-lowered types would
// reach the GLSL emitter and compilation would fail, so the CHECK lines below would not
// match.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute

RWStructuredBuffer<int> inBuf;
AppendStructuredBuffer<int> appendBuf;
ConsumeStructuredBuffer<int> consumeBuf;
RWStructuredBuffer<int> outBuf;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    appendBuf.Append(inBuf[i]);
    uint n, s;
    appendBuf.GetDimensions(n, s);
    outBuf[i] = consumeBuf.Consume() + int(n);
}

// The Append/Consume types are lowered into synthesized accessor functions:
// CHECK-DAG: AppendStructuredBuffer_Append
// CHECK-DAG: ConsumeStructuredBuffer_Consume
