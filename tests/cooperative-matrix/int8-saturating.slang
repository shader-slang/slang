//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

// CUDA `int8_t` saturating cooperative matrix multiply-add.  Targets PTX
// `mma.sync.m16n8k16.row.col.satfinite.s32.s8.s8.s32`.
//
// Inputs are chosen so that the unsaturated accumulator stays well within
// the int32 range, so the saturating and non-saturating mma give the same
// result.  This focused test exercises the `coopMatMulAdd<int, true>` /
// `mma_sat<char, int32_t, ...>` code path through to PTX without depending
// on the driver's actual saturation behaviour (which is reportedly broken on
// some GPU/driver combinations -- see `mat-mul-add.slang` for context).

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// int8 16x16 identity matrix.
//TEST_INPUT:ubuffer(data=[0x00000001 0 0 0  0x00000100 0 0 0  0x00010000 0 0 0  0x01000000 0 0 0  0 0x00000001 0 0  0 0x00000100 0 0  0 0x00010000 0 0  0 0x01000000 0 0  0 0 0x00000001 0  0 0 0x00000100 0  0 0 0x00010000 0  0 0 0x01000000 0  0 0 0 0x00000001  0 0 0 0x00000100  0 0 0 0x00010000  0 0 0 0x01000000], stride=4, count=64),name=input1
ByteAddressBuffer input1;

// int8 B (16x16): first row [1, 2, 3, 4, 0, ..., 0], rest zero.
//TEST_INPUT:ubuffer(data=[0x04030201 0 0 0], stride=4, count=64),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias I8CoopMatA = CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias I8CoopMatB = CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias I32CoopMat = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let mat1 = I8CoopMatA.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = I8CoopMatB.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);
    I32CoopMat result = I32CoopMat(0);
    // saturatingAccumulation = true -> .satfinite mma form.
    result = coopMatMulAdd<int, true>(mat1, mat2, result);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
