// coopmat-map-element-crash.slang
//TEST:SIMPLE:-target spirv -profile glsl_450 -stage compute -entry coopmat_map_element_repro -emit-spirv-directly

struct ReproParams
{
    float* output;
    int outputW;
};

[numthreads(256, 1, 1)]
void coopmat_map_element_repro(
    ConstantBuffer<ReproParams> params,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID)
{
    static const int TILE_M = 64;
    static const int TILE_N = 32;
    linalg::CoopMat<float, MemoryScope.Workgroup, TILE_M, TILE_N, linalg::CoopMatMatrixUse.MatrixAccumulator> matC;
    matC.fill(1.0f);

    matC = matC.MapElement((uint row, uint col, float acc) => {
        int outIdx = col * params.outputW;
        params.output[outIdx] = acc;
        return acc;
    });
}
