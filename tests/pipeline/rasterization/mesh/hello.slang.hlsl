#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

static const float3  colors_0[int(3)] = { float3(1.00000000000000000000, 1.00000000000000000000, 0.00000000000000000000), float3(0.00000000000000000000, 1.00000000000000000000, 1.00000000000000000000), float3(1.00000000000000000000, 0.00000000000000000000, 1.00000000000000000000) };
static const float2  positions_0[int(3)] = { float2(0.00000000000000000000, -0.50000000000000000000), float2(0.50000000000000000000, 0.50000000000000000000), float2(-0.50000000000000000000, 0.50000000000000000000) };
struct Vertex_0
{
    float4 pos_0 : SV_Position;
    float3 color_0 : Color;
};

[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void main(uint tig_0 : SV_GROUPINDEX, vertices out Vertex_0  verts_0[int(3)], indices out uint3  triangles_0[int(1)])
{
    SetMeshOutputCounts(3U, 1U);
    if(tig_0 < 3U)
    {
        Vertex_0 _S1 = { float4(positions_0[tig_0], 0.00000000000000000000, 1.00000000000000000000), colors_0[tig_0] };
        verts_0[tig_0] = _S1;
    }
    else
    {
    }
    if(tig_0 < 1U)
    {
        triangles_0[tig_0] = uint3(0U, 1U, 2U);
    }
    else
    {
    }
    return;
}

