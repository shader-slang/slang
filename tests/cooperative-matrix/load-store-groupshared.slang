//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
#pragma warning(disable:41017)
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=input
ByteAddressBuffer input;

// TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer1
RWStructuredBuffer<int32_t> outputBuffer1;

using namespace linalg;

groupshared int32_t[256] tempShared;
groupshared uint4[256 / 4] tempShared1;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
    let stride = 4;

    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(input, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(tempShared, 0, stride);

    let result = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(tempShared, 0, stride);
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);

    Ptr<int32_t, Access::ReadWrite, AddressSpace::GroupShared> tempSharedPtr = __getAddress(tempShared[0]);
    let matC = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor>(tempSharedPtr, stride);


    Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> tempSharedPtr1 = __getAddress(tempShared1[0]);
    matC.Store<CoopMatMatrixLayout.RowMajor, uint4>(tempSharedPtr1, stride);

    let matC1 = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor>(tempSharedPtr1, stride);
    matC1.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer1, 0, stride);
}
