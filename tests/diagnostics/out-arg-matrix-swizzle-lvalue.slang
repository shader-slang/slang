//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -entry computeMain -stage compute

// Regression test for #10212.
//
// Passing a matrix swizzle, or a constant-indexed element of a matrix swizzle,
// to an `out` parameter used to ICE during the call write-back fixup.

ConstantBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<uint> output;

bool floatCheck(out float data, float valueToCheckFor)
{
    bool result = data < valueToCheckFor;
//diag:                ^ use of uninitialized out parameter
//diag:                ^ use of uninitialized out parameter 'data'
    return result;
/*diag:
    ^^^^^^ returning without initializing out parameter
    ^^^^^^ returning without initializing out parameter 'data'
*/
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 M1 = matrixBuffer;
    float4x4 M2 = mul(M1, M1);
    output[0] = uint(floatCheck(M2._41, 20.0) && floatCheck(M2._41_32[0], 20.0));
}
