//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile cs_6_10 -entry computeMain
//TEST:SIMPLE(filecheck=HLSL_U):-target hlsl -profile cs_6_10 -entry computeMainU

// Regression test for the HLSL lowering of `CoopMat.Load/Store` on `groupshared`
// arrays. Both must lower to the groupshared overloads of
// `dx::linalg::Matrix::Load/Store`. Before they had an `hlsl` case, `Store` was
// rejected with E36107 and `Load` was silently dropped together with every
// instruction that depended on the loaded matrix, leaving an empty kernel.

using namespace linalg;

typealias MatA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;
typealias MatB = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>;
typealias MatC = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

StructuredBuffer<half> inputBuffer;
RWByteAddressBuffer outputBuffer;

groupshared half s_Stage[2 * 16 * 16];
groupshared float s_Tile[16 * 16];

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    for (uint i = tid.x; i < 512; i += 32)
        s_Stage[i] = inputBuffer[i];
    GroupMemoryBarrierWithGroupSync();

    // The loads, the multiply-accumulate and the stores must all survive to the output.
    // HLSL: ::Load((s_Stage_0), (0U), (16U), dx::linalg::MatrixLayout::RowMajor)
    // HLSL: ::Load((s_Stage_0), (256U), (16U), dx::linalg::MatrixLayout::ColMajor)
    let a = MatA.Load<CoopMatMatrixLayout.RowMajor>(s_Stage, 0, 16);
    let b = MatB.Load<CoopMatMatrixLayout.ColumnMajor>(s_Stage, 256, 16);
    MatC c = MatC(0.0f);
    c = coopMatMulAdd<float, false, half, half, float, MemoryScope.Subgroup, 16, 16, 16>(a, b, c);

    // HLSL: .Store((s_Tile_0), (0U), (16U), dx::linalg::MatrixLayout::RowMajor)
    // HLSL: .Store((s_Tile_0), (0U), (16U), dx::linalg::MatrixLayout::ColMajor)
    c.Store<CoopMatMatrixLayout.RowMajor>(s_Tile, 0, 16);
    c.Store<CoopMatMatrixLayout.ColumnMajor>(s_Tile, 0, 16);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer.Store(tid.x * 4, asuint(s_Tile[tid.x]));
}

groupshared uint s_Words[16 * 16];

// The `Load<layout, U, V>` / `Store<layout, U, V>` overloads take an array whose element type
// differs from the matrix element type.
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMainU(uint3 tid: SV_DispatchThreadID)
{
    // HLSL_U: ::Load((s_Words_0), (0U), (16U), dx::linalg::MatrixLayout::RowMajor)
    let c = MatC.Load<CoopMatMatrixLayout.RowMajor>(s_Words, 0, 16);
    // HLSL_U: .Store((s_Words_0), (0U), (16U), dx::linalg::MatrixLayout::ColMajor)
    c.Store<CoopMatMatrixLayout.ColumnMajor>(s_Words, 0, 16);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer.Store(tid.x * 4, s_Words[tid.x]);
}
