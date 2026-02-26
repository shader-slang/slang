//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry computeMain -stage compute
// CHECK: OpEntryPoint GLCompute
// CHECK: computeMain
// CHECK-NOT: internal compiler error

// Regression test: internal compiler error "unimplemented: assignment" on out/inout parameters
// Issue: https://github.com/shader-slang/slang/issues/10212
// Pattern 1 trigger: out parameter with matrix-swizzle l-value targets.
// Pattern 2 is covered in tests/bugs/gh-10212-pattern2.slang.

ConstantBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<uint> output;

// Intentionally using `out` here to preserve the original trigger for Pattern 1.
// Do not change this to `inout` in this regression test.
bool floatCheck(out float data, float valueToCheckFor) {
    return data < (valueToCheckFor + 0.001) && data > valueToCheckFor - 0.001;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) {
    float4x4 M1 = matrixBuffer;
    float4x4 M2 = mul(M1, M1);
    output[0] = uint(floatCheck(M2._41, 20) && floatCheck(M2._41_32[0], 20));
}
