// sv-clip-distance-vector-unsupported.slang

// Targets and shader shapes that cannot hold a vector-typed SV_ClipDistanceN /
// SV_CullDistanceN (issue #13931) report an error rather than silently keeping only the
// first component.

//DIAGNOSTIC_TEST:SIMPLE(diag=MESH): -target spirv -stage mesh -entry meshMain
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL): -target metal -stage vertex -entry vsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=METALFS): -target metal -stage fragment -entry fsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL): -target wgsl -stage vertex -entry vsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=HULL): -target spirv -stage hull -entry hsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=GSIN): -target spirv -stage geometry -entry gsMain
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAYMIX): -target spirv -stage vertex -entry vsArrayMix
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAYMIX): -target glsl -stage vertex -entry vsArrayMix

// A mesh-shader vertex output is arrayed per vertex; its clip distances do not share the
// flat array that the packing writes into.
struct MeshVertex
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
//MESH:    ^^^^^ vector clip or cull distance in an arrayed varying
//MESH:    ^^^^^ system value semantic 'SV_ClipDistance' cannot have type 'vector<float,4>' here: when targeting SPIR-V or GLSL, vector-typed clip and cull distances are not supported in arrayed varyings (mesh-shader vertex outputs, hull-shader control points, and geometry or tessellation inputs). Declare it as 'float' or as a 'float' array instead.
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

// Hull-shader control-point outputs are arrayed per control point.
struct HSControlPointIn
{
    float4 position : SV_Position;
};

struct HSControlPointOut
{
    float4 position : SV_Position;
    float4 clip0 : SV_ClipDistance0;
//HULL:    ^^^^^ vector clip or cull distance in an arrayed varying
//HULL:    ^^^^^ system value semantic 'SV_ClipDistance' cannot have type 'vector<float,4>' here: when targeting SPIR-V or GLSL, vector-typed clip and cull distances are not supported in arrayed varyings (mesh-shader vertex outputs, hull-shader control points, and geometry or tessellation inputs). Declare it as 'float' or as a 'float' array instead.
};

struct HSPatchConstants
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSPatchConstants patchConstants(InputPatch<HSControlPointIn, 3> patch)
{
    HSPatchConstants o;
    o.edges[0] = 1;
    o.edges[1] = 1;
    o.edges[2] = 1;
    o.inside = 1;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchConstants")]
HSControlPointOut hsMain(InputPatch<HSControlPointIn, 3> patch, uint id: SV_OutputControlPointID)
{
    HSControlPointOut o;
    o.position = patch[id].position;
    o.clip0 = float4(1, 2, 3, 4);
    return o;
}

// Geometry-shader inputs are arrayed per vertex of the input primitive.
struct GSIn
{
    float4 position : SV_Position;
    float2 cull0 : SV_CullDistance0;
//GSIN:    ^^^^^ vector clip or cull distance in an arrayed varying
//GSIN:    ^^^^^ system value semantic 'SV_CullDistance' cannot have type 'vector<float,2>' here: when targeting SPIR-V or GLSL, vector-typed clip and cull distances are not supported in arrayed varyings (mesh-shader vertex outputs, hull-shader control points, and geometry or tessellation inputs). Declare it as 'float' or as a 'float' array instead.
};

struct GSOut
{
    float4 position : SV_Position;
};

[maxvertexcount(1)]
void gsMain(triangle GSIn input[3], inout PointStream<GSOut> stream)
{
    GSOut o;
    o.position = input[0].position + input[1].cull0.x;
    stream.Append(o);
}

// A `float` array keeps a builtin array of its own, so it cannot share its entry point with
// other values of the same semantic.
struct VSArrayMixOut
{
    float4 position : SV_Position;
    float clip[2] : SV_ClipDistance0;
/*ARRAYMIX:
          ^^^^ clip or cull distance array mixed with other values
          ^^^^ a 'SV_ClipDistance' declared as an array must be the only 'SV_ClipDistance' value of its entry point when targeting SPIR-V or GLSL. Declare all of them as one 'float' array, or none of them as an array.
*/
    float2 clip2 : SV_ClipDistance2;
    float cull[2] : SV_CullDistance0;
/*ARRAYMIX:
          ^^^^ clip or cull distance array mixed with other values
          ^^^^ a 'SV_CullDistance' declared as an array must be the only 'SV_CullDistance' value of its entry point when targeting SPIR-V or GLSL. Declare all of them as one 'float' array, or none of them as an array.
*/
    float cull2 : SV_CullDistance2;
};

VSArrayMixOut vsArrayMix(uint vid: SV_VertexID)
{
    VSArrayMixOut o;
    o.position = float4(vid, 0, 0, 1);
    o.clip[0] = 1;
    o.clip[1] = 2;
    o.clip2 = float2(3, 4);
    o.cull[0] = 5;
    o.cull[1] = 6;
    o.cull2 = 7;
    return o;
}
