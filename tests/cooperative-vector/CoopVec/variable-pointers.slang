//TEST(compute):SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// SPIRV: OpCapability VariablePointers
// CHECK: type: int32_t
// CHECK-NEXT: 35
// CHECK-NEXT: 76
// CHECK-NEXT: 117
// CHECK-NEXT: 158

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

//TEST_INPUT:ubuffer(data=[67305985], stride=4),name=input
//[1 2 3 4]
ByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[67305985 134678021 202050057 269422093], stride=4),name=matrix
//[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16]
StructuredBuffer<int32_t> matrix;

//TEST_INPUT:ubuffer(data=[5 6 7 8], stride=4),name=bias
StructuredBuffer<int32_t> bias;

struct MatrixBiasBuffer
{
    StructuredBuffer<int32_t> matrix;
    StructuredBuffer<int32_t> bias;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    let vec = coopVecLoad<4, int8_t>(input);

    MatrixBiasBuffer layerParameters[2];
    for (int i = 0; i < 2; ++i)
    {
        layerParameters[i].matrix = matrix;
        layerParameters[i].bias = bias;
    }

    // Testing that we are not passing in a memory point to `matrix` and `bias`.
    // They should be sent to the driver as a direct reference to the shader parameters.
    var result = coopVecMatMulAdd<int32_t, 4, 4>(
        vec,
        CoopVecComponentType.SignedInt8,
        layerParameters[0].matrix,
        0,
        CoopVecComponentType.SignedInt8,
        layerParameters[0].bias,
        0,
        CoopVecComponentType.SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];;
}

