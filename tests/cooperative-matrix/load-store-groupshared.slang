//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -xslang -DMETAL

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

// CHECK_METAL: 1.0
// CHECK_METAL-NEXT: 2.0
// CHECK_METAL-NEXT: 3.0
// CHECK_METAL-NEXT: 4.0
// CHECK_METAL-NEXT: 5.0
// CHECK_METAL-NEXT: 6.0
// CHECK_METAL-NEXT: 7.0
// CHECK_METAL-NEXT: 8.0

// CHECK_METAL: 1.0
// CHECK_METAL-NEXT: 2.0
// CHECK_METAL-NEXT: 3.0
// CHECK_METAL-NEXT: 4.0
// CHECK_METAL-NEXT: 5.0
// CHECK_METAL-NEXT: 6.0
// CHECK_METAL-NEXT: 7.0
// CHECK_METAL-NEXT: 8.0

#pragma warning(disable:41017)

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=inputInt
ByteAddressBuffer inputInt;

// TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferInt
RWStructuredBuffer<int32_t> outputBufferInt;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferInt1
RWStructuredBuffer<int32_t> outputBufferInt1;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, count=256):name=inputFloat
StructuredBuffer<float> inputFloat;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat
RWStructuredBuffer<float> outputBufferFloat;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat2
RWStructuredBuffer<float> outputBufferFloat2;

using namespace linalg;

groupshared int32_t[256] tempSharedInt;
groupshared uint4[256 / 4] tempSharedInt1;
groupshared float[256] tempSharedFloat;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
#ifndef METAL
    let stride = 4;

    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputInt, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(tempSharedInt, 0, stride);

    let result = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(tempSharedInt, 0, stride);
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBufferInt, 0, stride);

    Ptr<int32_t, Access::ReadWrite, AddressSpace::GroupShared> tempSharedPtr = __getAddress(tempSharedInt[0]);
    let matC = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor>(tempSharedPtr, stride);

    Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> tempSharedPtr1 = __getAddress(tempSharedInt1[0]);
    matC.Store<CoopMatMatrixLayout.RowMajor, uint4>(tempSharedPtr1, stride);

    let matC1 = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor>(tempSharedPtr1, stride);
    matC1.Store<CoopMatMatrixLayout.RowMajor>(outputBufferInt1, 0, stride);
#else
    let stride = 8;
    let mat = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputFloat, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(tempSharedFloat, 0, stride);

    let result = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(tempSharedFloat, 0, stride);
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat, 0, stride);

    mat.Store<CoopMatMatrixLayout.RowMajor, uint4, 64>(tempSharedInt1, 0u, uint(stride));
    let result2 = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor, uint4, 64>(tempSharedInt1, 0u, uint(stride));
    result2.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat2, 0u, uint(stride));
#endif
}
