//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// 1. float groupshared T[V] round-trip
// CHECK: type: float
// CHECK: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0

// 2. cross-type groupshared uint4[V] round-trip
// CHECK: type: float
// CHECK: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 6.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0

#pragma warning(disable:41017)

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, count=256):name=inputFloat
StructuredBuffer<float> inputFloat;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat
RWStructuredBuffer<float> outputBufferFloat;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat2
RWStructuredBuffer<float> outputBufferFloat2;

using namespace linalg;

groupshared float[256] tempSharedFloat;
groupshared uint4[64] tempSharedUint4;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
    let stride = 8;

    // 1. Load from buffer, store to groupshared float[], load back, store to output
    let mat = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputFloat, 0u, uint(stride));
    mat.Store<CoopMatMatrixLayout.RowMajor>(tempSharedFloat, 0u, uint(stride));

    let result = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor>(tempSharedFloat, 0u, uint(stride));
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat, 0u, uint(stride));

    // 2. Store to groupshared uint4[] (cross-type), load back, store to output
    mat.Store<CoopMatMatrixLayout.RowMajor, uint4, 64>(tempSharedUint4, 0u, uint(stride));
    let result2 = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>.Load<CoopMatMatrixLayout.RowMajor, uint4, 64>(tempSharedUint4, 0u, uint(stride));
    result2.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat2, 0u, uint(stride));
}
