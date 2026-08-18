//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Top-level uniform int** global. The compiler packs uniform globals into
// a GlobalParams struct inside a ConstantBuffer, so the multi-level rule
// fires through the ConstantBuffer element-type path — NOT through the
// IRPtrTypeBase discovery path (which handles bare pointer-to-struct cases).
//
// See: https://github.com/shader-slang/slang/issues/10841

uniform int** pp;
RWStructuredBuffer<int> output;

// The multi-level pointer should be lowered to ulong inside GlobalParams.
// CHECK: struct GlobalParams
// CHECK: ulong pp

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = **pp;
}
