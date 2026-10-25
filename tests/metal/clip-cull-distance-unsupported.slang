// MSL has no cull-distance attribute, and defines [[clip_distance]] only for `float` or `float[n]`
// vertex and mesh outputs (reading it in a fragment function is undefined), so Slang reports these
// system values as unsupported for Metal instead of emitting an empty `[[]]` attribute, an
// unattributed parameter or an invalid declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CULL): -target metal -stage vertex -entry vsCull
//DIAGNOSTIC_TEST:SIMPLE(diag=CLIPIN): -target metal -stage fragment -entry psClipInput
//DIAGNOSTIC_TEST:SIMPLE(diag=CULLIN): -target metal -stage fragment -entry psCullInput
//TEST:SIMPLE(filecheck=SHAREDVS): -target metal -stage vertex -entry vsShared
//DIAGNOSTIC_TEST:SIMPLE(diag=SHAREDPS): -target metal -stage fragment -entry psShared
//DIAGNOSTIC_TEST:SIMPLE(diag=TWOCLIP): -target metal -stage vertex -entry vsTwoClip
//DIAGNOSTIC_TEST:SIMPLE(diag=MESHCULL): -target metal -stage mesh -entry msCull
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED): -target metal -stage fragment -entry psNested
//DIAGNOSTIC_TEST:SIMPLE(diag=UNKNOWN): -target metal -stage vertex -entry vsUnknown
//DIAGNOSTIC_TEST:SIMPLE(diag=HALFVS): -target metal -stage vertex -entry vsHalf
//DIAGNOSTIC_TEST:SIMPLE(diag=HALFMESH): -target metal -stage mesh -entry msHalf
//DIAGNOSTIC_TEST:SIMPLE(diag=MESHTWOCLIP): -target metal -stage mesh -entry msTwoClip
//DIAGNOSTIC_TEST:SIMPLE(diag=INNERCOV): -target metal -stage fragment -entry psNestedInnerCoverage

struct CullOut
{
    float4 pos : SV_Position;
    float cull[2] : SV_CullDistance;
//CULL:   ^^^^ error E55202
//CULL:   ^^^^ system value semantic 'sv_culldistance' is not supported for the current target.
};

[shader("vertex")]
CullOut vsCull(uint vid : SV_VertexID)
{
    CullOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.cull[0] = 1.0;
    o.cull[1] = -1.0;
    return o;
}

[shader("fragment")]
float4 psClipInput(float4 pos : SV_Position, float clip : SV_ClipDistance0) : SV_Target
//CLIPIN:                                          ^^^^ error E55202
//CLIPIN:                                          ^^^^ system value semantic 'sv_clipdistance0' is not supported for the current target.
{
    return float4(clip, 0, 0, 1);
}

[shader("fragment")]
float4 psCullInput(float4 pos : SV_Position, float cull : SV_CullDistance0) : SV_Target
//CULLIN:                                          ^^^^ error E55202
//CULLIN:                                          ^^^^ system value semantic 'sv_culldistance0' is not supported for the current target.
{
    return float4(cull, 0, 0, 1);
}

// One struct used as the vertex output and the fragment input: the output maps to
// [[clip_distance]], and the fragment input is reported.
struct SharedVaryings
{
    float4 pos : SV_Position;
    float clip : SV_ClipDistance0;
/*SHAREDPS:
          ^^^^ error E55202
          ^^^^ system value semantic 'sv_clipdistance0' is not supported for the current target.
*/
};

// SHAREDVS: float clip{{[_0-9]*}} {{\[\[}}clip_distance{{\]\]}};
[shader("vertex")]
SharedVaryings vsShared(uint vid : SV_VertexID)
{
    SharedVaryings o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.clip = 1.0;
    return o;
}

[shader("fragment")]
float4 psShared(SharedVaryings input) : SV_Target
{
    return input.pos;
}

// MSL allows one [[clip_distance]] member and Slang does not yet pack several clip-distance outputs
// into one array for Metal, so a second clip-distance output is reported rather than emitted as a
// second attribute, which the Metal compiler rejects.
struct TwoClipOut
{
    float4 pos : SV_Position;
    float clip0 : SV_ClipDistance0;
    float clip1 : SV_ClipDistance1;
//TWOCLIP:^^^^^ error E55220
//TWOCLIP:^^^^^ multiple SV_ClipDistance outputs are not yet supported on Metal; declare all clip distances as one 'float[N]' array with 'SV_ClipDistance'.
};

[shader("vertex")]
TwoClipOut vsTwoClip(uint vid : SV_VertexID, out float clip2 : SV_ClipDistance2)
//TWOCLIP:                                             ^^^^^ error E55220
//TWOCLIP:                                             ^^^^^ multiple SV_ClipDistance outputs are not yet supported on Metal; declare all clip distances as one 'float[N]' array with 'SV_ClipDistance'.
{
    TwoClipOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.clip0 = 1.0;
    o.clip1 = -1.0;
    clip2 = 0.5;
    return o;
}

struct CullMeshVertex
{
    float4 position : SV_Position;
    float cull[2] : SV_CullDistance;
/*MESHCULL:
          ^^^^ error E55202
          ^^^^ system value semantic 'sv_culldistance' is not supported for the current target.
*/
};

[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msCull(
    uint tid : SV_GroupThreadID,
    OutputVertices<CullMeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    CullMeshVertex v;
    v.position = float4(float(tid), 0, 0, 1);
    v.cull[0] = 1.0;
    v.cull[1] = -1.0;
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}

// A clip distance in a nested input struct reaches legalization as a stage_in struct member
// rather than as an entry-point parameter, and is reported the same way.
struct NestedClip
{
    float2 uv : TEXCOORD0;
    float clip : SV_ClipDistance0;
/*NESTED:
          ^^^^ error E55202
          ^^^^ system value semantic 'sv_clipdistance' is not supported for the current target.
*/
};

struct NestedVaryings
{
    float4 pos : SV_Position;
    NestedClip inner;
};

[shader("fragment")]
float4 psNested(NestedVaryings input) : SV_Target
{
    return float4(input.inner.uv, input.inner.clip, 1);
}

// A system value without a Metal case is reported once, not followed by a second diagnostic or an
// internal error.
//UNKNOWN: error E40006
//UNKNOWN: unknown system-value semantic 'sv_shadingrate'
struct UnknownOut
{
    float4 pos : SV_Position;
    uint rate : SV_ShadingRate;
};

[shader("vertex")]
UnknownOut vsUnknown(uint vid : SV_VertexID)
{
    UnknownOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.rate = 0;
    return o;
}

// MSL takes only `float` or `float[n]` for [[clip_distance]], so a `half` clip distance, which the
// front end accepts, is reported rather than emitted as an invalid declaration.
struct HalfClipOut
{
    float4 pos : SV_Position;
    half clip : SV_ClipDistance0;
/*HALFVS:
         ^^^^ error E55202
         ^^^^ system value semantic 'sv_clipdistance' is not supported for the current target.
*/
};

[shader("vertex")]
HalfClipOut vsHalf(uint vid : SV_VertexID)
{
    HalfClipOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.clip = half(1.0);
    return o;
}

struct HalfClipMeshVertex
{
    float4 position : SV_Position;
    half clip[2] : SV_ClipDistance;
/*HALFMESH:
         ^^^^ error E55202
         ^^^^ system value semantic 'sv_clipdistance' is not supported for the current target.
*/
};

[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msHalf(
    uint tid : SV_GroupThreadID,
    OutputVertices<HalfClipMeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    HalfClipMeshVertex v;
    v.position = float4(float(tid), 0, 0, 1);
    v.clip[0] = half(1.0);
    v.clip[1] = half(-1.0);
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}

// The mesh vertex type takes each attribute once, so a second mesh clip distance is reported too.
struct TwoClipMeshVertex
{
    float4 position : SV_Position;
    float clip0 : SV_ClipDistance0;
    float clip1 : SV_ClipDistance1;
/*MESHTWOCLIP:
          ^^^^^ error E55220
          ^^^^^ multiple SV_ClipDistance outputs are not yet supported on Metal; declare all clip distances as one 'float[N]' array with 'SV_ClipDistance'.
*/
};

[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msTwoClip(
    uint tid : SV_GroupThreadID,
    OutputVertices<TwoClipMeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    TwoClipMeshVertex v;
    v.position = float4(float(tid), 0, 0, 1);
    v.clip0 = 1.0;
    v.clip1 = -1.0;
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}

// A system value that Metal provides only for an entry-point parameter has no attribute to put
// on a stage_in member, so a nested SV_InnerCoverage is reported instead of being emitted as
// `[[]]`. The parameter form (`uint ic : SV_InnerCoverage`) still compiles.
struct NestedInnerCoverage
{
    float2 uv : TEXCOORD0;
    uint ic : SV_InnerCoverage;
/*INNERCOV:
         ^^ error E55202
         ^^ system value semantic 'sv_innercoverage' is not supported for the current target.
*/
};

struct InnerCoverageVaryings
{
    float4 pos : SV_Position;
    NestedInnerCoverage inner;
};

[shader("fragment")]
float4 psNestedInnerCoverage(InnerCoverageVaryings input) : SV_Target
{
    return float4(input.inner.uv, float(input.inner.ic), 1);
}
