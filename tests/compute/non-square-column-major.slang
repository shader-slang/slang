// non-square-column-major.slang

// Note! This test doesn't work on CUDA or CPU targets, because both these targets ignore
// row/column major setting, as well as they handle alignment differently.

//TEST:COMPARE_COMPUTE(targets=all-cuda-cpu-llvm):-output-using-type -xslang -matrix-layout-column-major
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=METAL):-output-using-type
//TEST:COMPARE_COMPUTE(targets=llvm, filecheck-buffer=LLVM):-output-using-type -xslang -matrix-layout-column-major

// matrix<R, C>
//TEST_INPUT:cbuffer(data=[1.0 0.0 10.0 0.0  0.0 1.0 20.0 0.0]):name matrixBuffer
ConstantBuffer<float3x2> matrixBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name output
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(1, 2, 1);

    float3x2 M = matrixBuffer;

    float2 r = mul(v, M);

    // Metal always uses scalar layout + row major, so it is computing
    //              [1  0]
    // [ 1 2 1]  *  [10 0]  = [ 21 1 ]
    //              [0  1]
    // METAL: 21
    // METAL: 1
    //
    // LLVM on CPU uses scalar layout + column major.
    //
    // LLVM: 11
    // LLVM: 1
    output[0] = r.x;
    output[1] = r.y;
}
