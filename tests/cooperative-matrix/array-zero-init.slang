// A cooperative matrix must never lower to an empty (zero-constituent)
// OpConstantComposite: SPV_KHR_cooperative_matrix requires exactly one
// constituent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

using namespace linalg;

typealias TileSum = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

RWStructuredBuffer<half> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain()
{
    //CHECK: %[[COOPTY:[0-9A-Za-z_]+]] = OpTypeCooperativeMatrixKHR
    TileSum sums[2] = (TileSum[2])0;

    // The trailing `%<operand>` is what an empty composite lacks; the CHECK-NOTs
    // reject the empty form regardless of where in the module it would appear.
    //CHECK-NOT: OpConstantComposite %[[COOPTY]]{{$}}
    //CHECK: OpC{{(onstantComposite|ompositeConstruct)}} %[[COOPTY]] %{{[0-9A-Za-z_]+$}}
    //CHECK-NOT: OpConstantComposite %[[COOPTY]]{{$}}

    // Store the elements so the zero-init stays live (otherwise it is dead-code
    // eliminated and the composite the CHECKs look for is never emitted).
    sums[0].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
    sums[1].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 256, 16);
}
