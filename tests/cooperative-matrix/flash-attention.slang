//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Standalone flashAttention_cm kernel with fixed constants and direct half output.

// Check that key opcodes are generated.

// CHECK-DAG: OpCooperativeMatrixMulAdd
// CHECK-DAG: OpCooperativeMatrixPerElementOpNV
// CHECK-DAG: OpCooperativeMatrixReduceNV
// CHECK-DAG: OpCooperativeMatrixLoadTensorNV
// CHECK-DAG: OpCreateTensorLayoutNV
// CHECK-DAG: OpTensorLayoutSetDimensionNV
// CHECK-DAG: OpTensorLayoutSliceNV
// CHECK-DAG: OpCreateTensorViewNV
// CHECK-DAG: OpTensorLayoutSetClampValueNV
// CHECK-DAG: OpTypeTensorLayoutNV
// CHECK-DAG: OpCooperativeMatrixConvertNV

struct FlashAttentionCmStandaloneParams
{
    half* Q;
    half* K;
    half* V;
    half* O;
    uint seq_len_q;
    uint seq_len_kv;
    uint num_heads;
    float scale;
}

static const int kWG_SIZE = 256;
static const int kBR = 64;
static const int kBC = 64;
static const int kBK = 32;
static const int kHeadDim = 64;
static const bool kIsCausal = false;
static const bool kSeqAligned = false;

linalg::TensorLayout<2, linalg::CoopMatClampMode::Constant> makeTensorLayout2D(
    uint dim0,
    uint dim1,
    uint blockSize0,
    uint blockSize1,
    uint stride0,
    uint stride1,
    int clampValue)
{
    linalg.TensorLayout<2, linalg.CoopMatClampMode.Constant> layout;
    return layout.BlockSize(blockSize0, blockSize1)
        .ClampValue(clampValue)
        .Dimension(dim0, dim1)
        .Stride(stride0, stride1);
}

[numthreads(kWG_SIZE, 1, 1)]
void flashAttention_cm_standalone(
    ConstantBuffer<FlashAttentionCmStandaloneParams, CDataLayout> params,
    uint3 groupThreadID : SV_GroupThreadID,
    uint3 groupID : SV_GroupID)
{
    uint baseRow = groupID.x * kBR;
    uint head_idx = groupID.y;
    uint batch_idx = groupID.z;

    const float NEG_INF = -3.402823466e+38f;

    linalg::CoopMat<float, MemoryScope.Workgroup, kBR, kBC, linalg::CoopMatMatrixUse.MatrixAccumulator> mPrevMat;
    linalg::CoopMat<float, MemoryScope.Workgroup, kBR, kBC, linalg::CoopMatMatrixUse.MatrixAccumulator> lPrevMat;
    mPrevMat.fill(NEG_INF);
    lPrevMat.fill(0.0f);
    linalg::CoopMat<float, MemoryScope.Workgroup, kBR, kHeadDim, linalg::CoopMatMatrixUse.MatrixAccumulator> oAcc;
    oAcc.fill(0.0f);

    uint num_kv_blocks = (params.seq_len_kv + uint(kBC) - 1) / uint(kBC);
    uint num_k_tiles = (uint(kHeadDim) + uint(kBK) - 1) / uint(kBK);

    uint qStride = uint(kHeadDim);
    uint kvStride = uint(kHeadDim);
    uint outStride = uint(kHeadDim);
    uint qBatchHeadStride = params.seq_len_q * qStride;
    uint kvBatchHeadStride = params.seq_len_kv * kvStride;
    uint outBatchHeadStride = params.seq_len_q * outStride;
    uint qBatchHeadBase = (batch_idx * params.num_heads + head_idx) * qBatchHeadStride;
    uint kvBatchHeadBase = (batch_idx * params.num_heads + head_idx) * kvBatchHeadStride;
    uint outBatchHeadBase = (batch_idx * params.num_heads + head_idx) * outBatchHeadStride;

    linalg::CoopMat<half, MemoryScope.Workgroup, kBR, kBK, linalg::CoopMatMatrixUse.MatrixA> zeroA;
    zeroA.fill(half(0));
    linalg::CoopMat<half, MemoryScope.Workgroup, kBK, kBC, linalg::CoopMatMatrixUse.MatrixB> zeroB;
    zeroB.fill(half(0));
    linalg::CoopMat<half, MemoryScope.Workgroup, kBC, kHeadDim, linalg::CoopMatMatrixUse.MatrixB> zeroV;
    zeroV.fill(half(0));

    if (!kSeqAligned && baseRow >= params.seq_len_q)
        return;

    for (uint j = 0; j < num_kv_blocks; j++)
    {
        uint baseCol = j * uint(kBC);

        if (!kSeqAligned && baseCol >= params.seq_len_kv)
            continue;
        if (kIsCausal)
        {
            uint maxRow = baseRow + uint(kBR) - 1;
            if (maxRow < baseCol)
                continue;
        }

        linalg::CoopMat<float, MemoryScope.Workgroup, kBR, kBC, linalg::CoopMatMatrixUse.MatrixAccumulator> matS;
        matS.fill(0.0f);

        for (uint kt = 0; kt < num_k_tiles; kt++)
        {
            uint kStart = kt * uint(kBK);

            linalg::TensorLayout<2, linalg::CoopMatClampMode::Constant> qLayout =
                makeTensorLayout2D(
                    params.seq_len_q,
                    uint(kHeadDim),
                    1,
                    1,
                    qStride,
                    1,
                    0).Slice(baseRow, kBR, kStart, kBK);
            uint qBase = qBatchHeadBase;
            let matA =
                linalg::CoopMat<half, MemoryScope.Workgroup, kBR, kBK, linalg::CoopMatMatrixUse.MatrixA>
                    ::Load((void*)(params.Q + qBase), zeroA, qLayout);

            linalg::TensorLayout<2, linalg::CoopMatClampMode::Constant> kLayout =
                makeTensorLayout2D(
                    params.seq_len_kv,
                    uint(kHeadDim),
                    1,
                    1,
                    kvStride,
                    1,
                    0).Slice(baseCol, kBC, kStart, kBK);
            uint kBase = kvBatchHeadBase;
            linalg.TensorView<2, false, 1, 0> view;
            let matB =
                linalg::CoopMat<half, MemoryScope.Workgroup, kBK, kBC, linalg::CoopMatMatrixUse.MatrixB>
                    ::Load((void*)(params.K + kBase), zeroB, kLayout, view);

            matS = linalg::coopMatMulAdd<float, false>(matA, matB, matS);
        }

        FlashAttentionCmStandaloneParams pLocal = params;
        uint baseRowLocal = baseRow;
        uint baseColLocal = baseCol;
        matS = matS.MapElement((uint r, uint c, float elem) =>
        {
            uint row = baseRowLocal + r;
            uint col = baseColLocal + c;
            bool valid = true;
            if (!kSeqAligned)
                valid = (row < pLocal.seq_len_q) && (col < pLocal.seq_len_kv);
            if (kIsCausal && row < col)
                valid = false;
            return valid ? (elem * pLocal.scale) : NEG_INF;
        });
        let rowMaxBC = matS.ReduceRow<kBC>((float a, float b) => max(a, b));
        let rowPrevBC = mPrevMat;
        let rowNewBC = makeTuple(rowMaxBC, rowPrevBC).MapElement((uint r, uint c, float a, float b) => max(a, b));
        let rowExpPrevBC =
            makeTuple(rowPrevBC, rowNewBC).MapElement((uint r, uint c, float a, float b) => exp(a - b));
        let rowExpTileBC =
            makeTuple(rowMaxBC, rowNewBC).MapElement((uint r, uint c, float a, float b) => exp(a - b));
        mPrevMat = rowNewBC;

        matS = makeTuple(matS, rowMaxBC).MapElement((uint r, uint c, float elem, float rowMax) =>
        {
            if (rowMax <= (NEG_INF * 0.5f))
                return 0.0f;
            return exp(elem - rowMax);
        });

        let rowSum = matS.ReduceRow<kBC>((float a, float b) => a + b);
        lPrevMat = lPrevMat * rowExpPrevBC + rowSum * rowExpTileBC;

        let rowExpPrevD = rowExpPrevBC.ReduceRow<kHeadDim>((float a, float b) => a);
        let rowExpTileD = rowExpTileBC.ReduceRow<kHeadDim>((float a, float b) => a);

        let matP = linalg.CoopMat<half, MemoryScope.Workgroup, kBR, kBC, linalg.CoopMatMatrixUse.MatrixAccumulator>(matS)
            .convertUse<linalg::CoopMatMatrixUse.MatrixA>();

        {
            linalg::TensorLayout<2, linalg::CoopMatClampMode::Constant> vLayout =
                makeTensorLayout2D(
                    params.seq_len_kv,
                    uint(kHeadDim),
                    1,
                    1,
                    kvStride,
                    1,
                    0).Slice(baseCol, kBC, 0, kHeadDim);
            uint vBase = kvBatchHeadBase;
            let matV =
                linalg::CoopMat<half, MemoryScope.Workgroup, kBC, kHeadDim, linalg::CoopMatMatrixUse.MatrixB>
                    ::Load((void*)(params.V + vBase), zeroV, vLayout);

            linalg::CoopMat<float, MemoryScope.Workgroup, kBR, kHeadDim, linalg::CoopMatMatrixUse.MatrixAccumulator> matO;
            matO.fill(0.0f);
            matO = linalg::coopMatMulAdd<float, false>(matP, matV, matO);

            oAcc = oAcc * rowExpPrevD + matO * rowExpTileD;
        }
    }

    let rowL = lPrevMat.ReduceRow<kHeadDim>((float a, float b) => a);

    FlashAttentionCmStandaloneParams pLocal = params;
    uint outBaseLocal = outBatchHeadBase;
    uint outStrideLocal = outStride;
    makeTuple(oAcc, rowL)
        .MapElement((uint r, uint c, float elem, float l) =>
        {
            uint row = baseRow + r;
            uint dIdx = c;
            if (row < pLocal.seq_len_q && dIdx < uint(kHeadDim))
            {
                float inv = (l == 0.0f) ? 0.0f : (1.0f / l);
                float val = elem * inv;
                pLocal.O[outBaseLocal + row * outStrideLocal + dIdx] = half(val);
            }
            return elem;
        });
}
