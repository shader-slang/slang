// sv-clip-distance-vector-unsupported.slang

// Targets and shader shapes that cannot hold a vector-typed SV_ClipDistanceN /
// SV_CullDistanceN (issue #13931) report an error rather than silently keeping only the
// first component.

//DIAGNOSTIC_TEST:SIMPLE(diag=MESH): -target spirv -stage mesh -entry meshMain
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL): -target metal -stage vertex -entry vsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=METALFS): -target metal -stage fragment -entry fsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL): -target wgsl -stage vertex -entry vsMain

// A mesh-shader vertex output is arrayed per vertex; its clip distances do not share the
// flat array that the packing writes into.
struct MeshVertex
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
//MESH:    ^^^^^ vector clip or cull distance in an arrayed varying
//MESH:    ^^^^^ system value semantic 'SV_ClipDistance' cannot have type 'vector<float,4>' here: vector-typed clip and cull distances are only supported for varyings that are not arrays, such as vertex-shader outputs, on this target. Declare it as 'float' or as a 'float' array instead.
};

[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void meshMain(OutputVertices<MeshVertex, 3> verts, OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    verts[0].position = float4(0, 0, 0, 1);
    verts[0].clip0 = float4(1, 2, 3, 4);
    indices[0] = uint3(0, 1, 2);
}

// WGSL reports these at the struct, and Metal without a source location, because neither
// keeps a location for the individual fields of an entry-point output struct.
struct VSOut
//WGSL:^^^^^ system value semantic not supported
//WGSL:^^^^^ system value semantic 'sv_clipdistance' is not supported for the current target.
//WGSL:^^^^^ system value semantic not supported
//WGSL:^^^^^ system value semantic 'sv_culldistance' is not supported for the current target.
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
    float2 cull0 : SV_CullDistance0;
};

//METAL: system value semantic not supported
//METAL: system value semantic 'sv_clipdistance' is not supported for the current target.
//METAL: system value semantic not supported
//METAL: system value semantic 'sv_culldistance' is not supported for the current target.

VSOut vsMain(uint vid: SV_VertexID)
{
    VSOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip0 = float4(1, 2, 3, 4);
    o.cull0 = float2(5, 6);
    return o;
}

float4 fsMain(float2 clip0: SV_ClipDistance0) : SV_Target
//METALFS:           ^^^^^ system value semantic not supported
//METALFS:           ^^^^^ system value semantic 'sv_clipdistance0' is not supported for the current target.
{
    return float4(clip0, 0, 1);
}
