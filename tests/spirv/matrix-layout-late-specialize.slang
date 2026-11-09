//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -matrix-layout-row-major -conformance "Thing<float>:IThing=0"

// Generic bodies from an imported module are cloned lazily when the conformance's specialized
// type is instantiated - after specializeMatrixLayout. The unknown-layout matrix minted there
// used to lower to a second, identically named _MatrixStorage struct (the disassembler
// suffixes it _0), and an OpPtrAccessChain mixing the two fails spirv-val.

import "matrix-layout-late-specialize-module";

uniform StructuredBuffer<IThing> things;
RWStructuredBuffer<float> outBuf;

[Differentiable]
float f(IThing t, no_diff int i, float4 v)
{
    return t.eval(i, v);
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    var dv = diffPair(float4(1), float4(0));
    __bwd_diff(f)(things[0], int(tid.x), dv, 1.0f);
    outBuf[0] = dv.d.x;
}

// The positive check anchors the negative one so a storage-struct rename cannot make it
// vacuous. `natural` is the buffer layout rule, orthogonal to -matrix-layout-row-major.
// CHECK: OpEntryPoint
// CHECK: %_MatrixStorage_float4x4natural = OpTypeStruct
// CHECK-NOT: _MatrixStorage_float4x4natural_0
