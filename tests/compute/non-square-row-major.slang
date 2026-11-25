// non-square-row-major.slang

// Note! This test doesn't work on CUDA or CPU targets, because both these targets
// assume matrices are tightly packed, whereas GPU targets align rows to 16 bytes.

//TEST:COMPARE_COMPUTE(targets=cpu, filecheck-buffer=PACKED):-output-using-type -compile-arg -O3 -xslang -matrix-layout-row-major
//TEST:COMPARE_COMPUTE(targets=all-cuda-cpu-llvm-metal, filecheck-buffer=ALIGNED):-output-using-type -xslang -matrix-layout-row-major
//TEST:COMPARE_COMPUTE(targets=cuda, filecheck-buffer=PACKED):-output-using-type -xslang -matrix-layout-row-major
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=PACKED): -output-using-type
//TEST:COMPARE_COMPUTE(targets=llvm, filecheck-buffer=PACKED):-output-using-type -xslang -fvk-use-scalar-layout -xslang -matrix-layout-row-major

// matrix<R, C>
//TEST_INPUT:cbuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 0.0 0.0  10.0 20.0 0.0 0.0 ]):name matrixBuffer
ConstantBuffer<float3x2> matrixBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name output
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(1, 2, 1);

    float3x2 M = matrixBuffer;

    float2 r = mul(v, M);

    // ALIGNED: 21
    // ALIGNED: 34

    // PACKED: 12
    // PACKED: 16
    output[0] = r.x;
    output[1] = r.y;
}
