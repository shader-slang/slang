//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2

// CHECK: 0
// CHECK-NEXT: 0
// CHECK: 1
// CHECK-NEXT: 2

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[4 3 5 7], stride=4, count=256),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[2 3 4 5], stride=4, count=256),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias CoopMatIntType = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;
typealias CoopMatFloatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let mat1 = CoopMatIntType.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = CoopMatIntType.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);

    let mat3 = CoopMatFloatType(mat1);
    let mat4 = CoopMatFloatType(mat2);


    let result = mat1 % mat2;
    let result2 = CoopMatIntType(mat3 % mat4);

    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
    result2.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 16, stride);
}
