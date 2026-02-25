//DIAGNOSTIC_TEST:SIMPLE: -target spirv -entry computeMain -stage compute

// Regression test: ICE "unimplemented: assignment" on out/inout parameters
// Issue: https://github.com/shader-slang/slang/issues/10212
// Two patterns trigger the same crash. This tests Pattern 1 (out param with matrix swizzle).

ConstantBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<uint> output;

bool floatCheck(out float data, float valueToCheckFor) {
    return data < (valueToCheckFor + 0.001) && data > valueToCheckFor - 0.001;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) {
    float4x4 M1 = matrixBuffer;
    float4x4 M2 = mul(M1, M1);
    output[0] = uint(floatCheck(M2._41, 20) && floatCheck(M2._41_32[0], 20));
}
