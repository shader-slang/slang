//TEST:SIMPLE(filecheck=CHECK): -target spirv

StructuredBuffer<int> input1;

RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

int MapOp(uint32_t row, uint32_t col, int value)
{
    return value * 2 + 1 + 2 + 3;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(ConstantBuffer<int> cb)
{
    let stride = 16;
    CoopMatType mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);

    // CHECK: ([[# @LINE+1]]):{{.*}}error 51701:
    let result = mat1.MapElement((uint r, uint c, int x)=>x+cb);

    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
